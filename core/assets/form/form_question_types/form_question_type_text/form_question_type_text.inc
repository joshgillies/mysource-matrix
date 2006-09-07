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
* $Id: form_question_type_text.inc,v 1.3.2.1 2006/09/07 02:09:47 bcaldwell Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question/form_question.inc';


/**
* Form_Question_Type_Text
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Revision: 1.3.2.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Type_Text extends Form_Question
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	* @param array	$data		the section's entry for this question (contains all question data)
	*
	*/
	function Form_Question_Type_Text($assetid=0, $data=Array())
	{
		$this->Form_Question($assetid, $data);

	}//end constructor


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array();

	}//end _getAllowedLinks()


	/**
	* generates the html for this question to display it in the frontend
	*
	* @access public
	* @return string
	*/
	function getHtmlField()
	{
		$height = ($this->attr('height')) ? $this->attr('height') : 1;
		$width  = $this->attr('width');
		$max    = $this->attr('max');
		$extras = $this->attr('extra');
		$name   = 'q'.$this->id;

		// prepare a valid value for the field id
		$extras = $extras.' id="'.str_replace(':', '_', $name).'"';

		$value = $this->getValue();
		$value  = htmlspecialchars($value);

		ob_start();
		if ($height == 1) {
			text_box($name, $value, $width, $max, false, $extras);
		} else {
			text_area($name, $value, $width, $height, $max, $extras);
		}
		$html = ob_get_contents();
		ob_end_clean();

		// calling text_box will convert the open and close php tags into their html counter-parts
		// so we need to convert them back to proper php tags

		return html_entity_decode($html);

	}//end getHtmlField()


	/**
	* Look at user-specified and internal rules to see if this question currently has a valid value
	*
	* @access public
	* @return boolean
	*/
	function hasValidValue($answer=NULL)
	{
		if (is_null($answer)) {
			$answer = $this->getValue();
		}

		if ($this->attr('is_required')) {
			$req_rule = Array('rule_code' => 'comparison', 'operator' => '!=', 'value' => '');
			array_push($this->vars['rules']['value'], $req_rule);
		}

		$ok = parent::hasValidValue($answer);

		if ($this->attr('is_required')) {
			array_pop($this->vars['rules']['value']);
		}

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
		if ($this->attr('is_required')) {
			$req_rule = Array('rule_code' => 'comparison', 'operator' => '!=', 'value' => '');
			array_push($this->vars['rules']['value'], $req_rule);
		}

		$code = parent::generateJSCode();

		if ($this->attr('is_required')) {
			array_pop($this->vars['rules']['value']);
		}

		return $code;

	}//end generateJSCode()


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
		return Array('regexp', 'comparison', 'numeric', 'found_in', 'contain', 'length', 'email', 'begins_with', 'ends_with');

	}//end getAllowedRules()


	/**
	* returns an answer summary in XML format (suitable for submission logs)
	*
	* @param mixed	$answer	the answer for the question
	*
	* @access public
	* @return string the summary XML
	*/
	function getXML($answer)
	{
		ob_start();

		echo '<text_q id="'.addslashes($this->id).'"  name="'.htmlspecialchars($this->attr('name')).'">';
		echo htmlspecialchars($answer);
		echo '</text_q>';

		$contents = ob_get_contents();
		ob_end_clean();

		return $contents;

	}//end getXML()


}//end class
?>
