<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: form_question_rule.inc,v 1.4 2003/09/26 05:26:41 brobertson Exp $
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
	function processForm(&$submission, $questionid, $question_name)
	{
		return true;

	} // end process form()


	/**
	* returns a list of question types that are selections
	* the rule uses this information to see if it should be displaying
	* any more information in the rule itself
	*
	* @access public
	* @return Array()
	*/
	function getSelectionTypeQuestions()
	{
		$questions = Array(
			'form_question_type_select',
			'form_question_type_tickbox',
		);

		return $questions;
	
	} // end getSelectionTypeQuestions()


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
	* returns TRUE if the question that this rule belongs to is a selection type question
	*
	* @access public
	* @return boolean
	*/
	function isSelection()
	{
		if (in_array($this->getMyQuestionType(), $this->getSelectionTypeQuestions())) {
			return true;
		}
		return false;
	
	} // end isSelection()

	/**
	* prints a dropdown list of questions (id => question name)
	*
	* @access public
	* @return boolean
	*/
	function getQuestionDropdownList()
	{
		return true;

	} // end getQuestionDropdownList()

} // end class

?>
