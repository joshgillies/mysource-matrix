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
* $Id: form_question_rule_type_word_count_edit_fns.inc,v 1.1 2013/08/09 01:02:43 akarelia Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule_edit_fns.inc';

/**
* Form_Question_Rule_Type_Word_Count_Edit_Fns
*
* Purpose
*
*
* @author  Ash Karelia <akarelia@squiz.com.au>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Rule_Type_Word_Count_Edit_Fns extends Form_Question_Rule_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	function paintRule(&$asset, &$o, $prefix, $rule_data, $write_access)
	{
		$operator_list = $asset->getOperators();

		$op    = array_get_index($rule_data, 'operator', '==');
		$value = array_get_index($rule_data, 'value', '');

		$comparison_text = Array(
			'==' => translate('Response must be exactly %s words long'),
			'!=' => translate('Response must not be %s words long'),
			'>'  => translate('Response must be longer than %s words'),
			'<'  => translate('Response must be shorter than %s words'),
			'>=' => translate('Response must be no shorter than %s words'),
			'<=' => translate('Response must be no longer than %s words'),
		);

		if (!$write_access) {
			$o->openField(translate('Rule Details'));
			echo sprintf(
				$comparison_text[$op],
				$value
			);
			$o->closeField();

		} else {

			$o->openField(translate('Operator'));

			// list of operators
			combo_box($prefix.'[operator]', $operator_list, false, $op);
			$o->closeField();

			$o->openField(translate('Number of Words'));

			// give a text box to write the comparison value in
			int_text_box($prefix.'[value]', $value, false, 3);
			echo ' '.translate(' words');

			$o->closeField();

			$o->sectionNote(translate('Enter the number of words to compare the length of the response against in the text box above.'));

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
