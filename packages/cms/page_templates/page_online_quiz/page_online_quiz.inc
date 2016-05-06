<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: page_online_quiz.inc,v 1.47 2013/09/10 07:30:36 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_asset_listing/page_asset_listing.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';


define('SQ_ONLINE_QUIZ_SESSION_VAR',   'SQ_ONLINE_QUIZ');
define('SQ_ONLINE_QUIZ_FLAG_COMPLETE', 'is_complete');


/**
* Page_Online_Quiz
*
* Purpose
* 	A customisable online quiz
*
* Bodycopies
* 	- page_contents
* 	- no_results
* 	- user_details
* 	- thank_you
*	- already_taken
*
* Forms
* 	- user_details
*
* General notes
* 	- "question pool": the group of question assets that can be used in the quiz,
* 					   determined by the type, status and root-node preferences
*
* 	- A-Z listing is currently unsupported. Everything except for
* 	  getCurrentPage() is currently implemented, however implementing
* 	  this function without significant wastage of resources trying to
* 	  find out the current page has proven to be exceedingly difficult
*
* 	- The functions in the "Listing Functions" section (eg. getPageAssets())
* 	  are what remains of an attempt to break out the listing_engine pagination
* 	  process into seperate functions - A-Z listing, however, prevented this
* 	  being done in a generic fashion, hence the functions now live here
*
* 	- The following settings are currently "hidden" from the user
* 	  (with defaults being set in the management file):
* 		- Types
* 		- List Format (A-Z removed from the dropdown)
*
*
* @author  Robert Howard <rhoward@squiz.net>
* @author  Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.47 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Online_Quiz extends Page_Asset_Listing
{


	/**
	* @var array	$_bodycopies	a list of the bodycopies attached to this page, in the format: Array('bodycopy_code' => 'Bodycopy Name')
	* @access private
	*/
	var $_bodycopies;


	/**
	* @var array	$_error_stack	a list of warnings and errors populated during processQuestions(), for use by the keyword replacement functions to display errors to the user
	* @access private
	*/
	var $_error_stack = Array(
							'errors'	=> Array(),
							'warnings'	=> Array(),
						);


	/**
	* Constructor
	*
	* @param string	$assetid	the assetid of the asset to load into this object
	*
	* @return void
	* @access public
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

		$this->_bodycopies = Array(
								'page_contents'	=> translate('Page Contents'),

								'no_results'	=> translate('Page Contents (No Results)'),

								'thank_you'		=> translate('Thank You Bodycopy'),

								'already_taken'	=> translate('Already Taken Bodycopy'),

								'user_details'	=> translate('User Details Bodycopy'),

							 );

	}//end constructor


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();

		$page_links[SQ_LINK_TYPE_1]['folder']						= Array('card' => 'M');
		$page_links[SQ_LINK_TYPE_1]['online_quiz_question']			= Array('card' => 'M');
		$page_links[SQ_LINK_TYPE_1]['online_quiz_question_group'] 	= Array('card' => 'M');
		$page_links[SQ_LINK_TYPE_2]['bodycopy']             		= Array('card' => 'M');
		$page_links[SQ_LINK_TYPE_3]['bodycopy']             		= Array('card' => 'M');
		$page_links[SQ_LINK_TYPE_2]['simple_form']          		= Array('card' => 'M');
		$page_links[SQ_LINK_TYPE_3]['simple_form']          		= Array('card' => 'M');
		$page_links[SQ_LINK_NOTICE]['user']                 		= Array('card' => 'M');

		return $page_links;

	}//end _getAllowedLinks()


//--       CREATION       --//


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Asset listings create bopycopy and folder asset when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		// create the user_details form (to go with the matching bodycopy)
		if (!$this->createForm()) return FALSE;
		if (!$this->_createSubmissionsFolder()) return FALSE;

		// create the type
		return $this->createTypeFormat('online_quiz_question');

	}//end _createAdditional()


	/**
	* Creates bodycopies for this asset
	*
	* Overridden, as we need an extra Thank You bodycopy
	*
	* @return boolean
	* @access protected
	*/
	function _createBodycopies()
	{
		$bodycopies = $this->_bodycopies;

		// leave out no_results for this - it's created later
		unset($bodycopies['no_results']);

		// only create the other bodycopies if they're needed at the moment
		if ($this->attr('after_submit_action') != 'thank_you') {
			unset($bodycopies['thank_you']);
		}
		if ($this->attr('can_retake')) {
			unset($bodycopies['already_taken']);
		}


		foreach ($bodycopies as $bodycopy_code => $bodycopy_name) {
			if (!$this->createBodycopy($bodycopy_code)) {
				return FALSE;
			}
		}

		if (!$this->createNoResultsBodycopy(TRUE)) {
			return FALSE;
		}

		return TRUE;

	}//end _createBodycopies()


	/**
	* Create the folder for storing submissions
	*
	* @return boolean
	* @access protected
	*/
	function _createSubmissionsFolder()
	{
		$existing = $this->getSubmissionsFolder();
		if (!empty($existing)) return TRUE;
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');
		$folder = new Folder();
		$folder->setAttrValue('name', translate('Submissions'));

		$folder_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'value' => 'submissions_folder', 'is_dependant' => 1, 'is_exclusive' =>      1);
		if (!$folder->create($folder_link)) {
			trigger_localised_error('CMS0070', translate('Cannot create form - problem creating submissions folder'), E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// Deny public read access to the submissions folder by default
		if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($folder->id, 'permissions')) {
			return FALSE;
		}
		if (!$GLOBALS['SQ_SYSTEM']->am->setPermission($folder->id, $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user'), SQ_PERMISSION_READ, 0, TRUE, TRUE)) {
				return FALSE;
		}
		$GLOBALS['SQ_SYSTEM']->am->releaseLock($folder->id, 'permissions');
		return TRUE;

	}//end _createSubmissionsFolder()


	/**
	* Work out the unique cache key to represent the current page
	* Overridden to ensure that the Online Quiz asset is not cached
	*
	* @return string
	* @access private
	*/
	function _getCacheKey()
	{
		return '';

	}//end _getCacheKey()


	/**
	* Creates a new submission object
	*
	* @return object
	* @access public
	*/
	function &createSubmission()
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('online_quiz_submission');

		// create the form submission asset
		$submission = new Online_Quiz_Submission();
		$submissions_folder = $this->getSubmissionsFolder();
		if (is_null($submissions_folder)) {
			trigger_localised_error('CMS0069', translate('Cannot create form submission - submissions folder not found.  Upgrade may be required.'), E_USER_WARNING);
			$null = NULL;
			return $null;
		}
		$copy_link  = Array('asset' => &$submissions_folder, 'link_type' => SQ_LINK_TYPE_3, 'is_dependant' => 0, 'is_exclusive' => 0);

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
	* Get the submissions folder
	*
	* @return object
	* @access public
	*/
	function &getSubmissionsFolder()
	{
		$res = NULL;
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, 'submissions_folder');
		if (!empty($link)) {
			$res = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], 'folder');
		}
		return $res;

	}//end getSubmissionsFolder()


	/**
	* Get the default content for a bodycopy
	*
	* @param string	$bodycopy_code	The code name for the bodycopy
	*
	* @return string
	* @access private
	*/
	function _getDefaultBodycopyContent($bodycopy_code)
	{
		$content = '';

		switch ($bodycopy_code) {
			// regular bodycopies
			case 'page_contents':
				$content  = '%asset_listing% %submit_button%';
			break;
			case 'no_results':
				return '<p>'.translate('There are no quiz questions to display.').'</p>';

			break;
			case 'thank_you':
				$content  = '<p>'.translate('Thank you for taking the quiz; your results are displayed below:').'</p>'."\n";

				$content .= '%results_table_full%';
			break;
			case 'user_details':
				$content  = '<p>'.translate('Please fill out your details below:').'</p>'."\n";

				$content .= '%form_contents%';
			break;
			case 'already_taken':
				$content = '<p>'.translate('You\'ve already taken this quiz, and cannot take it again.').'</p>';

			break;

			// type formats
			case 'online_quiz_question':
				$content .= '<div>%question_text%</div>'."\n";
				$content .= '%response_form%';
			break;
		}

		if (!$content) {
			$content = parent::_getDefaultBodycopyContent($bodycopy_code);
		}

		return $content;

	}//end _getDefaultBodycopyContent()


//--        Bodycopy-handling       --//


	/**
	* Creates a bodycopy
	*
	* @param string		$bodycopy_code		the code that indicates which bodycopy to manipulate (see $_bodycopies)
	* @param boolean	$enable_on_create	if TRUE, will set the bodycopy to be enabled (TYPE_2 link) when it is created. If FALSE, it creates it disabled (TYPE_3 link).
	*
	* @return boolean
	* @access public
	*/
	function createBodycopy($bodycopy_code, $enable_on_create=TRUE)
	{
		$bodycopy_link = $this->getBodycopyLink($bodycopy_code, SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3);

		// we already have a bodycopy link: bail out
		if ($bodycopy_link) {
			return FALSE;
		} else {
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');

			$link_type =  ($enable_on_create ? SQ_LINK_TYPE_2 : SQ_LINK_TYPE_3);

			$asset = new Bodycopy();
			$copy_link =  Array(
							'asset'			=> &$this,
							'value'			=> $bodycopy_code,
							'link_type'		=> $link_type,
							'is_dependant'	=> 1,
							'is_exclusive'	=> 1,
						  );

			$asset->setAttrValue('name', $this->_bodycopies[$bodycopy_code]);
			$args = Array(
						'content'	=> $this->_getDefaultBodycopyContent($bodycopy_code),
					);

			if (!$asset->create($copy_link, $args)) return FALSE;
			unset($asset);
		}

		return TRUE;

	}//end createBodycopy()


	/**
	* Returns the a bodycopy
	*
	* @param string		$bodycopy_code		the code that indicates which bodycopy to manipulate (see $_bodycopies)
	* @param boolean	$only_if_enabled	if TRUE, will return NULL if the bodycopy is
	*										disabled. If FALSE, will only return NULL if
	*										never created.
	*
	* @return mixed object|NULL
	* @access public
	*/
	function &getBodycopy($bodycopy_code, $only_if_enabled=TRUE)
	{
		$asset = NULL;

		if ($only_if_enabled) {
			$link_types = SQ_LINK_TYPE_2;
		} else {
			$link_types = SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3;
		}

		$bodycopy_link = $this->getBodycopyLink($bodycopy_code, $link_types);
		if ($bodycopy_link) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link['minorid'], 'bodycopy');
		}

		return $asset;

	}//end getBodycopy()


	/**
	* Returns the link to a bodycopy
	*
	* @param string	$bodycopy_code	the code that indicates which bodycopy to manipulate (see $_bodycopies)
	* @param int	$link_type		ensures the link returned is of this type (eg. SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3)
	*
	* @return array
	* @access public
	*/
	function getBodycopyLink($bodycopy_code, $link_type)
	{
		$tmp_bodycopy = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, $link_type, 'bodycopy', FALSE, 'major', $bodycopy_code);
		return reset($tmp_bodycopy);

	}//end getBodycopyLink()


	/**
	* Handles the enabling or disabling of a bodycopy
	*
	* @param string		$bodycopy_code	the code that indicates which bodycopy to manipulate (see $_bodycopies)
	* @param boolean	$enabled		set to TRUE to enable the bodycopy, and FALSE to disable it
	*
	* @return boolean
	* @access public
	*/
	function setBodycopy($bodycopy_code, $enabled)
	{
		$bodycopy_link = $this->getBodycopyLink($bodycopy_code, SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3);

		if (!$bodycopy_link) {
			// no bodycopy yet? If we're trying to set to disabled, then we don't
			// need to do anything - if not then we need to create it
			if ($enabled) {
				if (!$this->createBodycopy($bodycopy_code)) {
					return FALSE;
				}
			}
		} else {
			// set link type to either TYPE_2 if enabled or TYPE_3 if disabled
			$new_link_type = ($enabled) ? SQ_LINK_TYPE_2 : SQ_LINK_TYPE_3;
			if ($bodycopy_link['link_type'] != $new_link_type) {
				$GLOBALS['SQ_SYSTEM']->am->updateLink($bodycopy_link['linkid'], $new_link_type);
			}
		}

		return TRUE;

	}//end setBodycopy()


	/**
	* Called by the design to print the body of this asset
	*
	* @return boolean
	* @access public
	*/
	function printBody()
	{
		if ($this->getFlag(SQ_ONLINE_QUIZ_FLAG_COMPLETE) && !$this->attr('can_retake')) {
			$this->printBodycopy('already_taken');
		} else if ($this->attr('interactive_mode')){
			$this->printContents();
		} else {
			parent::printBody();
		}

		return TRUE;

	}//end printBody()


	/**
	* Prints a bodycopy (if available)
	*
	* @param string	$bodycopy_code	the code that indicates which bodycopy to manipulate (see $_bodycopies)
	*
	* @return boolean
	* @access public
	*/
	function printBodycopy($bodycopy_code)
	{
		$bodycopy_printed =  FALSE;
		$bodycopy         = $this->getBodycopy($bodycopy_code);

		if (!is_null($bodycopy)) {
			// keywords
			require_once SQ_FUDGE_PATH.'/general/text.inc';
			$keywords = $bodycopy->getKeywords();
			$replacements = Array();
			foreach ($keywords as $keyword) {
				$replacements[$keyword] = $this->getKeywordReplacement($keyword);
			}
			$bodycopy->setKeywordReplacements($replacements);

			$bodycopy->printBody();
			$bodycopy_printed = TRUE;
		}

		return $bodycopy_printed;

	}//end printBodycopy()


