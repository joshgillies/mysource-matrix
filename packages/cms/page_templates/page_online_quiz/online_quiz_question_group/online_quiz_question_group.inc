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
* $Id: online_quiz_question_group.inc,v 1.3 2007/05/21 05:00:00 mbrydon Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/folder/folder.inc';


/**
* Online Quiz Question Group
*
* Purpose
*	A group to house Online Quiz Questions
*
* @author  Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Online_Quiz_Question_Group extends Folder
{


	/**
	* Constructor
	*
	* @param string	$assetid	the assetid of the asset to load into this object
	*
	* @return void
	* @access public
	*/
	function Online_Quiz_Question_Group($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		$this->Folder($assetid);

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
		$page_links[SQ_LINK_TYPE_2]['online_quiz_question'] = Array('card' => 'M', 'exclusive' => FALSE);
		$page_links[SQ_LINK_TYPE_3]['online_quiz_question'] = Array('card' => 'M', 'exclusive' => FALSE);
		$page_links[SQ_LINK_TYPE_2]['online_quiz_question_group'] = Array('card' => 'M', 'exclusive' => FALSE);
		$page_links[SQ_LINK_TYPE_3]['online_quiz_question_group'] = Array('card' => 'M', 'exclusive' => FALSE);
		return $page_links;

	}//end _getAllowedLinks()


	/**
	* Returns a list of Online Quiz Question assets associated with this group
	*
	* @return array
	* @access public
	*/
	function getQuestions()
	{
		$questions = $GLOBALS['SQ_SYSTEM']->am->getChildren($this->id, 'online_quiz_question', FALSE);

		return $questions;

	}//end getQuestions()


	/**
	* Returns the Score Categories associated with this group
	*
	* @return array
	* @access public
	*/
	function getScoreCategories()
	{
		return $this->attr('score_categories');

	}//end getScoreCategories()


	/**
	* Returns the applicable Score Category for the score provided
	*
	* @param int	$score	A total score for the Question Group
	*
	* @return string
	* @access public
	*/
	function getScoreCategoryForScore($score)
	{
		$score_category = NULL;
		$score_categories = $this->getScoreCategories();

		foreach ($score_categories as $key => $current_score_category) {
			$min_score = $current_score_category['min_score'];
			$max_score = $current_score_category['max_score'];

			// Score categories can be open-ended or closed. Examples follow:
			// Min score (empty), Max score = 10:	Score of 10 or lower
			// Min score = 0, Max score = 10:		Any score 0 to 10 inclusive
			// Min score = 1, Max score (empty):	Score of 1 or greater

			if ((strlen($min_score) == 0) && (strlen($max_score) > 0) && ($score <= $max_score)) {
				$score_category = $current_score_category;
				break;
			}

			if ((strlen($min_score) > 0) && (strlen($max_score) > 0) && ($score >= $min_score) && ($score <= $max_score)) {
				$score_category = $current_score_category;
				break;
			}

			if ((strlen($min_score) > 0) && (strlen($max_score) == 0) && ($score >= $min_score)) {
				$score_category = $current_score_category;
				break;
			}
		}

		return $score_category;

	}//end getScoreCategoryForScore()


}//end class
?>
