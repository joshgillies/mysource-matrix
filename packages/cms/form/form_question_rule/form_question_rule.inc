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
* $Id: form_question_rule.inc,v 1.8 2003/11/26 00:51:27 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Form_Question_Rule
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Rule extends Asset
{


	/**
	* the question id that this rule belongs to 
	* @var integer
	*/
	var $my_question_id = 0;


	/**
	* the question type that this rule belongs to 
	* @var string
	*/
	var $my_question_type = '';


	/**
	* the question object that this rule belongs to
	* @var &object form_question
	*/
	var $my_question = null;


	/**
	* the active submission object
	* @var object
	*/
	var $active_submission = null;


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form_Question_Rule($assetid=0) 
	{
		$this->Asset($assetid);

	} // end Form_Question_Rule()


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

	}// end create()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_TYPE_2]['form_question'] = Array('card' => '1', 'exclusive' => true);
		return $page_links;

	} // end _getAllowedLinks()


	/**
	* @abstract
	*/
	function processForm($questionid, $question_name)
	{
		return true;

	} // end processForm()


	/**
	* returns the question that this rule belongs to
	*
	* @access public
	* @return &object form_question
	*/
	function &getMyQuestion()
	{
		if (is_null($this->my_question)) {
			$this->my_question = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->my_question_id, $this->getMyQuestionType());
			if (is_null($this->my_question)) return null;
		}
		return $this->my_question;
	
	} // end getMyQuestion()


	/**
	* returns the question type that this rule belongs to
	* 
	* @access public
	* @return string
	*/
	function getMyQuestionType()
	{
		if (!$this->my_question_type) {
			$my_question_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($this->my_question_id));
	
			if (!empty($my_question_info)) {
				$this->my_question_type = $my_question_info[$this->my_question_id]['type_code'];
			}
		}
		
		return $this->my_question_type;

	} // end getMyQuestionType()


	/**
	* returns a dropdown list of the options in a select type question
	*
	* @access public
	* @return boolean
	*/
	function getOptionDropdownList()
	{
		$my_question = &$this->getMyQuestion();
		if (is_null($my_question)) return false;

		$options = Array('------ None ------');
		$options =  array_merge($options, $my_question->getOptions());

		return combo_box($this->getPrefix().'_value', $options, false, $this->attr('value'), 1, '', 30);
	
	} // end getOptionDropdownList()


	/**
	* prints a the asset finder to choose a question for comparison
	*
	* @param integer $id the current chosen id
	* 
	* @access public
	* @return boolean
	*/
	function printAssetFinder($id=0)
	{
		return asset_finder($this->getPrefix().'_comparison_question_id', $id, Array('form_question' => 'D'));
		
	} // end getQuestionDropdownList()


	/**
	* compares the user's answer with the rule value
	*
	* @access public
	* @return boolean
	*/
	function valueComparison()
	{
		if (!$this->active_submission) return false;

		$answer = $this->active_submission->getAnswer($this->my_question_id);
		$value	= $this->attr('value');
		
		if ($this->attr('comparison_question_id')) {
			return $this->questionComparison();
		} else {
			if (is_array($answer)) {
				$valid = false;
				foreach ($answer as $a) {
					if ($this->compareQuestionValues($a, $value)) {
						$valid = true;
						break;
					}
				}
			} else {
				($this->compareQuestionValues($answer, $value)) ? $valid = true : $valid = false; 
			}
			if ($valid) {
				return true;
			} else {
				$q = &$this->getMyQuestion();
				// if this rule's question is a selection, we need to ask it to covert from an offset, to a value
				if ($q->isSelection()) {
					bam($value);
					$value = $q->getAnswerByOffset($value);
				}
				$error = $this->generateComparisonError($value);
				$this->active_submission->setError($error);
				return false;
			}
		}
	} // end valueComparison()

	
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

		// check to see if this user is a dickhead, and has chosen to compare with
		// itself
		if ($comp_qid == $this->id) {
			$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
			$message_body = 'Question '.$this->name .'(#'.$this->id.') cannot be compared with itself';
			$message = $ms->newMessage(Array(), 'Form Error', $message_body, 'asset.error');
			$message->parameters['assetid'] = $this->id;
			$message->send();

			return false;
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
		$converted = false;
		if (is_array($answer)) {
			foreach ($answer as $a) {
				$a = $my_question->getAnswerByOffset($a);
				
				if (is_array($comp_answer)) {
					foreach ($comp_answer as $c) {
						$c = $comp_question->getAnswerByOffset($c);
						if ($this->compareQuestionValues($a, $c)) {
							$valid = true;
							break;
						}
					}
					if ($valid) break;
				} else {
					if ($comp_question->isSelection()) {
						$comp_answer = $comp_question->getAnswerByOffset($comp_answer);
						$converted = true;
					}
					if ($this->compareQuestionValues($a, $comp_answer)) {
						$valid = true;
						break;
					}
				}
			}
		} else {
			if (is_array($comp_answer)) {
				foreach ($comp_answer as $ca) {
					// the compare question is a selection, so convert the offset to a string value
					$ca = $comp_question->getAnswerByOffset($ca);
					// check to see if my question is a selection
					if ($my_question->isSelection()) {
						$answer = $my_question->getAnswerByOffset($answer);
					}

					if ($this->compareQuestionValues($answer, $ca)) {
						$valid = true;
						break;
					}
				}
			} else {
				// we need to convert this question from a offset to an answer
				if ($my_question->isSelection()) $answer = $my_question->getAnswerByOffset($answer);
				if ($comp_question->isSelection()) {
					$comp_answer = $comp_question->getAnswerByOffset($comp_answer);
					$converted = true;
				}
				if ($this->compareQuestionValues($answer, $comp_answer)) $valid = true;
			}
		}

		if ($valid) {
			return true;
		} else {
			if (is_array($comp_answer)) {
				$comp_answer = $comp_question->getAnswerByOffset($comp_answer);
				$comp_answer = implode(', ', $comp_answer);
			} else {
				if ($comp_question->isSelection() && !$converted) {
					$comp_answer = $comp_question->getAnswerByOffset($comp_answer);
				}
			}
			$error = $this->generateQuestionComparisonError($comp_question, $comp_answer);
			$this->active_submission->setError($error);
			
			return false;
		}
	} // end compareQuestionValues()

} // end class

?>
