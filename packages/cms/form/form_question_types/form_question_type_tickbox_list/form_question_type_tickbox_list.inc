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
* $Id: form_question_type_tickbox_list.inc,v 1.7.2.1 2004/03/26 16:41:54 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../form_question/form_question.inc';

/**
* Form_Question_Type_Tickbox_List
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Type_Tickbox_List extends Form_Question
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form_Question_Type_Tickbox_List($assetid=0) 
	{
		$this->_ser_attrs = true;
		$this->Asset($assetid);
	
	} // end Form_Question_Type_Tickbox_List()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$links[SQ_LINK_TYPE_3]['form_question_rule_type_comparison'] = Array('card' => '1', 'exclusive' => true);
		$links[SQ_LINK_TYPE_3]['form_question_rule_type_contain'] = Array('card' => '1', 'exclusive' => true);

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
		// generate some php for the value, which can be used for both
		// auto-answer and returning to saved forms
		
		$html  = '<'.'?php $q = &$GLOBALS[\'SQ_SYSTEM\']->am->getAsset('.$this->id.', \''.get_class($this).'\'); ?'.'>';
		$html .= '<'.'?php $q->active_submission =& $this->active_submission; $q->active_form =& $active_form; ?'.'>';
		$html .= $this->listBox($this->getOptions(), $this->attr('multiple'), $this->attr('horizontal'), $this->attr('extra'));
		return $html;

	} // end getAnswer()


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
	function listBox($options, $multiple = false, $hor = false, $extras ='')
	{
		$type  = ($multiple) ? 'checkbox' : 'radio';
		if (!stristr($extras, 'class=')) $extras .= ' class="sq-form-field"';
		$name  = $this->id;

		$html  = '';
		$html .= "<table cellpadding=\"0\" cellspacing=\"0\" width=\"100%\">\n";
		$html .= ($hor) ? "<tr>\n" : '';
		
		foreach ($options as $key => $value) {
			$html .= ($hor) ? "<td nowrap>\n" : "<tr><td>\n";
			$html .= '<input type="'.$type.'" ';
			$html .= 'name="'.$this->id.(($multiple) ? '[]' : '').'" ';
			$html .= 'value="'.htmlspecialchars($key).'"';
			$html .= '<'.'?php echo ($q->isSelected('.$key.')) ? \' checked\' : \'\'; ?'.'>';
			$html .= $extras.' />';
			$html .= '&nbsp;'.$value;
			$html .= ($hor) ? "</td>\n" : "</td></tr>";
		}

		$html .= ($hor) ? "</tr>\n" : '';
		$html .= '</table>';
		
		return $html;
	
	} // end ListBox()


	/**
	* returns an array of the options set in the backend
	*
	* @access public
	* @return Array
	*/
	function getOptions()
	{
		return split("\r\n", $this->attr('options'));
	
	} // end getOptions()


	/**
	* process this question in the form
	*
	* @param &object $submission the submission object
	*
	* @access public
	* @return boolean
	*/
	function processForm()
	{
		if (!$this->active_submission) return false;
		$val = isset($_POST[$this->id]) ? $_POST[$this->id] : '';
		
		if (is_array($val)) {
			$value = implode(', ', $this->getAnswerByOffset($val));
		} else {
			$value = $this->getAnswerByOffset($val);
		}
		$this->active_submission->setAnswer($this->id, $val);
		$this->active_submission->setSummary($this->id, $this->attr('name'), $value, $this->section_id, $this->section_name);
		
		parent::processForm();
		
		return true;

	} // end processForm()


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
	
	} // end getSummary()


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
			if ($values) {
				if (is_array($values)) {
					if (in_array($value, $values)) return true;
				} else {
					if ($value == $values) return true;
				}
			} 
		} else {
			if ($this->attr('auto_answer')) {
				if (!is_null($this->active_form)) {
					$answer =  $this->active_form->getAutoAnswer($this->attr('auto_answer'));
					if ($value == $answer) return true;
				}
			}
			$defaults = $this->attr('default');
			if (in_array($value, $defaults)) return true;
		}
		return false;
	
	} // end isSelected()


}//end class
?>
