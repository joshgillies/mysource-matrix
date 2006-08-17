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
* $Id: form_question_rule_type_file_size.inc,v 1.2.2.1 2006/08/17 06:40:33 bcaldwell Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule.inc';

/**
* Form_Question_Rule_Type_File_Size
*
* Purpose
*
*
* @static
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.2.2.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Rule_Type_File_Size extends Form_Question_Rule
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form_Question_Rule_Type_File_Size($assetid=0)
	{
		$this->Form_Question_Rule($assetid);

	}//end constructor


	/**
	* Returns a list of operators for this rule type
	*
	* @access public
	* @return array (mixed => string) an array of descriptions indexed by operator id (which can be
	*                                 anything, generally string or (int)0|1 for a boolean value)
	*/
	function getOperators()
	{
		$operators = Array(
						'<='	=> translate('core_form_rule_file_size_<='),
						'>='	=> translate('core_form_rule_file_size_>='),
					 );
		return $operators;

	}//end getOperators()


	/**
	* processes the form question and returns true if it is valid
	*
	* Because the Form Question Rule classes will not be instantiable by Matrix NOR will be
	* instantiated manually in code, this function MUST called statically.
	*
	* @param object Form_Question	&$q_asset		reference to the question asset
	* @param string					$operator		the operator being used to compare
	* @param mixed					&$comparison	either a value or a form_question asset (based on
	*												comparison question id), being used to compare
	*
	* @see Form_Question_Rule::processForm()
	* @static
	* @access public
	* @return boolean
	*/
	function evaluate($answer, $rule_data, &$q_asset)
	{
		// prefix is added in form_question_type_file_upload::getHtmlField()
		$q_file_index = 'q'.$q_asset->id;

		$answer = $_FILES[$q_file_index];
		$operator = array_get_index($rule_data, 'operator', '<=');

		// comparison value will be the concatenation of the value and the
		// multiplier
		$comparison = array_get_index($rule_data, 'value', '').array_get_index($rule_data, 'multiplier', '');

		$filesize = $answer['size'];
		$comparison = strtolower(trim($comparison));
		$comp_value = floatval($comparison);

		// allow users to specify abbreviated forms of file size (kB, MB, GB)
		// the conversion used is 1024 to the kByte, rather than the decimal 1000
		if ((substr($comparison,-2) == 'kb') || (substr($comparison,-1) == 'k')) {
			$comp_value *= 1024;
		} else if ((substr($comparison,-2) == 'mb') || (substr($comparison,-1) == 'm')) {
			$comp_value *= 1024 * 1024;
		} else if ((substr($comparison,-2) == 'gb') || (substr($comparison,-1) == 'g')) {
			$comp_value *= 1024 * 1024 * 1024;
		}

		$expression = "return $filesize $operator $comp_value;";
		$valid = eval($expression);

		return $valid;

	}//end evaluate()


	/**
	* returns a description of the rule
	*
	* @param object Form_Question	&$q_asset	the question asset that failed
	* @param array					$rule_data	the rule data
	*
	* @return string	the error message
	* @access public
	*/
	function ruleDescription(&$q_asset, $rule_data)
	{
		$operator = array_get_index($rule_data, 'operator', '<=');
		$comparison = array_get_index($rule_data, 'value', '').' '.array_get_index($rule_data, 'multiplier', '');

		$error_str = translate('core_form_rule_file_size_desc_'.$operator, $q_asset->attr('name'), $comparison);

		return $error_str;

	}//end ruleDescription()


}//end class
?>
