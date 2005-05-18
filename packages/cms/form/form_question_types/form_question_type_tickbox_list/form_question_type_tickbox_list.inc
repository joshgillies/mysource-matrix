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
* $Id: form_question_type_tickbox_list.inc,v 1.35 2005/05/18 04:51:17 tbarrett Exp $
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
* @version $Revision: 1.35 $
* @package MySource_Matrix_Packages
* @subpackage cms
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
	function getAnswer()
	{
		return $this->listBox($this->getOptions(), true, $this->attr('horizontal'), $this->attr('extra'));

	}//end getAnswer()


	/**
	* creates the list box for the frontend
	*
	* @param Array(mixed)	$options	the options in the list
	* @param boolean		$multiple	if TRUE checkboxes, otherwise RADIO buttons will be used
	* @param boolean		$hor		if TRUE the list will be printed horizontally
	* @param sting			$extra		extras in the input tag eg. className
	*
	* @access public
	* @return string
	*/
	function listBox($options, $multiple=false, $hor=false, $extras='')
	{
		$type  = ($multiple) ? 'checkbox' : 'radio';
		if (!stristr($extras, 'class=')) $extras .= ' class="sq-form-field"';
		$name  = $this->id;

		$html  = '';
		$html .= "<table>";
		$html .= ($hor) ? "<tr>" : '';

		$defaults = $this->getValue();
		if (!is_array($defaults)) $defaults = Array($defaults);
		foreach ($options as $key => $value) {
			$html .= ($hor) ? "<td nowrap=\"nowrap\">" : "<tr><td>";
			$html .= '<input type="'.$type.'" ';
			$html .= 'name="'.$this->id.(($multiple) ? '[]' : '').'" ';
			$html .= 'id="'.$this->id.'_'.$key.'"';
			$html .= 'value="'.htmlspecialchars($key).'"';

			if (in_array($key, $defaults)) {
				$html .= ' checked="checked"';
			}

			$html .= $extras.' />';
			$html .= '&nbsp;<label for="'.$this->id.'_'.$key.'">'.$value.'</label>';
			$html .= ($hor) ? "</td>" : "</td></tr>";
		}

		$html .= ($hor) ? "</tr>" : '';
		$html .= '</table>';

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
			if ($options[$k] == "") {
				unset($options[$k]);
			}
		}

		return $options;

	}//end getOptions()


	/**
	* process this question in the form
	*
	* @access public
	* @return boolean
	*/
	function evaluate()
	{
		$form =& $this->getMySection();

		$val = isset($_POST[$this->id]) ? $_POST[$this->id] : '';

		if (is_array($val)) {
			$value = implode(', ', $this->getAnswerByOffset($val));
		} else {
			$value = $this->getAnswerByOffset($val);
		}

		if ($this->attr('required')) {
			$req_rule = Array('rule_code' => 'select_limit', 'operator' => '>=', 'value' => '1');
			array_push($this->vars['rules']['value'], $req_rule);
		}

		$ok = parent::evaluate();

		if ($this->attr('required')) {
			array_pop($this->vars['rules']['value']);
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
		if ($this->attr('required')) {
			$req_rule = Array('rule_code' => 'select_limit', 'operator' => '>=', 'value' => '1');
			array_push($this->vars['rules']['value'], $req_rule);
		}

		$code = parent::generateJSCode();

		if ($this->attr('required')) {
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
	* @param string $answer the answer to get the summary of
	*
	* @access public
	* @return mixed
	*/
	function getSummary($answer)
	{
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
		if (isset($this->active_submission)) {
			$values = $this->active_submission->getAnswer($this->id);
			if (isset($values)) {
				if (is_array($values)) {
					return in_array($value, $values);
				} else {
					return ($value == $values && !empty($values));
				}
			}
		} else {
			$defaults = $this->attr('default');
			if (in_array($value, $defaults)) return true;
		}

		return false;

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
		if (!is_array($answer)) {
			$answer = Array($answer);
		}

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
