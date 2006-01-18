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
* $Id: thumbnail.inc,v 1.21 2006/01/18 00:59:32 lwright Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/files/image/image.inc';

/**
* Thumbnail
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.21 $
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
	* Checks a file on the file system to see if it can be used as a thumbnail
	*
	* @param array	$info	an array of information about the uploaded file
	*
	* @return boolean
	* @access public
	*/
	function validFile($info)
	{
		if (!parent::validFile($info)) return FALSE;

		return TRUE;

	}//end validFile()


	/**
	* Returns TRUE if the image is not bigger than the allowed dimension
	*
	* @param array	$info	an array of information about the uploaded file
	*
	* @return boolean
	* @access private
	*/
	function _checkImageSize($info)
	{
		$size = @getimagesize($info['filename']);
		if (!empty($size) && ($size[0] > $this->max_width || $size[1] > $this->max_height)) {
			trigger_localised_error('CORE0108', E_USER_WARNING, $this->max_width, $this->max_height);
			return FALSE;
		}

		return TRUE;

	}//end _checkImageSize()


}//end class

?>
