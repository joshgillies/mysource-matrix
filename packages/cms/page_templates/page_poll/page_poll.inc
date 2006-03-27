<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: page_poll.inc,v 1.5.2.1 2006/03/27 10:52:51 gnoel Exp $
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
* @version $Revision: 1.5.2.1 $
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
	function Page_Poll($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Page($assetid);

	}//end constructor


	/**
	* Prepares for linking by checking that the link values are valid for this link
	*
	* This function will be called if this asset is the major or minor party in the link, so the side_of_link flag
	* should be checked to work out what side of the link this asset is on. The return value should indicate
	* if any of the link fields were changed.
	*
	* @param object Asset	$asset			the major or minor asset that we are linking to
	* @param string		$side_of_link		the side of the link we are on (major or minor)
	* @param string		$link_type		the type of link we are creating
	* @param string		$value			the value that is to be associated with the link
	* @param string		$sort_order		the position in the links list that this link should take,
	* @param string		$dependant		'0' / '1' on whether the this asset is dependant on the
	*						asset that will be linked by the new link
	* @param string		$exclusive		'0' / '1' on whether the this asset is linked exclusivly
	*						to the asset that will be linked by the new link
	*
	* @return boolean
	* @access public
	*/
	function prepareLink(&$asset, $side_of_link, &$link_type, &$value, &$sort_order, &$dependant, &$exclusive)
	{
		$return_value = false;

		// if this is a poll_question then we need to make it a dependant link and of SQ_LINK_TYPE_2
		if ($side_of_link == 'major' && is_a($asset, 'poll_question')) {
			if ($dependant != '1') {
				$dependant = '1';
				$return_value = true;
			}
			if ($link_type != SQ_LINK_TYPE_2) {
				$link_type = SQ_LINK_TYPE_2;
				$return_value = true;
			}
		}

		return $return_value;

	}//end prepareLink()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();

		$page_links[SQ_LINK_TYPE_2]['poll_question'] = Array('card' => 'M', 'exclusive' => FALSE);

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
		$active_question_id = $this->attr('active_question');
		$format_type 		= 'question_format';
		$prefix 			= $this->getPrefix();
		$can_vote 			= false;
		$am					= $GLOBALS['SQ_SYSTEM']->am;

		// validation
		$valid = true;
		if ($active_question_id) {
			$active_question = &$GLOBALS['SQ_SYSTEM']->am->getAsset($active_question_id, '', true);

			// alias question to active_question
			$question = &$active_question;

			if (is_null($question) || $question->status < SQ_STATUS_LIVE) {
				$valid = false;
			}

		} else {
			$valid = false;
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
				$question = &$GLOBALS['SQ_SYSTEM']->am->getAsset($SQ_POLL_ID, '', true);

				// if the question doesn't have an ID, or isn't of type poll_question, or isn't a child of the poll
				if (!$question->id ||
					!$am->getAssetTypeInfo(Array($question->id), Array('poll_question'), true) ||
					$question->status < SQ_STATUS_LIVE ||
					!$am->getLinkByAsset($this->id, $question->id, SQ_LINK_TYPE_2)
					) {
						// revert to the default (active_question)
						$question = $active_question;
				}

			}
		}
		// end validation

		// default view, above determines if they can vote
		$format_type = 'results_format';

		// determine which method is used to check if a user can vote
		switch ($this->attr('Vote_Behaviour')) {
			case 'sessions':
				if (array_get_index($_SESSION, $prefix.'_VOTED') == false) {

					$can_vote = true;
				}
				break;

			case 'login':
				// if the user is logged in...
				if (!is_null($GLOBALS['SQ_SYSTEM']->user->name)) {
					$voters = $question->attr('voters');

					// if the user is not already listed...
					if (!array_get_index($voters, $GLOBALS['SQ_SYSTEM']->user->name)) {
						// add them to the array of previously-voted users
						$voters[$GLOBALS['SQ_SYSTEM']->user->name] = true;

						// setAttrValue() now, saveAttr() in _processVote later on.
						$question->setAttrValue('voters', $voters);
						$can_vote = true;
					}
				}
				break;

			default:
				$can_vote = true;
				break;
		} //end switch ($this->attr('Vote_Behaviour'))

		// if the poll was just voted on...
		if (isset($_POST['option_id'])) {
			// save the results
			if ($can_vote) {
				$this->_processVote($question, $_POST['option_id']);
			}
			$format_type = 'results_format';

		} else {

			// if accessing the results screen by ID of View Results, or cannot vote
			if (array_get_index($_GET, 'SQ_ACTION') == $prefix.'_results') {
				$format_type = 'results_format';
			} else if (array_get_index($_GET, 'SQ_ACTION') == $prefix.'_past_polls' ) {
				$format_type = 'past_questions_format';
			} else if ($can_vote) {
				// set the question to vote on to the active question
				$question = $active_question;
				$format_type = 'question_format';
			}

		}//end if ($_POST['option_id'])


		// define the keywords array
		$keywords = Array(
				'question'						=> $this->_componentQuestion($question),
				'date' 							=> $this->_componentDate($question),
				'option_list'					=> $this->_componentOptionList($question),
				'vote_button' 					=> $this->_componentVoteButton(),
				'vote_button_javascript'		=> $this->_componentVoteButtonJavascript(),
				'view_results'					=> $this->_componentViewResults(),
				'view_results_url'				=> $this->_componentViewResultsURL(),
				'view_past_questions'			=> $this->_componentViewPastQuestions(),
				'view_past_questions_url'		=> $this->_componentViewPastQuestionsURL(),
				'graph'							=> $this->_componentGraph($question),
				'key'							=> $this->_componentKey($question),
				'question_list'					=> $this->_componentQuestionList($active_question_id, false, false),
				'question_list_date'			=> $this->_componentQuestionList($active_question_id, false, true),
				'question_list_no_active'		=> $this->_componentQuestionList($active_question_id, true, false),
				'question_list_date_no_active'	=> $this->_componentQuestionList($active_question_id, true, true),
				'active_question'				=> $this->_componentActiveQuestion($question, false),
				'active_question_date'			=> $this->_componentActiveQuestion($question, true)
		);


		// keyword replacement
		$format = $this->attr($format_type);
		echo replace_keywords($format, $keywords);

	}//end printBody()


	/**
	* Increments the option voted for
	*
	* @param object Poll_Question &$question		reference to poll_question being voted on
	* @param int $option_id		key index of the option voted for
	*
	* @return void
	* @access private
	*/
	function _processVote(&$question, $option_id)
	{
		$options = $question->attr('options');

		// if a valid option was voted for...
		if (isset($options[$option_id])) {

			// increase votes, and queue the result for saving
			$options[$option_id]['votes']++;
			$question->setAttrValue('options', $options);

			// override write-protection
			$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
			$question->saveAttributes();
			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

			if ($this->attr('Vote_Behaviour') == 'sessions') {
				$_SESSION[$this->getPrefix().'_VOTED'] = true;
			}

		}//end if (isset($options[$option_id]))

	}//end _processVote()


