<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: hipo_job_bulk_file_import.inc,v 1.1 2006/10/19 02:23:39 skim Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

/**
* HIPO for importing files into Matrix assets
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Bulk_File_Import extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name		a unique codename the HIPO
	*/
	function HIPO_Job_Bulk_File_Import($code_name='')
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
	function paintConfig(&$o, $class)
	{
		$write_access = $this->writeAccess();

		// file import threshhold HIPO config entry
		$o->openField(translate('file_import_tools_hipo_threshold'));

		if ($write_access) {
			text_box($class.'[SQ_HIPO_BULK_FILE_IMPORT_THRESHOLD]', SQ_HIPO_BULK_FILE_IMPORT_THRESHOLD, 5);
			echo ' '.translate('assets');
			$o->note(translate('hipo_value_divided_by_permission'));
		} else {
			echo SQ_HIPO_BULK_FILE_IMPORT_THRESHOLD;
			echo ' '.translate('assets');
		}

		$o->closeField();

	}//end paintConfig()


	/**
	* Sets some info about thresholds that this hipo requires.
	*
	* @return array
	* @access public
	*/
	function getConfigVars()
	{
		return Array(
				'SQ_HIPO_BULK_FILE_IMPORT_THRESHOLD'	=> Array('editable'	=> 1, 'default'	=> 5),
			   );

	}//end getConfigVars()


	/**
	* Returns a unique id for this job
	*
	* @return string
	* @access public
	*/
	function getCodeName()
	{
		$dirs_array = array_keys($this->_running_vars['dirs_list']);
		$suffix = str_replace(' ', '_', implode('_', $dirs_array));
		return 'HIPO_Job_File_Import-'.$suffix;

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('file_import_tools_hipo_threshold');

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
					'name'			=> 'Import Files',
					'function_call'	=> Array(
										'process_function'	=> 'importFiles',
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
	* Prepares the running vars and gets the children of the assets in question.
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		if (empty($this->_running_vars['total_count'])) {
			return FALSE;
		}

		if (empty($this->_running_vars['dirs_list'])) {
			return FALSE;
		}

		foreach ($this->_running_vars['dirs_list'] as $key => $data) {
			foreach ($data['file_list'] as $file_name) {
				$this->_running_vars['todo_counts'][] = Array(
															'file_name'	=> $file_name,
															'full_path'	=> $key.'/'.$file_name,
															'root_id'	=> $data['root_id'],
														);
			}
			if (!isset($this->_running_vars['allow_unrestricted'])) {
				$this->_running_vars['allow_unrestricted'] = TRUE;
			}
			if (!isset($this->_running_vars['link_type'])) {
				$this->_running_vars['link_type'] = SQ_LINK_TYPE_1;
			}
		}

		$this->_running_vars['done_counts'] = Array();
		return parent::prepare();

	}//end prepare()


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
		if (SQ_HIPO_BULK_FILE_IMPORT_THRESHOLD == 0) return 0;
		if (!isset($this->_running_vars['todo_counts'])) {
			return 0;
		}
		return (count($this->_running_vars['todo_counts']) / SQ_HIPO_BULK_FILE_IMPORT_THRESHOLD) * 100;

	}//end getThresholdPercentageRequired()


	/**
	* Performs the duties of this hipo, without showing any output on the frontend.
	*
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		while (!empty($this->_running_vars['todo_counts'])) {
			if (!$this->importFiles($this->_steps[1], get_class($this))) {
				return FALSE;
			}
		}
		return TRUE;

	}//end freestyle()


	/**
	* Edit permissions for a single asset in the to-do list
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function importFiles(&$step_data, $prefix)
	{
		if (!empty($this->_running_vars['todo_counts'])) {

			$todo = array_shift($this->_running_vars['todo_counts']);
			$parent_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($todo['root_id']);
			$import_link = Array('asset' => &$parent_asset, 'link_type' => $this->_running_vars['link_type']);
			switch (get_file_type($todo['file_name'])) {
				case 'doc' :
				case 'dot' :
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('word_doc');
					$new_asset_type = 'word_doc';
				break;
				case 'pdf' :
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('pdf_file');
					$new_asset_type = 'pdf_file';
				break;
				case 'gif' :
				case 'jpg' :
				case 'jpeg' :
				case 'png' :
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('image');
					$new_asset_type = 'image';
				break;
				default :
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('file');
					$new_asset_type = 'file';
				break;
			}

			// create an asset under the new parent of the correct type
			$temp_info = Array('name' => $todo['file_name'], 'tmp_name' => $todo['full_path'], 'non_uploaded_file' => TRUE);
			$new_file =& new $new_asset_type();
			$new_file->_tmp['uploading_file'] = TRUE;
			$new_file->setAttrValue('name', $todo['file_name']);
			$new_file->setAttrValue('allow_unrestricted', $this->_running_vars['allow_unrestricted']);

			if (!$new_file->create($import_link, $temp_info)) {
				$this->_addError(translate('file_import_tools_hipo_can_not_create_file', $todo['file_name'], $parent->id), TRUE);
			}
			$this->_running_vars['done_counts'][] = $todo;

		}//end if

		if (empty($this->_running_vars['todo_counts'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
			unset($this->_running_vars['done_counts']);
		} else {
			$total = count($this->_running_vars['todo_counts']) + count($this->_running_vars['done_counts']);
			$step_data['percent_done'] = (count($this->_running_vars['done_counts']) / $total) * 100;
			$step_data['complete']     = FALSE;
		}

		return TRUE;

	}//end importFiles()


}//end class

?>
