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
* $Id: image.inc,v 1.15 2004/07/23 07:09:33 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/


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
	* @access private
	*/
	function _getExistingFile()
	{
		$existing = parent::_getExistingFile();
		if (!empty($existing['path'])) {
			$size = getimagesize($existing['path']);
			$existing['width']  = $size[0];
			$existing['height'] = $size[1];
		}
		return $existing;

	}//end _getExistingFile()


	/**
	* Prints the tag for an image
	*
	* The output includes the width and height of the image, and the image 
	* name for the alt tag
	*
	* @param string 	$alt 	the alt tag content. If empty the filename is used
	* @param boolean 	$return if TRUE the tag will be returned
	*
	* @return string | void
	* @access public
	*/
	function printImageTag($alt='', $return=false)
	{
		if (empty($alt)) $alt = $this->name;
		$info = $this->_getExistingFile();
		$tag = '<img src="'.$this->getURL().'" width="'.$info['width'].'" height="'.$info['height'].'" alt="'.$alt.'" title="'.$alt.'" />';
		if ($return) return $tag;
		echo $tag;

	}//end printImageTag()


}//end class

?>
