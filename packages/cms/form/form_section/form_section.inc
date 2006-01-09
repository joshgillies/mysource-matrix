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
* $Id: form_section.inc,v 1.47 2006/01/09 03:41:04 emcdonald Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Form_Section
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Revision: 1.47 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
register_implementation('form_section', 'bridge');
class Form_Section extends Asset
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
	var $current_answers = Array();


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
		$this->_ser_attrs = TRUE;
		$this->Asset($assetid);

	}//end constructor


//--        BRIDGE FUNCTIONS        --//


	/**
	* Returns a reference to the asset represented by the passed assetid
	*
	* @param int		$shadowid		the shadow part asset id to be loaded (i.e part after the ':')
	* @param string		$type_code		if this exists then this object is used to
	*									load the asset, if not then the DB is queried
	*									to find out the asset type
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed because
	*									you can't use the '@' operator when returning by reference
	*
	* @return mixed object|null
	* @access public
	* @see Asset_Manager::&getAsset()
	*/
	function &getAsset($shadowid, $type_code='', $mute_errors=FALSE)
	{
		$asset = null;
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
	function getLinks($assetid, $link_types, $type_code='', $strict_type_code=TRUE, $side_of_link='major', $sort_by=null)
	{
		// there are no links away from the shadow asset (ie:questions)
		$links = Array();

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

		$new_sort_order = 0;

		// There are no real links made at a section level
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

			// we would remove real links at this point, but Form Section doesn't
			// have any real links underneath it

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
		// basically get the parents of the section, and add itself
		$query = $GLOBALS['SQ_SYSTEM']->am->generateGetParentsQuery($this->id, $type_code, $strict_type_code);
		if (empty($query)) return Array();

		$queried_parents = $GLOBALS['SQ_SYSTEM']->db->getAll(implode(' ', $query));
		assert_valid_db_result($queried_parents);
		$parents = Array();

		foreach ($queried_parents as $queried_parent) {
			$parents[$queried_parent['majorid']] = $queried_parent['type_code'];
		}

		// TODO: add this asset only if it meets type code requirements
		$parents[$this->id] = $this->type();

		return $parents;

	}//end getParents()


	/**
	* Get all asset ids that are below the passed assetid in the various trees in which it exists
	*
	* returns array(int => string)	returns an array of assetids and their type code
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

		// we don't have to get any real links, sections only have question children

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
	* Returns an array of assetid's + info in the order that they propogate out from THIS BRIDGE'S url
	*
	* @param string	$assetid	the id of the last asset in the lineage
	* @param string	$protocol	the protocol to match -> null means it is ignored
	* @param string	$url		the urls to check for -> null defaults it to current url
	*
	* @return array
	* @access public
	*/
	function getLineageFromURL($assetid, $protocol, $url)
	{
		return Array();

	}//end getLineageFromURL()


	/**
	* get the section links attached to this form
	*
	* @param boolean	$force	whether to force... something or not
	*
	* @return array
	* @access public
	*/
	function getQuestions($force=FALSE)
	{
		return $this->attr('questions');

	}//end getQuestions()


	/**
	* get the question assets attached to this section
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
				$id           =  $this->id.':q'.$q_id;
				$result[$id]  =& $am->getAsset($id);
			}
		}

		return $result;

	}//end getQuestionAssets()


	/**
	* get a specific question
	*
	* @param int	$questionid	The id of the desired question
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
	* get specific question by order
	*
	* @param int	$orderid	the sequence of the desired question
	*
	* @return array
	* @access public
	*/
	function &getQuestionByOrder($orderid)
	{
		$sort_order =& $this->attr('sort_order');
		return $GLOBALS['SQ_SYSTEM']->am->getAsset($sort_order[$orderid]);

	}//end getQuestionByOrder()


	/**
	* attach a question to this form
	*
	* FALSE if database problem or question already attached, TRUE if sucessful
	*
	* @param string	$type_code	the type of the question to attach
	*
	* @return boolean
	* @access public
	*/
	function attachQuestion($type_code)
	{
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
								'attributes'			=> Array(
															'name'	=> 'Question '.(count($questions)+1),
														   ),
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
			return FALSE;
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return TRUE;

	}//end attachQuestion()


	/**
	* delete a question to this form
	*
	* returns FALSE if database problem or question already deleted, TRUE if sucessful
	*
	* @param object	&$question	the question object to delete
	*
	* @return boolean
	* @access public
	*/
	function deleteQuestion(&$question)
	{
		if ($question->_questionid == 0) return FALSE;

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
			return FALSE;
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return TRUE;

	}//end deleteQuestion()


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
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		return Array();

	}//end _getAllowedLinks()


	/**
	* regenerates the content file for this section
	*
	* @param boolean	$form_regen	if TRUE the section will get all its forms to regen as well
	*
	* @return boolean
	* @access public
	*/
	function fileRegeneration($form_regen=TRUE)
	{
		// we want to regen this section
		$edit =& $this->getEditFns();
		$edit->generateContentFile($this);

		// sometimes, we might not want to regen the form (eg. if the form is calling this function!)
		if ($form_regen) {
			// then, get all the links of this guy's form parents
			$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'form', FALSE, 'minor');

			foreach ($links as $l) {
				$form =& $GLOBALS['SQ_SYSTEM']->am->getAsset($l['majorid'], $l['major_type_code']);
				if (is_null($form)) continue;
				if (!$form->_updated()) {
					unset($form);
					return FALSE;
				}
				unset($form);
			}
		}//end if

		return TRUE;

	}//end fileRegeneration()


	/**
	* Clones certain specified components of the asset
	*
	* @param object	&$clone		the clone shell
	* @param array	$components	the wanted components to clone eg.
	* <pre>
	*                       Array(
	*                         'attributes',
	*                         'metadata_shemas',
	*                         'metadata',
	*                         'workflow',
	*                         'permissions',
	*                         'data',
	*						  'content_tags'
	*                       );
	*                       or alternately
	*                       Array('all');
	* </pre>
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
			return FALSE;
		}

		if (in_array('attributes', $components) || in_array('all', $components)) {
			$sort_order = $this->attr('sort_order');
			$sort_order = str_replace($this->id.':q', $clone->id.':q', $sort_order);

			// save the information
			$clone->setAttrValue('sort_order', $sort_order);
			$clone->saveAttributes();

			// Now remap any parent forms, but only if we are being cloned along with a form clone:
			// remap to the cloned form. If we are being clone within a form, we will not remap.
			$form_parents = $GLOBALS['SQ_SYSTEM']->am->getDependantParents($this->id, 'form_email', FALSE);
			$clone_form_parents = $GLOBALS['SQ_SYSTEM']->am->getDependantParents($clone->id, 'form_email', FALSE);

			$clone_form_parents = array_diff($clone_form_parents, $form_parents);

			foreach ($clone_form_parents as $parent_assetid) {
				$form =& $GLOBALS['SQ_SYSTEM']->am->getAsset($parent_assetid);

				$recip = unserialize($form->attr('recipient_emails_format'));
				$recip = str_replace('%response_'.$this->id.'_', '%response_'.$clone->id.'_', $recip);
				$rec = unserialize($form->attr('receipt_emails_format'));
				$rec = str_replace('%response_'.$this->id.'_', '%response_'.$clone->id.'_', $rec);
				$staf = unserialize($form->attr('staf_format'));
				$staf = str_replace('%response_'.$this->id.'_', '%response_'.$clone->id.'_', $staf);
				$sel = $form->attr('selective_emails');
				foreach ($sel as $key => $null) {
					$sel[$key]['address'] = str_replace('%response_'.$this->id.'_', '%response_'.$clone->id.'_', $sel[$key]['address']);
					$sel[$key]['assetid'] = str_replace($this->id.':q', $clone->id.':q', $sel[$key]['assetid']);
				}

				// field selections for STAF and receipt
				$staf_field = str_replace($this->id.':q', $clone->id.':q', $form->attr('staf_field'));
				$receipt_field = str_replace($this->id.':q', $clone->id.':q', $form->attr('receipt_field'));

				// save the information
				$form->setAttrValue('recipient_emails_format', serialize($recip));
				$form->setAttrValue('receipt_emails_format', serialize($rec));
				$form->setAttrValue('staf_format', serialize($staf));
				$form->setAttrValue('selective_emails', $sel);

				$form->setAttrValue('staf_field', $staf_field);
				$form->setAttrValue('receipt_field', $receipt_field);

				$form->saveAttributes();
			}
		}//end if

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return TRUE;

	}//end cloneComponents()


	/**
	* returns the count of questions linked to this section
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
	* returns the name of this section
	*
	* @param boolean	$short_name	if TRUE, returns the short name
	*
	* @return string
	* @access private
	*/
	function _getName($short_name=FALSE)
	{
		return $this->attr('name');

	}//end _getName()


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
		$questions = $this->getQuestions();
		$code = '';						// blank sheet of code to start with

		foreach ($questions as $q_id => $question) {
			$q_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($this->id.':q'.$q_id);
			$q_code = $q_asset->generateJSCode();

			$code .= $q_code.(empty($q_code) ? '' : "\n");
		}

		return $code;

	}//end generateJSCode()


}//end class
?>