//--        Form-manipulation        --//


	/**
	* Get the user_details simple form linked to this asset, if any
	*
	* @return int
	* @access private
	*/
	function &getForm()
	{
		$form = NULL;

		$form_link = $this->getFormLink(SQ_LINK_TYPE_2);
		if (!empty($form_link['minorid'])) {
			$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($form_link['minorid'], 'simple_form');
		}

		return $form;

	}//end getForm()


	/**
	* Returns the link to a form
	*
	* @param int	$link_type	ensures the link returned is of this type (eg. SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3)
	*
	* @return array
	* @access public
	*/
	function getFormLink($link_type)
	{
		$tmp_form = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, $link_type, 'simple_form', FALSE, 'major', 'user_details');
		return reset($tmp_form);

	}//end getFormLink()


	/**
	* Add a simple form linked under this asset
	*
	* @param int	$link_type	specifies which type of link the new form should be created with
	*
	* @return boolean
	* @access public
	*/
	function createForm($link_type=SQ_LINK_TYPE_2)
	{
		$form_link = $this->getFormLink(SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3);
		if (empty($form_link)) {
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('simple_form');
			$form = new Simple_Form();
			$form->setAttrValue('name', translate('User Details Form'));

			$link = Array(
						'asset'			=> &$this,
						'link_type'		=> $link_type,
						'value'			=> 'user_details',
						'is_dependant'	=> 1,
						'is_exclusive'	=> 1,
					);
			if (!$form->create($link)) return FALSE;
		}

		return TRUE;

	}//end createForm()


	/**
	* Handles the enabling or disabling of a bodycopy
	*
	* @param boolean	$enabled	set to TRUE to enable the bodycopy, and FALSE to disable it
	*
	* @return boolean
	* @access public
	*/
	function setForm($enabled)
	{
		$form_link = $this->getFormLink(SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3);

		if (empty($form_link)) {
			// no bodycopy yet? If we're trying to set to disabled, then we don't
			// need to do anything - if not then we need to create it
			if (($enabled) && (!$this->createForm())) {
				return FALSE;
			}
		} else {
			// set link type to either TYPE_2 if enabled or TYPE_3 if disabled
			$new_link_type = ($enabled) ? SQ_LINK_TYPE_2 : SQ_LINK_TYPE_3;
			if ($form_link['link_type'] != $new_link_type) {
				return $GLOBALS['SQ_SYSTEM']->am->updateLink($form_link['linkid'], $new_link_type);
			}
		}

		return TRUE;

	}//end setForm()


	/**
	* Get the replacement for the form_contents keyword
	*
	* @return string
	* @access public
	*/
	function getFormContentsKeywordReplacement()
	{
		$form = $this->getForm();
		if (!is_null($form)) {
			ob_start();
				$form->printBody(TRUE, TRUE, current_url(TRUE, TRUE, FALSE).'?'.$this->id.'_formid='.$form->id);
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($form);
			return ob_get_clean();
		}

		return '';

	}//end getFormContentsKeywordReplacement()


	/**
	* Get the replacement for the form_errors keyword
	*
	* @return string
	* @access public
	*/
	function getFormErrorsKeywordReplacement()
	{
		if (!empty($this->_tmp['form_errors'])) {
			ob_start();
			echo '<ul>';
			foreach ($this->_tmp['form_errors'] as $error) {
				echo '<li>';
				echo $error;
				echo '</li>';
			}
			echo '</ul>';
			return ob_get_clean();
		}

		return '';

	}//end getFormErrorsKeywordReplacement()


	//-- Type-Format Auto-creation --//


	/**
	* Creates a new type format for the type_code specified by $new_type
	*
	* @param string	$new_type	the type_code of the type format to create
	*
	* @return boolean
	* @access public
	*/
	function createTypeFormat($new_type)
	{
		$edit_fns = $this->getEditFns();
		$folder   = $this->getFolder('type_formats');
		$types    =  $edit_fns->_getAvailableTypes($this);

		// check that the new type is valid
		if ($new_type != '' && !isset($types[$new_type])) {
			trigger_localised_error('CMS0045', sprintf(translate('Cannot customise the format of type %s, type is not valid'), $new_type), E_USER_WARNING);
			return FALSE;
		}

		// check that this type isnt already customised
		$current_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, $new_type);
		if (empty($current_link)) {
			// create a new format bodycopy for this type
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');

			$copy_link = Array(
							'asset'			=> &$folder,
							'value'			=> $new_type,
							'link_type'		=> SQ_LINK_TYPE_2,
							'is_dependant'	=> 1,
							'is_exclusive'	=> 1,
						 );

			$bodycopy = new Bodycopy();
			$bodycopy->setAttrValue('name', $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($new_type, 'name').' Format');
			$args = Array('content' => $this->_getDefaultBodycopyContent($new_type));
			if (!$bodycopy->create($copy_link, $args)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		} else {
			trigger_localised_error('CMS0046', sprintf(translate('Cannot customise type %s, it is already customised'), $new_type), E_USER_WARNING);
			return FALSE;
		}

		return TRUE;

	}//end createTypeFormat()


