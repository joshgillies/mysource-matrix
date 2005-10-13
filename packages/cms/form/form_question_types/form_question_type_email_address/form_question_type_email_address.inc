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
* $Id: form_question_type_email_address.inc,v 1.16.2.1 2005/10/13 04:14:12 sdanis Exp $
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
* @version $Revision: 1.16.2.1 $
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

	}//end constructor


	/**
	* returns an array of the types of rules
	*
	* Returned array is 'short name' => 'full name'
	*
	* @access public
	* @return Array(string => string)
	*/
	function getAllowedRules()
	{
		return Array('comparison', 'found_in', 'contain', 'length', 'begins_with', 'ends_with');

	}//end getAllowedRules()


	/**
	* Look at user-specified and internal rules to see if this question currently has a valid value
	*
	* @access public
	* @return boolean
	*/
	function hasValidValue()
	{
		$value = $this->getValue();
		if (empty($value)) return parent::hasValidValue();

		// Since we know the answer is not empty, borrow a 'valid email address' rule
		$req_rule = Array('rule_code' => 'email', 'operator' => '1');
		array_push($this->vars['rules']['value'], $req_rule);
		$ok = parent::hasValidValue();
		array_pop($this->vars['rules']['value']);
		return $ok;

	}//end hasValidValue()


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
