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
* $Id: form_question_rule_type_contain_edit_fns.inc,v 1.6 2013/04/19 06:36:00 ewang Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule_edit_fns.inc';

/**
* Form_Question_Rule_Type_Contain_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Rule_Type_Contain_Edit_Fns extends Form_Question_Rule_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Form_Question_Rule_Type_Contain_Edit_Fns()
	{
		$this->Form_Question_Rule_Edit_Fns();

	}//end constructor


	function paintRule(&$asset, &$o, $prefix, $rule_data, $write_access)
	{
		$operator_list = $asset->getOperators();

		$comparison_text = Array(
			'1'   => translate('The response must contain "%s"'),
			'1_q' => translate('The response must contain the response to question "%s"'),
			'0'   => translate('The response must NOT contain "%s"'),
			'0_q' => translate('The response must NOT contain the response to question "%s"'),
		);

		$cq_id = (string) array_get_index($rule_data, 'comparison_question_id', '0');
		$op    = (string) array_get_index($rule_data, 'operator', '1');
		$value = (string) array_get_index($rule_data, 'value', '');
		$case  = (int) array_get_index($rule_data, 'case_sensitive', 1);

		if (!$write_access) {
			$o->openField(translate('Rule Details'));
			if ($cq_id !== '0') {
				$value = get_asset_tag_line($cq_id);
				$op   .= '_q';
			}

			echo sprintf(
				$comparison_text[$op],
				$value
			);

			if ($case === 1) {
				echo ' ('.translate('case sensitive').')';
			}
			$o->closeField();

		} else {

			$o->openField(translate('Operator'));

			// list of operators
			combo_box($prefix.'[operator]', $operator_list, false, $op);
			$o->closeField();

			$o->openField(translate('Static Value'));

			// give a text box to write the comparison value in
			text_box($prefix.'[value]', $value, 30);
			$o->closeField();

			$o->openField(translate('OR Value of Question'));

			// give a text box to write the comparison value in
			asset_finder($prefix.'[asset]', $cq_id, Array('form_question' => 'D'));
			$o->closeField();

			$o->openField(translate('Case Sensitive?'));

			// give a text box to write the comparison value in
			check_box($prefix.'[case]', 1, $case);
			$o->closeField();

			$o->sectionNote(translate('Enter either a static value to compare responses against, or select another question from this form to compare with that question\'s response. The selection of a question takes precedence over a static value.'));

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
		$comparison = array_get_index($_POST[$prefix], 'value', null);
		$comp_asset = array_get_index($_POST[$prefix], 'asset', null);
		$case = array_get_index($_POST[$prefix], 'case', 0);

		$rule_data['operator'] = $operator;
		$rule_data['case_sensitive'] = $case;
		if (is_null($comp_asset) || ($comp_asset['assetid'] == '0')) {
			$rule_data['value'] = $comparison;
			$rule_data['comparison_question_id'] = '0';
		} else {
			$rule_data['comparison_question_id'] = $comp_asset['assetid'];
			$rule_data['value'] = null;
		}

		return true;

	}//end processRule()


}//end class
?>
