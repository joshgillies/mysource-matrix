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
* $Id: form.inc,v 1.148 2007/01/15 01:09:03 lwright Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_FUDGE_PATH.'/general/datetime.inc';
require_once SQ_FUDGE_PATH.'/general/general.inc';

/**
* Form
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Revision: 1.148 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
register_implementation('form', 'bridge');
class Form extends Asset
{


	/**
	* the current answers to the form questions
	* @var object Form_Submission
	*/
	var $current_answers = Array();


	/**
	* the active section object
	* @var object
	*/
	var $active_section = NULL;


	/**
	* the current submission's errors
	* @var object
	*/
	var $submission_errors = NULL;


	/**
	* the current submission's asset
	* @var object
	*/
	var $submission_asset = NULL;


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	* @return void
	* @access public
	*/
	function Form($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		$this->Asset($assetid);

	}//end constructor


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
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
	* Allow any required processesing to occur when any link is updated for the asset
	*
	* Bug Fix #1671
	* When a new link to the section is created or deleted, update the content file
	*
	* @return void
	* @access protected
	*/
	function linksUpdated()
	{
		$this->fileRegeneration();

	}//end linksUpdated()


//--        BRIDGE FUNCTIONS        --//


	/**
	* Returns a reference to the asset represented by the passed assetid (or NULL on error)
	*
	* @param int		$shadowid		the shadow part asset id to be loaded (i.e part after the ':')
	* @param string		$type_code		if this exists then this object is used to
	*									load the asset, if not then the DB is queried
	*									to find out the asset type
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed because
	*									you can't use the '@' operator when returning by reference
	*
	* @return object
	* @access public
	* @see Asset_Manager::&getAsset()
	*/
	function &getAsset($shadowid, $type_code='', $mute_errors=FALSE)
	{
		$asset = NULL;
		$id_parts = explode(':', $shadowid);

		if (isset($id_parts[1])) {
			$shadowid = $id_parts[1];
		} else {
			return $asset;
		}

		// Questions follow the format '<assetid>:q<shadowid>'
		if ($shadowid{0} == 'q') {

			// pick up the question in question (!)
			$questions = $this->attr('questions');
			$questionid = substr($shadowid, 1, strlen($shadowid));
			if (empty($questions[$questionid])) return $asset;

			$q_type_code = $questions[$questionid]['question_type_code'];

			// not fussed about type code?
			if (empty($type_code)) $type_code = $q_type_code;

			// only give the asset back if of the right type code
			if (in_array($q_type_code, $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($type_code, TRUE))) {
				$questions[$questionid]['questionid'] = $questionid;
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($q_type_code);

				// this eval returns the necessary object for the question type
				$asset =& new $q_type_code($this->id, $questions[$questionid]);
			}
		}

		return $asset;

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
	* @param boolean		$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	* @param string			$side_of_link		Which side of the link this (the current) asset
	*											is on ('major' or 'minor')
	* @param string			$sort_by			a field in the asset table you would like the results
	*											sorted by (eg. name, short_name etc)
	*
	* @return array
	* @access public
	*/
	function getLinks($assetid, $link_types, $type_code='', $strict_type_code=TRUE, $side_of_link='major', $sort_by=NULL)
	{
		$id_parts = explode(':', $assetid);
		if (isset($id_parts[1])) $assetid = $id_parts[1];

		$links = Array();
		$new_sort_order = 0;

		// shadow assets have no children, but they have one minor link the other way
		if (!is_numeric($assetid)) {
			if ($side_of_link == 'minor') {
				$return_link = FALSE;

				if ($type_code == '') {
					// not fussed what we're getting, so return me
					$return_link = TRUE;
				} else if ($strict_type_code) {
					// strict type code check
					$return_link = ($type_code == $this->type());
				} else {
					$return_link = (in_array($this->type(), $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($type_code, TRUE)));
				}

				if ($return_link) {
					$links[] = Array(
								'linkid'			=> 0,
								'majorid'			=> $this->id,
								'minorid'			=> $this->id.':'.$assetid,
								'major_type_code'	=> $this->type(),
								'value'				=> '',
								'link_type'			=> SQ_LINK_TYPE_2,
								'is_dependant'		=> TRUE,
								'is_exclusive'		=> FALSE,
								'sort_order'		=> 0,
							   );
				}
			}

			return $links;
		}

		// are we getting our bodycopy folder and/or sections (if they all exist)?
		$real_type_codes = Array(
							'folder'		=> SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3,
							'form_section'	=> SQ_LINK_TYPE_2,
						   );

		foreach ($real_type_codes as $real_type_code => $allowed_link_types) {
			if ($type_code == '') {
				// not fussed what we're getting, so return me
				$get_type = TRUE;
			} else if ($strict_type_code) {
				$get_type = ($type_code == $real_type_code);
			} else {
				// specific type code set, non-strict check
				$get_type = (in_array($real_type_code, $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($type_code, TRUE)));
			}

			// we are getting this type?
			if ($get_type) {
				$query = $GLOBALS['SQ_SYSTEM']->am->generateGetLinksQuery($assetid, $link_types & $allowed_link_types, $real_type_code, FALSE);
				if (empty($query)) return Array();

				$type_links = $GLOBALS['SQ_SYSTEM']->db->getAll(implode(' ', $query));
				assert_valid_db_result($type_links);

				foreach (array_keys($type_links) as $key) {
					$type_links[$key]['sort_order'] = $new_sort_order;
					$links[] = $type_links[$key];
					$new_sort_order++;
				}
			}
		}

		// but we also need question shadow asset links
		if ($link_types & SQ_LINK_TYPE_2) {
			$questions = $this->attr('questions');

			// keep a cache of asset type codes if we are checking on a non-strict
			// type code, to stop us calling getTypeDescendants() all the time
			// (not needed if strict type check or if not fussed)
			$get_question_types = Array();

			foreach ($questions as $questionid => $data) {
				if ($type_code == '') {
					// not fussed what we're getting, so return me
					$get_question = TRUE;
				} else if ($strict_type_code) {
					// strict type code check
					$get_question = ($type_code == $data['question_type_code']);
				} else {
					// if we've already cached whether this question type is to be
					// returned, then we don't need to look it up again
					if (isset($get_question_types[$data['question_type_code']])) {
						$get_question = $get_question_types[$data['question_type_code']];
						$get_question_types[$data['question_type_code']] = $get_question;
					} else {
						$get_question = (in_array($data['question_type_code'], $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($type_code, TRUE)));
					}
				}

				if ($get_question) {
					$links[] = Array(
								'linkid'			=> 0,
								'majorid'			=> $this->id,
								'minorid'			=> $this->id.':q'.$questionid,
								'minor_type_code'	=> $data['question_type_code'],
								'value'				=> '',
								'link_type'			=> SQ_LINK_TYPE_2,
								'is_dependant'		=> TRUE,
								'is_exclusive'		=> FALSE,
								'sort_order'		=> $new_sort_order,
							   );
				}
				$new_sort_order++;
			}//end foreach
		}//end if

		if (is_null($sort_by)) $sort_by = 'sort_order';

		uasort($links, create_function('$a,$b','return $a["'.$sort_by.'"] > $b["'.$sort_by.'"];'));

		return $links;

	}//end getLinks()


	/**
	* Used by asset map to return a asset map-style link array
	*
	* @return array
	* @access public
	*/
	function getAssetMapLinks()
	{
		$new_sort_order = 0;
		$old_links = $this->getLinks($this->id, SQ_SC_LINK_BACKEND_NAV);
		$links = Array();
		while (!empty($old_links)) {
			array_push($links, array_shift($old_links));
		}

		$questions = $this->attr('questions');

		foreach (array_keys($links) as $i) {

			$link =& $links[$i];

			// remove real links
			if ($link['minor_type_code'] == 'folder') {
				unset($links[$i]);
				continue;
			}

			if ($link['minor_type_code'] == 'form_section') {
				unset($links[$i]);
				continue;
			}

			// mould it all to the asset map's liking
			$link['url'] = '';
			$link['path'] = '';
			$link['num_kids'] = 0;
			$link['accessible'] = 1;

			$link['assetid'] = $link['minorid'];
			$link['type_code'] = $link['minor_type_code'];
			$link['linkid'] = $link['majorid'].':'.$link['minorid'];

			$questionid = str_replace($this->id.':q', '', $link['assetid']);

			// make name and short name the same
			$link['name'] = $questions[$questionid]['attributes']['name'];
			$link['short_name'] = $link['name'];

			// make the status the same as the form's one
			$link['status'] = $this->status;

			unset($link['minor_type_code']);
			unset($link['majorid']);
			unset($link['minorid']);
			unset($link['value']);
			unset($link['is_dependant']);
			unset($link['is_exclusive']);

		}//end foreach

		return $links;

	}//end getAssetMapLinks()


	/**
	* Get all asset ids that are above the passed assetid in the various trees in which it exists
	*
	* @param int			$shadowid			the shadow ID of the shadow asset to get (ie. the
	*											part after the ':')
	* @param string|array	$type_code			the type of asset that is linked (eg 'User', 'User_Group', etc)
	*											if an array returns link if matches any of the array values
	* @param boolean		$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	*
	* @return array
	* @access public
	* @see Asset_Manager::getParents()
	*/
	function getParents($shadowid, $type_code='', $strict_type_code=TRUE)
	{
		$id_parts = explode(':', $shadowid);
		if (isset($id_parts[1])) $shadowid = $id_parts[1];

		// basically get the parents of the section, and add itself in the
		$query = $GLOBALS['SQ_SYSTEM']->am->generateGetParentsQuery($this->id, $type_code, $strict_type_code);

		if (empty($query)) return Array();

		$queried_parents = $GLOBALS['SQ_SYSTEM']->db->getAll(implode(' ', $query));
		assert_valid_db_result($queried_parents);
		$parents = Array();

		foreach ($queried_parents as $queried_parent) {
			$parents[$queried_parent['majorid']] = $queried_parent['type_code'];
		}

		$parents[$this->id] = $this->type();

		return $parents;

	}//end getParents()


	/**
	* Get all asset ids that are below the passed assetid in the various trees in which it exists
	*
	* Return structure: Array(int => string) - asset id => type_code
	*
	* @param int			$assetid			the id of the asset to get its children for
	* @param string|array	$type_code			the type of asset that is linked
	*											(eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param boolean		$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	* @param boolean		$dependant			if TRUE, results will be filtered to assets that are dependants.
	*											If FALSE, results will all not be dependants.
	*											If NULL, results will not be filtered
	* @param string			$sort_by			a field in the asset table you would like the results
	*											sorted by (eg. name, short_name etc)
	*
	* @return array
	* @access public
	* @see Asset_Manager::getChildren()
	*/
	function getChildren($assetid, $type_code='', $strict_type_code=TRUE, $dependant=NULL, $sort_by=NULL)
	{
		// no shadow assets have children in this asset
		if (!is_numeric($assetid)) return Array();

		if (!is_array($type_code)) {
			if (empty($type_code)) {
				$type_code = Array();
			} else {
				$type_code = Array($type_code);
			}
		}

		$children = Array();

		// this is so we get the sections as we go
		// have to do this all in here because calling asset manager will bring on a vicious loop
		$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_a($asset, 'bridge') && ($assetid != $this->id)) {
			$children = $asset->getChildren($assetid, $type_code, $dependant, $strict_type_code, $sort_by);
			$this->forgetAsset($asset);
		} else {
			$query = $GLOBALS['SQ_SYSTEM']->am->generateGetChildrenQuery($asset, $type_code, $strict_type_code, $dependant, $sort_by);
			unset($asset);

			if (!empty($query)) {
				$result = $GLOBALS['SQ_SYSTEM']->db->getAssoc(implode(' ', $query));
				assert_valid_db_result($result);

				$children =& $result;
			}
		}

		// now questions
		$entries = $this->attr('questions');
		if (empty($type_code)) {
			foreach ($entries as $questionid => $data) {
				$children[$this->id.':q'.$questionid] = $data['question_type_code'];
			}
		} else {
			foreach ($type_code as $this_type_code) {
				foreach ($entries as $questionid => $data) {
					if ($strict_type_code) {
						if ($data['question_type_code'] == $this_type_code) {
							$children[$this->id.':q'.$questionid] = $data['question_type_code'];
						}
					} else {
						$type_desc = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($this_type_code) + Array($this_type_code);
						if (in_array($data['question_type_code'], $type_desc)) {
							$children[$this->id.':q'.$questionid] = $data['question_type_code'];
						}
					}
				}
			}
		}

		if (!is_null($sort_by)) {
			uasort($children, create_function('$a,$b','return $a["'.$sort_by.'"] > $b["'.$sort_by.'"]'));
		}

		return $children;

	}//end getChildren()


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
	function _updated($update_parents=TRUE)
	{
		if (!parent::_updated($update_parents)) return FALSE;
		if (SQ_IN_BACKEND) return $this->fileRegeneration();
		return TRUE;

	}//end _updated()


	/**
	* Returns TRUE if the passed minor asset can be linked to this asset, or a string with the error msg
	*
	* @param object	&$minor		the minor asset that we are linking to
	* @param string	$link_type	the type of link this is
	* @param int	$exclusive	the exclusive status of the link (gets passed to canLinkToType)
	*
	* @return mixed boolean|string
	* @access public
	*/
	function canCreateLink(&$minor, $link_type, $exclusive)
	{
		if (is_a($minor, 'form_submission') || is_a($minor, 'form_section')) {
			return TRUE;
		}

		return parent::canCreateLink($minor, $link_type, $exclusive);

	}//end canCreateLink()


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
		if ($side_of_link == 'major' && $asset->type() == 'form_section') {
			$link_type = SQ_LINK_TYPE_2;
			$dependant = TRUE;
		}

	}//end prepareLink()


	/**
	* regenerates the content file for this form
	*
	* @return boolean
	* @access public
	*/
	function fileRegeneration()
	{
		$edit = $this->getEditFns();
		return $edit->generateStandardContentFile($this);

	}//end fileRegeneration()


