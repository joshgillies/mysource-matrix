<?php

require_once dirname(__FILE__).'/../../question_rule/question_rule_management.inc';

/**
* Question_Rule_Type_Email_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class Question_Rule_Type_Email_Management extends Question_Rule_Management
{

	/**
	* Constructor
	*
	*/
	function Question_Rule_Type_Email_Management(&$pm)
	{
		$this->Question_Rule_Management($pm);

		$this->vars = Array(	'operator'	=>	Array(
												'added'     => '0.0.1',
												'type'      => 'text',  
												'default'   => '',
												),
							);
	}//end constructor

}//end class
?>
