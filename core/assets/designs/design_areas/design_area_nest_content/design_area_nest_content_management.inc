<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Nest_Content_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class Design_Area_Nest_Content_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Design_Area_Nest_Content_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
							'type_codes' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'serialise',
								'default'		=> Array(),
								'description'   => 'Comma or space separated list of type codes to restrict the flash menu to display, these types are not strict however so any sub-types of these will also be valid'
							),
						);

	}//end constructor

}//end class
?>
