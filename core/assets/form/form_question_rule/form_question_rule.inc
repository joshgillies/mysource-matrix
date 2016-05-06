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
* $Id: form_question_rule.inc,v 1.9 2013/04/22 03:31:29 ewang Exp $
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
* @version $Revision: 1.9 $
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


	/**
	* Returns a list of operators available for a validation condition
	*
	* @return array
	* @access public
	*/
	public static function getConditionOperators()
	{
		return Array(
				'='	 => translate('matches a regex'),

				'!=' => translate('does not match a regex'),

				'>'	 => translate('is greater than'),

				'<'	 => translate('is less than'),

			   );

	}//end getOperators()


	/**
	* Returns whehter validation needs to be executed or not
	*
	* If the validation condition is set, the validation should not be executed when the condition doesn't match.
	* This function checks if the condition to run the validation is met or not.
	*
	* @param array	$rule_data	the rule data
	*
	* @return boolean
	* @access public
	*/
	public static function isValidationRequired($rule_data) {
		$result = TRUE;

		// if the target question asset to evaluate the condition is not set, the validation is for all cases without a condition
		// and need to run the validation
		if (!isset($rule_data['condition_asset']) || empty($rule_data['condition_asset'])) return $result;
		$target_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($rule_data['condition_asset']);
		if(empty($target_asset))  return $result;

		// get the data we need to evaluate the condition.
		// not using getValue() here because it could cause infinate loop
		$target_value = $target_asset->value;
		if(empty($target_value)) $target_value = '';
		
		$condition_operator = isset($rule_data['condition_operator']) ? $rule_data['condition_operator'] : NULL;
		$condition_value = isset($rule_data['condition_value']) ? $rule_data['condition_value'] : NULL;
		if (is_null($condition_operator) || is_null($condition_value) || is_null($target_value)) return $result;

		// evaluate the condition to see if we need to run the validation
		switch ($condition_operator) {
			case '=':
				$result = preg_match('/'.$condition_value.'/', $target_value);
				break;
			case '!=':
				$result = !preg_match('/'.$condition_value.'/', $target_value);
				break;
			case '>':
				$result = $target_value > $condition_value;
				break;
			case '<':
				$result = $target_value < $condition_value;
				break;
			default:
				$result = TRUE;
		}

		return $result;

	}


	/**
	* Writes JavaScript code for the validation condition part for client side validation
	*
	* The validation condition allows users to set a condition depending on the answer to another question.
	* For example, if the submitted answer to Question1 is 'a', then the answer to Question2 needs to be either '1' or '2'.
	* If such a validation condition is set, this function returns the JavaScript for determining if the condition has been
	* met or not.
	*
	* @param array	$rule_data	the rule data
	*
	* @return boolean
	* @access public
	*/
	function generateConditionJSCode($rule_data)
	{
		$condition_asset    = isset($rule_data['condition_asset']) ? $rule_data['condition_asset'] : NULL;
		$condition_operator = isset($rule_data['condition_operator']) ? $rule_data['condition_operator'] : NULL;
		$condition_value    = isset($rule_data['condition_value']) ? $rule_data['condition_value'] : NULL;

		if (!$condition_asset) return '';

		$form_element = 'form.elements["q'.$condition_asset.'"].value';

		switch ($condition_operator) {
			case '=':
				$js_condition = $form_element.'.match(/'.$condition_value.'/)';
				break;
			case '!=':
				$js_condition = '!'.$form_element.'.match(/'.$condition_value.'/)';
				break;
			case '>':
				$js_condition = $form_element.' > '.$condition_value;
				break;
			case '<':
				$js_condition = $form_element.' < '.$condition_value;
				break;
			default:
				$js_condition = '';
		}

		return $js_condition;

	}//end generateJSCode()

}//end class

?>
