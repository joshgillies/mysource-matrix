<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: form_question_type_password_management.inc,v 1.1 2003/09/28 22:57:41 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../form_question/form_question_management.inc';

/**
* Form_Question_Type_Password_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Type_Password_Management extends Form_Question_Management
{

	/**
	* Constructor
	*
	*/
	function Form_Question_Type_Password_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
							
							'default'	=> Array(
									'added'		=> '0.0.1',
									'type'		=> 'text',
									'default'	=> '',
								),
							'width' => Array(
									'added'     => '0.0.1',
									'type'      => 'text',  
									'default'   => '30',
									),
							'verify'=> Array(
									'added'		=> '0.0.1',
									'type'		=> 'boolean',
									'default'	=> true,
							),
							'max' => Array(
									'added'     => '0.0.1',
									'type'      => 'text',  
									'default'   => '30',
							),
							'min' => Array(
									'added'     => '0.0.1',
									'type'      => 'text',  
									'default'   => '2',
							),
							'extra'	=> Array(
									'added'     => '0.0.1',
									'type'      => 'text',  
									'default'   => '',
							),
								
			);
	}//end constructor

}//end class
?>
