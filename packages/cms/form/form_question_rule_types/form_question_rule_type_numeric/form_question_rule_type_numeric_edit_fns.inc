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
* $Id: form_question_rule_type_numeric_edit_fns.inc,v 1.8.6.1 2005/04/14 01:43:57 lwright Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule_edit_fns.inc';

/**
* Form_Question_Rule_Type_Numeric_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.8.6.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Rule_Type_Numeric_Edit_Fns extends Form_Question_Rule_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Form_Question_Rule_Type_Numeric_Edit_Fns()
	{
		$this->Form_Question_Rule_Edit_Fns();
	}//end constructor


	function paintRule(&$asset, &$o, $prefix, $rule_data, $write_access)
	{
		$operator_list = $asset->getOperators();

		if (!$write_access) {
			$o->openField('Rule Details');
			echo 'Response '.strtolower($operator_list[array_get_index($rule_data, 'operator', 1)]);
			$o->closeField();

		} else {

			$o->openField('Condition');
			// list of operators
			combo_box($prefix.'[operator]', $operator_list, false, array_get_index($rule_data, 'operator', '1'));
			$o->closeField();

			$o->sectionNote('This rule requires only the condition to be satisfied to be set.');
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

		$operator = array_get_index($_POST[$prefix], 'operator', '1');
		$rule_data['operator'] = $operator;

		return true;

	}//end processRule()


}//end class
?>
