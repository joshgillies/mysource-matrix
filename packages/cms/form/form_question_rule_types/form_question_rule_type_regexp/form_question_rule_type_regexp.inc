<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: form_question_rule_type_regexp.inc,v 1.4 2003/09/28 23:00:06 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule.inc';

/**
* Form_Question_Rule_Type_Regexp
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Rule_Type_Regexp extends Form_Question_Rule
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form_Question_Rule_Type_Regexp($assetid=0) 
	{
		$this->Form_Question_Rule($assetid);
	
	}// end Form_Question_Rule_Type_Regexp()


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
		$value = $this->attr('value');

		$value = str_replace('|', '\|', $value);
		$pattern = "|".$value."|";
		(preg_match($pattern, $answer)) ? $valid = 1 : $valid = 0; 

		if ($valid == $operator) {
			return true;
		} else {
			$error = $question_name.' is not in the correct format';
			$this->active_submission->setError($error);
			return false;
		}

	}// end processForm()

}// end class
?>
