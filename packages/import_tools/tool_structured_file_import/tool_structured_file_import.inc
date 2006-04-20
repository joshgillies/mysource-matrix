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
* $Id: tool_structured_file_import.inc,v 1.3 2006/04/20 03:46:31 skim Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/assets/system/tool/tool.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_DATA_PATH.'/private/conf/tools.inc';

/**
* Tool_Structured_File_Import
*
* Purpose
*
*    Tool for reindexing assets so that the Search Manager can use the index for
*    searching.
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.3 $
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
	function paintTool(&$o, $type_code)
	{
		ob_start();

		$am = $GLOBALS['SQ_SYSTEM']->am;
		$itm =& $am->getSystemAsset('import_tools_manager');

		$import_dir = $itm->attr('import_dir');
		if (empty($import_dir)) {

			$o->openSection('Warning');
				$o->openField(translate('import_tools_not_ready'));
					echo translate('import_tools_not_ready_msg');
				$o->closeField();
			$o->closeSection();

		} else {

			// Some variable initialisation
			$tool_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);
			$o->openSection($tool_info['name']);

			$o->closeSection();

			$o->openSection(translate('import_tools_new_html_file'));

			$o->closeSection();


			$o->openSection(translate('import_tools_available_files'));

				$compatible_types = $itm->attr('valid_file_types');
				$o->openField(translate('import_tools_import_dir'));
					echo $import_dir;
				$o->closeField();
				$o->openField(translate('import_tools_files'));
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
					<table class="sq-backend-table" style="width: 300px;">
						<tr>
							<th class="sq-backend-table-header"><?php echo translate('import_tools_select_file'); ?></th>
							<th class="sq-backend-table-header" style="width: 100px;"><?php echo translate('import_tools_select_for_import'); ?></th>
						</tr>
						<tr>
							<td class="sq-backend-table-cell"><?php echo translate('none'); ?></td>
							<td class="sq-backend-table-cell" style="text-align: center;"><?php radio_button('structured_file_import_filename', '', TRUE); ?></td>
						</tr>
						<?php
							foreach ($files_array as $file_name) {
								$stripped_fname = substr($file_name, strrpos($file_name, '/') + 1);
								?>
									<tr>
										<td class="sq-backend-table-cell"><?php echo $stripped_fname; ?></td>
										<td class="sq-backend-table-cell" style="text-align: center;"><?php radio_button('structured_file_import_filename', $file_name, FALSE); ?></td>
									</tr>
								<?php
							}
						?>
					</table>
					<?php
					$o->note(translate('import_tools_select_file'));
				$o->closeField();
				$o->openField(translate('import_tools_asset_to_import'));
					asset_finder($type_code.'_root_assetid', '');
					?>
						<span style="font-weight: bold;"><?php echo strtoupper(translate('or')); ?></span>
					<?php
				$o->closeField();
				$o->openField(translate('import_tools_new_site'));
					?>
					<table class="sq-backend-table" style="width: 300px;">
						<tr>
							<td>
								<?php echo translate('import_tools_create_new_site'); ?>
							</td>
							<td>
								<?php
									check_box($type_code.'_create_new_site', '1');
								?>
							</td>
						</tr>
						<tr>
							<td>
								<?php echo translate('import_tools_create_new_site_name'); ?>
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
					$o->note(translate('import_tools_create_new_site_note'));
				$o->closeField();
				$o->openField('Headings to Split On');
					check_box($type_code.'_no_headings', '1', FALSE, 'document.getElementById(\''.$type_code.'_headings_table\').style.visibility = (this.checked == true) ? \'hidden\' : \'\';');
					label('Import as a Single Page (Headings will be ignored)', $type_code.'_no_headings');
					$heading_defaults = $itm->attr('heading_defaults');
					?>
					<table id="<?php echo $type_code.'_headings_table'; ?>" class="sq-backend-table" style="width: 300px;">
						<tr>
							<th class="sq-backend-table-header"><?php echo translate('import_tools_heading_style'); ?></th>
							<th class="sq-backend-table-header" style="width: 120px;"><?php echo translate('import_tools_use_for_structure'); ?></th>
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
				$o->openField(translate('import_tools_create_css'));
					check_box($type_code.'_create_css', '1', $itm->attr('create_css_file'));
					$o->note(translate('import_tools_create_css_note'));
				$o->closeField();
				$o->openField(translate('import_tools_leading_text'));
					check_box($type_code.'_leading_text', '1');
					$o->note(translate('import_tools_leading_text_note'));
				$o->closeField();
				$o->openField(translate('import_tools_leading_page_name'));
					text_box($type_code.'_leading_name', 'Preface');
					$o->note(translate('import_tools_leading_page_name_note'));
				$o->closeField();
			$o->closeSection();

			$o->openSection(translate('import_tools_tidying_options'));
				$o->openField(translate('import_tools_word_tags'));
					check_box($type_code.'_remove_word', '1');
				$o->closeField();
				$o->openField(translate('import_tools_html_tidy'));
					check_box($type_code.'_run_tidy');
					$o->note(translate('import_tools_tidy_note'));
				$o->closeField();
				$o->openField(translate('import_tools_footnotes'));
					check_box($type_code.'_footnotes');
				$o->closeField();
			$o->closeSection();

			$o->openSection(translate('import_tools_perform_operations'));
				$o->openField('', 'commit');
					echo '&nbsp;';
					hidden_field('committed_tool_type_code', $type_code);
					submit_button('tool_commit', SQ_CONF_COMMIT_BUTTON_TEXT);
				$o->closeField();
			$o->closeSection();
		}
		return ob_get_clean();

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
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$itm =& $am->getSystemAsset('import_tools_manager');
		$vars = Array();
		$vars['import_file'] = array_get_index($_POST, 'structured_file_import_filename', '');
		$vars['create_new_site'] = FALSE;
		if (isset($_POST[$type_code.'_create_new_site'])) {
			// Create New Site Checked
			$vars['create_new_site'] = TRUE;
			$vars['new_site_name'] = $_POST[$type_code.'_new_site_name'];
		}
		$vars['root_assetid'] = $_POST[$type_code.'_root_assetid']['assetid'];
		$vars['use_headings'] = isset($_POST[$type_code.'_no_headings']) ? FALSE : TRUE;
		$vars['headings'] = array_keys($_POST['structured_file_import_headings']);
		$vars['import_dir'] = $itm->attr('import_dir');
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

		// Importing section

		if (($vars['import_file'] == '') || (empty($vars['headings']) && ($vars['use_headings'])) || ($vars['root_assetid'] == '' && $vars['create_new_site'] == FALSE)) {
			Tool_Structured_File_Import::paintTool($o, $type_code);
			return;
		}

		// We have all the required data, so queue up our HIPO and go to town
		$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$hh->queueHipo('hipo_job_structured_file_import', $vars, '', SQ_PACKAGES_PATH.'/import_tools/hipo_jobs/');
		$url = $hh->runQueuedJobs();
		if (!empty($url)) $o->setRedirect($url);

		return;

	}//end processTool()


}//end class


?>
