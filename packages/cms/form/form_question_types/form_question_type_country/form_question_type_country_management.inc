<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: form_question_type_country_management.inc,v 1.2 2003/10/13 05:25:12 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../form_question/form_question_management.inc';

/**
* Form_Question_Type_Country_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Type_Country_Management extends Form_Question_Management
{

	/**
	* Constructor
	*
	*/
	function Form_Question_Type_Country_Management(&$pm)
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
							'extra'	=> Array(
									'added'     => '0.0.1',
									'type'      => 'text',  
									'default'   => '',
									),
							'default' => Array(
										'added'		=> '0.0.1',
										'type'		=> 'serialise',
										'default'	=> Array(),
										'update_fn' => '',
									),
							'available' => Array(
										'added'		=> '0.0.1',
										'type'		=> 'serialise',
										'default'	=> Array(),
										'update_fn' => '',
									),
					);
	}//end constructor

} // end class
?>
