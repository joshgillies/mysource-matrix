<?php

require_once SQ_CORE_PACKAGE_PATH.'/files/file/file_edit_fns.inc';

/**
* Image_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class Image_Edit_Fns extends File_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Image_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

	}//end constructor


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
		if ($asset->writeAccess()) {
			file_upload($prefix);

			?><br/><span style="font-size: 7pt"><b>Note:</b> You may only upload files of type <?php echo $this->_getExtensionString($asset->allowed_extensions)?></span><?php
		}

		$this->_printExistingFileInfo($asset);
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
		if (!$asset->writeAccess()) return false;

		if (empty($info)) {
			$info = get_file_upload_info($prefix);
			// return on failed or no upload
			if ($info === false || empty($info)) return false;
		}

		// check that the extension is allowed
		if (!$this->validFile($info['name'])) return false;

		return parent::processFileUpload($asset, $o, $prefix, $info);

	}//end processFileUpload()

}//end class
?>
