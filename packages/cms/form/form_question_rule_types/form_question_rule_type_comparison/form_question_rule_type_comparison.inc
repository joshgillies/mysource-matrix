<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: form_question_rule_type_comparison.inc,v 1.7 2003/09/28 23:00:06 mmcintyre Exp $
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

		$answer   = $this->active_submission->getAnswer($questionid);
		$operator = $this->attr('operator');
		$value	  = $this->attr('value'); 
		
		$operators = $this->getOperators();

		// the operator is in '==, !=, <, >, <=, >='
		// so the query will look something like this:
		// 12==12

		// if there is no operator, assign it to ==
		if (!$operator) $operator = '==';

		if ($this->attr('comparison_question_id')) {
			return $this->questionComparison();
		} else {
			if (is_array($answer)) {
				$valid = false;		
				foreach ($answer as $a) {
					$query = 'return \''.$a.'\''.$operator.'\''.$value.'\';';
					if (eval($query)) {
						$valid = true;
						break;
					}
				}
			} else {
				$query = 'return \''.$answer.'\''.$operator.'\''.$value.'\';';
				(eval($query)) ? $valid = true : $valid = false; 
			}
			if ($valid) {
				return true;
			} else {
				$q = &$this->getMyQuestion();
				// if this rule's question is a selection, we need to ask it to covert from an offset, to a value
				if ($q->isSelection()) {
					$value = $q->getAnswerByOffset($value);
				}
				$error = $question_name.' '.$operators[$operator].' \''.$value.'\'';
				$this->active_submission->setError($error);
				return false;
			}
		}

	}// end processForm()


	/**
	* compares this question with the value of another question
	*
	* @access public
	* @return boolean
	*/
	function questionComparison()
	{
		$comp_qid = $this->attr('comparison_question_id');
		if (!$comp_qid) return false;

		$operators = $this->getOperators();

		// check to see if this user is a dickhead, and has chosen to compare with
		// itself
		if ($comp_qid == $this->id) {
		
		}

		// check to see if the comparison question has been processed yet
		if (!$this->active_submission->isProcessed($comp_qid)) {
			$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
			$message_body = 'Question '.$this->name .'(#'.$this->id.') was compared ';
			$message = $ms->newMessage(Array(), 'Form Error', $message_body, 'asset.error');
			$message->parameters['assetid'] = $this->id;
			$message->send();
		
			return false;
		}

		$comp_question = &$GLOBALS['SQ_SYSTEM']->am->getAsset($comp_qid);
		if (!$comp_question) return false;
		
		$comp_answer = $this->active_submission->getAnswer($comp_qid);
		$operator = $this->attr('operator');
		$answer = $this->active_submission->getAnswer($this->my_question_id);

		$my_question = &$this->getMyQuestion();

		$valid = false;
		if (is_array($answer)) {
			foreach ($answer as $a) {
				$a = $my_question->getAnswerByOffset($a);
				
				if (is_array($comp_answer)) {
					foreach ($comp_answer as $c) {
						$c = $comp_question->getAnswerByOffset($c);
						$query = 'return \''.$a.'\''.$operator.'\''.$c.'\';';
						if (eval($query)) {
							$valid = true;
							break;
						}
					}
					if ($valid) break;
				} else {
					if ($comp_question->isSelection()) {
						$comp_answer = $comp_question->getAnswerByOffset($comp_answer);
					}
					$query = 'return \''.$a.'\''.$operator.'\''.$comp_answer.'\';';
					if (eval($query)) {
						$valid = true;
						break;
					}
				}
			}
		} else {
			if (is_array($comp_answer)) {
				foreach ($comp_answer as $a) {
					// the compare question is a selection, so convert the offset to a string value
					$a = $comp_question->getAnswerByOffset($a);
					// check to see if my question is a selection
					if ($my_question->isSelection()) {
						$answer = $my_question->getAnswerByOffset($answer);
					}

					$query = 'return \''.$answer.'\''.$operator.'\''.$a.'\';';
					if (eval($query)) {
						$valid = true;
						break;
					}
				}
			} else {
				// we need to convert this question from a offset to an answer
				if ($my_question->isSelection()) $answer = $my_question->getAnswerByOffset($answer);
				if ($comp_question->isSelection()) $comp_answer = $comp_question->getAnswerByOffset($comp_answer);
				$query = 'return \''.$answer.'\''.$operator.'\''.$comp_answer.'\';';

				if (eval($query)) $valid = true;
			}
		}

		if ($valid) {
			return true;
		} else {
			if (is_array($comp_answer)) {
				$comp_answer = $comp_question->getAnswerByOffset($comp_answer);
				$comp_answer = implode(', ', $comp_answer);
			} else {
				if ($comp_question->isSelection()) {
					$comp_answer = $comp_question->getAnswerByOffset($comp_answer);
				}
			}
			$error = $my_question->name.' '.$operators[$operator].
			' '.$comp_question->_getName().' in '.$comp_question->getMySectionName().' (\''.$comp_answer.'\')';
			$this->active_submission->setError($error);
			
			return false;
		}
	} // end questionComparison()

}// end class
?>
