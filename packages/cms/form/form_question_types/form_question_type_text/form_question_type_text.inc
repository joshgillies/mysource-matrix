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
* $Id: form_question_type_text.inc,v 1.19 2004/05/12 06:43:42 lwright Exp $
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
	* @param array	$data		the section's entry for this question (contains all question data)
	*
	*/
	function Form_Question_Type_Text($assetid=0, $data=Array()) 
	{
		$this->Form_Question($assetid, $data);

	}//end Form_Question_Type_Text()


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
		$value  = ($value == "") ? $this->attr('default') : $value;
	
		ob_start();
		if ($height == 1) {
			text_box($name, $value, $width, $max, false, $extras);
		} else {
			text_area($name, $value, $width, $height, $max, $extras);
		}
		$html = ob_get_contents();
		ob_end_clean();
		
		// calling text_box will convert the open and close php tags into their html counter-parts
		// so we need to convert them back to proper php tags

		return html_entity_decode($html);

	}//end getAnswer()


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
		$missing = false;
		$form =& $this->getMySection();

		if (isset($_POST[$this->id])) {
			$val = $_POST[$this->id];
			if ($this->attr('required') && ($val == '')) {
				$missing = true;
			} else {
				$form->active_submission[$this->id] = $val;
			}
		} else {
			$missing = true;
		}

		parent::processForm();

		if ($missing) {
			$form->submission_errors[] = 'The question "'.$this->attr('name').'" must be answered';
			return false;
		}
		
		return true;
	
	}//end processForm()

	
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
		return Array('regexp', 'comparison', 'numeric', 'found_in', 'contain', 'length', 'email');

	}//end getAllowedRules()

}//end class
?>