//--       QUESTION MANAGEMENT       --//


	/**
	* get the question links attached to this form
	*
	* @return array
	* @access public
	*/
	function &getQuestions()
	{
		$questions = $this->attr('questions');
		return $questions;

	}//end getQuestions()


	/**
	* get the assetids of ALL of this form's questions, including those
	* inside sections
	*
	* Returns an array of (full assetid => question detail)
	*
	* @return array
	* @access public
	*/
	function &getAllQuestions()
	{
		// getQuestions() returns an array of (q_id => detail) - this is
		// ambiguous to return if sections are present, so we need to convert
		// to an asset ID

		$all_questions = Array();
		$questions = $this->getQuestions();
		foreach ($questions as $q_id => $question_detail) {
			$all_questions[$this->id.':q'.$q_id] = $question_detail;
		}

		$sections =& $this->getSections();
		foreach ($sections as $section) {
			$questions = $section->getQuestions();
			foreach ($questions as $q_id => $question_detail) {
				$all_questions[$section->id.':q'.$q_id] = $question_detail;
			}
		}

		return $all_questions;

	}//end getAllQuestions()


	/**
	* get the question assets attached to this form
	*
	* @return array
	* @access public
	*/
	function &getQuestionAssets()
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$result = Array();

		$questions = $this->attr('questions');
		if (!empty($questions)) {
			foreach ($questions as $q_id => $question) {
				$id           = $this->id.':q'.$q_id;
				$result[$id]  =& $am->getAsset($id);
			}
		}

		return $result;

	}//end getQuestionAssets()


	/**
	* get the question assets attached to this form, even inside sections
	*
	* @return array
	* @access public
	*/
	function &getAllQuestionAssets()
	{
		$all_questions =& $this->getQuestionAssets();

		$sections =& $this->getSections();
		foreach ($sections as $section) {
			$questions =& $section->getQuestionAssets();
			$all_questions = array_merge($all_questions, $questions);
		}

		return $all_questions;

	}//end getAllQuestionAssets()


