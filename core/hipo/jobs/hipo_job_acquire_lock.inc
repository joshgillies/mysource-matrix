<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: hipo_job_acquire_lock.inc,v 1.23.2.2 2004/02/26 18:22:38 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to acquire locks
*
* <pre>
* Initial Running Vars :
*    int    assetid             -   the assetid with whose children we are going to attempt to lock
*    Array(string)|string lock_type           -   the type of lock(s) to get see Asset_Manager::acquireLock()
*    bool   [dependants_only]   -   whether dependants only or all children, defaults to 
*    bool   [forceably_acquire] -   whether to attempt to forceably acquire the lock or not, defaults to false
* </pre>
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Acquire_Lock extends HIPO_Job
{

	/**
	* The name of this HIPO job (should not be modified by code)
	* @var string
	*/
	var $hipo_name = 'Acquire Lock';

	/**
	* Array of steps to perform in this HIPO
	* @var Array
	*/
	var $_steps = Array(
					Array(
						'name'             => 'Acquiring Lock',
						'function_call'    => Array(
												'process_function' => 'processLocking',
												),
						'running_mode'     => 'server',
						'auto_step'        => true,
						'percent_done'     => 0,
						'complete'         => false,
						'message'          => '',
					),
				);


	/**
	* Constructor
	*
	* @param string	$code_name		a unique codename the HIPO
	*/
	function HIPO_Job_Acquire_Lock($code_name='')
	{
		$this->HIPO_Job($code_name);

	}//end constructor


	/**
	* Paints backend of this job
	*
	* @param object Backend_Outputter	&$o			reference to the backend outputter
	* @param string						$class		calling class name
	*
	* @return void
	* @access private
	*/
	function paintConfig(&$o, $class)
	{
		$write_access = $this->writeAccess();
		
		// metadata regeneration threshhold HIPO config entry
		$o->openField('Acquire Lock Threshold');

		if($write_access) {
			text_box($class.'[SQ_HIPO_ACQUIRE_LOCK_THRESHOLD]', SQ_HIPO_ACQUIRE_LOCK_THRESHOLD, 5);
		} else {
			echo SQ_HIPO_ACQUIRE_LOCK_THRESHOLD;
		}

		$o->closeField();

	}//end paintConfig()

	
	/**
	* Sets some info about thresholds that this hipo requires.
	*
	* @return Array
	* @access public
	*/
	function getConfigVars()
	{
		return Array(
			'SQ_HIPO_ACQUIRE_LOCK_THRESHOLD' => Array('editable' => 1, 'default' => 1),
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
		return parent::getCodeName().'-'.$this->_running_vars['assetid'].'-'.implode(',',$this->_running_vars['lock_types']);

	}//end getCodeName()


	/**
	* Resets the vars that the HIPO steps use
	*
	* @param Array	$vars
	*
	* @return void
	* @access public
	*/
	function setRunningVars($vars)
	{
		if (isset($vars['lock_type']) && isset($vars['assetid'])) {
			// OK, the main reason that we are getting the lock types here is so that we don't end up 
			// with lock types in a child being locked that the parent doesn't have, because if they do then
			// we end up with a source lockid that is not actually creatable
			// EG. 'mail' lock is available in 'Inbox', but not 'User' and when you locked the user you would end
			//     up with a lock 'asset.5.mail' and a source lock 'asset.4.mail' which is not available
			// -- If this doesn't make sense ask me -- BCR
			
			if (is_array($vars['lock_type'])) {
				$vars['lock_types'] = Array();
				foreach ($vars['lock_type'] as $lock_type) {
					$vars['lock_types'] = array_merge($vars['lock_types'], $GLOBALS['SQ_SYSTEM']->am->getLockTypes($vars['assetid'], $lock_type));
					$vars['lock_types'] = array_unique($vars['lock_types']);
				}
			} else {
				$vars['lock_types'] = $GLOBALS['SQ_SYSTEM']->am->getLockTypes($vars['assetid'], $vars['lock_type']);
			}
		}

		return parent::setRunningVars($vars);

	}//end setRunningVars()


	/**
	* Returns the current Threshold.
	*
	* Returns a value that represents the total percentage that this hipo would run in freestyle mode in its current state.
	* 
	* @return float
	* @access public
	*/
	function getThreshold()
	{
		if (SQ_HIPO_ACQUIRE_LOCK_THRESHOLD == 0) return 0;
		if (!isset($this->_running_vars['todo_assetids'])) return 0;
		// note that we add 1 to the cound to take the root assetid into account
		return (((count($this->_running_vars['todo_assetids']) + 1) / SQ_HIPO_ACQUIRE_LOCK_THRESHOLD) * 100);
	
	}//end getThreshold()


	/**
	* Performs the duties of this hipo, without showing any output on the frontend.
	* 
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		while (!empty($this->_running_vars['todo_assetids']) || !$this->_running_vars['done_root']) {
			if (!$this->processLocking($this->_steps[0], get_class($this))) return false;
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
		if (empty($this->_running_vars['assetid']) || empty($this->_running_vars['lock_type'])) {
			trigger_error('Cannot initialise Acquire Locks HIPO, no start asset ID and/or lock type specified', E_USER_WARNING);
			return '';
		}

		// so we have an assetid to start at, but make sure it is a valid assetid
		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['assetid']);
		if (is_null($asset)) {
			trigger_error('Cannot initialise Acquire Locks HIPO, invalid start asset ID specified ('.$this->_running_vars['assetid'].')', E_USER_WARNING);
			return '';
		}

		// we were passed an invalid lock type, bugger off
		if (empty($this->_running_vars['lock_types'])) {
			trigger_error('Cannot initialise Acquire Locks HIPO, lock type "'.$this->_running_vars['lock_type'].'" invalid', E_USER_WARNING);
			return '';
		}

		// we don't need this anymore, we use the array 'lock_types'
		unset($this->_running_vars['lock_type']);

		if (!isset($this->_running_vars['todo_assetids']) || empty($this->_running_vars['todo_assetids'])) {
			// we have a valid asset, so get all the assets below this one in the tree(s) so we know what assets
			// we will be acquiring locks on

			// if there is no dependants setting, or if it is true, get only dependants
			if (!isset($this->_running_vars['dependants_only']) || !empty($this->_running_vars['dependants_only'])) {
				$this->_running_vars['dependants_only'] = false;
				$child_assets = $GLOBALS['SQ_SYSTEM']->am->getDependantChildren($asset->id);
			} else {
				// get locks for all children
				$this->_running_vars['dependants_only'] = true;
				$child_assets = $GLOBALS['SQ_SYSTEM']->am->getChildren($asset->id);
			}
			$this->_running_vars['todo_assetids'] = $child_assets;
		}

		$this->_running_vars['done_root']     = false;
		$this->_running_vars['done_assetids'] = Array();

		if (!isset($this->_running_vars['forceably_acquire'])) {
			$this->_running_vars['forceably_acquire'] = false;
		} else {
			$this->_running_vars['forceably_acquire'] = (bool) $this->_running_vars['forceably_acquire'];
		}

		unset($asset);

		return parent::prepare();
	
	}//end prepare()


	/**
	* Set up vars and database information that the HIPO will need to run
	*
	* The Acquire Lock HIPO also checks that there is an assetid to start this action on
	* (the top most parent). It also saves a list of assets to perform this action on in the running vars.
	*
	* @param string		$source_code_name	the code name of another job that wants to use this job
	*
	* @return string	returns the code_name for the new job
	* @access public
	*/
	function initialise($source_code_name=null)
	{
		if (!$this->is_prepared) {
			$this->prepare();
		}
		return parent::initialise($source_code_name);

	}//end initialise()


	/**
	* Acquire a lock on a single asset in the to-do list
	*
	* @param array()	&$step_data	a reference to the array of information about the current step
	* @param string		$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processLocking(&$step_data, $prefix)
	{
		if (!empty($this->_running_vars['todo_assetids']) || !$this->_running_vars['done_root']) {

			if (empty($this->_running_vars['done_assetids'])) {
				// make sure we process the root asset first
				$assetid = $this->_running_vars['assetid'];
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				$this->_running_vars['done_root'] = true;
			} else {
				$assetid = array_shift(array_keys($this->_running_vars['todo_assetids']));
				$asset_type = $this->_running_vars['todo_assetids'][$assetid];
				unset($this->_running_vars['todo_assetids'][$assetid]);
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $asset_type);
			}

			if (!is_null($asset)) {
				foreach($this->_running_vars['lock_types'] as $lock_type) {
					$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, $lock_type, $this->_running_vars['assetid'], $this->_running_vars['forceably_acquire']);
				}
				$step_data['message'] = 'Acquiring Lock on "'.$asset->name.'"';
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				unset($asset);
			} else {
				$step_data['message'] = 'Skipping asset '.$assetid;
				$this->_addError('Could not Acquire Lock for asset #'.$assetid.', asset does not exist');
			}

			// add this assetid to the done array so we dont do it again
			$this->_running_vars['done_assetids'][] = $assetid;
		}

		if (empty($this->_running_vars['todo_assetids'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = true;
		} else {
			$total = count($this->_running_vars['todo_assetids']) + count($this->_running_vars['done_assetids']);
			$step_data['percent_done'] =(count($this->_running_vars['done_assetids']) / $total) * 100;
			$step_data['complete']     = false;
		}

		return true;

	}//end processLocking()

}//end class

?>