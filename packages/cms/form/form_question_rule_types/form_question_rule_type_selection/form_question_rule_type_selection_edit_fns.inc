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
* $Id: form_question_rule_type_selection_edit_fns.inc,v 1.4 2005/04/29 05:40:29 gsherwood Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule_edit_fns.inc';

/**
* Form_Question_Rule_Type_Selection_Edit_Fns
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Rule_Type_Selection_Edit_Fns extends Form_Question_Rule_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Form_Question_Rule_Type_Selection_Edit_Fns()
	{
		$this->Form_Question_Rule_Edit_Fns();

	}//end constructor


	function paintRule(&$asset, &$o, $prefix, $rule_data, $write_access, &$q_asset)
	{
		$operator_list = $asset->getOperators();
		$options = $q_asset->getOptions();

		if (!$write_access) {
			$o->openField('Rule Details');
			echo 'Option "'.$rule_data['value'].'" '.strtolower($operator_list[array_get_index($rule_data, 'operator', 1)]);

			$o->closeField();

		} else {

			$o->openField('Operator');
			// list of operators
			combo_box($prefix.'[operator]', $operator_list, false, array_get_index($rule_data, 'operator', 1));
			$o->closeField();

			$o->openField('Option');
			// give a text box to write the comparison value in
			combo_box($prefix.'[value]', $options, false, array_get_index($rule_data, 'value', ''));
			$o->closeField();
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

		$rule_data['operator'] = $operator;
		$rule_data['value'] = $comparison;

		return true;

	}//end processRule()


}//end class
?>