//--        ADDING AND DELETING QUESTIONS        --//


	/**
	* attach a question to this form
	*
	* Returns boolean FALSE if a database problem occurs or if question already
	* attached, or TRUE if sucessful
	*
	* @param string	$type_code	the type code of the question we are adding
	*
	* @return boolean
	* @access public
	*/
	function attachQuestion($type_code)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$questions = $this->getQuestions();
		if ($this->attr('next_questionid') > 0) {
			$new_key = $this->attr('next_questionid');
		} else if (empty($questions)) {
			$new_key = 1;
		} else {
			$new_key = max(array_keys($questions)) + 1;
		}

		$questions[$new_key] = Array(
								'question_type_code'	=> $type_code,
								'attributes'			=> Array('name' => 'Question '.(count($questions)+1)),
							   );

		$this->setAttrValue('questions', $questions);
		$this->setAttrValue('next_questionid', $new_key + 1);

		$sort_order = $this->attr('sort_order');

		if (empty($sort_order)) {
			$new_index = 0;
		} else {
			$new_index = max(array_keys($sort_order))+1;
		}
		$sort_order[$new_index] = $this->id.':q'.$new_key;
		$this->setAttrValue('sort_order', $sort_order);

		if (!$this->saveAttributes()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// acquire the lock on the new question
		$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id.':q'.$new_key, 'all', $this->id, TRUE);
		$GLOBALS['SQ_SYSTEM']->am->updateLock($this->id, 'all');

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end attachQuestion()


	/**
	* delete a question to this form
	*
	* Returns boolean FALSE if a database problem occurs or if question already
	* attached, or TRUE if sucessful
	*
	* @param object	&$question	the Form_Question object to delete
	*
	* @return boolean
	* @access public
	*/
	function deleteQuestion(&$question)
	{
		if ($question->_questionid == 0) return FALSE;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$questions = $this->getQuestions();

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
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end deleteQuestion()


	/**
	* returns the number of sections in this form
	*
	* @return int
	* @access public
	*/
	function getQuestionCount()
	{
		$questions = $this->getQuestions();
		return count($questions);

	}//end getQuestionCount()


	/**
	* get a specific question, and return the question's link
	*
	* @param string	$questionid	The question's (shadow?) ID
	*
	* @return array
	* @access public
	*/
	function getQuestionByID($questionid)
	{
		$questions = $this->getQuestions();
		return (isset($questions[$questionid])) ? $questions[$questionid] : Array();

	}//end getQuestionByID()


	/**
	* get specific question by order, and return the question's link
	*
	* @param int	$orderid	The question's sort order
	*
	* @return array
	* @access public
	*/
	function &getQuestionByOrder($orderid)
	{
		$sort_order = $this->attr('sort_order');
		return $GLOBALS['SQ_SYSTEM']->am->getAsset($sort_order[$orderid]);

	}//end getQuestionByOrder()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		$page_links = Array(
						SQ_LINK_TYPE_2	=> Array(
											'form_section'	=> Array(
																'card'		=> 'M',
																'exclusive'	=> FALSE,
															   ),
											'folder'		=> Array(
																'card'		=> '1',
																'exclusive'	=> TRUE,
															   ),
										   ),
						SQ_LINK_TYPE_3	=> Array(
											'form_submission'	=> Array(
																	'card'		=> 'M',
																	'exclusive'	=> FALSE,
																   ),
										   ),
					  );
		return $page_links;

	}//end _getAllowedLinks()