//--        PAINTING       --//


	/**
	* Returns the URL that the form will use for its target
	*
	* @return string
	* @access private
	*/
	function _getFormUrlComponents()
	{
		$base_url = ($this->attr('submit_to_page_url') ? $this->getURL() : current_url(TRUE, TRUE, FALSE));
		$form_method = strtolower($this->_getFormSubmitMethod());

		$replacements = Array(
							$this->id.'_formid'	=> NULL,
						);

		if ($form_method == 'get') {
			// no point retaining query string vars for a GET form action because the browser
			// won't submit them - instead we will preserve them as hidden fields later
			$url = $base_url;
		} else {
			$url = replace_query_string_vars($replacements, $base_url);
		}

		return Array(
				'url'		=> $url,
				'url_base'	=> $base_url,
			   );

	}//end _getFormUrlComponents()


	/**
	* Prints out the Frontend for this asset
	*
	* @return void
	* @access public
	*/
	function printFrontend()
	{

		// Tie the process and following paint together using db2 in order to
		// avoid possible replication slowdown on db1
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');

		$can_take_quiz = TRUE;
		if ($this->getFlag(SQ_ONLINE_QUIZ_FLAG_COMPLETE)) {
			if (!$this->attr('can_retake')) {
				$can_take_quiz = FALSE;
			}
		}

		if ($can_take_quiz) {
			if ($this->attr('interactive_mode')){
				$this->processInteractiveQuestions();
			} else {
				$this->processQuestions();
				$this->processAssetSelections();
			}
		}
		parent::printFrontend();
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

	}//end printFrontend()
	
	/**
	* Constructs the carts contents
	*
	* @return void
	* @access public
	*/
	function printContents()
	{
		// collect the user details - don't let the user see the test until we have what we need
		// if handleUserDetailsCollection() returns FALSE, we haven't managed to finish acquiring the user's details,
		if (!$this->handleUserDetailsCollection()) return;

		$flag_complete = $this->getFlag(SQ_ONLINE_QUIZ_FLAG_COMPLETE);

		// submit the quiz for marking if the submit button was processed
		if (isset($_REQUEST[$this->getPrefix().'_submit_button']) && !$flag_complete) {
			// gather the points for each answer
			$questions = $this->getQuestions();
			$results   = Array(
							'questions'	=> Array(),
							'totals'	=> Array(
											'points'			=> 0,
											'available_points'	=> 0,
										   ),
						 );
			foreach ($questions as $id => $details) {
				$question = $GLOBALS['SQ_SYSTEM']->am->getAsset($id);
				if (!is_null($question)) {
					$question_results = $question->getResults();

					$points           = $question->getPoints();
					$available        = $question->getAvailablePoints();
					$position         = $question->getPosition();

					$results['questions'][$position]        = $question_results;
					$results['totals']['points']           += $points;
					$results['totals']['available_points'] += $available;

					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($question);
				}
			}

			// sort the results by question number
			ksort($results['questions'], SORT_NUMERIC);

			if ($this->attr('interactive_mode')){
				//we need to adjust the points such that the retried questions don't count
				foreach (array_keys($results['questions']) as $position){
					if (strpos($position, ' retry ') !== FALSE){
						$components = explode(' retry ', $position);
						if ($components[1] > 1){
							$previous_position = $components[0].' retry '.--$components[1];
						} else {
							$previous_position = $components[0];
						}

						if (isset($results['questions'][$previous_position])){
							$results['totals']['points'] -= $results['questions'][$previous_position]['points'];
							$results['questions'][$previous_position]['points'] = 0;
							$results['totals']['available_points'] -= $results['questions'][$previous_position]['available_points'];
							$results['questions'][$previous_position]['available_points'] = 0;
						}
					}
				}
				ksort($results['questions'], SORT_STRING);
			}

			// set up the results for the thank you page and email later
			$this->setResults($results);

			// Log the Quiz Submission if required. This comprises both the User Details Form and the Quiz responses.
			if ($this->attr('log_submissions')) {
				$submission_asset = $this->createSubmission();

				$user_details_form = $this->getForm();
				$user_details_form_questions = $user_details_form->getQuestions();

				$user_details = $this->getUserDetails();
				$user_details_raw = $this->getUserDetails(True);
				$quiz_results = $this->getResults();
				$question_results = $quiz_results['questions'];

				// Log the answers given for each User Details question
				foreach ($user_details as $question_id => $user_response) {
					$question_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($question_id);
					$submission_asset->setUserDetailsQuestion($question_id, $question_asset->getVal('title'));
					if($user_response === $user_details_raw[$question_id]){
						$submission_asset->setUserDetailsAnswer($question_id, $user_response);
					}else{
						$submission_asset->setUserDetailsAnswer($question_id, $user_response.' - '.$user_details_raw[$question_id]);
					}
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($question_asset);
				}

				foreach ($question_results as $question_position => $question_result) {
					$question_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($question_result['assetid']);

					$question_text = strip_tags($question_asset->getQuestionText());
					$question_result['correct_summary'] = trim(strip_tags($question_result['correct_summary']));
					$question_result['summary'] = trim(strip_tags($question_result['summary']));

					$submission_asset->setQuizQuestion($question_result['assetid'], trim($question_text));
					$submission_asset->setQuizAnswer($question_result['assetid'], $question_result);
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($question_asset);
				}

				$submission_asset->setScore($this->getScoreKeywordReplacement());
				$submission_asset->setIP($_SERVER['REMOTE_ADDR']);
				$submission_asset->setAttrValue('complete', TRUE);
				$submission_asset->saveAttributes();
			}//end if

			// send the results off to the user
			$this->sendResultsEmail();

			// if appropriate, print the Thank You bodycopy
			switch ($this->attr('after_submit_action')) {
				case 'thank_you':
					$this->printBodycopy('thank_you');

					// clear the questions and results
					$this->clearQuizData(array_keys($questions));
					$this->setFlag(SQ_ONLINE_QUIZ_FLAG_COMPLETE, TRUE, TRUE);
					return;
				break;

				case 'return_to_quiz':
					// clear the questions and results
					$this->clearQuizData(array_keys($questions));
					$this->setFlag(SQ_ONLINE_QUIZ_FLAG_COMPLETE, TRUE, TRUE);

					// collect the user details - don't let the user see the test until we have what we need
					// if handleUserDetailsCollection() returns FALSE, we haven't managed to finish acquiring the user's details,
					if (!$this->handleUserDetailsCollection()) return;

					// in the extreme instance where the following settings are set:
					// - cannot retake
					// - return to quiz (no thank you page)
					// - user details source is the current user
					// ... the following code is necessary to plug the gap (otherwise
					// the user is presented with the quiz again)
					if (!$this->attr('can_retake')) {
						$this->printBodycopy('already_taken');
						return;
					}
				break;
			}

		}//end if (isset($_REQUEST(submit button))

		// whether or not a user can retake the quiz is handled in printFrontend()
		// all we're doing here is making sure that the Complete flag is switched
		// off so the user can complete the quiz
		if ($flag_complete) {
			$this->setFlag(SQ_ONLINE_QUIZ_FLAG_COMPLETE, FALSE, TRUE);
		}

		if ($this->attr('interactive_mode')){
			$question_pagination = $this->loadQuestionPagination();
			if (empty($question_pagination['pages_to_questions'])){
				$this->printBodycopy('no_results');
			} else {
				// Make a clean URL to submit to
				$form_method = strtolower($this->_getFormSubmitMethod());
				$form_url_components = $this->_getFormUrlComponents();
				$form_url			 = htmlspecialchars($form_url_components['url']);
				?> <form id="<?php echo $this->getPrefix() ?>" method="<?php echo $form_method ?>" action="<?php echo $form_url; ?>"> <?php
				$this->printBodycopy('page_contents');
				?> </form> <?php
			}

			return;
		}

		//// print the listing ////

		// print the old_result_page, so we can keep track of which page we just came from when changing page
		$this->registerFormField('old_result_page');
		hidden_field('quiz_'.$this->id.'_old_result_page', $this->getCurrentPage());

		// print the result_page as a hidden form, so that it shows up in $_REQUEST when the form is submitted by the pagination vars
		$this->registerFormField('result_page');
		hidden_field('quiz_'.$this->id.'_result_page', '');

		//provide the listing engine with the current page to display.
		$_REQUEST['result_'.$this->id.'_result_page'] = $this->getCurrentPage();

		// Alrighty, if we're not using the %asset_listing% keyword, we have to make sure that the appropriate keywords are replaced, as Listing Engine will not do this for us
		$page_contents_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'page_contents');
		if (!empty($page_contents_link)) {
			$format_bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($page_contents_link['minorid'], $page_contents_link['minor_type_code']);

			require_once SQ_FUDGE_PATH.'/general/text.inc';
			$keywords = $format_bodycopy->getKeywords();

			if (!in_array('asset_listing', $keywords)) {
				$replacements = Array();
				foreach ($keywords as $word) {
					$replacements[$word] = $this->getKeywordReplacement($word);
				}

				$format_bodycopy->setKeywordReplacements($replacements);
			}
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($format_bodycopy);

		}

		parent::printContents();

	}//end printContents()


	/**
	* Get the list of asset that should be printed
	*
	* The return array is in the form Array(assetid => type_code) unless we are grouping by letter,
	* in which case the return array is
	* <pre>
	* Array(
	* 	assetid	=> Array(
	* 				'type_code' => type_code,
	* 				'first_letter' => first_letter,
	* 			   ),
	* );
	* </pre>
	*
	* @return array
	* @access protected
	*/
	function getAssetList()
	{
		$assets_to_list = Array();
		$questions      = $this->getQuestions();

		if (!empty($questions)) {
			foreach ($questions as $id => $details) {
				// compatibility with different listing formats
				if ($this->attr('group_by') == 'letter') {
					$assets_to_list[$id] = $details;
				} else {
					$assets_to_list[$id] = Array( 0 => Array('type_code' => $details['type_code']));
				}
			}
		}

		return $assets_to_list;

	}//end getAssetList()


	/**
	* Returns the questions for the current user, and optionally generates a new list of questions
	* depending on the value of $refresh:
	*
	* If $refresh is TRUE, the question list will be refreshed regardless of whether it currently exists or not
	* If FALSE, the list will not be generated if it doesn't exist
	* If NULL, the list will only be generated if it doesn't already exist
	* In Interactive Mode, the question list is always refreshed.
	*
	* @param boolean	$refresh			Determines whether or not the question list will be refreshed (see above)
	* @param boolean	$all_interactive	In interactive mode, whether to return all quiz questions under the root nodes,
	* 										otherwise only the relevant subset of questions loaded into the user quiz session.
	* 										See Page_Online_Quiz::loadQuestionPagination().
	*
	* @return array
	* @access public
	*/
	function getQuestions($refresh=NULL, $all_interactive=FALSE)
	{

		if ($this->attr('interactive_mode')){
			if ($all_interactive){
				//used by the edit interface
				$interactive_questions = $this->getInteractiveQuestions();
			} else {
				//while undertaking the quiz, need to always refresh the question list to add the retry questions
				$question_pagination = $this->loadQuestionPagination();
				$interactive_questions = $question_pagination['questions_to_pages'];
			}

			$this->saveQuestionList(array_keys($interactive_questions));
			$questions = $this->loadQuestions();
			return $questions;
		}

		// if $refresh is TRUE, pretend we have no questions to force the regeneration of questions
		if ($refresh === TRUE) {
			$questions = Array();
		} else {
			$questions = $this->loadQuestions();
		}

		// only refresh if $refresh is TRUE or NULL
		if (empty($questions) && $refresh !== FALSE) {

			$assets_to_list = $this->getManuallyConfiguredQuestions();
			$manually_configured_questions = !empty($assets_to_list);

			if (empty($assets_to_list)) {
				$assets_to_list = parent::getAssetList();
				parent::filterAssetTypes($assets_to_list);
			}

			if (empty($assets_to_list)) return Array();

			// work out how many assets we need to list
			$attr_num_questions = $this->attr('num_questions');
			if ($attr_num_questions <= 0 || $attr_num_questions > count($assets_to_list)) {
				$num_questions = count($assets_to_list);
			} else {
				$num_questions = $attr_num_questions;
			}

			// Don't randomise question display if we have set them up manually!
			if ($manually_configured_questions) {
				$random_asset_list =& $assets_to_list;
			} else {
				// Randomize the order
				// array_rand(array, 1) will produce a string instead of an array - compensate
				$random_asset_list = Array();

				// Fix for #4058 Online quiz with specific questions asked first and random number of questions
				// now before we randomly drop out the questions we need to make sure we do not drop out the compulsory ones
				// viz-a-viz the ones in Asset Positions under List Position Settings
				$mandatory_listing_assets = $this->attr('asset_positions');
				foreach ($mandatory_listing_assets as $index => $info) {
					if (isset($assets_to_list[$info['id']])) unset ($assets_to_list[$info['id']]);
				}
				// while we are at it, lets make space for the assets from asset_positions. and from here we can pick number of questions apart from compulsory listing ones
				$num_questions = $num_questions - count($mandatory_listing_assets);

				$keys = array_keys($assets_to_list);
				shuffle($keys);
				$random_asset_list_keys = Array();
				foreach($keys as $key => $val) {
					if ($key > ($num_questions-1)) break;
					$random_asset_list_keys[$key] = $val; 
				}

				foreach($mandatory_listing_assets as $index => $info) {
					$random_asset_list[$info['id']] = Array ( '0' => Array ( 'type_code' => $info['type'] ));
				}

				foreach ($random_asset_list_keys as $key) {
					$random_asset_list[$key] = $assets_to_list[$key];
				}
			}

			// modify the asset list to conform to the question format
			foreach ($random_asset_list as $assetid => $type) {
				// the 'letter' group_by return the 'type' as an array,
				// containing extra information, eg. first_letter
				// mix these extra elements in with the rest for compatibility's sake
				if ($this->attr('group_by') == 'letter') {
					$questions[$assetid]             = $type;
					$questions[$assetid]['value']    = NULL;
					$questions[$assetid]['position'] = NULL;
				} else {
					$questions[$assetid]          = Array(
														'type_code'	=> $type,
														'value'		=> NULL,
														'position'	=> NULL,
													);
				}
			}

			// fixate the questions now that we've generated the list
			$this->saveQuestions($questions);
			$this->saveQuestionList(array_keys($questions));

		}//end if ($refresh != FALSE)


		// verify that the current questions still exist
		$existing_questions = array_flip($GLOBALS['SQ_SYSTEM']->am->assetExists(array_keys($questions)));
		foreach ($questions as $id => $question) {
			if (!isset($existing_questions[$id])) {
				unset($questions[$id]);
			}
		}

		return $questions;

	}//end getQuestions()


	/**
	* Get all the online_quiz_question_multichoice questions that can be used for the interactive mode.
	*
	* @param boolean $grouped group by root node (pool)?
	*
	* @return array
	* @access public
	*/
	function getInteractiveQuestions($grouped = FALSE)
	{
		require_once SQ_FUDGE_PATH.'/general/general.inc';

		$interactive_questions = Array ();

		$pool_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE, '', FALSE, 'major', 'root');
		foreach ($pool_links as $link){
			$questions = $GLOBALS['SQ_SYSTEM']->am->getChildren($link['minorid'], 'online_quiz_question_multichoice', TRUE, NULL, NULL, SQ_PERMISSION_READ);
			if (!empty($questions)){
				$questions = dal_array_flatten($questions);

				//filter out what we don't have read access to
				foreach ($questions as $question_id => $type_code){
					$question = $GLOBALS['SQ_SYSTEM']->am->getAsset($question_id);
					if (is_null($question) || !$question->readAccess()) {
						unset($questions[$question_id]);
					}
				}

				//randomising
				$keys = array_keys($questions);
				shuffle($keys);
				$randomised_questions = Array();
				foreach ($keys as $key){
					$randomised_questions[$key] = $questions[$key];
					unset($questions[$key]);
				}

				if ($grouped){
					$interactive_questions[$link['minorid']] = $randomised_questions;
				} else {
					$interactive_questions += $randomised_questions;
				}
			}
		}

		return $interactive_questions;

	}//end getInteractiveQuestions()


	/**
	* Checks whether the questions were supplied via an asset listing (by using the '%asset_listing%' keyword), or manually.
	* If the questions were supplied manually, the asset IDs and types returned will consist of these assets instead of using the Listing Engine.
	*
	* @return array
	* @access public
	*/
	function getManuallyConfiguredQuestions()
	{
		$assets_to_list = Array();

		$page_contents_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'page_contents');
		if (!empty($page_contents_link)) {
			$format_bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($page_contents_link['minorid'], $page_contents_link['minor_type_code']);
			require_once SQ_FUDGE_PATH.'/general/text.inc';
			$keywords = $format_bodycopy->getKeywords();
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($format_bodycopy);

			// Question display was manually configured. These questions will be scored and tracked as available questions when the
			// question response is shown (%question_X_response_form%)
			if (!in_array('asset_listing', $keywords)) {
				$replacements = Array();
				foreach ($keywords as $keyword) {
					if (preg_match('/^(question)_([0-9]*)_(response_form)$/', $keyword, $matches)) {
						if (isset($matches[1]) && isset($matches[2]) && isset($matches[3])) {
							// Add this question to a list of quiz questions
							$question_asset_id = $matches[2];
							$question = $GLOBALS['SQ_SYSTEM']->am->getAsset($question_asset_id);

							// Check again to ensure that it is a question
							$question_type = $question->type();
							if ($question_type == 'online_quiz_question_multichoice') {
								if (isset($matches[1]) && isset($matches[2])) {
									// Check permissions to this asset before listing (read permission must be there)
									if ($question->readAccess()) {
										$assets_to_list[$question_asset_id] = $question_type;
									}
								}
							}

							$GLOBALS['SQ_SYSTEM']->am->forgetAsset($question);
						}
					}
				}
			}
		}//end if

		return $assets_to_list;

	}//end getManuallyConfiguredQuestions()


	/**
	* Send the email containing the quiz results to the user
	*
	* @return boolean
	* @access public
	*/
	function sendResultsEmail()
	{
		$email_attr           = $this->getAttribute('results_email_format');
		$value                =  unserialize($email_attr->value);

		// make sure the proper version of the results table is used in the plaintext version of the email
		$value['text_format'] =  str_replace('%results_table_full%',   '%results_table_full_text%',   $value['text_format']);
		$value['text_format'] =  str_replace('%results_table_simple%', '%results_table_simple_text%', $value['text_format']);

		// ...and for Question and Question Group result HTML-based keywords. Use a "_plain" suffix if the last bit is "_text" to avoid "%fred_text_text%"
		$result_keywords = Array(
							'question_text', 'answer_text', 'correct_answer_text', 'question_note',
						   );
		foreach ($result_keywords as $result_keyword) {
			$value['text_format'] = preg_replace('/%(result)_([0-9]*)_('.$result_keyword.')%/', '%result_$2_$3_plain%', $value['text_format']);
		}
		$value['text_format'] = preg_replace('/%(result)_([0-9]*)_(response_supplement)%/', '%result_$2_$3_text%', $value['text_format']);
		$value['text_format'] = preg_replace('/%(question_group)_([0-9]*)_(score_category_supplement)%/', '%question_group_$2_$3_text%', $value['text_format']);

		$email_attr->value    =  serialize($value);

		$keywords = $email_attr->getKeywords();

		$replacements = Array();
		foreach ($keywords as $keyword) {
			$replacements[$keyword] = $this->getKeywordReplacement($keyword);
		}

		// email_format already checks for us if there are no 'To' addresses
		return $email_attr->sendMail($replacements);

	}//end sendResultsEmail()


	/**
	* Send the email containing the quiz results to the user
	*
	* @param array	$questions	an array of question assetids to be cleared (if NULL, then the results of getQuestions() is used)
	*
	* @return boolean
	* @access public
	*/
	function clearQuizData($questions=NULL)
	{
		if (is_null($questions)) {
			$questions = array_keys($this->getQuestions());
		}

		if (is_array($questions)) {
			foreach ($questions as $assetid) {
				unset($_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['question'][$assetid]);
			}
		}

		// unset all the quiz data (results, flags, question list)
		unset($_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]);

	}//end clearQuizData()


	/**
	* Returns whatever details we have on the current user
	* @param boolean	$values	determins whether to get the summary or the value
	*					True for the "raw" value, False for the summary
	*
	* @return array
	* @access public
	*/
	function getUserDetails($values = False)
	{
		$details = Array();
		if (!$values && !empty($_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['user_details'])) {
			$details = $_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['user_details'];
		} else if ($values && !empty($_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['user_details_raw'])) {
			$details = $_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['user_details_raw'];
		}

		return $details;

	}//end getUserDetails()

	/**
	* Sets the details we have of the user for later use
	*
	* @param array	$summary	user details in the format: Array('assetid:questionid' => 'Answer Summary')
	* @param array	$raw_values	user details in the format: Array('assetid:questionid' => 'Answer Value')
	*
	* @return boolean
	* @access public
	*/
	function setUserDetails($summary, $raw_values)
	{
		if (is_array($summary) && is_array($raw_values)) {
			$_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['user_details'] = $summary;
			$_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['user_details_raw'] = $raw_values;
			return TRUE;
		}

		return FALSE;

	}//end setUserDetails()


	/**
	* Attempts to gather the user details - if they haven't been collected (and need to be, in the
	* case of public_users or 'user_details_source' being set to 'form'), then print the user
	* details form and return FALSE
	* Otherwise, return TRUE
	*
	* @return boolean
	* @access public
	*/
	function handleUserDetailsCollection()
	{
		// if we're using the current user's details collect user data during quiz submission, so
		// we can just make calls to the user's data on the fly, rather than attempting to
		// precache everything (eg. attributes, metadata)

		// always enforce the use of a form if the current user is the public user;
		// "Public User" doesn't tend to look good on quiz results
		if ($this->attr('user_details_source') == 'form' || $GLOBALS['SQ_SYSTEM']->userPublic()) {
			$user_details = $this->getUserDetails();
			if (empty($user_details)) {
				// determine whether we need to get details from the current user, or a form
				// fill in dummy values for public user
				$form = $this->getForm();
				if (!is_null($form)) {
					// check to see if we need to process
					if (array_get_index($_REQUEST, $this->id.'_formid') && $form->process()) {
						// process, then let through to the rest of the function to print the questions, etc.
						// gets the summaries for the user details form questions
						$summary = $form->getAnswerSummaries();
						// gets the "raw" values for the user details form questions
						$raw_values = $form->getAnswerValues();
						$this->setUserDetails($summary, $raw_values);
					} else {
						// either not processed or details missing, so display the form again and bail out
						$this->_tmp['form_errors'] = $form->getErrors();
						$this->printBodycopy('user_details');
						return FALSE;
					}
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($form);
				}
			}
		}
		return TRUE;

	}//end handleUserDetailsCollection()


	/**
	* Returns a flag for the current quiz, eg. 'is_complete'
	*
	* @param string	$flag	the flag to return the status of
	*
	* @return mixed int|float|string|boolean|array|NULL
	* @access public
	*/
	function getFlag($flag)
	{
		$return = NULL;

		// check links first, unless we're the public user
		if (!$GLOBALS['SQ_SYSTEM']->userPublic()) {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($this->id, $GLOBALS['SQ_SYSTEM']->user->id, SQ_LINK_NOTICE);
			if (!empty($link)) {
				$link_value = unserialize($link['value']);
				$return = array_get_index($link_value, $flag, NULL);
			}
		}

		// fallback to the session
		if (is_null($return)) {
			if (isset($_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['flags'][$flag])) {
				$return = $_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['flags'][$flag];
			}
		}

		return $return;

	}//end getFlag()


	/**
	* Sets a flag for the current quiz, eg. 'is_complete'
	*
	* @param string		$flag		the flag to set the status of
	* @param mixed		$value		the value to set the flag to
	* @param boolean	$permanent	if TRUE, then use a link to store the flag, instead of just using the (temporary) session
	*
	* @return boolean
	* @access public
	*/
	function setFlag($flag, $value, $permanent=FALSE)
	{
		if (empty($flag)) return FALSE;

		// only allow permanent flags if the user is logged in - the best we can
		// do for the public_user is the session
		if ($permanent && !$GLOBALS['SQ_SYSTEM']->userPublic()) {

			$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

			$link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($this->id, $GLOBALS['SQ_SYSTEM']->user->id, SQ_LINK_NOTICE);
			if (!empty($link)) {
				// update instead of create
				$link_value        = unserialize($link['value']);
				$link_value[$flag] = $value;
				$new_link_value    = serialize($link_value);
				$res = $GLOBALS['SQ_SYSTEM']->am->updateLink($link['linkid'], SQ_LINK_NOTICE, $new_link_value);
			} else {
				$new_value      = Array(
									$flag	=> $value,
								  );
				$new_link_value = serialize($new_value);
				$res = $GLOBALS['SQ_SYSTEM']->am->createAssetLink($this, $GLOBALS['SQ_SYSTEM']->user, SQ_LINK_NOTICE, $new_link_value);
			}

			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

			return $res;
		}

		// fall back to just storing the flag in an array in the session
		$_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['flags'][$flag] = $value;

		return TRUE;

	}//end setFlag()


	/**
	* Returns the quiz results for the current user, in the following format:
	* <pre>
	* Array(
	* 	'questions'	=> Array(
	* 					question_num	=> Array(
	*										'assetid'			=> assetid,
	* 										'value'				=> string,	// raw value
	* 										'summary'			=> string,	// human-readible value
	* 										'points'			=> int,		// number of points scored
	* 										'available_points'	=> int,		// maximum number of points able to be scored
	* 									   ),
	*				   ),
	* 	'totals'	=> Array(
	* 					'points'			=> int,	// total number of points scored
	* 					'available_points	=> int,	// maximum number of points able to be scored across the entire quiz
	* 				   ),
	* )
	* </pre>
	*
	* @return array
	* @access public
	*/
	function getResults()
	{
		$results = Array();
		if (!empty($_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['results'])) {
			$results = $_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['results'];
		}

		return $results;

	}//end getResults()


	/**
	* Sets the results for the for the current user
	*
	* @param array	$results	the results array to store
	*
	* @return boolean
	* @access public
	* @see getResults()
	*/
	function setResults($results)
	{
		$_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['results'] = $results;
		return TRUE;

	}//end setResults()


	/**
	* Retrieve the questions, along with their data
	*
	* @return array
	* @access public
	*/
	function loadQuestions()
	{
		$questions = Array();

		// get the question list first, then load the data for each
		$question_list = $this->loadQuestionList();
		if (!empty($question_list)) {
			foreach ($question_list as $assetid) {
				// load the premade question entries, and manually construct the rest
				if (!empty($_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['question'][$assetid])) {
					$questions[$assetid] = $_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['question'][$assetid];
				} else {
					$question = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
					if (!is_null($question)) {
						$questions[$assetid] = $question->getQuestionData();
						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($question);
					}
				}
			}
		}

		return $questions;

	}//end loadQuestions()


	/**
	* Take the raw question data and save it in the session
	*
	* @param array	$questions	the question data to be saved
	*
	* @return void
	* @access public
	*/
	function saveQuestions($questions)
	{
		if (!empty($questions)) {
			foreach ($questions as $assetid => $question_data) {
				$question = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				if (!is_null($question)) {
					$questions[$assetid] = $question->setValue($question_data['value']);
					$questions[$assetid] = $question->setPosition($question_data['position']);
					$question->saveQuestion();
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($question);
				}
			}
		}

	}//end saveQuestions()


	/**
	* Load the list of questions associated with this quiz & user
	*
	* @return array
	* @access public
	*/
	function loadQuestionList()
	{
		$question_list = Array();

		// load everything from the session
		if (!empty($_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['question_list'])) {
			$question_list = $_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['question_list'];
		}

		return $question_list;

	}//end loadQuestionList()


	/**
	* Saves the list of questions associated with this quiz & user
	*
	* @param array	$question_list	the questions to save
	*
	* @return boolean
	* @access public
	*/
	function saveQuestionList($question_list)
	{
		if (!is_array($question_list)) {
			$question_list = Array();
		}

		// load everything from the session
		$_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['question_list'] = $question_list;

		return TRUE;

	}//end saveQuestionList()


	/**
	* Load the question pagination cache containing the 'questions_to_pages' and 'pages_to_questions' indices.
	* Additionally containing 'pages_to_status' and 'pages_to_retry_questions' indices in interactive mode.
	*
	* @return array
	* @access public
	*/
	function loadQuestionPagination()
	{
		$question_pagination = Array();
		if (!empty($_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['question_pagination'])) {
			$question_pagination = $_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['question_pagination'];
		}

		if (empty($question_pagination) || empty($question_pagination['pages_to_questions'])) {
			// regenerate the question pagination cache
			if ($this->attr('interactive_mode')){
				 /**
				 * This mode heavily depends on this question pagination cache, and is updated frequently while undertaking the quiz.
				 *
				 * 'pages_to_questions' index will always contain up to date information on which question is to be displayed on each page
				 * and each page will only contain one question at any one time. This question may be the initial question or the retry question.
				 *
				 * 'questions_to_pages' index initially contains all questions to be displayed and is updated by adding the questions used on a retry.
				 *
				 * 'pages_to_retry_questions' index contains the retry reserve available for each page.
				 *
				 * 'pages_to_status' index contains each page status used to control what is to be displayed on each page & to combat browser navigation.
				 *
				 */
				$question_pool = $this->getInteractiveQuestions(TRUE);
				$pool_setup = $this->attr('question_pool_setup');

				$page_no = 1;
				foreach ($pool_setup as $pool_id => $info){
					if (isset($question_pool[$pool_id])){
						$questions = $question_pool[$pool_id];
					} else {
						continue;
					}
					//give one question per page upto the pool size
					for ($size = 0; $size < $info['size']; $size++){
						reset($questions);
						$question_id = key($questions);
						if (empty($question_id)) {
							trigger_localised_error('CMS0112', sprintf(translate('Shortage of questions for pool root node (Id: #%s). Check for correct pool size or user read access.'), $pool_id), E_USER_WARNING);
							break;
						}
						$question_pagination['pages_to_questions'][$page_no][$question_id] = $questions[$question_id]['type_code'];
						$question_pagination['questions_to_pages'][$question_id] = Array(
																			'page'			=> $page_no,
																			'list_position'	=> $page_no
																		 	);
						$question_pagination['pages_to_status'][$page_no] = 'submit_question';
						unset($questions[$question_id]);

						//give this page the required retry questions
						for ($retries = 0; $retries < $info['retries']; $retries++){
							reset($questions);
							$question_id = key($questions);
							if (empty($question_id)) {
								trigger_localised_error('CMS0112', sprintf(translate('Shortage of questions for pool root node (Id: #%s). Check for correct pool size or user read access.'), $pool_id), E_USER_WARNING);
								break;
							}
							$question_pagination['pages_to_retry_questions'][$page_no][$question_id] = $questions[$question_id]['type_code'];
							unset($questions[$question_id]);
						}
						$page_no++;
					}
				}

			} else {
				// grab the list of question assets for chunking later
				$children = $this->getAssetList();
				$this->filterAssetList($children);
				$replacements = Array();
				$question_pagination = Array(
										'pages_to_questions'	=> Array(),
										'questions_to_pages'	=> Array(),
									   );

				// as an initial attempt, load the current page of assets, and store it in the question pagination array
				// check $replacements['total_pages'] to see how many more chunks we have to get

				// assumption: any assets that are of interest to us (online_quiz_questions) will never be parents
				// of other assets that interest us (other quiz questions)
				// eg. the following code will break if the below is encountered:
				// [quiz:1]
				//    |--- [quiz:2]
				//    '--- [quiz:3]
				$current_result_page = $this->getCurrentPage();
				require_once SQ_FUDGE_PATH.'/general/general.inc';
				$current_page_assets = dal_array_flatten($this->getPageAssets($children, $replacements, $current_result_page), 'type_code');
				$total_pages = $replacements['total_pages'];

				if (!empty($current_page_assets)) {
					$question_pagination['pages_to_questions'][$current_result_page] = $current_page_assets;

					// load all the pages, skipping over anything that's already been loaded (ie. the current page)
					for ($ii = 1; $ii <= $total_pages; $ii++) {
						if (isset($question_pagination['pages_to_questions'][$ii])) {
							$page_assets = $question_pagination['pages_to_questions'][$ii];
						} else {
							$page_assets = dal_array_flatten($this->getPageAssets($children, $replacements, $ii), 'type_code');
							$question_pagination['pages_to_questions'][$ii] = $page_assets;
						}

						$asset_count  = 1;
						$num_per_page = $this->_getNumPerPage();
						foreach ($page_assets as $assetid => $type) {
							$question_pagination['questions_to_pages'][$assetid] = Array(
																					'page'			=> $ii,
																					'list_position'	=> ($ii - 1) * $num_per_page + $asset_count,
																				   );
							$asset_count++;
						}
					}
				}
			}//end else (not interactive mode)

			$this->saveQuestionPagination($question_pagination);

		}//end if (empty($question_pagination))

		return $question_pagination;

	}//end loadQuestionPagination()


	/**
	* Save the question pagination cache containing the 'questions_to_pages' and' pages_to_questions' indices.
	* Additionally containing 'pages_to_status' and 'pages_to_retry_questions' indices in interactive mode.
	*
	* @param array	$question_pagination	the question pagination details to save
	*
	* @return boolean
	* @access public
	*/
	function saveQuestionPagination($question_pagination)
	{
		// load everything from the session
		$_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['question_pagination'] = $question_pagination;

		return TRUE;

	}//end saveQuestionPagination()


	/**
	* Process the interactive questions before later displaying or submitting the quiz.
	* Save the question answers and update all session data.
	*
	* @return boolean
	* @access public
	*/
	function processInteractiveQuestions()
	{

		// figure out which assets are on what pages
		$question_pagination = $this->loadQuestionPagination();
		if (empty($question_pagination['pages_to_questions'])) return TRUE;

		// figure out which assets are on the current page
		$current_result_page = $this->getCurrentPage();
		$current_question_id = key($question_pagination['pages_to_questions'][$current_result_page]);

		$current_status = $question_pagination['pages_to_status'][$current_result_page];

		$prefix = $this->getPrefix();

		if (isset($_POST[$prefix.'_submit_next_button']) && $current_status == 'feedback_next'){
			$_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['interactive_page'] = $current_result_page + 1;
			$question_pagination['pages_to_status'][$current_result_page] = 'answered';
		} else if (isset($_POST[$prefix.'_submit_retry_button']) && $current_status == 'feedback_retry'){
			//shouldn't be empty, just double checking
			if (!empty($question_pagination['pages_to_retry_questions'][$current_result_page])) {
				//grab the next question in the same pool
				$question_id = key($question_pagination['pages_to_retry_questions'][$current_result_page]);
				$question_type = $question_pagination['pages_to_retry_questions'][$current_result_page][$question_id];
				unset($question_pagination['pages_to_retry_questions'][$current_result_page][$question_id]);
				$question_pagination['pages_to_questions'][$current_result_page] = Array ($question_id => $question_type);

				//add the retry question to the 'questions_to_pages' and give it its retry position.
				$question_pagination['questions_to_pages'][$question_id] = $question_pagination['questions_to_pages'][$current_question_id];
				$question_list_position = $question_pagination['questions_to_pages'][$question_id]['list_position'];
				if (strpos($question_list_position, ' retry ') !== FALSE){
					$components = explode(' retry ', $question_list_position);
					$question_list_position = $components[0].' retry '.++$components[1];
				} else {
					$question_list_position .= ' retry 1';
				}
				$question_pagination['questions_to_pages'][$question_id]['list_position'] = $question_list_position;

				//update the page status
				$question_pagination['pages_to_status'][$current_result_page] = 'submit_question_retry';
			} else {
				$question_pagination['pages_to_status'][$current_result_page] = 'feedback_next';
			}
		} else if (isset($_POST[$prefix.'_submit_question_button']) && strpos($current_status, 'submit_question') === 0){
			$question = $GLOBALS['SQ_SYSTEM']->am->getAsset($current_question_id);
			if (!is_null($question)) {
				if ($current_status == 'submit_question_retry'){
					if (isset($_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['interactive_score']['retries'])){
						$_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['interactive_score']['retries']++;
					} else {
						$_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['interactive_score']['retries'] = 1;
					}
				} else {
					if (isset($_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['interactive_score']['total_questions'])){
						$_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['interactive_score']['total_questions']++;
					} else {
						$_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['interactive_score']['total_questions'] = 1;
					}
				}

				$errors = $question->processResponseForm();
				if ($current_result_page == count($question_pagination['pages_to_questions'])) {
					$question_pagination['pages_to_status'][$current_result_page] = 'feedback_last';
				} else {
					$question_pagination['pages_to_status'][$current_result_page] = 'feedback_next';
				}

				$value = $question->getValue();
				if (!is_array($value)) $value = Array($value);
				if (array_intersect($value, $question->getCorrectValue()) === Array()){
					if(!empty($question_pagination['pages_to_retry_questions'][$current_result_page])){
						$question_pagination['pages_to_status'][$current_result_page] = 'feedback_retry';
					}
				} else {
					if (isset($_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['interactive_score']['correct_answers'])){
						$_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['interactive_score']['correct_answers']++;
					} else {
						$_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['interactive_score']['correct_answers'] = 1;
					}
				}
			}
		}

		$this->saveQuestionPagination($question_pagination);

		//set and save the position of the current page question. Need to use up to date data.
		$question_pagination = $this->loadQuestionPagination();
		$current_result_page = $this->getCurrentPage();
		$current_question_id = key($question_pagination['pages_to_questions'][$current_result_page]);

		if (!empty($question_pagination['questions_to_pages'][$current_question_id]['list_position'])) {
			$question = $GLOBALS['SQ_SYSTEM']->am->getAsset($current_question_id);
			if (!is_null($question)) {
				// set the asset position
				$question->setPosition($question_pagination['questions_to_pages'][$current_question_id]['list_position']);
				$question->saveQuestion();
			}
		}

		return TRUE;

	}//end processInteractiveQuestions()


	/**
	* Process the questions (validating, saving answers) before later displaying or submitting the quiz
	*
	* @return boolean
	* @access public
	*/
	function processQuestions()
	{
		// load all questions, and separate them into three: questions from the page we just came from,
		// questions from the page we're about to display, and the rest of the questions.
		// process and collect errors from the first group, leave the second, and collect
		// errors from the third by means of validateValue()

		// load all the questions attached to the quiz
		$questions = $this->getQuestions();

		// set up the error stack for later use
		$error_stack = Array(
						'errors'	=> Array(),
						'warnings'	=> Array(),
					   );

		// figure out which assets are on what pages
		$question_pagination = $this->loadQuestionPagination();

		// figure out which assets are on the current page
		$current_result_page  =  $this->getCurrentPage();
		$current_page_assets  =& $question_pagination['pages_to_questions'][$current_result_page];

		// now grab the previous page of assets (if applicable)
		$previous_page_assets =  Array();
		$old_result_page      = array_get_index($_REQUEST, 'quiz_'.$this->id.'_old_result_page', NULL);
		if (!is_null($old_result_page)) {
			$previous_page_assets =& $question_pagination['pages_to_questions'][$old_result_page];
		}

		foreach ($questions as $questionid => $question_details) {
			$question = $GLOBALS['SQ_SYSTEM']->am->getAsset($questionid);
			if (!is_null($question)) {
				// set the asset position
				if (!empty($question_pagination['questions_to_pages'][$questionid]['list_position'])) {
					$question->setPosition($question_pagination['questions_to_pages'][$questionid]['list_position']);
				}

				// Figure out what to process
				if (isset($previous_page_assets[$questionid])) {
					// process any questions that were on the page we just came from
					$errors = $question->processResponseForm();
				} else {
					// just get the warnings
					$errors = $question->validateValue();
				}

				// only get the warnings if the questions aren't on the current page, or warn_on_missing_answers is off
				// otherwise, present warnings to the user via javascript
				if (!isset($current_page_assets[$questionid]) || !$this->attr('clientside_validation')) {
					if (!empty($errors['errors'])) {
						$error_stack['errors'] = array_merge($error_stack['errors'], $errors['errors']);
					}
					if (!empty($errors['warnings'])) {
						$error_stack['warnings'] = array_merge($error_stack['warnings'], $errors['warnings']);
					}
				}

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($question);
			}//end if (!is_null($question))
		}//end foreach ($questions)

		// set the error_stack, to be used later when the doing keyword replacements (eg. submit button prints the warnings)
		$this->_error_stack = $error_stack;

		return TRUE;

	}//end processQuestions()


	/**
	* Gets the specified chunk based on group-by and pagination options
	*
	* Overridden because we need to modify the pagination keyword replacements so that
	* they use javascript to submit the form
	*
	* @param array	&$children		the list of assets that qualify to be
	*								printed
	* @param array	&$replacements	the current replacements array, which will
	*								be added to by this function
	* @param array	$keywords		Keywords that need replacing
	* @param array	$result_page	result page
	* @param array	$num_per_page	number of assets per page
	* @param array	$group_by		what to group by
	*
	* @return array
	* @access public
	*/
	function &getChunk(&$children, &$replacements, $keywords, $result_page, $num_per_page, $group_by=NULL)
	{
		$todo =& parent::getChunk($children, $replacements, $keywords, $result_page, $num_per_page, $group_by);

		// if the page keywords are present, deconstruct them, add the javascript to force the processing of the form,
		// and reconstruct them in the form '<a href="url"' + javascript + '>link text</a>'

		// the following javascript:
		// - sets the value of the #_result_page hidden field, to control which page we're going to end up at
		// - removes #_result_page from the GET vars of the form action, to avoid conflicts
		// - submits the form
		$link_js    =  ' onClick="document.getElementById(\'quiz_'.$this->id.'_result_page\').value=\'%s\'; '
					  .'frm=document.getElementById(\''.$this->getPrefix().'\'); '
					  .'frm.action=\''.htmlentities(replace_query_string_vars(Array('quiz_'.$this->id.'_result_page' => NULL))).'\'; '
					  .'frm.submit(); return false;" ';

		// we have the luxury of assuming that #_result_page will always exist if a link is present
		$link_regex =  '|(<a href=")(([^"]*result_'.$this->id.'_result_page=)([[:alnum:]]*))([^"]*")(>[^<]*</a>)|';

		$url_reps   = Array(
						$this->id.'_formid'	=> NULL,
					  );

		// next_page keyword
		preg_match($link_regex, $replacements['next_page'], $matches);
		if (!empty($matches)) {
			// remove the formid query var
			list($base, $query) = explode('?', $matches[2]);
			$url       = replace_query_string_vars($url_reps, $base, $query);

			// sub the destination page into the javascript
			$dest_page = $matches[4];
			$replacements['next_page'] = $matches[1].htmlentities($url).$matches[5].sprintf($link_js, $dest_page).$matches[6];
		}

		// previous_page keyword
		preg_match($link_regex, $replacements['previous_page'], $matches);
		if (!empty($matches)) {
			// remove the formid query var
			list($base, $query) = explode('?', $matches[2]);
			$url       = replace_query_string_vars($url_reps, $base, $query);

			// sub the destination page into the javascript
			$dest_page = $matches[4];
			$replacements['previous_page'] = $matches[1].htmlentities($url).$matches[5].sprintf($link_js, $dest_page).$matches[6];
		}


		// page_list keywords
		// do some trickery with the keyword replacements - replace %d in $link_js with a regex special character,
		// to get preg_replace to sub in the target page number for us
		$replacements['page_list'] = preg_replace(
										'|(<a href="[^"]*result_'.$this->id.'_result_page=)([[:alnum:]]*)([^"]*")([^>]*>)|',
										'$1$2$3'.sprintf($link_js, '$2').'$4',
										$replacements['page_list']
									 );
		$replacements['page_list_without_unused'] = preg_replace(
														'|(<a href="[^"]*result_'.$this->id.'_result_page=)([[:alnum:]]*)([^"]*")([^>]*>)|',
														'$1$2$3'.sprintf($link_js, '$2').'$4',
														$replacements['page_list']
													);


		// unset the next_page_href and previous_page_href for safety (allows for the form
		// to change pages without submitting the form to save the questions)
		unset($replacements['next_page_href']);
		unset($replacements['previous_page_href']);

		return $todo;

	}//end getChunk()


