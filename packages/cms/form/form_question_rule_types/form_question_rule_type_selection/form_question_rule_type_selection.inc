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
* $Id: form_question_rule_type_selection.inc,v 1.1 2004/10/12 01:35:03 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule.inc';

/**
* Form_Question_Rule_Type_Selection
*
* Purpose
*
*
* @static
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Rule_Type_Selection extends Form_Question_Rule
{


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
		$attrs['show_asset_finder'] = false;
		$attrs['validation_rule'] = false;
		return $attrs;

	}//end getAttributes()
	
	
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
			'1' => 'Option must be selected: ',
			'0' => 'Option must not be selected: ',
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
			'1' => 'must be selected',
			'0' => 'must not be selected',
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
		if (!isset($q_asset->active_submission)) return false;
		$answer = $q_asset->getValue();

		$operators = eval('return '.__CLASS__.'::_getOperators();');	// get private operator texts

		$valid = (array_search($comparison, $answer) !== false);

		if ($valid == $operator) {
			return true;
		} else {
			// tailor the error response depending
			$error = $q_asset->name.$this->_getOperators[$operator];
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

		$valid = (array_search($comparison, $answer) !== false);

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

	}//end generateJSCode()


}//end class
?>
