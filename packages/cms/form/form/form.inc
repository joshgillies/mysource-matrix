<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: form.inc,v 1.11 2003/09/26 06:01:26 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';
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
class Form extends Asset
{

	/**
	* the active submission object
	* @var object 
	*/
	var $active_submission = null;

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
		$this->Asset($assetid);
	
	} // end Form()


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

		// create a form asset and some other assets in this stucture
		//
		// |- form
		//    |- folder
		//       |- bodycopy
		//		 |- bodycopy

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$linkid = parent::create($link);
		if (!$linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		// create a folder for the bodycopies
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');
		$folder = new Folder();
		$folder->setAttrValue('name', 'Bodycopies');

		$copy_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'value' => 'bodycopies_folder', 'dependant' => 1, 'exclusive' => 1);

		if (!$folder->create($copy_link)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		// create a link to the folder so we can put the bodycopyies in it
		$copy_link = Array('asset' => &$folder, 'link_type' => SQ_LINK_TYPE_2, 'dependant' => 1, 'exclusive' => 1);

		// add a bodycopy to this page when creating
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');

		// create some bodycopies for the folder
		$bodycopies = Array(
			'Format',
			'Thank You', 
			'Validate',
		);

		foreach ($bodycopies as $name) {
			$bodycopy = new Bodycopy();
			$bodycopy->setAttrValue('name', $name);
			
			// create an appropriate value for this link
			// to reflect that of the name
			$name = strtolower(str_replace(' ', '_', $name));
			$copy_link['value'] = $name;

			if (!$bodycopy->create($copy_link)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$this->id = 0;
				return false;
			}
		} // end foreach

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
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_TYPE_2]['bodycopy'] = Array('card' => '1', 'exclusive' => true);
		$page_links[SQ_LINK_TYPE_2]['folder'] = Array('card' => 'M', 'exclusive' => true);
		$page_links[SQ_LINK_TYPE_2]['form_section'] = Array('card' => 'M', 'exclusive' => true);
		$page_links[SQ_LINK_TYPE_3]['form_submission'] = Array('card' => 'M', 'exclusive' => true);
		return $page_links;

	} // end _getAllowedLinks()


	/**
	* regenerates the content file for this form
	*
	* @access public
	* @return boolean
	*/
	function fileRegeneration()
	{
		$edit = $this->getEditFns();
		if ($this->attr('wizard')) {
			return $edit->generateWizardContentFile($this);
		} else {
			return $edit->generateStandardContentFile($this);
		}
	} // end fileRegeneration()


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
			$this->tmp['section_links'] = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'form_section', true, 'major', '', '1', '1');
		}
		return (!empty($this->tmp['section_links'])) ? $this->tmp['section_links'] : Array();

	} // end getSectionLinks()


	/**
	* returns a section coresponding to the order it appears in the flash
	*
	* @param &object $asset the form asset
	* @param integer $order the order of the section
	*
	* @access public
	* @return &object section asset
	*/
	function &getSectionByOrder($order)
	{
		$links = $this->getSectionLinks();
		$l = $links[$order - 1];
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		if ($a = &$am->getAsset($l['minorid'], $l['minor_type_code'])) {
			return $a;
		}
		return false;
	
	} // end getSectionByOrder()


	/**
	* returns a question asset based on the section order and question order
	*
	* @param &object $asset the form asset
	* @param integer $s_order the section order
	* @param integer $q_order the question order 
	*
	* @access public
	* @return &object asset the question asset
	*/
	function &getQuestionByOrder($s_order, $q_order)
	{
		// first get the section asset that contains the question
		if (!$section = &$this->getSectionByOrder($s_order)) return false;
		// get the question from the section based on the order
		if (!$question = &$section->getQuestionByOrder($q_order)) return false;
		
		return $question; 
	
	} // getQuestionByOrder()
	

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
		speed_check();
		$prefix = $this->getPrefix(); 
		$submission = '';

		?>
		<form name="<?php echo $prefix?>" enctype="multipart/form-data" action="<?php echo $_SERVER['PHP_SELF']?>" method="POST">
		<?php

		// check to see if there is a past submission that belongs to this user
		if (isset($_COOKIE['SQ_FORM_'.$prefix])) {
			$submissionid = $_COOKIE['SQ_FORM_'.$prefix];
			if ($submissionid) {
				$this->active_submission = &$GLOBALS['SQ_SYSTEM']->am->getAsset($submissionid, 'form_submission');
			}
		}
		// check to see if this form is set up to be a wizard or not
		if ($this->attr('wizard')) {
			$this->printWizard();
		} else {
			$this->printStandard();
		}
		echo "</form>\n";
		speed_check();

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
			if (!$this->active_submission) {
				$this->active_submission = &$this->createSubmission();
				$this->setFormCookie($this->active_submission->id, $this->attr('cookie_time'));
			}
			
			// flush any errors that exist in the submisssion asset
			$this->active_submission->flushErrors();
			$this->processForm();
			
			// check to see if there is an error with the form
			if ($this->active_submission->isError()) { 
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
	* prints the form if it is in wizard mode
	*
	* @param &object $submission the submission object
	*
	* @access public
	* @return boolean
	*/
	function printWizard() {
	
		switch ($this->getStage()) {
			
			case 'first':
			
				$order = 1;
				break;
			
			case 'validate':
					
				if (!isset($_POST['section'])) return false;
		
				// we need to process the section before printing the validation details
				$this->processSection($_POST['section']);
				$this->includeContentFile();
				$this->saveStage($_POST['section'] + 1);
					
				return true;
			
			case 'finish':
					
				if (!isset($_POST['section'])) return false;
				
				// only process the section if the validate case hasn't already done so
				if (!$this->attr('validate')) $this->processSection($_POST['section']);
				
				if ($this->active_submission->isError()) {
					$this->printErrors();
					$order = $this->getSectionCount();
					break;
				} else {
					$this->finishForm();
					return true;
				}
		
			default :

				// create a submission asset if we don't already have one
				if (!$this->active_submission) {
					$this->active_submission = &$this->createSubmission();
					
					// create a cookie and set the id to the id of the submission asset
					$this->setFormCookie($this->active_submission->id, $this->attr('cookie_time'));
				}
				$order = isset($_POST['section']) ? $_POST['section'] : 1;
				
				$this->processSection($order);
				// check to see if there is any errors with the submission,
				// otherwise increment to the next section
				
				if ($this->active_submission->isError()) {
					$this->printErrors();
				} else {
					$order++;
				}

		} // end switch

		// get the section that we are currently on
		// this is needed for the included file	
		$this->active_section = &$this->getSectionByOrder($order);
		
		// tell the section what the current submission object is
		$this->active_section->active_submisssion =& $this->active_submission;
		
		// let the section know what its question index is
		$this->active_section->question_index = $this->attr('question_index');

		// set a hidden field of the current section being completed by the user
		$this->saveStage($order);

		// include the content file for this form
		$this->includeContentFile('Wizard');

	} // end printWizard()


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
			// let this section know what the current submission object is
			$posted_section->active_submission =& $this->active_submission;
		}
		
		return $posted_section->processForm($this->active_submission);

	} // end  processSection()


	/**
	* saves the current stage that we are up to in a wizard
	*
	* @param string $stage the stage to save
	* 
	* @access public
	* @return boolean
	*/
	function saveStage($stage)
	{
		return hidden_field('section', $stage);
	
	} // end saveStage()


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
	* creates a new submission object
	*
	* @access public
	* @return &object $submission the submission object
	*/
	function &createSubmission()
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('form_submission');
		
		// create some questions
		$submission = new Form_Submission();
		$copy_link  = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_3, 'dependant' => 1, 'exclusive' => 1);
		
		// give an appropriate name to this question
		if (!$submission->create($copy_link)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return $submission;

	} // end createSubmission()


	/**
	* gets the current stage that a wizard form is up to
	*
	* @access public
	* @return string stage
	*/
	function getStage()
	{
		$section = isset($_POST['section']) ? $_POST['section'] : 0;
		
		// if there is an error, then we are not moving forward at all, 
		// so take one off the section
		if (isset($this->active_submission) && $this->active_submission->isError()) {
			$section -= 1;
		}
		$count = $this->getSectionCount();

		if (!$section) return 'first';
		if (($section == $count) && ($this->attr('validate'))) return 'validate';
		
		// if there is no validation screen and we are one screen away from the finish
		// then return last
		if ((!$this->attr('validate')) && ($section == ($count - 1))) return 'last';
		
		// if there is a validation screen and we are on the it, return last 
		if ($section >= $count) return 'finish';
		
		return $section;
	
	} // end getStage()


	/**
	* get a button for a wizard based on where the wizard is up to
	* this function is called by the included content_file.php
	*
	* @param &object form	$asset the for asset
	* @param string $type	the type of button
	*
	* @access public
	* @return string 
	*/
	function getButton($type)
	{
		$stage = $this->getStage();

		switch ($type) {
			case 'next':
				if ($stage == 'last' || $stage == 'validate') return '';
			break;
			case 'back' :
				if ($stage == 'first') return '';
			break;
			case 'submit': 
				if ($this->attr('validate')) {
					if ($stage != 'validate') return '';
				} else {
					if ($stage != 'last') return '';
				}
			break;
			default:
				return '';
		} // end switch

		// if there is no value set in the backend for this button,
		// just set the text to the type of button

		if (!$text = $this->attr($type.'_button')) {
			$text = $type;
		} 
	
		$extras = ($type == 'back') ? 'var s = document.'.$this->getPrefix().'.section; s.value -= 2;' : '';
		
		ob_start();
		submit_button($this->getPrefix().'_'.$type, $text, $extras);
		$html = ob_get_contents();
		ob_end_clean();

		return $html;

	} // getButton()


	/**
	* processes the form (non-wizard)
	*
	* @param &object $submission the submisssion object
	*
	* @access public
	* @return boolean
	*/
	function processForm() {
	
		if (!$this->active_submission) return false;

		$sections = &$this->getSections();
		for (reset($sections); null !== ($k = key($sections)); next($sections)) {
			$section =& $sections[$k];
			$section->active_submission =& $this->active_submission;
			$section->processForm();
		}
		return true;

	} // end processForm()


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
		if (empty($folder_link)) return false;
		
		$folder = &$am->getAsset($folder_link['minorid'], $folder_link['minor_type_code']); 
		if (is_null($folder)) return false;

		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', true, $name);
		if (empty($bodycopy_link)) return false;

		$bodycopy = &$am->getAsset($bodycopy_link['minorid'], $bodycopy_link['minor_type_code']);
		
		return ($bodycopy) ? $bodycopy : false;
	
	} // end getBodycopy()


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
		$this->setFormCookie(0, -1);
		
		$tyb = &$this->getBodycopy('thank_you');
		
		if (!$tyb->isEmpty()) {
			$tyb->printBody();
		}

		// print the results if we need to 
		if ($this->attr('show_results')) $this->printSummary(true);

		// if we are not logging submissions, then we want to remove the submission asset
		// now that we have finished with it

		if (!$this->attr('log_submissions')) {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->active_submission->id, SQ_LINK_TYPE_3, 'form', false, '', 'minor');
			$this->deleteLink($link['linkid']);
		}

	} // end finishForm()


	/**
	* prints the errors that occured in a form submission
	*
	* @access public
	* @return boolean
	*/
	function printErrors()
	{
		if (!$this->active_submission) return false;

		$errors = $this->active_submission->getErrors();
		if(empty($errors)) return false;

		?>
		<SPAN style="color: red">
		<B>The Following Errors Occured</B><BR/>
		<UL>
		<?php

		foreach ($errors as $error) {
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
		$summary = $this->active_submission->getSummary();
		if (empty($summary)) return false;

		echo ($html) ? '<table>' : '';
		foreach ($summary as $id => $s_info) {
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
	* returns the info corresponding to the value associated with a question
	*
	* @param string $info the wanted info
	*
	* @return mixed
	* @access public
	*/
	function getSpecialInformation($info)
	{
		$i = $this->attr('special_info');
		if (isset($i[$info]['id'])) {
			return $this->active_submission->getAnswer($i[$info]['id']);
		}
		return false;
	
	} // end getSpecialInformation()


	/**
	* set the info corresponding to the value associated with a question
	*
	* @param integer $questionid		the questionid you want to accociate the info with
	* @param stribng $info				the special information
	*
	* @return mixed
	* @access public
	*/
	function setSpecialInformation($questionid, $info)
	{
		$i = $this->attr('special_info');
		
		// there can only be one question with this special information value
		if (isset($i[$info]) && ($i[$info]['id'] != $questionid)) {
			trigger_error('A Question has already been assigned to that Special Information (Asset id #'.$i[$info]['id'].')', E_USER_WARNING);
			return false;
		}

		// check to see if this question has been assigned to something else
		foreach ($i as $name => $i) {
			if ($i['id'] == $questionid) {
				unset($i[$name]); 
				unset($i['id']);
				break;
			}
		}

		// set the value to the question id
		$i[$info]['id'] = $questionid;
		if (!$this->setAttrValue('special_info', $i)) {
			return false;
		}

		return true;

	} // end setSpecialInformation()


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