//--        Keyword Listings        --//


	/**
	* Add valid keywords for this asset to an array of keywords when asked
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster we add keywords to the $vars['keywords'] array
	*
	* @return boolean
	* @access private
	*/
	function onRequestKeywords(&$broadcaster, $vars=Array())
	{
		if (!isset($vars['keywords'])) return;
		if (!($broadcaster instanceof Content_Type)) return;

		$parents  = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id, 'bodycopy', TRUE);

		// the broadcaster could be our page contents bodycopy, in which case
		// we want to supply our own keyword replacements
		$page_contents_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'page_contents');
		if (!empty($page_contents_link)) {
			if (isset($parents[$page_contents_link['minorid']])) {
				$vars['keywords'] += $this->_getContentsKeywords();
				$vars['keywords'] += $this->_getQuestionKeywords();
				if ($this->attr('interactive_mode')){
					$vars['keywords'] += $this->_getInteractiveKeywords();
				}
				return;
			}
		}

		// and the same for the thank_you page
		$thank_you_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'thank_you');
		if (!empty($thank_you_link)) {
			if (isset($parents[$thank_you_link['minorid']])) {
				$vars['keywords'] += $this->_getResultsKeywords();
				$vars['keywords'] += $this->_getUserDetailsKeywords();
				$vars['keywords'] += $this->_getQuestionResultKeywords();
				$vars['keywords'] += $this->_getQuestionGroupResultKeywords();
				$vars['keywords'] += Array ('quiz_score' => translate('Quiz Score (using Quiz Score Format setting)'));

				return;
			}
		}

		// and for the user_details page
		$user_details_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'user_details');
		if (!empty($user_details_link)) {
			if (isset($parents[$user_details_link['minorid']])) {
				$vars['keywords'] += $this->_getFormKeywords();
				return;
			}
		}


		// and the already_taken page
		$user_details_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'already_taken');
		if (!empty($user_details_link)) {
			if (isset($parents[$user_details_link['minorid']])) {
				return;
			}
		}

		// type folder links
		$folder = $this->getFolder();
		$type_formats = Array();
		if (!is_null($folder)) {
			$type_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($folder->id, SQ_LINK_TYPE_2, 'bodycopy');
			foreach ($type_links as $link_info) {
				if (isset($parents[$link_info['minorid']])) {
					$type_formats[] = $link_info['value'];
				}
			}
		}

		// if it's not the page contents bodycopy, it's something that we've
		// inherited from the listing engine, so pass it on
		parent::onRequestKeywords($broadcaster, $vars);

		// if any type formats present, override the asset_position keyword
		if (count($type_formats) > 0) {
			// set up the 'question_number' alias (mimics asset_position)
			$vars['keywords']['asset_position'] = translate('Question Number (Asset Position)');

		}

	}//end onRequestKeywords()


	/**
	* Return the available keywords in the Page Contents Bodycopy for this asset
	*
	* The return value is in the form:
	* <pre>
	* Array(
	*     'keyword' => 'name',
	* )
	* </pre>
	*
	* @return array
	* @access public
	*/
	function _getContentsKeywords()
	{
		$keywords = parent::_getContentsKeywords();
		$GLOBALS['SQ_SYSTEM']->lm->includeAssetStrings('page_asset_listing');

		$keywords += Array(
						'error_list'		=> translate('Error list'),

						'warning_list'		=> translate('Warning list'),

						'question_range'	=> translate('Question Range (which questions are currently being displayed)'),

						'question_count'	=> translate('Question Count (number of questions in this quiz)'),

					 );

		// unset the next_page_href and previous_page_href for safety (allows for the form
		// to change pages without submitting the form to save the questions)
		unset($keywords['next_page_href']);
		unset($keywords['previous_page_href']);

		return $keywords;

	}//end _getContentsKeywords()


	/**
	* Returns a list of keywords that can be used on the User Details bodycopy when
	* printing the user details form
	*
	* @return array
	* @access public
	*/
	function _getFormKeywords()
	{
		$keywords = Array();

		$keywords['form_contents'] = translate('Form Contents');

		$keywords['form_errors'] = translate('List of Submission Errors');


		return $keywords;

	}//end _getFormKeywords()


	/**
	* Returns a list of keywords that can be used to print to display the user's details
	*
	* @return array
	* @access public
	*/
	function _getUserDetailsKeywords()
	{
		$keywords = Array();

		// grab the questions attached to the form
		$form = $this->getForm();
		if (!is_null($form)) {
			$questions = $form->getQuestions(TRUE);
			foreach ($questions as $assetid => $question_details) {
				list($bridgeid, $shadowid) = explode(':', $assetid);
				$keywords['user_details_form_'.$bridgeid.'-'.$shadowid] = 'User Details Form Field: #'.$assetid.' ('.$question_details['name'].')';
				$keywords['user_details_form_'.$bridgeid.'-'.$shadowid.'_raw'] = 'User Details Form Raw Value: #'.$assetid.' ('.$question_details['name'].')';
			}
		}

		// get the generic user to provide us with a list of keywords
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('user');
		$user = new User();
		$user_keywords = $user->getAvailableKeywords();
		foreach ($user_keywords as $keyword => $description) {
			$keywords['user_details_current_user_'.$keyword] = 'Current User: '.$description;
		}

		return $keywords;

	}//end _getUserDetailsKeywords()


	/**
	* Returns a list of keywords that can be used to print to display the user's details
	*
	* @return array
	* @access public
	*/
	function _getResultsKeywords()
	{
		$keywords = Array();

		$keywords['results_table_full']   = translate('Results Table (Full)');

		$keywords['results_table_simple'] = translate('Results Table (Simple)');


		return $keywords;

	}//end _getResultsKeywords()


	/**
	* Returns a list of keywords that can be used in the page contents bodycopy in interactive mode.
	*
	* @return array
	* @access public
	*/
	function _getInteractiveKeywords()
	{
		$keywords = Array();

		$keywords['interactive_question_response_form']   = translate('Interactive Question Response Form');

		$keywords['interactive_question_text'] = translate('Interactive Question Text');

		$keywords['interactive_question_note'] = translate('Interactive Question Note');

		$keywords['interactive_question_number'] = translate('Interactive Question Number');

		$keywords['interactive_button'] = translate('Interactive Button');

		$keywords['quiz_score'] = translate('Quiz Score (using Quiz Score Format setting)');


		return $keywords;

	}//end _getInteractiveKeywords()


	/**
	* Returns a list of keywords that can be used to print an Online Quiz Question asset
	*
	* @return array
	* @access public
	*/
	function _getQuestionKeywords()
	{
		$keywords = Array();

		// Refresh the question list, if we are manually configuring the page's question content and adding/removing/editing this order.
		// Only do this if the %asset_listing% keyword is missing from the Page Contents bodycopy.
		$page_contents_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'page_contents');
		if (!empty($page_contents_link)) {
			$format_bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($page_contents_link['minorid'], $page_contents_link['minor_type_code']);

			require_once SQ_FUDGE_PATH.'/general/text.inc';
			$content_keywords = $format_bodycopy->getKeywords();

			if (!in_array('asset_listing', $content_keywords)) {
				$form = $this->getForm();
				if (!is_null($form)) {
					$questions = $form->getQuestions(TRUE);
				}
			}
		}

		$keywords = Array();
		// Obtain the available keywords for a quiz question
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('online_quiz_question');
		$question = new Online_Quiz_Question();
		$question_keywords = $question->getAvailableKeywords();
		foreach ($question_keywords as $keyword => $description) {
			$keywords['question_X_'.$keyword] = 'Question X: '.$description;
		}

		return $keywords;

	}//end _getQuestionKeywords()


	/**
	* Returns a list of keywords that can be used to print an Online Quiz Question asset and its score.
	* The keywords available here are specific only to results, and do not contain inherited Question asset keywords
	*
	* @return array
	* @access public
	*/
	function _getQuestionResultKeywords()
	{
		$keywords = Array();
		$available_keywords = Array(
								'question_number'		=> translate('Question Number'),

								'question_text'			=> translate('Question Text'),

								'answer'				=> translate('Your Answer'),

								'answer_text'			=> translate('Answer Text'),

								'correct_answer'		=> translate('Correct Answer'),

								'correct_answer_text'	=> translate('Correct Answer Text'),

								'points'				=> translate('Your Score'),

								'available_points'		=> translate('Maximum Score'),

								'response_supplement'	=> translate('Response Supplement'),

								'question_note'			=> translate('Question Note'),

							  );

		$locale_text = translate('Result for Question X');

		foreach ($available_keywords as $keyword => $description) {
			$keywords['result_X_'.$keyword] = $locale_text.': '.$description;
		}

		// And some more general keywords...
		$keywords['quiz_total_points'] = translate('Quiz: Total Points Obtained');

		$keywords['quiz_total_available_points'] = translate('Quiz: Maximum Total Points');


		return $keywords;

	}//end _getQuestionResultKeywords()


	/**
	* Returns a list of keywords that can be used to print an Online Quiz Question asset and its score.
	* The keywords available here are specific only to results, and do not contain inherited Question asset keywords
	*
	* @return array
	* @access public
	*/
	function _getQuestionGroupResultKeywords()
	{
		$keywords = Array();
		$keywords['question_group_X_score_category_name'] = translate('Result for Question Group X').': '.translate('Score Category Name');


		$keywords['question_group_X_score_category_supplement'] = translate('Result for Question Group X').': '.translate('Score Category Supplementary Content');



		return $keywords;

	}//end _getQuestionGroupResultKeywords()


