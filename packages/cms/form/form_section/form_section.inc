<?php

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Form_Question
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage cms
*/
class Form_Section extends Asset
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form_Section($assetid=0) 
	{
		$this->Asset($assetid);
	}


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
	* Creates a link between this asset and the passed asset, of a certain type
	* and returns the linkid of the created link
	*
	* @return int
	* @access public
	* @see Asset::creatLink();
	*/
	function createLink(&$minor, $link_type, $value='', $sort_order=-1, $dependant='0', $exclusive='0')
	{
		// if this is a bodycopy table then we need to make it a dependant link
		if (is_a($minor, 'bodycopy')) {
			$dependant = '1';
		}
		return parent::createLink($minor, $link_type, $value, $sort_order, $dependant, $exclusive);
		//return $this->fileRegeneration();

	}// end createLink()


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
		$main_bodycopy_link = $this->getLink(SQ_LINK_TYPE_2, 'main_bodycopy');
		$looping_bodycopy_link = $this->getLink(SQ_LINK_TYPE_2, 'looping_bodycopy');

		if ($main_bodycopy_link[0]['linkid'] == $linkid && !$GLOBALS['SQ_PURGING_TRASH']) return 'You cant delete the link between the main bodycopy and the custom form';

		if ($main_bodycopy_link[0]['linkid'] == $linkid && !$GLOBALS['SQ_PURGING_TRASH']) return 'You cant delete the link between the looping bodycopy and the custom form';

		return parent::canDeleteLink($linkid);
	}//end canDeleteLink()


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

	}//end _getAllowedLinks()


	/**
	* gets called when a asset is moved linked to this section 
	*
	* @access public
	* @return boolean
	*/
	function moveLink()
	{
		parent::moveLink();
		// regen the content file
		return $this->fileRegeneration();
	
	} // end moveLink()


	/**
	* regenerates the content file for this section
	*
	* @access public
	* @return boolean
	*/
	function fileRegeneration()
	{
		// get all the links of this guy's form parents
		$links = $this->getLinks(SQ_LINK_TYPE_2, 'form', false, 'minor');
		
		foreach ($links as $l) {
			$form = &$GLOBALS['SQ_SYSTEM']->am->getAsset($l['majorid'], $l['type_code']);
			if (is_null($form)) continue;
			if (!$form->fileRegeneration()) return false;
		}

		// we also want to regen this section as well
		$edit = $this->getEditFns();
		$edit->generateContentFile($this);

		return true;

	} // end fileRegeneration()


	/**
	* Prints out the backend interface for the asset
	*
	* @param object Backend_Outputter $o
	*
	* @access public
	*/
	function paintBackend(&$o)
	{
			parent::paintBackend($o);
			
	}// end paintBackend()


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
		return $this->getLinks(SQ_LINK_TYPE_2, 'form_question', false);
	
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
		$question_links = $this->getQuestionLinks();
		return count($question_links);
	
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
		if ($a = &$am->getAsset($l['minorid'], $l['type_code'])) {
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
			$questions[$link['sort_order']] = &$am->getAsset($link['minorid'], $link['type_code']);
		}
		return $questions;

	} // end getQuestions()


	/**
	* prints the body of this section
	*
	* @param int $submission the submission id for this form 
	* @param bool $include signifies if the file is included or echoed
	*
	* @access public
	*/
	function printBody($submissionid=0, $include=true)
	{
		// get the asset manager for the questions to print out their values
		// (they do this in the included file)
		$am = &$GLOBALS['SQ_SYSTEM']->am;

		// get the submission object
		if ($submissionid) {
			$submission = &$am->getAsset($submissionid, 'form_submission');
		}
		if (!file_exists($this->data_path.'/content_file.php')) {
			$edit = $this->getEditFns();
			$edit->generateContentFile($this);
		} 
		if ($include) {
			include($this->data_path.'/content_file.php');
		} else {
			echo file_to_string($this->data_path.'/content_file.php');
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
	function processForm(&$submission) {
		if (!$submission) return false;
	
		//get the asset manager
		$am = &$GLOBALS['SQ_SYSTEM']->am;

		$links = $this->getQuestionLinks();
		foreach ($links as $link) {
			if (isset($_POST[$link['minorid']])) {
				$question = &$am->getAsset($link['minorid'], $link['type_code']);
				$question->processForm($submission);
			}
		}
		return true;
	
	} // end processForm()


	/**
	* Returns name of the asset
	*
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @see Asset::_getName()
	* @access public
	*/
	function _getName($short_name=false)
	{
		return $this->attr('name');

	}// end _getName()

}//end class
?>
