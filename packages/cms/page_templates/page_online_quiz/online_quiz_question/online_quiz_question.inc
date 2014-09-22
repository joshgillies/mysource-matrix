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
* $Id: online_quiz_question.inc,v 1.10 2012/08/30 00:58:09 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';


define('SQ_ONLINE_QUIZ_QUESTION_SESSION_VAR', 'SQ_ONLINE_QUIZ');


/**
* Online_Quiz_Question
*
* Purpose
*	Abstract class from which online quiz questions inherit
*
*	NOTE: It is recommended that a question only be used once per user, even if the question is shared amongst different quizes
*	Simultaneously taking two tests with the same question contained in each will cause both questions to share data
*
*
* @author  Robert Howard <rhoward@squiz.net>
* @author  Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Online_Quiz_Question extends Asset
{


	/**
	* This value should never be set without validation value
	*
	* @var mixed	$_value	the question's value for the current user
	*
	* @access private
	*/
	var $_value = NULL;


	/**
	* @var int	$_position	the current position in the question list
	*
	* @access private
	*/
	var $_position = NULL;


	/**
	* Constructor
	*
	* @param string	$assetid	the assetid of the asset to load into this object
	*
	* @return void
	* @access public
	*/
	function Online_Quiz_Question($assetid=0)
	{
		$this->Asset($assetid);

		// on question load, load up the question values for the current user
		$this->loadQuestion();

	}//end constructor


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=FALSE)
	{
		return $this->attr('name');

	}//end _getName()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowedLinks()
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_TYPE_2]['bodycopy'] = Array('card' => 1, 'exclusive' => FALSE);
		$page_links[SQ_LINK_TYPE_3]['bodycopy'] = Array('card' => 1, 'exclusive' => FALSE);
		return $page_links;

	}//end _getAllowedLinks()


	/**
	* Retrieve the question data
	*
	* @return boolean
	* @access public
	*/
	function loadQuestion()
	{
		$question = Array(
						'type_code'	=> $this->type(),
						'value'		=> NULL,
						'position'	=> NULL,
					);

		// load everything from the session
		if (!empty($_SESSION[SQ_ONLINE_QUIZ_QUESTION_SESSION_VAR]['question'][$this->id])) {
			$question = $_SESSION[SQ_ONLINE_QUIZ_QUESTION_SESSION_VAR]['question'][$this->id];
		}

		// use the question data to set instance vars
		$this->_value = array_get_index($question, 'value', NULL);
		$this->_position = array_get_index($question, 'position', NULL);

		return TRUE;

	}//end loadQuestion()


	/**
	* Save the question data
	*
	* @return boolean
	* @access public
	*/
	function saveQuestion()
	{
		$question = $this->getQuestionData();

		// save everything from the session
		$_SESSION[SQ_ONLINE_QUIZ_QUESTION_SESSION_VAR]['question'][$this->id] = $question;

		return TRUE;

	}//end saveQuestion()


	/**
	* Returns the current user response
	*
	* @return string
	* @access public
	*/
	function getValue()
	{
		return $this->_value;

	}//end getValue()


	/**
	* Sets the current user response
	*
	* @param mixed	$value	the value to set
	*
	* @return void
	* @access public
	*/
	function setValue($value)
	{
		$this->_value = $value;

	}//end setValue()


	/**
	* Returns the current user response in a human-readable format
	*
	* @return string
	* @access public
	*/
	function getSummary()
	{
		return $this->getValue();

	}//end getSummary()


	/**
	* Returns the correct response
	*
	* @return string
	* @access public
	*/
	function getCorrectValue()
	{
		return $this->getValue();

	}//end getCorrectValue()


	/**
	* Returns the correct response in a human-readable format
	*
	* @return string
	* @access public
	*/
	function getCorrectSummary()
	{
		return $this->getSummary();

	}//end getCorrectSummary()


	/**
	* Returns the current position for this question in the question list
	*
	* @return int
	* @access public
	*/
	function getPosition()
	{
		return $this->_position;

	}//end getPosition()


	/**
	* Sets the current position for this question in the question list
	*
	* @param int	$position	the position number to set
	*
	* @return void
	* @access public
	*/
	function setPosition($position)
	{
		$this->_position = $position;

	}//end setPosition()


	/**
	* Returns either the straight question text, or gets the question text bodycopy to print itself
	*
	* @return string
	* @access public
	*/
	function getQuestionText()
	{
		ob_start();
			$printed = $this->printQuestionTextBodycopy();
		$question_text = ob_get_clean();

		if ($printed) {
			return $question_text;
		} else {
			return $this->attr('question_text');
		}

	}//end getQuestionText()


	/**
	* Returns the Question Note associated with this question
	*
	* @return string
	* @access public
	*/
	function getQuestionNote()
	{
		$question_note = $this->attr('question_note');

		// replace internal links to pages and images with their full URLs
		$matches = Array();
		preg_match_all('|\./\?a=([0-9]+)|', $question_note, $matches);
		$urls = $GLOBALS['SQ_SYSTEM']->am->getAssetURL(array_unique($matches[1]));
		foreach ($urls as $assetid => $url) {
			$question_note = preg_replace('|\./\?a='.$assetid.'([^0-9])|', $url.'\\1', $question_note);
		}

		return $question_note;

	}//end getQuestionNote()


	/**
	* Returns the question data array
	*
	* @return array
	* @access public
	*/
	function getQuestionData()
	{
		return Array(
				'type_code'	=> $this->type(),
				'value'		=> $this->getValue(),
				'position'	=> $this->getPosition(),
			   );

	}//end getQuestionData()


	/**
	* Processes the form using submitted values
	*
	* NOTE: Returns TRUE if the value is valid, otherwise returns FALSE or an array in the following format:
	* <pre>
	* Array(
	* 	'errors'	=> Array('Sample error string'),
	* 	'warnings'	=> Array('Question #12 has not been answered'),
	* )
	* </pre>
	*
	* @return boolean
	* @access public
	*/
	function processResponseForm()
	{
		// children should look at $_REQUEST, validate the result, and set $this->_value
		$value = array_get_index($_REQUEST, $this->getPrefix().'_test', NULL);

		$valid = $this->validateValue($value);
		$errors = array_get_index($valid, 'errors', Array());
		if (empty($errors)) {
			$this->_value = $value;
			$this->saveQuestion();
		}

		// return the error stack
		return $valid;

	}//end processResponseForm()


	/**
	* Determine whether or not this question
	* currently has a valid value
	*
	* NOTE: Returns an array in the following format:
	* <pre>
	* Array(
	* 	'errors'	=> Array('Sample error string'),
	* 	'warnings'	=> Array('Question #12 has not been answered'),
	* )
	* </pre>
	* The 'errors' and 'warnings' keys will always be present, but empty if no errors
	* or warnings need to be returned
	*
	* @param mixed	$value	Custom value to compare with - if left NULL (or unspecified), use the current value of the question
	*
	* @return array
	* @access public
	*/
	function validateValue($value=NULL)
	{
		if (is_null($value)) $value = $this->_value;

		$errors = Array(
					'errors'	=> Array(),
					'warnings'	=> Array(),
				  );

		if (empty($value)) {
			$errors['warnings'][] = sprintf(translate('Question #%s has not been answered.'), $this->getPosition());

		}

		return $errors;

	}//end validateValue()


	/**
	* Returns the javascript used to validate the question
	*
	* @return string
	* @access public
	*/
	function getValidationJS()
	{
		return '';

	}//end getValidationJS()


	/**
	* Determine how many points the given value (or the question's current value) is worth
	*
	* @param mixed	$value	the value to use determining the number of points scored - if NULL, use the question's value
	*
	* @return int
	* @access public
	*/
	function getPoints($value=NULL)
	{
		if (is_null($value)) $value = $this->getValue();

		return 0;

	}//end getPoints()


	/**
	* Determine how many points are available from this question in total
	*
	* @return int
	* @access public
	*/
	function getAvailablePoints()
	{
		return 0;

	}//end getAvailablePoints()


	/**
	* Compiles the results array to be used by the quiz during marking
	*
	* The results are returned in the following format:
	* <pre>
	* Array(
	*	'assetid'			=> assetid,
	*	'value'				=> string,	// flattened raw value
	*	'summary'			=> string,	// flattened human-readible value
	*	'points'			=> int,		// earned points
	*	'available_points'	=> int,		// available points
	* )
	* </pre>
	*
	* @return array
	* @access public
	*/
	function getResults()
	{
		$value           = $question->getValue();
		$summary         = $question->getSummary();
		$correct_value   = $question->getCorrectValue();
		$correct_summary = $question->getCorrectSummary();
		$position        = $question->getPosition();
		$points          = $question->getPoints();
		$available       = $question->getAvailablePoints();

		$results   = Array(
						'assetid'			=> $id,
						'value'				=> $value,
						'summary'			=> $summary,
						'correct_value'		=> $correct_value,
						'correct_summary'	=> $correct_summary,
						'points'			=> $points,
						'available_points'	=> $available,
					 );

		return $results;

	}//end getResults()


