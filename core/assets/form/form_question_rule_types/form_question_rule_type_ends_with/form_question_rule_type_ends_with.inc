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
* $Id: form_question_rule_type_ends_with.inc,v 1.11 2013/04/19 06:36:00 ewang Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule.inc';

/**
* Form_Question_Rule_Type_Ends_With
*
* Purpose
*	Form Question Rule: Value Ends With...
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.11 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Rule_Type_Ends_With extends Form_Question_Rule
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
		return Array(
				1	=> translate('Must end with'),

				0	=> translate('Must NOT end with'),

			   );

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
	* @return string
	*/
	function generateJSCode(&$q_asset, $rule_data)
	{
		// have we printed out the validation function yet?
		// (since this is static across all class instances, this will work)
		static $printed_fn = false;

		$operator = array_get_index($rule_data, 'operator', 1);
		$cq_id = array_get_index($rule_data, 'comparison_question_id', 0);
		$value = array_get_index($rule_data, 'value', '');
		$case_sensitive = array_get_index($rule_data, 'case_sensitive', 1);

		// if the validation condition is set, we need to output the part that checks whether the condition is met or not as well
		$condition_js = parent::generateConditionJSCode($rule_data);
		$additional_condition = '';
		if (strlen(trim($condition_js))) {
			$additional_condition = 'if (!'.$condition_js.') return true;';
		}

		ob_start();			// buffer this please

			if (!$printed_fn) {
				$printed_fn = true;

					?>
					function sq_form_validate_ends_with(answer, comparison, case_sensitive)
					{
						<?php echo $additional_condition; ?>
						if (case_sensitive) {
							pos = answer.indexOf(comparison);
						} else {
							pos =  answer.toLowerCase().indexOf(comparison.toLowerCase());
						}

						// if position is less than zero than it can't be found,
						// therefore it can't end with it
						return (pos >= 0) && (pos == answer.length - comparison.length);
					}//end sq_form_validate_ends_with()
					<?php

			}

			if ($cq_id != 0) {
				$comparison = 'form.elements["q'.$cq_id.'"].value';
			} else {
				$comparison = '"'.addslashes($value).'"';
			}

			?>
				if (<?php echo ($operator == 1) ? '!' : ''; ?>sq_form_validate_ends_with(form.elements["q<?php echo $q_asset->id; ?>"].value, <?php echo $comparison ?>, <?php echo (int)$case_sensitive; ?>)) {
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
		$operator = array_get_index($rule_data, 'operator', 1);
		$cq_id = array_get_index($rule_data, 'comparison_question_id', 0);
		$value = array_get_index($rule_data, 'value', '');
		$case_sensitive = array_get_index($rule_data, 'case_sensitive', 1);

		if ($cq_id != 0) {
			$comparison = $GLOBALS['SQ_SYSTEM']->am->getAsset($cq_id);
			$value = $comparison->getValue();
		}

		if (!$case_sensitive) {
			$value = strtolower($value);
			$answer = strtolower($answer);
		}

		// we could use strrpos() if we were in PHP5, but since it's limited to
		// one character needles in PHP4, we need to do an iterative search

		$last_pos = -1;
		while (($pos = strpos($answer, $value, $last_pos + 1)) !== false) {
			$last_pos = $pos;
		}

		$valid = ($last_pos === strlen($answer)-strlen($value)) ? 1 : 0;
		return $valid == $operator;

	}//end evaluate()


	/**
	* returns a description of this rule
	*
	* @param object Form_Question	&$q_asset	the question asset that failed
	* @param array					$rule_data	the rule data
	*
	* @return string	the error message
	* @access public
	*/
	function ruleDescription(&$q_asset, $rule_data)
	{
	    $errors = Array(
                    'true_q'             => translate('The response to "%1$s" must end with the response to question "%2$s" (%3$s)'),
                    'false_q'            => translate('The response to "%1$s" must NOT end with the response to question "%2$s" (%3$s)'),
                    'true'               => translate('The response to "%1$s" must end with "%2$s" (%3$s)'),
                    'false'              => translate('The response to "%1$s" must NOT end with "%2$s" (%3$s)'),
                    'true_q_accessible'  => translate('The response must end with the response to question "%2$s" (%3$s).'),
                    'false_q_accessible' => translate('The response must NOT end with the response to question "%2$s" (%3$s).'),
                    'true_accessible'    => translate('The response must end with "%2$s" (%3$s).'),
                    'false_accessible'   => translate('The response must NOT end with "%2$s" (%3$s).'),
        );

        $case_options = Array(
                            0 => translate('not case-sensitive'),
                            1 => translate('case sensitive'),
        );

		$operator = array_get_index($rule_data, 'operator', 1);
		$cq_id = array_get_index($rule_data, 'comparison_question_id', 0);
		$value = array_get_index($rule_data, 'value', '');
		$case_sensitive = array_get_index($rule_data, 'case_sensitive', 1);

		$string_code = ($operator ? 'true' : 'false');
		$cq_id = array_get_index($rule_data, 'comparison_question_id', 0);

		if ($cq_id != 0) {
			$string_code .= '_q';
			$comparison = $GLOBALS['SQ_SYSTEM']->am->getAsset($cq_id);
			$value = $comparison->name;
		} else {	// sent just a value
			$value = array_get_index($rule_data, 'value', '');
		}
		$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($q_asset->_formid);
		$use_accessible_string = $form->useAccessibleFormat();

		if ($use_accessible_string) {
		    $string_code .= '_accessible';
		}

	    $error_str = sprintf(
	        $errors[$string_code],
	        $q_asset->name,
	        $value,
	        $case_options[(int) $case_sensitive]
	    );

		return $error_str;

	}//end ruleDescription()


}//end class
?>
