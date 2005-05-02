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
* $Id: form_question_type_numeric.inc,v 1.7.2.2 2005/05/02 02:44:57 gsherwood Exp $
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
* @version $Revision: 1.7.2.2 $
* @package MySource_Matrix_Packages
* @subpackage cms
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
	* @access public
	* @return Array(string => string)
	*/
	function getAllowedRules()
	{
		return Array('comparison', 'integer', 'length');

	}//end getAllowedRules()


	/**
	* process this question
	*
	* @access public
	* @return boolean
	*/
	function evaluate($answer=null)
	{
		$val = $this->getValue();

		if (!empty($val)) {
			$req_rule = Array('rule_code' => 'numeric', 'operator' => 1);
			array_unshift($this->vars['rules']['value'], $req_rule);

			// Add Force Integer if required
			if ($this->attr('is_int')) {
				$req_rule = Array('rule_code' => 'integer', 'operator' => 1);
				array_unshift($this->vars['rules']['value'], $req_rule);
			}
		}

		$ok = parent::evaluate();

		if (!empty($val)) {
			if ($this->attr('is_int')) {						// Remove Force Integer
				array_shift($this->vars['rules']['value']);
			}
			array_shift($this->vars['rules']['value']);			// Remove Force Numeric in any case
		}

		return $ok;

	}//end evaluate()


	/**
	* writes JavaScript code for client side validation
	*
	* This function assumes the skeleton of a JS validation function has been created around this code
	* with a form variable named 'form' passed (this is done when generating the content file).
	*
	* @access public
	* @return string the javascript code used for client-side validation
	*/
	function generateJSCode()
	{
		// Add Force Numeric in any case
		$req_rule = Array('rule_code' => 'numeric', 'operator' => 1);
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
