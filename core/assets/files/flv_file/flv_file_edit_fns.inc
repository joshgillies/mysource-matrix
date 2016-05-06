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
* $Id: flv_file_edit_fns.inc,v 1.3 2012/08/30 01:09:07 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/files/file/file_edit_fns.inc';

/**
* FLV_File_Edit_Fns
*
* Purpose
*
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class FLV_File_Edit_Fns extends File_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
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
	* @param File				$asset				the file asset
	* @param Backend_Outputter	$o					the Backend Outputter object
	* @param string				$prefix				prefix for form elements
	* @param array				$info				info about the uploaded file
	* @param boolean			$redirect_in_limbo	redirect when the file name changes in limbo
	* @param boolean			$asset_cloned		to check if we creating a new asset or just cloning
	*
	* @return boolean
	* @access public
	*/
	function processFileUpload(&$asset, &$o, $prefix, $info=Array(), $redirect_in_limbo=TRUE, $asset_cloned=FALSE)
	{
		if (!parent::processFileUpload($asset, $o, $prefix, $info, $redirect_in_limbo, $asset_cloned)) {
			return FALSE;
		}

		//assetid will not be set if we are creating a new asset
		//otherwise, tell the FLV asset that we want to get its basic metadata and store them in its attributes
		if (!empty($asset->id)) {
			$asset->setAttrValue('extract_metadata', TRUE);
		}

		return TRUE;

	}// end processFileUpload()


	/**
	* Paint the interface for metadata extraction
	*
	* @param FLV_File			$asset	the asset whose interface we are processing
	* @param Backend_Outputter	$o		the outputter class
	* @param string				$prefix	prefix for form elements
	*
	* @return boolean
	*/
	function paintExtractMetadata(FLV_File $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		if ($write_access) {
			check_box($prefix.'_extract_metadata', 1, FALSE);
		} else {
			?><em>Acquire Locks on the asset to access this option</em><?php
		}

		// Check to see if getID3() is enabled
		if (!SQ_TOOL_GETID3_ENABLED) {
			?><br/><span class="sq-backend-warning">The getID3() tool is not currently enabled</span><?php
		} else if (!is_file(SQ_TOOL_GETID3_PATH.'/getid3.php')) {
			?><br/><span class="sq-backend-warning">The path to getID3() tool is invalid</span><?php
		}

		return $write_access;

	}// end paintExtractMetadata()


	/**
	* Process the interface for metadata extraction
	*
	* If checked, get the basic metadata and store them in the attributes.
	*
	* @param FLV_File			$asset	the asset whose interface we are processing
	* @param Backend_Outputter	$o		the outputter class
	* @param string				$prefix	prefix for form elements
	*
	* @return boolean
	*/
	public function processExtractMetadata(FLV_File $asset, Backend_Outputter $o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		$extract = array_get_index($_POST, $prefix.'_extract_metadata', FALSE);

		if ($extract) {
			//tell the FLV asset that it needs to extract basic metadata and store them in its attributes
			$asset->setAttrValue('extract_metadata', TRUE);
		}

		return TRUE;

	}// end processExtractMetadata()


}//end class

?>
