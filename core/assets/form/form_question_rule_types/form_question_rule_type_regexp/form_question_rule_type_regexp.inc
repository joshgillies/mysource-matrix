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
* $Id: form_question_rule_type_regexp.inc,v 1.13 2013/04/19 06:36:01 ewang Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule.inc';

/**
* Form_Question_Rule_Type_Regexp
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Revision: 1.13 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Rule_Type_Regexp extends Form_Question_Rule
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
				1	=> translate('Must Match the Regular Expression'),

				0	=> translate('Must NOT Match the Regular Expression'),

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
		$operator = array_get_index($rule_data, 'operator', 1);
		$value = array_get_index($rule_data, 'value', '');
		$case_sensitive = array_get_index($rule_data, 'case_sensitive', 1) ? '' : 'i';

		// if the validation condition is set, we need to output the part that checks whether the condition is met or not as well
		$condition_js = parent::generateConditionJSCode($rule_data);
		if (strlen(trim($condition_js))) $condition_js = ' && '.$condition_js;

		ob_start();			// buffer this please

			// JavaScript only accepts forward slashes for their regexs
			?>
				if (<?php echo ($operator == 1) ? '!' : ''; ?>form.elements["q<?php echo $q_asset->id; ?>"].value.match(/<?php echo str_replace('/', '\/', $value) ?>/<?php echo $case_sensitive; ?>)<?php echo $condition_js; ?>) {
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
	* [value] => comparison value (regular expression)
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

		$pattern = '/'.str_replace('/', '\\'.'/', array_get_index($rule_data, 'value', '')).'/';

		// case insensitive?
		if (!array_get_index($rule_data, 'case_sensitive', 1)) {
			$pattern .= 'i';
		}

		$valid = (preg_match($pattern, $answer)) ? 1 : 0;

		return ($valid == $operator);

	}//end evaluate()


	/**
	* returns a default error message for this rule
	*
	* @param object Form_Question	&$q_asset	the question asset that failed
	* @param array					$rule_data	the rule data
	*
	* @return string	the error message
	* @access public
	*/
	function defaultError(&$q_asset, $rule_data)
	{
		$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($q_asset->_formid);
		$use_accessible_string = $form->useAccessibleFormat();
		if($use_accessible_string)
		    $error_str = translate('The response is not in the correct format.');

		else
		    $error_str = sprintf(translate('The response to question "%s" is not in the correct format'), $q_asset->name);


		return $error_str;

	}//end defaultError()


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
		$operator = array_get_index($rule_data, 'operator', 1);
		$value = array_get_index($rule_data, 'value', '');
		$operator_list = $this->getOperators();

		$error_str = translate('core_form_rule_regexp_desc_'.($operator ? 'true' : 'false'), $q_asset->attr('name'), $value);

		return $error_str;

	}//end ruleDescription()


}//end class
?>
