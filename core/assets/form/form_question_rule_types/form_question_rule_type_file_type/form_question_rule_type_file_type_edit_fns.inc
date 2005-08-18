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
* $Id: form_question_rule_type_file_type_edit_fns.inc,v 1.3 2005/08/18 00:01:24 lwright Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule_edit_fns.inc';
require_once SQ_SYSTEM_ROOT.'/core/attributes/option_list/option_list.inc';

/**
* Form_Question_Rule_Type_File_Type_Edit_Fns
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Rule_Type_File_Type_Edit_Fns extends Form_Question_Rule_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Form_Question_Rule_Type_File_Type_Edit_Fns()
	{
		$this->Form_Question_Rule_Edit_Fns();

	}//end constructor


	function paintRule(&$asset, &$o, $prefix, $rule_data, $write_access)
	{
		$operator_list = $asset->getOperators();

		if (!$write_access) {
			$o->openField(translate('core_form_rule_details'));
			echo translate('core_form_rule_file_type_edit_'.(array_get_index($rule_data, 'operator', 1) ? 'true' : 'false'), $rule_data['value']);
			$o->closeField();

		} else {

			$o->openField(translate('operator'));
			// list of operators
			combo_box($prefix.'[operator]', $operator_list, false, array_get_index($rule_data, 'operator', 1));
			$o->closeField();

			$o->openField(translate('core_form_rule_file_type_ext_list'));
			// give a text box to write the comparison value in
			$option_list =& new Asset_Attribute_Option_List(0, array_get_index($rule_data, 'value', ''));
			$option_list->paint($prefix.'_value');

			$o->closeField();

			$o->sectionNote(translate('core_form_rule_file_type_note'));
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
		if (!isset($_POST[$prefix])) return false;

		$operator = array_get_index($_POST[$prefix], 'operator', null);
		$comparison = trim(implode("\r\n", array_get_index($_POST, str_replace(':', '_', $prefix).'_value_options', Array())));

		$rule_data['operator'] = $operator;
		$rule_data['value'] = $comparison;

		return true;

	}//end processRule()


}//end class
?>
