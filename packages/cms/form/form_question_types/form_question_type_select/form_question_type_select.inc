<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: form_question_type_select.inc,v 1.4 2003/09/26 05:26:41 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../form_question/form_question.inc';

/**
* Form_Question_Type_Select
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Type_Select extends Form_Question
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form_Question_Type_Select($assetid=0) 
	{
		$this->_ser_attrs = true;
		$this->Asset($assetid);

	} // end Form_Question_Type_Select()


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

	} // end create()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$links[SQ_LINK_TYPE_3]['form_question_rule_type_comparison'] = Array('card' => '1', 'exclusive' => true);
		
		return $links;

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
		if ($content_file) {
			$html  = '<'.'?php $q = &$am->getAsset('.$this->id.$suffix.', \'form_question_type_select\'); ?'.'>';
			$html .= '<'.'?php combo_box($q->id, $q->getOptions(), $q->attr(\'multiple\'), $q->getValue($submission), 	$q->attr(\'height\'), $q->attr(\'extra\')) ?'.'>';
		} else {
			ob_start();
			combo_box($this->id.$suffix, $this->getOptions(), $this->attr('multiple'), $value, $this->attr('height'), '');
			$html = ob_get_contents();
			ob_end_clean();
		}
		
		return html_entity_decode($html);

	} // end getAnswer()

	
	/**
	* returns the current value of this question
	*
	* @param &object $submission the current submission object
	*
	* @access public
	* @return mixed
	*/
	function getValue(&$submission)
	{
		if ($submission) {
			$answer = $submission->getAnswer($this->id);
			if ($answer) {
				if (is_array($answer)) {
					foreach ($answer as $a) {
					}
				} else {
					return $this->getAnswerByOffset($answer);
				}
			}

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
	* returns the options currently in the dropdown list
	* 
	* @access public
	* @return Array()
	*/
	function getOptions()
	{
		return split("\r\n", $this->attr('options'));

	} // end getoptions()


	/**
	* process this question in the form
	*
	* @param &object $submission the submission object
	*
	* @access public
	* @return boolean
	*/
	function processForm(&$submission)
	{
		if (!$submission) return false;
		$val = isset($_POST[$this->id]) ? $_POST[$this->id] : '';
		
		if (is_array($val)) {
			$value = implode(', ', $this->getAnswerByOffset($val));
		} else {
			$value = $this->getAnswerByOffset($val);
		}
		$submission->setAnswer($this->id, $val);
		$submission->setSummary($this->id, $this->attr('name'), $value, $this->section_id, $this->section_name);
		
		parent::processForm($submission);
		
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
		} else {
			return $this->getAnswerByOffset($answer);
		}
	} // end getSummary()


}//end class
?>
