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
* $Id: form_question_type_text.inc,v 1.10.4.1 2013/08/26 04:08:13 akarelia Exp $
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
* @version $Revision: 1.10.4.1 $
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
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		return Array();

	}//end _getAllowedLinks()


	/**
	* generates the html for this question to display it in the frontend
	*
	* @return string
	* @access public
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

		if ($this->attr('tabindex')) {
			$extras .= ' tabindex="'.$this->attr('tabindex').'"';
		}

		$value = $this->getValue();

		ob_start();
		if ($height == 1) {
			text_box($name, $value, $width, $max, FALSE, $extras);
		} else {
			text_area($name, $value, $width, $height, $max, $extras);
		}
		$html = ob_get_contents();
		ob_end_clean();

		// calling text_box will convert the open and close php tags into their html counter-parts
		// so we need to convert them back to proper php tags

		return $html;

	}//end getHtmlField()


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

		if ($this->attr('is_required') && $this->_isValidationRequired()) {
			$req_rule = Array('rule_code' => 'comparison', 'operator' => '!=', 'value' => '');
			array_push($this->vars['rules']['value'], $req_rule);
		}

		$ok = parent::hasValidValue($answer, $mute_errors);

		if ($this->attr('is_required') && $this->_isValidationRequired()) {
			array_pop($this->vars['rules']['value']);
		}

		return $ok;

	}//end hasValidValue()


	/**
	* writes JavaScript code for client side validation
	*
	* This function assumes the skeleton of a JS validation function has been created around this code
	* with a form variable named 'form' passed (this is done when generating the content file).
	* returns the javascript code used for client-side validation
	*
	* @return string
	* @access public
	*/
	function generateJSCode()
	{
		if ($this->attr('is_required')) {
			$req_rule = Array('rule_code' => 'comparison', 'operator' => '!=', 'value' => '');
			$rule_data = $this->attr('validation_condition');
			$req_rule = array_merge($req_rule, $rule_data);
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
	* @return array
	* @access public
	*/
	function getAllowedRules()
	{
		return Array('regexp', 'comparison', 'numeric', 'found_in', 'contain', 'length', 'email', 'begins_with', 'ends_with', 'word_count');

	}//end getAllowedRules()


	/**
	* returns an answer summary in XML format (suitable for submission logs)
	*
	* @param mixed	$answer	the answer for the question
	*
	* @return string
	* @access public
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
