<?php

require_once SQ_CORE_PACKAGE_PATH.'/files/file/file.inc';

/**
* Image
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Image extends File
{

	/**
	* All allowed extensions for an image
	* @var Array(string)
	*/
	var $allowed_extensions = Array('gif', 'jpg', 'jpeg', 'png');


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Image($assetid=0)
	{
		$this->File($assetid);

	}//end constructor


	/**
	* Checks a file on the file system to see if it can be used as an image
	*
	* @param string	$file_path	the path of the file we are checking
	*
	* @return boolean
	* @access public
	*/
	function validFile($file_path)
	{
		if (!parent::validFile($file_path)) return false;

		// check that the extension is allowed
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$ext = get_file_type($file_path);
		if (!in_array($ext, $this->allowed_extensions)) {
			$edit_fns = $this->getEditFns();
			trigger_error('Uploaded file is not an image. Image files must be of type '.$edit_fns->_getExtensionString($this->allowed_extensions, 'or'), E_USER_WARNING);
			return false;
		}
		return true;

	}//end validFile()


	/**
	* Get an array of some useful data about the existing uploaded file
	*
	* @return array(string)
	* @access public
	*/
	function _getExistingFile()
	{
		$existing = parent::_getExistingFile();
		if (!empty($path)) {
			$size = getimagesize($existing['path']);
			$existing['width']  = $size[0];
			$existing['height'] = $size[1];
		}
		return $existing;

	}//end _getExistingFile()




}//end class
?>
