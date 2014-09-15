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
* $Id: form_question_type_password.inc,v 1.12 2013/04/08 07:52:53 ewang Exp $
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
* @version $Revision: 1.12 $
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
		$width  = $this->attr('width');
		$max    = $this->attr('max');
		$extras = $this->attr('extra');
		$name_a   = 'q'.$this->id;
		$name_b  = 'q'.$this->id.'_2';
		$title = $this->attr('name');
		$field_name = $this->attr('field_name');
		$confirm_field_name = $this->attr('confirm_field_name');

		// prepare a valid value for the field id
		$extras_a = $extras.' id="'.str_replace(':', '_', $name_a).'"';
		$extras_b = $extras.' id="'.str_replace(':', '_', $name_b).'"';

		if ($this->attr('tabindex')) {
			$extras_a .= ' tabindex="'.$this->attr('tabindex').'"';
			$extras_b .= ' tabindex="'.$this->attr('tabindex').'"';
		}

		$value = $this->attr('default');

		$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_formid);
		ob_start();
		if($form->useAccessibleFormat()) {
		    echo '<div class="sq-form-question-password">';
		    label($field_name, str_replace(':', '_', $name_a), 'class="sq-form-question-title"');
		    echo '<div  class="sq-form-question-password-answer">';
		    password_box($name_a, $value, $width, $max, $extras_a);
		    echo '</div>';
		    echo '</div>';
		    if ($this->attr('verify')) {
			echo '<div class="sq-form-question-password-verify">';
			label($confirm_field_name, str_replace(':', '_', $name_b), 'class="sq-form-question-title"');
			echo '<div class="sq-form-question-password-verify-answer">';
			password_box($name_b, $value, $width, $max, $extras_b);
			echo '</div>';
			echo '</div>';
		     }

		}
		else {
		    password_box($name_a, $value, $width, $max, $extras_a);
		    if ($this->attr('verify')) {
			    echo '<br />';
			    password_box($name_b, $value, $width, $max, $extras_b);
		    }
		}
		$html = ob_get_contents();
		ob_end_clean();

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

		$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_formid);
		$use_accessible_string = $form->useAccessibleFormat();
		if($use_accessible_string) {
		    $core_form_password_entered_differently = translate('core_form_password_entered_differently_accessible');
		    $core_form_password_length_min = sprintf(translate('core_form_password_length_>=_accessible'), $this->attr('min'));
		    $core_form_password_length_max = sprintf(translate('core_form_password_length_<=_accessible'), $this->attr('max'));
		}
		else {
		    $core_form_password_entered_differently = sprintf(translate('core_form_password_entered_differently'), $this->attr('name'));
		    $core_form_password_length_min = sprintf(translate('core_form_password_length_>='), $this->attr('name'), $this->attr('min'));
		    $core_form_password_length_max = sprintf(translate('core_form_password_length_<='), $this->attr('name'), $this->attr('max'));
		}

		// Only try to check "passwords entered differently" if we've actually
		// submitted something to the form
		if ($this->attr('verify') && isset($this->_tmp['verify_value']) && ($answer != $this->_tmp['verify_value'])) {

			$this->failed_rules[] = $core_form_password_entered_differently;
		}
		if (strlen($answer) < $this->attr('min')) {
			$this->failed_rules[] = $core_form_password_length_min;
		}
		if (strlen($answer) > $this->attr('max')) {
			$this->failed_rules[] = $core_form_password_length_max;
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
		return TRUE;

	}//end populate()


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
	* @return string
	* @access public
	*/
	function getSummary()
	{
		return '';

	}//end getSummary()


	/**
	* Set the value of this question
	*
	* @param string	$value	the new value
	*
	* @return void
	* @access public
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
	* @return boolean
	* @access public
	*/
	function isEditable()
	{
		return FALSE;

	}//end isEditable()


	/**
	* Use fieldset tag to group question content?
	*
	* @return bool
	* @access public
	*/
	function useFieldset()
	{
		return TRUE;
	}//end useFieldset()


	/**
	* Get the HTML label for this form question
	*
	* @return boolean
	* @access public
	*/
	function getHtmlLabel()
	{
		$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_formid);
		if($form->useAccessibleFormat()) {
		    $required_mark = $form->getRequiredFieldMark();
		    return '<legend class="sq-form-question-title">'.$this->attr('name').$required_mark.'</legend>';
		}
		else {
		    return $this->attr('name');
		}

	}//end getHtmlLabel()


}//end class
?>