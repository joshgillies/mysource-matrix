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
* $Id: form_question_rule_type_email.inc,v 1.12 2004/07/07 01:05:29 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule.inc';
require_once SQ_FUDGE_PATH .'/general/www.inc';

/**
* Form_Question_Rule_Type_Email
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
class Form_Question_Rule_Type_Email extends Form_Question_Rule
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form_Question_Rule_Type_Email($assetid=0) 
	{
		$this->Form_Question_Rule($assetid);
	
	}//end Form_Question_Rule_Type_Email()


	/**
	* Returns an array of definition variables in an array
	*
	* Since we are not instantiating the question rules, we do not have access to member variables
	* (at least not until PHP5). So this overrides the usual attribute gathering function.
	* Those methods wishing to override this function should call its parent first then modify the
	* returned array.
	*
	* @access public
	* @return array (string => mixed)
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
		return Array(1 => 'Must Be a Valid E-mail Address',
			0 => 'Must Not Be a Valid E-mail Address');
	
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
		$answer   = $q_asset->active_submission[$q_asset->id];

		$valid = valid_email($answer);

		if ($valid == $operator) {
			return true;
		} else {
			$error = $q_asset->name.' must '. (($operator) ? '' : 'not') .' be a valid Email Address';
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
		$valid = valid_email($answer);

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

		// regex code from fudge valid_email()
		$regex_code = '/^[_\.0-9a-zA-Z-]+@([0-9a-zA-Z]([0-9A-Za-z-]*)+\.)+[a-zA-Z]{2,4}$/';

		$code .= 'if ('.$op.'answer.match('.$regex_code.')) {';	// START of js if
		$error = $q_asset->name.' must '. (($operator) ? '' : 'not ') .'be an valid e-mail address';

		$code .= 'submission_errors[i] = "'.addslashes($error).'"; i++;'; // add submission error
		$code .= '}';	//end of js if
		return $code;

	}//end generateJSCode()


}//end class
?>