//--        Bodycopy Manipulation        --//


	/**
	* Creates the Question Text bodycopy
	*
	* @param boolean	$enable_on_create	if TRUE, will set the bodycopy to be enabled
	*										(TYPE_2 link) when it is created. If FALSE,
	*										creates it disabled (TYPE_3 link).
	*
	* @return boolean
	* @access public
	*/
	function createQuestionTextBodycopy($enable_on_create=TRUE)
	{
		$bodycopy_link = $this->getQuestionTextBodycopyLink(SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3);

		// we already have a bodycopy link: bail out
		if ($bodycopy_link) {
			return FALSE;
		} else {
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');

			$link_type =  ($enable_on_create ? SQ_LINK_TYPE_2 : SQ_LINK_TYPE_3);

			$asset = new Bodycopy();
			$copy_link =  Array(
							'asset'			=> &$this,
							'value'			=> 'question_text',
							'link_type'		=> $link_type,
							'is_dependant'	=> 1,
							'is_exclusive'	=> 1,
						  );

			$asset->setAttrValue('name', translate('Question Text'));

			$args = Array(
						'content'	=> $this->attr('question_text'),
					);
			if (!$asset->create($copy_link, $args)) return FALSE;

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			unset($asset);
		}

		return TRUE;

	}//end createQuestionTextBodycopy()


	/**
	* Returns the Question Text bodycopy
	*
	* @param boolean	$only_if_enabled	if TRUE, will return NULL if the bodycopy is
	*										disabled. If FALSE, will only return NULL if
	*										never created.
	*
	* @return mixed object|NULL
	* @access public
	*/
	function &getQuestionTextBodycopy($only_if_enabled=TRUE)
	{
		$asset = NULL;

		if ($only_if_enabled) {
			$link_types = SQ_LINK_TYPE_2;
		} else {
			$link_types = SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3;
		}

		$bodycopy_link = $this->getQuestionTextBodycopyLink($link_types);
		if ($bodycopy_link) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link['minorid'], 'bodycopy');
		}

		return $asset;

	}//end getQuestionTextBodycopy()


	/**
	* Returns the link to the Question Text bodycopy
	*
	* @param int	$link_type	ensures the link returned is of this type (eg. SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3)
	*
	* @return array
	* @access public
	*/
	function getQuestionTextBodycopyLink($link_type)
	{
		$tmp_bodycopy = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, $link_type, 'bodycopy', FALSE, 'major', 'question_text');
		return reset($tmp_bodycopy);

	}//end getQuestionTextBodycopyLink()


	/**
	* Handles the enabling or disabling of the Question Text bodycopy
	*
	* @param boolean	$enabled	set to TRUE to enable the format bodycopy, and FALSE to disable it
	*
	* @return boolean
	* @access public
	*/
	function setQuestionTextBodycopy($enabled)
	{
		$bodycopy_link = $this->getQuestionTextBodycopyLink(SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3);

		if (!$bodycopy_link) {
			// no bodycopy yet? If we're trying to set to disabled, then we don't
			// need to do anything - if not then we need to create it
			if ($enabled) {
				if (!$this->createQuestionTextBodycopy()) return FALSE;
			}
		} else {
			// set link type to either TYPE_2 if enabled or TYPE_3 if disabled
			$new_link_type = ($enabled) ? SQ_LINK_TYPE_2 : SQ_LINK_TYPE_3;
			if ($bodycopy_link['link_type'] != $new_link_type) {
				$GLOBALS['SQ_SYSTEM']->am->updateLink($bodycopy_link['linkid'], $new_link_type);
			}
		}

		return TRUE;

	}//end setQuestionTextBodycopy()


	/**
	* Prints the "Question Text" bodycopy (if available)
	*
	* @return boolean
	* @access public
	*/
	function printQuestionTextBodycopy()
	{
		$bodycopy_printed =  FALSE;
		$bodycopy         = $this->getQuestionTextBodycopy();

		if (!is_null($bodycopy)) {
			$bodycopy->printBody();
			$bodycopy_printed = TRUE;
		}

		return $bodycopy_printed;

	}//end printQuestionTextBodycopy()


