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
* $Id: form_question_rule_type_numeric.inc,v 1.14 2013/04/19 06:36:01 ewang Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule.inc';

/**
* Form_Question_Rule_Type_Numeric
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Revision: 1.14 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Rule_Type_Numeric extends Form_Question_Rule
{


	/**
	* Constructor
	*
	* [Will we still need a constructor seeing we will only be calling this class statically?]
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
	* Returns an array in the format (mixed => string) of descriptions indexed by operator id (which can be
	* anything, generally string or (int)0|1 for a boolean value)
	*
	* @return array
	* @access public
	*/
	public static function getOperators()
	{
		return Array(
				1	=> translate('core_form_rule_numeric_true'),
				0	=> translate('core_form_rule_numeric_false'),
			   );

	}//end getOperators()


	/**
	* writes JavaScript code for client side validation
	*
	* @param object	&$q_asset	reference to the Form_Question asset
	* @param array	$rule_data	the rule data being used
	*
	* @return boolean
	* @access public
	*/
	function generateJSCode(&$q_asset, $rule_data)
	{
		// have we printed out the validation function yet?
		// (since this is static across all class instances, this will work)
		static $printed_fn_required = FALSE;
		static $printed_fn_not_required = FALSE;

		$operator = array_get_index($rule_data, 'operator', 1);

        // regex code - allow empty or leading plus or minute, digits, decimal point
        $regex_code_not_required = '/(^$)|(^[-+]?\d*\.?\d+$)/';
        // regex code - allow leading plus or minute, digits, decimal point, but not empty
        $regex_code_required = '/^[-+]?\d*\.?\d+$/';

		// if the validation condition is set, we need to output the part that checks whether the condition is met or not as well
		$condition_js = parent::generateConditionJSCode($rule_data);
		$additional_condition = '';
		if (strlen(trim($condition_js))) {
			$additional_condition = 'if (!'.$condition_js.') return true;';
		}

		ob_start();			// buffer this please

            // output js function for *required* validation
			if (!$printed_fn_required && $q_asset->attr('is_required')) {
				$printed_fn_required = TRUE;

					?>
					function sq_form_validate_numeric_required(answer)
					{
						<?php echo $additional_condition; ?>
						answer.replace(/,/, "");
						return answer.match(<?php echo $regex_code_required ?>);

					}//end sq_form_validate_numeric_required()
					<?php
            // output js function for validation where field is not required to be filled
            } else if (!$printed_fn_not_required) {
				$printed_fn_not_required = TRUE;

					?>
					function sq_form_validate_numeric_not_required(answer)
					{
						<?php echo $additional_condition; ?>
						answer.replace(/,/, "");
						return answer.match(<?php echo $regex_code_not_required ?>);

                    }//end sq_form_validate_numeric_not_required()

					<?php

            }

            // generate calls to the above 2 functions
            // if this field is required
            if ($q_asset->attr('is_required')) {
                 	?>
                    if (<?php echo ($operator == 1) ? '!' : ''; ?>sq_form_validate_numeric_required(form.elements["q<?php echo $q_asset->id; ?>"].value)) {
                        submission_errors[submission_errors.length] = "<?php
                            if (empty($rule_data['custom_text'])) {
                                echo addslashes($this->defaultError($q_asset, $rule_data));
                            } else {
                                echo addslashes($rule_data['custom_text']);
                            }
                        ?>";
                    } <?php
            // if this field is not required
            } else if (!$q_asset->attr('is_required')) {
             		?>
                    if (<?php echo ($operator == 1) ? '!' : ''; ?>sq_form_validate_numeric_not_required(form.elements["q<?php echo $q_asset->id; ?>"].value)) {
                        submission_errors[submission_errors.length] = "<?php
                            if (empty($rule_data['custom_text'])) {
                                echo addslashes($this->defaultError($q_asset, $rule_data));
                            } else {
                                echo addslashes($rule_data['custom_text']);
                            }
                        ?>";
                    } <?php
            }

		$contents = ob_get_contents();
		ob_end_clean();

		return $contents;

	}//end generateJSCode()


	/**
	* processes a test on a single static value
	*
	* Rule data is an array with the following fields:
	* [operator] => operator to compare with (boolean: 1 or 0)
	* return true only if rule succeeds
	*
	* @param mixed	$answer		the answer being passed to us
	* @param string	$rule_data	the rule's data
	*
	* @return boolean
	* @access public
	*/
	function evaluate($answer, $rule_data)
	{
		$operator = array_get_index($rule_data, 'operator', 1);

		$valid = (is_numeric($answer)) ? 1 : 0;
		return ($valid == $operator);

	}//end evaluate()


	/**
	* returns a description of this rule
	*
	* @param object	&$q_asset	the Form_Question asset that failed
	* @param array	$rule_data	the rule data
	*
	* @return string
	* @access public
	*/
	function ruleDescription(&$q_asset, $rule_data)
	{

		$operator = array_get_index($rule_data, 'operator', 1);
		$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($q_asset->_formid);
	    	$use_accessible_string = $form->useAccessibleFormat();
		if($use_accessible_string)
		    $error_str = translate('core_form_rule_numeric_desc_'.($operator ? 'true' : 'false').'_accessible');
		else
		    $error_str = translate('core_form_rule_numeric_desc_'.($operator ? 'true' : 'false'), $q_asset->attr('name'));

		return $error_str;

	}//end ruleDescription()


}//end class
?>
