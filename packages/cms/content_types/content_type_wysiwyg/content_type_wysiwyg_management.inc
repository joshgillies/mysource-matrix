<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: content_type_wysiwyg_management.inc,v 1.7 2003/09/26 05:26:40 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Content_Type_WYSIWYG_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Content_Type_WYSIWYG_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Content_Type_WYSIWYG_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array();

	}//end constructor


}//end class
?>
