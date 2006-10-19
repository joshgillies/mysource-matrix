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
* $Id: tool_bulk_file_import.inc,v 1.1 2006/10/19 02:23:48 skim Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/assets/system/tool/tool.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_DATA_PATH.'/private/conf/tools.inc';

/**
* Tool_Bulk_File_Import
*
* Purpose
*
*    Tool for importing files into Matrix
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage search
*/

class Tool_Bulk_File_Import extends Tool
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Tool_Bulk_File_Import($assetid=0)
	{
		$this->Tool($assetid);

	}//end constructor


	/**
	* Paints the tool interface.
	*
	* @param object	&$o			reference to the backend outputter of the object painting the tool
	* @param string	$type_code	the type code of the tool being painted
	*
	* @return string
	* @access public
	*/
	function paintTool(&$o, $type_code)
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$itm =& $am->getSystemAsset('import_tools_manager');

		$import_dir = $itm->attr('file_import_dir');
		if (empty($import_dir)) {

			$o->openSection(translate('file_import_tools_warning'));
				$o->openField(translate('file_import_tools_not_ready'));
					echo translate('file_import_tools_not_ready_msg');
				$o->closeField();
			$o->closeSection();

		} else {

			// Some variable initialisation
			$tool_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);
			$o->openSection($tool_info['name']);

			$o->closeSection();

			$o->openSection(translate('import_tools_available_files'));

				$compatible_types = $itm->attr('valid_file_types');

				// File Import Directory
				$o->openField(translate('import_tools_import_dir'));
					echo hide_system_root($import_dir);
				$o->closeField();

				$o->openField(translate('import_tools_files'));
					// This will get us a list of all the files in the import directory
					$dirs_array = list_dirs($import_dir, TRUE);
					?>
					<table class="sq-backend-table" style="width: 600px;">
						<tr>
							<th class="sq-backend-table-header" style="width: 130px;"><?php echo 'Directory'; ?></th>
							<th class="sq-backend-table-header" style="width: 80px; text-align: center;"><?php echo 'Select for Import'; ?></th>
							<th class="sq-backend-table-header" style="width: 390px; text-align: left;"><?php echo 'Import Under'; ?></th>
						</tr>
						<?php
							$files = Tool_Bulk_File_Import::_getFileList($import_dir);
							if (!empty($files)) {
								$num_of_files = count($files);
								?>
								<tr>
									<td class="sq-backend-table-cell" style="border-bottom: 0px;"><?php echo $num_of_files; ?> file<?php echo ($num_of_files > 1) ? 's' : ''; ?>.</td>
									<td class="sq-backend-table-cell" style="border-bottom: 0px; text-align: center;"><?php check_box('chunk_file_import', '1', FALSE); ?></td>
									<td class="sq-backend-table-cell" style="border-bottom: 0px; text-align: center;"><?php asset_finder('chunk_file_import_root_assetid', ''); ?></td>
								</tr>
								<tr>
									<td class="sq-backend-table-cell" style="border-top: 0px;"colspan="3"><?php echo translate('file_import_tools_file_chunk', '<b>'.$import_dir.'</b>') ?></td>
								</tr>
								<?php
							}
						?>
						<?php
							foreach ($dirs_array as $dir_name) {
								$files = Tool_Bulk_File_Import::_getFileList($dir_name);
								$num_of_files = count($files);
								$stripped_dname = substr($dir_name, strrpos($dir_name, '/') + 1);
								?>
								<tr>
								<td class="sq-backend-table-cell"><?php echo $stripped_dname; ?>&nbsp;(<?php echo $num_of_files; ?> file<?php echo ($num_of_files > 1) ? 's' : ''; ?>)</td>
									<td class="sq-backend-table-cell" style="text-align: center;"><?php check_box('file_import_dirname_'.str_replace(' ', '_', $stripped_dname), $dir_name, FALSE); ?></td>
									<td class="sq-backend-table-cell" style="text-align: center;"><?php asset_finder(str_replace(' ', '_', $stripped_dname).'_root_assetid', ''); ?></td>
								</tr>
								<?php
							}
						?>
					</table>
					<?php
					$o->note(translate('file_import_tools_select_file'));
				$o->closeField();

				$o->openField('Allow Unrestricted');
					combo_box($type_code.'_allow_unrestricted', Array(1 => 'Yes', 0 => 'No'));
					echo '<br />'.translate('file_import_tools_allow_unrestricted_note');
				$o->closeField();

				$o->openField('Link Type');
					combo_box($type_code.'_link_type', Array(SQ_LINK_TYPE_1 => 'TYPE_1', SQ_LINK_TYPE_2 => 'TYPE_2'));
					echo '<br />'.translate('file_import_tools_file_link_type_msg');
				$o->closeField();

		}//end else

	}//end paintTool()


	/**
	* Processes the user input and performs the function of the tool.
	*
	* @param object	&$o			reference to the backend outputter of the object painting the tool
	* @param string	$type_code	the type code of the tool being painted
	*
	* @return boolean
	* @access public
	*/
	function processTool(&$o, $type_code)
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$vars = Array();
		$itm =& $am->getSystemAsset('import_tools_manager');
		$import_dir = $itm->attr('file_import_dir');
		$dirs_array = list_dirs($import_dir, TRUE);
		foreach ($dirs_array as $dir_name) {
			$files = Tool_Bulk_File_Import::_getFileList($dir_name);
			$num_of_files = count($files);
			$stripped_dname = substr($dir_name, strrpos($dir_name, '/') + 1);

			$tmp = array_get_index($_POST, 'file_import_dirname_'.str_replace(' ', '_', $stripped_dname), '');
			if (!empty($tmp)) {

				$tmp = array_get_index($_POST, str_replace(' ', '_', $stripped_dname).'_root_assetid', '');
				if (!empty($tmp['assetid'])) {
					if (!isset($vars['total_count'])) {
						$vars['total_count'] = $num_of_files;
					} else {
						$vars['total_count'] = $vars['total_count'] + $num_of_files;
					}
					$vars['dirs_list'][$dir_name]['file_list'] = $files;
					$vars['dirs_list'][$dir_name]['num_of_files'] = $num_of_files;
					$vars['dirs_list'][$dir_name]['root_id'] = $tmp['assetid'];
				}
			}
		}//end foreach

		$tmp = array_get_index($_POST, 'chunk_file_import', '');
		if (!empty($tmp)) {

			$tmp = array_get_index($_POST, 'chunk_file_import_root_assetid', '');
			if (!empty($tmp['assetid'])) {
				$files = Tool_Bulk_File_Import::_getFileList($import_dir);
				if (!isset($vars['total_count'])) {
					$vars['total_count'] = count($files);
				} else {
					$vars['total_count'] = $vars['total_count'] + count($files);
				}
				$vars['dirs_list'][$import_dir]['file_list'] = $files;
				$vars['dirs_list'][$import_dir]['num_of_files'] = count($files);
				$vars['dirs_list'][$import_dir]['root_id'] = $tmp['assetid'];
			}
		}

		if (!empty($vars['dirs_list'])) {

			$vars['allow_unrestricted'] = array_get_index($_POST, $type_code.'_allow_unrestricted', TRUE);
			$vars['link_type'] = array_get_index($_POST, $type_code.'_link_type', SQ_LINK_TYPE_1);

			// We have all the required data, so queue up our HIPO and go to town
			$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$hh->queueHipo('hipo_job_bulk_file_import', $vars, '', SQ_PACKAGES_PATH.'/import_tools/hipo_jobs/');
			$url = $hh->runQueuedJobs();
			if (!empty($url)) $o->setRedirect($url);

			return TRUE;
		}

		return FALSE;

	}//end processTool()


	/**
	* Returns the list of files available for import in the directory
	*
	* @param string	$dir	the path to the directory
	*
	* @return array
	* @access public
	*/
	function _getFileList($dir)
	{
		$files = list_files($dir);
		foreach ($files as $key => $file_name) {
			if (substr($file_name, 0, 1) == '.') {
				unset($files[$key]);
			}
		}

		return $files;

	}//end _getFileList()


}//end class


?>
