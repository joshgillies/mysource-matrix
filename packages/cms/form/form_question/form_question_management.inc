<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Question_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class Form_Question_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Form_Question_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array('name' => Array(
												'added'     => '0.0.1',
												'type'      => 'text',  
												'default'   => '',
												),
							'note' => Array(
												'added'     => '0.0.1',
												'type'      => 'text',  
												'default'   => '',
												),
							'special_info'	=> Array(
														'added'		=> '0.0.1',
														'type'		=> 'selection',
														'default'	=> '',
														'update_fn'	=> '',
														'parameters'	=> Array(	
														'options'	=> Array(
															''			=> '--- No Special Information ---',
															'name'		=> 'User\'s Name',
															'firstname'	=> 'User\'s Firstname',
															'surname'	=> 'User\'s Surname',
															'email'		=> 'User\'s Email',
															'friend'	=> 'Send a note to these Friends\' Email Addresses',
														),
												),		
											),
									);
	}//end constructor

}//end class
?>
