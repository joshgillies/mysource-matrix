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
* $Id: page_poll.inc,v 1.26 2012/08/30 00:58:09 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once SQ_FUDGE_PATH.'/general/datetime.inc';

/**
* Page_Poll
*
* Controls the presentation of Poll_Question children
*
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Robert Howard <rhoward@squiz.net>
* @author  David Heppell <dheppell@squiz.net>
* @version $Revision: 1.26 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Poll extends Page
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

	}//end constructor


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Asset listings create bopycopy and folder asset when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		// add a bodycopy to this page when creating
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');

		$folders = Array('vote_formats','results_formats','past_questions_formats');
		$type = 'folder';

		$formats = Array();
		foreach ($folders as $name) {
			$asset = new $type();
			$copy_link = Array('asset' => &$this, 'value' => $name ,'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

			$asset->setAttrValue('name', ucwords(str_replace('_',' ', $name)));
			if (!$asset->create($copy_link)) return FALSE;

			$formats[$name] = $asset;

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			unset($asset);
		}

		//-- Create Bodycopies --/
		$bodycopies = Array();
		$bodycopies['question_format'] = 'vote_formats';
		$bodycopies['option_list_format'] = 'vote_formats';
		$bodycopies['results_format'] = 'results_formats';
		$bodycopies['results_list_format'] = 'results_formats';
		$bodycopies['past_questions_format'] = 'past_questions_formats';
		$bodycopies['past_questions_list_format'] = 'past_questions_formats';

		foreach ($bodycopies as $value => $folder_asset) {
			$asset = new Bodycopy();
			$copy_link = Array('asset' => &$formats[$folder_asset], 'value' => $value ,'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);
			$asset->setAttrValue('name', ucwords(str_replace('_',' ',$value)));
			if (!$asset->create($copy_link)) return FALSE;
		}

		return TRUE;

	}//end _createAdditional()


	/**
	* Prepares for linking by checking that the link values are valid for this link
	*
	* This function will be called if this asset is the major or minor party in the link, so the side_of_link flag
	* should be checked to work out what side of the link this asset is on. The return value should indicate
	* if any of the link fields were changed.
	*
	* @param object	&$asset			the major or minor asset that we are linking to
	* @param string	$side_of_link	the side of the link we are on (major or minor)
	* @param string	&$link_type		the type of link we are creating
	* @param string	&$value			the value that is to be associated with the link
	* @param string	&$sort_order	the position in the links list that this link should take,
	* @param string	&$dependant		'0' / '1' on whether the this asset is dependant on the
	*								asset that will be linked by the new link
	* @param string	&$exclusive		'0' / '1' on whether the this asset is linked exclusivly
	*								to the asset that will be linked by the new link
	*
	* @return boolean
	* @access public
	*/
	function prepareLink(&$asset, $side_of_link, &$link_type, &$value, &$sort_order, &$dependant, &$exclusive)
	{
		$return_value = FALSE;

		// if this is a poll_question then we need to make it SQ_LINK_TYPE_2
		if ($side_of_link == 'major' &&( $asset instanceof poll_question)) {
			if ($link_type != SQ_LINK_TYPE_2) {
				$link_type = SQ_LINK_TYPE_2;
				$return_value = TRUE;
			}
		}

		return $return_value;

	}//end prepareLink()


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
		$page_links[SQ_LINK_TYPE_2]['folder']   = Array('card' => 'M', 'exclusive' => FALSE);
		$page_links[SQ_LINK_TYPE_2]['poll_question']   = Array('card' => 'M', 'exclusive' => FALSE);

		$page_links[SQ_LINK_NOTICE]['asset']    = Array('card' => 'M', 'exclusive' => FALSE);
		$page_links[SQ_LINK_TYPE_2]['bodycopy'] = Array('card' => 3,   'exclusive' => FALSE);
		return $page_links;

	}//end _getAllowedLinks()


	/**
	* Prints out the body of the asset
	*
	* Switches between the Voting, Results and Past Questions screen
	*
	* @return void
	* @access public
	* @see printFrontend()
	*/
	function printBody()
	{
		// declaration
		$active_question_id	= $this->attr('active_question');
		$format_type 		= 'question_format';
		$prefix 			= $this->getPrefix();
		$can_vote 			= FALSE;
		$am					= $GLOBALS['SQ_SYSTEM']->am;

		// validation
		$valid = TRUE;
		if ($active_question_id) {
			$active_question = $GLOBALS['SQ_SYSTEM']->am->getAsset($active_question_id, '', TRUE);

			// alias question to active_question
			$question =& $active_question;
			if (is_null($question)) $valid = FALSE;

		} else {
			$valid = FALSE;
		}

		if (!$valid) {
			// active question not found
			return;
		}


		// if a previous question is selected...
		if (array_get_index($_GET, 'SQ_POLL_ID') && array_get_index($_GET, 'SQ_ACTION')) {
			$SQ_POLL_ID = (int) $_GET['SQ_POLL_ID'];
			$SQ_ACTION = $_GET['SQ_ACTION'];

			// if the POLL_ID is valid, and we're viewing the results page,
			if (is_integer($SQ_POLL_ID) && $SQ_POLL_ID > 0 && $SQ_ACTION == $prefix.'_results') {

				// grab the question by the poll id
				$question = $GLOBALS['SQ_SYSTEM']->am->getAsset($SQ_POLL_ID, '', TRUE);

				// if the question doesn't have an ID, or isn't of type poll_question, or isn't a child of the poll
				if (!$question->id || !$am->getAssetTypeInfo(Array($question->id), Array('poll_question'), TRUE) || $question->status < SQ_STATUS_LIVE || !$am->getLinkByAsset($this->id, $question->id, SQ_LINK_TYPE_2)) {
						// revert to the default (active_question)
						$question = $active_question;
				}

			}
		}
		// end validation

		// default view, above determines if they can vote
		$format_type = 'results_format';
		$keyword_reps = Array();

		// determine which method is used to check if a user can vote
		if (!$this->attr('allow_multiple_votes')) {
			// A user is logged in
			if ((!is_null($GLOBALS['SQ_SYSTEM']->user->name))  && (!$GLOBALS['SQ_SYSTEM']->userPublic())) {
				// check the "shadow link" table whether this user has already voted...
				if (!$GLOBALS['SQ_SYSTEM']->am->getShadowLinkByAsset($question->id, $GLOBALS['SQ_SYSTEM']->user->id, SQ_LINK_NOTICE)) {
					// add them to the array of previously-voted users, only if they are voting though
					if (isset($_POST['option_id'])) {
						$GLOBALS['SQ_SYSTEM']->am->createShadowAssetLink($question, $GLOBALS['SQ_SYSTEM']->user, SQ_LINK_NOTICE);
					}
					$can_vote = TRUE;
				}
			} else {
				if (array_get_index($_SESSION, $prefix.'_VOTED') == FALSE) {
					$can_vote = TRUE;
				}
			}
		} else {
			$can_vote = TRUE;
		}

		// if the poll was just voted on...
		if (isset($_POST['option_id'])) {
			// save the results (if applicable), then show the results screen
			if ($can_vote) {
				// tie any vote processing to the subsequent printing of results
				// (due to possible slow DB replication issues) using db2
				$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
				$this->_processVote($question, $_POST['option_id']);
			}
			$format_type = 'results_format';
			$keyword_reps = $this->_getResultsFormatKeywordReplacements($question);
		} else {

			// if accessing the results screen by ID of View Results, or cannot vote
			if (array_get_index($_GET, 'SQ_ACTION') == $prefix.'_past_polls') {
				$format_type = 'past_questions_format';
				$keyword_reps = $this->_getPastQuestionFormatKeywordReplacements($question, $active_question_id);
			} else {
				// if can vote, but we're not specifically asking for the results screen
				if ($can_vote && array_get_index($_GET, 'SQ_ACTION') != $prefix.'_results') {
					// set the question to vote on to the active question
					$question     = $active_question;
					$format_type  = 'question_format';
					$keyword_reps = $this->_getQuestionFormatKeywordReplacements($question);
				} else {
					// print the results screen
					$format_type = 'results_format';
					$keyword_reps = $this->_getResultsFormatKeywordReplacements($question);
				}
			}

		}//end if ($_POST['option_id'])

		$format_bodycopy = $this->getFormatAsset($format_type);
		if (!is_null($format_bodycopy)) {
			$keywords = $format_bodycopy->getKeywords();
			foreach ($keywords as $keyword) {
				if (!isset($keyword_reps[$keyword])) {
					$keyword_reps[$keyword] = $this->getKeywordReplacement($keyword);
				}
			}

			$format_bodycopy->setKeywordReplacements($keyword_reps);
			$this->_printFormatAsset($format_bodycopy, $format_type);
		}

		if (isset($_POST['option_id']) && $can_vote) {
			// untie the connection
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		}

	}//end printBody()


	/**
	* Print a format bodycopy
	* Add extra HTML if required (surround the question format with a form tag)
	*
	* @param object	$format_bodycopy	a format bodycopy
	* @param string	$format_type		the format type name
	*
	* @return void
	* @access protected
	*/
	protected function _printFormatAsset($format_bodycopy, $format_type)
	{
		if ($format_type == 'question_format') {
			echo '<form id="'.$this->getPrefix().'_poll" action="'.$this->getURL().'" method="post">';
			$format_bodycopy->printBody();
			echo '</form>';
		} else {
			$format_bodycopy->printBody();
		}

	}//end _printFormatAsset()


	/**
	* Return a reference to a folder linked to this asset determined by the passed in value on the link
	*
	* @param string	$type	the value on the link for this folder
	*
	* @return object
	* @access public
	*/
	function &getFolder($type='type_formats')
	{
		$null = NULL;
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, $type);

		if (empty($link)) return $null;

		$folder = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		if (is_null($folder)) return $null;

		return $folder;

	}//end getFolder()


	/**
	* Returns a list of keywords
	*
	* @param object	&$broadcaster	Broadcaster
	* @param array	$vars			Keywords
	*
	* @return object
	* @access public
	*/
	function onRequestKeywords(&$broadcaster, $vars=Array())
	{
		$parent_folders = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id, 'folder', TRUE);
		$parent_bodycopy = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id, 'bodycopy', TRUE);

		$vote_formats = $this->getFolder('vote_formats');
		$result_formats = $this->getFolder('results_formats');
		$past_questions_formats = $this->getFolder('past_questions_formats');

		$keywords = Array();
		if (in_array($vote_formats->id, array_keys($parent_folders))) {
			$quetion_format = $this->getFormatAsset('question_format');
			$option_list_format = $this->getFormatAsset('option_list_format');
			if (in_array($quetion_format->id, array_keys($parent_bodycopy))) {
				$keywords = $this->_getQuestionFormatKeywords();
			} else if (in_array($option_list_format->id, array_keys($parent_bodycopy))) {
				$keywords = $this->_getOptionListFormatKeywords();
			}
		} else if (in_array($result_formats->id, array_keys($parent_folders))) {
			$results_format = $this->getFormatAsset('results_format');
			$results_list_format = $this->getFormatAsset('results_list_format');
			if (in_array($results_format->id, array_keys($parent_bodycopy))) {
				$keywords = $this->_getResultsFormatKeywords();
			} else if (in_array($results_list_format->id, array_keys($parent_bodycopy))) {
				$keywords = $this->_getResultsListFormatKeywords();
			}
		} else if (in_array($past_questions_formats->id, array_keys($parent_folders))) {
			$past_questions_format = $this->getFormatAsset('past_questions_format');
			$past_questions_list_format = $this->getFormatAsset('past_questions_list_format');
			if (in_array($past_questions_format->id, array_keys($parent_bodycopy))) {
				$keywords = $this->_getPastQuestionsFormatKeywords();
			} else if (in_array($past_questions_list_format->id, array_keys($parent_bodycopy))) {
				$keywords = $this->_getPastQuestionsListFormatKeywords();
			}
		}

		$vars['keywords'] = array_merge($vars['keywords'], $keywords);

	}//end onRequestKeywords()


	/**
	* Keyword replacements
	*
	* @param object	$question	Question Asset
	*
	* @return array
	* @access public
	*/
	function _getQuestionFormatKeywordReplacements($question)
	{

		$keywords = Array(
						'poll_question'						=> $this->_componentQuestion($question),
						'poll_date'							=> $this->_componentDate($question),
						'poll_option_list'					=> $this->_componentOptionList($question),
						'poll_vote_button'					=> $this->_componentVoteButton(),
						'poll_vote_button_new_window'		=> $this->_componentVoteButton(TRUE),
						'poll_vote_button_js'				=> $this->_componentVoteButtonJavascript(),
						'poll_vote_button_js_new_window'	=> $this->_componentVoteButtonJavascript(TRUE),
						'poll_view_results'					=> $this->_componentViewResults(),
						'poll_view_results_url'				=> $this->_componentViewResultsURL(),
						'poll_view_past_questions'			=> $this->_componentViewPastQuestions(),
						'poll_view_past_questions_url'		=> $this->_componentViewPastQuestionsURL(),
						'poll_results_list'					=> $this->_componentGraph($question),
						'poll_active_question'				=> $this->_componentActiveQuestion($question, FALSE),
						'poll_active_question_results'		=> $this->_componentActiveQuestion($question, FALSE, TRUE),
						'poll_active_question_date'			=> $this->_componentActiveQuestion($question, TRUE),
						'poll_active_question_date_results'	=> $this->_componentActiveQuestion($question, TRUE, TRUE),
					);

		return $keywords;

	}//end _getQuestionFormatKeywordReplacements()


	/**
	* Return array of keywords
	*
	* @return array
	* @access public
	*/
	function _getQuestionFormatKeywords()
	{
		$keywords = Array();
		$keywords['poll_question'] = translate('cms_poll_question');
		$keywords['poll_date'] = translate('cms_poll_date');
		$keywords['poll_option_list'] = translate('cms_poll_option_list');
		$keywords['poll_vote_button'] = translate('cms_poll_vote_button');
		$keywords['poll_vote_button_new_window'] = translate('cms_poll_vote_button_new_window');
		$keywords['poll_vote_button_js'] = translate('cms_poll_vote_button_js');
		$keywords['poll_vote_button_js_new_window'] = translate('cms_poll_vote_button_js_new_window');
		$keywords['poll_view_results'] = translate('cms_poll_view_results');
		$keywords['poll_view_results_url'] = translate('cms_poll_view_results_url');
		$keywords['poll_view_past_questions'] = translate('cms_poll_view_past_questions');
		$keywords['poll_view_past_questions_url'] = translate('cms_poll_view_past_questions_url');
		$keywords['poll_active_question'] = translate('cms_poll_active_question');
		$keywords['poll_active_question_results'] = translate('cms_poll_active_question_results');
		$keywords['poll_active_question_date'] = translate('cms_poll_active_question_date');
		$keywords['poll_active_question_date_results'] = translate('cms_poll_active_question_date_results');

		return $keywords;

	}//end _getQuestionFormatKeywords()


	/**
	* Return array of keywords
	*
	* @return array
	* @access public
	*/
	function _getOptionListFormatKeywords()
	{
		$keywords = Array();
		$keywords['poll_option_label'] = translate('cms_poll_option_label');
		$keywords['poll_option_number'] = translate('cms_poll_option_number');
		$keywords['poll_option_id'] = translate('cms_poll_option_id');
		$keywords['poll_option_vote_selection_button'] = translate('cms_poll_option_vote_selection_button');

		return $keywords;

	}//end _getOptionListFormatKeywords()


	/**
	* Return array of keywords
	*
	* @return array
	* @access public
	*/
	function _getResultsFormatKeywords()
	{
		$keywords = Array();
		$keywords = $this->_getQuestionFormatKeywords();
		$keywords['poll_results_list'] = translate('cms_poll_results_list');
		if (isset($keywords['poll_option_list'])) {
			unset($keywords['poll_option_list']);
		}

		return $keywords;

	}//end _getResultsFormatKeywords()


	/**
	* keyword replacements
	*
	* @param object	$question	Question Asset
	*
	* @return array
	* @access public
	*/
	function _getResultsFormatKeywordReplacements($question)
	{
		return $this->_getQuestionFormatKeywordReplacements($question);

	}//end _getResultsFormatKeywordReplacements()


	/**
	* Return array of keywords
	*
	* @return array
	* @access public
	*/
	function _getResultsListFormatKeywords()
	{
		$keywords = Array();
		$keywords['poll_option_number'] = translate('cms_poll_option_number');
		$keywords['poll_option_id'] = translate('cms_poll_option_id');
		$keywords['poll_option_label'] = translate('cms_poll_option_label');
		$keywords['poll_option_bar'] = translate('cms_poll_option_bar');
		$keywords['poll_option_votes_number'] = translate('cms_poll_option_votes_number');
		$keywords['poll_option_votes_percent'] = translate('cms_poll_option_votes_percent');

		return $keywords;

	}//end _getResultsListFormatKeywords()


	/**
	* Return array of keywords
	*
	* @return array
	* @access public
	*/
	function _getPastQuestionsFormatKeywords()
	{
		$keywords = Array();
		$keywords['poll_question_list'] = translate('cms_poll_question_list');
		$keywords['poll_question_list_date'] = translate('cms_poll_question_list_date');
		$keywords['poll_question_list_no_active'] = translate('cms_poll_question_list_no_active');
		$keywords['poll_question_list_date_no_active'] = translate('cms_poll_question_list_date_no_active');
		$keywords['poll_active_question'] = translate('cms_poll_active_question');
		$keywords['poll_date'] = translate('cms_poll_date');


		return $keywords;

	}//end _getPastQuestionsFormatKeywords()


	/**
	* keyword replacements
	*
	* @param object	$question			Question Object
	* @param int	$active_question_id	Asset Id of the active question
	*
	* @return array
	* @access public
	*/
	function _getPastQuestionFormatKeywordReplacements($question, $active_question_id)
	{

		$keywords = Array(
						'poll_date'							=> $this->_componentDate($question),
						'poll_question_list'				=> $this->_componentQuestionList($active_question_id, FALSE, FALSE),
						'poll_question_list_date'			=> $this->_componentQuestionList($active_question_id, TRUE, FALSE),
						'poll_question_list_no_active'		=> $this->_componentQuestionList($active_question_id, FALSE, TRUE),
						'poll_question_list_date_no_active'	=> $this->_componentQuestionList($active_question_id, TRUE, TRUE),
						'poll_active_question'				=> $this->_componentActiveQuestion($question, FALSE),
					);

		return $keywords;

	}//end _getPastQuestionFormatKeywordReplacements()


	/**
	* Return array of keywords
	*
	* @return array
	* @access public
	*/
	function _getPastQuestionsListFormatKeywords()
	{
		$keywords = Array();
		$keywords['poll_question'] = translate('cms_poll_question');
		$keywords = array_merge($keywords,$this->getAvailableKeywords());

		return $keywords;

	}//end _getPastQuestionsListFormatKeywords()


	/**
	* Return a reference to a format linked to this asset
	*
	* @param string	$format	the value on the link for this folder
	*
	* @return object
	* @access public
	*/
	function &getFormatAsset($format='question_format')
	{
		$null = NULL;
		$folders = $this->attr('format_bodycopies');

		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, $folders[$format]);
		if (empty($link)) return $null;

		$folder = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		if (is_null($folder)) return $null;

		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, $format);
		if (empty($bodycopy_link)) return $null;

		$format_bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link['minorid'], $bodycopy_link['minor_type_code']);

		return $format_bodycopy;

	}//end getFormatAsset()


	/**
	* Increments the option voted for
	*
	* Returns TRUE if any valid options were voted for
	*
	* @param object	&$question	reference to poll_question being voted on
	* @param array	$option_ids	key index of the option voted for
	*
	* @return boolean
	* @access private
	*/
	function _processVote(&$question, $option_ids)
	{
		if (!is_array($option_ids)) {
			$option_ids = Array($option_ids);
		}

		$options = $question->attr('options');
		$save = FALSE;
		foreach ($option_ids as $option_id) {
			// if a valid option was voted for...
			if (isset($options[$option_id])) {

				// increase votes, and queue the result for saving
				$options[$option_id]['votes']++;
				$question->setAttrValue('options', $options);
				$save = TRUE;
			}
		}

		if ($save) {
			// override write-protection
			$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
			$question->saveAttributes();
			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

			$_SESSION[$this->getPrefix().'_VOTED'] = TRUE;
		}

		return $save;

	}//end _processVote()


