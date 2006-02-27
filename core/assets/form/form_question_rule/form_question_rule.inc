<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: form_question_rule.inc,v 1.2 2006/02/27 05:19:26 bcaldwell Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Form_Question_Rule
*
* Purpose
*
*
* @static
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Rule extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form_Question_Rule($assetid=0)
	{
		$this->Asset($assetid);

	}//end constructor


	/**
	* return true if this rule can be used for form validation
	*
	* @return boolean
	* @access public
	*/
	function isValidationRule()
	{
		return TRUE;

	}//end isValidationRule()


	/**
	* return true if this rule can be used for processing selective emails
	*
	* @return boolean
	* @access public
	*/
	function isSelectiveEmailRule()
	{
		return TRUE;

	}//end isSelectiveEmailRule()


	/**
	* Returns a list of operators for this rule type
	*
	* Returns an array of descriptions indexed by operator id (which can be
	* anything, generally string or (int)0|1 for a boolean value)
	*
	* @return array
	* @access public
	*/
	function getOperators()
	{
		return Array();

	}//end getOperators()


	/**
	* Processes the form question and returns true if it is valid
	*
	* You should overwrite this function
	*
	* @param string	$answer		The answer to the question
	* @param array	$rule_data	the rule data
	*
	* @return boolean
	* @access public
	*/
	function evaluate($answer, $rule_data)
	{
		return TRUE;

	}//end evaluate()


	/**
	* Writes JavaScript code for client side validation
	*
	* You should overwrite this function
	*
	* @param object	&$q_asset	reference to the question asset
	* @param array	$rule_data	the rule data
	*
	* @return boolean
	* @access public
	*/
	function generateJSCode(&$q_asset, $rule_data)
	{
		return '';

	}//end generateJSCode()


	/**
	* Returns a default error message for this rule
	*
	* You should overwrite this function
	*
	* @param object	&$q_asset	the question asset that failed
	* @param array	$rule_data	the rule data
	*
	* @return string
	* @access public
	*/
	function defaultError(&$q_asset, $rule_data)
	{
		return $this->ruleDescription($q_asset, $rule_data);

	}//end defaultError()


	/**
	* Returns a description of the rule
	*
	* You should overwrite this function
	*
	* @param object	&$q_asset	the question asset that failed
	* @param array	$rule_data	the rule data
	*
	* @return string
	* @access public
	*/
	function ruleDescription(&$q_asset, $rule_data)
	{
		return '';

	}//end ruleDescription()


}//end class

?>
