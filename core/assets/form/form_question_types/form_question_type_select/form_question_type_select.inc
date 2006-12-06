<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: form_question_type_select.inc,v 1.9 2006/12/06 00:35:31 emcdonald Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question/form_question.inc';


/**
* Form_Question_Type_Select
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Type_Select extends Form_Question
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	* @param array	$data		the section's entry for this question (contains all question data)
	*
	*/
	function Form_Question_Type_Select($assetid=0,$data=Array())
	{
		$this->Form_Question($assetid,$data);

	}//end constructor


	/**
	* returns an array of the types of rules that are allowed upon this asset
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
	* generates the html for this question to display it in the frontend or backend
	*
	* @return string
	* @access public
	*/
	function getHtmlField()
	{
		$name = 'q'.$this->id;

		// prepare a valid value for the field id
		$extras = 'id="'.str_replace(':', '_', $name).'" '.$this->attr('extra');

		if ($this->attr('tabindex')) {
			$extras .= ' tabindex="'.$this->attr('tabindex').'"';
		}

		ob_start();
		combo_box($name, $this->getOptions(), $this->attr('multiple'), $this->getValue(), $this->attr('height'), $extras);
		$html = ob_get_contents();
		ob_end_clean();

		return $html;

	}//end getHtmlField()


	/**
	* returns the options currently in the dropdown list
	*
	* @return array
	* @access public
	*/
	function getOptions()
	{
		$options = split("\r\n", $this->attr('options'));
		// remove empty options
		foreach (array_keys($options) as $k) {
			$options[$k] = htmlspecialchars($options[$k]);
			if ($options[$k] == '') unset($options[$k]);
		}
		// if allow empty is true, add the '-- Leave Empty --' as the first option
		if ($this->attr('allow_empty')) {
			$empty = Array(htmlspecialchars($this->attr('empty_text')));
			$options = array_merge($empty, $options);
		}

		return $options;

	}//end getOptions()


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
				// if the option is '-- Leave Empty --', we should return empty string
				$answer_by_offset = $this->getAnswerByOffset($a);
				if ($answer_by_offset != $this->attr('empty_text')) {
					$ans_str[] = $this->getAnswerByOffset($a);
				} else {
					$ans_str[] = '';
				}
			}
			return implode(', ', $ans_str);
		}

		// if the option is '-- Leave Empty --', we should return empty string
		$answer_by_offset = $this->getAnswerByOffset($answer);
		if ($answer_by_offset != $this->attr('empty_text')) {
			return $this->getAnswerByOffset($answer);
		} else {
			return '';
		}

	}//end getSummary()


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
	* returns TRUE if this particuar option is selected
	*
	* @param string	$option	the value to check
	*
	* @return boolean
	* @access public
	*/
	function isSelected($option)
	{
		return in_array($option, $this->getValue());

	}//end isSelected()


	/**
	* Get option name
	*
	* Get the name for one particular option, specified by option code.
	* Assumes option id is valid
	*
	* @param string	$option_code	unique option id for this question
	*
	* @return array
	* @access public
	*/
	function getOptionName($option_code)
	{
		return $this->getAnswerByOffset($option_code);

	}//end getOptionName()


}//end class
?>
