<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Menu_Type_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
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
								'added'     => '0.0.1',
								'type'      => 'text',
								'default'   => ''
							),
							'settings' => Array(
								'added'     => '0.0.1',
								'type'      => 'array',
								'default'   => Array()

							),
						);

	}//end constructor


}//end class
?>
