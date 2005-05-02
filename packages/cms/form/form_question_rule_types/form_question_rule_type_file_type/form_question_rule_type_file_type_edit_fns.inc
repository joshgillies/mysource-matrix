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
* $Id: form_question_rule_type_file_type_edit_fns.inc,v 1.1.2.2 2005/05/02 02:44:57 gsherwood Exp $
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
* @version $Revision: 1.1.2.2 $
* @package MySource_Matrix_Packages
* @subpackage cms
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
			$o->openField('Rule Details');
			echo 'Uploaded file  '.strtolower($operator_list[array_get_index($rule_data, 'operator', 1)]);

			echo ' '.$rule_data['value'];
			$o->closeField();

		} else {

			$o->openField('Operator');
			// list of operators
			combo_box($prefix.'[operator]', $operator_list, false, array_get_index($rule_data, 'operator', 1));
			$o->closeField();

			$o->openField('Extension List');
			// give a text box to write the comparison value in
			$option_list = new Asset_Attribute_Option_List(0, array_get_index($rule_data, 'value', ''));
			$option_list->paint($prefix.'_value');

			$o->closeField();

			$o->sectionNote('Enter a list of extensions to include or exclude, clicking \'Add\' after each one. To remove an extension, click the \'Del\' link next to the particular extension. Do not include the full stop as part of the extension.');
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
		$comparison = trim(array_get_index($_POST, str_replace(':', '_', $prefix).'_value_options', null));

		$rule_data['operator'] = $operator;
		$rule_data['value'] = $comparison;

		return true;

	}//end processRule()


}//end class
?>
