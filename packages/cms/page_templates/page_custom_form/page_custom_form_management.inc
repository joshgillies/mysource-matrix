<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* User_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class Page_Custom_Form_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Page_Custom_Form_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array('name' => Array (
												'added'		=> '0.0.1',
												'type'		=> 'text',
												'default'	=> '',
										), // end name
							
						);
	}//end constructor

}//end class
?>
