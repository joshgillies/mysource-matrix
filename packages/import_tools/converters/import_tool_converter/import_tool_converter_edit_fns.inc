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
* $Id: import_tool_converter_edit_fns.inc,v 1.7 2012/08/30 00:58:50 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

/**
* Import_Tool_Converter_Edit_Fns
*
* Purpose
*
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage import_tools
*/
class Import_Tool_Converter_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Import_Tool_Converter_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

	}//end constructor


	/**
	* Paint Upload Interface
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintUpload(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		$o->openField('Existing Asset');
			asset_finder($prefix.'_existing_asset', 0, Array('file' => 'D'));
			echo '&nbsp;&nbsp;&nbsp;&nbsp;<span style="font-weight: bold">OR</span>';
		$o->closeField();
		$o->openField(translate('Select a document to upload'));

			file_upload($prefix.'_file_import_upload', $extras='');
			echo '&nbsp;&nbsp;&nbsp;&nbsp;';
			check_box($prefix.'_file_import_upload_overwrite');
			label(translate('Overwrite existing document'), $prefix.'_file_import_upload_overwrite');

		$o->closeField();

		return TRUE;

	}//end paintUpload()


	/**
	* Process Uploads to this asset
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processUpload(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		// Handle some file uploads
		$import_dir = $asset->attr('upload_directory');
		if(!empty($import_dir) && strpos($import_dir, '/') !== 0)     $import_dir = SQ_SYSTEM_ROOT.'/'.$import_dir;
		$compatible_types = $asset->attr('valid_file_types');

		// Check if we have an existing asset selected and copy it over
		if (isset($_POST[$prefix.'_existing_asset']) && ($_POST[$prefix.'_existing_asset']['assetid'] != '0')) {
			$existing_asset = $am->getAsset($_POST[$prefix.'_existing_asset']['assetid']);
			$file_data = $existing_asset->getExistingFile();
			if (!empty($file_data)) {
				copy($file_data['path'], $import_dir.'/'.str_replace(' ', '_', $file_data['filename']));
			}
			return TRUE;
		}

		$new_file_upload = get_file_upload_info($prefix.'_file_import_upload');

		if ($new_file_upload === FALSE) {
			trigger_error('Error Uploading Word Document', E_USER_WARNING);
		} else {
			if (!empty($new_file_upload)) {
				$itm = $am->getSystemAsset('import_tools_manager');
				$max_size = $itm->attr('max_file_size');
				if (!is_dir($import_dir)) {
					create_directory($import_dir);
				}
				$overwrite = isset($_POST[$prefix.'_file_import_upload_overwrite']);
				commit_file_upload($prefix.'_file_import_upload', $import_dir, $overwrite, $max_size, TRUE, $compatible_types);

				// create a word document under the manager
				$filename = $new_file_upload['name'];
				$import_link = Array('asset' => $asset, 'link_type' => SQ_LINK_TYPE_2);
				$temp_info = Array('name' => $filename, 'tmp_name' => $import_dir.'/'.$filename, 'non_uploaded_file' => TRUE);

				$asset_type = $asset->attr('create_asset_type');
				$am->includeAsset($asset_type);
				$new_file = new $asset_type;
				$new_file->_tmp['uploading_file'] = TRUE;
				$new_file->setAttrValue('name', $filename);

				if (!$new_file->create($import_link, $temp_info)) {
					trigger_error('Failed to import '.$asset_type.' '.$filename, E_USER_WARNING);
				}
				// Change the file name, so we can use it in windows
				rename($import_dir.'/'.$new_file_upload['name'], $import_dir.'/'.str_replace(' ', '_', $new_file_upload['name']));
			}
		}

	}//end processUpload()


}//end class
?>
