<?php
/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
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
* $Id: form_section.inc,v 1.12 2003/11/18 15:43:37 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Form_Question
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Section extends Asset
{


	/**
	* the current form object using this section
	*
	* @var &object Form
	*/
	var $active_form = null;

	/**
	* the question index type
	* @var integer
	*/
	var $question_index = 0;

	/**
	* the width of the question name column (used when printing a generic section)
	* @var integer
	*/
	var $question_col_width = 0;

	/**
	* the width of the answer column (used when printing a generic section)
	* @var integer
	*/
	var $answer_col_width = 0;

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
	function Form_Section($assetid=0) 
	{
		$this->Asset($assetid);

	} // end Form_Section()


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

		$copy_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'dependant' => 1, 'exclusive' => 1);

		// create a main bodycopy for header and footer html
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$main_bodycopy = new Bodycopy();
		$main_bodycopy->setAttrValue('name', 'Main Bodycopy');

		$copy_link['value'] = 'main_bodycopy';
		if (!$main_bodycopy->create($copy_link)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		// create a bodycopy for the looping of questions
		$looping_bodycopy = new Bodycopy();
		$looping_bodycopy->setAttrValue('name', 'Looping Bodycopy');
		$copy_link['value'] = 'looping_bodycopy';
		if (!$looping_bodycopy->create($copy_link)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		// create a text question for them to start off with
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('form_question_type_text');
		$question = new Form_Question_Type_Text();
		
		$question->setAttrValue('name', 'Question 1');
		if (!$question->create($copy_link)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return $linkid;

	} // end create()

	
	/**
	* gets called when things change (new links, delete links etc)
	*
	* @see Asset::_updated
	*
	* @access private
	* @return boolean
	*/
	function _updated()
	{
		if (!parent::_updated()) return false;
		if (SQ_IN_BACKEND) {
			return $this->fileRegeneration();
		}
		return true;
	
	} // end _updated()


	/**
	* Returns TRUE if we can delete the passed link, or a string with the error msg
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return mixed TRUE or error msg string
	* @access public
	*/
	function canDeleteLink($linkid)
	{
		// the only reason why we would object to this link being
		// deleted is if it is the bodycopy linked to us
		$main_bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'main_bodycopy');
		$looping_bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'looping_bodycopy');

		if ($main_bodycopy_link[0]['linkid'] == $linkid && !$GLOBALS['SQ_PURGING_TRASH']) return 'You cant delete the link between the main bodycopy and the custom form';

		if ($looping_bodycopy_link[0]['linkid'] == $linkid && !$GLOBALS['SQ_PURGING_TRASH']) return 'You cant delete the link between the looping bodycopy and the custom form';

		return parent::canDeleteLink($linkid);
	
	}// end canDeleteLink()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_TYPE_2]['form_question'] = Array('card' => 'M', 'exclusive' => true);
		$page_links[SQ_LINK_TYPE_2]['bodycopy'] = Array('card' => 'M', 'exclusive' => true);
		return $page_links;

	} // end _getAllowedLinks()


	/**
	* regenerates the content file for this section
	*
	* @param boolean $form_regen	if TRUE the section will get all its forms to regen as well
	*
	* @access public
	* @return boolean
	*/
	function fileRegeneration($form_regen=true)
	{
		// we want to regen this section
		$edit = &$this->getEditFns();
		$edit->generateContentFile($this);

		// sometimes, we might not want to regen the form (eg. if the form is calling this function!)
		if ($form_regen) {
			// then, 
			// get all the links of this guy's form parents
			$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'form', false, 'minor');
			
			foreach ($links as $l) {
				$form = &$GLOBALS['SQ_SYSTEM']->am->getAsset($l['majorid'], $l['major_type_code']);
				if (is_null($form)) continue;
				if (!$form->_updated()) {
					unset($form);
					return false;
				}
				unset($form);
			}
		} // end if

		return true;

	} // end fileRegeneration()


	/**
	* returns the links to the questions of this section
	*
	* @param &object asset the section asset
	*
	* @return array(array links)
	* @access public
	*/
	function getQuestionLinks() 
	{
		return  $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'form_question', false);
	
	} // end getQuestionLinks()


	/**
	* returns the count of questions linked to this section
	*
	* @param &object asset the section asset
	* 
	* @return integer question count
	* @access public
	*/
	function getQuestionCount()
	{
		return $GLOBALS['SQ_SYSTEM']->am->countLinks($this->id, 'major', SQ_LINK_TYPE_2, 'form_question', false);
	
	} // getQuestionCount()


	/**
	* gets a question asset based on its sort order
	*
	* @param &object $asset the asset of the section
	* @param integer $order the order of the question wanted
	*
	* @access public
	* @return &object
	*/
	function &getQuestionByOrder($order)
	{
		$links = $this->getQuestionLinks();
		if (empty($links)) return false;

		$l = $links[$order - 1];
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		if ($a = &$am->getAsset($l['minorid'], $l['minor_type_code'])) {
			return $a;
		}
		return false;
	
	} // end getQuestionByOrder()


	/**
	* gets the questions linked to this section
	*
	* @param &object asset the section asset
	*
	* @return array(&object asset)
	* @access public
	*/
	function &getQuestions()
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$question_links = $this->getQuestionLinks();
		$questions = Array();

		foreach ($question_links as $link) {
			$questions[$link['sort_order']] = &$am->getAsset($link['minorid'], $link['minor_type_code']);
		}
		return (!empty($questions)) ? $questions : Array();

	} // end getQuestions()


	/**
	* prints the body of this section
	*
	* @param int $submission the submission id for this form 
	* @param bool $include signifies if the file is included or echoed
	*
	* @access public
	*/
	function printBody($include=true)
	{
		// get the asset manager for the questions to print out their values
		// (they do this in the included file)
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$active_form =& $this->active_form;

		if (!file_exists($this->data_path.'/content_file.php')) {
			$edit = $this->getEditFns();
			$edit->generateContentFile($this);
		} 
		if ($include) {
			include($this->data_path.'/content_file.php');
		} else {
			if (file_exists($this->data_path.'/content_file.php')) {
				echo file_to_string($this->data_path.'/content_file.php');
			}
		}

	}// end printBody()

	
	/**
	* gets each of this section's question to process the saubmitted values for them
	*
	* @param &object submission_form $submission the submission asset for this form
	*
	* @access public
	* @return boolean
	*/
	function processForm() {	
		// get the asset manager
		$am = &$GLOBALS['SQ_SYSTEM']->am;

		$links = $this->getQuestionLinks();
		foreach ($links as $link) {
			$question = &$am->getAsset($link['minorid'], $link['minor_type_code']);
				
			// let the question know what the current submission object is 
			$question->active_submission =& $this->active_submission;

			// tell the question the name and id of this section
			$question->setSection($this->attr('name'), $this->id);
			
			$question->processForm();
		}
		return true;
	
	} // end processForm()


	/**
	* returns the name of this section
	*
	* @param boolean $short_name if TRUE, returns the short name
	*
	* @access private
	* @return string
	*/
	function _getName($short_name = false)
	{
		return $this->attr('name');

	} // end _getName


	/**
	* returns the question index for a particular question (eg, 1, a, i, I)
	* 
	* @param integer $order the question order
	*
	* @access public
	* @return string
	*/
	function getQuestionIndex($order)
	{
		$index = $this->question_index;
		if (!$index) return '';

		switch ($index) {
			case '1':
				return $order.'.';
			break;
			case '2':
				$i = 96 + $order;
				return chr($i).'.';
			break;
			case '3':
				$i = 64 + $order;
				return chr($i).'.';
			break;
			case '4':
				return strtolower(int_to_roman($order)).'.';
			break;
			case '5':
				return int_to_roman($order).'.';
			break;

		} // end switch
	} // end getQuestionIndex()


}//end class
?>