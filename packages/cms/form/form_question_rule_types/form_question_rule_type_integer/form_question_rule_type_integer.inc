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
* $Id: form_question_rule_type_integer.inc,v 1.5 2004/06/21 06:54:51 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule.inc';

/**
* Form_Question_Rule_Type_Integer
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
class Form_Question_Rule_Type_Integer extends Form_Question_Rule
{


	/**
	* Constructor
	*
	* [Will we still need a constructor seeing we will only be calling this class statically?]
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form_Question_Rule_Type_Integer($assetid=0) 
	{
		$this->Form_Question_Rule($assetid);
	
	}//end Form_Question_Rule_Type_Integer()


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
		$attrs['no_comparison'] = true;
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
		return Array(1 => 'Must Be an Integer',
			0 => 'Must Not Be an Integer');
	
	}//end getOperators()
	
	
	/**
	* processes the form question and returns true if it is valid
	*
	* Because the Form Question Rule classes will not be instantiable, this function MUST called
	* statically.
	* 
	* @param object form_question	&$q_asset		reference to the question asset
	* @param string					$operator		the operator being used to compare
	*
	* @see Form_Question_Rule::processForm()
	* @access public
	* @return boolean
	*/
	function processForm(&$q_asset, $operator)
	{
		if (!isset($q_asset->active_submission)) return false;

		$answer   = $q_asset->active_submission[$q_asset->id];

		$valid = (is_numeric($answer) && ($answer == intval($answer))) ? 1 : 0; 

		if ($valid == $operator) {
			return true;
		} else {
			$error = $q_asset->name.' must '. (($operator) ? '' : 'not') .' be an Integer';
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
	*
	* @static
	* @access public
	* @return boolean true only if rule succeeds
	*/
	function valueTest($answer, $operator)
	{
		$valid = (is_numeric($answer) && ($answer == intval($answer))) ? 1 : 0; 

		return ($valid == $operator);

	}//end valueTest()
	
	
	/**
	* writes JavaScript code for client side validation
	* 
	* @param object form_question	&$q_asset		reference to the question asset
	* @param string					$operator		the operator being used to compare
	*
	* @access public
	* @return boolean
	*/
	function generateJSCode(&$q_asset, $operator)
	{
		// this assumes the skeleton of a JS validation function has been created around this code
		// with a 'form' variable defined (this should have been done when generating content file).
		$code = 'var answer = document.getElementById("'.$q_asset->id.'").value; ';

		$op = ($operator == 1) ? '!' : '';

		$regex_code = '/^[-+]?\d+$/';		// allow digits, leading +/-, no dec pt!

		$code .= 'answer.replace(/,/, ""); ';	// get rid of comma

		// this is a form question - get its current value from the form then
		$code .= 'if ('.$op.'answer.match('.$regex_code.')) {';	// START of js if
		$error = $q_asset->name.' must '. (($operator) ? '' : 'not ') .'be an integer';

		$code .= 'submission_errors[i] = "'.addslashes($error).'"; i++;'; // add submission error
		$code .= '}';	// END of js if
		return $code;

	}//end generateJSCode()
	
	
}//end class
?>
