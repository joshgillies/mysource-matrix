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
* $Id: form_question_type_password.inc,v 1.6 2006/09/07 03:50:10 bcaldwell Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question/form_question.inc';


/**
* Form_Question_Type_Password
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Type_Password extends Form_Question
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	* @param array	$data		the section's entry for this question (contains all question data)
	*
	*/
	function Form_Question_Type_Password($assetid=0,$data=Array())
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
		$width  = $this->attr('width');
		$max    = $this->attr('max');
		$extras = $this->attr('extra');
		$name_a   = 'q'.$this->id;
		$name_b  = 'q'.$this->id.'_2';

		// prepare a valid value for the field id
		$extras_a = $extras.' id="'.str_replace(':', '_', $name_a).'"';
		$extras_b = $extras.' id="'.str_replace(':', '_', $name_b).'"';

		$value = $this->attr('default');

		ob_start();
		password_box($name_a, $value, $width, $max, $extras_a);
		if ($this->attr('verify')) {
			echo '<br />';
			password_box($name_b, $value, $width, $max, $extras_b);
		}
		$html = ob_get_contents();
		ob_end_clean();

		// calling text_box will convert the open and close php tags into their html counter-parts
		// so we need to convert them back to proper php tags

		return html_entity_decode($html);

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
	* @access public
	* @return boolean
	*/
	function hasValidValue($answer=NULL, $mute_errors=FALSE)
	{
		if (is_null($answer)) {
			$answer = $this->getValue();
		}

		if ($this->attr('verify') && ($val != $this->_tmp['verify_value'])) {
			$this->failed_rules[] = translate('core_form_password_entered_differently', $this->attr('name'));
		}
		if (strlen($val) < $this->attr('min')) {
			$this->failed_rules[] = translate('core_form_password_length_>=', $this->attr('name'), $this->attr('min'));
		}
		if (strlen($val) > $this->attr('max')) {
			$this->failed_rules[] = translate('core_form_password_length_<=', $this->attr('name'), $this->attr('max'));
		}

		if (!empty($this->failed_rules)) return FALSE;

		return parent::hasValidValue($answer, $mute_errors);

	}//end hasValidValue()


	/**
	* Populates the question response from the form data
	*
	* @return boolean
	* @access public
	*/
	function populate()
	{
		$name = 'q'.$this->id;

		if (isset($_POST[$name])) {
			$this->setValue($_POST[$name]);
			if ($this->attr('verify')) {
				$this->_tmp['verify_value'] = $_POST[$name.'_2'];
			}
		}
		return true;

	}//end populate()


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

		// save as a text question for now, we may need a better idea here
		echo '<text_q id="'.addslashes($this->id).'"  name="'.htmlspecialchars($this->attr('name')).'">';
		echo htmlspecialchars($answer);
		echo '</text_q>';

		$contents = ob_get_contents();
		ob_end_clean();

		return $contents;

	}//end getXML()


	/**
	* returns the answer for this question for the summary, generally overidden by children
	*
	* @access public
	* @return string
	*/
	function getSummary()
	{
		return '';

	}//end getSummary()


	/**
	* Set the value of this question
	*
	* @access public
	* @return void
	*/
	function setValue($value)
	{
		$this->value = $value;
		if ($this->attr('verify')) {
			$this->_tmp['verify_value'] = $value;
		}

	}//end setValue()


	/**
	* Returns whether questions of this type can be edited once the form has been initially submitted
	*
	* @access public
	* @return boolean
	*/
	function isEditable()
	{
		return false;

	}//end isEditable()


}//end class
?>