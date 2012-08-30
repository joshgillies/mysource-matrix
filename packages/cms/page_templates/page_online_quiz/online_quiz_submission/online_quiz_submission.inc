<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: online_quiz_submission.inc,v 1.4 2012/08/30 00:58:09 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Online Quiz Submission
*
* @author  Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.4 $
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
		if (empty($a['misc'])) $a['misc'] = Array();

		$a['misc']['ip'] = $ip;

		if (!$this->setAttrValue('attributes', $a)) {
			return FALSE;
		}

		return TRUE;

	}//end setIP()


	/**
	* Returns the score of this submission
	*
	* @return mixed boolean|string
	* @access public
	*/
	function getScore()
	{
		$a = $this->attr('attributes');
		if (isset($a['misc']['score'])) {
			return $a['misc']['score'];
		}
		
		return FALSE;

	}//end getScore()


	/**
	* Sets the score of this submission
	*
	* @param string	$score
	*
	* @return boolean
	* @access public
	*/
	function setScore($score)
	{
		$a = $this->attr('attributes');
		if (empty($a['misc'])) $a['misc'] = Array();

		$a['misc']['score'] = $score;

		if (!$this->setAttrValue('attributes', $a)) {
			return FALSE;
		}

		return TRUE;

	}//end setScore()
	
	
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
	* @param boolean 	$incl_user_details		include summary for User Details Form?
	* @param boolean 	$incl_quiz_responses	include summary for Online Quiz?
	* 
	* @return array
	* @access public
	*/
	function getSummary($incl_user_details=TRUE, $incl_quiz_responses=TRUE)
	{
		$summary = Array();

		if ($incl_user_details){
			$user_details_questions = $this->getUserDetailsQuestions();
			$user_details_answers = $this->getUserDetailsAnswers();
			foreach ($user_details_answers as $question_id => $answer_details) {
				$summary[$question_id]['name'] = $user_details_questions[$question_id];
				$summary[$question_id]['answer'] = $answer_details['answer'];
			}
		}
		
		if ($incl_quiz_responses){
			$quiz_questions = $this->getQuizQuestions();
			$quiz_answers = $this->getQuizAnswers();
			foreach ($quiz_answers as $question_id => $answer_details) {
				$summary[$question_id]['name'] = $quiz_questions[$question_id];
				$summary[$question_id]['answer'] = $answer_details['summary'];
			}
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

	
	/**
	* Return the keywords for this asset type.
	*
	* This function is called in asset-listing situations like the asset listing, search page etc
	* The return value is in the form:
	* <PRE>
	* Array(
	*   'name' => 'description' => 'the asset name',
	* );
	* </PRE>
	*
	* @return array
	* @access public
	*/
	function getAvailableKeywords()
	{
		$res = parent::getAvailableKeywords();
		$res['quiz_qX_name'] = 'The name of the question with ID X';
		$res['quiz_qX_answer_value'] = 'The answer to the question with ID X';
		$res['quiz_qX_answer_summary'] = 'The summary of the question with ID X';
		$res['quiz_qX_answer_awarded'] = 'The points awarded of the question with ID X';
		$res['quiz_qX_answer_available'] = 'The points available of the question with ID X';
		$res['user_details_X:qY_question'] = 'The user deatil form question with asset ID X:qY';
		$res['user_details_X:qY_answer'] = 'The user deatil form answer with question asset ID X:qY';
		$res['quiz_qX_answer_available'] = 'The points available of the question with ID X';
		$res['user_ip_address'] = 'IP address that created this submission';
		$res['user_details'] = 'User details summary, blank if User Details form not used';
		$res['user_asset_id'] = 'The User who created the Submission,';
		$res['quiz_responses'] = 'Summary of responses nad points awarded and maximum points per answer';
		$res['quiz_total'] = 'Total points awarded and available';
		$res['quiz_score'] = 'Quiz Score';
		return $res;

	}//end getAvailableKeywords()

	
	/**
	* Get bodycopy keyword replacement
	*
	* Provides a mechanism for providing keyword replacement functions by
	* looking up the function named 'getKeywordNameKeywordReplacement'
	* so an asset wishing to add support for 'best_ever' custom keyword
	* should implement a function getBestEverKeywordReplacement()
	* which returns a replacement string
	*
	* @param string	$keyword	Keyword to find a replacement for
	*
	* @return string
	* @access public
	*/
	function getKeywordReplacement($keyword)
	{

		$matches = Array();
		if (preg_match('/quiz\_q([0-9]+)\_name/', $keyword, $matches)) {
			$attrs = $this->attr('quiz_questions');
			foreach ($attrs as $id => $value) {			
				if ($id == $matches[1]) {
					return $value;
				}
			}
			return '';
		}
		
		if (preg_match('/quiz\_q([0-9]+)\_answer_value/', $keyword, $matches)) {
			$attrs = $this->attr('quiz_answers');
			foreach ($attrs as $id => $value) {			
				if ($id == $matches[1]) {
					return $value['value'];
				}
			}
			return '';
		}
		
		if (preg_match('/quiz\_q([0-9]+)\_answer_summary/', $keyword, $matches)) {
			$attrs = $this->attr('quiz_answers');
			foreach ($attrs as $id => $value) {			
				if ($id == $matches[1]) {
					return $value['summary'];
				}
			}
			return '';
		}
		
		if (preg_match('/quiz\_q([0-9]+)\_points_awarded/', $keyword, $matches)) {
			$attrs = $this->attr('quiz_answers');
			foreach ($attrs as $id => $value) {			
				if ($id == $matches[1]) {
					return $value['points'];
				}
			}
			return '';
		}

		if (preg_match('/quiz\_q([0-9]+)\_points_available/', $keyword, $matches)) {
			$attrs = $this->attr('quiz_answers');
			foreach ($attrs as $id => $value) {			
				if ($id == $matches[1]) {
					return $value['available_points'];
				}
			}
			return '';
		}
		
		if (preg_match('/user_details\_([0-9]+:q[0-9]+)\_question/', $keyword, $matches)) {
			
			$attrs = $this->attr('user_details_questions');
			foreach ($attrs as $id => $value) {			
				if ($id == $matches[1]) {
					return $value;
				}
			}
			return '';
		}
		
	
		if (preg_match('/user_details\_([0-9]+:q[0-9]+)\_answer/', $keyword, $matches)) {
			$attrs = $this->attr('user_details_answers');
			foreach ($attrs as $id => $value) {			
				if ($id == $matches[1]) {
					return $value['answer'];
				}
			}
			return '';
		}
		
		if (preg_match('/user_ip_address/', $keyword, $matches)) {
			return $this->getIP();
		}
		
	
		if (preg_match('/user_asset_id/', $keyword, $matches)) {
			return $this->updated_userid;
		}
		
		if (preg_match('/user_details/', $keyword, $matches)) {
			$string = '<table border="0" cellspacing="2" cellpadding="2">'."\n";
			$string .= $this->getSummaryUserDetails();
			$string .="</table>\n";
			return $string;
		}
		
		if (preg_match('/quiz_responses/', $keyword, $matches)) {
			$string = '<table border="0" cellspacing="2" cellpadding="2">'."\n";
			$string .= $this->getSummaryQuizResponse();
			$string .="</table>\n";
			return $string;
		}
		
		if (preg_match('/quiz_total/', $keyword, $matches)) {
			$string = '<table border="0" cellspacing="2" cellpadding="2">'."\n";
			$string .= $this->getSummaryPoints();
			$string .="</table>\n";
			return $string;
		}
		
		if (preg_match('/quiz_score/', $keyword, $matches)) {
			return $this->getScore();
		}
		
		return parent::getKeywordReplacement($keyword);

	}//end getKeywordReplacement()
	
	
	
	/**
	* Print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody() {
		echo '<table border="0" cellspacing="2" cellpadding="2">'."\n";
		echo ($this->getSummaryUserDetails());
		echo '<tr><td><strong>User IP Address</strong></td><td>'.$this->getIP().'</td></tr>';
		echo ($this->getSummaryQuizResponse());
		echo ($this->getSummaryPoints());
		echo "</table>\n";
	
	}//end printBody()
	
	
	/**
	* Get summary of user details form in html format
	*
	* @return string
	* @access public
	*/
	function getSummaryUserDetails()
	{
		$user_details_questions = $this->getUserDetailsQuestions();
		$user_details_answers = $this->getUserDetailsAnswers();
		
		$string = '';
		foreach ($user_details_questions as $id => $value) {
			$string .= "<tr>\n<td><strong>$value</strong></td>\n";
			if(isset($user_details_answers[$id]['answer'])) {
				$string .= "<td>".$user_details_answers[$id]['answer']."</td>";
			}
			$string .= "\n</tr>\n";
		}
		
		if (empty($string)) {
			return "<tr>\n<td><strong>User Asset ID</strong></td>\n<td>$this->updated_userid</td>\n</tr>\n";
		}
		else {
			return $string;
		}
			
	}//end getSummaryUserDetails()
	

	/**
	* Get summary of quiz question in html format
	*
	* @return string
	* @access public
	*/
	function getSummaryQuizResponse()
	{
		$quiz_questions = $this->getQuizQuestions();
		$quiz_answers = $this->getQuizAnswers();
		
		$string = '';
		foreach ($quiz_questions as $id => $value) {
			$string .= "<tr>\n<td><strong>$value</strong></td>\n<td>";
			if(isset($quiz_answers[$id]['summary'])) {
				$string .= $quiz_answers[$id]['summary'].", ";
			}
			if(isset($quiz_answers[$id]['points']) && isset($quiz_answers[$id]['available_points'])) {
				$string .= " ".$quiz_answers[$id]['points']." of ".$quiz_answers[$id]['available_points']." points";
			}
			
			$string .= "</td>\n</tr>\n";
		}
	
		return $string;
					
	}//end getSummaryQuizResponse()
	
	
	/**
	* Get summary of points in html format
	*
	* @return string
	* @access public
	*/
	function getSummaryPoints()
	{

		$quiz_answers = $this->getQuizAnswers();
		$total_points_available = 0;
		$total_points_awarded = 0;
		
		foreach ($quiz_answers as $id => $value) {
			if(isset($value['available_points']))
				$total_points_available += $value['available_points'];
			if(isset($value['points']))
				$total_points_awarded += $value['points'];
		}
	
		$string =  "<tr>\n<td><strong>Total points awarded</strong></td>\n<td>$total_points_awarded</td>\n</tr>\n";
		$string .="<tr>\n<td><strong>Total points available</strong></td>\n<td>$total_points_available</td>\n</tr>\n";
		return $string;
					
	}//end getSummaryPoints()
	
}//end class
?>
