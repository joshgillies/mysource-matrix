<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: form_question_rule_type_found_in.inc,v 1.5 2003/10/14 22:54:06 mmcintyre Exp $
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
