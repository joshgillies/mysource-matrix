<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: form_question_rule_type_file_type.inc,v 1.7 2007/12/20 22:06:04 bshkara Exp $
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
* @version $Revision: 1.7 $
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
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Returns a list of operators for this rule type
	*
	* @access public
	* @return array (mixed => string) an array of descriptions indexed by operator id (which can be
	*                                 anything, generally string or (int)0|1 for a boolean value)
	*/
	public static function getOperators()
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
		if (empty($answer) || $answer == 'No file uploaded') return true;
		$operator = array_get_index($rule_data, 'operator', 1);
		$comparison = array_get_index($rule_data, 'value', '');

		if (strrpos($answer, '.') === false) {
			$extension = '';
		} else {
			$extension = substr($answer, strrpos($answer, '.') + 1);
		}

		// If the extension is empty, return as invalid, otherwise strpos will not take empty argument. 
		if (empty($extension) && !empty($comparison)) {
			$valid = 0;
		} else {
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
