<?php

require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Form
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage cms
*/
class Form extends Asset
{


	/**
	* @var
	*/
	var $active_submission = '';

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

		// add a bodycopy to this page when creating
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$bodycopy = new Bodycopy();
		$copy_link = Array('asset' => &$this, 'value' => 'form_bodycopy', 'link_type' => SQ_LINK_TYPE_2, 'dependant' => 1, 'exclusive' => 1);

		$bodycopy->setAttrValue('name', 'Form Bodycopy');
		if (!$bodycopy->create($copy_link)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		// create a folder for the bodycopies
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');
		$folder = new Folder();
		$folder->setAttrValue('name', 'Bodycopies');

		// change the value on the link to reflect that of the folder
		$copy_link['value'] = 'bodycopies_folder';

		if (!$folder->create($copy_link)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}
		// create a link to the folder so we can put the bodycopyies in it
		$copy_link = Array('asset' => &$folder, 'link_type' => SQ_LINK_TYPE_2, 'dependant' => 1, 'exclusive' => 1);

		// create some bodycopies for the folder
		$bodycopies = Array(
			'Thank You Bodycopy', 
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
		$bodycopy_link = $this->getLinks(SQ_LINK_TYPE_2, 'bodycopy', true, 'major', '', '1', '1');
		if ($bodycopy_link[0]['linkid'] == $linkid && !$GLOBALS['SQ_PURGING_TRASH']) return 'You cant delete the link between the bodycopy and the form';

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
		$page_links[SQ_LINK_TYPE_2]['bodycopy'] = Array('card' => '1', 'exclusive' => true);
		$page_links[SQ_LINK_TYPE_2]['folder'] = Array('card' => 'M', 'exclusive' => true);
		$page_links[SQ_LINK_TYPE_2]['form_section'] = Array('card' => 'M', 'exclusive' => true);
		$page_links[SQ_LINK_TYPE_3]['form_submission'] = Array('card' => 'M', 'exclusive' => true);
		return $page_links;

	}//end _getAllowedLinks()


	/**
	* gets called when a link attached to this asset is moved
	*
	* @access public
	* @return boolean
	*/
	function moveLink()
	{
		parent::moveLink();
		return $this->fileRegeneration();
	
	} // moveLink()


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
			$this->tmp['section_links'] = $this->getLinks(SQ_LINK_TYPE_2, 'form_section', true, 'major', '', '1', '1');
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
		if ($a = &$am->getAsset($l['minorid'], $l['type_code'])) {
			return $a;
		}
		return false;
	}


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
			$sections[$link['sort_order']] = &$am->getAsset($link['minorid'], $link['type_code']);
		}
		return (!empty($sections)) ? $sections : Array();

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
		return count($this->getSectionLinks());

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
			
			// we need to create this $am variable for the php
			// in the include file to use!
			$am = &$GLOBALS['SQ_SYSTEM']->am;

			if (!file_exists($this->data_path.'/content_file.php')) {
				$edit = &$this->getEditFns();
				$edit->generateStandardContentFile($this);
			}
			include($this->data_path.'/content_file.php');
		}
		return true;
	}


	/**
	* prints the form if it is in wizard mode
	*
	* @param &object $submission the submission object
	*
	* @access public
	* @return boolean
	*/
	function printWizard() {
	
		if ($this->getStage() == 'first') {
			$order = 1;
		} else {
			// create a submission asset if we don't already have one
			if (!$this->active_submission) {
				$this->active_submission = &$this->createSubmission();
				
				// create a cookie and set the id to the id of the submission asset
				$this->setFormCookie($this->active_submission->id, $this->attr('cookie_time'));
			}
			$order = (int) $_POST['section'];
			
			// get the section to process itself
			$posted_section = &$this->getSectionByOrder($order);
			$posted_section->processForm($this->active_submission);

			// check to see if there is any errors with the submission,
			// otherwise increment to the next section
			if ($submission->isError()) {
				$this->printErrors();
			} else {
				$order++;
			}

		} // end if

		// set a hidden field of the current section being completed by the user
		hidden_field('section', $order);

		// check to see if the form has anymore sections to print
		if ($this->getSectionCount() >= $order) {
			// get the section that we are currently on
			// this is needed for the included file
			
			$section = &$this->getSectionByOrder($order);
			
			if (!file_exists($this->data_path.'/content_file.php')) {
				$edit = &$this->getEditFns();
				$edit->generateWizardContentFile($this);
			}
			include($this->data_path.'/content_file.php');
		// form is finished
		} else {
			$this->finishForm();
		}
	} // printWizard()


	/**
	* creates a new submission object
	*
	* @access public
	* @return &object $submission the submission object
	*/
	function &createSubmission()
	{
		inject_beef();
		$am = &$GLOBALS['SQ_SYSTEM']->am;

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('form_submission');
		
		// create some questions
		$submission = new Form_Submission();
		inject_beef();
		$copy_link  = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_3, 'dependant' => 1, 'exclusive' => 1);
		
		// give an appropriate name to this question
		if (!$submission->create($copy_link)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}
		inject_beef();
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		inject_beef();
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
		if (!isset($_POST['section']) || $_POST['section'] == 0) return 'first';
		// get the last section order completed by the user
		$section = $_POST['section'];
		$count = $this->getSectionCount();
		
		// if there is no validation screen and we are one screen away from it
		// then return last
		if ((!$this->attr('validate')) && ($section == ($count - 1))) return 'last';
		// if there is a validation screen and we are on the it, return last 
		if ($section == $count) return 'last';
		return $section;
	}


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
				if ($stage == 'last') return '';
			break;
			case 'back' :
				if ($stage == 'first') return '';
			break;
			case 'submit': 
				if ($stage != 'last') return '';
			break;
			default:
				return '';
		} // end switch

		// if there is no value set in the backend for this button,
		// just set the text to the type of button

		if (!$text = $this->attr($type.'_button')) {
			$text = $type;
		} 
	
		($type == 'back') 
			? $extras = 'var s = document.'.$this->getPrefix().'.section; s.value -= 2;'
			: $extras = '';
		
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
			$section->processForm($this->active_submission);
		}
		return true;
	} // end processForm()


	/**
	* 
	* 
	*
	*/
	function validateForm()
	{
		$validate_link = $this->getLink(SQ_LINK_TYPE_2, 'bodycopy', true, 'validate_bodycopy');
		$validate = &$am->getAsset($validate_link['minorid'], $validate_link['type_code']);
		hidden_value('validate', '1');
	}

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
		
		$folder_link = $this->getLink(SQ_LINK_TYPE_2, 'folder', true, 'bodycopies_folder');
		$folder = &$am->getAsset($folder_link['minorid'], $folder_link['type_code']); 
		
		$tyb_link = $folder->getLink(SQ_LINK_TYPE_2, 'bodycopy', true, 'thank_you_bodycopy');
		$tyb = &$am->getAsset($tyb_link['minorid'], $tyb_link['type_code']);
		
		if (!$tyb->isEmpty()) {
			$tyb->printBody();
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

	}


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
	}


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


}// end class
?>
