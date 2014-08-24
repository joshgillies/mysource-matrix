<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: hipo_job_acquire_locks.inc,v 1.29 2012/08/30 01:09:18 ewang Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to acquire locks
*
* <pre>
* Initial Running Vars :
*    array(int)           assetids          - the assetids with whose children we are going to attempt to lock
*    array(string)|string lock_type         - the type of lock(s) to get (see Asset_Manager::acquireLock())
*    bool                 dependants_only   - whether dependants only or all children, defaults to true
*    bool                 forceably_acquire - whether to attempt to forceably acquire the lock or not,  defaults to false
* </pre>
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.29 $
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Acquire_Locks extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name		a unique codename the HIPO
	*/
	function HIPO_Job_Acquire_Locks($code_name='')
	{
		$this->HIPO_Job($code_name);

	}//end constructor


	/**
	* Paints backend of this job
	*
	* @param object	&$o		reference to the backend outputter
	* @param string	$class	calling class name
	*
	* @return void
	* @access private
	*/
	public static function paintConfig(&$o, $class, $write_access)
	{
		// metadata regeneration threshhold HIPO config entry
		$o->openField(translate('acquire_lock_threshold'));

		echo '<span class="sq-inline-fields-wrapper">';
		if ($write_access) {
			text_box($class.'[SQ_HIPO_ACQUIRE_LOCK_THRESHOLD]', SQ_HIPO_ACQUIRE_LOCK_THRESHOLD, 1);
		} else {
			echo SQ_HIPO_ACQUIRE_LOCK_THRESHOLD;
		}
		echo ' '.translate('assets');
		echo '</span>';

		$o->closeField();

	}//end paintConfig()


	/**
	* Sets some info about thresholds that this hipo requires.
	*
	* @return array
	* @access public
	*/
	public static function getConfigVars()
	{
		return Array(
				'SQ_HIPO_ACQUIRE_LOCK_THRESHOLD'	=> Array('editable'	=> 1, 'default'	=> 20),
			   );

	}//end getConfigVars()


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* The Acquire Lock HIPO appends the ID of the asset that the lock is being acquired on
	* (the top most parent) so that any other locking jobs on this asset do not start while it
	* is running. Due to locking of the interface, nobody should be allowed to run this HIPO at
	* the same time on the same parent asset.
	*
	* @return string
	* @access public
	*/
	function getCodeName()
	{
		$code_name = parent::getCodeName();
		return $code_name.'-'.md5(implode(',', $this->_running_vars['assetids']));

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('hipo_name_acquire_locks');

	}//end getHipoName()


	/**
	* Returns the steps in this hipo, possibly with localised step names
	* and messages
	*
	* @return string
	* @access public
	*/
	function getInitialStepData()
	{
		return Array(
				Array(
					'name'			=> translate('hipo_acquiring_locks'),
					'function_call'	=> Array(
										'process_function'	=> 'processLocking',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> true,
					'allow_cancel'	=> true,
					'percent_done'	=> 0,
					'complete'		=> false,
					'message'		=> '',
				),
			   );

	}//end getInitialStepData()


	/**
	* Returns asset lock types
	*
	* @param int	$assetid	Asset ID
	*
	* @return void
	* @access public
	*/
	function getAssetLockTypes($assetid)
	{
		$types = Array();
		if (isset($this->_running_vars['lock_type']) && isset($assetid)) {
			if (is_array($this->_running_vars['lock_type'])) {
				foreach ($this->_running_vars['lock_type'] as $lock_type) {
					foreach ($GLOBALS['SQ_SYSTEM']->am->getLockTypes($assetid, $lock_type) as $type) {
						if (!in_array($type, $types)) $types[] = $type;
					}
				}
			} else {
				$types = $GLOBALS['SQ_SYSTEM']->am->getLockTypes($assetid, $this->_running_vars['lock_type']);
			}
		}
		return $types;

	}//end getAssetLockTypes()


	/**
	* Returns the current Threshold.
	*
	* Returns a value that represents the total percentage that this hipo would run in
	* freestyle mode in its current state.
	*
	* @return float
	* @access public
	*/
	function getThresholdPercentageRequired()
	{
		if (SQ_HIPO_ACQUIRE_LOCK_THRESHOLD == 0) return 0;
		if (!isset($this->_running_vars['child_assetids'])) {
			return 0;
		}
		// note that we add 1 to the count to take the root assetid into account
		return ((($this->_running_vars['total_count'] + 1) / SQ_HIPO_ACQUIRE_LOCK_THRESHOLD) * 100);

	}//end getThresholdPercentageRequired()


	/**
	* Performs the duties of this hipo, without showing any output on the frontend.
	*
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		while (!empty($this->_running_vars['assetids'])/* || !$this->_running_vars['done_root']*/) {
			if (!$this->processLocking($this->_steps[0], get_class($this))) {
				return false;
			}
		}
		return true;

	}//end freestyle()


	/**
	* Prepares the running vars and gets the children of the asset in question.
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		// to start acquiring locks, we need to have an asset id to acquire a lock on
		if (empty($this->_running_vars['assetids']) || empty($this->_running_vars['lock_type'])) {
			trigger_localised_error('HIPO0035', E_USER_WARNING);
			return false;
		}

		if (!isset($this->_running_vars['todo_assetids']) || empty($this->_running_vars['todo_assetids'])) {
			// we have a valid asset, so get all the assets below this one in the tree(s) so we know what assets
			// we will be acquiring locks on

			$child_assetids = Array();
			$this->_running_vars['total_count'] = 0;
			// if there is no dependants setting, or if it is true, get only dependants
			if (!isset($this->_running_vars['dependants_only']) || !empty($this->_running_vars['dependants_only'])) {
				$this->_running_vars['dependants_only'] = true;
				foreach ($this->_running_vars['assetids'] as $assetid) {
					$this->_running_vars['total_count']++;
					$child_assets[$assetid] = $GLOBALS['SQ_SYSTEM']->am->getDependantChildren($assetid);
					$this->_running_vars['total_count'] += count($child_assets[$assetid]);

				}
			} else {
				// get locks for all children
				$this->_running_vars['dependants_only'] = false;
				foreach ($this->_running_vars['assetids'] as $assetid) {
					$this->_running_vars['total_count']++;
					$child_assets[$assetid] = $GLOBALS['SQ_SYSTEM']->am->getChildren($assetid);
					$this->_running_vars['total_count'] += count($child_assets[$assetid]);
				}
			}
			$this->_running_vars['child_assetids'] = $child_assets;
			$this->_running_vars['current_assetid'] = reset($this->_running_vars['assetids']);
		} else {
			$this->_running_vars['current_assetid'] = key($this->_running_vars['todo_assetids']);
			$this->_running_vars['total_count'] = count($this->_running_vars['todo_assetids']);
		}


		$this->_running_vars['done_assetids'] = Array();

		if (!isset($this->_running_vars['forceably_acquire'])) {
			$this->_running_vars['forceably_acquire'] = false;
		} else {
			$this->_running_vars['forceably_acquire'] = (bool) $this->_running_vars['forceably_acquire'];
		}

		return parent::prepare();

	}//end prepare()


	/**
	* Acquire a lock on a single asset in the to-do list
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processLocking(&$step_data, $prefix)
	{
		if (!empty($this->_running_vars['assetids'])) {
			$parent_assetid = $this->_running_vars['current_assetid'];

			if (!in_array($parent_assetid, $this->_running_vars['done_assetids'])) {
				$assetid = $parent_assetid;
				$asset =$GLOBALS['SQ_SYSTEM']->am->getAsset($parent_assetid);
			} else {
				$parent_assetid_keys = array_keys($this->_running_vars['child_assetids'][$parent_assetid]);
				$assetid = array_shift($parent_assetid_keys);
				if (is_array($this->_running_vars['child_assetids'][$parent_assetid][$assetid])) {
					$asset_type = $this->_running_vars['child_assetids'][$parent_assetid][$assetid][0]['type_code'];
				} else {
					$asset_type = $this->_running_vars['child_assetids'][$parent_assetid][$assetid];
				}
				unset($this->_running_vars['child_assetids'][$parent_assetid][$assetid]);
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $asset_type);
			}

			if (!is_null($asset)) {
				$lock_types = $this->getAssetLockTypes($parent_assetid);
				foreach ($lock_types as $lock_type) {
					$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, $lock_type, $parent_assetid, $this->_running_vars['forceably_acquire']);
				}
				$step_data['message'] = translate('hipo_acquire_lock', htmlentities($asset->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET));
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				unset($asset);
			} else {
				$step_data['message'] = translate('hipo_skipping_asset', $assetid);
				$this->_addError(translate('hipo_asset_not_exist', $assetid));
			}

			// add this assetid to the done array so we dont do it again
			$this->_running_vars['done_assetids'][] = $assetid;

			// if we are done with the current parent asset's children, then move onto the next one
			if (empty($this->_running_vars['child_assetids'][$parent_assetid])) {
				array_shift($this->_running_vars['assetids']);
				if (!empty($this->_running_vars['assetids'])) {
					$this->_running_vars['current_assetid'] = $this->_running_vars['assetids'][0];
				}
			}
		}

		if (empty($this->_running_vars['assetids'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = true;
		} else {
			$step_data['percent_done'] = (count($this->_running_vars['done_assetids']) / $this->_running_vars['total_count']) * 100;
			$step_data['complete']     = false;
		}

		return true;

	}//end processLocking()


}//end class

?>
