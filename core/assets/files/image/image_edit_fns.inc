<?php

require_once SQ_CORE_PACKAGE_PATH.'/file/file_edit_fns.inc';

/**
* Image_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
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
	*
	* @return string
	* @access public
	*/
	function _getExtensionString($extensions)
	{
		if (empty($extensions) || !is_array($extensions)) return '';
		
		$count = count($extensions);
		$retval = '';
		
		for ($i = 0; $i < $count; $i++) {
			$ext = $extensions[$i];
			if ($i != 0 && ($i+1) == $count) {
				// the last in the list
				$retval .= ' and ';
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
		file_upload($prefix);

		?><br/><span style="font-size: 7pt"><b>Note:</b> You may only upload files of type <?php echo $this->_getExtensionString($asset->allowed_extensions)?></span><?php

		$existing = $asset->_getExistingFile();
		if (!empty($existing)) {
			?> 
				<p style="font-size: 7pt">
				<b>Current File</b><br/>
				<b>Size:</b> <?php echo easy_filesize($existing['size'])?><br/>
				<b>Updated:</b> <?php echo readable_datetime($existing['modified'])?><br/>
				</p>
			<?php
		}
		return true;
	}//end paintFileUpload()


	/**
	* Processes the interface for uploading a file
	*
	* @param object Image	&$asset	the file asset
	* @param string			$prefix	prefix for form elements
	* @param array(string)	$info	info about the uploaded file
	*
	* @return boolean
	* @access public
	*/
	function processFileUpload(&$asset, $prefix, $info=Array())
	{
		if (empty($info)) {
			$info = get_file_upload_info($prefix);

			// return on failed or no upload
			if ($info === false || empty($info)) return false;
			if (!$info['name'] || !is_uploaded_file($info['tmp_name'])) return false;
		}

		// check that the extension is allowed
		$ext = strtolower(substr(strrchr($info['name'],'.'), 1));
		if (!in_array($ext, $asset->allowed_extensions)) {
			trigger_error('Uploaded file was not an image, You may only upload files of type '.$this->_getExtensionString($asset->allowed_extensions), E_USER_WARNING);
			return false;
		}

		if (!parent::processFileUpload($asset, $prefix, $info)) return false;

		return true;
	}//end processFileUpload()

}//end class
?>
