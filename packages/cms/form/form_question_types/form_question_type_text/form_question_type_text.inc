<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: form_question_type_text.inc,v 1.9 2003/10/15 03:21:52 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../form_question/form_question.inc';

/**
* Form_Question_Type_Text
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Type_Text extends Form_Question
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form_Question_Type_Text($assetid=0) 
	{
		$this->Asset($assetid);

	} // end Form_Question_Type_Text()


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
		$name   = $this->id;


		// generate some php for the value, which can be used for both
		// auto-answer and returning to saved forms
		if ($content_file) {
			$html  = '<'.'?php $q = &$GLOBALS[\'SQ_SYSTEM\']->am->getAsset('
			.$this->id.', \''.get_class($this).'\'); ';
			$html .=' $q->active_submission =& $this->active_submission; $q->active_form =& $active_form; ?'.'>';

			$value = '<'.'?php echo $q->getValue(); ?'.'>';
		} else {
			$html = '';
		}
		
		ob_start();
		if ($height == 1) {
			text_box($name, $value, $width, $max, $extras);
		} else {
			text_area($name, $value, $width, $height, $max, $extras);
		}
		$html .= ob_get_contents();
		ob_end_clean();
		
		// calling text_box will convert the open and close php tags into their html counter-parts
		// so we need to convert them back to proper php tags

		return html_entity_decode($html);

	} // end getAnswer()


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
		
		if (isset($_POST[$this->id])) {
			$val = $_POST[$this->id];
			$this->active_submission->setAnswer($this->id, $val);
			$this->active_submission->setSummary($this->id, $this->attr('name'), $val, $this->section_id, $this->section_name);

			parent::processForm();
		}
		
		return true;
	
	} // end processForm()

}//end class
?>
