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
* $Id: form.inc,v 1.19 2004/04/26 23:30:19 lwright Exp $
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
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form($assetid=0) 
	{
		$this->_ser_attrs = true;
		$this->Bridge($assetid);

	} // end Form()


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

			$data['questionid'] = $questionid;

			$GLOBALS['SQ_SYSTEM']->am->includeAsset($data['question_type_code']);

			$type_code = $data['question_type_code'];

			// this eval returns the necessary object for the question type
			$asset = new $type_code($this->id, $data);
		}
		//die;

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
			$links = array(); //$this->getSectionLinks();

			// but we also need question shadow asset links
			$entries = $this->getQuestions();

//				die;

			$i = 0;

			foreach ($links as $link) {
				$i = max($i, $link['sort_order'] + 1);
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
		
		// this is so we get the sections as we go
		$children = Asset::getChildren($assetid, $type_code, $strict_type_code, $sort_by);

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
	* regenerates the content file for this form
	*
	* @access public
	* @return boolean
	*/
	function fileRegeneration()
	{
		$edit = $this->getEditFns();
		return $edit->generateStandardContentFile($this);

	} // end fileRegeneration()

	
	/**
	* get the question links attached to this form
	* 
	* @access public
	* @return array(link)
	*/
	function &getQuestions($force=false)
	{
		return $this->attr('questions');

	} // end getQuestions()
	
	
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

	} // end attachQuestion()
	
	
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

	} // end deleteQuestion()
	
	
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

	} // end getQuestionCount()
	
	
	/**
	* get a specific question
	* 
	* @access public
	* @return array(link)
	*/
	function getQuestionByID($questionid)
	{
		//if (empty($this->tmp['questions'])) {
		$questions = &$this->getQuestions();
		//}
		return (isset($questions[$questionid])) ? $questions[$questionid] : Array();

	} // end getQuestionById()
	
	
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

	} // end getQuestionByOrder()
	

	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$page_links = Array(SQ_LINK_TYPE_2 => Array(
						'form_section' => Array('card' => 'M', 'exclusive' => false)
						));
		return $page_links;

	} // end _getAllowedLinks()


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

	} // end getSections()

	
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

	} // end getSectionCount()


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

	} // end getSectionLinks()


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

	} // end setFormCookie() 


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
		<form name="<?php echo $prefix?>" enctype="multipart/form-data" action="<?php echo $_SERVER['PHP_SELF']?>" method="POST">
		<?php

		// check to see if there is a past submission that belongs to this user
		// check to see if this form is set up to be a wizard or not
		$this->printStandard();
		echo ?></form><?php

	} // end printBody()


	/**
	* prints the standard form i.e not a wizard
	*
	* @param &object $submission the submission object
	* 
	* @access public
	* @return boolean
	*/
	function printStandard() {
		
		$prefix = $this->getPrefix();
		$is_error = false;

		// check to see if the form has been submitted or not
		if (isset($_POST[$prefix.'_submit'])) {
			/*if (!$this->active_submission) {
				$this->active_submission = &$this->createSubmission();
				$this->setFormCookie($this->active_submission->id, $this->attr('cookie_time'));
			}*/
			
			// flush any errors that exist in the submisssion asset
			//$this->active_submission->flushErrors();
			$this->processForm();
			
			// check to see if there is an error with the form
			if (count($this->submission_errors)) { 
				$this->printErrors();
				$is_error = true;
			} else {
				$this->finishForm();
			}
		}

		// nothing submitted yet, or there was an error
		if (!isset($_POST[$prefix.'_submit']) || $is_error) {
			$this->includeContentFile('Standard');
		}
		return true;

	} // end printSummary()


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
			//$posted_section->active_form =& $this;
		}
		
		return $posted_section->processForm();

	} // end  processSection()


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

	} // end includeContentFile()
	

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

	} // end processForm()


	/**
	* prints the validatin bodycopy
	* 
	* @access public
	* @return boolean
	*/
	function validateForm()
	{
		$validate = &$this->getBodycopy('validate');
		if (!is_null($validate)) $validate->printBody();

		// print a summary of the answers
		$this->printSummary(true);
		
		// set a hidden value so that we know that we have already validated the details
	
	} // end validateForm()


	/**
	* function to finish off form 
	*
	* @access public
	* @return boolean
	*/
	function finishForm() {
		
		$am = &$GLOBALS['SQ_SYSTEM']->am;

		// we want delete the cookie, so that when they come back to fill
		// this form on another day, they start fresh, so set the number of days to -1
		//$this->setFormCookie(0, -1);
		
		/*$tyb = &$this->getBodycopy('thank_you');
		
		if (!$tyb->isEmpty()) {
			$tyb->printBody();
		}*/

		?>
		<SPAN style="color: red">
		<B>Thank you, here are your responses</B><BR/>
		</SPAN>
		<?php

		// print the results if we need to 
		if ($this->attr('show_results')) $this->printSummary(true);

		// if we are not logging submissions, then we want to remove the submission asset
		// now that we have finished with it

		/*if (!$this->attr('log_submissions')) {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->active_submission->id, SQ_LINK_TYPE_3, 'form', false, '', 'minor');
			$this->deleteLink($link['linkid']);
		}*/

	} // end finishForm()


	/**
	* prints the errors that occured in a form submission
	*
	* @access public
	* @return boolean
	*/
	function printErrors()
	{
		//if (!$this->active_submission) return false;

		if(empty($this->submission_errors)) return false;

		?>
		<SPAN style="color: red">
		<B>The Following Errors Occured</B><BR/>
		<UL>
		<?php

		foreach ($this->submission_errors as $error) {
			echo "<LI>".$error."</LI>\n";
		}

		?>
		</UL>
		</SPAN>
		<?php
		
		return true;

	} // end printErrors()


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
		//$summary = $this->active_submission->getSummary();
		//if (empty($summary)) return false;

		echo ($html) ? '<table>' : '';
		/*foreach ($summary as $id => $s_info) {
			echo ($html) ? '<tr><td colspan="2">' : '';
			echo $s_info['section_name'];
			echo ($html) ? '</td></tr>' : '\n\n';
			foreach ($s_info['questions'] as $id => $q_info) {
				echo ($html) ? '<tr><td><li>' : '';
				echo $q_info['name'];
				echo ($html) ? ': </li></td><td>' : ' : ';
				echo $q_info['answer'];
				echo ($html) ? '</td></tr>' : '\n';
			}
		}*/

		// unattached questions first
		foreach($this->active_submission as $k => $v) {
			
			// check to see whether this belongs to us (if it isn't then perhaps a section took care)
			if ($this->id == ($asset_id = strtok($k, ':'))) {
				$question = &$GLOBALS['SQ_SYSTEM']->am->getAsset($k);
				echo ($html) ? '<tr><td><li>' : '';
				echo $question->attr('name');
				echo ($html) ? ': </li></td><td>' : ' : ';
				echo $question->getSummary($this->active_submission[$k]);
				echo ($html) ? '</td></tr>' : "\n";
			}

		}

		// now sections
		$sections = &$this->getSections();
		for (reset($sections); null !== ($k = key($sections)); next($sections)) {
			$section =& $sections[$k];
			echo ($html) ? '<tr><td colspan="2">' : '';
			echo $section->attr('name');
			echo ($html) ? '</td></tr>' : "\n\n";

			foreach($this->active_submission as $k => $v) {
			
				// check to see whether this belongs to the section
				if ($section->id == ($asset_id = strtok($k, ':'))) {
					$question = &$GLOBALS['SQ_SYSTEM']->am->getAsset($k);
					echo ($html) ? '<tr><td><li>' : '';
					echo $question->attr('name');
					echo ($html) ? ': </li></td><td>' : ' : ';
					echo $this->active_submission[$k];
					echo ($html) ? '</td></tr>' : "\n";
				}

			}
			echo ($html) ? '' : "\n";
		}

		echo ($html) ? '</table>' : '';
	
	} // end printSummary()


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

	} // end getFormInfo()


	/**
	* returns the section index for a section
	*
	* @param $order the order of the section (if ommited, the posted order is used)
	*
	* @access public
	* @return string
	*/
	function getSectionIndex($order = '')
	{
		$index = $this->attr('section_index');
		if (!$index) return '';
		
		if (!$order) {
			$order = (isset($_POST['section'])) ? $_POST['section'] + 1 : 1;
		}

		switch ($index) {
			// numbers
			case '1': 		
				return $order.'.';	
			break;
			
			// lowercase letters
			case '2': 
				$i = 96 + $order;
				return chr($i).'.';
			break;
			
			// uppercase letters
			case '3': 	
				$i = 64 + $order;
				return chr($i).'.';	
			break;
			
			// lowercase Roman Numerals
			case '4': 
				return strtolower(int_to_roman($order)).'.';
			break;
			
			// uppercase Roman Numerals
			case '5': 
				return int_to_roman($order).'.';
			break;

		} // end switch

	} // end getSectionIndex()
	

} // end class
?>
