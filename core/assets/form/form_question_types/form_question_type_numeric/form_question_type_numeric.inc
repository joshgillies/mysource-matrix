<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: form_question_type_numeric.inc,v 1.7 2013/04/19 06:36:02 ewang Exp $
*
*/


require_once dirname(__FILE__).'/../form_question_type_text/form_question_type_text.inc';
require_once SQ_FUDGE_PATH.'/general/www.inc';


/**
* Form_Question_Type_Email_Address
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Type_Numeric extends Form_Question_Type_Text
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	* @param array	$data		the section's entry for this question (contains all question data)
	*
	*/
	function Form_Question_Type_Numeric($assetid=0, $data=Array())
	{
		$this->Form_Question_Type_Text($assetid, $data);

	}//end constructor


	/**
	* returns an array of the types of rules that are allowed upon this asset
	*
	* Returned array is 'short name' => 'full name'
	*
	* @return array
	* @access public
	*/
	function getAllowedRules()
	{
		return Array('comparison', 'integer', 'length');

	}//end getAllowedRules()


	/**
	* Look at user-specified and internal rules to see if this question
	* currently has a valid value
	*
	* Returns TRUE if all validation rules pass
	*
	* @param mixed		$answer			Custom value to compare with - if left
	*									NULL (or unspecified), use the current
	*									value of the question
	* @param boolean	$mute_errors	If TRUE, do not add to the "Failed
	*									Rules" log, but simply return FALSE
	*
	* @return boolean
	* @access public
	*/
	function hasValidValue($answer=NULL, $mute_errors=FALSE)
	{
		if (is_null($answer)) $answer = $this->getValue();

		if (!empty($answer)) {
			$req_rule = Array('rule_code' => 'numeric', 'operator' => 1);
			array_unshift($this->vars['rules']['value'], $req_rule);

			// Add Force Integer if required
			if ($this->attr('is_int')) {
				$req_rule = Array('rule_code' => 'integer', 'operator' => 1);
				array_unshift($this->vars['rules']['value'], $req_rule);
			}
		}

		$ok = parent::hasValidValue($answer, $mute_errors);

		if (!empty($answer)) {
			if ($this->attr('is_int')) {						// Remove Force Integer
				array_shift($this->vars['rules']['value']);
			}
			array_shift($this->vars['rules']['value']);			// Remove Force Numeric in any case
		}

		return $ok;

	}//end hasValidValue()


	/**
	* writes JavaScript code for client side validation
	*
	* This function assumes the skeleton of a JS validation function has been created around this code
	* with a form variable named 'form' passed (this is done when generating the content file). It
	* returns the javascript code used for client-side validation
	*
	* @return string
	* @access public
	*/
	function generateJSCode()
	{
		// Add Force Numeric in any case
		$req_rule = Array('rule_code' => 'numeric', 'operator' => 1);
		$rule_data = $this->attr('validation_condition');
		$req_rule = array_merge($req_rule, $rule_data);
		array_unshift($this->vars['rules']['value'], $req_rule);

		// Add Force Integer if required
		if ($this->attr('is_int')) {
			$req_rule = Array('rule_code' => 'integer', 'operator' => 1);
			array_unshift($this->vars['rules']['value'], $req_rule);
		}

		$code = parent::generateJSCode();

		if ($this->attr('is_int')) {						// Remove Force Integer
			array_shift($this->vars['rules']['value']);
		}
		array_shift($this->vars['rules']['value']);			// Remove Force Numeric in any case

		return $code;

	}//end generateJSCode()


}//end class
?>