//--       SECTION MANAGEMENT       --//


	/**
	* returns all of the sections in this form in order of 'sort_order'
	*
	* Structure of return array is Array(object Form_Section).
	*
	* @return array
	* @access public
	*/
	function &getSections()
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$section_links =& $this->getSectionLinks();
		$sections = Array();

		foreach ($section_links as $link) {
			$sections[$link['sort_order']] =& $am->getAsset($link['minorid'], $link['minor_type_code']);
		}
		return $sections;

	}//end getSections()


	/**
	* returns the number of sections in this form
	*
	* @return int
	* @access public
	*/
	function getSectionCount()
	{
		return $GLOBALS['SQ_SYSTEM']->am->countLinks($this->id, 'major', SQ_LINK_TYPE_2, 'form_section', TRUE);

	}//end getSectionCount()


	/**
	* get the section links attached to this form
	*
	* @return array
	* @access public
	*/
	function &getSectionLinks()
	{
		// This is what one would find in Asset_Manager::&getAsset() except we can't call that,
		// lest we spin off into an infinite loop involving OUR &getAsset().

		$query = $GLOBALS['SQ_SYSTEM']->am->generateGetLinksQuery($this->id, SQ_LINK_TYPE_2, 'form_section', TRUE, 'major', '', '1', '0');
		if (empty($query)) return Array();
		$query['select'] .= ', l.majorid';		// we need the major id too
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$result = $db->getAll(implode(' ', $query));
		assert_valid_db_result($result);
		return $result;

	}//end getSectionLinks()


	/**
	* Clones certain specified components of the asset
	*
	* @param object	&$clone		the clone asset shell
	* @param array	$components	the wanted components to clone eg.
	*							Array(
	*							'attributes',
	*							'metadata_schemas',
	*							'metadata',
	*							'workflow',
	*							'permissions',
	*							'data',
	*							);
	*							or alternately
	*							Array('all');
	*
	* @return boolean
	* @access public
	* @see asset::cloneComponents()
	*/
	function cloneComponents(&$clone, $components)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!parent::cloneComponents($clone, $components)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		if (in_array('attributes', $components) || in_array('all', $components)) {
			$sort_order = $this->attr('sort_order');
			$sort_order = str_replace($this->id.':q', $clone->id.':q', $sort_order);

			// save the information
			$clone->setAttrValue('sort_order', $sort_order);
			$clone->saveAttributes();

		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end cloneComponents()


//--       BODYCOPIES       --//


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		$prefix = $this->getPrefix();

		$clientside = ($this->attr('use_client_side')) ? 'onsubmit="return beforeSubmit_'.$prefix.'(this);"' : '';

		$form_action = $_SERVER['PHP_SELF'];

		// Add the query string as well if it exists, so asset references with ?a=###
		// submit back to themselves.
		if (!empty($_SERVER['QUERY_STRING'])) {
			$form_action .= '?'.$_SERVER['QUERY_STRING'];
		}

		ob_start();
		echo '<form id="'.$prefix.'" enctype="multipart/form-data" action="'.$form_action.'" method="post" '.$clientside.'>';
		if ($this->attr('use_client_side')) {
			?><script type="text/javascript">
			function beforeSubmit_<?php echo $this->getPrefix() ?>(form) {
				var submission_errors = new Array();
				i = 0;
					<?php
					echo $this->generateJSCode();
				?>
					if (submission_errors.length > 0) {
					var errors_list = "<?php echo translate('cms_form_submission_errors'); ?>\n";
					for(x in submission_errors) {
						errors_list += submission_errors[x] + "\n";
					}
					alert(errors_list);
					return false;
				}
				return true;
			}
			</script>
		<?php
		}

		$this->printStandard();
		echo '</form>';

		$result = ob_get_contents();
		ob_end_clean();

		echo $result;

	}//end printBody()


	/**
	* Prints the form to screen
	*
	* @param array	$replacements	a list of additional replacements that
	*								subclasses can use to insert additional
	*								keywords
	*
	* @return void
	* @access public
	*/
	function printForm($replacements=Array())
	{
		ob_start();
			$this->includeContentFile('Standard');
			$cf_output = ob_get_contents();
		ob_end_clean();

		$error = !empty($this->submission_errors);

		if ($error) {
			ob_start();
				$this->printErrors();
				$err_output = ob_get_contents();
			ob_end_clean();
			$err_msg = $this->attr('submission_errors_msg');
		} else {
			$err_output = '';
			$err_msg = '';
		}

		$bodycopy =& $this->getBodycopy('Page Contents');

		// if no title for the submit button is specified in the backend
		// just use 'Submit'
		$button_name = ($this->attr('submit_button')) ? $this->attr('submit_button') : translate('submit');

		ob_start();
			submit_button($this->getPrefix().'_submit', $button_name);
			$submit_button = ob_get_contents();
		ob_end_clean();

		$reset_button_name = ($this->attr('reset_button')) ? $this->attr('reset_button') : translate('clear_form');

		ob_start();
			reset_button($this->getPrefix().'_reset', $reset_button_name);
			$reset_button = ob_get_contents();
		ob_end_clean();

		// check to see if the bodycopy attached to this form has any content
		if ($this->attr('use_bodycopy_main') && $bodycopy && !$bodycopy->isEmpty()) {
			$questions = $this->getQuestions();
			$sections =& $this->getSections();

			foreach ($questions as $q_id => $question) {
				$q_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($this->id.':q'.$q_id);
				$q_name = $q_asset->attr('name');
				$replacements['question_field_'.$this->id.'_q'.$q_id] = $q_asset->getHtmlField();
				$replacements['question_id_'.$question->id.'_q'.$q_id] = 'q'.$this->id.'_q'.$q_id;
				$replacements['question_note_'.$this->id.'_q'.$q_id] = $q_asset->attr('note');
				$replacements['question_label_'.$this->id.'_q'.$q_id] = $q_asset->getHtmlLabel();
			}

			foreach ($sections as $section) {
				$replacements['section_title_'.$section->id] = $section->attr('name');
				$questions = $section->getQuestions();
				foreach ($questions as $q_id => $question) {
					$q_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($section->id.':q'.$q_id);
					$q_name = $section->attr('name').': '.$q_asset->attr('name');
					$replacements['question_field_'.$section->id.'_q'.$q_id] = $q_asset->getHtmlField();
					$replacements['question_id_'.$section->id.'_q'.$q_id] = 'q'.$section->id.'_q'.$q_id;
					$replacements['question_note_'.$section->id.'_q'.$q_id] = $q_asset->attr('note');
					$replacements['question_label_'.$section->id.'_q'.$q_id] = $q_asset->getHtmlLabel();
				}
			}

			$replacements['form_contents'] = &$cf_output;
			$replacements['form_errors']   = &$err_output;
			$replacements['form_errors_message'] = &$err_msg;
			$replacements['submit_button'] = &$submit_button;
			$replacements['reset_button']  = &$reset_button;

			// add global keywords
			$keywords = $bodycopy->getKeywords();
			foreach ($keywords as $word) {
				if (!isset($replacements[$word])) {
					$replacements[$word] = $this->getKeywordReplacement($word);
				}
			}

			ob_start();
				$bodycopy->setKeywordReplacements($replacements);
				$bodycopy->printBody();
				$output = ob_get_contents();
			ob_end_clean();

			echo $output;

		} else {
			$output = $cf_output;

			$output .= '<table width="'.$this->attr('form_width').'"><tr>';
			$output .= '<td';
			$output .= ($this->attr('question_col_width')) ? ' 	width="'.$this->attr('question_col_width').'">' : '>';
			$output .='&nbsp;</td><td';
			$output .= ($this->attr('answer_col_width')) ? ' 	width="'.$this->attr('answer_col_width').'">' 	: '>';

			$output .= $submit_button;
			$output .= $reset_button;

			$output .= '</td></tr>';
			if ($error) {
				$output .= '<tr><td colspan="3">'.$err_output.'</td></tr>';
			}
			$output .= '</table>';

			echo $output;
		}

	}//end printForm()


	/**
	* Prints the standard form i.e not a wizard
	*
	* @return boolean
	* @access public
	*/
	function printStandard()
	{
		$prefix = $this->getPrefix();

		// Make sure that the thank you bodycopy is always able to access the
		// newly created submission in replicated database instances, by tying
		// it all to db2
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		// check to see if the form has been submitted or not
		if (isset($_POST[$prefix.'_submit']) && $this->processForm()) {
			$this->finishForm();
			return TRUE;
		}
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		// nothing submitted yet, or there was an error
		$this->printForm();
		return TRUE;

	}//end printStandard()


	/**
	* create a bodycopy and attach to the Bodycopies folder of the form
	*
	* The $name attribute will also be used for the Link Value of the link between
	* the form's Bodycopies folder and the new bodycopy. The name will be lowercased,
	* spaces replaced with underscores, and assigned to the link value. So if 'Thank
	* You' is the name of the bodycopy (for instance), it will be linked to the
	* Bodycopies folder with the link value 'thank_you'.
	* Returns boolean FALSE if the bodycopy already exists or if the creation
	* failed for some reason, or TRUE if it was successfully created
	*
	* @param string	$name	the name of the bodycopy to create
	*
	* @return boolean
	* @access public
	*/
	function createBodycopy($name)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// search for a bodycopies folder and create one if there isn't one there
		$folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, 'bodycopies_folder');
		if (empty($folder_link)) {

			// create a folder for the bodycopies
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');
			$folder =& new Folder();
			$folder->setAttrValue('name', 'Bodycopies');

			$folder_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'value' => 'bodycopies_folder', 'is_dependant' => 1, 'is_exclusive' => 1);

			if (!$folder->create($folder_link)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		} else {
			// folder already exists, get the reference
			$folder =& $GLOBALS['SQ_SYSTEM']->am->getAsset($folder_link['minorid'], $folder_link['minor_type_code']);
		}
		if (is_null($folder)) return FALSE;

		// and see whether this bodycopy already exists
		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, $name);
		if (!empty($bodycopy_link)) return TRUE;

		// bodycopy doesn't exist, so create it willya
		// create a link to the folder so we can put the bodycopies in it
		$copy_link = Array('asset' => &$folder, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1, 'value' => $name);

		// create the thing
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$bodycopy =& new Bodycopy();
		$bodycopy->setAttrValue('name', $name);

		// create an appropriate value for this link
		// to reflect that of the name
		$name = strtolower(str_replace(' ', '_', $name));

		if (!$bodycopy->create($copy_link)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end createBodycopy()


	/**
	* returns a reference to a bodycopy based of the name of the bodycopy
	*
	* @param string	$name	the name of the bodycopy
	*
	* @return object
	* @access public
	*/
	function &getBodycopy($name='')
	{
		$null = NULL;

		if (!$name) return $null;
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		$folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, 'bodycopies_folder');
		if (empty($folder_link)) return $null;

		$folder =& $am->getAsset($folder_link['minorid'], $folder_link['minor_type_code']);
		if (is_null($folder)) return $null;

		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, $name);
		if (empty($bodycopy_link)) return $null;

		$bodycopy =& $am->getAsset($bodycopy_link['minorid'], $bodycopy_link['minor_type_code']);

		return $bodycopy;

	}//end getBodycopy()


	/**
	* includes the content file for this form
	*
	* For now 'standard' is the only option available.
	*
	* @param string	$type	the type of content file to generate
	*
	* @return boolean
	* @access public
	*/
	function includeContentFile($type='standard')
	{
		if (!file_exists($this->data_path.'/content_file.php')) {
			$edit =& $this->getEditFns();
			$fn = 'generate'.$type.'ContentFile';

			if (!method_exists($edit, $fn)) return FALSE;
			if (!$edit->$fn($this)) return FALSE;
		}
		include($this->data_path.'/content_file.php');

		return TRUE;

	}//end includeContentFile()


