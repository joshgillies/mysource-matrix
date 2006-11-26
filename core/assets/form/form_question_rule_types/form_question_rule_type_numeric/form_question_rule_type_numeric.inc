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
* $Id: form_question_rule_type_numeric.inc,v 1.5 2006/11/26 23:34:23 rhoward Exp $
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
* @version $Revision: 1.5 $
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
	function Form_Question_Rule_Type_Numeric($assetid=0)
	{
		$this->Form_Question_Rule($assetid);

	}//end constructor


	/**
	* Returns a list of operators for this rule type
	*
	* @access public
	* @return array (mixed => string) an array of descriptions indexed by operator id (which can be
	*                                 anything, generally string or (int)0|1 for a boolean value)
	*/
	function getOperators()
	{
		return Array(
				1	=> translate('core_form_rule_numeric_true'),
				0	=> translate('core_form_rule_numeric_false'),
			   );

	}//end getOperators()


	/**
	* writes JavaScript code for client side validation
	*
	* @param object Form_Question	&$q_asset		reference to the question asset
	* @param Array					$rule_data		the rule data being used
	*
	* @access public
	* @return boolean
	*/
	function generateJSCode(&$q_asset, $rule_data)
	{
		// have we printed out the validation function yet?
		// (since this is static across all class instances, this will work)
		static $printed_fn = false;

		$operator = array_get_index($rule_data, 'operator', 1);

		// regex code - allow leading plus or minute, digits, decimal point
		$regex_code = '/^[-+]?\d*\.?\d+$/';

		ob_start();			// buffer this please

			if (!$printed_fn) {
				$printed_fn = true;

					?>
					function sq_form_validate_numeric(answer)
					{
						answer.replace(/,/, "");
						return answer.match(<?php echo $regex_code ?>);

					}//end sq_form_validate_numeric()
					<?php

			}

			?>
				if (<?php echo ($operator == 1) ? '!' : ''; ?>sq_form_validate_numeric(form.elements["q<?php echo $q_asset->id; ?>"].value)) {
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
	* [operator] => operator to compare with (boolean: 1 or 0)
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

		$valid = (is_numeric($answer)) ? 1 : 0;
		return ($valid == $operator);

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
		$operator = array_get_index($rule_data, 'operator', 1);
		$error_str = translate('core_form_rule_numeric_desc_'.($operator ? 'true' : 'false'), $q_asset->attr('name'));

		return $error_str;

	}//end ruleDescription()


}//end class
?>
