<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Declared_Vars_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
*/
class Design_Area_Declared_Vars_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Design_Area_Declared_Vars_Management(&$pm)
	{
		$this->Asset_Management(&$pm);

		$this->vars = Array(
							'id_name'   => Array(
								'default'   => 'Design Name'
							),
							'declared_vars' => Array(
								'added'     => '0.0.1',
								'type'      => 'text',
								'default'   => ''
							),
						);

	}//end constructor

}//end class
?>