//--       FORM PROCESSING       --//


	/**
	* processes the form and returns TRUE or FALSE depending on whether form processed successfully
	*
	* @return boolean
	* @access public
	*/
	function processForm()
	{
		$ok = TRUE;
		$this->submission_errors = Array();

		$questions =& $this->getAllQuestionAssets();
		for (reset($questions); $question =& $questions[key($questions)]; next($questions)) {
			$question->populate();
			if (!$question->hasValidValue()) {
				$this->submission_errors = array_merge($this->submission_errors, $question->failed_rules);
				$ok = FALSE;
			}
			$this->current_answers[$question->id] = $question->getValue();
		}

		if ($ok) {
			// save question values
			for (reset($questions); $question =& $questions[key($questions)]; next($questions)) {
				$question->saveValue();

				// If the question is sticky, stick the value then
				if ($question->attr('sticky')) {
					$cookie_expiry = $this->attr('cookie_expiry');
					if ($cookie_expiry == 0) {
						$question->stickValue($question->getValue());
					} else {
						$question->stickValue($question->getValue(), time() + $this->attr('cookie_expiry'));
					}
				}
			}

			// create the submission asset
			if ($this->attr('log_submissions')) {
				$this->submission_asset =& $this->createSubmission();
				for (reset($questions); $question =& $questions[key($questions)]; next($questions)) {
					$this->submission_asset->setAnswer(key($questions), $question->getValue());
					$this->submission_asset->setSummary(key($questions), $question->attr('name'), $question->getSummary());
				}
				$this->submission_asset->setIP($_SERVER['REMOTE_ADDR']);
				$this->submission_asset->setAttrValue('complete', TRUE);
				$this->submission_asset->setAttrValue('xml', $this->getXML());
				$this->submission_asset->saveAttributes();
			}
		}

		return $ok;

	}//end processForm()


	/**
	* re-populates the active submissions of its members
	*
	* @return void
	* @access public
	*/
	function repopulate()
	{
		if (empty($this->current_answers)) return;

		$questions = $this->getAllQuestionAssets();
		foreach (array_keys($questions) as $key) {
			$question =& $GLOBALS['SQ_SYSTEM']->am->getAsset($key);
			$question->setValue($this->current_answers[$question->id]);
		}

	}//end repopulate()


	/**
	* function to finish off form and print thank you bodycopy
	*
	* @return boolean
	* @access public
	*/
	function finishForm()
	{
		$tyb =& $this->getBodycopy('Thank You');

		ob_start();
		$this->printSummary(TRUE);
		$summary_info = ob_get_contents();
		ob_end_clean();

		// check to see if the bodycopy attached to this form has any content
		if ($this->attr('use_bodycopy_thank_you') && $tyb && !$tyb->isEmpty()) {

			if ($this->attr('show_results')) {

				// add global keywords
				$keywords = $tyb->getKeywords();
				foreach ($keywords as $word) {

					// If form_info or form_summary, we need to make sure the
					// HTML version gets printed
					if (($word === 'form_info') || ($word == 'form_summary')) {
						$replacements[$word] = $this->_getThankYouKeywordReplacement($word.'_html');
					} else {
						$replacements[$word] = $this->_getThankYouKeywordReplacement($word);
					}

					if (!isset($replacements[$word])) {
						$replacements[$word] = $this->getKeywordReplacement($word);
					}
				}

				$tyb->setKeywordReplacements($replacements);
			}//end if

			$tyb->printBody();

		} else {
			// no bodycopy
			// print the results if we need to
			if ($this->attr('show_results')) echo $summary_info;

		}

	}//end finishForm()


	/**
	* prints the errors that occured in a form submission
	*
	* @return boolean
	* @access public
	*/
	function printErrors()
	{
		if (!empty($this->submission_errors)) {
			?><ul><?php
			foreach ($this->submission_errors as $error) {
				?><li><?php echo $error ?></li><?php
			}
			?></ul><?php
		}
		return TRUE;

	}//end printErrors()


	/**
	* returns an answer summary in XML format (suitable for submission logs)
	*
	* @return boolean
	* @access public
	*/
	function getXML()
	{
		ob_start();
		echo '<submission time="'.str_replace(' ','T',ts_iso8601(time())).'"';
		echo ' ip="'.$this->submission_asset->getIP().'"';
		if (!is_null($cur_user =& $GLOBALS['SQ_SYSTEM']->user)) {
			echo ' user="'.$cur_user->id.'"';
		}
		echo '>';

		// unattached questions first
		$sort_order = $this->attr('sort_order');

		foreach ($sort_order as $sort_order_index => $assetid) {

			// check to see whether this belongs to us (if it isn't then perhaps a section took care)
			if ($this->id == strtok($assetid, ':')) {
				$question =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				echo $question->getXML($question->getValue());
			}

		}

		// now sections
		$sections =& $this->getSections();
		for (reset($sections); NULL !== ($k = key($sections)); next($sections)) {
			$section =& $sections[$k];
			echo '<section id="'.$section->id.'" name="'.$section->attr('name').'">';

			$sort_order = $section->attr('sort_order');

			foreach ($sort_order as $sort_order_index => $assetid) {

				// check to see whether this belongs to the section
				if ($section->id == strtok($assetid, ':')) {
					$question =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
					echo $question->getXML($question->getValue($assetid));
				}

			}

			echo '</section>';

		}

		echo '</submission>';

		$contents = ob_get_contents();
		ob_end_clean();

		return $contents;

	}//end getXML()


	/**
	* returns a summary about the answers in the form submission
	*
	* @param boolean	$html	return html formatted or not
	*
	* @return boolean
	* @access public
	*/
	function printSummary($html=FALSE)
	{
		echo ($html) ? '<table>' : '';

		// unattached questions first
		$sort_order = $this->attr('sort_order');

		foreach ($sort_order as $sort_order_index => $assetid) {

			// check to see whether this belongs to us (if it isn't then perhaps a section took care)
			if ($this->id == strtok($assetid, ':')) {
				$question =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				echo ($html) ? '<tr><td><ul><li>' : '';
				echo $question->attr('name');
				echo ($html) ? ': </li></ul></td><td>' : ' : ';
				echo $question->getSummary();
				echo ($html) ? '</td></tr>' : "\n";
			}

		}

		// now sections
		$sections =& $this->getSections();
		for (reset($sections); NULL !== ($k = key($sections)); next($sections)) {
			$section =& $sections[$k];
			$section->current_answers =& $this->current_answers;
			echo ($html) ? '<tr><td colspan="2">' : "\n";
			echo '<strong>'.$section->attr('name').'</strong>';
			echo ($html) ? '</td></tr>' : "\n\n";

			$sort_order = $section->attr('sort_order');

			foreach ($sort_order as $sort_order_index => $assetid) {

				// check to see whether this belongs to the section
				if ($section->id == strtok($assetid, ':')) {
					$question =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
					echo ($html) ? '<tr><td><ul><li>' : '';
					echo $question->attr('name');
					echo ($html) ? ': </li></ul></td><td>' : ' : ';
					echo $question->getSummary();
					echo ($html) ? '</td></tr>' : "\n";
				}

			}
			echo ($html) ? '' : "\n";
		}

		echo ($html) ? '</table>' : '';

	}//end printSummary()


	/**
	* returns information about where the form exists
	*
	* @return array
	* @access public
	*/
	function getFormInfo()
	{
		$info = Array();
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		$lineage = $am->getLineageFromURL();
		foreach ($lineage as $asset) {
			$info[$asset['type_code'].'_name'] = $asset['name'];
			$info[$asset['type_code'].'_id']   = $asset['assetid'];
		}
		$info['url'] = current_url(TRUE, TRUE);

		if (!is_null($this->submission_asset)) {
			$info['submission_id'] = $this->submission_asset->id;
		}

		return $info;

	}//end getFormInfo()


	/**
	* writes JavaScript code for client side validation
	*
	* This function assumes the skeleton of a JS validation function has been created around this code
	* with a form variable named 'form' passed (this is done when generating the content file).
	*
	* @return string
	* @access public
	*/
	function generateJSCode()
	{
		if (!$this->attr('use_client_side')) return '';

		$keywords = Array();
		$use_bc = ($this->attr('use_bodycopy_main') == 1) ? TRUE : FALSE;

		if ($use_bc) {
			// Bodycopy is being used, so lets get its keywords
			$bc = $this->getBodycopy('Page Contents');
			$keywords = $bc->getKeywords();

			// Form contents is printed, which is just like not using a bodycopy(in this context)
			// So we will print all questions' js code.
			if (in_array('form_contents', $keywords)) {
				$use_bc = FALSE;
			}
		}

		ob_start();
		?>
			submission_errors = new Array();
		<?php
		$code = ob_get_contents();
		ob_end_clean();

		$questions = $this->getAllQuestionAssets();

		foreach (array_keys($questions) as $key) {
			$question =& $GLOBALS['SQ_SYSTEM']->am->getAsset($key);
			$q_code = '';
			if ($use_bc) {
				$keyword_search = 'question_field_'.str_replace(':', '_', $question->id);
				if (in_array($keyword_search, $keywords)) {
					// keyword is present in the bodycopy, we can safely print the js code.
					$q_code = $question->generateJSCode();
				}
			} else {
				$q_code = $question->generateJSCode();
			}
			$code .= $q_code.(empty($q_code) ? '' : "\n");
		}

		return $code;

	}//end generateJSCode()


	/**
	* creates a new submission object
	*
	* @return object
	* @access public
	*/
	function &createSubmission()
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('form_submission');

		// create the form submission asset
		$submission =& new Form_Submission();
		$copy_link  = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_3, 'is_dependant' => 1, 'is_exclusive' => 1);

		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
		if (!$submission->create($copy_link)) {
			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return $submission;

	}//end createSubmission()


	/**
	* Add valid keywords for this asset to an array of keywords when asked
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								we add keywords to the $vars['keywords'] array
	*
	* @return boolean
	* @access private
	*/
	function onRequestKeywords(&$broadcaster, $vars=Array())
	{
		if (!isset($vars['keywords'])) return;

		$keywords = Array();
		// search for a bodycopies folder
		$folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, 'bodycopies_folder');

		$folder =& $GLOBALS['SQ_SYSTEM']->am->getAsset($folder_link['minorid'], $folder_link['minor_type_code']);

		if (is_null($folder)) return FALSE;

		// lets work out which bodycopy the WYSIWYG cell that wants our keywords
		// is in so we can return the correct keywords for the screen
		$bodycopy_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($folder->id, SQ_LINK_TYPE_2, 'bodycopy');
		$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id, 'bodycopy', TRUE);

		$bodycopies = Array();
		foreach ($bodycopy_links as $link_info) {
			if (isset($parents[$link_info['minorid']])) {
				$bodycopies[] = $link_info['value'];
			}
		}
		if (empty($bodycopies)) return;

		$questions = $this->getQuestions();
		$sections =& $this->getSections();

		foreach ($bodycopies as $bodycopy) {
			if ($bodycopy == 'Page Contents') {
				$keywords['form_contents'] = translate('cms_form_contents');
				$keywords['form_errors']   = translate('cms_form_errors');
				$keywords['form_errors_message']   = translate('cms_form_errors_message');

				foreach ($questions as $q_id => $question) {
					$q_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($this->id.':q'.$q_id);
					$q_name = $q_asset->attr('name');
					$keywords['question_field_'.$this->id.'_q'.$q_id] = translate('cms_form_question_field', ellipsisize($q_name,30));
					$keywords['question_id_'.$this->id.'_q'.$q_id] = translate('cms_form_question_id', ellipsisize($q_name,30));
					$keywords['question_note_'.$this->id.'_q'.$q_id] = translate('cms_form_question_note', ellipsisize($q_name,30));
					$keywords['question_label_'.$this->id.'_q'.$q_id] = 'Question label for '.ellipsisize($q_name,30);
				}

				foreach ($sections as $section) {
					$keywords['section_title_'.$section->id] = translate('cms_form_section_title', $section->attr('name'));
					$questions = $section->getQuestions();
					foreach ($questions as $q_id => $question) {
						$q_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($section->id.':q'.$q_id);
						$q_name = $q_asset->attr('name');
						$keywords['question_field_'.$section->id.'_q'.$q_id] = translate('cms_form_section_q_field', ellipsisize($section->attr('name'),20), ellipsisize($q_name,30));
						$keywords['question_id_'.$section->id.'_q'.$q_id] = translate('cms_form_section_q_id', ellipsisize($section->attr('name'),20), ellipsisize($q_name,30));
						$keywords['question_note_'.$section->id.'_q'.$q_id] = translate('cms_form_section_q_note', ellipsisize($section->attr('name'),20), ellipsisize($q_name,30));
						$keywords['question_label_'.$section->id.'_q'.$q_id] = 'Question label for '.ellipsisize($q_name,30);
					}
				}

				// These are best at the end
				$keywords['reset_button']  = translate('cms_form_reset_button');
				$keywords['submit_button'] = translate('cms_form_submit_button');

			} else if ($bodycopy == 'Thank You') {
				$keywords['form_summary']               = translate('cms_form_submission_summary');
				$keywords['form_info']                  = translate('cms_form_submission_info');
				$keywords['form_submission_id']         = translate('cms_form_submission_id');
				$keywords['form_submission_ip_address'] = translate('cms_form_submission_ip_address');
				$keywords['form_submission_time']       = translate('cms_form_submission_time_format', translate('date_format_default'));

				foreach (get_date_format_names() as $date_name => $date_format) {
					$keywords['form_submission_time_'.$date_name] = translate('cms_form_submission_time_format', translate('date_format_'.$date_name));
				}

				foreach ($questions as $q_id => $question) {
					$q_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($this->id.':q'.$q_id);
					$q_name = $q_asset->attr('name');
					$keywords['response_'.$this->id.'_q'.$q_id]      = translate('cms_form_question_response', ellipsisize($q_name,30));
					$keywords['question_name_'.$this->id.'_q'.$q_id] = translate('cms_form_question_name', ellipsisize($q_name,30));
					$keywords['question_note_'.$this->id.'_q'.$q_id] = translate('cms_form_question_note', ellipsisize($q_name,30));
				}

				foreach ($sections as $section) {
					$keywords['section_title_'.$section->id] = translate('cms_form_section_title', $section->attr('name'));
					$questions = $section->getQuestions();
					foreach ($questions as $q_id => $question) {
						$q_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($section->id.':q'.$q_id);
						$q_name = $q_asset->attr('name');
						$keywords['response_'.$section->id.'_q'.$q_id]   = translate('cms_form_section_response', ellipsisize($section->attr('name'),20), ellipsisize($q_name,30));
						$keywords['question_name_'.$this->id.'_q'.$q_id] = translate('cms_form_section_q_name', ellipsisize($q_name,30));
						$keywords['question_note_'.$this->id.'_q'.$q_id] = translate('cms_form_section_q_note', ellipsisize($q_name,30));
					}
				}

			}


		}//end foreach

		$vars['keywords'] = array_merge($vars['keywords'], $keywords);

	}//end onRequestKeywords()


	/**
	* Returns an array of assetid's + info in the order that they propogate out from THIS BRIDGE'S url
	*
	* @param string	$assetid	the id of the last asset in the lineage
	* @param string	$protocol	the protocol to match -> null means it is ignored
	* @param string	$url		the url to check for -> null defaults it to current url
	*
	* @return array
	* @access public
	*/
	function getLineageFromURL($assetid, $protocol, $url)
	{
		return Array();

	}//end getLineageFromURL()


	/**
	* Returns an array of all user or user_group assets that have the passed permission for the passed asset
	*
	* @param int		$assetid		the assetid for asset whose permissions to get
	* @param string		$permission		the permission code you are getting
	* @param boolean	$granted		type of Access : null = all, TRUE = granted, FALSE = denied
	* @param boolean	$and_greater	get effective permission (eg read access = read
	*									or write or admin)
	* @param boolean	$expand_groups	expand user groups so only user ids get returned
	*									(NOTE: only valid if $all_info is FALSE)
	* @param boolean	$all_info		when FALSE fn just returns an array of userids for
	*									those that have permission
	*									When TRUE fn returns all info about the permission in the form of
	*									Array(userid => granted)
	*									(NOTE: TRUE is only valid if $and_greater and
	*									$expand_groups are FALSE)
	*
	* @return array
	* @access public
	*/
	function getPermission($assetid, $permission, $granted=NULL, $and_greater=TRUE, $expand_groups=FALSE, $all_info=FALSE)
	{
		return Array();

	}//end getPermission()


	/**
	* Sets a permission for the passed user or user_group
	*
	* @param int		$assetid	the assetid for asset whose permissions to set
	* @param int		$userid		the id of the user or user_group to add the permission for
	* @param string		$permission	the permission code you are adding
	* @param boolean	$granted	whether this permission grants access or not
	*
	* @return boolean
	* @access public
	*/
	function setPermission($assetid, $userid, $permission, $granted)
	{
		return FALSE;

	}//end setPermission()


	/**
	* Deletes a permission with the passed user or user_group
	*
	* @param int	$assetid	the assetid for asset whose permissions to delete
	* @param int	$userid		the id of the user or user_group to remove the permission from
	* @param string	$permission	the permission code you are deleting
	*
	* @return boolean
	* @access public
	*/
	function deletePermission($assetid, $userid, $permission)
	{
		return FALSE;

	}//end deletePermission()


	/**
	* Returns an array of information about assets of the passed codes
	*
	* If one value in $field then the return value is Array(assetid => '[field]'), otherwise Array(assetid => Array())
	*
	* @param array		$assetids			an array of assets to limit
	* @param mixed		$type_code			an asset type code, or array of typecodes, to limit results to
	* @param boolean	$strict_type_code	ensure returned assets are of the passed type codes
	* @param string		$field				single field from the asset table to return (along with the assetid)
	*										Default is all fields
	*
	* @return array
	* @access public
	*/
	function getAssetInfo($assetids, $type_code=Array(), $strict_type_code=TRUE, $field='')
	{
		return Array();

	}//end getAssetInfo()


	/**
	* Determine if the passed assetid or assetids refer(s) to a valid asset in the system
	*
	* Note that this will always return FALSE if you pass it a shadow asset
	*
	* @param mixed	$assetids	the ID of the asset we are going to the check for existence
	*							or an array of assetids we want to check
	*
	* @return mixed boolean|array
	* @access public
	*/
	function assetExists($assetids)
	{
		return FALSE;

	}//end assetExists()


	/**
	* Get details of a link with the specified characteristics
	*
	* @param int		$assetid			id of the the $side_of_link asset
	* @param int		$link_type			integer that should be a single integer of the SQ_LINK_* constants
	* @param mixed		$type_code			the type of asset that is linked
	*										(eg 'Page', 'File', etc)
	*										if an array returns link if matches any
	*										of the array values
	* @param boolean	$strict_type_code	whether we are finding an asset that
	*										is just a $type_code or potentially an inherited type
	*										or $type_code and any of it's sub-classes
	* @param string		$value				the value that is associated with this link
	* @param string		$side_of_link		Which side of the link this (the current) asset
	*										is on ('major' or 'minor')
	* @param boolean	$exclusive			The exclusive status for the link must be
	*										this (if not null)
	*
	* @return array
	* @access public
	*/
	function getLink($assetid, $link_type=NULL, $type_code='', $strict_type_code=TRUE, $value=NULL, $side_of_link='major', $exclusive=NULL)
	{
		return Array();

	}//end getLink()


	/**
	* Get details of the link with specified linkid
	*
	* @param int	$linkid			id of the link were returning
	* @param int	$assetid		id of the the $side_of_link asset
	*								if zero no check is made and both major and minor information is returned
	* @param string	$side_of_link	Which side of the link the asset represented by $assetid is on ('major' or 'minor')
	*
	* @return array
	* @access public
	*/
	function getLinkById($linkid, $assetid=0, $side_of_link='major')
	{
		return Array();

	}//end getLinkById()


	/**
	* Return the number of links in the system that involve the specified asset
	*
	* Can be restricted by either link type afnd/or asset type code
	*
	* @param int		$assetid			id of the the $side_of_link asset
	* @param string		$side_of_link		Which side of the link this (the current) asset
	*										is on ('major' or 'minor')
	* @param int		$link_types			integer that can be the product of bitwise operations
	*										on the SQ_LINK_* constants
	* @param mixed		$type_code			the type of asset that is linked (eg 'Page', 'File', etc)
	*										if an array returns link if matches any
	*										of the array values
	* @param boolean	$strict_type_code	whether we are finding assets that are just a $type_code
	*										or $type_code and any of it's sub-classes
	* @param int		$ignore_linkid		ignore the link represented by this link
	*										id when returning the count
	*
	* @return int
	* @access public
	*/
	function countLinks($assetid, $side_of_link='major', $link_types=0, $type_code='', $strict_type_code=TRUE, $ignore_linkid=0)
	{
		return 0;

	}//end countLinks()


	/**
	* Get details of the link(s) between the two specified assets
	*
	* @param int		$assetid		id of the the $side_of_link asset
	* @param int		$other_assetid	the asset on the other side of the link
	* @param int		$link_types		integer that can be the product of bitwise operations
	*									on the SQ_LINK_* constants
	* @param string		$value			the value that is associated with this link
	* @param string		$side_of_link	Which side of the link the first assetid is on
	*									('major' or 'minor')
	* @param boolean	$force_array	force the results to return an array of links
	*									even if there is only one link
	* @param boolean	$dependant		The dependant status for all the links must be
	*									this (if not null)
	* @param boolean	$exclusive		The exclusive status for all the links must be
	*									this (if not null)
	*
	* @return array
	* @access public
	*/
	function getLinkByAsset($assetid, $other_assetid, $link_types=NULL, $value=NULL, $side_of_link='major', $force_array=FALSE, $dependant=NULL, $exclusive=NULL)
	{
		return Array();

	}//end getLinkByAsset()


	/**
	* Get details of the links from the specified asset to all its children
	*
	* @param string	$assetid	the assetid of the asset to find the child links for
	* @param int	$link_type	the type of links to get
	*
	* @return array
	* @access public
	*/
	function getAllChildLinks($assetid, $link_type=0)
	{
		return Array();

	}//end getAllChildLinks()


	/**
	* Update the details of an existing link
	*
	* If any of the detail vars are NULL they are not updated.
	*
	* @param int	$linkid		the link id of the link to update
	* @param int	$link_type	the type of the link (one of the SQ_LINK_* constants)
	* @param string	$value		the value to place on the link
	* @param int	$sort_order	the position in the links list that this link should take,
	*							if less than zero places at end of list
	*
	* @return boolean
	* @access public
	*/
	function updateLink($linkid, $link_type=NULL, $value=NULL, $sort_order=NULL)
	{
		return FALSE;

	}//end updateLink()


	/**
	* Remove a link by id
	*
	* @param int		$linkid	the link id of the link to remove
	* @param boolean	$moving	TRUE if this delete is part of a move operation
	*
	* @return boolean
	* @access public
	*/
	function deleteAssetLink($linkid, $moving=FALSE)
	{
		return FALSE;

	}//end deleteAssetLink()


	/**
	* Create a link between the two supplied assets
	*
	* @param object		&$major		the major asset that we are linking from
	* @param object		&$minor		the minor asset that we are linking to
	* @param string		$link_type	the type of link this is
	* @param string		$value		the value that is to be associated with this link
	* @param string		$sort_order	the position in the links list that this link should take,
	*								if null or less than zero places at end of list
	* @param string		$dependant	'0' / '1' - whether the minor asset is dependant on the major
	* @param string		$exclusive	'0' / '1' - whether the major asset is to be the minor's only parent
	* @param boolean	$moving		whether or not this asset is being moved
	*
	* @return int
	* @access public
	*/
	function createAssetLink(&$major, &$minor, $link_type, $value='', $sort_order=NULL, $dependant='0', $exclusive='0', $moving=FALSE)
	{
		return 0;

	}//end createAssetLink()


	/**
	* Get appropriate keywords for Thank You bodycopy and emails
	*
	* @param string		$keyword	keyword to replace
	*
	* @return string
	* @access protected
	*/
	function _getThankYouKeywordReplacement($keyword)
	{
		$replacement = '%'.$keyword.'%';
		$prefix = $this->getPrefix();

		// referrer gets referred to (!) in multiple keywords, so get it up here
		$referrer = (isset($_POST[$prefix.'_referral_url'])) ? $_POST[$prefix.'_referral_url'] : '';

		// Easier to use if-else if rather than switch, because some of these
		// will be regex matches

		if ($keyword === 'form_info_html') {
			// Form info keyword - HTML email
			// %form_info_html%

			$replacement = $this->getFormInfo(TRUE);

		} else if ($keyword === 'form_info_text') {
			// Form info keyword - text email
			// %form_info_text%

			$replacement = $this->getFormInfo(FALSE);

		} else if ($keyword === 'form_summary_html') {
			// Form summary keyword - HTML email
			// %form_summary_html%

			ob_start();
				$this->printSummary(TRUE);
				$replacement = ob_get_contents();
			ob_end_clean();

		} else if ($keyword === 'form_summary_text') {
			// Form summary keyword
			// %form_summary_text%

			ob_start();
				$this->printSummary(FALSE);
				$replacement = ob_get_contents();
			ob_end_clean();

		} else if ($keyword === 'form_submission_id') {
			// Asset ID of the form submission
			// %form_submission_id%

			if (!is_null($this->submission_asset)) {
				$replacement = $this->submission_asset->id;
			}

		} else if (preg_match('|^form_submission_time(_.*)|', $keyword, $matches)) {
			// Submission time - can use date formats - if not recognised as
			// a valid shorthand date format, it's assumed to be a date() format
			// (just like date keywords elsewhere)
			// %form_submission_time_rfc2822%

			if (empty($matches[1])) {
				$date_format = 'Y-m-d H:i:s';
			} else {
				$date_format = substr($matches[1], 1);
			}

			if (!is_null($this->submission_asset)) {
				$date_formats_list = get_date_formats();
				if (array_key_exists($date_format, $date_formats_list)) {
					$replacement = format_date($this->submission_asset->created, $date_format);
				} else {
					$replacement = date($date_format, $this->submission_asset->created);
				}
			}

		} else if ($keyword === 'form_submission_ip_address') {
			// IP address of the submitting user
			// %form_submission_ip_address%

			// TODO: Should we handle proxy-forwarded IP addresses
			//       (X-Forwarded-For header)?
			$ip_address = $_SERVER['REMOTE_ADDR'];
			$replacement = $ip_address;

		} else if (preg_match('|^form_asset_(.*)|', $keyword, $matches)) {
			// form PAGE asset keyword
			// eg. %form_asset_assetid%

			$parent_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'page_custom_form', FALSE, NULL, 'minor');
			$parent_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($parent_link['majorid']);

			$form_keyword = 'asset_'.$matches[1];
			$asset_replacement = $parent_asset->getKeywordReplacement($form_keyword);
			if ($asset_replacement !== "%$form_keyword%") {
				$replacement = $asset_replacement;
			}

		} else if ($keyword === 'referral_url') {
			// URL of the referring page/asset
			// %referral_url%

			$replacement = $referrer;

		} else if (preg_match('|^referring_asset_(.*)|', $keyword, $matches)) {
			// referring asset keyword, if we have been referred from an
			// asset within MySource Matrix
			// eg. %referring_asset_assetid%

			if (!empty($referrer)) {
				// use the url to work out the referring asset
				$url_parts = parse_url($referrer);
				$protocol = array_get_index($url_parts, 'scheme');
				$path = array_get_index($url_parts, 'host').array_get_index($url_parts, 'path');

				$referrer_asset =& $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL($protocol, $path, TRUE, TRUE);
				if (!is_null($referrer_asset)) {
					$referrer_keyword = 'asset_'.$matches[1];

					$referrer_replacement = $referrer_asset->getKeywordReplacement($referrer_keyword);
					if ($referrer_replacement !== "%$referrer_keyword%") {
						$replacement = $referrer_replacement;
					} else {
						$replacement = '';
					}
				}
			}//end if referrer is not empty

		} else {
			// questions that depend on knowing what questions and sections we have
			// We will cache them though, so we don't load them up all the time

			if (empty($this->_tmp['keyword_cache'])) {
				$this->_tmp['keyword_cache']['questions'] =& $this->getQuestions();
				$this->_tmp['keyword_cache']['sections' ] =& $this->getSections();
			}

			$questions =& $this->_tmp['keyword_cache']['questions'];
			$sections  =& $this->_tmp['keyword_cache']['sections'];

			if (preg_match('|^section_title_(.*)|', $keyword, $matches)) {
					// Section title keyword
					// eg. %section_title_123%

					$section_assetid = $matches[1];
					$section_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($section_assetid, 'form_section');
					if (!empty($section_asset)) {
						$replacement = $section_asset->attr('title');
					}

			} else if (preg_match('|^question_([^_]*)_([^_]*_)?q(.*)|', $keyword, $matches)) {
					// Question name/note keywords, eg.
					// %question_note_123_q4% ... note for q.4 from form/section
					//                            with ID 123
					// %question_name_q4% ....... name of q.4 from the form

					$question_keyword = $matches[1];
					if (empty($matches[2])) {
						// One asset ID
						$section_assetid = $this->id;
						$question_assetid = $matches[3];
					} else {
						$section_assetid = str_replace('_', '', $matches[2]);
						$question_assetid = $matches[3];
					}

					$question_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($section_assetid.':q'.$question_assetid);
					if (!empty($question_asset)) {
						switch ($question_keyword) {
							case 'name':
								$replacement = $question_asset->attr('name');
							break;

							case 'note':
								$replacement = $question_asset->attr('note');
							break;
						}
					}

			} else if (preg_match('|^response_(.*_)?q(.*)?|', $keyword, $matches)) {
					// Question response keywords, eg.
					// %response_123_q4% ... question 4 from form/section ID 123
					// %response_q4% ....... question 4 from the form

					if (empty($matches[1])) {
						// One asset ID
						$section_assetid = $this->id;
						$question_assetid = $matches[2];
					} else {
						$section_assetid = str_replace('_', '', $matches[1]);
						$question_assetid = $matches[2];
					}

					$question_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($section_assetid.':q'.$question_assetid);
					if (!empty($question_asset)) {
						$replacement = $question_asset->getSummary();
					}

			}

		}//end if - keyword list

		return $replacement;

	}//end _getThankYouKeywordReplacement()


}//end class
?>
