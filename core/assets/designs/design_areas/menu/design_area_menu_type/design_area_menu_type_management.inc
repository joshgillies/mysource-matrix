<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Menu_Type_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class Design_Area_Menu_Type_Management extends Asset_Management
{
	/**
	* Constructor
	*
	*/
	function Design_Area_Menu_Type_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
							'level' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'int',
								'default'		=> 0
							),
							'settings' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'serialise',
								'default'		=> Array()

							),
							'type_codes' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'serialise',
								'default'		=> Array(),
								'description'   => 'Comma or space separated list of type codes to restrict the menu to display, these types are not strict however so any sub-types of these will also be valid'
							),
						);

	}//end constructor


}//end class
?>
