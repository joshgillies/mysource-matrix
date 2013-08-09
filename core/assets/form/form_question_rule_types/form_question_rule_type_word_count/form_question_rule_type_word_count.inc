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
* $Id: form_question_rule_type_word_count.inc,v 1.1 2013/08/09 01:02:43 akarelia Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule.inc';

/**
* Form_Question_Rule_Type_Word_Count
*
* Purpose
*
*
* @author  Ash Karelia <akarelia@squiz.com.au>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Rule_Type_Word_Count extends Form_Question_Rule
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
						'=='	=> translate('core_form_rule_count_=='),
						'!='	=> translate('core_form_rule_count_!='),
						'>'		=> translate('core_form_rule_count_>'),
						'<'		=> translate('core_form_rule_count_<'),
						'>='	=> translate('core_form_rule_count_>='),
						'<='	=> translate('core_form_rule_count_<='),
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
		$value = array_get_index($rule_data, 'value', '');

		// if the validation condition is set, we need to output the part that checks whether the condition is met or not as well
		$condition_js = parent::generateConditionJSCode($rule_data);
		if (strlen(trim($condition_js))) $condition_js = ' && '.$condition_js;
		
		ob_start();			// buffer this please

			?>
				if (!(form.elements["q<?php echo $q_asset->id; ?>"].value.split(" ").length <?php echo $operator ?> <?php echo $value ?>)<?php echo $condition_js; ?>) {
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
		$value = array_get_index($rule_data, 'value', '');

		$valid = eval('return \''.str_word_count($answer).'\''.$operator.'\''.$value.'\';');
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

		$value = array_get_index($rule_data, 'value', '');
		$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($q_asset->_formid);
	    	$use_accessible_string = $form->useAccessibleFormat();
		if($use_accessible_string)
		    $error_str = translate('core_form_rule_count_desc_'.$operator.'_accessible', $value);
		else
		     $error_str = translate('core_form_rule_count_desc_'.$operator, $q_asset->attr('name'), $value);

		return $error_str;

	}//end ruleDescription()


}//end class

?>
