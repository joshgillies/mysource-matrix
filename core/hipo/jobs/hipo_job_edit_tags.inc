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
* $Id: hipo_job_edit_tags.inc,v 1.18 2012/08/30 01:09:18 ewang Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to edit content tags
*
* @author  Elden McDonald <emcdonald@squiz.net>
* @version $Revision: 1.18 $
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Edit_Tags extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name	a unique codename the HIPO
	*/
	function HIPO_Job_Edit_Tags($code_name='')
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
		$o->openField(translate('edit_tag_threshold'));

		echo '<span class="sq-inline-fields-wrapper">';
		if ($write_access) {
			text_box($class.'[SQ_HIPO_TAG_THRESHOLD]', SQ_HIPO_TAG_THRESHOLD, 1);
		} else {
			echo SQ_HIPO_TAG_THRESHOLD;
		}
		echo ' '.translate('assets');
		$o->note(translate('hipo_value_divided_by_tag'));
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
				'SQ_HIPO_TAG_THRESHOLD'	=> Array('editable'	=> 1, 'default'	=> '15'),
			   );

	}//end getConfigVars()


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* The Edit Tag HIPO appends the ID of the asset that the tags are being set at
	* (the top most parent) so that any other tag jobs on this asset do not start while it
	* is running. Due to locking of the interface, nobody should be allowed to run this HIPO at
	* the same time on the same parent asset.
	*
	* @return string
	* @access public
	*/
	function getCodeName()
	{
		return parent::getCodeName().'-'.md5(implode('-',array_keys($this->_running_vars['assets'])));

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('hipo_name_edit_tags');

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
					'name'			=> translate('hipo_updating_tags'),
					'function_call'	=> Array(
										'process_function'	=> 'processTags',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
			   );

	}//end getInitialStepData()


	/**
	* Returns the current Threshold.
	*
	* Returns a value that represents the total percentage that this hipo would run
	* in freestyle mode in its current state
	*
	* @return float
	* @access public
	*/
	function getThresholdPercentageRequired()
	{

		if (SQ_HIPO_TAG_THRESHOLD == 0) return 0;
		if (!isset($this->_running_vars['todo_assetids'])) {
			return 0;
		}
		return ((count($this->_running_vars['todo_assetids']) / (SQ_HIPO_TAG_THRESHOLD / count($this->_running_vars['tag_changes']))) * 100);


	}//end getThresholdPercentageRequired()


	/**
	* Performs the duties of this hipo, without showing any output on the frontend.
	*
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		while (!empty($this->_running_vars['todo_assetids'])) {
			if (!$this->processTags($this->_steps[0], get_class($this))) {
				return FALSE;
			}
		}
		return TRUE;

	}//end freestyle()


	/**
	* Prepares the running vars and gets the children of the asset in question
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		// to start editing tags, we need to have an asset id to start editing tags at
		if (empty($this->_running_vars['assets'])) {
			trigger_localised_error('HIPO0017', E_USER_WARNING);
			return FALSE;
		}

		$this->_running_vars['done_assetids'] = Array();

		if (empty($this->_running_vars['tag_changes'])) {
			// no tag changes, so do nothing
			$this->_running_vars['todo_assetids'] = Array();
		} else {
			$assets = $this->_running_vars['assets'];
			$child_assets = Array();
			for (reset($assets); NULL !== ($assetid = key($assets)); next($assets)) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				if (is_null($asset)) {
					trigger_localised_error('HIPO0016', E_USER_WARNING, $assetid);
					return '';
				}
				$child_assets += $GLOBALS['SQ_SYSTEM']->am->getChildren($assetid, Array(), NULL, FALSE, NULL, NULL, TRUE, NULL, NULL, TRUE, NULL, Array(SQ_SC_LINK_BACKEND_NAV));

				$child_assets[$assetid]	= Array (
											0	=> Array (
													'type_code'	=> $asset->type(),
												   ),
										  );

				// getChildren is not supposed to return any assets that are 
				// linked by notice links but at time of writing it did, resulting in
				// tags getting tagged. Check for assets returned that have no type
				$child_assets = array_filter($child_assets);	//TODO: Running array_filter wouldnt have any effect, since it's multi-dimensional array now.
				unset($asset);
			}
			$this->_running_vars['todo_assetids'] = $child_assets;
		}

		return parent::prepare();

	}//end prepare()


	/**
	* Edit Tags for a single asset in the to-do list
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processTags(&$step_data, $prefix)
	{
		if (!empty($this->_running_vars['todo_assetids'])) {
			$tag_manager = $GLOBALS['SQ_SYSTEM']->getTagManager();
			reset($this->_running_vars['todo_assetids']);
			$assetid	= key($this->_running_vars['todo_assetids']);
			$asset_type	= $this->_running_vars['todo_assetids'][$assetid][0]['type_code'];
			unset($this->_running_vars['todo_assetids'][$assetid]);

			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $asset_type);

			if (!is_null($asset)) {
				if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($assetid, 'links')) {
					foreach ($this->_running_vars['tag_changes'] as $tag_data) {
						if (!is_array($tag_data['tag_id'])) {
							$tag_data['tag_id'] = Array($tag_data['tag_id']);
						}
						switch ($tag_data['action']) {
							case 'add':
								foreach ($tag_data['tag_id'] as $tag_data_id) {
									// if this asset is already tagged with this tag, update the weight
									$link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($asset->id, $tag_data_id, SQ_LINK_NOTICE);
									if (empty($link)) {
										$tag_manager->setTag($asset->id, $tag_data_id, $tag_data['weight']);
									} else {
										$tag_manager->setTagWeight($link['linkid'], $tag_data['weight']);
									}
								}
							break;

							case 'delete':
								$tag_manager->deleteTag($asset->id, $tag_data['tag_id']);
							break;
						}
					}
					$step_data['message'] = translate('hipo_editing_tags', htmlentities($asset->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET));
					$GLOBALS['SQ_SYSTEM']->am->releaseLock($assetid, 'links');
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
					unset($asset);
				} else {
					// we do not acquire lock
					trigger_localised_error('SYS0100', E_USER_WARNING, 'links', $asset->name, ' skipping asset.');
				}
			} else {
				$step_data['message'] = translate('hipo_skipping_asset', $assetid);
				$this->_addError(translate('hipo_cannot_edit_tags', $assetid));
			}

			// add this assetid to the done array so we don't do it again
			$this->_running_vars['done_assetids'][] = $assetid;
		}//end if there are assets to process

		if (empty($this->_running_vars['todo_assetids'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
			unset($this->_running_vars['done_assetids']);
		} else {
			$total = count($this->_running_vars['todo_assetids']) + count($this->_running_vars['done_assetids']);
			$step_data['percent_done'] =(count($this->_running_vars['done_assetids']) / $total) * 100;
			$step_data['complete']     = FALSE;
		}

		return TRUE;

	}//end processTags()


	/**
	* This function is used to set-up the details for the locking the assets by the nested HIPO_Job
	*
	* @param string	&$job_type		Job type
	* @param array	&$running_vars	hipo job's running vars
	* @param array	&$options		Options supplied
	*
	* @return void
	* @access public
	*/
	function getLockAssetDetails(&$job_type, &$running_vars, &$options)
	{
		$job_type = 'hipo_job_acquire_locks';

		$options['auto_complete'] = TRUE;


		$running_vars = Array(
							'assetids'			=> array_keys($this->_running_vars['assets']),
							'lock_type'			=> Array('links'),
							'forceably_acquire'	=> FALSE,
							'dependants_only'	=> FALSE,
						);

	}//end getLockAssetDetails()


	/**
	* Release the locks we acquired while setting the tags
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function releaseLocks(&$step_data, $prefix)
	{
		if (!isset($this->_running_vars['re_lock_assets'])) {
			$this->_running_vars['re_lock_assets'] = $this->_running_vars['assets'];
		}

		$assetid = key($this->_running_vars['re_lock_assets']);
		unset($this->_running_vars['re_lock_assets'][$assetid]);

		$children = $GLOBALS['SQ_SYSTEM']->am->getDependantChildren($assetid);

		foreach ($children as $dep_child_id => $dep_child_data) {
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($dep_child_id, 'links');
		}

		$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid));

		$step_data['message'] = translate('hipo_release_locks', $asset_info[$assetid]['name']).translate('hipo_release_locks_dependents');

		$GLOBALS['SQ_SYSTEM']->am->releaseLock($assetid, 'all');

		$this->_running_vars['done_assetids'][] = $assetid;

		if (empty($this->_running_vars['re_lock_assets'])) {
			unset($this->_running_vars['done_assetids']);
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
		} else {
			$step_data['percent_done'] = (count($this->_running_vars['done_assetids']) / count($this->_running_vars['assets'])) * 100;
			$step_data['complete']     = FALSE;
		}

		return TRUE;

	}//end releaseLocks()


}//end class

?>
