<?php
/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
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
* $Id: form_question_rule_type_email.inc,v 1.7 2003/11/18 15:43:36 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule.inc';
require_once SQ_FUDGE_PATH .'/general/www.inc';

/**
* Form_Question_Rule_Type_Email
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Rule_Type_Email extends Form_Question_Rule
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form_Question_Rule_Type_Email($assetid=0) 
	{
		$this->Form_Question_Rule($assetid);
	
	}// end Form_Question_Rule_Type_Email()


	/**
	* returns true if the question is valid, otherwise 
	* adds an error to the submission object, and return false
	* 
	* @see Form_Question_Rule::processForm()
	*
	* @param	int						$questionid		the id of the question this rule is linked to 
	* @param string						$question_name	the name of the question (used for producing errors)
	*
	* @access public
	* @return boolean
	*/
	function processForm($questionid, $question_name)
	{
		$answer   = $this->active_submission->getAnswer($questionid);
		$operator = ($this->attr('operator')) ? 1 : 0;

		$valid = valid_email($answer);

		if ($valid == $operator) {
			return true;
		} else {
			$error = $question_name.' must '. (($operator) ? '' : 'not') .' be a valid Email Address';
			$this->active_submission->setError($error);
			return false;
		}

	}// end processForm()

}// end class
?>
