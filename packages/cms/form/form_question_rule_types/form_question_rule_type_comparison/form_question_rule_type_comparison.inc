<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: form_question_rule_type_comparison.inc,v 1.26 2005/05/11 23:12:50 lwright Exp $
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
* @version $Revision: 1.26 $
* @package MySource_Matrix_Packages
* @subpackage cms
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
						'=='	=> translate('cms_form_rule_comparison_=='),
						'!='	=> translate('cms_form_rule_comparison_!='),
						'>'		=> translate('cms_form_rule_comparison_>'),
						'<'		=> translate('cms_form_rule_comparison_<'),
						'>='	=> translate('cms_form_rule_comparison_>='),
						'<='	=> translate('cms_form_rule_comparison_<='),
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
		$operator = array_get_index($rule_data, 'operator', 1);
		$cq_id = array_get_index($rule_data, 'comparison_question_id', 0);
		$value = array_get_index($rule_data, 'value', '');

		ob_start();			// buffer this please

			if ($cq_id != 0) {
				$comparison = 'document.getElementById("'.$cq_id.'").value';
			} else {
				$comparison = '"'.addslashes($value).'"';
			}

			// if it's a numeric field, make sure that JavaScript
			// interprets it as a number
			if (is_a($q_asset, 'form_question_type_numeric')) {	?>
				if (!(parseFloat(document.getElementById("<?php echo $q_asset->id; ?>").value) <?php echo $operator ?> parseFloat(<?php echo $comparison ?>))) {
			<?php } else { ?>
				if (!(document.getElementById("<?php echo $q_asset->id; ?>").value <?php echo $operator ?> <?php echo $comparison ?>)) {
			<?php } ?>
					submission_errors[submission_errors.length] = "<?php
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
	* @param mixed	$answer			the answer being passed to us
	* @param string	$rule_data		the rule's data
	*
	* @access public
	* @return boolean true only if rule succeeds
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
	* @param object Form_Question	&$q_asset	the question asset that failed
	* @param array					$rule_data	the rule data
	*
	* @return string	the error message
	* @access public
	*/
	function ruleDescription(&$q_asset, $rule_data)
	{
		$operator = array_get_index($rule_data, 'operator', '==');
		$cq_id = array_get_index($rule_data, 'comparison_question_id', 0);
		$value = array_get_index($rule_data, 'value', '');

		if (($value == '') && ($operator == '!=')) {
			$error_str = translate('cms_form_rule_comparison_required', $q_asset->attr('name'));
		} else {
			$operator_list = $this->getOperators();
			$string_code = 'cms_form_rule_comparison_desc_'.$operator;

			if ($cq_id != 0) {
				$string_code .= '_q';
				$comparison =& $GLOBALS['SQ_SYSTEM']->am->getAsset($cq_id);
				$value = $comparison->name;
			} else {	// sent just a value
				$value = array_get_index($rule_data, 'value', '');
			}

			$error_str = translate($string_code, $q_asset->name, $value).' ('.translate('cms_form_'.($case_sensitive ? '' : 'not_').'case_sensitive').')';
		}

		return $error_str;

	}//end ruleDescription()


}//end class
?>
