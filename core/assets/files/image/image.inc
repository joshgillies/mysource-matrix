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
* $Id: image.inc,v 1.18 2005/01/06 17:05:22 brobertson Exp $
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
	* Set the value of an attribute
	*
	* @param boolean	$dont_run_updated	if true this->_updated() won't be run
	*										(THIS IS ONLY EVER CALLED FROM Asset::create())
	*
	* @return boolean
	* @access public
	*/
	function saveAttributes($dont_run_updated=false)
	{
		if (!$this->id) return true;

		// if there are no values for the alt or title use the filename
		if (isset($this->_tmp['vars_set']['alt']) && trim($this->attr('alt')) == '') {
			$this->setAttrValue('alt', $this->attr('name'));
		}//end if
		if (isset($this->_tmp['vars_set']['title']) && trim($this->attr('title')) == '') {
			$this->setAttrValue('title', $this->attr('name'));
		}//end if

		return parent::saveAttributes($dont_run_updated);

	}//end saveAttributes()


	/**
	* Set the value for the passed variable to the passed value
	* If the user changes the name of this file, we also need
	* to update the file system
	*
	* @param string		$name				the name of the attribute
	* @param string		$value				the new value of the attribute
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value)
	{
		if ($name == 'alt' || $name == 'title') {
			$value = htmlspecialchars($value);
		}

		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


	/**
	* Checks a file on the file system to see if it can be used as an image
	*
	* @param array	$info	an array of information about the uploaded file
	*
	* @return boolean
	* @access public
	*/
	function validFile($info)
	{
		if (!parent::validFile($info)) return false;

		// check that the extension is allowed
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$ext = get_file_type($info['name']);
		if (!in_array($ext, $this->allowed_extensions)) {
			$edit_fns = $this->getEditFns();
			trigger_error('Uploaded file is not an image. Image files must be of type '.$edit_fns->_getExtensionString($this->allowed_extensions, 'or').'. The uploaded file is of type "'.strtoupper($ext).'"', E_USER_WARNING);
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
		if (!isset($this->_tmp['existing_file'])) {
			$this->_tmp['existing_file'] = parent::_getExistingFile();
			if (!empty($this->_tmp['existing_file']['path'])) {
				$size = getimagesize($this->_tmp['existing_file']['path']);
				$this->_tmp['existing_file']['width']  = $size[0];
				$this->_tmp['existing_file']['height'] = $size[1];
			}
		}

		return $this->_tmp['existing_file'];

	}//end _getExistingFile()


	/**
	* Prints the tag for an image
	*
	* The output includes the width and height of the image, and the image
	* name for the alt tag
	*
	* @param string 	$alt	the alt tag content. If empty the alt var is used
	* @param string 	$title	the alt tag content. If empty the and $alt is set, then that is used, otherwise the title var is used
	* @param boolean 	$return	if TRUE the tag will be returned
	*
	* @return string | void
	* @access public
	*/
	function printImageTag($alt='', $title='', $return=false)
	{
		if ($title == '') {
			$title = ($alt != '') ? $alt : $this->atr('title');
		}
		if ($alt == '') $alt = $this->attr('alt');

		$tag = '<img src="'.$this->getURL().'" width="'.$this->attr('width').'" height="'.$this->attr('height').'" alt="'.$alt.'" title="'.$title.'" />';
		if ($return) return $tag;
		echo $tag;

	}//end printImageTag()


	/**
	* Return the keywords with their replacements
	*
	* The return value is in the form:
	* <pre>
	* Array(
	*     'name' => Array (
	*          'value' => 'page',
	*          'description' => 'the asset name',
	*     ),
	* )
	* </pre>
	*
	* @param boolean $descriptions if TRUE descriptions will be included in the array
	*
	* @access public
	* @return Array(mixed)
	*/
	function getAssetKeywords($descriptions=false)
	{
		$keywords = parent::getAssetKeywords($descriptions);

		$existing = $this->_getExistingFile();
		if (!isset($existing['height'])) $existing['height'] = '0';
		if (!isset($existing['width'])) $existing['width'] = '0';
		$keywords['image_width'] = Array('value' => $existing['width']);
		$keywords['image_height'] = Array('value' => $existing['height']);

		if ($descriptions) {
			$keywords['image_width']['description'] = 'Width of the image in pixels';
			$keywords['image_height']['description'] = 'Height of the image in pixels';
		}

		return $keywords;

	}//end getAssetKeywords()


}//end class

?>
