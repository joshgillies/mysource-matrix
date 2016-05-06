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
* $Id: hipo_job_bulk_file_import.inc,v 1.14 2013/05/20 06:48:17 cupreti Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

/**
* HIPO for importing files into Matrix assets
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.14 $
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
	public static function paintConfig(&$o, $class, $write_access)
	{
		$o->openField(translate('Bulk File Import Threshold'));

		echo '<span class="sq-inline-fields-wrapper">';
		if ($write_access) {
			text_box($class.'[SQ_HIPO_BULK_FILE_IMPORT_THRESHOLD]', SQ_HIPO_BULK_FILE_IMPORT_THRESHOLD, 4);
		} else {
			echo SQ_HIPO_BULK_FILE_IMPORT_THRESHOLD;
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
		$dir_names = '';
		if (!empty($this->_running_vars['server_file_import'])) {
			$dir_names = implode('_', array_keys($this->_running_vars['server_file_import']));
		}

		$file_names = '';
		if (!empty($this->_running_vars['local_file_import'])) {
			foreach ($this->_running_vars['local_file_import']['files'] as $data) {
				$file_names .= $data['name'];
			}
		}

		return 'HIPO_Job_File_Import-'.md5($dir_names.$file_names);

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('Hipo Job Bulk File Import');


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
										'process_function'	=> 'importServerFiles',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> 'Import Files',
					'function_call'	=> Array(
										'process_function'	=> 'importLocalFiles',
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
		// Nothing to import, return
		if (empty($this->_running_vars['server_file_import']) && empty($this->_running_vars['local_file_import'])) {
			return FALSE;
		}

		$this->_running_vars['server_todo_counts'] = Array();
		$this->_running_vars['local_todo_counts'] = Array();

		// Add server files if there are any
		if (!empty($this->_running_vars['server_file_import'])) {
			foreach ($this->_running_vars['server_file_import'] as $key => $data) {
				foreach ($data['file_list'] as $file_name) {
					$this->_running_vars['server_todo_counts'][] = Array(
																	'file_name'	=> $file_name,
																	'full_path'	=> $key.'/'.$file_name,
																	'root_id'	=> $data['root_id'],
																   );
				}
			}
			$this->_running_vars['server_done_counts'] = Array();
		}

		// Add local files if there are any
		if (!empty($this->_running_vars['local_file_import'])) {
			foreach ($this->_running_vars['local_file_import']['files'] as $data) {
				$file_info = Array(
								'file_name'	=> $data['name'],
								'full_path'	=> $this->_running_vars['local_file_import']['location'].'/'.$data['name'],
								'root_id'	=> $this->_running_vars['local_file_import']['rootid'],
								'type'		=> $data['file_type'],
								'title'		=> $data['title'],
							 );
				if ($file_info['type'] == 'image') {
					$file_info['alt']		= $data['alt'];
					$file_info['caption']	= $data['caption'];
				}
				$this->_running_vars['local_todo_counts'][] = $file_info;
			}
			$this->_running_vars['local_done_counts'] = Array();
		}

		// Global Options
		if (!isset($this->_running_vars['allow_unrestricted'])) {
			$this->_running_vars['allow_unrestricted'] = TRUE;
		}
		if (!isset($this->_running_vars['link_type'])) {
			$this->_running_vars['link_type'] = SQ_LINK_TYPE_1;
		}

		if (!isset($this->_running_vars['allow_virus_scan'])) {
			$this->_running_vars['allow_virus_scan'] = TRUE;
		}

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
		$server_files = count($this->_running_vars['server_todo_counts']);
		$local_files = count($this->_running_vars['local_todo_counts']);
		$total = $server_files + $local_files;
		if ($total == 0) return 0;

		return ($total / SQ_HIPO_BULK_FILE_IMPORT_THRESHOLD) * 100;

	}//end getThresholdPercentageRequired()


	/**
	* Performs the duties of this hipo, without showing any output on the frontend.
	*
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		while (!empty($this->_running_vars['server_todo_counts'])) {
			if (!$this->importServerFiles($this->_steps[1], get_class($this))) {
				return FALSE;
			}
		}

		while (!empty($this->_running_vars['local_todo_counts'])) {
			if (!$this->importLocalFiles($this->_steps[1], get_class($this))) {
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
	function importServerFiles(&$step_data, $prefix)
	{
		if(isset($this->_running_vars['allow_virus_scan']) && !$this->_running_vars['allow_virus_scan'])
			$GLOBALS['SQ_SKIP_VIRUS_SCAN'] = TRUE;

		if (!empty($this->_running_vars['server_todo_counts'])) {

			$todo = array_shift($this->_running_vars['server_todo_counts']);
			$parent_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($todo['root_id']);
			$import_link = Array('asset' => &$parent_asset, 'link_type' => $this->_running_vars['link_type']);
			$file_type = get_file_type($todo['file_name']);
			switch ($file_type) {
				case 'doc' :
				case 'dot' :
				case 'docx':
				case 'docm':
				case 'dotx':
				case 'dotm':
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('word_doc');
					$new_asset_type = 'word_doc';
				break;
				case 'pdf' :
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('pdf_file');
					$new_asset_type = 'pdf_file';
				break;
				case 'xls' :
				case 'xlt' :
				case 'xlsx' :
				case 'xlsm' :
				case 'xltx' :
				case 'xltm' :
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('excel_doc');
					$new_asset_type = 'excel_doc';
				break;
				case 'ppt' :
				case 'pot' :
				case 'pps' :
				case 'pptx' :
				case 'potx' :
				case 'ppsx' :
				case 'pptm' :
				case 'potm' :
				case 'ppsm' :
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('powerpoint_doc');
					$new_asset_type = 'powerpoint_doc';
				break;
				case 'rtf' :
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('rtf_file');
					$new_asset_type = 'rtf_file';
				break;
				case 'txt' :
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('text_file');
					$new_asset_type = 'text_file';
				break;
				case 'js' :
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('js_file');
					$new_asset_type = 'js_file';
				break;
				case 'css' :
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('css_file');
					$new_asset_type = 'css_file';
				break;
				case 'gif' :
				case 'jpg' :
				case 'jpeg' :
				case 'png' :
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('image');
					$new_asset_type = 'image';
				break;
				case 'mp3' :
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('mp3_file');
					$new_asset_type = 'mp3_file';
				break;
				case 'mov' :
				case 'avi' :
				case 'wmv' :
				case 'asf' :
				case 'flv' :
				case 'mp4' :
				case 'm4v' :
				case 'mpg' :
				case 'mpeg':
				case 'ogv':
				case 'ogg':
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('video_file');
					$new_asset_type = 'video_file';
				break;
				default :
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('file');
					$new_asset_type = 'file';
				break;
			}

			// create an asset under the new parent of the correct type
			$temp_info = Array('name' => $todo['file_name'], 'tmp_name' => $todo['full_path'], 'non_uploaded_file' => TRUE);
			$new_file = new $new_asset_type();
			$new_file->_tmp['uploading_file'] = TRUE;
			$new_file->setAttrValue('name', $todo['file_name']);
			$new_file->setAttrValue('allow_unrestricted', $this->_running_vars['allow_unrestricted']);

			if (!$new_file->create($import_link, $temp_info)) {
				$this->_addError(sprintf(translate('File Import failed to create "%s" asset under Assetid : %d'), $todo['file_name'], $parent->id), TRUE);

			}
			$this->_running_vars['server_done_counts'][] = $todo;

		}//end if

		if (empty($this->_running_vars['server_todo_counts'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
			unset($this->_running_vars['server_done_counts']);
		} else {
			$total = count($this->_running_vars['server_todo_counts']) + count($this->_running_vars['server_done_counts']);
			$step_data['percent_done'] = (count($this->_running_vars['server_done_counts']) / $total) * 100;
			$step_data['complete']     = FALSE;
		}

		return TRUE;

	}//end importServerFiles()


	/**
	* Edit permissions for a single asset in the to-do list
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function importLocalFiles(&$step_data, $prefix)
	{
		if(isset($this->_running_vars['allow_virus_scan']) && !$this->_running_vars['allow_virus_scan'])
			$GLOBALS['SQ_SKIP_VIRUS_SCAN'] = TRUE;

		if (!empty($this->_running_vars['local_todo_counts'])) {

			$todo = array_shift($this->_running_vars['local_todo_counts']);
			$parent_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($todo['root_id']);
			$import_link = Array('asset' => &$parent_asset, 'link_type' => $this->_running_vars['link_type']);
			switch ($todo['type']) {
				case 'word' :
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('word_doc');
					$new_asset_type = 'word_doc';
				break;
				case 'pdf' :
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('pdf_file');
					$new_asset_type = 'pdf_file';
				break;
				case 'powerpoint' :
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('powerpoint_doc');
					$new_asset_type = 'powerpoint_doc';
				break;
				case 'excel' :
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('excel_doc');
					$new_asset_type = 'excel_doc';
				break;
				case 'rtf' :
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('rtf_file');
					$new_asset_type = 'rtf_file';
				break;
				case 'text' :
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('text_file');
					$new_asset_type = 'text_file';
				break;
				case 'css' :
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('css_file');
					$new_asset_type = 'css_file';
				break;
				case 'js' :
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('js_file');
					$new_asset_type = 'js_file';
				break;
				case 'image' :
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('image');
					$new_asset_type = 'image';
				break;
				case 'mp3' :
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('mp3_file');
					$new_asset_type = 'mp3_file';
				break;
				case 'video' :
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('video_file');
					$new_asset_type = 'video_file';
				break;
				default :
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('file');
					$new_asset_type = 'file';
				break;
			}

			// create an asset under the new parent of the correct type
			$temp_info = Array('name' => $todo['file_name'], 'tmp_name' => $todo['full_path'], 'non_uploaded_file' => TRUE);
			$new_file = new $new_asset_type();
			$new_file->_tmp['uploading_file'] = TRUE;
			$new_file->setAttrValue('name', $todo['file_name']);
			$new_file->setAttrValue('title', $todo['title']);
			$new_file->setAttrValue('allow_unrestricted', $this->_running_vars['allow_unrestricted']);

			// Extra attributes for particular asset type
			switch ($todo['type']) {
				case 'image' :
					$new_file->setAttrValue('alt', $todo['alt']);
					$new_file->setAttrValue('caption', $todo['caption']);
				break;
			}

			if (!$new_file->create($import_link, $temp_info)) {
				$this->_addError(sprintf(translate('File Import failed to create "%s" asset under Assetid : %d'), $todo['file_name'], $parent->id), TRUE);

			}
			$this->_running_vars['local_done_counts'][] = $todo;

		}//end if

		if (empty($this->_running_vars['local_todo_counts'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
			unset($this->_running_vars['local_todo_counts']);
		} else {
			$total = count($this->_running_vars['local_todo_counts']) + count($this->_running_vars['local_done_counts']);
			$step_data['percent_done'] = (count($this->_running_vars['local_done_counts']) / $total) * 100;
			$step_data['complete']     = FALSE;
		}

		return TRUE;

	}//end importLocalFiles()


}//end class

?>
