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
* $Id: form_question_type_password.inc,v 1.3 2003/11/26 00:51:28 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../form_question/form_question.inc';

/**
* Form_Question_Type_Password
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Type_Password extends Form_Question
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form_Question_Type_Password($assetid=0) 
	{
		$this->Asset($assetid);

	} // end Form_Question_Type_Password()


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
		$links = parent::_getAllowedLinks();
		$links[SQ_LINK_TYPE_3]['form_question_rule_type_comparison'] = Array('card' => '1', 'exclusive' => true);
		$links[SQ_LINK_TYPE_3]['form_question_rule_type_email'] = Array('card' => '1', 'exclusive' => true);
		$links[SQ_LINK_TYPE_3]['form_question_rule_type_numeric'] = Array('card' => '1', 'exclusive' => true);
		$links[SQ_LINK_TYPE_3]['form_question_rule_type_found_in'] = Array('card' => '1', 'exclusive' => true);
		$links[SQ_LINK_TYPE_3]['form_question_rule_type_regexp'] = Array('card' => '1', 'exclusive' => true);
		$links[SQ_LINK_TYPE_3]['form_question_rule_type_contain'] = Array('card' => '1', 'exclusive' => true);
		return $links;

	}//end _getAllowedLinks()


	/**
	* generates the html for this question to display it in the frontend
	*
	* @param boolean	$content_file	if TRUE, this function will generate content for the content file
	* @param string		$value			current value of the question
	*
	* @access public
	* @return string
	*/
	function getAnswer($content_file=true, $value='')
	{
		$height = ($this->attr('height')) ? $this->attr('height') : 1;
		$width  = $this->attr('width');
		$max    = $this->attr('max');
		$extras = $this->attr('extra');
		$name   = $this->id.'_1';
		$name2   = $this->id.'_2';


		// generate some php for the value, which can be used for both
		// auto-answer and returning to saved forms
		if ($content_file) {
			$html  = '<'.'?php $q = &$GLOBALS[\'SQ_SYSTEM\']->am->getAsset('
			.$this->id.', \'form_question_type_password\'); '
			.' $q->active_submission =& $this->active_submission; ?'.'>';
			$value = '<'.'?php echo $q->getValue(); ?'.'>';
		} else {
			$html = '';
		}
		
		ob_start();
		password_box($name, $value, $width, $max, $extras);
		if ($this->attr('verify')) {
			echo '<BR />';
			password_box($name2, $value, $width, $max, $extras);
		}
		$html .= ob_get_contents();
		ob_end_clean();
		
		// calling text_box will convert the open and close php tags into their html counter-parts
		// so we need to convert them back to proper php tags

		return html_entity_decode($html);

	} // end getAnswer()

	
	/**
	* echos the current value
	*
	* @param &$object $submission the current submission object
	*
	* @access public
	* @return void
	*/
	function getValue()
	{
		if ($this->active_submission) {
			$answer = $this->active_submission->getAnswer($this->id);
			if ($answer) echo $answer;
		}
	
	} // end getValue()


	/**
	* process this question
	*
	* @param &object $submission the current submission object
	*
	* @access public
	* @return boolean
	*/
	function processForm()
	{
		if (!$this->active_submission) return false;
		
		if (isset($_POST[$this->id.'_1'])) {
			$val = $_POST[$this->id.'_1'];

			$this->active_submission->setAnswer($this->id, $val);
			$this->active_submission->setSummary($this->id, $this->attr('name'), $val, $this->section_id, $this->section_name);

			parent::processForm();
		}
		
		return true;
	
	} // end processForm()

	/**
	* returns TRUE if this question has been answered correctly
	*
	* @access public
	* @return boolean
	*/
	function answeredCorrectly()
	{
		if (!isset($_POST[$this->id.'_1'])) return false;
		$val1 = $_POST[$this->id.'_1'];
		if (strlen($val1) > $this->attr('max') || strlen($val1) < $this->attr('min')) return false;

		if ($this->attr('verify')) {
			if (!isset($_POST[$this->id.'_2'])) return false;
			$val2 = $_POST[$this->id.'_2'];
			if ($val1 != $val2) return false;
		}
		return true;
	}

}//end class
?>