<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: video_file_edit_fns.inc,v 1.1.4.2 2009/04/28 06:37:04 mbrydon Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/files/file/file_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';

/**
* Video_File_Edit_Fns
*
* Purpose
*    Edit functions file for the Video File asset.
*
*
* @author     Nicholas Hubbard <nic@zedsaid.com>
* @package    MySource_Matrix_Packages
* @version    $Revision: 1.1.4.2 $
* @subpackage __core__
*/
class Video_File_Edit_Fns extends File_Edit_Fns
{


	function Video_File_Edit_Fns()
	{
		$this->File_Edit_Fns();
		$this->static_screens['details']['force_unlock'] = 1;

	}//end constructor


	/**
	* Processes the interface for uploading a file
	*
	* If you want to process an upload of a file that was not uploaded through PHP
	* (possible because of FTP or it was in a tar archive), ensure you pass the information
	* about the file in the $info array.
	* The fields required are:
	*     name              => the filename without path (eg file.txt)
	*     tmp_name          => the location of the file on the file system
	*     non_uploaded_file => must equal TRUE
	*
	* @param object	&$asset	the file asset
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for form elements
	* @param array	$info	info about the uploaded file
	*
	* @return boolean
	* @access public
	*/
	function processFileUpload(&$asset, &$o, $prefix, $info=Array())
	{
		if (!parent::processFileUpload($asset, $o, $prefix, $info)) {
			return FALSE;
		}

		// assetid will not be set if we are creating
		if (!$asset->id) return TRUE;

		$info = $asset->getExistingFile();

		// Check to see if our getID3 System Tool is enabled
		// If so, flag the asset for ID3 extraction on save
		if (SQ_TOOL_GETID3_ENABLED) {
			$asset->setAttrValue('extract_id3', TRUE);
		}

		return TRUE;

	}//end processFileUpload()


	/**
	* Paint the interface for the "extract ID3 metadata" field
	*
	* @param Video_File			$asset	the asset whose interface we are processing
	* @param Backend_Outputter	$o		the outputter class
	* @param string				$prefix	prefix for form elements
	*
	* @return boolean
	*/
	function paintExtractID3(Video_File $asset, Backend_Outputter $o, $prefix)
	{
		// Write attributes
		$wa = $asset->writeAccess('attributes');

		if ($wa) {
			check_box($prefix.'_extract_id3', 1, FALSE);
		} else {
			?><em>Lock the asset to access this option</em><?php
		}

		if (!SQ_TOOL_GETID3_ENABLED) {
			// Check to see if getID3 is installed and turned on
			?><br/><span class="sq-backend-warning">Video Data extraction is not currently enabled</span><?php
		}

		return $wa;

	}//end paintExtractID3()


	/**
	* Process the interface for the "extract ID3 metadata" field
	*
	* If checked, set the extract_id3 flag attribute to TRUE, so that the metadata
	* will be extracted. If not, then DON'T set to FALSE, because it may have been
	* set in the file upload.
	*
	* @param Video_File			$asset	the asset whose interface we are processing
	* @param Backend_Outputter	$o		the outputter class
	* @param string				$prefix	prefix for form elements
	*
	* @return boolean
	*/
	public function processExtractID3(Video_File $asset, Backend_Outputter $o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		$extract = isset($_POST[$prefix.'_extract_id3']);

		if ($extract) {
			$asset->setAttrValue('extract_id3', TRUE);
		}

		return TRUE;

	}//end processExtractID3()


}//end class
?>
