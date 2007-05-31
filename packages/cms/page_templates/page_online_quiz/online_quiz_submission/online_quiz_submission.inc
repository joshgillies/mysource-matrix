<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: online_quiz_submission.inc,v 1.1 2007/05/31 06:47:48 mbrydon Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Online Quiz Submission
*
* @author  Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Online_Quiz_Submission extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Online_Quiz_Submission($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		return $this->Asset($assetid);

	}//end constructor


	/**
	* Save attributes set with setAttrValue to the database
	*
	* @param boolean	$dont_run_updated	if true this->_updated() won't be run
	*										(THIS IS ONLY EVER CALLED FROM Asset::create())
	*
	* @return boolean
	* @access public
	* @see asset::setAttrValue()
	*/
	function saveAttributes($dont_run_updated=FALSE)
	{
		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
		$val = parent::saveAttributes($dont_run_updated);
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

		return $val;

	}//end saveAttributes()


	/**
	* Returns the IP address of this submission
	*
	* @return mixed boolean|string
	* @access public
	*/
	function getIP()
	{
		$a = $this->attr('attributes');
		if (isset($a['misc']['ip'])) return $a['misc']['ip'];
		return FALSE;

	}//end getIP()


	/**
	* Sets the IP address of this submission
	*
	* @param string	$ip	the ip of the user who submitted
	*
	* @return boolean
	* @access public
	*/
	function setIP($ip)
	{
		$a = $this->attr('attributes');
		if (empty($a['mics'])) $a['misc'] = Array();

		$a['misc']['ip'] = $ip;

		if (!$this->setAttrValue('attributes', $a)) {
			return FALSE;
		}

		return TRUE;

	}//end setIP()


	/**
	* Obtain the answer logged for User Details Form question
	*
	* @param string	$asset_id	The asset ID of a User Details Form question
	*
	* @return string
	* @access public
	*/
	function getUserDetailsAnswer($asset_id)
	{
		$user_details_answers = $this->getUserDetailsAnswers();
		$user_details_answer = NULL;

		if (isset($user_details_answers[$asset_id]['answer'])) {
			$user_details_answer = $user_details_answers[$asset_id]['answer'];
		}

		return $user_details_answer;

	}//end getUserDetailsAnswer()


	/**
	* Log an answer which originated from the User Details Form
	*
	* @param string	$asset_id	The asset ID of a User Details Form question
	* @param string	$answer		The answer to be assigned
	*
	* @return string
	* @access public
	*/
	function setUserDetailsAnswer($asset_id, $answer)
	{
		$user_details_answers = $this->getUserDetailsAnswers();
		if (empty($user_details_answers)) {
			$user_details_answers = Array();
		}

		$user_details_answers[$asset_id]['answer'] = $answer;
		$result = $this->setAttrValue('user_details_answers', $user_details_answers);

		return $result;

	}//end setUserDetailsAnswer()


	/**
	* Obtain the answer logged for a Quiz Question
	*
	* @param string	$asset_id	The asset ID of an Online Quiz Question
	*
	* @return string
	* @access public
	*/
	function getQuizAnswer($asset_id)
	{
		$quiz_answers = $this->getQuizAnswers();
		$quiz_answer = NULL;

		if (isset($quiz_answers[$asset_id]['answer'])) {
			$quiz_answer = $quiz_answers[$asset_id]['answer'];
		}

		return $quiz_answer;

	}//end getQuizAnswer()


	/**
	* Log an answer which originated from an Online Quiz Question
	*
	* @param string	$asset_id	The asset ID of an Online Quiz Question
	* @param string	$answer		The answer to be assigned
	*
	* @return string
	* @access public
	*/
	function setQuizAnswer($asset_id, $answer)
	{
		$quiz_answers = $this->getQuizAnswers();
		if (empty($quiz_answers)) $quiz_answers = Array();

		$quiz_answers[$asset_id] = $answer;
		$result = $this->setAttrValue('quiz_answers', $quiz_answers);

		return $result;

	}//end setQuizAnswer()


	/**
	* Obtain all answers logged for the User Details Form
	*
	* @return array
	* @access public
	*/
	function getUserDetailsAnswers()
	{
		$user_details_answers = $this->attr('user_details_answers');

		return $user_details_answers;

	}//end getUserDetailsAnswers()


	/**
	* Obtain all logged Online Quiz answers
	*
	* @return array
	* @access public
	*/
	function getQuizAnswers()
	{
		$quiz_answers = $this->attr('quiz_answers');

		return $quiz_answers;

	}//end getQuizAnswers()


	/**
	* Obtain a summary of question names and answers for the User Details Form and Online Quiz
	*
	* @return array
	* @access public
	*/
	function getSummary()
	{
		$summary = Array();

		$user_details_questions = $this->getUserDetailsQuestions();
		$user_details_answers = $this->getUserDetailsAnswers();

		$quiz_questions = $this->getQuizQuestions();
		$quiz_answers = $this->getQuizAnswers();

		foreach ($user_details_answers as $question_id => $answer_details) {
			$summary[$question_id]['name'] = $user_details_questions[$question_id];
			$summary[$question_id]['answer'] = $answer_details['answer'];
		}

		foreach ($quiz_answers as $question_id => $answer_details) {
			$summary[$question_id]['name'] = $quiz_questions[$question_id];
			$summary[$question_id]['answer'] = $answer_details['summary'];
		}

		return $summary;

	}//end getSummary()


	/**
	* Log the question name of a User Details Form question
	*
	* @param string	$asset_id		The asset ID of a User Details Form question
	* @param string	$question_name	The question name to be assigned
	*
	* @return string
	* @access public
	*/
	function setUserDetailsQuestion($asset_id, $question_name)
	{
		$user_details_questions = $this->getUserDetailsQuestions();
		if (empty($user_details_questions)) {
			$user_details_questions = Array();
		}

		$user_details_questions[$asset_id] = $question_name;
		$result = $this->setAttrValue('user_details_questions', $user_details_questions);

		return $result;

	}//end setUserDetailsQuestion()


	/**
	* Log the question name of an Online Quiz Question
	*
	* @param string	$asset_id		The asset ID of an Online Quiz Question
	* @param string	$question_name	The question name to be assigned
	*
	* @return string
	* @access public
	*/
	function setQuizQuestion($asset_id, $question_name)
	{
		$quiz_questions = $this->getQuizQuestions();
		if (empty($quiz_questions)) $quiz_questions = Array();

		$quiz_questions[$asset_id] = $question_name;
		$result = $this->setAttrValue('quiz_questions', $quiz_questions);

		return $result;

	}//end setQuizQuestion()


	/**
	* Obtain all question names logged for the User Details Form
	*
	* @return array
	* @access public
	*/
	function getUserDetailsQuestions()
	{
		return $this->attr('user_details_questions');

	}//end getUserDetailsQuestions()


	/**
	* Obtain all question names logged for Online Quiz Questions
	*
	* @return array
	* @access public
	*/
	function getQuizQuestions()
	{
		return $this->attr('quiz_questions');

	}//end getQuizQuestions()


}//end class
?>
