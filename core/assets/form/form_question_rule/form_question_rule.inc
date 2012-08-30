<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: form_question_rule.inc,v 1.6 2012/08/30 01:09:07 ewang Exp $
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
* @version $Revision: 1.6 $
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
	function __construct($assetid=0)
	{
		$this->Asset($assetid);

	}//end constructor


	/**
	* return true if this rule can be used for form validation
	*
	* @return boolean
	* @access public
	*/
	public static function isValidationRule()
	{
		return TRUE;

	}//end isValidationRule()


	/**
	* return true if this rule can be used for processing selective emails
	*
	* @return boolean
	* @access public
	*/
	public static function isSelectiveEmailRule()
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
	public static function getOperators()
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
