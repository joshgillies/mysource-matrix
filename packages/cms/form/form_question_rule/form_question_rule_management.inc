<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Question_Rule_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class Form_Question_Rule_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Form_Question_Rule_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array('operator'	=>	Array(
												'added'     => '0.0.1',
												'type'      => 'text',  
												'default'   => '',
												),
							'value'		=>	Array(
												'added'		=> '0.0.1',
												'type'		=> 'text',
												'default'	=> '',
							),
									);
	}//end constructor

}//end class
?>
