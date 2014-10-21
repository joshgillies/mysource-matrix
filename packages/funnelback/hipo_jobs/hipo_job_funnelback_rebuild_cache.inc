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
* $Id: hipo_job_funnelback_rebuild_cache.inc,v 1.3 2012/08/30 00:58:43 ewang Exp $
*
*/


require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to rebuild the site map cache
*
* Initial Running Vars :
*    root_assetid	-	the ID of the root asset of the tree that will be reindexed
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix
* @subpackage funnelback
*/
class HIPO_Job_Funnelback_Rebuild_Cache extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name		a unique codename the HIPO
	*/
	function HIPO_Job_Rebuild_Cache($code_name='')
	{
		$this->uses_trans = FALSE;
		$this->HIPO_Job($code_name);
		$GLOBALS['SQ_SYSTEM']->lm->includePackageStrings('funnelback');
		$this->_hipo_vars['job_dir'] = SQ_PACKAGES_PATH.'/funnelback/hipo_jobs';

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
		$o->openField(translate('Funnelback Rebuild Cache'));


		echo '<span class="sq-inline-fields-wrapper">';
		if ($write_access) {
			text_box($class.'[SQ_HIPO_REBUILD_FUNNELBACK_CACHE]', SQ_HIPO_REBUILD_FUNNELBACK_CACHE, 1);
		} else {
			echo SQ_HIPO_REBUILD_FUNNELBACK_CACHE;
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
				'SQ_HIPO_REBUILD_FUNNELBACK_CACHE'	=> Array('editable'	=> 1, 'default'	=> 5),
			   );

	}//end getConfigVars()


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
		return 0;

	}//end getThresholdPercentageRequired()


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* The Regenerate Metdata HIPO appends the ID of the schema that was updated so that any other
	* regeneration jobs for this schema do not start while it is running.
	*
	* @return string
	* @access public
	*/
	function getCodeName()
	{
		return parent::getCodeName().'-'.implode(',', $this->_running_vars['root_assetid']);

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('Funnelback Rebuild Cache');


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
					'name'			=> translate('Flushing the existing Funnelback Cache'),

					'function_call'	=> Array(
										'process_function'	=> 'processFlushTable',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
					'allow_cancel'	=> TRUE,
				),
				Array(
					'name'			=> translate('Rebuilding the Funnelback Cache'),

					'function_call'	=> Array(
										'process_function'	=> 'processRebuild',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
					'allow_cancel'	=> TRUE,
				),
			   );

	}//end getInitialStepData()


	/**
	* Performs the duties of this hipo, without showing any output on the frontend.
	*
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		// Put the HIPO in some context ;)
		if (array_get_index('contextid', $this->_running_vars)) {
			$this->_running_vars['contextid'] = $GLOBALS['SQ_SYSTEM']->getContextId();
		}

		// Empty the existing table
		$this->processFlushTable($this->_steps[0], get_class_lower($this));

		while (!empty($this->_running_vars['todo_assetids'])) {
			if (!$this->processRebuild($this->_steps[0], get_class_lower($this))) {
				return FALSE;
			}
		}

		return TRUE;

	}//end freestyle()


	/**
	* Prepares the running vars and gets the children of the asset in question.
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		// to start rebuilding the table, we need to have a root assetid - default to root folder if none is supplied
		if (is_null($this->_running_vars['root_assetid']) || empty($this->_running_vars['root_assetid'])) {
			$this->_running_vars['root_assetid'] = 1;	// the root folder
		}

		if (!is_array($this->_running_vars['root_assetid'])) {
			$this->_running_vars['root_assetid'] = Array($this->_running_vars['root_assetid']);
		}

		// If we have not been told which context to re-index, assume we're in
		// it already (but this is not recommended if not freestyling, since it
		// could be stuck somewhere where the context you want is not set)
		if (array_get_index('contextid', $this->_running_vars)) {
			$this->_running_vars['contextid'] = $GLOBALS['SQ_SYSTEM']->getContextId();
		}

		$this->_running_vars['todo_assetids'] = Array();
		$this->_running_vars['done_assetids'] = Array();
		foreach ($this->_running_vars['root_assetid'] as $assetid) {
			// so we have an assetid to start at, but make sure it is a valid assetid
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			if (is_null($asset)) {
				trigger_localised_error('FNB0022', sprintf(translate('Unable to initialise Funnelback Rebuild Cache HIPO, invalid root asset ID specified (%s)'), $assetid), E_USER_WARNING);
				return FALSE;
			}

			$child_assets = $GLOBALS['SQ_SYSTEM']->am->getChildren($asset->id);
			$child_assets[$asset->id][0]['type_code'] = $asset->type();
			$this->_running_vars['todo_assetids'] += $child_assets;
		}//end foreach

		return parent::prepare();

	}//end prepare()


	/**
	* Flush the table only on the root folder. This will clean any assets not existing anymore
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processFlushTable(&$step_data, $prefix)
	{
		// If this hipo is running against the root folder, then flush the whole table, otherwise let update asset handle the deleting
		if (in_array('1', $this->_running_vars['root_assetid'])) {
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			MatrixDAL::executeSql('DELETE FROM sq_fnb_idx');
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		}//end if
		if ($step_data['percent_done'] == 0) {
			$step_data['percent_done'] = 50;
		} else {
			$step_data['percent_done'] = 100;
			$step_data['complete'] = TRUE;
		}
		return TRUE;

	}//end processFlushTable()


	/**
	* Rebuild the cache table for the sitemap/secmap in funnelback
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processRebuild(&$step_data, $prefix)
	{
		if (!empty($this->_running_vars['todo_assetids'])) {
			$GLOBALS['SQ_SYSTEM']->changeContext(array_get_index($this->_running_vars, 'contextid', 0));

			$am =& $GLOBALS['SQ_SYSTEM']->am;
			$fm = $am->getSystemAsset('funnelback_manager');
			if (is_null($fm)) return FALSE;

			// get next asset to rebuild
			reset($this->_running_vars['todo_assetids']);
			$assetid = key($this->_running_vars['todo_assetids']);
			$asset_type = $this->_running_vars['todo_assetids'][$assetid][0]['type_code'];
			unset($this->_running_vars['todo_assetids'][$assetid]);
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $asset_type);

			if ($asset) {
				$step_data['message'] = sprintf(translate('Refreshing the Cache for Asset "%s"'), $asset->name);

				$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				$fm->updateAsset($asset, Array('update'));
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset, TRUE);
			}

			// add this assetid to the done array
			$this->_running_vars['done_assetids'][] = $assetid;

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($fm, TRUE);
			$GLOBALS['SQ_SYSTEM']->restoreContext();
		}

		if (empty($this->_running_vars['todo_assetids'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
		} else {
			$total = count($this->_running_vars['todo_assetids']) + count($this->_running_vars['done_assetids']);
			$step_data['percent_done'] =(count($this->_running_vars['done_assetids']) / $total) * 100;
			$step_data['complete']     = FALSE;
		}

		return TRUE;

	}//end processRebuild()


}//end class

?>
