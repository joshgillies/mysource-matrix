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
* $Id: form_question_rule_type_selection.inc,v 1.7 2013/04/08 07:52:53 ewang Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule.inc';

/**
* Form_Question_Rule_Type_Selection
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Rule_Type_Selection extends Form_Question_Rule
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* return true if this rule can be used for form validation
	*
	* @return boolean
	* @access public
	*/
	public static function isValidationRule()
	{
		return false;

	}//end isValidationRule()


	/**
	* Returns a list of operators for this rule type
	*
	* @access public
	* @return array (mixed => string) an array of descriptions indexed by operator id (which can be
	*                                 anything, generally string or (int)0|1 for a boolean value)
	*/
	public static function getOperators()
	{
		$operators = Array(
						1	=> translate('Option must be selected'),

						0	=> translate('Option must NOT be selected'),

					 );

		return $operators;

	}//end getOperators()


	/**
	* writes JavaScript code for client side validation
	*
	* This function assumes the skeleton of a JS validation function has been created around this code
	* with a form variable named 'form' passed (this is done when generating the content file).
	*
	* @param object Form_Question	&$q_asset		reference to the question asset
	* @param Array					$rule_data		the rule data being used
	*
	* @access public
	* @return boolean
	*/
	function generateJSCode(&$q_asset, $rule_data)
	{
		// this isn't used as a validation rule, so no JS code is required
		return '';

	}//end generateJSCode()


	/**
	* processes a test on a single static value
	*
	* Rule data is an array with the following fields:
	* [operator] => operator to compare with (string, ==, !=, <, <=, >, >=)
	* [value] => comparison static value
	*
	* @param mixed	$answer			the answer being passed to us
	* @param string	$rule_data		the rule's data
	*
	* @access public
	* @return boolean true only if rule succeeds
	*/
	function evaluate($answer, $rule_data)
	{
		$operator = array_get_index($rule_data, 'operator', 1);
		$value = array_get_index($rule_data, 'value', '');

		if (!is_array($answer)) {	// not an array, could be selection type
			$answer = Array($answer);
		} else if (count($answer) == 0) {	// no selections made at all
			$answer = Array();
		}

		$valid = (array_search($value, $answer) !== false);

		return ($valid == $operator);

	}//end evaluate()


	/**
	* returns a description of this rule
	*
	* @param object Form_Question	&$q_asset		reference to the question asset
	* @param Array					$rule_data		the rule data being used
	*
	* @access public
	* @return boolean true only if rule succeeds
	*/
	function ruleDescription(&$q_asset, $rule_data)
	{
		$operator = (string) array_get_index($rule_data, 'operator', '1');
		$value    = array_get_index($rule_data, 'value', '');

		$description_text = Array(
			'1'     => translate('Option "%1$s" must be selected at "%2$s"'),
			'1_acc' => translate('Option "%1$s" must be selected.'),
			'0'     => translate('Option "%1$s" must NOT be selected at "%2$s"'),
			'0_acc' => translate('Option "%1$s" must NOT be selected.'),
			
		);

		$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($q_asset->_formid);
	    $use_accessible_string = $form->useAccessibleFormat();

		if ($use_accessible_string) {
			$operator .= '_acc';
		}

		$error_str = sprintf(
			$description_text[$operator],
			$q_asset->getSummary($value),
			(!$use_accessible_string) ? $q_asset->attr('name') : ''
		);

		return $error_str;

	}//end ruleDescription()


}//end class
?>
