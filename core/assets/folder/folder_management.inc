<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Folder_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class Folder_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Folder_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array('name' => Array(
												'added'     => '0.1',
												'type'      => 'text', 'default'   => ''
												)
							);
	}//end constructor

}//end class
?>