//--        Keyword Replacements        --//


	/**
	* Get bodycopy keyword replacement
	*
	* Provides a mechanism for providing keyword replacement functions by
	* looking up the function named 'getKeywordNameKeywordReplacement'
	* so an asset wishing to add support for 'best_ever' custom keyword
	* should implement a function getBestEverKeywordReplacement()
	* which returns a replacement string
	* (Overridden in page_online_quiz to handle dynamic keywords, eg. user_details_form_q##)
	*
	* @param string	$keyword	Keyword to find a replacement for
	*
	* @return string
	* @access public
	*/
	function getKeywordReplacement($keyword)
	{

		if (empty($keyword)) return '';

		$keyword = parse_keyword($keyword, $modifiers);

		if ($this->attr('interactive_mode') && strpos($keyword, 'interactive_') === 0){
			if ($keyword == 'interactive_button'){
				$replacement =  $this->getInteractiveButtonKeywordReplacement();
			} else {
				$replacement = $this->getInteractiveQuestionKeywordReplacement(substr($keyword, strlen('interactive_')));
			}
			if (!is_null($replacement)) apply_keyword_modifiers($replacement, $modifiers, Array('assetid' => $this->id));
			return $replacement;
		}

		if ($keyword == 'quiz_score'){
			$replacement = $this->getScoreKeywordReplacement();
			apply_keyword_modifiers($replacement, $modifiers, Array('assetid' => $this->id));
			return $replacement;
		}

		// check the keyword against the dynamic keywords
		// in this case - user details:
		if (preg_match('/^(user_details_form)_([0-9]*-q[0-9]*)$/', $keyword, $matches)) {
			if (isset($matches[1]) && isset($matches[2])) {
				// arg = question assetid
				$arg          = str_replace('-', ':', $matches[2]);
				$keyword_stem = $matches[1];
			}
		}

		// check the keyword against the dynamic keywords
		// in this case - user details raw values:
		if (preg_match('/^(user_details_form)_([0-9]*-q[0-9]*)_(raw)$/', $keyword, $matches)) {
			if (isset($matches[1]) && isset($matches[2]) && isset($matches[3])) {
				// arg = question assetid
				$arg          = str_replace('-', ':', $matches[2]);
				$keyword_stem = $matches[1].'_'.$matches[3];
			}
		}

		if (preg_match('/^(user_details_current_user)_(.*)$/', $keyword, $matches)) {
			if (isset($matches[1]) && isset($matches[2])) {
				// if the current user is the public user, the keyword replacements that would've
				// been collected below would have been invalid
				// return a blank replacement instead
				if ($GLOBALS['SQ_SYSTEM']->userPublic()) return '';

				// arg = user attribute
				$arg          = $matches[2];
				$keyword_stem = $matches[1];
			}
		}

		if (preg_match('/^(question)_([0-9]*)_(.*)$/', $keyword, $matches)) {
			if (isset($matches[1]) && isset($matches[2]) && isset($matches[3])) {
				// Keyword match example: question_123_response_form becomes (question, 123, response_form)
				$keyword_stem = $matches[1];
				$asset_id     = $matches[2];
				$attr         = $matches[3];

				// Grab the question keyword
				if (!empty($keyword_stem) && !empty($asset_id) && !empty($attr)) {
					$replacement = $this->getQuestionKeywordReplacement($asset_id, $attr);
					apply_keyword_modifiers($replacement, $modifiers, Array('assetid' => $this->id));
					return $replacement;
				}
			}
		}

		// Question and result keywords for the "Thank You" bodycopy
		if (preg_match('/^(result)_([0-9]*)_(.*)$/', $keyword, $matches)) {
			if (isset($matches[1]) && isset($matches[2]) && isset($matches[3])) {
				// Keyword match example: result_123_answer becomes (result, 123, answer)
				$keyword_stem = $matches[1];
				$asset_id     = $matches[2];
				$attr         = $matches[3];
				// Grab the question result (answers etc.) keyword
				if (!empty($keyword_stem) && !empty($asset_id) && !empty($attr)) {
					$replacement = $this->getQuestionResultKeywordReplacement($asset_id, $attr);
					apply_keyword_modifiers($replacement, $modifiers, Array('assetid' => $this->id));
					return $replacement;
				}
			}
		}

		// Question Group result keywords for the "Thank You" bodycopy
		if (preg_match('/^(question_group)_([0-9]*)_(.*)$/', $keyword, $matches)) {
			if (isset($matches[1]) && isset($matches[2]) && isset($matches[3])) {
				$keyword_stem = $matches[1];
				$asset_id     = $matches[2];
				$attr         = $matches[3];

				// Grab the question result (answers etc.) keyword
				if (!empty($keyword_stem) && !empty($asset_id) && !empty($attr)) {
					$replacement = $this->getQuestionResultKeywordReplacement($asset_id, $attr);
					apply_keyword_modifiers($replacement, $modifiers, Array('assetid' => $this->id));
					return $replacement;
				}
			}
		}

		// setup the replacement function call
		if (!empty($keyword_stem) && !empty($arg)) {
			$func_name = 'get'.ucwords_no_space($keyword_stem).'KeywordReplacement';
			if (method_exists($this, $func_name)) {
				return $this->$func_name($arg);
			}
		}

		// otherwise, fall back to the parent
		$replacement = parent::getKeywordReplacement($keyword);
		if (!is_null($replacement)) apply_keyword_modifiers($replacement, $modifiers, Array('assetid' => $this->id));

		return $replacement;

	}//end getKeywordReplacement()


	/**
	* Returns a replacement for interactive_question_X keywords using the current page question.
	*
	* @param string	$keyword Keyword to find a replacement for
	*
	* @return string
	* @access public
	*/
	function getInteractiveQuestionKeywordReplacement($keyword)
	{
		$replacement = NULL;

		$question_pagination = $this->loadQuestionPagination();
		$current_result_page  =  $this->getCurrentPage();

		$question_id = key($question_pagination['pages_to_questions'][$current_result_page]);
		$question = $GLOBALS['SQ_SYSTEM']->am->getAsset($question_id, 'online_quiz_question_multichoice');
		if ((is_null($question))) return $replacement;

		switch ($keyword){
			case 'question_response_form':
				$current_status = $question_pagination['pages_to_status'][$current_result_page];
				if (strpos($current_status, 'feedback') !== FALSE){
					//print the question but with disabled fields and add special classes & tags.
					$replacement = $question->getResponseFormKeywordReplacement($this->attr('question_option_format'), TRUE);
				} else {
					$replacement = $question->getResponseFormKeywordReplacement($this->attr('question_option_format'));
				}
				break;
			case 'question_note':
				$replacement = $question->getQuestionNoteKeywordReplacement();
				break;
			case 'question_text':
				$replacement = $question->getQuestionTextKeywordReplacement();
				break;
			case 'question_number':
				$replacement = $question->getQuestionNumberKeywordReplacement();
				break;
		}

		return $replacement;

	}//end getInteractiveQuestionKeywordReplacement()


	/**
	* Return a keyword replacement for 'interactive_button' keyword.
	* Prints the appropriate interactive mode submission button based on the pages current status.
	*
	* @return string
	* @access public
	*/
	function getInteractiveButtonKeywordReplacement()
	{
		$prefix = $this->getPrefix();
		$current_result_page = $this->getCurrentPage();
		$question_pagination = $this->loadQuestionPagination();

		ob_start();

		$current_status = $question_pagination['pages_to_status'][$current_result_page];
		if ($current_status == 'feedback_retry'){
			submit_button($prefix.'_submit_retry_button', $this->attr('retry_button_text'));
		} else if ($current_status == 'feedback_last'){
			submit_button($prefix.'_submit_button', $this->attr('submit_button_text'));
		} else if ($current_status == 'feedback_next'){
			submit_button($prefix.'_submit_next_button', $this->attr('next_button_text'));
		} else if (strpos($current_status, 'submit_question') === 0){
			//using this so we can have JS validation for the currently displayed question.
			echo $this->getSubmitButtonKeywordReplacement($prefix.'_submit_question_button', $this->attr('submit_question_button_text'));
		}

		return ob_get_clean();


	}//end getInteractiveButtonKeywordReplacement()


	/**
	* Return a keyword replacement for 'quiz_score' keyword in the format given by 'score_format' attribute.
	* Also used for submission log records.
	*
	* @return string
	* @access public
	*/
	function getScoreKeywordReplacement()
	{
		$replacement = $this->attr('score_format');

		if ($this->attr('interactive_mode')) {

			//Number of questions done so far
			$total_questions = 0;
			if (!empty($_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['interactive_score']['total_questions'])){
			    $total_questions = $_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['interactive_score']['total_questions'];
			}

			//Don't display the score if we havn't answered any questions yet. i.e first page.
			if ($total_questions < 1) return NULL;

			//Number of questions answered correctly.
			$correct_answers = 0;
			if (!empty($_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['interactive_score']['correct_answers'])){
			    $correct_answers = $_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['interactive_score']['correct_answers'];
			}

			//Number of question retries so far
			$retries = 0;
			if (!empty($_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['interactive_score']['retries'])){
			    $retries = $_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['interactive_score']['retries'];
			}

			$total_questions_plus_retries = $total_questions + $retries;

			$replacement = str_replace('%correct_answers%', $correct_answers, $replacement);
			$replacement = str_replace('%total_questions%', $total_questions, $replacement);
			$replacement = str_replace('%total_questions_plus_retries%', $total_questions_plus_retries, $replacement);
			$replacement = str_replace('%total_retries%', $retries, $replacement);
		}

		//these only work in the thank you bodycopy.
		$question_results = $this->getResults();
		if (isset($question_results['totals']['points'])){
			$replacement = str_replace('%quiz_total_points%', $question_results['totals']['points'], $replacement);
		}
		if (isset($question_results['totals']['available_points'])){
			$replacement = str_replace('%quiz_total_available_points%', $question_results['totals']['available_points'], $replacement);
		}

		return $replacement;

	}//end getScoreKeywordReplacement()


	/**
	* Returns the keyword replacements for any 'user_details_form_[##-q##]' keywords
	*
	* @param string	$questionid	the form question to get the answer from
	*
	* @return array
	* @access public
	*/
	function getUserDetailsFormKeywordReplacement($questionid)
	{
		$user_details = $this->getUserDetails(False);
		$return = '';
		if (!empty($user_details)) {
			$return = array_get_index($user_details, $questionid, '');
		}

		return $return;

	}//end getUserDetailsFormKeywordReplacement()

	/**
	* Returns the keyword replacements for any 'user_details_form_[##-q##]_raw' keywords
	* (replaces keywords with the value attribute of the the form field not the summary)
	*
	* @param string	$questionid	the form question to get the answer from
	*
	* @return array
	* @access public
	*/
	public function getUserDetailsFormRawKeywordReplacement($questionid)
	{
		$user_details = $this->getUserDetails(True);
		$return = '';
		if (!empty($user_details)) {
			$return = array_get_index($user_details, $questionid, '');
		}

		return $return;

	}//end getUserDetailsFormRawKeywordReplacement()


	/**
	* Returns the keyword replacements for any 'user_details_current_user_[keyword]' keywords
	*
	* @param string	$keyword	the keyword to get the replacement of
	*
	* @return array
	* @access public
	*/
	function getUserDetailsCurrentUserKeywordReplacement($keyword)
	{
		// ask the user asset to get the replacement for us
		return $GLOBALS['SQ_SYSTEM']->user->getKeywordReplacement('asset_'.$keyword);

	}//end getUserDetailsCurrentUserKeywordReplacement()


	/**
	* Get the replacement for "submit_button" keyword
	*
	* Overridden so we can have the list of warnings appear when the user clicks the submit button
	*
	* @param string	$field_name
	* @param string	$button_text
	*
	* @return string
	* @access public
	*/
	function getSubmitButtonKeywordReplacement($field_name = '', $button_text = '')
	{
		if (empty($field_name)){
			$prefix = $this->getPrefix();
			$field_name = $prefix.'_submit_button';
		}

		$this->registerFormField($field_name);

		if (empty($button_text)) {
			$button_text = $this->attr('submit_button_text');
		}

		// construct the javascript here
		$button_js = '';
		ob_start();
			if ($this->attr('clientside_validation')) {
				?>
				<script type="text/javascript">
					//<![CDATA[

					/**
					* Prints an alert() or confirm() containing errors taken from the error_stack
					*
					* @return boolean
					*/
					function print_error_summary()
					{
						form = document.getElementById("<?php echo $this->getPrefix(); ?>");
						if (!form) {
							return true;
						}

						var summary = "";
						var box_type = "";
						var error_stack = new Array();

						error_stack["errors"]   = new Array();
						error_stack["warnings"] = new Array();

						<?php
							$errors = array_get_index($this->_error_stack, 'errors', Array());
							foreach ($errors as $idx => $error) {
								?>
								error_stack["errors"][<?php echo $idx; ?>] = "<?php echo str_replace('"', '\"', $error); ?>";
								<?php
							}

							$warnings = array_get_index($this->_error_stack, 'warnings', Array());
							foreach ($warnings as $idx => $warning) {
								?>
								error_stack["warnings"][<?php echo $idx; ?>] = "<?php echo str_replace('"', '\"', $warning); ?>";
								<?php
							}

							// getQuestionPagination allows us to skip
							// reloading and chunking the question assets
							$pages             = array_get_index($this->loadQuestionPagination(), 'pages_to_questions', Array());
							$current_questions = array_get_index($pages, $this->getCurrentPage(), Array());
							foreach ($current_questions as $assetid => $type) {
								$question = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
								if (!is_null($question)) {
									echo $question->getValidationJS();
									$GLOBALS['SQ_SYSTEM']->am->forgetAsset($question);
								}
							}
						?>

						if (error_stack["errors"].length) {
							summary += "The following errors are present:\n";
							for (var idx in error_stack["errors"]) {
								summary += "  - " + error_stack["errors"][idx] + "\n";
							}
							summary += "\n";
							box_type = "alert";
						}

						if (error_stack["warnings"].length > 0) {
							summary += "The following warnings are present:\n";
							for (var idx in error_stack["warnings"]) {
								summary += "  - " + error_stack["warnings"][idx] + "\n";
							}
							summary += "\n";
							if (!box_type.length) {
								summary += "\nAre you sure you want to continue?";
								box_type = "confirm";
							}
						}

						if (box_type == "alert") {
							alert(summary);
							return false;
						} else if (box_type == "confirm") {
							return confirm(summary);
						} else {
							return true;
						}

					}//end function print_error_summary()
				//]]>
				</script>
				<?php

				$button_js = 'return print_error_summary();';
			}//end if (clientside_validation)

			submit_button($field_name, $button_text, $button_js);
		return ob_get_clean();

	}//end getSubmitButtonKeywordReplacement()


	/**
	* Get the replacement for the "error_list" keyword
	*
	* @return string
	* @access public
	*/
	function getErrorListKeywordReplacement()
	{
		$errors = array_get_index($this->_error_stack, 'errors', Array());
		if (!empty($errors)) {
			ob_start();
				?><ul><?php
				foreach ($errors as $error) {
					?><li><?php echo $error; ?></li><?php
				}
				?></ul><?php
			return ob_get_clean();
		}

		return '';

	}//end getErrorListKeywordReplacement()


	/**
	* Get the replacement for the "error_list" keyword
	*
	* @return string
	* @access public
	*/
	function getWarningListKeywordReplacement()
	{
		$warnings = array_get_index($this->_error_stack, 'warnings', Array());
		if (!empty($warnings)) {
			ob_start();
				?><ul><?php
				foreach ($warnings as $warning) {
					?><li><?php echo $warning; ?></li><?php
				}
				?></ul><?php
			return ob_get_clean();
		}

		return '';

	}//end getWarningListKeywordReplacement()


	/**
	* Returns the keyword replacements for the 'question_range' keyword
	*
	* @return array
	* @access public
	*/
	function getQuestionRangeKeywordReplacement()
	{
		$question_pagination = $this->loadQuestionPagination();
		$pages_to_questions  = array_get_index($question_pagination, 'pages_to_questions', Array());
		$questions_to_pages  = array_get_index($question_pagination, 'questions_to_pages', Array());
		$current_page_assets = array_get_index($pages_to_questions, $this->getCurrentPage(), Array());

		$first = NULL;
		$last  = NULL;
		foreach ($current_page_assets as $assetid => $type) {
			if (is_null($first)) {
				$first_asset = array_get_index($questions_to_pages, $assetid, Array());
				$first       = array_get_index($first_asset, 'list_position', NULL);
			}

			// keep grabbing items, in case it's the last one
			$last_asset = array_get_index($questions_to_pages, $assetid, Array());
			$last       = array_get_index($last_asset, 'list_position', NULL);
		}

		if (is_null($first) || is_null($last)) {
			return NULL;
		}

		return $first.' - '.$last;

	}//end getQuestionRangeKeywordReplacement()


	/**
	* Returns the keyword replacements for the 'question_count' keyword
	*
	* @return array
	* @access public
	*/
	function getQuestionCountKeywordReplacement()
	{
		if ($this->attr('interactive_mode')){
			$question_pagination = $this->loadQuestionPagination();
			//one question per page so number of questions = number of pages.
		    return count($question_pagination['pages_to_questions']);
		} else {
			return count($this->getQuestions());
		}

	}//end getQuestionCountKeywordReplacement()


	/**
	* Returns the keyword replacements for any 'results_[type]_table' keywords
	*
	* @param array		$show_columns	the columns to include in the table
	* @param boolean	$show_totals	whether or not to show totals at the bottom of the table
	* @param boolean	$html			whether or not to render the table in HTML or plain text
	*
	* @return array
	* @access public
	*/
	function getResultsTableKeywordReplacement($show_columns=NULL, $show_totals=TRUE, $html=TRUE)
	{
		if (!is_array($show_columns)) {
			// set to defaults
			$show_columns = Array(
								'question_num'		=> translate('Question Number'),

								'points'			=> translate('Your Score'),

								'available_points'	=> translate('Maximum Score'),

							);
		}

		// slight hack - tabs show up when printing plain text, but not when displaying as html
		$available_columns = Array(
								'question_num'		=> translate('Question Number'),

								'assetid'			=> translate('Asset ID'),

								'question_text'		=> translate('Question Text'),

								'value'				=> translate('Your Answer'),

								'summary'			=> translate('Answer Text'),

								'correct_value'		=> translate('Correct Answer'),

								'correct_summary'	=> translate('Correct Answer Text'),

								'points'			=> translate('Your Score'),

								'available_points'	=> translate('Maximum Score'),

							 );

		// plain-text column formatting:
		$column_name_suffix    = ': ';
		$longest_column_length = 0;

		$columns = Array();
		foreach ($show_columns as $column => $column_title) {
			if (isset($available_columns[$column])) {
				$columns[$column] = $column_title;
				if (strlen($column_title) > $longest_column_length) {
					$longest_column_length = strlen($column_title);
				}
			}
		}

		$results = $this->getResults();
		$return = '';
		if (!empty($results) && !empty($columns)) {
			ob_start();

				// HACK: hardcode a short name for the CSS class, to get around problems with long class names and Mac IE
				$css_class_prefix = str_replace('_', '-', 'quiz_res');

				if ($html) {
					?>
					<table class="<?php echo $css_class_prefix; ?>" id="<?php echo $css_class_prefix.'-'.$this->id; ?>">
					<tr>
					<?php

					$column_counter = 1;
					foreach ($columns as $column => $column_name) {
						?><th class="<?php echo $css_class_prefix.'-c'.$column_counter; ?>"><?php echo $column_name; ?></th><?php
						$column_counter++;
					}

					?>
					</tr>
					<?php
				}

				$row_counter = 1;
				foreach (array_get_index($results, 'questions', Array()) as $question_num => $question_results) {
					if ($html) {
						?>
						<tr>
						<?php
					}

					$column_counter = 1;
					foreach ($columns as $column => $column_name) {
						$alignment_padding = '';
						if ($html) {
							?><td class="<?php echo $css_class_prefix.'-c'.$column_counter; ?> <?php echo $css_class_prefix.'-r'.$row_counter; ?>"><?php
						} else {
							// pad
							echo $column_name;
							echo $column_name_suffix;
							for ($ii = 0; $ii < ($longest_column_length - strlen($column_name)); $ii++) {
								$alignment_padding .= ' ';
								echo ' ';
							}
							for ($ii = 0; $ii < strlen($column_name.$column_name_suffix); $ii++) {
								$alignment_padding .= ' ';
							}
						}

						ob_start();

							switch ($column) {
								case 'question_num':
									echo $question_num;
								break;

								case 'question_text':
									$question = $GLOBALS['SQ_SYSTEM']->am->getAsset($question_results['assetid']);
									if (!is_null($question)) {
										ob_start();
											echo $question->getQuestionText();
										$question_text = ob_get_clean();
										if (!$html) {
											// a hack to make bodycopy-derived question text appear normal when sending a plain-text email
											$question_text = trim(strip_tags($question_text));
											$question_text = html_entity_decode($question_text);

											// indent each line
											$question_text = str_replace("\n", "\n".$alignment_padding, $question_text);
										}
										echo $question_text;
										$GLOBALS['SQ_SYSTEM']->am->forgetAsset($question);
									}
								break;

								case 'assetid':
								case 'value':
								case 'correct_value':
								case 'points':
								case 'available_points':
									echo array_get_index($question_results, $column);

								break;

								case 'summary':
								case 'correct_summary':
									if ($html) {
										// explode into <li>
										$summary_text  = array_get_index($question_results, $column, '');
										if (!empty($summary_text)) {
											$summary_array = explode("\n", array_get_index($question_results, $column, ''));
											if (!empty($summary_array)) {
												?><ul><?php
													foreach ($summary_array as $summary_item) {
														?><li><?php
														echo $summary_item;
														?></li><?php
													}
												?></ul><?php
											}
										}
									} else {
										// indent each new line
										echo str_replace("\n", "\n".$alignment_padding, array_get_index($question_results, $column));
									}
								break;
							}//end switch

						$cell_contents = ob_get_clean();
						if ($html && !strlen($cell_contents)) {
							echo translate('&nbsp;');

						} else {
							echo $cell_contents;
						}

						if ($html) {
							?></td><?php
						} else {
							echo "\n";
						}
						$column_counter++;
					}//end foreach ($columns)

					if ($html) {
						?>
						</tr>
						<?php
					} else {
						echo "\n";
					}

					$row_counter++;
				}//end foreach ($results['questions'])

				// check if either points or available_points is set, if so
				// replace question_num, assetid or the column to the left
				// of the first points column with "Total"
				if ($show_totals) {

					$points_columns_counter = 0;
					$points_column = FALSE;
					$available_points_column = FALSE;
					if (isset($columns['points'])) {
						$points_columns_counter++;
						$points_column = TRUE;
					}
					if (isset($columns['available_points'])) {
						$points_columns_counter++;
						$available_points_column = TRUE;
					}

					// if we have at least one point column
					if ($points_columns_counter) {
						if ($html) {

							?><tr><?php
							$column_counter = 0;
							foreach ($columns as $column => $column_name) {
								?><td class="<?php echo $css_class_prefix.'_c'.($column_counter + 1); ?>"><?php

								// is this the first column, and are there enough columns to support a "Total" label?
								// also make sure the first column isn't a points column
								if ($column == 'points' || $column == 'available_points') {
									echo $results['totals'][$column];
								} else {
									if ($column_counter == 0 && count($columns) > $points_columns_counter + 1) {
										// print "Totals"
										echo translate('Totals');

									} else {
										// otherwise print an "empty" cell
										echo translate('&nbsp;');

									}
								}

								$column_counter++;

								?></td><?php
							}//end for (columns)
							?></tr><?php

						} else {
							// plain-text column formatting:
							$longest_column_length = 0;
							$totals_columns = Array(
												'points'           => translate('Total Score'),
												'available_points' => translate('Total Maximum Score'),
											  );
							foreach ($totals_columns as $column_name) {
								if (strlen($column_name) > $longest_column_length) {
									$longest_column_length = strlen($column_name);
								}
							}

							foreach ($columns as $column => $column_name) {
								if ($column == 'points' || $column == 'available_points') {
									echo $totals_columns[$column];
									echo $column_name_suffix;
									for ($ii = 0; $ii < ($longest_column_length - strlen($column_name)); $ii++) {
										echo ' ';
									}
									echo $results['totals'][$column];
									echo "\n";
								}
							}

						}
					}//end if ($points_columns_counter)
				}//end if ($show_totals)

				if ($html) {
					?>
					</table>
					<?php
				}
			$return = ob_get_clean();
		}//end if (!empty($results) && !empty($columns))

		return $return;

	}//end getResultsTableKeywordReplacement()


	/**
	* Returns the keyword replacements for the 'results_table_full' keyword
	*
	* @param boolean	$html	Whether the resulting table should be formatted with HTML or plain-text spacing
	*
	* @return array
	* @access public
	*/
	function getResultsTableFullKeywordReplacement($html=TRUE)
	{
		$columns = Array(
					'question_num'			=> translate('Question Number'),

					'question_text'			=> translate('Question Text'),

					'value'					=> translate('Your Answer'),

					'summary'				=> translate('Answer Text'),

					'correct_value'			=> translate('Correct Answer'),

					'correct_summary'		=> translate('Correct Answer Text'),

					'points'				=> translate('Your Score'),

					'available_points'		=> translate('Maximum Score'),

					'response_supplement'	=> translate('Maximum Score'),

				   );
		return $this->getResultsTableKeywordReplacement($columns, TRUE, $html);

	}//end getResultsTableFullKeywordReplacement()


	/**
	* Returns the keyword replacements for the 'results_table_full_text' keyword
	*
	* @return array
	* @access public
	*/
	function getResultsTableFullTextKeywordReplacement()
	{
		return $this->getResultsTableFullKeywordReplacement(FALSE);

	}//end getResultsTableFullTextKeywordReplacement()


	/**
	* Returns the keyword replacements for the 'results_table_simple' keyword
	*
	* @param boolean	$html	Whether the resulting table should be formatted with HTML or plain-text spacing
	*
	* @return array
	* @access public
	*/
	function getResultsTableSimpleKeywordReplacement($html=TRUE)
	{
		$columns = Array(
					'question_num'		=> translate('Question Number'),

					'value'				=> translate('Your Answer'),

					'correct_value'		=> translate('Correct Answer'),

					'points'			=> translate('Your Score'),

					'available_points'	=> translate('Maximum Score'),

				   );
		return $this->getResultsTableKeywordReplacement($columns, TRUE, $html);

	}//end getResultsTableSimpleKeywordReplacement()


	/**
	* Returns the keyword replacements for the 'results_table_simple_text' keyword
	*
	* @return array
	* @access public
	*/
	function getResultsTableSimpleTextKeywordReplacement()
	{
		return $this->getResultsTableSimpleKeywordReplacement(FALSE);

	}//end getResultsTableSimpleTextKeywordReplacement()


	/**
	* Returns the keyword replacements for Question-based keywords (format: %question_X_keyword%)
	*
	* @param int	$question_asset_id	The asset ID of the question
	* @param string	$keyword			The keyword to retrieve from the question
	*
	* @return array
	* @access public
	*/
	function getQuestionKeywordReplacement($question_asset_id, $keyword)
	{
		$replacement = '%question_'.$question_asset_id.'_'.$keyword.'%';

		$question = $GLOBALS['SQ_SYSTEM']->am->getAsset($question_asset_id, 'online_quiz_question_multichoice');
		if ((!is_null($question)) && ($question->readAccess())) {
			$replacement = $question->getKeywordReplacement($keyword);
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($question);
		}

		return $replacement;

	}//end getQuestionKeywordReplacement()


	/**
	* Returns the keyword replacements for Question-based result keywords (format: %result_X_keyword%)
	*
	* @param int	$question_asset_id	The asset ID of the question
	* @param string	$keyword			The keyword to retrieve from the question
	*
	* @return array
	* @access public
	*/
	function getQuestionResultKeywordReplacement($question_asset_id, $keyword)
	{
		$replacement = '%result_'.$question_asset_id.'_'.$keyword.'%';

		$question = $GLOBALS['SQ_SYSTEM']->am->getAsset($question_asset_id, 'online_quiz_question_multichoice');
		if ((!is_null($question)) && ($question->readAccess())) {
			$question_results = $question->getResults();

			// Some replacements are not provided in the question results, so we have to ask for these...
			$keyword_replaced = FALSE;

			switch ($keyword) {
				case 'question_number':				$replacement = $question->getPosition();
													$keyword_replaced = TRUE;
				break;

				case 'question_text':   			$replacement = $question->getKeywordReplacement('question_text');
													$keyword_replaced = TRUE;
				break;

				case 'question_text_plain':			$replacement = trim(strip_tags($question->getKeywordReplacement('question_text')));
													$keyword_replaced = TRUE;
				break;

				case 'question_note':				$replacement = $question->getQuestionNote();
													$keyword_replaced = TRUE;
				break;

				case 'question_note_plain':			$replacement = trim(strip_tags($question->getQuestionNote()));
													$keyword_replaced = TRUE;
				break;

				case 'answer':						$keyword = 'value';
				break;

				case 'answer_text':					$keyword = 'summary';
				break;

				case 'answer_text_plain':			if (isset($question_results['summary'])) {
														$replacement = trim(strip_tags($question_results['summary']));
														$keyword_replaced = TRUE;
													}
				break;

				case 'correct_answer':				$keyword = 'correct_value';
				break;

				case 'correct_answer_text':			$keyword = 'correct_summary';
				break;

				case 'correct_answer_text_plain':	if (isset($question_results['correct_summary'])) {
														$replacement = trim(strip_tags($question_results['correct_summary']));
														$keyword_replaced = TRUE;
													}
				break;

				case 'response_supplement':			$replacement = $question->getResponseSupplement();
				break;

				case 'response_supplement_text':	$replacement = trim(strip_tags($question->getResponseSupplement()));
				break;
			}//end switch

			// This keyword is obtained from the results...
			if (!$keyword_replaced) {
				if (isset($question_results[$keyword])) {
					$replacement = $question_results[$keyword];
				}
			}

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($question);
		}//end if

		return $replacement;

	}//end getQuestionResultKeywordReplacement()


	/**
	* Return the total points obtained by the current user for the quiz.
	* This value is dependent on the questions displayed in the results if the
	* results are customised.
	*
	* @return string
	* @access public
	*/
	function getQuizTotalPointsKeywordReplacement()
	{
		$total_points = '';

		$quiz_result = $this->getResults();
		$total_points = $quiz_result['totals']['points'];

		return $total_points;

	}//end getQuizTotalPointsKeywordReplacement()


	/**
	* Return the total available points for the quiz.
	* This value is dependent on the questions displayed in the results if the
	* results are customised.
	*
	* @return string
	* @access public
	*/
	function getQuizTotalAvailablePointsKeywordReplacement()
	{
		$total_points = '';

		$quiz_result = $this->getResults();
		$total_points = $quiz_result['totals']['available_points'];

		return $total_points;

	}//end getQuizTotalAvailablePointsKeywordReplacement()


	/**
	* Returns the keyword replacements for Question Group-based result keywords (format: %question_group_X_keyword%)
	*
	* @param int	$question_group_asset_id	The asset ID of the question group
	* @param string	$keyword					The keyword to retrieve from the question group
	*
	* @return string
	* @access public
	*/
	function getQuestionGroupResultKeywordReplacement($question_group_asset_id, $keyword)
	{
		$replacement = '%question_group_'.$question_group_asset_id.'_'.$keyword.'%';

		$question_group = $GLOBALS['SQ_SYSTEM']->am->getAsset($question_group_asset_id, 'online_quiz_question_group');
		if ((!is_null($question_group)) && ($question_group->readAccess())) {
			$group_questions = array_keys($question_group->getQuestions());
			$quiz_questions = array_keys($this->getQuestions());

			// Add up the totals for the questions which have been asked
			$questions_asked = array_intersect($quiz_questions, $group_questions);

			// Free memory
			$group_questions = NULL;
			$quiz_questions = NULL;

			$section_total = 0;
			foreach ($questions_asked as $question_asset_id) {
				$question = $GLOBALS['SQ_SYSTEM']->am->getAsset($question_asset_id);
				if (!is_null($question)) {
					$section_total += $question->getPoints();
				}
			}

			$score_category = $question_group->getScoreCategoryForScore($section_total);
			if (!is_null($score_category)) {
				switch ($keyword) {
					case 'score_category_name':				$replacement = $score_category['name'];
					break;

					case 'score_category_supplement':		$replacement = $score_category['content'];

															// replace internal links to pages and images with their full URLs
															$matches = Array();
															preg_match_all('|\./\?a=([0-9]+)|', $replacement, $matches);
															$urls = $GLOBALS['SQ_SYSTEM']->am->getAssetURL(array_unique($matches[1]));
															foreach ($urls as $assetid => $url) {
																$replacement = preg_replace('|\./\?a='.$assetid.'([^0-9])|', $url.'\\1', $replacement);
															}
					break;

					case 'score_category_supplement_text':	$replacement = strip_tags($score_category['content']);
					break;
				}
			}
		}//end if

		return $replacement;

	}//end getQuestionGroupResultKeywordReplacement()


	/**
	* Get the current form submit method (always POST)
	*
	* @return string
	* @access protected
	*/
	function _getFormSubmitMethod()
	{
		return 'post';

	}//end _getFormSubmitMethod()


