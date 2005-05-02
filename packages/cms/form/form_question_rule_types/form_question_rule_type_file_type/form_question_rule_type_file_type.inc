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
* $Id: form_question_rule_type_file_type.inc,v 1.1.2.2 2005/05/02 02:44:57 gsherwood Exp $
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
* @version $Revision: 1.1.2.2 $
* @package MySource_Matrix_Packages
* @subpackage cms
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
				1	=> 'Must be one of these extensions...',
				0	=> 'Must not be one of these extensions...',
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
	function valueTest($answer, $operator, &$comparison)
	{
		if (strrpos($answer, '.') === false) {
			$extension = '';
		} else {
			$extension = substr($answer, strrpos($answer, '.'));
		}

		$valid = (strpos(strtolower($comparison), strtolower($answer)) !== false) ? 1 : 0;

		return ($valid == $operator);


	}//end valueTest()


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

		if (strrpos($answer, '.') === false) {
			$extension = '';
		} else {
			$extension = substr($answer, strrpos($answer, '.') + 1);
		}

		$valid = (strpos(strtolower($comparison), strtolower($extension)) !== false) ? 1 : 0;

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

		$error_text = 'Uploaded file name "'.$q_asset->getValue().'" at question "'.$q_asset->attr('name').'" is invalid - extension must '.($operator ? '' : 'not ').'be one of "'.$comparison.'"';

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

		return 'Uploaded file at "'.$q_asset->attr('name').'" '.strtolower($operator_list[array_get_index($rule_data, 'operator', 1)]).' '.$rule_data['value'];

	}//end ruleDescription()


}//end class
?>
