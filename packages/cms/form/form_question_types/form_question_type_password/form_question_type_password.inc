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
* $Id: form_question_type_password.inc,v 1.6 2004/04/28 03:07:49 lwright Exp $
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
	* @param array	$data		the section's entry for this question (contains all question data)
	*
	*/
	function Form_Question_Type_Password($assetid=0,$data=Array()) 
	{
		$this->Form_Question($assetid, $data);

	} // end Form_Question_Type_Password()


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
		$width  = $this->attr('width');
		$max    = $this->attr('max');
		$extras = $this->attr('extra');
		$name   = $this->id.'_1';
		$name2   = $this->id.'_2';


		// generate some php for the value, which can be used for both
		// auto-answer and returning to saved forms
		/*if ($content_file) {
			$html  = '<'.'?php $q = &$GLOBALS[\'SQ_SYSTEM\']->am->getAsset('
			.$this->id.', \'form_question_type_password\'); '
			.' $q->active_submission =& $this->active_submission; ?'.'>';
			$value = '<'.'?php echo $q->getValue(); ?'.'>';
		} else {*/
			$html = '';
		//}
		
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
		$form = &$this->getMySection();
		if ($form->active_submission) {
			$answer = $form->active_submission[$this->id];
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
		$form = &$this->getMySection();
		//if (!$form->active_submission) return false;
				
		if (isset($_POST[$this->id.'_1'])) {
			$val = $_POST[$this->id.'_1'];

			if (strlen($val) < $this->attr('min')) {
				$form->submission_errors[] = 'Password field "'.$this->attr('name').'" must be no less than '.$this->attr('min').' characters';
				return false;
			}
			if (strlen($val) > $this->attr('max')) {
				$form->submission_errors[] = 'Password field "'.$this->attr('name').'" must be no more than '.$this->attr('max').' characters';
				return false;
			}
			if (!$this->answeredCorrectly()) {
				$form->submission_errors[] = 'Password field "'.$this->attr('name').'" was entered differently on 	each try';
				return false;
			}

			$form->active_submission[$this->id] = $val;
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