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
* $Id: form_section.inc,v 1.26 2004/10/15 03:13:35 lwright Exp $
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
* @version $Revision: 1.26 $
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
	* Used by asset map to return a asset map-style link array
	*
	* @return array
	* @access public
	*/
	function getAssetMapLinks()
	{
		$old_links = $this->getLinks($this->id, SQ_SC_LINK_BACKEND_NAV, 'asset', false, 'major', 'sort_order');
		$links = Array();
		while (!empty($old_links)) {
			array_push($links, array_shift($old_links));
		}

		foreach(array_keys($links) as $i) {

			$link =& $links[$i];

			// mould it all to the asset map's liking
			$link['url'] = '';
			$link['path'] = '';
			$link['num_immediate_kids'] = 0;
			$link['accessible'] = 1;

			$link['assetid'] = $link['minorid'];
			$link['type_code'] = $link['minor_type_code'];

			$link_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link['assetid']);
			$link['short_name'] = $link_asset->short_name;
			$link['name'] = $link_asset->name;
			$link['status'] = $link_asset->status;

			unset($link['minor_type_code']);
			unset($link['majorid']);
			unset($link['minorid']);
			unset($link['value']);
			unset($link['dependant']);
			unset($link['exclusive']);

		}

		return $links;

	}//end getAssetMapLinks()


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

		$entries = $this->getQuestions();

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
	* get the question assets attached to this section
	*
	* @access public
	* @return array
	*/
	function getQuestionAssets() {
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$result = Array();

		$questions = $this->attr('questions');
		if (!empty($questions)) {
			foreach ($questions as $q_id => $question) {
				$id           = $this->id.':q'.$q_id;
				$result[$id]  = $am->getAsset($id);
			}
		}

		return $result;

	}//end getQuestionAssets()


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
	* @param object Form_Question	&$question	the question object to attach
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
	* @param array	&$link	information used to create the initial link
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
	* Set the last updated info for this asset
	*
	* Call this function when the asset has been changed in some way so we can indicated in the
	* asset table when the asset was last changed and who by. This function will also increment the micro
	* version number for the asset.
	*
	* @param boolean	$update_parents	should we go through an update all our dependant parents as well ?
	*									NOTE: this is passed to incrementVersion to let it know that the parents
	*									should have their versions updated as well.
	*
	* @return boolean
	* @access private
	*/
	function _updated($update_parents=true)
	{
		if (!parent::_updated($update_parents)) return false;
		if (SQ_IN_BACKEND) {
			return $this->fileRegeneration();
		}
		return true;

	}//end _updated()


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
			// then, get all the links of this guy's form parents
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
		}//end if

		return true;

	}//end fileRegeneration()


	/**
	* Clones certain specified components of the asset
	*
	* @param object asset &$clone the clone asset shell
	* @param array			$components	the wanted components to clone eg.
	*                       Array(
	*                           'attributes',
	*                           'metadata_shemas',
	*                           'metadata',
	*                           'workflow',
	*                           'permissions',
	*                           'data'
	*                       );
	*                       or alternately
	*                       Array('all');
	* 
	* @return boolean
	* @access public
	* @see asset::cloneComponents()
	*/
	function cloneComponents(&$clone, $components)
	{
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		
		if (!parent::cloneComponents($clone, $components)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;			
		}
		
		if (in_array('attributes', $components) || in_array('all', $components)) {
			$sort_order = $this->attr('sort_order');
			$sort_order = str_replace($this->id.':q', $clone->id.':q', $sort_order);
						
			// save the information
			$clone->setAttrValue('sort_order', $sort_order);
			$clone->saveAttributes();
			
		}
		
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');		
		return true;
	
	}//end cloneComponents()
	
	
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

	}//end getQuestionCount()


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