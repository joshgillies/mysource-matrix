<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: content_type_edit_fns.inc,v 1.16 2003/10/16 01:50:00 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Content_Type_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Content_Type_Edit_Fns extends Asset_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Content_Type_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

		unset($this->static_screens['permissions']);
		unset($this->static_screens['workflow']);
		unset($this->static_screens['metadataSchemas']);
		unset($this->static_screens['metadata']);
		unset($this->static_screens['linking']);
		unset($this->static_screens['settings']);
		unset($this->static_screens['history']);

	}//end constructor


	/**
	* Paints raw HTML to be printed on the frontend
	*
	* @param object Content_Type	$asset	the content type we are painting
	*
	* @return void
	* @access public
	*/
	function paint(&$asset, $generating=false)
	{

	}//end paint()


	/**
	* Prints any icons the content type needs next to the cell properties
	*
	* @param object Content_Type	&$asset	the content type object
	* @param string					$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintBackendIcons(&$asset, $prefix)
	{

	}//end paintBackendIcons()


}//end class

?>