<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: image_management.inc,v 1.4 2003/09/26 05:26:26 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Image_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Image_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Image_Management(&$pm)
	{
		$this->Asset_Management($pm);
	}//end constructor

}//end class
?>
