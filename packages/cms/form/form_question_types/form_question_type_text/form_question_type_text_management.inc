<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: form_question_type_text_management.inc,v 1.4 2003/09/26 05:26:41 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../form_question/form_question_management.inc';

/**
* Form_Question_Type_Text_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Type_Text_Management extends Form_Question_Management
{

	/**
	* Constructor
	*
	*/
	function Form_Question_Type_Text_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
							'width' => Array(
									'added'     => '0.0.1',
									'type'      => 'text',  
									'default'   => '30',
									),
							'height' => Array(
									'added'     => '0.0.1',
									'type'      => 'text',  
									'default'   => '1',
									),
							'max' => Array(
									'added'     => '0.0.1',
									'type'      => 'text',  
									'default'   => '300',
									),
							'extra'			=> Array(
									'added'     => '0.0.1',
									'type'      => 'text',  
									'default'   => '',
									),
								
			);
	}//end constructor

}//end class
?>
