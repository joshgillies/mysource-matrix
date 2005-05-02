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
* $Id: form_question_rule_type_comparison_edit_fns.inc,v 1.12.6.2 2005/05/02 02:44:57 gsherwood Exp $
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
* @version $Revision: 1.12.6.2 $
* @package MySource_Matrix_Packages
* @subpackage cms
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
			$o->openField('Rule Details');
			echo 'Response '.strtolower($operator_list[array_get_index($rule_data, 'operator', '==')]);

			if (($cq_id = array_get_index($rule_data, 'comparison_question_id', 0)) != 0) {
				$q_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($cq_id);
				echo ' the response to "'.$q_asset->attr('name').'" (Id: #'.$cq_id.')';
			} else {
				echo ' "'.array_get_index($rule_data, 'value', '').'"';
			}
			$o->closeField();

		} else {

			$o->openField('Operator');
			// list of operators
			combo_box($prefix.'[operator]', $operator_list, false, array_get_index($rule_data, 'operator', '=='));
			$o->closeField();

			$o->openField('Static Value');
			// give a text box to write the comparison value in
			text_box($prefix.'[value]', array_get_index($rule_data, 'value', ''), 30);
			$o->closeField();

			$o->openField('OR Value of Question');
			// give a text box to write the comparison value in
			asset_finder($prefix.'[asset]', array_get_index($rule_data, 'comparison_question_id', '0'), Array('form_question' => 'D'));
			$o->closeField();

			$o->sectionNote('Enter either a static value to compare responses against, or select another question from this form to compare with that question\'s response. The selection of a question takes precedence over a static value.');
		}

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
		if (!isset($_POST[$prefix])) {
			return false;
		}

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
