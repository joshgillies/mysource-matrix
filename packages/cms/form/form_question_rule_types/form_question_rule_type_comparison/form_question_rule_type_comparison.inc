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
* $Id: form_question_rule_type_comparison.inc,v 1.18 2004/09/23 02:00:25 arailean Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule.inc';

/**
* Form_Question_Rule_Type_Comparison
*
* Purpose
*
*
* @static
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Rule_Type_Comparison extends Form_Question_Rule
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
			'==' => 'Must Be Equal to ',
			'!=' => 'Must Not Be Equal to ',
			'>'  => 'Must be Greater Than ',
			'<'  => 'Must be Less Than ',
			'>=' => 'Must be Greater Than or Equal To ',
			'<=' => 'Must be Less Than or Equal To ',
		);

		return $operators;

	}//end getOperators()


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
		if (!isset($q_asset->active_submission)) return false;
		$answer = $q_asset->getValue();

		$operators = eval('return '.__CLASS__.'::getOperators();');

		// comparison sent is a question asset!!
		if (is_a($comparison, 'form_question')) {
			$comp_value = $comparison->getValue();
		} else {	// sent just a value
			$comp_value = $comparison;
		}

		$answer     = addslashes($answer);
		$comp_value = addslashes($comp_value);

		$expression = "return '$answer' $operator '$comp_value';";

		if (eval($expression)) {
			return true;
		} else {
			// tailor the error response depending
			if (is_a($comparison, 'form_question')) {
				$error = $q_asset->name.' '.$operators[$operator].
			' '.$comparison->_getName().' in '.$comparison->getMySectionName().' (\''.$comparison->getValue().'\')';
			} else {
				if ($q_asset->isSelection()) {
					$options = &$q_asset->getOptions();
					$comparison = '"'.$options[(int)$comparison].'"';
				}
				// special case for "not equal to empty string" which is basically 'Is Required' rule
				if (($operator == '!=') || ($comparison == '')) {
					$error = $q_asset->name.' must be filled in';
				} else {
					$error = $q_asset->name.' '.$operators[$operator].' '.$comparison;
				}
			}
			$q_asset->submission_errors[] = $error;
			return false;
		}

	}//end processForm()


	/**
	* processes a test on a single static value
	*
	* Used when we are using rules outside of validating a form - for example selective emails.
	*
	* Because the Form Question Rule classes will not be instantiable by Matrix NOR will be
	* instantiated manually in code, this function MUST called statically.
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
		$operators = eval('return '.__CLASS__.'::getOperators();');

		// comparison sent is a question asset!!
		if (is_a($comparison, 'form_question')) {
			$valid = eval('return \''.$answer.'\''.$operator.'\''.$comparison->getValue().'\';');
		} else {	// sent just a value
			$valid = eval('return \''.$answer.'\''.$operator.'\''.$comparison.'\';');
		}

		return $valid;

	}//end valueTest()


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

		if ($q_asset->type() == 'form_question_type_numeric') {
			$code .= 'answer = parseFloat(answer); ';
		}

		$operators = eval('return '.__CLASS__.'::getOperators();');

		if (is_a($comparison, 'form_question')) {
			$compare_value = 'document.getElementById("'.$comparison->id.'").value';
		} else {	// sent just a value
			$compare_value = '"'.$comparison.'"';
		}

		// I don't care what the comparison value is, if our question is numeric than we are
		// comparing it as a number!
		if ($q_asset->type() == 'form_question_type_numeric') {
			$compare_value_parsed = 'parseFloat('.$compare_value.')';
		} else {
			$compare_value_parsed = $compare_value;
		}

		// this is a form question - get its current value from the form then
		$code .= 'if (!(answer '.$operator.' '.$compare_value_parsed.')) {';	// START of js if
			if (is_a($comparison, 'form_question')) {
					$error = $q_asset->name.' '.$operators[$operator].
						' '.$comparison->_getName().' in '.$comparison->getMySectionName().'
						(\''.$compare_value.'\')';
			} else {
				if ($q_asset->isSelection()) {
					$options = &$q_asset->getOptions();
					$compare_value = '"'.$options[(int)$comparison].'"';
				}
				// special case for "not equal to empty string" which is basically 'Is Required' rule
				if (($operator == '!=') || ($compare_value == '')) {
					$error = $q_asset->name.' must be filled in';
				} else {
					$error = $q_asset->name.' '.$operators[$operator].' '.$compare_value;
				}
			}
		$code .= 'submission_errors[i] = "'.addslashes($error).'"; i++;'; // add submission error
		$code .= '}';	//end of js if

		return $code;

	}//end generateJSCode()


}//end class
?>
