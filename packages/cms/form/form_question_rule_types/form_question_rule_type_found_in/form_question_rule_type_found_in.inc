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
* $Id: form_question_rule_type_found_in.inc,v 1.7 2003/11/26 00:51:28 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule.inc';

/**
* Form_Question_Rule_Type_Found_In
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Rule_Type_Found_In extends Form_Question_Rule
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form_Question_Rule_Type_Found_In($assetid=0) 
	{
		$this->Form_Question_Rule($assetid);
	
	}// end Form_Question_Rule_Type_Found_In()


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
		if (!$this->active_submission) return false;
		
		return $this->valueComparison();

	}// end processForm()


	/**
	* returns TRUE if the two values evaluate with the current operator
	*
	* @param string		$answer			the left argument
	* @param string		$comparison		the right argument
	*
	* @access public
	* @return boolean
	*/
	function compareQuestionValues($answer, $comparison)
	{
		return (strpos($comparison, $answer) !== false) ? true : false; 
	
	} // end questionComparison()


	/**
	* returns an error message for comparing the answer with a value
	*
	* * @param string $answer the user's answer
	*
	* @access public
	* @return string
	*/
	function generateComparisonError($answer)
	{
		$my_question = &$this->getMyQuestion();
		$operator = $this->attr('operator');

		return  $this->my_question->name.' must '. (($operator) ? '' : 'not') .' be found in \''.$answer.'\'';
		
	} // end generateComparisonError()


	/**
	* generates an error for question comparison
	*
	* @param &object Question	$question		the comparison question
	* @param string				$comp_answer	the comparison answer
	*
	* @access public
	* @return string
	*/
	function generateQuestionComparisonError(&$comp_question, $comp_answer)
	{
		$operator = $this->attr('operator');

		return $this->my_question->name.' must '. (($operator) ? '' : 'not') .' be found in the value \''.$comp_answer.'\' ('.$comp_question->name.' in '.$comp_question->getMySectionName().')';

	} // end generateComparisonError()

}// end class
?>
