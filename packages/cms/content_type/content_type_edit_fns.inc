<?php

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Content_Type_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
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
													  )
								);


	/**
	* Constructor
	*
	*/
	function Content_Type_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
	}


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
	}

}//end class
?>
