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
* $Id: import_tools_manager_edit_fns.inc,v 1.2 2006/10/29 23:51:40 skim Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Import_Tools_Manager_Edit_Fns
*
* Edit functions for the search manager.
*
* @see Asset_Edit_Fns
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.2 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage import_tools
*/
class Import_Tools_Manager_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	* @see Asset_Edit_Fns::Asset_Edit_Fns
	*/
	function Import_Tools_Manager_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

	}//end constructor


	/**
	* Paint Headings
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintHeadings(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$heading_defaults = $asset->attr('heading_defaults');
		$o->openField('Heading Selection Defaults');
		// Images for showing boolean values, when there is no
		$tick = '<img src="'.sq_web_path('lib').'/web/images/tick.gif" alt="Yes" style="position: relative; right: 0px;"/>';
		$cross = '<img src="'.sq_web_path('lib').'/web/images/cross.gif" alt="Yes" />';
		?>
		<table class="sq-backend-table" style="width: 300px;">
			<tr>
				<th class="sq-backend-table-header">Heading Style</th>
				<th class="sq-backend-table-header" style="width: 120px;">Use for structure?</th>
			</tr>
			<?php
				for ($i = 1; $i <= 6; $i++) {
					$h_string = 'h'.strval($i);
					?>
					<tr>
						<td class="sq-backend-table-cell"><?php echo strtoupper($h_string); ?></td>
						<td class="sq-backend-table-cell" style="text-align: center;">
							<?php
								if ($write_access) {
									check_box($prefix.'_headings['.$h_string.']', '1', in_array($h_string, $heading_defaults));
								} else {
									echo (in_array($h_string, $heading_defaults)) ? $tick : $cross;
								}
							?>
						</td>
					</tr>
					<?php
				}
			?>
		</table>
		<?php
		$o->closeField();

	}//end paintHeadings()


	/**
	* Processes the interface for creating containers in the bodycopy
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processHeadings(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			$asset->setAttrValue('heading_defaults', array_keys($_POST[$prefix.'_headings']));
		}

	}//end processHeadings()


	/**
	* Processes the interface for creating containers in the bodycopy
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintHtmlUpload(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$o->openField(translate('import_tools_new_html_file_select'));
			if ($write_access) {
				file_upload('structured_file_import_html_upload', $extras='');
				echo '&nbsp;&nbsp;&nbsp;&nbsp;';
				check_box('structured_file_import_html_upload_overwrite');
				label(translate('import_tools_overwrite_existing'), 'structured_file_import_html_upload_overwrite');
			}
		$o->closeField();

	}//end paintHtmlUpload()


	/**
	* Processes the interface for creating containers in the bodycopy
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processHtmlUpload(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		if (!$write_access) {
			$html_upload = get_file_upload_info('structured_file_import_html_upload');
			$max_size = $asset->attr('max_file_size');
			if ($html_upload === FALSE) {
				trigger_error('Error Uploading HTML File', E_USER_WARNING);
			} else {
				if (!empty($html_upload)) {
					$import_dir = $asset->attr('import_dir');
					$compatible_types = $asset->attr('valid_file_types');
					$overwrite = isset($_POST['structured_file_import_html_upload_overwrite']);
					commit_file_upload('structured_file_import_html_upload', $import_dir, $overwrite, $max_size, TRUE, $compatible_types);
				}
			}
		}

	}//end processHtmlUpload()


	/**
	* Paint the interface for Bulk File Import Tool Settings
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintBulkFileImport(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$allowed_import_type = $asset->attr('allowed_import_type');
		$server_import_dir = $asset->attr('bulk_file_import_dir');
		$local_file_max_size = $asset->attr('bulk_file_import_max_file_size');

		if ($write_access) {
			$o->openField(translate('import_tools_manager_allowed_import_type'));
				combo_box($prefix.'_allowed_import_type', Array('Both' => 'Server and Local', 'Server Only' => 'Server Only', 'Local Only' => 'Local Only'), FALSE, $allowed_import_type, 0);
			$o->closeField();

			$o->openField(translate('import_tools_manager_server_import_dir'));
				text_box($prefix.'_server_import_directory', $server_import_dir, '50', '', FALSE);
				$o->note(translate('import_tools_manager_server_import_dir_msg'));
			$o->closeField();

		} else {
			$o->openField(translate('import_tools_manager_allowed_import_type'));
				echo $allowed_import_type;
			$o->closeField();

			$o->openField(translate('import_tools_manager_server_import_dir'));
				echo $server_import_dir;
				$o->note(translate('import_tools_manager_server_import_dir_msg'));
			$o->closeField();
		}

		return $write_access;

	}//end paintBulkFileImport()


	/**
	* Process the interface for Bulk File Import Tool Settings
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processBulkFileImport(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		if ($write_access) {
			$allowed_import_type = array_get_index($_POST, $prefix.'_allowed_import_type', '');
			$server_import_dir = array_get_index($_POST, $prefix.'_server_import_directory', '');

			$asset->setAttrValue('allowed_import_type', $allowed_import_type);
			$asset->setAttrValue('bulk_file_import_dir', $server_import_dir);
		}

		return $write_access;

	}//end processBulkFileImport()


}//end class
?>
