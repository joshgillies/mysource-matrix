<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
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
* $Id: form_section.inc,v 1.20 2004/06/30 00:47:00 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/bridge/bridge.inc';

/**
* Form_Section
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Section extends Bridge
{


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
	* the active submission array
	* @var Array
	*/
	var $active_submission = Array();


	/**
	* the active submission array
	* @var Array
	*/
	var $submission_errors = Array();
	
	
	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form_Section($assetid=0) 
	{
		$this->_ser_attrs = true;
		$this->Bridge($assetid);

	}//end Form_Section()


	/**
	* Returns a reference to the asset represented by the passed assetid
	*
	* @param int		$assetid		the asset id to be loaded
	* @param string		$type_code		if this exists then this object is used to
	*									load the asset, if not then the DB is queried
	*									to find out the asset type
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed because
	*									you can't use the '@' operator when returning by reference
	*
	* @return object Asset or null on error
	* @access public
	* @see Asset_Manager::&getAsset()
	*/
	function &getAsset($shadowid, $type_code='', $mute_errors=false)
	{
		$asset = null;
		if (empty($shadowid)) return $asset;

		if ($shadowid{0} == 'q') {	// Questions will be preceded with a 'q'
			
			$questionid = substr($shadowid, 1, strlen($shadowid));
			$data = $this->getQuestionById($questionid);
			if (empty($data)) return $asset;

			$data['questionid'] = $questionid;

			$GLOBALS['SQ_SYSTEM']->am->includeAsset($data['question_type_code']);

			$type_code = $data['question_type_code'];

			// this eval returns the necessary object for the question type
			$asset = new $type_code($this->id, $data);
			return $asset;

		}

	}//end getAsset()
	
	
	/**
	* Return all links that this asset has to other assets
	*
	* @param int			$assetid			id of the the $side_of_link asset
	* @param int			$link_types			integer that can be the product of bitwise operations
	*											on the SQ_LINK_* constants
	* @param string|array	$type_code			the type of asset that is linked
	*											(eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param bool			$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	* @param string			$side_of_link		Which side of the link this (the current) asset
	*											is on ('major' or 'minor')
	* @param string			$sort_by			a field in the asset table you would like the results
	*											sorted by (eg. name, short_name etc)
	*
	* @return array
	* @access public
	* @see Asset_Manager::getLinks()
	*/
	function getLinks($assetid, $link_types, $type_code='', $strict_type_code=true, $side_of_link='major', $sort_by=null)
	{
		// there are no links away from the shadow asset (ie:questions)
		if (!is_numeric($assetid)) return Array();

		if (!isset($this->_tmp['getLinks'][$assetid][$side_of_link])) {

			// no section links here, but we do need question shadow asset links
			$entries = $this->getQuestions();
			$links = Array();

			$i = 0;

			foreach ($entries as $questionid => $data) {

				$links[] = Array('linkid' => 0,
								 'majorid' => $this->id,
								 'minorid' => $this->id.':q'.$questionid,
								 'minor_type_code' => $data['question_type_code'],
								 'value' => '',
								 'link_type' => SQ_LINK_TYPE_2,
								 'dependant' => true,
								 'exclusive' => false,
								 'sort_order' => $i,
								);
				$i++;
			}

			$this->_tmp['getLinks'][$assetid][$side_of_link] = $links;

		}

		return $this->_tmp['getLinks'][$assetid][$side_of_link];

	}//end getLinks()
	
		
	/**
	* Get all asset ids that are below the passed assetid in the various trees in which it exists
	*
	* @param int			$assetid			the id of the asset to get its children for
	* @param string|array	$type_code			the type of asset that is linked
	*											(eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param bool			$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	* @param string			$sort_by			a field in the asset table you would like the results
	*											sorted by (eg. name, short_name etc)
	*
	* @return array(int => string)	returns an array of assetids and their type code
	* @access public
	* @see Asset_Manager::getChildren()
	*/
	function getChildren($assetid, $type_code='', $strict_type_code=true, $sort_by=null)
	{
		if (!is_numeric($assetid)) return Array();

		if (!is_array($type_code)) {
			if (empty($type_code)) $type_code = Array();
			else $type_code = Array($type_code);
		}
		
		$children = Array();

		//if (empty($this->_tmp['questions'])) {
		$entries = $this->getQuestions();
		//
		//$entries = $this->_tmp['questions'];

		foreach ($entries as $questionid => $data) {
			$children[$this->id.':q'.$questionid] = $data['question_type_code'];
		}

		return $children;

	}//end getChildren()
	
	
	/**
	* Get all asset ids that are above the passed assetid in the various trees in which it exists
	*
	* Can only find parents of type <i>form_section</i> or <i>form</i>
	*
	* @param int			$assetid			the id of the asset to get its parents for
	* @param string|array	$type_code			the type of asset that is linked (eg 'Form_Section', 'Form_Question', etc)
	*											if an array returns link if matches any of the array values
	* @param bool			$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	*
	* @return array(int)
	* @access public
	* @see Asset_Manager::getParents()
	*/
	function getParents($assetid, $type_code='', $strict_type_code=true)
	{
		// there are no links away from the shadow asset (ie:questions)
		if (!is_numeric($assetid)) return Array($this->id);

		$parents = Array();
		if (!is_array($type_code
			)) $type_code = Array($type_code);
		if (!in_array('form_section', $type_code) && !in_array('form', $type_code)) return Array();

		if (!isset($this->_tmp['getParents'][$assetid])) {
			// this is so we get the sections as we go
			$children = Asset::getParents($assetid, $type_code, $strict_type_code);
		}

		return $this->_tmp['getParents'][$assetid];

	}//end getParents()
	
	
	/**
	* get the section links attached to this form
	* 
	* @access public
	* @return array(link)
	*/
	function &getQuestions($force=false)
	{
		return $this->attr('questions');

	}//end getQuestions()
	
	
	/**
	* get a specific question
	* 
	* @access public
	* @return array(link)
	*/
	function getQuestionByID($questionid)
	{
		$questions = $this->getQuestions();
		return (isset($questions[$questionid])) ? $questions[$questionid] : Array();

	}//end getQuestionById()
	
	
	/**
	* get specific question by order
	* 
	* @access public
	* @return array(link)
	*/
	function &getQuestionByOrder($orderid)
	{
		$sort_order = &$this->attr('sort_order');
		return $GLOBALS['SQ_SYSTEM']->am->getAsset($sort_order[$orderid]);

	}//end getQuestionByOrder()
	
	
	/**
	* attach a question to this form
	* 
	* @param Form_Question &$question the question object to attach
	*
	* @access public
	* @return boolean false if database problem or question already attached, true if sucessful
	*/
	function attachQuestion($type_code)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$questions = &$this->getQuestions();
		if (empty($questions)) {
		  $new_key = 1;
		} else {
		  $new_key = max(array_keys($questions)) + 1;
		}

		$questions[$new_key] = Array('question_type_code' => $type_code,
							'attributes'	     => Array('name' => 'Question '.(count($questions)+1)));

		$this->setAttrValue('questions', $questions);

		$sort_order = $this->attr('sort_order');

		if (empty($sort_order)) {
			$new_index = 0;
		} else {
			$new_index = max(array_keys($sort_order))+1;
		}
		$sort_order[$new_index] = $this->id . ':q' . $new_key;
		$this->setAttrValue('sort_order', $sort_order);

		if (!$this->saveAttributes()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return true;

	}//end attachQuestion()
	
	
	/**
	* delete a question to this form
	* 
	* @param Form_Question &$question the question object to delete
	*
	* @access public
	* @return boolean false if database problem or question already deleted, true if sucessful
	*/
	function deleteQuestion(&$question)
	{
		if ($question->_questionid == 0) return false;

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$questions = &$this->getQuestions();

		unset($questions[$question->_questionid]);

		$this->setAttrValue('questions', $questions);

		$sort_order = $this->attr('sort_order');

		// remove it from the sort order - it does not matter that there are gaps
		$old_index = array_search($question->id, $sort_order);
		unset($sort_order[$old_index]);

		// unset the question
		unset($question);

		$this->setAttrValue('sort_order', $sort_order);

		if (!$this->saveAttributes()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return true;

	}//end deleteQuestion()
	
	
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

	}//end create()

	
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
	
	}//end _updated()


	/**
	* Returns TRUE if we can delete the passed link, or a string with the error msg
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return mixed TRUE or error msg string
	* @access public
	*/
	function isDeletableLink($linkid)
	{/*
		// the only reason why we would object to this link being
		// deleted is if it is the bodycopy linked to us
		$main_bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'main_bodycopy');
		$looping_bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'looping_bodycopy');

		if ($main_bodycopy_link[0]['linkid'] == $linkid && !$GLOBALS['SQ_PURGING_TRASH']) return 'You cant delete the link between the main bodycopy and the custom form';

		if ($looping_bodycopy_link[0]['linkid'] == $linkid && !$GLOBALS['SQ_PURGING_TRASH']) return 'You cant delete the link between the looping bodycopy and the custom form';

		return parent::isDeletableLink($linkid);
	
	*/}// end isDeletableLink()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array();

	}//end _getAllowedLinks()


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

	}//end fileRegeneration()


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
		$questions = &$this->getQuestions();
		return count($questions);
	
	} // getQuestionCount()

	
	/**
	* prints the body of this section
	*
	* @param int $submission the submission id for this form 
	* @param bool $include signifies if the file is included or echoed
	*
	* @access public
	*/
	function printBody($include=true)
	{/*
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

	*/}// end printBody()

	
	/**
	* gets each of this section's question to process the saubmitted values for them
	*
	* @param &object submission_form $submission the submission asset for this form
	*
	* @access public
	* @return boolean
	*/
	function processForm() {
		$questions = &$this->getQuestions();
		for (reset($questions); null !== ($k = key($questions)); next($questions)) {
			$question = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->id.':q'.$k);
			$question->submission_errors =& $this->submission_errors;
			$question->processForm();
		}
		return true;
	
	}//end processForm()


	/**
	* returns the name of this section
	*
	* @param boolean $short_name if TRUE, returns the short name
	*
	* @access private
	* @return string
	*/
	function _getName($short_name=false)
	{
		return $this->attr('name');

	}//end _getName()


	/**
	* returns the question index for a particular question (eg, 1, a, i, I)
	* 
	* @param integer $order the question order
	*
	* @access public
	* @return string
	*/
	function getQuestionIndex($order)
	{/*
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
	*/}//end getQuestionIndex()

	
	/**
	* writes JavaScript code for client side validation
	* 
	* This function assumes the skeleton of a JS validation function has been created around this code
	* with a form variable named 'form' passed (this is done when generating the content file).
	*
	* @access public
	* @return string
	*/
	function generateJSCode()
	{
		$questions = &$this->getQuestions();
		$code = '';						// blank sheet of code to start with

		foreach ($questions as $q_id => $question) {
			$q_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->id.':q'.$q_id);
			$q_code = $q_asset->generateJSCode();
			
			$code .= $q_code . (empty($q_code) ? "" : "\n");
		}

		return $code;

	}//end generateJSCode()

}//end class
?>