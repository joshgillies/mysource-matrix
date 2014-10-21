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
* $Id: form_question_rule_type_file_virus_check_edit_fns.inc,v 1.3 2013/04/19 06:36:00 ewang Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule_edit_fns.inc';
require_once SQ_SYSTEM_ROOT.'/core/attributes/option_list/option_list.inc';
require_once SQ_DATA_PATH.'/private/conf/tools.inc';
/**
* Form_Question_Rule_Type_File_Virus_Check_Edit_Fns
*
* Purpose
*
*
* @author  Edison Wang <ewang@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Rule_Type_File_Virus_Check_Edit_Fns extends Form_Question_Rule_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Form_Question_Rule_Type_File_Virus_Check_Edit_Fns()
	{
		$this->Form_Question_Rule_Edit_Fns();

	}//end constructor


	function paintRule(&$asset, &$o, $prefix, $rule_data, $write_access)
	{
		$default_option = Array (
							1 => translate('Yes'),

							2 => translate('No'),

						);

		// display warning message if global virus checker is disabled or not configured
		if(!SQ_TOOL_VIRUS_CHECKER_ENABLED || !file_exists(SQ_TOOL_VIRUS_CHECKER_PATH)) {
				$warning = '<p class="sq-backend-warning">'.translate('Virus Checker has to be configured and enabled in External Tool Configuration first.').'</p>';

		}


		if (!$write_access) {
			$o->openField(translate('Enable Virus Checking?'));

			if(empty($warning)){
				echo translate('core_form_rule_file_virus_check_'.(array_get_index($rule_data, 'value', TRUE) ? 'true' : 'false'));
			}
			else {
				echo $warning;
			}
			$o->closeField();
		} else {
			$o->openField(translate('Enable Virus Checking?'));

			if(empty($warning)){
				combo_box($prefix.'[check_virus]', $default_option, FALSE, array_get_index($rule_data, 'check_virus', 1));
			}
			else {
				echo $warning;
			}
			$o->closeField();

			$o->openField(translate('Custom Progress Indicator Text'));

			text_box($prefix.'[indicator_content]', array_get_index($rule_data, 'indicator_content', ''), 30);
			$o->closeField();

			$o->openField(translate('Custom Progress Indicator Class'));

			text_box($prefix.'[indicator_class]', array_get_index($rule_data, 'indicator_class', ''), 30);
			$o->closeField();

			$o->sectionNote(translate('Virus scanning progress indicator can only be displayed and customised when client side validation is turned on.'));

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

	 	$check_virus = array_get_index($_POST[$prefix], 'check_virus', null);
		$rule_data['check_virus'] = $check_virus;

		$indicator_content = array_get_index($_POST[$prefix], 'indicator_content', null);
		$rule_data['indicator_content'] = $indicator_content;

		$indicator_class = array_get_index($_POST[$prefix], 'indicator_class', null);
		$rule_data['indicator_class'] = $indicator_class;


		return true;

	}//end processRule()


}//end class
?>
