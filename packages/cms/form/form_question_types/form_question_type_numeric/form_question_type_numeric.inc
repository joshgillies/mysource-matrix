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
* $Id: form_question_type_numeric.inc,v 1.5 2004/11/05 00:08:50 lwright Exp $
* $Name: not supported by cvs2svn $
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
* @version $Version$ - 1.0
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

	}//end Form_Question_Type_Numeric()


	/**
	* returns an array of the types of rules that are allowed upon this asset
	*
	* Returned array is 'short name' => 'full name'
	*
	* @access public
	* @return Array(string => string)
	*/
	function getAllowedRules() {
		return Array('comparison', 'integer', 'length');
	
	}//end getAllowedRules()


	/**
	* process this question
	*
	* @access public
	* @return boolean
	*/
	function processForm()
	{
		$form =& $this->getMySection();

		if (isset($_POST[$this->id])) {
			$val = $_POST[$this->id];

			// Add Force Numeric in any case
			$req_rule = Array('rule_code' => 'numeric', 'operator' => 1);
			array_push($this->vars['rules']['value'], $req_rule);

			// Add Force Integer if required
			if ($this->attr('is_int')) {
				$req_rule = Array('rule_code' => 'integer', 'operator' => 1);
				array_push($this->vars['rules']['value'], $req_rule);
			}

			$form->active_submission->setValue($this->id, $val);
			$form->active_submission->setSummary($this->id,  $this->attr('name'), $val);

			parent::processForm();

			if ($this->attr('is_int')) {						// Remove Force Integer
				array_pop($this->vars['rules']['value']);
			}
			array_pop($this->vars['rules']['value']);			// Remove Force Numeric in any case

		}
		
		return true;
	
	}//end processForm()

	
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
		array_push($this->vars['rules']['value'], $req_rule);

		// Add Force Integer if required
		if ($this->attr('is_int')) {
			$req_rule = Array('rule_code' => 'integer', 'operator' => 1);
			array_push($this->vars['rules']['value'], $req_rule);
		}

		$code = parent::generateJSCode();

		if ($this->attr('is_int')) {						// Remove Force Integer
			array_pop($this->vars['rules']['value']);
		}
		array_pop($this->vars['rules']['value']);			// Remove Force Numeric in any case

		return $code;

	}//end generateJSCode()


}//end class
?>
