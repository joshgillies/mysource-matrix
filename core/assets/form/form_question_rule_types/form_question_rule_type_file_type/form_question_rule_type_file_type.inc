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
* $Id: form_question_rule_type_file_type.inc,v 1.2.2.1 2006/07/21 05:56:21 tbarrett Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule.inc';

/**
* Form_Question_Rule_Type_File_Type
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.2.2.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Rule_Type_File_Type extends Form_Question_Rule
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form_Question_Rule_Type_File_Type($assetid=0)
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
		return Array(
				1	=> translate('core_form_rule_file_type_true'),
				0	=> translate('core_form_rule_file_type_false'),
			   );

	}//end getOperators()



	/**
	* processes a test on a single static value
	*
	* Used when we are using rules outside of validating a form - for example selective emails.
	*
	* Because the Form Question Rule classes will not be instantiable by Matrix NOR will be
	* instantiated manually in code, this function MUST called statically.
	*
	* The value being tested in this case should be the file name of the uploaded file.
	*
	* @param mixed	$answer			the answer being passed to us
	* @param string	$operator		the operator being used to compare
	* @param mixed	&$comparison	either a value or a form_question asset (based on
	*								comparison question id), being used to compare
	*
	* @static
	* @access public
	* @return boolean true only if rule succeeds
	*/
	function evaluate($answer, $rule_data)
	{
		$operator = array_get_index($rule_data, 'operator', 1);
		$comparison = array_get_index($rule_data, 'value', '');

		if (($answer == '') || (strrpos($answer, '.') === FALSE)) {
			$valid = FALSE;
		} else {
			$extension = substr($answer, strrpos($answer, '.') + 1);
			$valid = (strpos(strtolower($comparison), strtolower($extension)) !== false) ? 1 : 0;
		}

		return ($valid == $operator);

	}//end evaluate()


	/**
	* writes JavaScript code for client side validation
	*
	* @param object form_question	&$q_asset		reference to the question asset
	* @param string					$operator		the operator being used to compare
	* @param mixed					&$comparison	either a value or a form_question asset (based on
	*												comparison question id), being used to compare
	*
	* @access public
	* @return boolean
	*/
	function generateJSCode(&$q_asset, $rule_data)
	{
		// Netscape and IE do not provide interfaces for accessing the
		// properties of file upload fields therefore no JS validation possible

		return '';

	}//end generateJSCode()


	/**
	* returns a default error message for this rule
	*
	* @param object Form_Question	&$q_asset	the question asset that failed
	* @param array					$rule_data	the rule data
	*
	* @return string	the error message
	* @access public
	*/
	function defaultError(&$q_asset, $rule_data)
	{
		$operator = array_get_index($rule_data, 'operator', 1);
		$comparison = array_get_index($rule_data, 'value', '');

		$comparison = str_replace("\r", '', $comparison);
		$comparison = str_replace("\n", ', ', $comparison);

		return translate('core_form_rule_file_type_error_'.($operator ? 'true' : 'false'), $q_asset->getValue(), $q_asset->attr('name'), $comparison);

		return $error_text;

	}//end defaultError()


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
		$operator = array_get_index($rule_data, 'operator', 1);
		$comparison = array_get_index($rule_data, 'value', '');

		$operator_list = $this->getOperators();

		return translate('core_form_rule_file_type_desc_'.($operator ? 'true' : 'false'), $q_asset->attr('name'), $rule_data['value']);

	}//end ruleDescription()


}//end class
?>
