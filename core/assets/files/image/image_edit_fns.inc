<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: image_edit_fns.inc,v 1.18 2004/07/23 07:09:59 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/files/file/file_edit_fns.inc';

/**
* Image_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Image_Edit_Fns extends File_Edit_Fns
{

	/**
	* Get a formatted string of extensions (eg 1,2,3 and 4)
	*
	* @param array(string)	$extensions	array of extensions to format
	* @param string			$glue		word used to join last extension to the string (and, or etc)
	*
	* @return string
	* @access public
	*/
	function _getExtensionString($extensions, $glue='and')
	{
		if (empty($extensions) || !is_array($extensions)) return '';
		
		$count = count($extensions);
		$retval = '';
		
		for ($i = 0; $i < $count; $i++) {
			$ext = $extensions[$i];
			if ($i != 0 && ($i+1) == $count) {
				// the last in the list
				$retval .= " $glue ";
			} else if ($i != 0) {
				$retval .= ', ';
			}
			$retval .= $ext;
		}
		
		return $retval;

	}//end _getExtensionString()


	/**
	* Prints info about the current uploaded file
	*
	* @param object File	&$asset	the file asset
	*
	* @return void
	* @access public
	*/
	function printExistingFileInfo(&$asset)
	{
		parent::printExistingFileInfo($asset);
		$existing = $asset->_getExistingFile();
		if (!empty($existing)) {
			?><b>Dimensions:</b> <?php echo $existing['width'].' x '.$existing['height']?> pixels<br/><?php
		}

	}//end printExistingFileInfo()


	/**
	* Prints the interface for uploading a file
	*
	* @param object Image				&$asset	the file asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintFileUpload(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			file_upload($prefix);

			?><br/><span style="font-size: 7pt"><b>Note:</b> You may only upload files of type <?php echo $this->_getExtensionString($asset->allowed_extensions)?></span><?php
		}

		$this->printExistingFileInfo($asset);
		return true;
	
	}//end paintFileUpload()


	/**
	* Processes the interface for uploading a file
	*
	* @param object Image				&$asset	the file asset
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for form elements
	* @param array(string)				$info	info about the uploaded file
	*
	* @return boolean
	* @access public
	*/
	function processFileUpload(&$asset, &$o, $prefix, $info=Array())
	{
		if (!$asset->writeAccess('attributes')) return false;

		if (empty($info)) {
			$info = get_file_upload_info($prefix);
			// return on failed or no upload
			if ($info === false || empty($info)) return false;
		}

		// check that the extension is allowed
		if (!$asset->validFile($info['name'])) return false;

		return parent::processFileUpload($asset, $o, $prefix, $info);

	}//end processFileUpload()

}//end class
?>
