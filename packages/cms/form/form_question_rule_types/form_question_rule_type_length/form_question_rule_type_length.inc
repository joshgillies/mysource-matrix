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
* $Id: form_question_rule_type_length.inc,v 1.1 2004/05/05 07:10:13 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule.inc';

/**
* Form_Question_Rule_Type_Length
*
* Purpose
*
*
* @static
* @author  Luke Wright <lwright@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Rule_Type_Length extends Form_Question_Rule
{


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
			'==' => 'Must be this many characters in length...',
			'!=' => 'Must not be this many characters in length...',
			'>'  => 'Length must be greater than this many characters...',
			'<'  => 'Length must be less than this many characters...',
			'>=' => 'Length must be at least this many characters...',
			'<=' => 'Length must be no more than this many characters...',
		);

		return $operators;

	}//end getOperators()


	/**
	* Returns a list of operators for this rule type, for use with error messages
	*
	* This extra operator function is declared in this case so that we can easily create a legible rule
	* using the public function then use this for the error messages.
	*
	* @access private
	* @return array (mixed => string) an array of descriptions indexed by operator id (which can be
	*                                 anything, generally string or (int)0|1 for a boolean value)
	*/
	function _getOperators()
	{
		$operators = Array(
			'==' => 'Must Be Equal to ',
			'!=' => 'Must Not Be Equal to ',
			'>'  => 'Must be Greater Than ',
			'<'  => 'Must be Less Than ',
			'>=' => 'Must be No Less Than ',
			'<=' => 'Must be No More Than ',
		);

		return $operators;

	}//end _getOperators()


	/**
	* processes the form question and returns true if it is valid
	*
	* Because the Form Question Rule classes will not be instantiable by Matrix NOR will be
	* instantiated manually in code, this function MUST called statically. (Hence also why we have
	* manually pass question assets all the time - don't think static variables would work here)
	* 
	* @param object form_question	&$q_asset		reference to the question asset
	* @param string					$operator		the operator being used to compare
	* @param mixed					&$comparison	either a value or a form_question asset (based on
	*												comparison question id), being used to compare
	*
	* @see Form_Question_Rule::processForm()
	* @access public
	* @return boolean
	*/
	function processForm(&$q_asset, $operator, &$comparison)
	{
		if (!$q_asset->active_submission) return false;
		$answer = $q_asset->getValue();

		$operators = eval('return '.__CLASS__.'::_getOperators();');	// get private operator texts

		// comparison sent is a question asset!!
		if (is_a($comparison, 'form_question')) {
			$valid = eval('return \''.strlen($answer).'\''.$operator.'\''.$comparison->getValue().'\';');
		} else {	// sent just a value
			$valid = eval('return \''.strlen($answer).'\''.$operator.'\''.$comparison.'\';');
		}

		if ($valid == $operator) {
			return true;
		} else {
			// tailor the error response depending on whether comparison is against question or static
			if (is_a($comparison, 'form_question')) {
				$error = $q_asset->name.' '.$operators[$operator].' '.$comparison->getValue(). ' characters in length ('.$comparison->_getName().' in '.$comparison->getMySectionName().')';
			} else {
				$error = $this->my_question->name.' '.$operators[$operator].' '.$comparison . ' characters in length';
			}
			$q_asset->submission_errors[] = $error;
			return false;
		}
		
	}//end processForm()


	/**
	* writes JavaScript code for client side validation
	*
	* This function assumes the skeleton of a JS validation function has been created around this code
	* with a form variable named 'form' passed (this is done when generating the content file).
	*
	* Because the Form Question Rule classes will not be instantiable by Matrix NOR will be
	* instantiated manually in code, this function MUST called statically.
	* 
	* @param object form_question	&$q_asset		reference to the question asset
	* @param string					$operator		the operator being used to compare
	* @param mixed					&$comparison	either a value or a form_question asset (based on
	*												comparison question id), being used to compare
	*
	* @access public
	* @return boolean
	*/
	function generateJSCode(&$q_asset, $operator, &$comparison)
	{
		$code = '';

		$code .= 'var answer = document.getElementById("'.$q_asset->id.'").value; ';

		$operators = eval('return '.__CLASS__.'::_getOperators();');	// get private operator texts

		if (is_a($comparison, 'form_question')) {
			$compare_value = 'document.getElementById("'.$comparison->id.'").value; ';
		} else {	// sent just a value
			$compare_value = '"'.$comparison.'"';
		}

		// this is a form question - get its current value from the form then
		$code .= 'if (!(answer.length '.$operator.' '.$compare_value.')) {';	// START of js if
			if (is_a($comparison, 'form_question')) {
					$error = $q_asset->name.' '.$operators[$operator].' '.$compare_value. ' characters in length ('.$comparison->_getName().' in '.$comparison->getMySectionName().')';
			} else {
				$error = $q_asset->name.' '.$operators[$operator].' '.$compare_value . ' characters in length';
			}
		$code .= 'submission_errors[i] = "'.addslashes($error).'"; i++;'; // add submission error
		$code .= '}';	// END of js if

		return $code;

	}//end generateJSCode()


}//end class
?>
