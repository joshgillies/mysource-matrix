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
* $Id: form_question_rule_edit_fns.inc,v 1.5 2013/04/19 06:36:00 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Form_Question_Rule_Edit_Fns

* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Rule_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Form_Question_Rule_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

	}//end constructor


	/**
	* Paints the rule's interface
	*
	* New question rule type must call this function from its overridden function by 'parent::paintRule($asset, $o, $prefix, $rule_data, $write_access, $open_field)'
	* in order to allow the new question rule type to set a condition to execute the validation.
	*
	* @param object		&$asset			The asset to paint
	* @param object		&$o				The backend outputter
	* @param string		$prefix			String prefix for this interface/asset
	* @param array		$rule_data		The rule data
	* @param boolean	$write_access	Whether or not the user has write access
	*
	* @return boolean
	* @access public
	*/
	function paintRule(&$asset, &$o, $prefix, $rule_data, $write_access, $open_field=TRUE)
	{
		// paint validation condition that is generic to every question rule type
		$condition_operators = $asset->getConditionOperators();
		$condition_exists = isset($rule_data['condition_asset']) && $rule_data['condition_asset'];

		if ($open_field) $o->openField(translate('Condition'));

		if (!$write_access) {
			if (!$condition_exists) {
				echo translate('No condition to run this validation');

			} else {
				$target_question = $GLOBALS['SQ_SYSTEM']->am->getAsset($rule_data['condition_asset']);
				$target = $target_question->getVal('title');
				$operators = $asset->getConditionOperators();
				$operator = isset($rule_data['condition_operator']) && isset($operators[$rule_data['condition_operator']]) ? $operators[$rule_data['condition_operator']] : NULL;
				$value = isset($rule_data['condition_value']) ? '/'.$rule_data['condition_value'].'/' : '';
				echo sprintf(translate('Run this validation when %1$s %2$s %3$s'), $target, $operator, $value);

			}
		} else {
			$enabled = isset($_POST[$prefix]) ? array_get_index($_POST[$prefix], 'enable', 0) : FALSE;
			label( translate('Add a condition to this validation rule?'), $prefix.'[enable]');

			check_box($prefix.'[enable]', '1', $enabled || $condition_exists);

			// only display condition configuration interface when required
			if($enabled || $condition_exists) {
			    echo '<br>';
			    echo translate('Validate if the answer of');

			    asset_finder($prefix.'[condition_asset]', array_get_index($rule_data, 'condition_asset', '0'), Array('form_question' => 'D'));
			    combo_box($prefix.'[condition_operator]', $condition_operators, false, array_get_index($rule_data, 'condition_operator', 1));
			    text_box($prefix.'[condition_value]', array_get_index($rule_data, 'condition_value', ''), 50);
			}
		}
		if ($open_field) $o->closeField();

		return $write_access;

	}//end paintRule()


	/**
	* Process the rule's interface
	*
	*
	* New question rule type must call this function from its overridden function by 'parent::processRule($asset, $o, $prefix, $rule_data)'
	* in order to allow the new question rule type to set a condition to execute the validation.
	* @param object	&$asset		The asset to paint
	* @param object	&$o			The backend outputter
	* @param string	$prefix		String prefix for this interface/asset
	* @param array	&$rule_data	The rule data
	*
	* @return boolean
	* @access public
	*/
	function processRule(&$asset, &$o, $prefix, &$rule_data)
	{
		// process validation condition settings that are generic to every question rule type
		if (!isset($_POST[$prefix])) return false;

		$condition_asset = array_get_index($_POST[$prefix], 'condition_asset', 0);
		$condition_operator = array_get_index($_POST[$prefix], 'condition_operator', '=');
		$condition_value = array_get_index($_POST[$prefix], 'condition_value', null);
		$enabled = array_get_index($_POST[$prefix], 'enable', 0);

		// remove the condition
		if(!$enabled) {
		    unset($rule_data['condition_asset']);
		    unset($rule_data['condition_operator']);
		    unset($rule_data['condition_value']);
		}
		else {
		    $condition_asset = $condition_asset['assetid'];
		    $rule_data['condition_asset'] = $condition_asset;
		    $rule_data['condition_operator'] = $condition_operator;
		    $rule_data['condition_value'] = $condition_value;
		}

		return true;

	}//end processRule()


}//end class
?>