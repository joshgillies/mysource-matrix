<?php

require_once dirname(__FILE__).'/../../form_question/form_question.inc';

/**
* Form_Question_Type_Select
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
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
	}


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
	* Creates a link between this asset and the passed asset, of a certain type
	* and returns the linkid of the created link
	*
	* @return int
	* @access public
	* @see Asset::creatLink();
	*/
	function createLink(&$minor, $link_type, $value='', $sort_order=-1, $dependant='0', $exclusive='0')
	{
		return parent::createLink($minor, $link_type, $value, $sort_order, $dependant, $exclusive);

	}// end createLink()


	/**
	* Returns TRUE if we can delete the passed link, or a string with the error msg
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return mixed TRUE or error msg string
	* @access public
	*/
	function canDeleteLink($linkid)
	{
		return parent::canDeleteLink($linkid);
	
	}//end canDeleteLink()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$links[SQ_LINK_TYPE_3]['form_question_rule_type_compare'] = Array('card' => '1', 'exclusive' => true);
	//	$links[SQ_LINK_TYPE_3]['question_rule_type_email'] = Array('card' => '1', 'exclusive' => true);
		return $links;

	}//end _getAllowedLinks()


	/**
	* Prints out the backend interface for the asset
	*
	* @param object Backend_Outputter $o
	*
	* @access public
	*/
	function paintBackend(&$o)
	{
		parent::paintBackend($o);
	
	}// end paintBackend()


	/**
	* Called by the design to print the body of this asset
	*
	* @access public
	*/
	function printBody()
	{
	
	} // end printBody()


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
	*
	*
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
	}

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
	}

	function getOptions()
	{
		return split("\r\n", $this->attr('options'));
	}


	/**
	*
	*
	*
	*
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
	}


	/**
	*
	*
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
	}


}//end class
?>
