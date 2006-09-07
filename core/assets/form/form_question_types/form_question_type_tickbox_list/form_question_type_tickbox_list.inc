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
* $Id: form_question_type_tickbox_list.inc,v 1.6.2.1 2006/09/07 04:26:43 bcaldwell Exp $
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
* @version $Revision: 1.6.2.1 $
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
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		return $links;

	}//end _getAllowedLinks()


	/**
	* generates the html for this question to display it in the frontend
	*
	* @access public
	* @return string
	*/
	function getHtmlField()
	{
		return $this->listBox($this->getOptions(), true, $this->attr('extra'));

	}//end getHtmlField()


	/**
	* creates the list box for the frontend
	*
	* @param Array(mixed)	$options	the options in the list
	* @param boolean		$multiple	if TRUE checkboxes, otherwise RADIO buttons will be used
	* @param sting			$extra		extras in the input tag eg. className
	*
	* @access public
	* @return string
	*/
	function listBox($options, $multiple=false, $extras='')
	{
		$type  = ($multiple) ? 'checkbox' : 'radio';
		if (!stristr($extras, 'class=')) {
			$extras .= ' class="sq-form-field"';
		}
		$name  = 'q'.$this->id;
		$safe_name = str_replace(':', '_', $name);

		$html = '<ul>';
		$defaults = $this->getValue();
		if (!is_array($defaults)) $defaults = Array($defaults);
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

			$html .= '&nbsp;<label for="'.$safe_name.'_'.$key.'">'.$value.'</label>';

			$html .= '</li>';
		}

		$html .= '</ul>';

		return $html;

	}//end listBox()


	/**
	* returns an array of the options set in the backend
	*
	* @access public
	* @return Array
	*/
	function getOptions()
	{
		$options = split("\r\n", $this->attr('options'));
		// remove empty options
		foreach (array_keys($options) as $k) {
			if ($options[$k] == '') unset($options[$k]);
		}

		return $options;

	}//end getOptions()


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

		if (is_array($answer)) {
			$value = implode(', ', $this->getAnswerByOffset($answer));
		} else {
			$value = $this->getAnswerByOffset($answer);
		}

		if ($this->attr('is_required')) {
			$req_rule = Array('rule_code' => 'select_limit', 'operator' => '>=', 'value' => '1');
			array_push($this->vars['rules']['value'], $req_rule);
		}

		$ok = parent::hasValidValue($value);

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
			$req_rule = Array('rule_code' => 'select_limit', 'operator' => '>=', 'value' => '1');
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
		return Array('select_limit', 'selection');

	}//end getAllowedRules()


	/**
	* returns a summary for this question
	*
	* @param string	$answer	The answer the print the summary for (optional)
	*
	* @access public
	* @return mixed
	*/
	function getSummary($answer=null)
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
	* @param int $value the value to check
	*
	* @access public
	* @return boolean
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
	* @param string $option_code    unique option id for this question
	*
	* @access public
	* @return string
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
	* @access public
	* @return string the summary XML
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


}//end class
?>
