<?php

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
