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
* $Id: form.inc,v 1.45 2004/07/07 01:00:35 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/bridge/bridge.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_FUDGE_PATH.'/general/datetime.inc';

/**
* Form
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form extends Bridge
{


	/**
	* the active submission array
	* @var Array (string => string) 
	*/
	var $active_submission = Array();

	/**
	* the active submission array
	* @var Array (of string) 
	*/
	var $submission_errors = Array();
	
	
	/**
	* the active section object
	* @var object 
	*/
	var $active_section = null;


	/**
	* available bodycopy keywords
	* @var Array() 
	*/
	var $_available_keywords = Array('form_contents', 'form_errors', 'submit_button', 'reset_button');


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form($assetid=0) 
	{
		$this->_ser_attrs = true;
		$this->Bridge($assetid);

	}//end Form()


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

		// if the shadow id is numeric then we want a REAL asset (ie. a section)
		if (is_numeric($shadowid)) return $GLOBALS['SQ_SYSTEM']->am->getAsset($shadowid);

		if ($shadowid{0} == 'q') {	// Questions will be preceded with a 'q'
				
			// pick up the question in question (!)
			$questionid = substr($shadowid, 1, strlen($shadowid));
			$data = $this->getQuestionById($questionid);
			if (empty($data)) return $asset;

			$data['questionid'] = $questionid;

			$GLOBALS['SQ_SYSTEM']->am->includeAsset($data['question_type_code']);

			$type_code = $data['question_type_code'];

			// this eval returns the necessary object for the question type
			$asset = new $type_code($this->id, $data);
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

			// We also need the REAL asset links here, so we can access the form sections
			$links = array();

			// but we also need question shadow asset links
			$entries = $this->getQuestions();

			$i = 0;

			foreach ($links as $link) {
				$i = max($i, $link['sort_order']);
			}

			foreach ($entries as $questionid => $data) {
				
				$links[] = Array('linkid' => 0,
								 'majorid' => $this->id,
								 'minorid' => $this->id.':q'.$questionid,
								 'minor_type_code' => $data['question_type_code'],
								 'value' => '',
								 'link_type' => SQ_LINK_TYPE_2,
								 'dependant' => true,
								 'exclusive' => false,
								 'sort_order' => $i + array_search($this->id.':q'.$questionid, $this->attr('sort_order')),
								);
			}

			if (!is_null($sort_by)) {
				uasort($links, create_function('$a,$b','return $a["'.$sort_by.'"] > $b["'.$sort_by.'"];'));
			}

			$this->_tmp['getLinks'][$assetid][$side_of_link] = $links;

		}

		return $this->_tmp['getLinks'][$assetid][$side_of_link];

	}//end getLinks()
	

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
			array_push($links, array_pop($old_links));
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

		// this is so we get the sections as we go
		// have to do this all in here because calling asset manager will bring on a vicious loop

		$asset = &$this->getAsset($assetid);
		if (is_a($asset, 'bridge') && ($assetid != $this->id)) {
			$children = $asset->getChildren($assetid, $type_code, $strict_type_code, $sort_by);
			$this->forgetAsset($asset);
		} else {

			$query = $GLOBALS['SQ_SYSTEM']->am->generateGetChildrenQuery($asset, $type_code, $strict_type_code, $sort_by);
			unset($asset);
			if (!empty($query)) {

				$result = $GLOBALS['SQ_SYSTEM']->db->getAssoc(implode(' ', $query));
				assert_valid_db_result($result);

				$children =& $result;
			}
		}

		// now questions

		$entries = $this->getQuestions();

		foreach ($entries as $questionid => $data) {
			$children[$this->id.':q'.$questionid] = $data['question_type_code'];
		}

		if (!is_null($sort_by)) {
			uasort($children, create_function('$a,$b','return $a["'.$sort_by.'"] > $b["'.$sort_by.'"]'));
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
	* regenerates the content file for this form
	*
	* @access public
	* @return boolean
	*/
	function fileRegeneration()
	{
		$edit = $this->getEditFns();
		return $edit->generateStandardContentFile($this);

	}//end fileRegeneration()

	
	/**
	* get the question links attached to this form
	* 
	* @access public
	* @return array(link)
	*/
	function &getQuestions($force=false)
	{
		return $this->attr('questions');

	}//end getQuestions()
	
	
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

		// acquire the lock on the new question
		$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id . ':q' . $new_key, 'all', $this->id, true);
		$GLOBALS['SQ_SYSTEM']->am->updateLock($this->id, 'all');

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
	* returns the number of sections in this form
	*
	* @param &object $asset the form asset
	*
	* @access public
	* @return integer number of sections
	*/
	function getQuestionCount()
	{
		$questions = &$this->getQuestions();
		return count($questions);

	}//end getQuestionCount()
	
	
	/**
	* get a specific question
	* 
	* @access public
	* @return array(link)
	*/
	function getQuestionByID($questionid)
	{
		$questions = &$this->getQuestions();
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
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$page_links = Array(SQ_LINK_TYPE_2 => Array(
						'form_section' => Array('card' => 'M', 'exclusive' => false),
						'folder' => Array('card' => '1', 'exclusive' => true)
						));
		return $page_links;

	}//end _getAllowedLinks()


	/**
	* returns all of the sections in this form in order of 'sort_order'
	* 
	* @param &object $asset the form asset
	*
	* @access public
	* @return &array(object form_section)
	*/
	function &getSections() 
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$section_links = &$this->getSectionLinks();
		$sections = Array();

		foreach ($section_links as $link) {
			$sections[$link['sort_order']] = &$am->getAsset($link['minorid'], $link['minor_type_code']);
		}
		return $sections;

	}//end getSections()

	
	/**
	* returns the number of sections in this form
	*
	* @param &object $asset the form asset
	*
	* @access public
	* @return integer number of sections
	*/
	function getSectionCount()
	{
		return $GLOBALS['SQ_SYSTEM']->am->countLinks($this->id, 'major', SQ_LINK_TYPE_2, 'form_section', true);

	}//end getSectionCount()


	/**
	* get the section links attached to this form
	* 
	* @param &object $asset the form asset
	* 
	* @access public
	* @return array(link)
	*/
	function getSectionLinks()
	{
		if (empty($this->tmp['section_links'])) {
			$this->tmp['section_links'] = Array();

			// This is what one would find in Asset_Manager::&getAsset() except we can't call that,
			// lest we spin off into an infinite loop involving OUR &getAsset().

			$query = $GLOBALS['SQ_SYSTEM']->am->generateGetLinksQuery($this->id, SQ_LINK_TYPE_2, 'form_section', true, 'major', '', '0', '0');
			if (empty($query)) return Array();
			$query['select'] .= ', l.majorid';		// we need the major id too

			$db = &$GLOBALS['SQ_SYSTEM']->db;
			$result = $db->getAll(implode(' ', $query));
			if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
			$this->tmp['section_links'] = &$result;
		}

		return (!empty($this->tmp['section_links'])) ? $this->tmp['section_links'] : Array();

	}//end getSectionLinks()


	/**
	* saves a cookie to the client based on a submission id so they can
	* come back later to view saved forms 
	*
	* @param string		$id		the value to be saved to the cookie
	* @param int		$days	the number of days before expiry
	*
	* @access public
	* @return boolean
	*/
	function setFormCookie($id, $days=0)
	{
		return setcookie('SQ_FORM_'.$this->getPrefix(), $id, time() + 60 * 60 * 24 * $days);

	}//end setFormCookie() 


	/**
	* Called by the design to print the body of this asset
	*
	* @access public
	* @return void
	*/
	function printBody()
	{
		$prefix = $this->getPrefix(); 

		?>
		<form name="<?php echo $prefix?>" enctype="multipart/form-data" action="<?php echo $_SERVER['PHP_SELF']?>" method="POST" <?php if ($this->attr('use_client_side')) { ?>
		onsubmit="return beforeSubmit_<?php echo $prefix?>(this);"<?php }
		echo '>';

		// check to see if there is a past submission that belongs to this user
		// check to see if this form is set up to be a wizard or not
		$this->printStandard();
		echo ?></form><?php

	}//end printBody()


	/**
	* prints the standard form i.e not a wizard
	*
	* @param &object $submission the submission object
	* 
	* @access public
	* @return boolean
	*/
	function printStandard()
	{
		$prefix = $this->getPrefix();
		$is_error = false;

		// check to see if the form has been submitted or not
		if (isset($_POST[$prefix.'_submit'])) {
			
			// flush any errors that exist in the submisssion asset
			$this->processForm();
			
			// check to see if there is an error with the form
			if (count($this->submission_errors)) { 
				$is_error = true;
			} else {
				$this->finishForm();
			}
		}

		// nothing submitted yet, or there was an error
		if (!isset($_POST[$prefix.'_submit']) || $is_error) {

			ob_start();
			$this->includeContentFile('Standard');
			$cf_output = ob_get_contents();
			ob_end_clean();

			if ($is_error) {
				ob_start();
				$this->printErrors();
				$err_output = ob_get_contents();
				ob_end_clean();
			} else {
				$err_output = '';
			}

			$bodycopy = &$this->getBodycopy('Page Contents');
			
			// if no title for the submit button is specified in the backend
			// just use 'Submit'
			$button_name = ($this->attr('submit_button')) ? $this->attr('submit_button') : 'Submit'; 
		
			ob_start();
			submit_button($this->getPrefix().'_submit', $button_name);
			$submit_button = ob_get_contents();
			ob_end_clean();

			$reset_button_name = ($this->attr('reset_button')) ? $this->attr('reset_button') : 'Clear Form'; 

			ob_start();
			reset_button($this->getPrefix().'_reset', $reset_button_name);
			$reset_button = ob_get_contents();
			ob_end_clean();
	
			// check to see if the bodycopy attached to this form has any content
			if ($this->attr('use_bodycopy_main') && $bodycopy && !$bodycopy->isEmpty()) {

				ob_start();
				$bodycopy->setKeywordReplacements(Array('form_contents' => &$cf_output, 'form_errors' => &$err_output, 'submit_button' => &$submit_button, 'reset_button' => &$reset_button));
				$bodycopy->printBody();
				$output = ob_get_contents(); 
				ob_end_clean();

				echo $output;

			} else {
				$output = $cf_output;
	
				$output .= '<table width="'.$this->attr('form_width').'"><tr>';
				if ($this->attr('question_index')) {
					$output .= '<td width="20">&nbsp;</td>';
				}
				$output .= '<td';
				$output .= ($this->attr('question_col_width')) ? ' 	width="'.$this->attr('question_col_width').'">' : '>';
				$output .='&nbsp;</td><td';
				$output .= ($this->attr('answer_col_width')) ? ' 	width="'.$this->attr('answer_col_width').'">' 	: '>';
			
				$output .= $submit_button;
	
				$output .= '</td></tr>';
				if ($is_error) {
					$output .= '<tr><td colspan="3">'.$err_output.'</td></tr>';
				}
				$output .= '</table>';

				echo $output;
			}

		}
		return true;

	}//end printSummary()


	/**
	* gets a section to process itself
	*
	* @param integer $order the section order to process
	*
	* @access public
	* @return boolean
	*/
	function processSection($order)
	{
		// get the section to process itself
		$posted_section = &$this->getSectionByOrder($order);
		
		if (!is_null($this->active_submission)) {
			// let this section know what the current submission and errors arrays are
			$posted_section->active_submission =& $this->active_submission;
			$posted_section->submission_errors =& $this->submission_errors;
		}
		
		return $posted_section->processForm();

	}//end processSection()


	/**
	* create a bodycopy and attach to form
	* 
	* @param string $name the name of the bodycopy
	*
	* @access public
	* @return boolean false on error or if bodycopy already exists, true if successful
	*/
	function createBodycopy($name)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// search for a bodycopies folder and create one if there isn't one there
		$folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', true, 'bodycopies_folder');
		if (empty($folder_link)) {

			// create a folder for the bodycopies
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');
			$folder = new Folder();
			$folder->setAttrValue('name', 'Bodycopies');

			$folder_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'value' => 'bodycopies_folder', 'dependant' => 1, 'exclusive' => 1);

			if (!$folder->create($folder_link)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		} else {
			// folder already exists, get the reference
			$folder = &$GLOBALS['SQ_SYSTEM']->am->getAsset($folder_link['minorid'], $folder_link['minor_type_code']); 
		}
		if (is_null($folder)) return false;

		// and see whether this bodycopy already exists
		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', true, $name);
		if (!empty($bodycopy_link)) return true;

		// bodycopy doesn't exist, so create it willya
		// create a link to the folder so we can put the bodycopies in it
		$copy_link = Array('asset' => &$folder, 'link_type' => SQ_LINK_TYPE_2, 'dependant' => 1, 'exclusive' => 1, 'value' => $name);

		// create the thing
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$bodycopy = new Bodycopy();
		$bodycopy->setAttrValue('name', $name);
			
		// create an appropriate value for this link
		// to reflect that of the name
		$name = strtolower(str_replace(' ', '_', $name));

		if (!$bodycopy->create($copy_link)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return true;

	}//end attachQuestion()
	
	
	/**
	* returns a reference to a bodycopy based of the name of the bodycopy
	*
	* @param string		$name	the name of the bodycopy
	*
	* @access public
	* @return &asset bodycopy
	*/
	function &getBodycopy($name='')
	{
		$null = null;
		if (!$name) return $null;
		$am = &$GLOBALS['SQ_SYSTEM']->am;

		$folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', true, 'bodycopies_folder');
		if (empty($folder_link)) return $null;
		
		$folder = &$am->getAsset($folder_link['minorid'], $folder_link['minor_type_code']); 
		if (is_null($folder)) return $null;

		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', true, $name);
		if (empty($bodycopy_link)) return $null;

		$bodycopy = &$am->getAsset($bodycopy_link['minorid'], $bodycopy_link['minor_type_code']);
		
		return ($bodycopy) ? $bodycopy : $null;
	
	}//end getBodycopy()

	
	/**
	* includes the content file for this form
	*
	* @param string $type the type of content file to generate
	*
	* @access public
	* @return boolean
	*/
	function includeContentFile($type = 'standard')
	{
		if (!file_exists($this->data_path.'/content_file.php')) {
			$edit = &$this->getEditFns();
			$fn = 'generate'.$type.'ContentFile';
			
			if (!method_exists($edit, $fn)) return false;
			if (!$edit->$fn($this)) return false;
		}
		include($this->data_path.'/content_file.php');

		return true;

	}//end includeContentFile()
	

	/**
	* processes the form (non-wizard)
	*
	* @param &object $submission the submisssion object
	*
	* @access public
	* @return boolean
	*/
	function processForm()
	{
		$questions = &$this->getQuestions();
		for (reset($questions); null !== ($k = key($questions)); next($questions)) {
			$question = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->id.':q'.$k);
			$question->submission_errors =& $this->submission_errors;
			$question->processForm();
		}
		
		$sections = &$this->getSections();
		for (reset($sections); null !== ($k = key($sections)); next($sections)) {
			$section =& $sections[$k];
			// there is still no problem with the section sharing the form's active submission
				$section->active_submission =& $this->active_submission;
				$section->submission_errors =& $this->submission_errors;
				$section->processForm();
		}

		return true;

	}//end processForm()


	/**
	* function to finish off form 
	*
	* @access public
	* @return boolean
	*/
	function finishForm()
	{
		$tyb = &$this->getBodycopy('Thank You');
					
		ob_start();
		$this->printSummary(true);
		$summary_info = ob_get_contents();
		ob_end_clean();
	
		// check to see if the bodycopy attached to this form has any content
		if ($this->attr('use_bodycopy_thank_you') && $tyb && !$tyb->isEmpty()) {

			if ($this->attr('show_results')) {
				$tyb->setKeywordReplacements(Array('form_summary' => &$summary_info));
			}

			$tyb->printBody();

		} else {			// no bodycopy

			// print the results if we need to 
			if ($this->attr('show_results')) $this->printSummary(true);

		}

	}//end finishForm()


	/**
	* prints the errors that occured in a form submission
	*
	* @access public
	* @return boolean
	*/
	function printErrors()
	{
		if(empty($this->submission_errors)) return false;

		?>
		<span>
		<b>The following errors must be corrected before the form can be submitted:</b><br />
		<ul>
		<?php

		foreach ($this->submission_errors as $error) {
			echo "<LI>".$error."</LI>\n";
		}

		?>
		</ul>
		</span>
		<?php
		
		return true;

	}//end printErrors()


	/**
	* returns an answer summary in XML format (suitable for submission logs)
	*
	* @access public
	* @return boolean
	*/
	function getXML()
	{
		ob_start();
		echo '<submission time="'.str_replace(' ','T',ts_iso8601(time())).'"';
		if (!is_null($cur_user =& $GLOBALS['SQ_SYSTEM']->user)) {
			echo ' user="'.$cur_user->id.'"';
		}
		echo '>';

		// unattached questions first
		$sort_order = $this->attr('sort_order');

		foreach($sort_order as $sort_order_index => $assetid) {
			
			// check to see whether this belongs to us (if it isn't then perhaps a section took care)
			if ($this->id == strtok($assetid, ':')) {
				$question = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				echo $question->getXML($this->active_submission[$assetid]);
			}

		}

		// now sections
		$sections = &$this->getSections();
		for (reset($sections); null !== ($k = key($sections)); next($sections)) {
			$section =& $sections[$k];
			echo '<section id="'.$section->id.'" name="'.$section->attr('name').'">';
		
			$sort_order = $section->attr('sort_order');

			foreach($sort_order as $sort_order_index => $assetid) {
			
				// check to see whether this belongs to the section
				if ($section->id == strtok($assetid, ':')) {
					$question = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
					echo $question->getXML($this->active_submission[$assetid]);
				}

			}

			echo '</section>';

		}

		echo '</submission>';

		$contents = ob_get_contents();
		ob_end_clean();

		return $contents;
	
	}//end printXML()
	
	
	/**
	* returns a summary about the answers in the form submission
	*
	* @param boolean	$html	return html formatted or not 
	*
	* @access public
	* @return boolean
	*/
	function printSummary($html = false)
	{
		echo ($html) ? '<table>' : '';

		// unattached questions first
		$sort_order = $this->attr('sort_order');

		foreach($sort_order as $sort_order_index => $assetid) {
			
			// check to see whether this belongs to us (if it isn't then perhaps a section took care)
			if ($this->id == strtok($assetid, ':')) {
				$question = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				echo ($html) ? '<tr><td><li>' : '';
				echo $question->attr('name');
				echo ($html) ? ': </li></td><td>' : ' : ';
				echo $question->getSummary(!isset($this->active_submission[$assetid]) ? '' : $this->active_submission[$assetid]);
				echo ($html) ? '</td></tr>' : "\n";
			}

		}

		// now sections
		$sections = &$this->getSections();
		for (reset($sections); null !== ($k = key($sections)); next($sections)) {
			$section =& $sections[$k];
			echo ($html) ? '<tr><td colspan="2">' : "\n";
			echo $section->attr('name');
			echo ($html) ? '</td></tr>' : "\n\n";

			$sort_order = $section->attr('sort_order');

			foreach($sort_order as $sort_order_index => $assetid) {
			
				// check to see whether this belongs to the section
				if ($section->id == strtok($assetid, ':')) {
					$question = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
					echo ($html) ? '<tr><td><li>' : '';
					echo $question->attr('name');
					echo ($html) ? ': </li></td><td>' : ' : ';
					echo $question->getSummary(!isset($this->active_submission[$assetid]) ? '' : $this->active_submission[$assetid]);
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
	* @access public
	* @return Array(string)
	*/
	function getFormInfo()
	{
		$info = Array();
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		
		$lineage = $am->getLineageFromURL();
		foreach ($lineage as $asset) {
			$info[$asset['type_code'].'_name'] = $asset['name'];
			$info[$asset['type_code'].'_id']   = $asset['assetid'];
		}
		$info['url'] = current_url(true, true);
		return $info;

	}//end getFormInfo()


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
		if (!$this->attr('use_client_side')) return '';

		$questions = &$this->getQuestions();
		$sections = &$this->getSections();
		$code = '';						// blank sheet of code to start with

		foreach ($questions as $q_id => $question) {
			$q_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->id.':q'.$q_id);
			$q_code = $q_asset->generateJSCode();
			$code .= $q_code . (empty($q_code) ? "" : "\n");
		}

		foreach ($sections as $section) {
			$s_code = $section->generateJSCode();
			$code .= $s_code . (empty($s_code) ? "" : "\n");
		}

		return $code;

	}//end generateJSCode()
	

	/**
	* logs submission to XML
	*
	* @access public
	* @return boolean
	*/
	function logSubmission($log_id = null)
	{
		if (!$this->attr('log')) return false;

		$contents = $this->getXML();

		$submission_date = substr(ts_iso8601(time()),0,10);

		$log_path = $this->data_path.'/submission_logs';		// log path

		if (!is_dir($log_path)) {	// doesn't exist = create it
			mkdir($log_path);
		}

		$filename_base = $log_path.'/log_';

		// if log id is set - only take it if the submission already exists
		if (!is_null($log_id)) {
			$filename = $filename_base.$log_id.'.xml';
			if (!file_exists($filename)) {	// file doesn't exist, we need a fresh ID
				$log_id = null;
			}
		}

		// run this always (instead of an 'else') because it may have been nulled out by prev block
		if (is_null($log_id)) {
			for($i=1;;$i++) {		// loop through possible files (to be replaced by attribute?)
			$filename = $filename_base.$i.'.xml';
				if (!file_exists($filename)) {	// file doesn't exist, sweet
					break;
				}
			}
		}

		return string_to_file($contents, $filename);

	}//end logSubmission()
	
	
	/**
	* Add valid keywords for this asset to an array of keywords when asked
	*
	* @param &object Asset	&$broadcaster	the asset that triggered the event
	* @param Array			$vars			the vars that get submitted by the broadcaster
	*										we add keywords to the $vars['keywords'] array
	*
	* @return boolean
	* @access private
	*/
	function onRequestKeywords(&$broadcaster, $vars=Array())
	{
		if (!isset($vars['keywords'])) return;

		$keywords = Array();
		// search for a bodycopies folder
		$folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', true, 'bodycopies_folder');

		$folder = &$GLOBALS['SQ_SYSTEM']->am->getAsset($folder_link['minorid'], $folder_link['minor_type_code']); 

		if (is_null($folder)) return false;

		// lets work out which bodycopy the WYSIWYG cell that wants our keywords
		// is in so we can return the correct keywords for the screen
		$bodycopy_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($folder->id, SQ_LINK_TYPE_2, 'bodycopy');
		$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id, 'bodycopy', true);

		$bodycopies = Array();
		foreach ($bodycopy_links as $link_info) {
			if (in_array($link_info['minorid'], $parents)) {
				$bodycopies[] = $link_info['value'];
			}
		}
		if (empty($bodycopies)) return;

		foreach ($bodycopies as $bodycopy) {
			if ($bodycopy == 'Page Contents') {
				// work out the query replacements
				foreach ($this->_available_keywords as $keyword) {
					$name = ucwords(str_replace('_', ' ', $keyword));
					$type = 'inline';
					if (array_search($keyword, Array('form_contents', 'form_errors')) !== 	false) {
						$type = 'block';
					}
					$default = $name;
					$keywords[$keyword] = Array('name' => $name, 'default' => $default, 'type' => $type);
				}	
			} else if ($bodycopy == 'Thank You') {
				$keywords['form_summary'] = Array('name' => 'Form Summary', 'default' => 'Summary of form results', 'type' => 'block');
			}


		}

		$vars['keywords'] = array_merge($vars['keywords'], $keywords);
		
	}//end onRequestKeywords()


}//end class
?>
