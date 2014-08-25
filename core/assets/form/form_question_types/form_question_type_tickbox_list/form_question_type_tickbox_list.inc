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
* $Id: form_question_type_tickbox_list.inc,v 1.17 2013/04/19 06:36:02 ewang Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question/form_question.inc';


/**
* Form_Question_Type_Tickbox_List
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Revision: 1.17 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Type_Tickbox_List extends Form_Question
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	* @param array	$data		the section's entry for this question (contains all question data)
	*
	*/
	function Form_Question_Type_Tickbox_List($assetid=0,$data=Array())
	{
		$this->Form_Question($assetid,$data);

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
		$extras = $this->attr('extra');

		if ($this->attr('tabindex')) {
			$extras .= ' tabindex="'.$this->attr('tabindex').'"';
		}
		return $this->listBox($this->getOptions(), TRUE, $extras);

	}//end getHtmlField()


	/**
	* creates the list box for the frontend
	*
	* @param array		$options	the options in the list
	* @param boolean	$multiple	if TRUE checkboxes, otherwise RADIO buttons will be used
	* @param string		$extras		extras in the input tag eg. className
	*
	* @return string
	* @access public
	*/
	function listBox($options, $multiple=FALSE, $extras='')
	{
		$type  = ($multiple) ? 'checkbox' : 'radio';
		if (!stristr($extras, 'class=')) {
			$extras .= ' class="sq-form-field"';
		}
		$name  = 'q'.$this->id;
		$safe_name = str_replace(':', '_', $name);


		$defaults = $this->getValue();
		if (!is_array($defaults)) $defaults = Array($defaults);
		$html = '<ul>';

		foreach ($options as $key => $value) {
			$html .= '<li>';

			$html .= '<input type="'.$type.'" ';
			$html .= 'name="'.$name.(($multiple) ? '[]' : '').'" ';
			$html .= 'id="'.$safe_name.'_'.$key.'" ';
			$html .= 'value="'.htmlspecialchars($key).'"';
			if (in_array($key, $defaults)) {
				$html .= ' checked="checked"';
			}
			$html .= ($extras == '') ? '/>' : ' '.$extras.' />';

			$html .= ' <label for="'.$safe_name.'_'.$key.'">'.$value.'</label>';

			$html .= '</li>';
		}

		$html .= '</ul>';

		return $html;

	}//end listBox()


	/**
	* returns an array of the options set in the backend
	*
	* @return array
	* @access public
	*/
	function getOptions()
	{
		$options = @unserialize($this->attr('options'));
		if (!is_array($options) && empty($options)) {
			$options = Array();
		}//end if
		// remove empty options
		foreach (array_keys($options) as $k) {
			if ($options[$k] == '') unset($options[$k]);
		}

		return $options;

	}//end getOptions()


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

		if (!is_array($answer)) {
			$answer = Array($answer);
		}
		$value = $this->getAnswerByOffset($answer);

		if ($this->attr('is_required') && $this->_isValidationRequired()) {
			$req_rule = Array('rule_code' => 'select_limit', 'operator' => '>=', 'value' => '1');
			array_push($this->vars['rules']['value'], $req_rule);
		}

		$ok = parent::hasValidValue($value, $mute_errors);

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
			$req_rule = Array('rule_code' => 'select_limit', 'operator' => '>=', 'value' => '1');
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
		return Array('select_limit', 'selection');

	}//end getAllowedRules()


	/**
	* returns a summary for this question
	*
	* @param string	$answer	The answer the print the summary for (optional)
	*
	* @return mixed array|string
	* @access public
	*/
	function getSummary($answer=NULL)
	{
		if (is_null($answer)) $answer = $this->getValue();

		if (is_array($answer)) {
			$ans_str = Array();
			foreach ($answer as $a) {
				$ans_str[] = $this->getAnswerByOffset($a);
			}
			return implode(', ', $ans_str);
		}
		return $this->getAnswerByOffset($answer);

	}//end getSummary()


	/**
	* returns TRUE if this particuar tickbox is selected
	*
	* @param int	$value	the value to check
	*
	* @return boolean
	* @access public
	*/
	function isSelected($value)
	{
		$values = $this->getValue();
		if (!is_null($values)) {
			if (is_array($values)) {
				return in_array($value, $values);
			} else {
				return ($value == $values && !empty($values));
			}
		} else {
			$defaults = $this->attr('default');
			return (in_array($value, $defaults));
		}

	}//end isSelected()


	/**
	* Get option name
	*
	* Get the name for one particular option, specified by option code.
	* Assumes option id is valid
	*
	* @param string	$option_code	unique option id for this question
	*
	* @return string
	* @access public
	*/
	function getOptionName($option_code)
	{
		return $this->getAnswerByOffset($option_code);

	}//end getOptionName()


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

		echo '<select_q id="'.addslashes($this->id).'"  name="'.htmlspecialchars($this->attr('name')).'">';
		if (!is_array($answer)) $answer = Array($answer);

		foreach ($answer as $a) {
			echo '<option value="'.$a.'">'.htmlspecialchars($this->getAnswerByOffset($a)).'</option>';
		}
		echo '</select_q>';

		$contents = ob_get_contents();
		ob_end_clean();

		return $contents;

	}//end getXML()


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
		    $required_mark = '';
		    if($this->attr('is_required')) {
			$required_mark = $form->getRequiredFieldMark();
		    }
		    return '<legend class="sq-form-question-title">'.$this->attr('name').$required_mark.'</legend>';
		}
		else {
		    return $this->attr('name');
		}

	}//end getHtmlLabel()




	/**
	* Populates
	*
	* @return boolean
	* @access public
	*/
	function populate()
	{
		$prefix = 'q'.$this->id;
		$this->setValue(array_get_index($_POST, $prefix, Array()));
		return TRUE;

	}//end populate()


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


}//end class
?>
