<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: thumbnail.inc,v 1.23 2006/12/18 05:18:02 arailean Exp $
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
* @version $Revision: 1.23 $
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
