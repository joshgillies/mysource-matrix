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
* $Id: form_question_rule_type_select_limit.inc,v 1.4 2004/06/22 00:20:32 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule.inc';

/**
* Form_Question_Rule_Type_Select_Limit
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
class Form_Question_Rule_Type_Select_Limit extends Form_Question_Rule
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
			'==' => 'Must select exactly this many options...',
			'!=' => 'Must not select this many options...',
			'>'  => 'Must select more than this many options...',
			'<'  => 'Must select less than this many options...',
			'>=' => 'Must select at least this many options...',
			'<=' => 'Must select no more than this many options...',
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
			'==' => 'must have exactly ',
			'!=' => 'must not have ',
			'>'  => 'must have more than ',
			'<'  => 'must have less than ',
			'>=' => 'must have no less than ',
			'<=' => 'must have no more than ',
		);

		return $operators;

	}//end _getOperators()


	/**
	* Returns an array of definition variables in an array
	*
	* Since we are not instantiating the question rules, we do not have access to member variables
	* (at least not until PHP5). So this overrides the usual attribute gathering function.
	* Those methods wishing to override this function should call its parent first then modify the
	* returned array.
	*
	* @access public
	* @return array (string => mixed) an array of parameter values indexed by a parameter id
	*/
	function getAttributes()
	{
		$attrs = parent::getAttributes();
		$attrs['force_text'] = true;
		return $attrs;

	}//end getAttributes()
	
	
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

		$operators = eval('return '.__CLASS__.'::_getOperators();');	// get private operator texts

		// comparison sent is a question asset!!
		if (is_a($comparison, 'form_question')) {
			$valid = eval('return \''.count($answer).'\''.$operator.'\''.$comparison->getValue().'\';');
		} else {	// sent just a value
			$valid = eval('return \''.count($answer).'\''.$operator.'\''.$comparison.'\';');
		}

		if ($valid == $operator) {
			return true;
		} else {
			// tailor the error response depending
			if (is_a($comparison, 'form_question')) {
				$error = $q_asset->name.' '.$operators[$operator].' '.$comparison->getValue(). ' options selected ('.$comparison->_getName().' in '.$comparison->getMySectionName().')';
			} else {
				$error = '"' . $q_asset->name.'" '.$operators[$operator].' '.$comparison . ' options selected';
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
		$operators = eval('return '.__CLASS__.'::_getOperators();');	// get private operator texts

		// comparison sent is a question asset!!
		if (is_a($comparison, 'form_question')) {
			$valid = eval('return \''.count($answer).'\''.$operator.'\''.$comparison->getValue().'\';');
		} else {	// sent just a value
			$valid = eval('return \''.count($answer).'\''.$operator.'\''.$comparison.'\';');
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
	* @access public
	* @return boolean
	*/
	function generateJSCode(&$q_asset, $operator, &$comparison)
	{
		$code = '';

		$q_id = $q_asset->id . ($q_asset->attr('multiple') ? '[]' : '');

		$code .= 'var select = document.getElementById("'.$q_id.'"); ';

		$operators = eval('return '.__CLASS__.'::_getOperators();');	// get private operator texts

		if (is_a($comparison, 'form_question')) {
			$compare_value = 'document.getElementById("'.$comparison->id.'").value';
		} else {	// sent just a value
			$compare_value = $comparison;
		}

		$code .= 'var selectCount = 0; ';

		// since this is going to be a generic rule type applicable to both select lists and tickbox
		// lists, we need different types of magic for each possible question type.

		// For a selection list (and descendants) we need something like this.......
		if (is_a($q_asset, 'form_question_type_select')) {
			$loop_code = ' for(i=0; i<select.options.length; i++) {if (select.options[i].selected) {selectCount++;} }';
		} else {			// tickbox (or option) list needs something else
			$loop_code = 'for(i=0; i<select.elements.length; i++) {if (select.elements[i].checked) {selectCount++;} }';
		}

		$code .= $loop_code;		// add loop code

		// this is a form question - get its current value from the form then
		$code .= 'if (!(selectCount '.$operator.' '.$compare_value.')) {';	// START of js if
			if (is_a($comparison, 'form_question')) {
					$error = $q_asset->name.' '.$operators[$operator].' '.$compare_value. ' options selected ('.$comparison->_getName().' in '.$comparison->getMySectionName().')';
			} else {
				$error = $q_asset->name.' '.$operators[$operator].' '.$compare_value . ' options selected';
			}
		$code .= 'submission_errors[i] = "'.addslashes($error).', "+selectCount+" selected"; i++;'; // add submission error
		$code .= '}';	// END of js if

		return $code;

	}//end generateJSCode()


}//end class
?>
