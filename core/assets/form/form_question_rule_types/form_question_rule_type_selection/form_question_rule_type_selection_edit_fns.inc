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
* $Id: form_question_rule_type_selection_edit_fns.inc,v 1.5 2013/04/19 06:36:02 ewang Exp $
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
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
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
			$o->openField(translate('core_form_rule_details'));
			echo translate('core_form_rule_selection_edit_'.(array_get_index($rule_data, 'operator', 1) ? 'true' : 'false'),  $q_asset->getSummary($rule_data['value']));

			$o->closeField();

		} else {

			$o->openField(translate('operator'));
			// list of operators
			combo_box($prefix.'[operator]', $operator_list, false, array_get_index($rule_data, 'operator', 1));
			$o->closeField();

			$o->openField(translate('option'));
			// give a text box to write the comparison value in
			combo_box($prefix.'[value]', $options, false, array_get_index($rule_data, 'value', ''));
			$o->closeField();
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

		$rule_data['operator'] = $operator;
		$rule_data['value'] = $comparison;

		return true;

	}//end processRule()


}//end class
?>
