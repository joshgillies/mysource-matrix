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
* $Id: form_question_rule_type_comparison.inc,v 1.6 2006/08/14 07:19:19 rhoward Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule.inc';

/**
* Form_Question_Rule_Type_Comparison
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Rule_Type_Comparison extends Form_Question_Rule
{


	/**
	* Returns a list of operators for this rule type
	*
	* @access public
	* @return array (mixed => string) an array of descriptions indexed by operator id (which can be
	*                                 anything, generally string or (int)0|1 for a boolean value)
	*/
	function getOperators()
	{
		$operators = Array(
						'=='	=> translate('core_form_rule_comparison_=='),
						'!='	=> translate('core_form_rule_comparison_!='),
						'>'		=> translate('core_form_rule_comparison_>'),
						'<'		=> translate('core_form_rule_comparison_<'),
						'>='	=> translate('core_form_rule_comparison_>='),
						'<='	=> translate('core_form_rule_comparison_<='),
					 );

		return $operators;

	}//end getOperators()


	/**
	* writes JavaScript code for client side validation
	*
	* This function assumes the skeleton of a JS validation function has been created around this code
	* with a form variable named 'form' passed (this is done when generating the content file).
	*
	* @param object	&$q_asset	reference to the question asset
	* @param array	$rule_data	the rule data being used
	*
	* @return boolean
	* @access public
	*/
	function generateJSCode(&$q_asset, $rule_data)
	{
		$operator = array_get_index($rule_data, 'operator', 1);
		$cq_id = array_get_index($rule_data, 'comparison_question_id', 0);
		$value = array_get_index($rule_data, 'value', '');

		ob_start();			// buffer this please

			$form =& $GLOBALS['SQ_SYSTEM']->am->getAsset($q_asset->_formid);
			?>
			form = document.getElementById("<?php echo $form->getPrefix(); ?>");
			<?php
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($form);

			if ($cq_id != 0) {
				$comparison = 'form.elements["q'.$cq_id.'"].value';
			} else {
				$comparison = '"'.addslashes($value).'"';
			}

			// if it's a numeric field, make sure that JavaScript
			// interprets it as a number
			if (is_a($q_asset, 'form_question_type_numeric')) {
				?>if (!(parseFloat(form.elements["q<?php echo $q_asset->id; ?>"].value) <?php echo $operator ?> parseFloat(<?php echo $comparison ?>))) {<?php
			} else {
				?>if (!(form.elements["q<?php echo $q_asset->id; ?>"].value <?php echo $operator ?> <?php echo $comparison ?>)) {<?php
			}
			?>submission_errors[submission_errors.length] = "<?php
						if (empty($rule_data['custom_text'])) {
							echo addslashes($this->defaultError($q_asset, $rule_data));
						} else {
							echo addslashes($rule_data['custom_text']);
						}
					?>";
				}
			<?php

		$contents = ob_get_contents();
		ob_end_clean();

		return $contents;

	}//end generateJSCode()


	/**
	* processes a test on a single static value
	*
	* Rule data is an array with the following fields:
	* [operator] => operator to compare with (string, ==, !=, <, <=, >, >=)
	* [value] => comparison static value
	* [comparison_question_id] => comparison question assetid
	* [case_sensitive] => whether the comparison is case sensitive
	*
	* @param mixed	$answer		the answer being passed to us
	* @param string	$rule_data	the rule's data
	*
	* @return boolean
	* @access public
	*/
	function evaluate($answer, $rule_data)
	{
		$operator = array_get_index($rule_data, 'operator', '==');
		$cq_id = array_get_index($rule_data, 'comparison_question_id', 0);
		$value = array_get_index($rule_data, 'value', '');

		if ($cq_id != 0) {
			$comparison_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($cq_id);
			$value = $comparison_asset->getValue();
		}

		switch ($operator) {
			case '==':
				$valid = ($answer == $value);
			break;

			case '!=':
				$valid = ($answer != $value);
			break;

			case '>':
				$valid = ($answer > $value);
			break;

			case '>=':
				$valid = ($answer >= $value);
			break;

			case '<':
				$valid = ($answer < $value);
			break;

			case '<=':
				$valid = ($answer <= $value);
			break;
		}

		return $valid;

	}//end evaluate()


	/**
	* returns a description of the rule
	*
	* @param object	&$q_asset	the question asset that failed
	* @param array	$rule_data	the rule data
	*
	* @return string
	* @access public
	*/
	function ruleDescription(&$q_asset, $rule_data)
	{
		$operator = array_get_index($rule_data, 'operator', '==');
		$cq_id = array_get_index($rule_data, 'comparison_question_id', 0);
		$value = array_get_index($rule_data, 'value', '');

		if (($value == '') && ($operator == '!=')) {
			if (strlen($q_asset->attr('cust_required_error'))) {
				$error_str = $q_asset->attr('cust_required_error');
			} else {
				$error_str = translate('core_form_rule_comparison_required', $q_asset->attr('name'));
			}
		} else {
			$operator_list = $this->getOperators();
			$string_code = 'core_form_rule_comparison_desc_'.$operator;

			if ($cq_id != 0) {
				$string_code .= '_q';
				$comparison =& $GLOBALS['SQ_SYSTEM']->am->getAsset($cq_id);
				$value = $comparison->name;
			} else {	// sent just a value
				$value = array_get_index($rule_data, 'value', '');
			}

			$error_str = translate($string_code, $q_asset->name, $value);
		}

		return $error_str;

	}//end ruleDescription()


}//end class
?>
