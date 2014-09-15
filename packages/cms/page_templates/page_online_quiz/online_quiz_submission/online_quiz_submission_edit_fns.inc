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
* $Id: online_quiz_submission_edit_fns.inc,v 1.5 2012/08/30 00:58:09 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Online Quiz Submission edit functions
*
* @author  Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Online_Quiz_Submission_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Online_Quiz_Submission_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

	}//end constructor


	/**
	* Paint interface to edit form submission answers
	*
	* @param object	&$asset	asset being edited
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for html elements
	*
	* @return void
	* @access public
	*/
	function paintSubmissionIpAddress(&$asset, &$o, $prefix)
	{
		echo $asset->getIP();

	}//end paintSubmissionIpAddress()


	/**
	* Paint Submission Score
	*
	* @param object	&$asset	asset being edited
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for html elements
	*
	* @return void
	* @access public
	*/
	function paintSubmissionScore(&$asset, &$o, $prefix)
	{
		echo $asset->getScore();

	}//end paintSubmissionScore()


	/**
	* Paint interface to edit form submission answers
	*
	* @param object	&$asset	asset being edited
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for html elements
	*
	* @return void
	* @access public
	*/
	function paintSubmissionTime(&$asset, &$o, $prefix)
	{
		echo date('d M Y H:i:s',$asset->created);

	}//end paintSubmissionTime()


	/**
	* Paint interface to display User Details responses
	*
	* @param object	&$asset	asset being edited
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for html elements
	*
	* @return void
	* @access public
	*/
	function paintUserDetailsAnswers(&$asset, &$o, $prefix)
	{

		$user_details_answers = $asset->attr('user_details_answers');

		// If it is a system user's submission,(which is not required to fill in user detail form), print the user's detail
		if(!isset($user_details_answers) || empty($user_details_answers)) {
			$sub_user = $asset->created_userid;
			$valid_user = FALSE;
			$o->openField(translate('user'));
						if (!empty($sub_user)) {
							$sub_user_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($sub_user);
							if (!empty($sub_user_asset)) {
								echo get_asset_tag_line($sub_user);
								$valid_user = TRUE;
							}
						}
						if (!$valid_user) {
						 	echo '<i>'.translate('online_quiz_no_logged_in_user').'</i>';
						}
			$o->closeField();
		}
		else { // if the user_details_answers is filled, print out the user's detail
			$form_id = key($GLOBALS['SQ_SYSTEM']->am->getParents(key($user_details_answers), 'simple_form', FALSE));
			$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($form_id);

			// We're using a Simple Form, so here is where it differs from the "Form Submission" asset
			$user_details_questions = $form->getQuestions();

			foreach ($user_details_questions as $question_id => $question) {
					$question = $GLOBALS['SQ_SYSTEM']->am->getAsset($question_id);
				$o->openField($question->getVal('title'), '', $question->getVal('note'));
				$answer_details = $user_details_answers[$question_id];
				echo $answer_details['answer'];
				$o->closeField();
			}
		}
		return TRUE;

	}//end paintUserDetailsAnswers()


	/**
	* Paint interface to display Online Quiz
	*
	* @param object	&$asset	asset being edited
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for html elements
	*
	* @return void
	* @access public
	*/
	function paintQuizAnswers(&$asset, &$o, $prefix)
	{
		$quiz_questions = $asset->attr('quiz_questions');
		$quiz_answers = $asset->attr('quiz_answers');

		foreach ($quiz_answers as $question_id => $answer_details) {
			$question_name = $quiz_questions[$question_id];
			$o->openField($question_name, '', '');
			if (isset($answer_details['summary'])) {
				echo $answer_details['summary'];
			}
			$o->closeField();
		}

		return TRUE;

	}//end paintQuizAnswers()


}//end class
?>
