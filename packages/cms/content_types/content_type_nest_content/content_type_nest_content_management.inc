<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Content_Type_Nest_Content_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage cms
*/
class Content_Type_Nest_Content_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Content_Type_Nest_Content_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array('nested_assetid' => Array(
											'added'     => '0.0.1',
											'type'      => 'int',  'default'   => '',
											'update_fn' => ''
											)
							);

	}//end constructor

}//end class
?>
