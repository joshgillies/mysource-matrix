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
* $Id: form_question_type_email_address.inc,v 1.9 2004/07/04 22:53:38 lwright Exp $
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
class Form_Question_Type_Email_Address extends Form_Question_Type_Text
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	* @param array	$data		the section's entry for this question (contains all question data)
	*
	*/
	function Form_Question_Type_Email_Address($assetid=0, $data=Array()) 
	{
		$this->Form_Question_Type_Text($assetid, $data);

	}//end Form_Question_Type_Email_Address()


	/**
	* returns an array of the types of rules 
	*
	* Returned array is 'short name' => 'full name'
	*
	* @access public
	* @return Array(string => string)
	*/
	function getAllowedRules() {
		return Array('comparison', 'found_in', 'contain', 'length', 'begins_with', 'ends_with');
	
	}//end getAllowedRules()
	
	
	/**
	* process this question
	*
	* @param &object $submission the current submission object
	*
	* @access public
	* @return boolean
	*/
	function processForm()
	{
		
		$form =& $this->getMySection();

		if (isset($_POST[$this->id])) {
			$val = $_POST[$this->id];
			$form->active_submission[$this->id] = $val;

			// borrow a 'valid email address' rule
			$req_rule = Array('rule_code' => 'email', 'operator' => '1');
			array_push($this->vars['rules']['value'], $req_rule);

			parent::processForm();

			array_pop($this->vars['rules']['value']);
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
		// borrow a valid email rule
		$req_rule = Array('rule_code' => 'email', 'operator' => '1');
		array_push($this->vars['rules']['value'], $req_rule);

		$code = parent::generateJSCode();

		array_pop($this->vars['rules']['value']);

		return $code;

	}//end generateJSCode()


}//end class
?>