//--        Interface Components        --//


	/**
	* Prints the name of the question
	*
	* @param object Poll_Question &$question	The question to grab the name from
	*
	* @access private
	* @return string
	*/
	function _componentQuestion(&$question)
	{
		return $question->attr('name');

	}//end _componentQuestion()


	/**
	* Prints the formatted creation date of the question
	*
	* @param object Poll_Question &$question	The question to grab the creation date from
	*
	* @access private
	* @return string
	*/
	function _componentDate(&$question)
	{
		return easy_datetime($question->created);

	}//end _componentDate()


	/**
	* Returns a radio button and a label for every option in $question
	*
	* @param object Poll_Question &$question	The question containing the options to print
	*
	* @access private
	* @return string
	*/
	function _componentOptionList(&$question)
	{
		ob_start();

		// grab the option list
		$options = $question->attr('options');

		?>
		<form name="<?php echo $this->getPrefix(); ?>_poll" action="<?php echo $this->getURL(); ?>" method="post" >
		<?php

		// $option_values is an array of the options text and the options votes
		foreach ($options as $option_id => $option_values ) {
			radio_button('option_id',$option_id);
			echo '&nbsp;'.$option_values['option_text'] . '<br />';
		}

		?>
		</form>
		<?php

		return ob_get_clean();

	}//end _componentOptionList()


	/**
	* Returns a button that submits the vote selected from _componentOptionList()
	*
	* @access private
	* @return string
	*/
	function _componentVoteButton()
	{
		ob_start();

		submit_button($this->getPrefix().'_poll', 'Vote', 'document.'.$this->getPrefix().'_poll.submit();');

		return ob_get_clean();

	}//end _componentVoteButton()


	/**
	* Returns the onClick javascript that submits the vote selected from _componentOptionList()
	* eg. &lt;img src="Foo.jpg onClick="%vote_button_javascript%" /&gt;
	*
	* @access private
	* @return string
	*/
	function _componentVoteButtonJavascript()
	{
		return 'document.'.$this->getPrefix().'_poll.submit();';

	}//end _componentVoteButtonJavascript()


	/**
	* Returns a link to the View Results screen
	*
	* @access private
	* @return string
	*/
	function _componentViewResults()
	{
		ob_start();

		?>
		<a href="<?php echo $this->getURL(); ?>?SQ_ACTION=<?php echo $this->getPrefix(); ?>_results">View Results</a>
		<?php

		return ob_get_clean();

	}//end _componentViewResults()


	/**
	* Returns the URL to the View Results screen
	*
	* @access private
	* @return string
	*/
	function _componentViewResultsURL()
	{
		return $this->getURL().'?SQ_ACTION='.$this->getPrefix().'_results';

	}//end _componentViewResultsURL()


	/**
	* Returns a link to the Previous Questions screen
	*
	* @access private
	* @return string
	*/
	function _componentViewPastQuestions()
	{
		ob_start();

		?>
		<a href="<?php echo $this->getURL(); ?>?SQ_ACTION=<?php echo $this->getPrefix(); ?>_past_polls">View Previous Questions</a>
		<?php

		return ob_get_clean();

	}//end _componentViewPastQuestions()


	/**
	* Returns the URL to the Previous Questions screen
	*
	* @access private
	* @return string
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
	* @param object Poll_Question &$question	The question to print the graph of
	*
	* @access private
	* @return string
	*/
	function _componentGraph(&$question)
	{
		return $this->_componentGraphHTML($question);

	}//end _componentGraph()


	/**
	* Returns a graph formatted using user-defined colours
	*
	* @param object Poll_Question &$question	The question to print the graph of
	*
	* @access private
	* @return string
	*/
	function _componentGraphHTML(&$question)
	{
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
		if ($poll_total == 0) {
			$poll_total++;
		}

		// foreach poll option, draw out a graph and its results
		foreach ($options as $option_id => $option_values) {
			$width = $option_values['votes'] / $poll_total;
			?>
			<div style="padding:5px; margin:5px;">
				<div id="sq-page-poll-result" style="padding: 5px"><?php echo $option_values['votes']?></div>
				<div id="sq-page-poll-key"><?php echo ($option_id + 1) . ') '; ?></div>
				<div id="sq-page-poll-graph-bar" style="width: <?php echo round($width * $question_width / 2); ?>px">&nbsp;</div>
			</div>
			<?php
		}//end foreach

		?>
		</div>
		<?php

		return ob_get_clean();

	}//end _componentGraphHTML()


	/**
	* Returns a graph key (legend) using user-defined colours
	*
	* @param object Poll_Question &$question	The question to print the key of
	*
	* @access private
	* @return string
	*/
	function _componentKey(&$question)
	{
		ob_start();

		?>
		<div>
		<ul>
		<?php

		// list each of the options for the poll
		foreach ($question->attr('options') as $option_id => $option_values) {
			?>
			<li><?php echo ($option_id + 1) .') '.$option_values['option_text']; ?></li>
			<?php
		}

		?>
		</ul>
		</div>
		<?php

		return ob_get_clean();

	}//end _componentKey()


	/**
	* Returns a list of child poll_questions
	*
	* @param int $active_question_id	The active question
	* @param boolean $date				If true, print the date next to each item
	* @param boolean $no_active			If true, do not include the active question in the list
	*
	* @access private
	* @return string
	*/
	function _componentQuestionList($active_question_id, $date=false, $no_active=false)
	{
		ob_start();

		// grab all the children of the $this that are poll_questions, and order them by date (oldest -> newest)
		$poll_children = &$GLOBALS['SQ_SYSTEM']->am->getLinks(	$this->id, SQ_LINK_TYPE_2,
																'poll_question',
																false,
																'major',
																null,
																null,
																null,
																'created',
																null
															);


		?>
		<ul>
		<?php

		// format the array to pass to...
		$asset_id_list = Array();
		foreach ($poll_children as $child_id => $child_data) {
			array_push($asset_id_list, $child_data['minorid']);
		}

		// ... getAssetInfo, which grabs a summary of the asset's children
		$info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($asset_id_list, 'poll_question');


		// loop through the array backwards (we want to order them newest -> oldest)
		array_reverse($info);
		foreach ($info as $info_index => $info_data) {

			// if the asset is not live, skip it
			if ($info_data['status'] < SQ_STATUS_LIVE) {
				continue;
			}

			// if the poll is the active poll, and they don't want to print
			// the active poll, jump to the next iteration.
			if ($info_index == $active_question_id && $no_active) {
				continue;
			}


			?>
			<li>
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
			</li>
			<?php


		}// end foreach ($info as ...)
		?>
		</ul>
		<?php

		return ob_get_clean();

	}//end _componentQuestionList()


	/**
	* Returns the name of the active question
	*
	* @param int $active_question_id	The active question
	* @param boolean $date				If true, print the date next to the item
	*
	* @access private
	* @return string
	*/
	function _componentActiveQuestion(&$active_question, $date)
	{
		ob_start();

		?>
		<a href="<?php echo $this->getURL(); ?>?SQ_ACTION=<?php echo $this->getPrefix(); ?>_results">
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
