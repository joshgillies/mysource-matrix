<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: form_question_rule_type_comparison.inc,v 1.8 2003/10/14 22:54:05 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule.inc';

/**
* Form_Question_Rule_Type_Comparison
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Rule_Type_Comparison extends Form_Question_Rule
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form_Question_Rule_Type_Comparison($assetid=0) 
	{
		$this->Form_Question_Rule($assetid);
	
	}// end Form_Question_Rule_Type_Email()


	/**
	* returns the operators 
	*
	* @access public
	* @return Array()
	*/
	function getOperators()
	{
		$operators = Array(
			'==' => 'Must Be Equal to ',
			'!=' => 'Must Not Be Equal to ',
			'>'  => 'Must be Greater Than ',
			'<'  => 'Must be Less Than ',
			'>=' => 'Must be Greater Than or Equal To ',
			'<=' => 'Must be Less Than or Equal To ',
		);

		return $operators;

	} // getOperators


	/**
	* returns true if the question is valid, otherwise 
	* adds an error to the submission object, and return false
	* 
	* @see Form_Question_Rule::processForm()
	*
	* @param &object form_submission	$submission		the submission object for this form
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
		$operator = $this->attr('operator');
		if (!$operator) $operator = '==';

		return eval('return \''.$answer.'\''.$operator.'\''.$comparison.'\';');
	
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
		$operators = $this->getOperators();

		return $this->my_question->name.' '.$operators[$operator].' '.$answer;
		
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
		$operators = $this->getOperators();
		$operator = $this->attr('operator');

		return $this->my_question->name.' '.$operators[$operator].
			' '.$comp_question->_getName().' in '.$comp_question->getMySectionName().' (\''.$comp_answer.'\')';

	} // end generateComparisonError()

}// end class
?>
