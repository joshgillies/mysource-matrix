<?php
/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
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
* $Id: thumbnail.inc,v 1.12 2003/11/18 15:37:26 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/files/image/image.inc';

/**
* Thumbnail
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Thumbnail extends Image
{

	/**
	* The maxium width a thumbnail can have (in pixels)
	* @var int
	*/
	var $max_width  = 100;

	/**
	* The maxium height a thumbnail can have (in pixels)
	* @var int
	*/
	var $max_height = 100;


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Thumbnail($assetid=0)
	{
		$this->Image($assetid);
	}//end constructor


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowedLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(SQ_LINK_TYPE_1 => Array(),
					 SQ_LINK_TYPE_2 => Array(),
					 SQ_LINK_TYPE_3 => Array(),
					 SQ_LINK_NOTICE => Array()
					);

	}//end _getAllowedLinks()


	/**
	* Checks a file on the file system to see if it can be used as a thumbnail
	*
	* @param string	$file_path	the path of the file we are checking
	*
	* @return boolean
	* @access public
	*/
	function validFile($file_path)
	{
		if (!parent::validFile($file_path)) return false;

		// check that the image dimensions are not greater than our limits
		$size = @getimagesize($file_path);
		if (!empty($size) && ($size[0] > $this->max_width || $size[1] > $this->max_height)) {
			trigger_error('Uploaded file is too large. Thumbnail files must be no more than '.$this->max_width.' x '.$this->max_height.' pixels', E_USER_WARNING);
			return false;
		}
		return true;

	}//end validFile()


}//end class
?>
