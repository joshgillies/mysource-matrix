<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: form_question_type_tickbox_list.inc,v 1.4 2003/10/13 05:22:45 mmcintyre Exp $
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
	* Create this asset
	*
	* @param array()	&$link	information used to create the initial link
	*
	* @see Asset::create()
	* @return mixed int or false
	* @access public
	*/
	function create(&$link)
	{
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$linkid = parent::create($link);

		if (!$linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return $linkid;

	}//end create()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$links[SQ_LINK_TYPE_3]['question_rule_type_compare'] = Array('card' => '1', 'exclusive' => true);
	//	$links[SQ_LINK_TYPE_3]['question_rule_type_email'] = Array('card' => '1', 'exclusive' => true);
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
		$html .= '<'.'?php $q->active_submission =& $this->active_submission; ?'.'>';
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
	* returns the current value of this question
	*
	* @access public
	* @return mixed
	*/
	function getValue()
	{
		if ($this->active_submission) {
			$answer = $this->active_submission->getAnswer($this->id);
			if ($answer) return $this->getAnswerByOffset($answer);
		} 
		return $this->attr('default');
	
	} // end getValue()


	/**
	* returns the answer by its offset in the list
	*
	* @param Array|int $offset the offset(s) to the answers
	*
	* @access public
	* @return mixed
	*/
	function getAnswerByOffset($offset = 0)
	{
		$options = $this->getOptions();
		if (is_array($offset)) {
			$answers = Array();
			foreach ($offset as $o) {
				if (isset($options[$o])) $answers[] = $options[$o]; 
			}
			return $answers;
		}
		if (isset($options[$offset])) return $options[$offset];
		return false;

	} // end getAnswerByOffset()


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
			$defaults = $this->attr('default');
			if (in_array($value, $defaults)) return true;
		}
		return false;
	
	} // end isSelected()


}//end class
?>
