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
* $Id: hipo_job_regenerate_metadata.inc,v 1.24.2.3 2006/06/28 23:42:01 rong Exp $
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
* @version $Revision: 1.24.2.3 $
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
		$o->openField(translate('metadata_regeneration_threshold'));

		if ($write_access) {
			text_box($class.'[SQ_HIPO_REGENERATE_METADATA_THRESHOLD]', SQ_HIPO_REGENERATE_METADATA_THRESHOLD, 5);
		} else {
			echo SQ_HIPO_REGENERATE_METADATA_THRESHOLD;
		}
		echo ' '.translate('assets');

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
		return Array('SQ_HIPO_REGENERATE_METADATA_THRESHOLD' => Array('editable' => 1, 'default' => 1));

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
		return translate('hipo_name_regenerate_metadata');

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
					'name'			=> translate('hipo_generating_metadata_files'),
					'function_call'	=> Array(
										'process_function'	=> 'processMetadata',
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
		// to start regenerating metadata, we need to have a schema that has been updated
		if (empty($this->_running_vars['schemaids'])) {
			trigger_localised_error('HIPO0003', E_USER_WARNING);
			return false;
		}

		$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();

		$this->_running_vars['todo_assetids'] = Array();
		$this->_running_vars['done_assetids'] = Array();

		foreach ($this->_running_vars['schemaids'] as $schemaid) {
			$this->_running_vars['todo_assetids'] += $mm->getSchemaAssets($schemaid, true);
		}

		// if nothing to do, bail out
		if (empty($this->_running_vars['todo_assetids'])) {
			return FALSE;
		}

		return parent::prepare();

	}//end prepare()


	/**
	* Regenerate Metatdata for a single asset in the todo list
	*
	* @param array()	&$step_data	a reference to the array of information about the current step
	* @param string		$prefix		prefix for form vars
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
			$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
			$assetid = array_shift($this->_running_vars['process_metadata']);
			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

			if (!is_null($asset)) {
				// try to acquire lock
				if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'metadata')) {
					$step_data['message'] = translate('hipo_skipping_asset', $assetid);
					trigger_localised_error('HIPO0077', E_USER_WARNING, $asset->name, $assetid);
				}

				if (!$asset->writeAccess('metadata')) {
					// we don't have write access to this asset for another
					// reason (eg. status = Archived), skip the asset
					$step_data['message'] = translate('hipo_skipping_asset', $assetid);
					trigger_localised_error('HIPO0078', E_USER_WARNING, $asset->name, $assetid);
				} else {
					if (!$mm->regenerateMetadata($asset->id)) return FALSE;
					$step_data['message'] = translate('hipo_generate_metadata', $asset->name);
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
					unset($asset);
				}

				// release lock
				$GLOBALS['SQ_SYSTEM']->am->releaseLock($assetid, 'metadata');

			} else {
				$step_data['message'] = translate('hipo_skipping_asset', $assetid);
				$this->_addError(translate('hipo_cannot_regenerate_metadata', $assetid));
			}

			// add this assetid to the done array so we dont do it again
			$this->_running_vars['done_assetids'][] = $assetid;
		}

		if (empty($this->_running_vars['process_metadata'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = true;
			unset($this->_running_vars['done_assetids']);
			$this->_running_vars['todo_assetids'] = Array();
		} else {
			$total = count($this->_running_vars['process_metadata']) + count($this->_running_vars['done_assetids']);
			$step_data['percent_done'] =(count($this->_running_vars['done_assetids']) / $total) * 100;
			$step_data['complete']     = false;
		}

		return true;

	}//end processMetadata()


}//end class

?>
