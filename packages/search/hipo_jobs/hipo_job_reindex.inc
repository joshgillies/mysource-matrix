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
* $Id: hipo_job_reindex.inc,v 1.38 2013/08/28 00:50:46 cupreti Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to reindex all the assets in the asset tree
* under the supplied asset
*
* Initial Running Vars :
*    root_assetid	-	the ID of the root asset of the tree that will be reindexed
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.38 $
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_ReIndex extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name		a unique codename the HIPO
	*/
	function HIPO_Job_ReIndex($code_name='')
	{
		$this->uses_trans = FALSE;
		$this->HIPO_Job($code_name);
		$GLOBALS['SQ_SYSTEM']->lm->includePackageStrings('search');
		$this->_hipo_vars['job_dir'] = SQ_PACKAGES_PATH.'/search/hipo_jobs';

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
		$o->openField(translate('Reindex Threshold'));


		echo '<span class="sq-inline-fields-wrapper">';
		if ($write_access) {
			text_box($class.'[SQ_HIPO_REINDEX_THRESHOLD]', SQ_HIPO_REINDEX_THRESHOLD, 1);
		} else {
			echo SQ_HIPO_REINDEX_THRESHOLD;
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
				'SQ_HIPO_REINDEX_THRESHOLD'	=> Array('editable'	=> 1, 'default'	=> 20),
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
		if (SQ_HIPO_REINDEX_THRESHOLD == 0) return 0;
		if (!isset($this->_running_vars['todo_assetids'])) {
			return 0;
		}

		return ((count($this->_running_vars['todo_assetids']) / SQ_HIPO_REINDEX_THRESHOLD) * 100);

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
		return parent::getCodeName().'-'.md5(implode(',', $this->_running_vars['root_assetid']));

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('Re-index Assets');


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
					'name'			=> translate('Reindexing Assets'),

					'function_call'	=> Array(
										'process_function'	=> 'processReIndex',
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

		while (!empty($this->_running_vars['todo_assetids'])) {
			if (!$this->processReIndex($this->_steps[0], get_class_lower($this))) {
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
		// to start regenerating metadata, we need to have a root assetid - default to root folder if none is supplied
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

		$todo_assetids = Array();
		foreach ($this->_running_vars['root_assetid'] as $assetid) {
			// so we have an assetid to start at, but make sure it is a valid assetid
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			if (is_null($asset)) {
				trigger_localised_error('SCH0022', sprintf(translate('Unable to initialise Search Reindexing HIPO, invalid root asset ID specified (%s)'), $assetid), E_USER_WARNING)
				return FALSE;
			}

			if (!empty($this->_running_vars['root_only']) && $this->_running_vars['root_only']) {
				$child_assets = $GLOBALS['SQ_SYSTEM']->am->getDependantChildren($asset->id);
			} else {
				$child_assets = $GLOBALS['SQ_SYSTEM']->am->getChildren($asset->id);
			}
			$todo_assetids += $child_assets;
			$todo_assetids[$asset->id][0]['type_code'] = $asset->type();

		}//end foreach

		$this->_running_vars['todo_assetids'] = $todo_assetids;
		$this->_running_vars['done_assetids'] = Array();

		return parent::prepare();

	}//end prepare()


	/**
	* Re-index asset keydata for a single asset in the todo list
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processReIndex(&$step_data, $prefix)
	{
		if (!empty($this->_running_vars['todo_assetids'])) {
			$GLOBALS['SQ_SYSTEM']->changeContext($this->_running_vars['contextid']);

			$am =& $GLOBALS['SQ_SYSTEM']->am;
			$sm = $am->getSystemAsset('search_manager');
			if (is_null($sm)) return FALSE;

			// get next asset to reindex
			reset($this->_running_vars['todo_assetids']);
			$assetid = key($this->_running_vars['todo_assetids']);
			$asset_type = $this->_running_vars['todo_assetids'][$assetid][0]['type_code'];
			unset($this->_running_vars['todo_assetids'][$assetid]);
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $asset_type);

			if ($asset) {
				$step_data['message'] = sprintf(translate('Reindexing %s'), $asset->name);

				$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				$sm->reindexAsset($asset, Array('all'));
				$sm->reindexAttributes($asset, Array('all'), TRUE);
				$sm->reindexContents($asset, Array('all'), TRUE);
				$sm->reindexMetadata($asset->id, Array('all'));
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			}

			// add this assetid to the done array
			$this->_running_vars['done_assetids'][] = $assetid;

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

	}//end processReIndex()


}//end class

?>
