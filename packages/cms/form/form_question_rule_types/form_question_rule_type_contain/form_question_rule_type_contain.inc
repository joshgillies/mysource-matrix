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
* $Id: form_question_rule_type_contain.inc,v 1.11 2004/06/15 23:15:07 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule.inc';

/**
* Form_Question_Rule_Type_Contain
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
class Form_Question_Rule_Type_Contain extends Form_Question_Rule
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
		return Array(1 => 'Must contain...',
			0 => 'Must Not contain...');
	
	}//end getOperators()
	
	
	/**
	* processes the form question and returns true if it is valid
	*
	* Because the Form Question Rule classes will not be instantiable by Matrix NOR will be
	* instantiated manually in code, this function MUST called statically.
	* 
	* @param object form_question	&$q_asset		reference to the question asset
	* @param string					$operator		the operator being used to compare
	* @param mixed					&$comparison	either a value or a form_question asset (based on
	*												comparison question id), being used to compare
	*
	* @see Form_Question_Rule::processForm()
	* @static
	* @access public
	* @return boolean
	*/
	function processForm(&$q_asset, $operator, &$comparison)
	{
		if (!$q_asset->active_submission) return false;
		$answer = $q_asset->getValue();

		// comparison sent is a question asset!!
		if (is_a($comparison, 'form_question')) {
			$valid = (strpos($answer, $comparison->getValue()) !== false) ? 1 : 0;			
		} else {	// sent just a value
			$valid = (strpos($answer, $comparison) !== false) ? 1 : 0;
		}

		if ($valid == $operator) {
			return true;
		} else {
			// tailor the error response depending
			if (is_a($comparison, 'form_question')) {
				$error = $q_asset->name.' must '. (($operator) ? '' : 'not') .' contain the value \''.$comparison->getValue().'\' ('.$comparison->name.' in '.$comparison->getMySectionName().')';
			} else {
				$error = $q_asset->name.' must '. (($operator) ? '' : 'not') .' contain \''.$comparison.'\'';
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
		// comparison sent is a question asset!!
		if (is_a($comparison, 'form_question')) {
			$valid = (strpos($answer, $comparison->getValue()) !== false) ? 1 : 0;			
		} else {	// sent just a value
			$valid = (strpos($answer, $comparison) !== false) ? 1 : 0;
		}

		return ($valid == $operator);
		
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
	* @static
	* @access public
	* @return string
	*/
	function generateJSCode(&$q_asset, $operator, &$comparison)
	{
		ob_start();			// buffer this please

		$code .= 'var answer = document.getElementById("'.$q_asset->id.'").value; ';

		if (is_a($comparison, 'form_question')) {
			$compare_value = 'document.getElementById("'.$comparison->id.'").value; ';
		} else {	// sent just a value
			$compare_value = '"'.$comparison.'"';
		}

		$op = ($operator == 1) ? '==' : '!=';

		// this is a form question - get its current value from the form then
		echo 'if (answer.indexOf('.$compare_value.') '.$op.' -1) {';	// START of js if
			if (is_a($comparison, 'form_question')) {
					$error = $q_asset->name.' must '.(($operator) ? '' : 'not') .' contain the value \''.$compare_value.'\' ('.$comparison->name.' in '.$comparison->getMySectionName().')';
			} else {
				$error = $q_asset->name.' must '. (($operator) ? '' : 'not') .' contain \''.$compare_value.'\'';
			}
		echo 'submission_errors[i] = "'.addslashes($error).'"; i++;'; // add submission error
		echo '}';	// END of js if

		$contents = ob_get_contents();		// get our crown jewels
		ob_end_clean();						// no more buffering please

		return $contents;

	}//end generateJSCode()
	
	
}//end class
?>