//--        Listing Functions        --//


	/**
	* Returns the page currently being displayed
	*
	* NOTE: Cannot handle a-z grouping
	*
	* @return int
	* @access public
	*/
	function getCurrentPage()
	{
		if ($this->attr('interactive_mode')){
			//using sessions
			$result_page = 1;
			if (isset($_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['interactive_page'])) {
				$result_page = (int)$_SESSION[SQ_ONLINE_QUIZ_SESSION_VAR]['quiz'][$this->id]['interactive_page'];
			}

			if ($result_page <= 0) $result_page = 1;

			return $result_page;

		} else {
			$asset_result_page_var   = 'quiz_'.$this->id.'_result_page';
			$generic_result_page_var = 'result_page';

			// have [assetid]_result_page take precedence over result_page
			$result_page = 1;
			if (isset($_REQUEST[$generic_result_page_var])) {
				$result_page = (int)$_REQUEST[$generic_result_page_var];
			}
			if (isset($_REQUEST[$asset_result_page_var])) {
				$result_page = (int)$_REQUEST[$asset_result_page_var];
			}

			if ($result_page <= 0) $result_page = 1;

			return $result_page;
		}

	}//end getCurrentPage()


	/**
	* Returns a page of assets
	*
	* NOTE: Cannot handle a-z grouping,
	*       Also, both $children and $replacements are modified during the course of this function
	*
	* @param array	&$children		the list of assets to paginate
	* @param array	&$replacements	an array to catch the keyword replacements spat out by getChunk() during the chunking process
	* @param int	$result_page	the page of assets to return
	*
	* @return array
	* @access public
	*/
	function &getPageAssets(&$children, &$replacements, $result_page=NULL)
	{
		// the list is sorted and positions set, now we can split to groups
		if ($this->attr('group_by') == 'grouped') {
			$this->groupAssetsRecursively($this->attr('asset_grouping'), $children, $children);
			$this->sortGroups($this->attr('asset_grouping'), $children, $children);
		}

		$num_per_page = $this->_getNumPerPage();

		if (is_null($result_page) || $result_page <= 0) {
			$result_page = $this->getCurrentPage();
		}
		$result_page = (int)$result_page;

		// get our page chunk here
		$todo =& $this->getChunk($children, $replacements, Array(), $result_page, $num_per_page);

		//Bug #5283: Need to extract the page ids from the grouping structure
		if ($this->attr('group_by') == 'grouped') {
			for ($i = 0; $i < count($this->attr('asset_grouping')); $i++){ //group levels
				$todo_pages = Array();
				foreach($todo as $group_key => $group_data){
					foreach($group_data as $key => $value){
						$todo_pages[$key] = $value;
					}
				}
				$todo = $todo_pages; //$todo will be the final traversal
			}
		}

		return $todo;

	}//end getPageAssets()


}//end class

?>
