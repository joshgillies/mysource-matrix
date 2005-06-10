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
* $Id: form_question_type_select.inc,v 1.26.2.4 2005/06/10 04:04:19 lwright Exp $
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
* @version $Revision: 1.26.2.4 $
* @package MySource_Matrix_Packages
* @subpackage cms
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
	* @access public
	* @return Array(string => string)
	*/
	function getAllowedRules()
	{
		return Array('select_limit', 'selection');

	}//end getAllowedRules()


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
	* generates the html for this question to display it in the frontend or backend
	*
	* @param boolean		$content_file	if this is for the content file or not
	* @param integer|Array	$value			the selected values of this question
	*
	* @access public
	* @return string
	*/
	function getAnswer($content_file=true, $value='', $suffix='')
	{
		if (!$suffix) $suffix = '';
		// generate some php for the value, which can be used for both
		// auto-answer and returning to saved forms

		ob_start();
		combo_box($this->id.$suffix, $this->getOptions(), $this->attr('multiple'), $this->getValue(), $this->attr('height'), '');
		$html = ob_get_contents();
		ob_end_clean();

		return html_entity_decode($html);

	}//end getAnswer()


	/**
	* returns the options currently in the dropdown list
	*
	* @access public
	* @return Array()
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


	/**
	* returns TRUE if this particuar option is selected
	*
	* @param string $value     the value to check
	*
	* @access public
	* @return boolean
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
	* @param string $option_code    unique option id for this question
	*
	* @access public
	* @return Array
	*/
	function getOptionName($option_code)
	{
		return $this->getAnswerByOffset($option_code);

	}//end getOptionName()


}//end class
?>