//--        Keyword Replacements        --//


	/**
	* Return the available keywords from the parents
	*
	* The return value is in the form:
	* <pre>
	* Array(
	*     'keyword' => 'name',
	* )
	* </pre>
	*
	* @return array
	* @access public
	*/
	function getAvailableKeywords()
	{
		$keywords = parent::getAvailableKeywords();

		$keywords['question_text']		= translate('Question Text');

		$keywords['response_form']		= translate('Response Form');

		$keywords['available_points']	= translate('Available Points');

		$keywords['question_number']	= translate('Question Number');

		$keywords['question_note']		= translate('Question Note');


		return $keywords;

	}//end getAvailableKeywords()


	/**
	* Get the replacement for the "question_text" keyword
	*
	* @return string
	* @access public
	*/
	function getQuestionTextKeywordReplacement()
	{
		return $this->getQuestionText();

	}//end getQuestionTextKeywordReplacement()


	/**
	* Get the replacement for the "response_form" keyword
	*
	* To be overridden by the children
	*
	* @return string
	* @access public
	*/
	function getResponseFormKeywordReplacement()
	{
		// print form element(s), eg. a text box, or list of checkboxes
		// $this->getPrefix should be used for any form elements

		$prefix = $this->getPrefix();

		// example element
		// <input id="PREFIX_test" name="PREFIX_test" type="text" value="online_quiz_question::getValue()">

		return '';

	}//end getResponseFormKeywordReplacement()


	/**
	* Get the replacement for the "available_points" keyword
	*
	* @return string
	* @access public
	*/
	function getAvailablePointsKeywordReplacement()
	{
		return $this->getAvailablePoints();

	}//end getAvailablePointsKeywordReplacement()


	/**
	* Get the replacement for the "question_number" keyword
	*
	* @return string
	* @access public
	*/
	function getQuestionNumberKeywordReplacement()
	{
		return $this->getPosition();

	}//end getQuestionNumberKeywordReplacement()


	/**
	* Get the replacement for the "question_note" keyword
	*
	* @return string
	* @access public
	*/
	function getQuestionNoteKeywordReplacement()
	{
		return $this->getQuestionNote();

	}//end getQuestionNoteKeywordReplacement()


}//end class
?>
