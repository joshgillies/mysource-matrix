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
* $Id: tool_bulk_file_import.inc,v 1.16.6.1 2013/07/26 01:40:44 akarelia Exp $
*
*/


require_once SQ_SYSTEM_ROOT.'/core/assets/system/tool/tool.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_DATA_PATH.'/private/conf/tools.inc';
require_once SQ_DATA_PATH.'/private/conf/import_tools_manager.inc';

/**
* Tool_Bulk_File_Import
*
* Purpose
*
*    Tool for importing files into Matrix
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.16.6.1 $
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
	public static function paintTool(&$o, $type_code)
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$itm = $am->getSystemAsset('import_tools_manager');
		$import_option = $itm->attr('allowed_import_type');
		$import_dir = BULK_FILE_IMPORT_DIR;
		$process_local = ($import_option != 'Server Only');
		$process_server = ($import_option != 'Local Only');

		$o->openSection(translate('file_import_tools_global_options'));
			$o->openField(translate('file_import_tools_allow_unrestricted'));
				combo_box($type_code.'_allow_unrestricted', Array(1 => 'Yes', 0 => 'No'));
				echo '<br />'.translate('file_import_tools_allow_unrestricted_note');
			$o->closeField();

			$o->openField('Link Type');
				combo_box($type_code.'_link_type', Array(SQ_LINK_TYPE_1 => 'TYPE_1', SQ_LINK_TYPE_2 => 'TYPE_2'));
				echo '<br />'.translate('file_import_tools_file_link_type_msg');
			$o->closeField();
		$o->closeSection();

		$o->openSection(translate('file_import_tools_file_dir_selection'));
			if (empty($import_dir)) {
				$o->openField(translate('file_import_tools_warning'));
					echo translate('file_import_tools_not_ready_msg');
				$o->closeSection();
			} else {

				if ($process_server) {

					$o->openField(translate('file_import_tools_server_import'));

							$compatible_types = $itm->attr('valid_file_types');

							// This will get us a list of all the files in the import directory
							$dirs_array = list_dirs($import_dir, TRUE);
							asort($dirs_array);
							?>
							<table class="sq-backend-table" style="width: 600px;">
								<tr>
									<th class="sq-backend-table-header" style="width: 130px;"><?php echo 'Directory'; ?></th>
									<th class="sq-backend-table-header" style="width: 80px; text-align: center;"><?php echo 'Select for Import'; ?></th>
									<th class="sq-backend-table-header" style="width: 390px; text-align: center;"><?php echo 'Import Under'; ?></th>
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

										// Local import temp directory, Do not display
										if (strpos($stripped_dname, '__tmp_file_upload__') !== FALSE) {
											continue;
										}

										?>
										<tr>
										<td class="sq-backend-table-cell"><?php echo $stripped_dname; ?>&nbsp;(<?php echo $num_of_files; ?> file<?php echo ($num_of_files > 1) ? 's' : ''; ?>)</td>
											<td class="sq-backend-table-cell" style="text-align: center;"><?php check_box('file_import_dirname_'.str_replace(' ', '_', $stripped_dname), $dir_name, FALSE); ?></td>
											<td class="sq-backend-table-cell" style="text-align: center;"><?php asset_finder(str_replace(' ', '_', $stripped_dname).'_root_assetid', ''); ?></td>
										</tr>
										<?php
									}
								?>
								<tr>
									<td class="sq-backend-table-cell" style="border-top: 0px;"colspan="3">
										<?php
											echo "Sort the files alphanumerically on import?   ";
											check_box('sort_file_import', '1', FALSE);
										?>
									</td>
								</tr>
							</table>
							<?php
							$o->note(translate('file_import_tools_select_file'));
					$o->closeField();

				}//end if Server Import

				if ($process_local) {

					$o->openField(translate('file_import_tools_local_import'));

						if (!is_writable($import_dir)) {
							$o->note(translate('file_import_tools_local_import_permission_warning', $import_dir));
						} else {
							// Max File Size for local file upload
							$max_size = Tool_Bulk_File_Import::_getMaxFileSize();
							?>
							<table class="sq-backend-table" style="width: 600px;">
								<tr>
									<th class="sq-backend-table-header" colspan="2"><?php
										echo translate('file_import_tools_local_file_import_root');
									?></th>
								</tr>
								<tr>
									<td class="sq-backend-table-header" style="width: 100px;"><?php
										echo translate('file_import_tools_root_asset');
									?></td>
									<td class="sq-backend-table-header" style="width: 500px;"><?php
										asset_finder('bulk_file_import_local_upload_root_asset', '');
									?></td>
								</tr>
							</table>

							<div style='display: none;`' id='bulk_file_import_local_file_input_container'></div>
							<div id='bulk_file_import_table_container'></div>

							<span style="font-size: 11px;">&nbsp;<?php echo translate('core_cannot_upload_file_larger_than', '<b>'.strtoupper(easy_filesize($max_size)).'</b>'); ?></span>
							<input type="hidden" name="MAX_FILE_SIZE" value="<?php echo $max_size; ?>" />

							<script type="text/javascript">
							//<![CDATA[

								function local_file_import_table()
								{
									this.tbody;
									this.table = document.createElement("table");
									this.id_count = 1;
									this.file_names = Array();

									// Browser Compatibility
									this.is_gecko = (navigator.product == 'Gecko');
									this.is_ie = (
													(navigator.userAgent.toLowerCase().indexOf("msie") != -1)
												);

									// Allowed extension for each file type
									this.image_exts = Array('jpg', 'jpeg', 'gif', 'png');
									this.doc_exts = Array('doc', 'dot', 'docx');
									this.pdf_exts = Array('pdf');
									this.xls_exts = Array('xls', 'xlt', 'xlsx', 'xlsm', 'xltx', 'xltm');
									this.rtf_exts = Array('rtf');
									this.txt_exts = Array('txt');
									this.js_exts = Array('js');
									this.ppt_exts = Array('ppt', 'pps', 'pot', 'pptx', 'ppsx', 'potx', 'pptm', 'potm', 'ppsm');
									this.css_exts = Array('css');
									this.mp3_exts = Array('mp3');
									this.video_exts = Array('mov', 'avi', 'wmv', 'asf', 'flv', 'mp4', 'm4v', 'mpg', 'mpeg');

									// Create the initial table for local imiport
									this.init = function()
									{
										var container = document.getElementById('bulk_file_import_table_container');

										this.table.className = 'sq-backend-table';
										this.table.style.width = '600px';

										// Create THead
										var thead = document.createElement('thead');
										var header_row = document.createElement('tr');
										var th = document.createElement('th');
										th.colSpan = '3';
										th.className = 'sq-backend-table-header';
										th.style.width = '100px';
										th.innerHTML = 'Files to Import';
										header_row.appendChild(th);
										thead.appendChild(header_row);

										// Create TBody
										var tbody = document.createElement('tbody');
										var body_row = document.createElement('tr');
										var row_id = 'bulk_file_upload_new_row_0';
										body_row.id = row_id;
										var td_1 = document.createElement('td');
										var td_2 = document.createElement('td');
										var td_3 = document.createElement('td');

										td_1.className = 'sq-backend-table-cell';
										td_2.className = 'sq-backend-table-cell';
										td_3.className = 'sq-backend-table-cell';

										td_1.style.borderTop = '1px solid #1A1A1A';
										td_2.style.borderTop = '1px solid #1A1A1A';
										td_3.style.borderTop = '1px solid #1A1A1A';

										td_1.style.width = '100px';
										td_2.style.width = '470px';
										td_3.style.width = '30px';

										td_1.innerHTML = '<b>Choose File</b>';
										td_2.innerHTML = '<input type="file" size="50" name="bulk_file_upload_file_chooser_0" id="bulk_file_upload_file_chooser_0" class="sq-form-field" onchange="local_file_table.fileSelected(this, \'0\');" />';
										td_3.innerHTML = '<img title="Delete News" src="<?php echo sq_web_path('data'); ?>/asset_types/bodycopy/images/icons/delete.png" onclick="local_file_table.removeRow(\'0\', true);">';

										body_row.appendChild(td_1);
										body_row.appendChild(td_2);
										body_row.appendChild(td_3);
										tbody.appendChild(body_row);
										this.tbody = tbody;

										// Create TFoot
										var tfoot = document.createElement('tfoot');
										var footer_row = document.createElement('tr');
										var td = document.createElement('td');
										td.className = 'sq-backend-table-cell';
										td.colSpan = '3';
										td.align = 'right';
										td.style.borderTop = '1px solid #000000';
										td.innerHTML = '<div id="insert_file_link" onclick="local_file_table.addRow();"><b>[Click Here to Add]</b></div>';
										footer_row.appendChild(td);
										tfoot.appendChild(footer_row);

										// Append all
										this.table.appendChild(thead);
										this.table.appendChild(tbody);
										this.table.appendChild(tfoot);
										container.appendChild(this.table);

									}//end init()


									this.addRow = function()
									{
										children = this.tbody.childNodes;
										this.tbody.appendChild(this._newRow(this.id_count));
										this.id_count++;

									}//end addRow()

									this._newRow = function(id)
									{
										var new_tr = document.createElement('tr');
										var row_id = 'bulk_file_upload_new_row_' + id;
										new_tr.id = row_id;
										var td_1 = document.createElement('td');
										var td_2 = document.createElement('td');
										var td_3 = document.createElement('td');

										td_1.className = 'sq-backend-table-cell';
										td_2.className = 'sq-backend-table-cell';
										td_3.className = 'sq-backend-table-cell';
										td_1.style.borderTop = '1px solid #19212A';
										td_2.style.borderTop = '1px solid #19212A';
										td_3.style.borderTop = '1px solid #19212A';
										td_1.innerHTML = '<b>Choose File</b>';
										td_2.innerHTML = '<input type="file" size="50" name="bulk_file_upload_file_chooser_' + id + '" id="bulk_file_upload_file_chooser_' + id + '" class="sq-form-field" onchange="local_file_table.fileSelected(this, \'' + id + '\');" />';
										td_3.innerHTML = '<img title="Delete News" src="<?php echo sq_web_path('data'); ?>/asset_types/bodycopy/images/icons/delete.png" onclick="local_file_table.removeRow(\'' + id + '\', true);">';

										new_tr.appendChild(td_1);
										new_tr.appendChild(td_2);
										new_tr.appendChild(td_3);

										return new_tr;

									}//end _newRow()

									this.removeRow = function(id, removeFromList)
									{
										elt = document.getElementById('bulk_file_upload_file_chooser_' + id);
										if (elt.tagName.toLowerCase() == 'span') {
											file_name = document.getElementById('bulk_file_upload_file_chooser_' + id + '_').value;
										} else if (elt.tagName == 'input') {
											file_name = '';
										}

										children = this.tbody.childNodes;
										for (var i = 0; i < children.length; i++) {
											if (children[i].id == 'bulk_file_upload_new_row_' + id) {
												type = this._currentFileType(id);
												if (type == 'image') {
													this.tbody.removeChild(children[i + 4]);
													this.tbody.removeChild(children[i + 3]);
													this.tbody.removeChild(children[i + 2]);
													this.tbody.removeChild(children[i + 1]);
												} else {
													if (children[i + 1] != null) {
														if (children[i + 1].id == 'bulk_file_upload_new_row_file_type_' + id) {
															this.tbody.removeChild(children[i + 2]);
															this.tbody.removeChild(children[i + 1]);
														}
													}
												}
												this.tbody.removeChild(children[i]);
												break;
											}
										}

										// Remove the file name from the list
										if (removeFromList) {
											new_file_names = Array();
											for (i = 0; i < this.file_names.length; i++) {
												if (file_name != this.file_names[i]) {
													new_file_names.push(this.file_names[i]);
												}
											}
											this.file_names = new_file_names;
										}

									}//end removeRow()

									// Set the type of the file as one of the specialised types, or file if none match
									this.fileSelected = function(elt, id)
									{
										// Replace input tag with span so that people can double select the file
										span = document.createElement('span');
										span.innerHTML = elt.value;
										span.id = elt.id;
										p_node = elt.parentNode;
										p_node.insertBefore(span, elt);
										elt.style.display = 'none';
										elt.id = elt.id + '_';

										// Duplication Checks.
										file_name = elt.value;
										for (i = 0; i < this.file_names.length; i++) {
											if (file_name == this.file_names[i]) {
												alert('The file, ' + file_name + ' already has been added.');
												this.removeRow(id, false);
												return FALSE;
											}
										}
										this.file_names.push(file_name);

										row_id = 'bulk_file_upload_new_row_' + id;
										children = this.tbody.childNodes;
										for (var i = 0; i < children.length; i++) {
											if (children[i].id == row_id) {
												extension = this._getFileType(file_name);
												if (i == (children.length - 1)) {
													// Last element, appendChild
													this.tbody.appendChild(this._fileTypeRow(id, extension));
													this.tbody.appendChild(this._newImageRow(id, 'Title'));
													if (this._currentFileType(id) == 'image') {
														this.tbody.appendChild(this._newImageRow(id, 'Alt'));
														this.tbody.appendChild(this._newImageRow(id, 'Caption'));
													}
												} else {
													// Somewhere in the middle, insertBefore
													this.tbody.insertBefore(this._fileTypeRow(id, extension), children[i + 1]);
													this.tbody.insertBefore(this._newImageRow(id, 'Title'), children[i + 2]);
													if (this._currentFileType(id) == 'image') {
														this.tbody.insertBefore(this._newImageRow(id, 'Alt'), children[i + 3]);
														this.tbody.insertBefore(this._newImageRow(id, 'Caption'), children[i + 4]);
													}
												}
												break;
											}
										}

									}//end fileSelected()

									this._fileTypeRow = function(id, ext)
									{
										new_tr = document.createElement('tr');
										new_tr.id = 'bulk_file_upload_new_row_file_type_' + id;
										td_1 = document.createElement('td');
										td_2 = document.createElement('td');


										td_1.className = 'sq-backend-table-cell';
										td_2.className = 'sq-backend-table-cell';
										td_1.innerHTML = 'File Type';

										// Detect the initial file type from the file extension
										if (this._isFileTypeIn(this.image_exts, ext)) {
											td_2.innerHTML = 'Image<input type="hidden" value="1" name="bulk_file_upload_row_' + id + '_file_type_image" id="bulk_file_upload_row_' + id + '_file_type_image" \>';
										} else if (this._isFileTypeIn(this.doc_exts, ext)) {
											td_2.innerHTML = 'MS Word<input type="hidden" value="1" name="bulk_file_upload_row_' + id + '_file_type_word" id="bulk_file_upload_row_' + id + '_file_type_word" \>';
										} else if (this._isFileTypeIn(this.pdf_exts, ext)) {
											td_2.innerHTML = 'PDF<input type="hidden" value="1" name="bulk_file_upload_row_' + id + '_file_type_pdf" id="bulk_file_upload_row_' + id + '_file_type_pdf" \>';
										} else if (this._isFileTypeIn(this.ppt_exts, ext)) {
											td_2.innerHTML = 'MS Powerpoint<input type="hidden" value="1" name="bulk_file_upload_row_' + id + '_file_type_powerpoint" id="bulk_file_upload_row_' + id + '_file_type_powerpoint" \>';
										} else if (this._isFileTypeIn(this.xls_exts, ext)) {
											td_2.innerHTML = 'MS Excel<input type="hidden" value="1" name="bulk_file_upload_row_' + id + '_file_type_excel" id="bulk_file_upload_row_' + id + '_file_type_excel" \>';
										} else if (this._isFileTypeIn(this.rtf_exts, ext)) {
											td_2.innerHTML = 'RTF<input type="hidden" value="1" name="bulk_file_upload_row_' + id + '_file_type_rtf" id="bulk_file_upload_row_' + id + '_file_type_rtf" \>';
										} else if (this._isFileTypeIn(this.txt_exts, ext)) {
											td_2.innerHTML = 'Text<input type="hidden" value="1" name="bulk_file_upload_row_' + id + '_file_type_text" id="bulk_file_upload_row_' + id + '_file_type_textf" \>';
										} else if (this._isFileTypeIn(this.css_exts, ext)) {
											td_2.innerHTML = 'CSS<input type="hidden" value="1" name="bulk_file_upload_row_' + id + '_file_type_css" id="bulk_file_upload_row_' + id + '_file_type_css" \>';
										} else if (this._isFileTypeIn(this.js_exts, ext)) {
											td_2.innerHTML = 'JS<input type="hidden" value="1" name="bulk_file_upload_row_' + id + '_file_type_js" id="bulk_file_upload_row_' + id + '_file_type_js" \>';
										} else if (this._isFileTypeIn(this.mp3_exts, ext)) {
											td_2.innerHTML = 'MP3<input type="hidden" value="1" name="bulk_file_upload_row_' + id + '_file_type_mp3" id="bulk_file_upload_row_' + id + '_file_type_mp3" \>';
										} else if (this._isFileTypeIn(this.video_exts, ext)) {
											td_2.innerHTML = 'Video<input type="hidden" value="1" name="bulk_file_upload_row_' + id + '_file_type_video" id="bulk_file_upload_row_' + id + '_file_type_video" \>';
										} else {
											td_2.innerHTML = 'Others<input type="hidden" value="1" name="bulk_file_upload_row_' + id + '_file_type_others" id="bulk_file_upload_row_' + id + '_file_type_others" \>';
										}

										td_2.colSpan = '2';
										new_tr.appendChild(td_1);
										new_tr.appendChild(td_2);

										return new_tr;

									}//end _fileTypeRow()

									this._isFileTypeIn = function(list, findme)
									{
										for (var i = 0; i < list.length; i++) {
											if (findme.toLowerCase() == list[i]) {
												return true;
											}
										}
										return false;

									}//end _isFileTypeIn()

									this._getFileType = function(name)
									{
										if (name.indexOf('.') == -1) {
											return '';
										}
										var arr = name.split('.');
										extension = arr[arr.length - 1];
										return extension.toLowerCase();

									}//end _isImage()

									// Return the currently chosen file type name in String
									// (one of the specialised types, or 'others' for other file types)
									// It also returns 'none' if the file has been chosen
									this._currentFileType = function(id)
									{
										types = Array('image', 'pdf', 'word', 'powerpoint', 'excel', 'rtf', 'text', 'js' , 'css', 'mp3', 'video', 'others');
										for (i = 0; i < types.length; i++) {
											radio_button = document.getElementById('bulk_file_upload_row_' + id + '_file_type_' + types[i]);
											if (radio_button != null) {
												if (radio_button.value == "1") {
													return types[i];
												}
											}
										}

									}//end _currentFileType()

									this._newImageRow = function(id, type)
									{
										var new_tr = document.createElement('tr');
										new_tr.id = 'bulk_file_upload_new_row_' + type.toLowerCase() + '_' + id;
										var td_1 = document.createElement('td');
										var td_2 = document.createElement('td');

										td_1.className = 'sq-backend-table-cell';
										td_2.className = 'sq-backend-table-cell';

										td_1.innerHTML = type;
										if (type == 'Caption') {
											td_2.innerHTML = '<textarea cols="60" rows="2" name="bulk_file_upload_new_row_' + type.toLowerCase() + '_msg_' + id + '" size="30" maxlength="255"></textarea>';
										} else {
											td_2.innerHTML = '<input type="text" name="bulk_file_upload_new_row_' + type.toLowerCase() + '_msg_' + id + '" size="30" maxlength="255" />';
										}
										td_2.colSpan = '2';

										new_tr.appendChild(td_1);
										new_tr.appendChild(td_2);

										return new_tr;

									}//end _newCaptionRow()

								}//end class local_file_import_table()

								var local_file_table = new local_file_import_table();
								local_file_table.init();
							//]]>
							</script>
							<?php
						}//end else

					$o->closeField();

				}//end if Local Upload

			}//end else

		$o->closeSection();

		// Divider
		$o->openSection(' ');
		$o->closeSection();

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
		$vars = Array();
		$itm = $am->getSystemAsset('import_tools_manager');
		$import_dir = BULK_FILE_IMPORT_DIR;
		$import_option = $itm->attr('allowed_import_type');
		$process_local = ($import_option != 'Server Only');
		$process_server = ($import_option != 'Local Only');

		// Process Server Upload
		if ($process_server) {
			$sorting = array_get_index($_POST, 'sort_file_import', FALSE);
			$dirs_array = list_dirs($import_dir, TRUE);

			foreach ($dirs_array as $dir_name) {
				$files = Tool_Bulk_File_Import::_getFileList($dir_name);
				$num_of_files = count($files);

				$stripped_dname = substr($dir_name, strrpos($dir_name, '/') + 1);
				$replaced_dname = str_replace(' ', '_', $stripped_dname);
				$tmp = array_get_index($_POST, 'file_import_dirname_'.$replaced_dname, '');
				if (!empty($tmp)) {

					$tmp = array_get_index($_POST, $replaced_dname.'_root_assetid', '');
					if (!empty($tmp['assetid'])) {
						if (!isset($vars['total_count'])) {
							$vars['total_count'] = $num_of_files;
						} else {
							$vars['total_count'] = $vars['total_count'] + $num_of_files;
						}
						$vars['server_file_import'][$dir_name]['file_list'] = $files;
						$vars['server_file_import'][$dir_name]['num_of_files'] = $num_of_files;
						$vars['server_file_import'][$dir_name]['root_id'] = $tmp['assetid'];
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
					$vars['server_file_import'][$import_dir]['file_list'] = $files;
					$vars['server_file_import'][$import_dir]['num_of_files'] = count($files);
					$vars['server_file_import'][$import_dir]['root_id'] = $tmp['assetid'];
				}
			}

			if ($sorting && isset($vars['server_file_import'])) {
				foreach ($vars['server_file_import'] as $import_dir_name => $import_file_info) {
					if (isset($vars['server_file_import'][$import_dir_name]['file_list']))
						sort($vars['server_file_import'][$import_dir_name]['file_list']);
				}
			}
		}//end if

		// Process Local Upload
		if ($process_local) {

			if (isset($_POST['bulk_file_import_local_upload_root_asset'])) {
				$local_import_rootid = $_POST['bulk_file_import_local_upload_root_asset']['assetid'];
			} else {
				if (!empty($_FILES)) {
					// the user didn't pick the root asset, show some warning.
					trigger_error(translate('file_import_tools_local_file_no_root_asset'), E_USER_WARNING);
				}
			}

			if (!empty($local_import_rootid) && !empty($_FILES)) {

				$error = FALSE;
				$file_types = Array('image', 'pdf', 'word','powerpoint', 'excel', 'rtf', 'text', 'js' , 'css', 'mp3', 'video', 'others');
				$uploaded_files = Array();
				$tmp_dir_name = $type_code.'__tmp_file_upload__';

				foreach ($_FILES as $id => $data) {
					$file_info = get_file_upload_info($id);

					if (!empty($file_info)) {

						// File Upload
						if (!file_exists($import_dir.'/'.$tmp_dir_name)) {
							if (!mkdir($import_dir.'/'.$tmp_dir_name)) {
								trigger_error(translate('file_import_tools_cannot_create_temp_dir'), E_USER_WARNING);
								return FALSE;
							}
						}

						$result = commit_file_upload($id, $import_dir.'/'.$tmp_dir_name, TRUE);
						if ($result === FALSE) {
							trigger_error(translate('file_import_tools_local_file_upload_error', $file_info['name']), E_USER_WARNING);
							$error = TRUE;
						} else if ($result === '') {
							trigger_error(translate('file_import_tools_local_file_cannot_upload', $file_info['name']), E_USER_WARNING);
							$error = TRUE;
						} else if ($result == $file_info['name']) {

							$text_length = strlen('bulk_file_upload_file_chooser_');
							$file_id = substr($id, $text_length);

							// get file type
							foreach ($file_types as $file_type) {
								$value = array_get_index($_POST, 'bulk_file_upload_row_'.$file_id.'_file_type_'.$file_type, '');
								if (!empty($value)) {
									$file_info['file_type'] = $file_type;
									break;
								}
							}

							// if it's image, get title, alt and caption
							if ($file_info['file_type'] == 'image') {
								foreach (Array('title', 'alt', 'caption') as $name) {
									$file_info[$name] = array_get_index($_POST, 'bulk_file_upload_new_row_'.$name.'_msg_'.$file_id, '');
								}
							} else {
								$file_info['title'] = array_get_index($_POST, 'bulk_file_upload_new_row_title_msg_'.$file_id, '');
							}

							$uploaded_files[] = $file_info;
						}

						// We may want to delete the uploaded files here, if it is required.
						if ($error) return FALSE;

					}//end if

				}//end foreach $_FILES

				if (!empty($uploaded_files)) {
					$vars['local_file_import']['rootid'] = $local_import_rootid;
					$vars['local_file_import']['location'] = $import_dir.'/'.$tmp_dir_name;
					$vars['local_file_import']['files'] = $uploaded_files;
				}

			}//end if !empty($_FILES)


		}//end if Process Local

		if (!empty($vars['server_file_import']) || !empty($vars['local_file_import'])) {

			$vars['allow_unrestricted'] = array_get_index($_POST, $type_code.'_allow_unrestricted', TRUE);
			$vars['link_type'] = array_get_index($_POST, $type_code.'_link_type', SQ_LINK_TYPE_1);
			// We have all the required data, so queue up our HIPO and go to town
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$result = $hh->queueHipo('hipo_job_bulk_file_import', $vars, '', SQ_PACKAGES_PATH.'/import_tools/hipo_jobs/');
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
	protected static function _getFileList($dir)
	{
		$files = list_files($dir);
		foreach ($files as $key => $file_name) {
			if (substr($file_name, 0, 1) == '.') {
				unset($files[$key]);
			}
		}

		return $files;

	}//end _getFileList()


	/**
	* Returns the maximum file size to be allowed to upload
	*
	* @return int
	* @access public
	*/
	protected static function _getMaxFileSize()
	{
		$user_prefs_max_file_size = $GLOBALS['SQ_SYSTEM']->getUserPrefs('file', 'SQ_FILE_MAX_SIZE');
		$max_size = strtolower($user_prefs_max_file_size);
		if ($max_size) {
			if (substr($max_size, -1) == 'k') {
				$max_size = $max_size * 1024;
			} else if (substr($max_size, -1) == 'm') {
				$max_size = $max_size * 1024 * 1024;
			}
		} else {
			// work out the max file size that PHP is allowing
			$ini_size = strtolower(ini_get('upload_max_filesize'));
			if (substr($ini_size, -1) == 'k') {
				$ini_size = $ini_size * 1024;
			} else if (substr($ini_size, -1) == 'm') {
				$ini_size = $ini_size * 1024 * 1024;
			}

			// work out the max post size that PHP is allowing
			$post_size = strtolower(ini_get('post_max_size'));
			if (substr($post_size, -1) == 'k') {
				$post_size = $post_size * 1024;
			} else if (substr($post_size, -1) == 'm') {
				$post_size = $post_size * 1024 * 1024;
			}

			$mem_limit_size = strtolower(ini_get('memory_limit'));
			if (substr($mem_limit_size, -1) == 'k') {
				$mem_limit_size = $mem_limit_size * 1024;
			} else if (substr($mem_limit_size, -1) == 'm') {
				$mem_limit_size = $mem_limit_size * 1024 * 1024;
			}
			$max_size = min($ini_size, $post_size, $mem_limit_size);
		}
		return $max_size;

	}//end _getMaxFileSize()


}//end class


?>
