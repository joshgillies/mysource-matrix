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
* $Id: hipo_job_regenerate_metadata.inc,v 1.39 2012/08/30 01:09:19 ewang Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to regenerate pre-processed metadata files
* for all assets that use the supplied schema
*
* Initial Running Vars :
*    schemaid	-	the ID of the schema that has been updated
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.39 $
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Regenerate_Metadata extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name		a unique codename the HIPO
	*/
	function HIPO_Job_Regenerate_Metadata($code_name='')
	{
		$this->uses_trans = FALSE;
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
		$o->openField(translate('Metadata Regeneration Threshold'));


		echo '<span class="sq-inline-fields-wrapper">';
		if ($write_access) {
			text_box($class.'[SQ_HIPO_REGENERATE_METADATA_THRESHOLD]', SQ_HIPO_REGENERATE_METADATA_THRESHOLD, 1);
		} else {
			echo SQ_HIPO_REGENERATE_METADATA_THRESHOLD;
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
		return Array('SQ_HIPO_REGENERATE_METADATA_THRESHOLD' => Array('editable' => 1, 'default' => 20));

	}//end getConfigVars()


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
		return parent::getCodeName().'-'.md5(implode('-',$this->_running_vars['schemaids']));

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('Regenerate Metadata');


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
					'name'			=> translate('Generating Metadata Files'),

					'function_call'	=> Array(
										'process_function'	=> 'processMetadata',
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
	* Returns a value that represents the total percentage that this hipo would run in freestyle mode in its current state.
	*
	* @return float
	* @access public
	*/
	function getThresholdPercentageRequired()
	{
		if (SQ_HIPO_REGENERATE_METADATA_THRESHOLD == 0) {
			return 0;
		}
		if (!isset($this->_running_vars['todo_assetids'])) {
			return 0;
		}
		return ((count($this->_running_vars['todo_assetids']) / SQ_HIPO_REGENERATE_METADATA_THRESHOLD) * 100);

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
			if (!$this->processMetadata($this->_steps[0], get_class($this))) {
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
		// to start regenerating metadata, we need to have a schema that has been updated
		if (empty($this->_running_vars['schemaids'])) {
			trigger_localised_error('HIPO0003', translate('Unable to initialise Regenerate Metadata HIPO, no schemaID(s) specified'), E_USER_WARNING)
			return FALSE;
		}

		// If no context IDs are provided, it will do all of them
		if (empty($this->_running_vars['contextids']) === TRUE) {
			$this->_running_vars['contextids'] = array_keys($GLOBALS['SQ_SYSTEM']->getAllContexts());
		}

		// If a scalar is given instead of an array, assume a single context
		// and convert it into an array
		if (is_array($this->_running_vars['contextids']) === FALSE) {
			$this->_running_vars['contextids'] = Array($this->_running_vars['contextids']);
		}

		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();

		$this->_running_vars['todo_assetids'] = Array();
		$this->_running_vars['done_assetids'] = Array();

		foreach ($this->_running_vars['schemaids'] as $schemaid) {
			$this->_running_vars['todo_assetids'] = array_merge($this->_running_vars['todo_assetids'], $mm->getSchemaAssets($schemaid, TRUE));
		}

		// remove duplicates
		$this->_running_vars['todo_assetids'] = array_values(array_unique($this->_running_vars['todo_assetids']));

		// if nothing to do, bail out
		if (empty($this->_running_vars['todo_assetids'])) {
			return FALSE;
		}

		return parent::prepare();

	}//end prepare()


	/**
	* Regenerate Metadata for a single asset in the todo list
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processMetadata(&$step_data, $prefix)
	{
		if (!isset($this->_running_vars['process_metadata'])) {
			$this->_running_vars['process_metadata'] = $this->_running_vars['todo_assetids'];
		}

		if (!empty($this->_running_vars['process_metadata'])) {
			$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
			$assetid = array_shift($this->_running_vars['process_metadata']);
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

			if (!is_null($asset)) {
				// try to acquire lock
				if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'metadata')) {
					$step_data['message'] = sprintf(translate('Skipping asset #%s'), $assetid);

					trigger_localised_error('HIPO0077', sprintf(translate('Unable to regenerate metadata on asset "%1$s" (Id: %2$s) as the lock has expired; aborting job - contact your system administrator to increase the lock length and try again'), $asset->name, $assetid), E_USER_WARNING)
				}

				if (!$asset->writeAccess('metadata')) {
					// we don't have write access to this asset for another
					// reason (eg. status = Archived), skip the asset
					$step_data['message'] = sprintf(translate('Skipping asset #%s'), $assetid);

					trigger_localised_error('HIPO0078', sprintf(translate('Skipped regenerating metadata on asset "%1$s" (Id: %2$s) as the current user does not have write access to the asset'), $asset->name, $assetid), E_USER_WARNING)
				} else {
					foreach ($this->_running_vars['contextids'] as $contextid) {
						if (!$mm->regenerateMetadata($asset->id, $contextid)) return FALSE;
					}
					$step_data['message'] = sprintf(translate('Generating metadata file for "%s"'), htmlentities($asset->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET));

					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
					unset($asset);
				}

				// release lock
				$GLOBALS['SQ_SYSTEM']->am->releaseLock($assetid, 'metadata');

			} else {
				$step_data['message'] = sprintf(translate('Skipping asset #%s'), $assetid);

				$this->_addError(sprintf(translate('Could not regenerate metadata file for asset #%s, asset does not exist'), $assetid));

			}

			// add this assetid to the done array so we dont do it again
			$this->_running_vars['done_assetids'][] = $assetid;
		}//end if

		if (empty($this->_running_vars['process_metadata'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
			unset($this->_running_vars['done_assetids']);
			$this->_running_vars['todo_assetids'] = Array();
		} else {
			$total = count($this->_running_vars['process_metadata']) + count($this->_running_vars['done_assetids']);
			$step_data['percent_done'] =(count($this->_running_vars['done_assetids']) / $total) * 100;
			$step_data['complete']     = FALSE;
		}

		return TRUE;

	}//end processMetadata()


}//end class

?>
