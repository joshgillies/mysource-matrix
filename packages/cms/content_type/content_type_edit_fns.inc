<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: content_type_edit_fns.inc,v 1.15 2003/09/26 05:26:40 brobertson Exp $
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
	* An array of tab code names and their display names
	* @var Array()
	*/
	var $static_screens = Array('details'     => Array('name' => 'Details',
													   'force_unlock' => true
													  ),
								'dependants'  => Array('name' => 'Dependants',
													   'force_unlock' => true
													  ),
								'logs'        => Array('name' => 'Logs',
													   'force_unlock' => true
													  ),
								);


	/**
	* Constructor
	*
	*/
	function Content_Type_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

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