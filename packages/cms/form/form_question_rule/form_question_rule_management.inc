<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: form_question_rule_management.inc,v 1.3 2003/09/26 05:26:41 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Question_Rule_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
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
