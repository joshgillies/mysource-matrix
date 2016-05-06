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
* $Id: form_question_rule_type_comparison.inc,v 1.13 2013/04/19 06:36:00 ewang Exp $
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
* @version $Revision: 1.13 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Rule_Type_Comparison extends Form_Question_Rule
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
	* Returns a list of operators for this rule type
	*
	* @access public
	* @return array (mixed => string) an array of descriptions indexed by operator id (which can be
	*                                 anything, generally string or (int)0|1 for a boolean value)
	*/
	public static function getOperators()
	{
		$operators = Array(
						'=='	=> translate('Must Be Equal To'),

						'!='	=> translate('Must NOT Be Equal To'),

						'>'		=> translate('Must Be Greater Than'),

						'<'		=> translate('Must Be Less Than'),

						'>='	=> translate('Must Be Greater Than Or Equal To'),

						'<='	=> translate('Must Be Less Than Or Equal To'),

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

		// if the validation condition is set, we need to output the part that checks whether the condition is met or not as well
		$condition_js = parent::generateConditionJSCode($rule_data);
		if (strlen(trim($condition_js))) $condition_js = ' && '.$condition_js;

		ob_start();			// buffer this please

			if ($cq_id != 0) {
				$comparison = 'form.elements["q'.$cq_id.'"].value';
			} else {
				$comparison = '"'.addslashes($value).'"';
			}

			// if it's a numeric field, make sure that JavaScript
			// interprets it as a number
			if ($q_asset instanceof Form_Question_Type_Numeric) {
				?>if ((<?php echo $comparison ?>.length && !(parseFloat(form.elements["q<?php echo $q_asset->id; ?>"].value) <?php echo $operator ?> parseFloat(<?php echo $comparison ?>))<?php echo $condition_js; ?>) || (!(form.elements["q<?php echo $q_asset->id; ?>"].value <?php echo $operator ?> <?php echo $comparison ?>)<?php echo $condition_js; ?>)) {<?php
			} else {
				?>if (!(form.elements["q<?php echo $q_asset->id; ?>"].value <?php echo $operator ?> <?php echo $comparison ?>)<?php echo $condition_js; ?>) {<?php
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
			$comparison_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($cq_id);
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
	    $errors = Array(
                    '==_q'            => translate('The response to "%1$s" must be equal to the response to question "%2$s"'),
                    '=='              => translate('The response to "%1$s" must be equal to "%2$s"'),
                    '!=_q'            => translate('The response to "%1$s" must NOT be equal to the response to question "%2$s"'),
                    '!='              => translate('The response to "%1$s" must NOT be equal to "%2$s"'),
                    '>_q'             => translate('The response to "%1$s" must be greater than the response to question "%2$s"'),
                    '>'               => translate('The response to "%1$s" must be greater than "%2$s"'),
                    '<_q'             => translate('The response to "%1$s" must be less than the response to question "%2$s"'),
                    '<'               => translate('The response to "%1$s" must be less than "%2$s"'),
                    '>=_q'            => translate('The response to "%1$s" must be greater than or equal to the response to question "%2$s"'),
                    '>='              => translate('The response to "%1$s" must be greater than or equal to "%2$s"'),
                    '<=_q'            => translate('The response to "%1$s" must be less than or equal to the response to question "%2$s"'),
                    '<='              => translate('The response to "%1$s" must be less than or equal to "%2$s"'),
                    '==_accessible'   => translate('The response must be equal to "%2$s".'),
                    '==_q_accessible' => translate('The response must be equal to the response to question "%1$s".'),
                    '!=_accessible'   => translate('The response must NOT be equal to "%2$s".'),
                    '!=_q_accessible' => translate('The response must NOT be equal to the response to question "%1$s".'),
                    '>_accessible'    => translate('The response must be greater than "%2$s".'),
                    '>_q_accessible'  => translate('The response must be greater than the response to question "%1$s".'),
                    '<_accessible'    => translate('The response must be less than "%2$s".'),
                    '<_q_accessible'  => translate('The response must be less than the response to question "%1$s".'),
                    '>=_accessible'   => translate('The response must be greater than or equal to "%2$s".'),
                    '>=_q_accessible' => translate('The response must be greater than or equal to the response to question "%1$s".'),
                    '<=_accessible'   => translate('The response must be less than or equal to "%2$s".'),
                    '<=_q_accessible' => translate('The response must be less than or equal to question "%1$s".'),
        );

		$operator = array_get_index($rule_data, 'operator', '==');
		$cq_id = array_get_index($rule_data, 'comparison_question_id', 0);
		$value = array_get_index($rule_data, 'value', '');
		$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($q_asset->_formid);
		$use_accessible_string = $form->useAccessibleFormat();

		if (($value == '') && ($operator == '!=')) {
			if (strlen($q_asset->attr('cust_required_error'))) {
				$error_str = $q_asset->attr('cust_required_error');
			} else {
				if($use_accessible_string)
				    $error_str = translate('This field is required. Please enter a value.');

				 else
				    $error_str = sprintf(translate('Question "%s" is a required field; it must be filled in'), $q_asset->attr('name'));

			}
		} else {
			$operator_list = $this->getOperators();
			$string_code = $operator;

			if ($cq_id != 0) {
				$string_code .= '_q';
				$comparison = $GLOBALS['SQ_SYSTEM']->am->getAsset($cq_id);
				$value = $comparison->name;
			} else {	// sent just a value
				$value = array_get_index($rule_data, 'value', '');
			}

			if ($use_accessible_string) {
			    $string_code .= '_accessible';
			}

			$error_str = sprintf($errors[$string_code], $q_asset->name, $value);
		}//end if

		return $error_str;

	}//end ruleDescription()


}//end class
?>
