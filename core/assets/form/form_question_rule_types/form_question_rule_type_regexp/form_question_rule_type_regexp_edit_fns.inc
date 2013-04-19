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
* $Id: form_question_rule_type_regexp_edit_fns.inc,v 1.6 2013/04/19 06:36:01 ewang Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule_edit_fns.inc';

/**
* Form_Question_Rule_Type_Regexp_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Rule_Type_Regexp_Edit_Fns extends Form_Question_Rule_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Form_Question_Rule_Type_Regexp_Edit_Fns()
	{
		$this->Form_Question_Rule_Edit_Fns();

	}//end constructor


	function paintRule(&$asset, &$o, $prefix, $rule_data, $write_access)
	{
		$operator_list = $asset->getOperators();

		if (!$write_access) {
			$o->openField(translate('core_form_rule_details'));
			echo translate('core_form_rule_regexp_edit_'.(array_get_index($rule_data, 'operator', 1) ? 'true' : 'false'), array_get_index($rule_data, 'value', translate('none')));
			$o->closeField();

		} else {

			$o->openField(translate('operator'));
			// list of operators
			combo_box($prefix.'[operator]', $operator_list, false, array_get_index($rule_data, 'operator', 1));
			$o->closeField();

			$o->openField(translate('regular_expression'));
			// give a text box to write the comparison value in
			text_box($prefix.'[value]', array_get_index($rule_data, 'value', ''), 30);
			$o->closeField();

			$o->openField(translate('core_form_case_sensitive_question'));
			// give a text box to write the comparison value in
			check_box($prefix.'[case]', 1, array_get_index($rule_data, 'case_sensitive', 1));
			$o->closeField();

			$o->sectionNote(translate('core_form_rule_regexp_note'));
		}
		parent::paintRule($asset, $o, $prefix, $rule_data, $write_access);
		return $write_access;

	}//end paintRule()


	/**
	* Process the rule's interface
	*
	* @return boolean
	* @access public
	*/
	function processRule(&$asset, &$o, $prefix, &$rule_data)
	{
		if (!isset($_POST[$prefix])) return false;
		parent::processRule($asset, $o, $prefix, $rule_data);

		$operator = array_get_index($_POST[$prefix], 'operator', 1);
		$regexp = array_get_index($_POST[$prefix], 'value', null);
		$case = array_get_index($_POST[$prefix], 'case', 0);

		$rule_data['operator'] = $operator;
		$rule_data['value'] = $regexp;
		$rule_data['case_sensitive'] = $case;

		return true;

	}//end processRule()


}//end class
?>
