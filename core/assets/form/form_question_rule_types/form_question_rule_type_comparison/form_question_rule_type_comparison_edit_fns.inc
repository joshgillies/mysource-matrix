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
* $Id: form_question_rule_type_comparison_edit_fns.inc,v 1.6 2013/04/19 06:36:00 ewang Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule_edit_fns.inc';

/**
* Form_Question_Rule_Type_Comparison_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Rule_Type_Comparison_Edit_Fns extends Form_Question_Rule_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Form_Question_Rule_Type_Comparison_Edit_Fns()
	{
		$this->Form_Question_Rule_Edit_Fns();

	}//end constructor


	function paintRule(&$asset, &$o, $prefix, $rule_data, $write_access)
	{
		$operator_list = $asset->getOperators();

		if (!$write_access) {
			$o->openField(translate('Rule Details'));

			if (($cq_id = array_get_index($rule_data, 'comparison_question_id', 0)) != 0) {
				$value = get_asset_tag_line($cq_id);
			} else {
				$value = array_get_index($rule_data, 'value', '');
			}

			echo translate('core_form_rule_comparison_edit_'.array_get_index($rule_data, 'operator', '==').($cq_id ? '_q' : ''), $value);
			$o->closeField();

		} else {

			$o->openField(translate('Operator'));

			// list of operators
			combo_box($prefix.'[operator]', $operator_list, false, array_get_index($rule_data, 'operator', '=='));
			$o->closeField();

			$o->openField(translate('Static Value'));

			// give a text box to write the comparison value in
			text_box($prefix.'[value]', array_get_index($rule_data, 'value', ''), 30);
			$o->closeField();

			$o->openField(translate('OR Value of Question'));

			// give a text box to write the comparison value in
			asset_finder($prefix.'[asset]', array_get_index($rule_data, 'comparison_question_id', '0'), Array('form_question' => 'D'));
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

		$operator = array_get_index($_POST[$prefix], 'operator', null);
		$comparison = array_get_index($_POST[$prefix], 'value', null);
		$comp_asset = array_get_index($_POST[$prefix], 'asset', null);

		$rule_data['operator'] = $operator;
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