//--        Interface Components        --//


	/**
	* Prints the name of the question
	*
	* @param object	&$question	The question to grab the name from
	*
	* @return string
	* @access private
	*/
	function _componentQuestion(&$question)
	{
		return $question->attr('name');

	}//end _componentQuestion()


	/**
	* Prints the formatted creation date of the question
	*
	* @param object	&$question	The question to grab the creation date from
	*
	* @return string
	* @access private
	*/
	function _componentDate(&$question)
	{
		return easy_datetime($question->created);

	}//end _componentDate()


	/**
	* Returns a radio button and a label for every option in $question
	*
	* @param object	&$question	The question containing the options to print
	*
	* @return string
	* @access private
	*/
	function _componentOptionList(&$question)
	{
		$format_bodycopy = $this->getFormatAsset('option_list_format');
		if (is_null($format_bodycopy)) return FALSE;
		$keywords = Array();

		// grab the option list
		$options = $question->attr('options');
		$output = '';

		// $option_values is an array of the options text and the options votes
		foreach ($options as $option_id => $option_values ) {
			$keywords['poll_option_label'] = $option_values['option_text'];
			$keywords['poll_option_number'] = $option_id + 1;
			$keywords['poll_option_id'] = $option_id;
			ob_start();
			if ($question->attr('allow_multiple')) {
				check_box('option_id[]',$option_id);
			} else {
				radio_button('option_id',$option_id);
			}
			$selection_button = ob_get_clean();
			$keywords['poll_option_vote_selection_button'] = $selection_button;

			ob_start();
			$format_bodycopy->setKeywordReplacements($keywords);
			$format_bodycopy->printBody();
			$output .= ob_get_clean();
		}

		return $output;

	}//end _componentOptionList()


	/**
	* Returns a button that submits the vote selected from _componentOptionList()
	*
	* @param boolean	$new_window	If set to true then results will be displayed in a new browser window
	*
	* @return string
	* @access private
	*/
	function _componentVoteButton($new_window=FALSE)
	{
		ob_start();
		$xtras = '';

		if ($new_window) $xtras .= $this->_newWindowOption();
		submit_button($this->getPrefix().'_poll_vote', $this->attr('vote_button_text'), $xtras);

		return ob_get_clean();

	}//end _componentVoteButton()


	/**
	* Returns the onClick javascript that submits the vote selected from _componentOptionList()
	* eg. &lt;img src="Foo.jpg" onClick="%vote_button_javascript%" /&gt;
	*
	* @param boolean	$new_window	Open poll results in a new window
	*
	* @return string
	* @access private
	*/
	function _componentVoteButtonJavascript($new_window=FALSE)
	{
		$xtras = '';

		if ($new_window) $xtras .= $this->_newWindowOption();

		return $xtras.' document.getElementById(\''.$this->getPrefix().'_poll\').submit();';

	}//end _componentVoteButtonJavascript()


	/**
	* Returns javascript for opening new window on submit
	*
	* @return string
	* @access private
	*/
	function _newWindowOption()
	{
		$width = $this->attr('new_window_width');
		$height = $this->attr('new_window_height');
		$options = $this->attr('new_window_options');

		$params  =	'location='.((strpos($options, 'hide_location'  ) !== FALSE) ? '0' : '1').','
					.'menubar='.((strpos($options, 'hide_menubar'   ) !== FALSE) ? '0' : '1').','
					.'scrollbars='.((strpos($options, 'hide_scrollbars') !== FALSE) ? '0' : '1').','
					.'statusbar='.((strpos($options, 'hide_status'    ) !== FALSE) ? '0' : '1').','
					.'toolbar='.((strpos($options, 'hide_toolbar'   ) !== FALSE) ? '0' : '1').','
					.'resizable='.((strpos($options, 'not_resizable'  ) !== FALSE) ? '0' : '1');

		$xtras = ' window.open(\'\', \'results\', \'width='.$width.',height='.$height.','.$params.'\'); document.getElementById("'.$this->getPrefix().'_poll").target=\'results\';';
		return $xtras;

	}//end _newWindowOption()


	/**
	* Returns a link to the View Results screen
	*
	* @return string
	* @access private
	*/
	function _componentViewResults()
	{
		ob_start();
		?>
		<a href="<?php echo $this->getURL(); ?>?SQ_ACTION=<?php echo $this->getPrefix(); ?>_results"><?php echo $this->attr('view_results_text'); ?></a>
		<?php

		return ob_get_clean();

	}//end _componentViewResults()


	/**
	* Returns the URL to the View Results screen
	*
	* @return string
	* @access private
	*/
	function _componentViewResultsURL()
	{
		return $this->getURL().'?SQ_ACTION='.$this->getPrefix().'_results';

	}//end _componentViewResultsURL()


	/**
	* Returns a link to the Previous Questions screen
	*
	* @return string
	* @access private
	*/
	function _componentViewPastQuestions()
	{
		ob_start();

		?>
		<a href="<?php echo $this->getURL(); ?>?SQ_ACTION=<?php echo $this->getPrefix(); ?>_past_polls"><?php echo $this->attr('previous_questions_text'); ?></a>
		<?php

		return ob_get_clean();

	}//end _componentViewPastQuestions()


	/**
	* Returns the URL to the Previous Questions screen
	*
	* @return string
	* @access private
	*/
	function _componentViewPastQuestionsURL()
	{
		return $this->getURL().'?SQ_ACTION='.$this->getPrefix().'_past_polls';

	}//end _componentViewPastQuestionsURL()


	/**
	*
	* Will determine which graph function to call, based on the
	* attribute 'poll_style'.
	* As this attribute hasn't been implemented, this function
	* calls the HTML graph straight off.
	*
	* @param object	&$question	The question to print the graph of
	*
	* @return string
	* @access private
	*/
	function _componentGraph(&$question)
	{
		return $this->_componentGraphHTML($question);

	}//end _componentGraph()


	/**
	* Returns a graph formatted using user-defined colours
	*
	* @param object	&$question	The question to print the graph of
	*
	* @return string
	* @access private
	*/
	function _componentGraphHTML(&$question)
	{
		$output = '';
		ob_start();

		$question_width = $this->attr('graph_box_width');
		$question_height = $this->attr('graph_box_height');
		$question_colour_bar = $this->attr('graph_box_colour_bar');

		$poll_total = 0;

		$options = $question->attr('options');

		// print out the stylesheet for the graph
		?>
		<style type="text/css">
			#sq-page-poll-graph {
				padding: 10px;
				width: <?php echo $question_width; ?>px;
			}

			#sq-page-poll-graph-bar {
				background: <?php echo $question_colour_bar; ?>;
				margin-left: 20px;
			}

			#sq-page-poll-result {
				float: right;
			}

			#sq-page-poll-key {
				float: left;
			}
		</style>
		<?php

		?>
		<div id="sq-page-poll-graph">
		<?php

		// grab the total number of votes
		foreach ($options as $option_values) {
			$poll_total += $option_values['votes'];
		}

		// ensure no division by zero
		if ($poll_total == 0) $poll_total++;

		$format_bodycopy = $this->getFormatAsset('results_list_format');
		if (is_null($format_bodycopy)) return FALSE;
		$keywords = Array();

		$output .= ob_get_clean();

		// foreach poll option, draw out a graph and its results
		foreach ($options as $option_id => $option_values) {
			ob_start();
			$width = $option_values['votes'] / $poll_total;

			$keywords['poll_option_label'] = $option_values['option_text'];
			$keywords['poll_option_number'] = $option_id + 1;
			$keywords['poll_option_id'] = $option_id;

			ob_start();
				?>
					<div id="sq-page-poll-graph-bar" style="width: <?php echo round($width * $question_width / 2); ?>px">&nbsp;</div>
				<?php
			$bar_output = ob_get_clean();
			$keywords['poll_option_bar'] = $bar_output;
			$keywords['poll_option_votes_number'] = $option_values['votes'];
			$keywords['poll_option_votes_percent'] = number_format(($width*100),0,'.',' ').'%';

			$format_bodycopy->setKeywordReplacements($keywords);
			$format_bodycopy->printBody();
			$output .= ob_get_clean();
		}//end foreach

		ob_start();
		?>
		</div>
		<?php

		$output .= ob_get_clean();
		return $output;

	}//end _componentGraphHTML()


	/**
	* Returns a list of child poll_questions
	*
	* @param int		$active_question_id	The active question
	* @param boolean	$date				If true, print the date next to each item
	* @param boolean	$no_active			If true, do not include the active question in the list
	*
	* @return string
	* @access private
	*/
	function _componentQuestionList($active_question_id, $date=FALSE, $no_active=FALSE)
	{
		$output = '';

		$format_bodycopy = $this->getFormatAsset('past_questions_list_format');
		if (is_null($format_bodycopy)) return FALSE;
		$keywords = Array();

		ob_start();

		// grab all the children of the $this that are poll_questions, and order them by date (oldest -> newest)
		$poll_children = $GLOBALS['SQ_SYSTEM']->am->getLinks(	$this->id, SQ_LINK_TYPE_2,
																'poll_question',
																FALSE,
																'major',
																NULL,
																NULL,
																NULL,
																'created',
																NULL
															);

		// format the array to pass to...
		$asset_id_list = Array();
		foreach ($poll_children as $child_id => $child_data) {
			array_push($asset_id_list, $child_data['minorid']);
		}

		// ... getAssetInfo, which grabs a summary of the asset's children
		$info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($asset_id_list, 'poll_question');


		// loop through the array backwards (we want to order them newest -> oldest)
		array_reverse($info);
		$output = ob_get_clean();

		foreach ($info as $info_index => $info_data) {
			ob_start();
				// if the asset is not live, skip it
				if ($info_data['status'] < SQ_STATUS_LIVE) continue;

				// if the poll is the active poll, and they don't want to print
				// the active poll, jump to the next iteration.
				if ($info_index == $active_question_id && $no_active) {
					continue;
				}

				$link = '';
				ob_start();
				?>
					<a href="<?php echo $this->getURL().'?SQ_ACTION='.$this->getPrefix().'_results&SQ_POLL_ID='.$info_index; ?>">
				<?php

				echo $info_data['name'];
				if ($date) {
					echo '&nbsp;('.easy_datetime(strtotime($info_data['created'])).')';
				}
				if ($info_index == $active_question_id) {
					echo '&nbsp;(<strong>Active</strong>)';
				}

				?>
					</a>
				<?php
				$link .= ob_get_clean();
				$keywords['poll_question'] = $link;
				$poll_question_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($info_index);
				foreach ($format_bodycopy->getKeywords() as $keyword) {
					if (empty($keywords[$keyword])) {
						$keywords[$keyword] = $poll_question_asset->getKeywordReplacement($keyword);
					}
				}

				$format_bodycopy->setKeywordReplacements($keywords);
				$format_bodycopy->printBody();
			$output .= ob_get_clean();
		}//end foreach ($info as ...)

		ob_start();

		$output .= ob_get_clean();
		return $output;

	}//end _componentQuestionList()


	/**
	* Returns the name of the active question
	*
	* @param int		&$active_question	The active question
	* @param boolean	$date				If true, print the date next to the item
	* @param boolean	$results			Link to the results page if true
	*
	* @return string
	* @access private
	*/
	function _componentActiveQuestion(&$active_question, $date, $results=FALSE)
	{
		ob_start();

		?>
		<a href="<?php echo $this->getURL(); ?>?SQ_ACTION=<?php echo $this->getPrefix(); ?><?php echo ($results ? '_results' : ''); ?>">
		<?php

		echo $active_question->attr('name');
		if ($date) {
			echo '&nbsp;('.easy_datetime($active_question->created).')';
		}

		?>
			</a>
		<?php

		return ob_get_clean();

	}//end _componentActiveQuestion()


}//end class


?>
