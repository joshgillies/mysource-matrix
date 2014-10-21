<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: tool_structured_file_import.inc,v 1.18 2012/08/30 00:58:50 ewang Exp $
*
*/


require_once SQ_SYSTEM_ROOT.'/core/assets/system/tool/tool.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_DATA_PATH.'/private/conf/tools.inc';
require_once SQ_DATA_PATH.'/private/conf/import_tools_manager.inc';

/**
* Tool_Structured_File_Import
*
* Purpose
*
*    Tool for reindexing assets so that the Search Manager can use the index for
*    searching.
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.18 $
* @package MySource_Matrix_Packages
* @subpackage search
*/

class Tool_Structured_File_Import extends Tool
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Tool_Structured_File_Import($assetid=0)
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
	public static function paintTool(&$o, $type_code)
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$itm = $am->getSystemAsset('import_tools_manager');

		$import_dir = STRUCTURED_FILE_IMPORT_DIR;
		if(!empty($import_dir) && strpos($import_dir, '/') !== 0)     $import_dir = SQ_SYSTEM_ROOT.'/'.$import_dir;
		if (empty($import_dir)) {

			$o->openSection('Warning');
				$o->openField(translate('Import Tool is not ready to use'));

					echo translate('Please set the import directory using config file data/private/conf/import_tools_manager.inc.');

				$o->closeField();
			$o->closeSection();

		} else {

			// Some variable initialisation
			$tool_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);
			$o->openSection($tool_info['name']);

				$compatible_types = $itm->attr('valid_file_types');
				$o->openField(translate('Import Directory'));

					echo $import_dir;
				$o->closeField();
				$o->openField(translate('Files'));

					// This will get us a list of all the files in the import directory
					$files_array = list_files($import_dir, TRUE);
					// Filter these to include only compatible files.
					foreach ($files_array as $id => $file_name) {
						$file_type = get_file_type($file_name);
						if (!in_array($file_type, $compatible_types)) {
							unset($files_array[$id]);
						}
					}
					?>
					<script type="text/javascript">
						<?php
						$tmp = 'var file_list =[';
						for (reset($files_array); $key = key($files_array); next($files_array)) {
							$strrpos_files_array = strrpos($files_array[$key], '/') + 1;
							$stripped_fname = substr($files_array[$key], $strrpos_files_array);
							$tmp .= '"'.str_replace(Array('.',' '), '_', $stripped_fname).'",';
						}
						echo substr($tmp, 0, -1).'];';
						?>
						function uncheck_import_file() {
							var form = document.getElementById("main_form");
							for (counter = 0; counter < form.structured_file_import_filename.length; counter++) {
								if (form.structured_file_import_filename[counter].value == '') {
									form.structured_file_import_filename[counter].checked = true;
								} else {
									form.structured_file_import_filename[counter].checked = false;
								}
							}
						}
						function can_select() {
							for (counter = 0; counter < file_list.length; counter++ ) {
								var delete_chkbox = document.getElementById("structured_file_remove_" + file_list[counter]);
								if (delete_chkbox.checked == true) {
									uncheck_import_file();
									alert("<?php echo translate("import_tools_javascript_warning"); ?>");
									break;
								}
							}
						}
					</script>
					<table class="sq-backend-table" style="width: 440px;">
						<tr>
							<th class="sq-backend-table-header" style="width: 240px;"><?php echo translate('Select a file to import into the system'); ?></th>

							<th class="sq-backend-table-header" style="width: 100px;"><?php echo translate('Select For Import'); ?></th>

							<th class="sq-backend-table-header" style="width: 100px; text-align: center;"><?php echo translate('Delete?'); ?></th>

						</tr>
						<tr>
							<td class="sq-backend-table-cell"><?php echo translate('None'); ?></td>

							<td class="sq-backend-table-cell" style="text-align: center;"><?php radio_button('structured_file_import_filename', '', TRUE); ?></td>
							<td class="sq-backend-table-cell" style="text-align: center;"></td>
						</tr>
						<?php
							foreach ($files_array as $file_name) {
								$strrpos_file_name = strrpos($file_name, '/') + 1;
								$stripped_fname = substr($file_name, $strrpos_file_name);
								$replaced_fname = str_replace(Array('.',' '), '_', $stripped_fname);
								?>
								<tr>
									<td class="sq-backend-table-cell"><?php echo $stripped_fname; ?></td>
									<td class="sq-backend-table-cell" style="text-align: center;"><?php radio_button('structured_file_import_filename', $file_name, FALSE, 'can_select();'); ?></td>
									<td class="sq-backend-table-cell" style="text-align: center;"><?php check_box('structured_file_remove_'.$replaced_fname, '1', FALSE, 'uncheck_import_file();'); ?></td>
								</tr>
								<?php
							}
						?>
					</table>
					<?php
					$o->note(translate('Select a file to import into the system'));

				$o->closeField();

				$o->openField(translate('Asset to import Under'));

					asset_finder($type_code.'_root_assetid', '');
				$o->closeField();
				$o->openField(translate('New Site'));

					?>
					<table class="sq-backend-table" style="width: 300px;">
						<tr>
							<td>
								<?php echo translate('Create A New Site'); ?>

							</td>
							<td>
								<?php
									check_box($type_code.'_create_new_site', '1');
								?>
							</td>
						</tr>
						<tr>
							<td>
								<?php echo translate('New Site Name'); ?>

							</td>
							<td>
								<?php
									text_box($type_code.'_new_site_name', '', '30', '255');
								?>
							</td>
						</tr>
						<tr>
							<td>
								<?php echo 'Create Site Map for New Site:'; ?>
							</td>
							<td>
								<?php
									check_box($type_code.'_create_new_map', '1');
								?>
							</td>
						</tr>
					</table>
					<?php
					$o->note(translate('If this option is selected, the new site will be created under the asset selected above. If no \'Asset to import Under\' has been selected, the new site will be created under the root.'));

				$o->closeField();
				$o->openField('Headings to Split On');
					check_box($type_code.'_no_headings', '1', FALSE, 'document.getElementById(\''.$type_code.'_headings_table\').style.visibility = (this.checked == true) ? \'hidden\' : \'\';');
					label('Import as a Single Page (Headings will be ignored)', $type_code.'_no_headings');
					$heading_defaults = $itm->attr('heading_defaults');
					?>
					<table id="<?php echo $type_code.'_headings_table'; ?>" class="sq-backend-table" style="width: 300px;">
						<tr>
							<th class="sq-backend-table-header"><?php echo translate('Heading Style'); ?></th>

							<th class="sq-backend-table-header" style="width: 120px;"><?php echo translate('Use for structure?'); ?></th>

						</tr>

						<?php
							for ($i = 1; $i <= 6; $i++) {
								$h_string = 'h'.strval($i);
								?>
								<tr>
									<td class="sq-backend-table-cell"><?php echo strtoupper($h_string); ?></td>
									<td class="sq-backend-table-cell" style="text-align: center;">
									<?php
										check_box('structured_file_import_headings['.$h_string.']', '1', in_array($h_string, $heading_defaults));
									?>
									</td>
								</tr>
								<?php
							}
						?>
					</table>
					<?php
				$o->closeField();
				$o->openField(translate('Create CSS'));

					check_box($type_code.'_create_css', '1', $itm->attr('create_css_file'));
					$o->note(translate('Creating a separate CSS file for the page. This overrides the default setting in Import Tools Manager.'));

				$o->closeField();
				$o->openField(translate('Include Leading Text'));

					check_box($type_code.'_leading_text', '1');
					$o->note(translate('Checking this will create a page for any text that is found in the document before the first heading'));

				$o->closeField();
				$o->openField(translate('Leading Text Page Name'));

					text_box($type_code.'_leading_name', 'Preface');
					$o->note(translate('If the above option is checked, the page will be named this'));

				$o->closeField();
			$o->closeSection();

			$o->openSection(translate('HTML Tidying Options'));

				$o->openField(translate('Attempt to remove Microsoft Word Tags'));

					check_box($type_code.'_remove_word', '1');
				$o->closeField();
				$o->openField(translate('Run HTML Tidy'));

					check_box($type_code.'_run_tidy');
					$o->note(translate('Selecting this option will generate well-formed HTML, but may affect the formatting of the document. It can also slow down the importing process.'));

				$o->closeField();
				$o->openField(translate('Fix Anchor Tags and Footnotes'));

					check_box($type_code.'_footnotes');
				$o->closeField();
			$o->closeSection();

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
	public static function processTool(&$o, $type_code)
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$itm = $am->getSystemAsset('import_tools_manager');
		$vars = Array();
		$vars['import_file'] = array_get_index($_POST, 'structured_file_import_filename', '');
		$vars['remove_file'] = array_get_index($_POST, 'structured_file_remove_filename', '');
		$vars['create_new_site'] = FALSE;
		if (isset($_POST[$type_code.'_create_new_site'])) {
			// Create New Site Checked
			$vars['create_new_site'] = TRUE;
			$vars['new_site_name'] = $_POST[$type_code.'_new_site_name'];
		}
		$import_dir = STRUCTURED_FILE_IMPORT_DIR;
		if(!empty($import_dir) && strpos($import_dir, '/') !== 0)     $import_dir = SQ_SYSTEM_ROOT.'/'.$import_dir;
		$vars['root_assetid'] = $_POST[$type_code.'_root_assetid']['assetid'];
		$vars['use_headings'] = isset($_POST[$type_code.'_no_headings']) ? FALSE : TRUE;
		$vars['headings'] = array_keys($_POST['structured_file_import_headings']);
		$vars['import_dir'] = $import_dir;
		$vars['create_css'] = isset($_POST[$type_code.'_create_css_file']);
		$vars['create_site_map'] = isset($_POST[$type_code.'_create_new_map']);
		$vars['leading_create'] = isset($_POST[$type_code.'_leading_text']);
		$vars['leading_text'] = $_POST[$type_code.'_leading_name'];

		$vars['remove_word'] = isset($_POST[$type_code.'_remove_word']);
		$vars['run_tidy'] = isset($_POST[$type_code.'_run_tidy']);
		$vars['fix_anchors'] = isset($_POST[$type_code.'_footnotes']);

		if ($vars['run_tidy']) {
			$vars['tidy_path'] = SQ_TOOL_HTML_TIDY_PATH;
		}

		// Any deletion?
		$files_array = list_files($import_dir, TRUE);
		$remove_list = Array();
		foreach ($files_array as $id => $file_name) {
			$strrpos_files_name = strrpos($file_name, '/') + 1;
			$stripped_fname = substr($file_name, $strrpos_files_name);
			$replaced_fname = str_replace(Array('.',' '), '_', $stripped_fname);
			if (isset($_POST['structured_file_remove_'.$replaced_fname])) {
				$remove_list[] = $stripped_fname;
			}
		}
		if (empty($remove_list)) {
			// Importing section
			if (($vars['import_file'] == '') || (empty($vars['headings']) && ($vars['use_headings'])) || ($vars['root_assetid'] == '' && $vars['create_new_site'] == FALSE)) {
				return FALSE;
			}

			// We have all the required data, so queue up our HIPO and go to town
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$hh->queueHipo('hipo_job_structured_file_import', $vars, '', SQ_PACKAGES_PATH.'/import_tools/hipo_jobs/');
			$url = $hh->runQueuedJobs();
			if (!empty($url)) $o->setRedirect($url);
			return TRUE;

		} else {
			// Delete the imported files
			foreach ($remove_list as $file_name) {
				$tmp = explode('.', $file_name);
				$fname_no_ext = $tmp[0];
				if (file_exists($import_dir.'/'.$file_name)) {
					$success = unlink($import_dir.'/'.$file_name);
				}
				if ($success && file_exists($import_dir.'/'.$fname_no_ext.'.doc')) {
					$success = unlink($import_dir.'/'.$fname_no_ext.'.doc');
				}
				if ($success && file_exists($import_dir.'/'.$fname_no_ext.'.dot')) {
					$success = unlink($import_dir.'/'.$fname_no_ext.'.dot');
				}
				if ($success && file_exists($import_dir.'/'.$fname_no_ext.'_files')) {
					require_once SQ_FUDGE_PATH.'/general/file_system.inc';
					$success = delete_directory($import_dir.'/'.$fname_no_ext.'_files');
				}
				if (!$success) {
					// error report
					trigger_error('Failed to delete the imported files.');
					break;
				}
			}
			return FALSE;
		}


	}//end processTool()


}//end class


?>
