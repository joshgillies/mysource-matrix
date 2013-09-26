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
* $Id: form_email.inc,v 1.84 2013/09/26 02:38:29 akarelia Exp $
*
*/


require_once dirname(__FILE__).'/../form/form.inc';
require_once SQ_FUDGE_PATH.'/general/www.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';

/**
* Form_Email
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Revision: 1.84 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Email extends Form
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form_Email($assetid=0)
	{
		$this->Form($assetid);

	}//end constructor


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	* @param int		$contextid	what context to return the name from
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=FALSE, $contextid=NULL)
	{
		// No context specified, using the current context
		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}//end if

		// Obtain the attribute value for Name from the specified Context
		$values = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('name', $this->type(), Array($this->id), $contextid);
		if (empty($values) === TRUE) {
			return parent::_getName($short_name, $contextid);
		} else {
			return $values[$this->id];
		}

	}//end _getName()


	/**
	* Lets asset.inc save a copy of this asset at this point in time
	* and then updates the reference to content_file_questions.php content_file.php
	* to point correctly to .sq_system/content_file_questions.php
	*
	* This copy gets used when the asset is live and the user does not have write access
	*
	* @return boolean
	* @access public
	*/
	function saveSystemVersion()
	{
		// let parents do the copying of the files first
		parent::saveSystemVersion();

		// bug fix #6553 Custom form and safe editing do not play well
		// the .sq_system/content_file.php is still pointing to the
		// content_file_questions.php for the logged in users. lets
		// fix that first and then save the content again for the file
		$content = file_to_string($this->data_path.'/.sq_system/content_file.php');

		$pattern = '|'.str_replace(SQ_DATA_PATH, 'SQ_DATA_PATH."', $this->data_path).'/content_file_questions.php"|';
		$replacement = str_replace(SQ_DATA_PATH, 'SQ_DATA_PATH."', $this->data_path).'/.sq_system/content_file_questions.php"';

		$content = preg_replace($pattern, $replacement, $content);

		return string_to_file($content, $this->data_path.'/.sq_system/content_file.php');

	}//end saveSystemVersion()


	/**
	* Let the parent replace the editing version with the system (live) version
	* and then updates the .sq_system reference within the file
	*
	*
	* @return boolean
	* @access public
	*/
	public function revertToSystemVersion()
	{
		// let asset.inc copy across the file back from .sq_system dir
		parent::revertToSystemVersion();

		// once copied fix the content of the files to not point at .sq_system
		return  parent::fileRegeneration();

	}//end revertToSystemVersion()


	/**
	* Returns the header of the form, ie. the start form tags and anything else
	* needed by them
	*
	* Form Email adds a hidden URL field specifying the referral URL, which can
	* be used by emails (in particular, generic send-to-a-friend type emails).
	*
	* @return boolean
	* @access public
	*/
	function getFormHeader()
	{
		$header = parent::getFormHeader();

		$prefix = $this->getPrefix();

		// If we have a multi-page form, we would only need to know how we got
		// here originally. Therefore, if we have been sent a referral URL from
		// a previous page, use that instead.
		// Otherwise, generate the referrer using what we have been sent by the
		// HTTP request (with all the caveats that implies).
		if (array_key_exists($prefix.'_referral_url', $_POST)) {
			$referer = $_POST[$prefix.'_referral_url'];
		} else {
			$referer = (isset($_SERVER['HTTP_REFERER'])) ? $_SERVER['HTTP_REFERER'] : '';
		}

		ob_start();
			?><input type="hidden" name="<?php echo $prefix; ?>_referral_url" value="<?php echo htmlentities($referer); ?>" /><?php
		$header .= ob_get_clean();

		return $header;

	}//end getFormHeader()


	/**
	* Return form information (site name, site id, page name, page id, url, submission date)
	*
	* @param boolean	$html	return html formatted or not
	*
	* @return string
	* @access public
	*/
	function getFormInfo($html=FALSE)
	{
		$info = parent::getFormInfo();
		$title = 'Form Information';
		$information  = ($html) ? '<b>'.$title.'</b><br />' : $title."\n\n";
		$information .= ($html) ? '<table>' : '';

		foreach ($info as $name => $value) {
			$information .= ($html) ? '<tr><td valign="top">'.ucwords(str_replace('_', ' ', $name)).'</td>' : ucwords(str_replace('_', ' ', $name)).' : ';
			$information .= ($html) ? '<td valign="top">'.$value.'</td></tr>' : $value."\n";
		}
		// add form submission time
		$now = date('d M Y g:i a', time());
		$information .= ($html) ? '<tr><td valign="top">'.translate('cms_form_submission_time').'</td>' : translate('cms_form_submission_time').' : ';
		$information .= ($html) ? '<td valign="top">'.$now.'</td></tr>' : $now."\n";
		// add submission ip address
		$ip_address = $_SERVER['REMOTE_ADDR'];
		$information .= ($html) ? '<tr><td valign="top">'.translate('cms_form_submission_ip_address').'</td>' : translate('cms_form_submission_ip_address').' : ';
		$information .= ($html) ? '<td valign="top">'.$ip_address.'</td></tr>' : $ip_address."\n";

		$information .= ($html) ? '</table>' : '';
		return $information;

	}//end getFormInfo()


	/**
	* Clones certain specified components of the asset
	*
	* @param object		&$clone		the clone asset shell
	* @param array		$components	the wanted components to clone eg.
	* <pre>
	* 								Array(
	* 									'attributes',
	* 									'metadata_schemas',
	* 									'metadata',
	* 									'workflow',
	* 									'permissions',
	* 									'data',
	* 									'content_tags',
	* 									'roles',
	* 								);
	* 								or alternately
	* 								Array('all');
	* </pre>
	* @param boolean	$override	whether or not to override the existing permission, metadata schemas, workflow schemas with the new ones.
	*
	* @return boolean
	* @access public
	* @see asset::cloneComponents()
	*/
	function cloneComponents(&$clone, $components, $override=FALSE)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// Remove metadata schemas from form submission clone components
		$metadata_schemas_key = array_search('metadata_schemas', $components);
		if ($metadata_schemas_key !== FALSE) {
			unset($components[$metadata_schemas_key]);
		}

		if (!parent::cloneComponents($clone, $components, $override)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		if (in_array('attributes', $components) || in_array('all', $components)) {
			// send to a friend format
			$recip = unserialize($this->attr('recipient_emails_format'));

			if(array_key_exists('to',$recip)) {
				$recip = str_replace('%response_'.$this->id.'_', '%response_'.$clone->id.'_', $recip);
				$recip['to'] = str_replace('%response_'.$this->id.'_', '%response_'.$clone->id.'_', $recip['to']);
			} else {
				$recip = str_replace('%response_'.$this->id.'_', '%response_'.$clone->id.'_', $recip);
			}

			$rec = unserialize($this->attr('receipt_emails_format'));

			if (array_key_exists('to',$rec)) {
				$rec = str_replace('%response_'.$this->id.'_', '%response_'.$clone->id.'_', $rec);
				$rec['to'] = str_replace('%response_'.$this->id.'_', '%response_'.$clone->id.'_', $rec['to']);
			} else {
				$rec = str_replace('%response_'.$this->id.'_', '%response_'.$clone->id.'_', $rec);
			}

			$staf = unserialize($this->attr('staf_format'));

			if (array_key_exists('to',$staf)) {
				$staf = str_replace('%response_'.$this->id.'_', '%response_'.$clone->id.'_', $staf);
				$staf['to'] = str_replace('%response_'.$this->id.'_', '%response_'.$clone->id.'_', $staf['to']);
			} else {
				$staf = str_replace('%response_'.$this->id.'_', '%response_'.$clone->id.'_', $staf);
			}

			$sel = $this->attr('selective_emails');
			foreach ($sel as $key => $null) {
				$sel[$key]['address'] = str_replace('%response_'.$this->id.'_', '%response_'.$clone->id.'_', $sel[$key]['address']);
				$sel[$key]['assetid'] = str_replace($this->id.':q', $clone->id.':q', $sel[$key]['assetid']);
			}

			// field selections for STAF and receipt
			$staf_field = str_replace($this->id.':q', $clone->id.':q', $this->attr('staf_field'));
			$receipt_field = str_replace($this->id.':q', $clone->id.':q', $this->attr('receipt_field'));

			// save the information
			$clone->setAttrValue('recipient_emails_format', serialize($recip));
			$clone->setAttrValue('receipt_emails_format', serialize($rec));
			$clone->setAttrValue('staf_format', serialize($staf));
			$clone->setAttrValue('selective_emails', $sel);

			$clone->setAttrValue('staf_field', $staf_field);
			$clone->setAttrValue('receipt_field', $receipt_field);

			$clone->saveAttributes();

		}//end if

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end cloneComponents()


	/**
	* delete a question to this form
	*
	* @param object	&$question	the question object to delete
	*
	* @return boolean
	* @access public
	*/
	function deleteQuestion(&$question)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!parent::deleteQuestion($question)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		if ($this->attr('staf_field') == $question->id) {
			$this->setAttrValue('staf_field', '');
		}

		if ($this->attr('receipt_field') == $question->id) {
			$this->setAttrValue('receipt_field', '');
		}

		if (!$this->saveAttributes()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end deleteQuestion()


//--        POST-SUBMISSION ACTIONS        --//


	/**
	* Send all the emails when the form is completed
	*
	* @return boolean
	* @access public
	*/
	function finishForm()
	{
		// Perform submission actions which need to be executed before finish form
		if (!$this->performSubmissionActions(TRUE)) {
			// We had problems, print last page again
			// Print the current page or the Confirmation Page if we're confirming things
			$current_page = $this->getCurrentPageNumber();
			$total_pages = $this->getTotalPages();

			echo $this->getFormHeader();

			if (($current_page == $total_pages) && $this->attr('use_confirmation_page')) {
				echo $this->getConfirmationPageContents();
			} else {
				$this->printPageContentsBodycopy();
			}

			echo $this->getFormFooter();

			return FALSE;
		}

		parent::finishForm();
		$this->mailRecipients();
		$this->mailReceipt();
		$this->sendToFriends();

		// Perform other submission actions that are active
		$this->performSubmissionActions();

		// Perform selective actions
		$this->runSelectiveEmails();

		return TRUE;

	}//end finishForm()


	/**
	* Mail a receipt to the user if a receipt email field has been assigned in Email Options
	*
	* @return boolean
	* @access public
	*/
	function mailReceipt()
	{
		$email_field_assetid = $this->attr('receipt_field');
		if (empty($email_field_assetid)) return FALSE;

		$email_field = $GLOBALS['SQ_SYSTEM']->am->getAsset($email_field_assetid);
		if (empty($email_field)) return FALSE;
		$address = $email_field->getValue();

		$additional_reps = Array(
							'receipt_address'	=> $address,
						   );

		$email_format_attr = $this->getAttribute('receipt_emails_format');

		// append the 'To:' address with the address from the e-mail field
		$current_value = @unserialize($email_format_attr->value);
		$current_value['to'][] = $address;
		$email_format_attr->value = serialize($current_value);

		$this->sendEmail($email_format_attr, $additional_reps);

		return TRUE;

	}//end mailReceipt()


	/**
	* Send mail to recipients set in the backend
	*
	* @return boolean
	* @access public
	*/
	function mailRecipients()
	{
		$email_format_attr = $this->getAttribute('recipient_emails_format');
		$this->sendEmail($email_format_attr);
		return TRUE;

	}//end mailRecipients()


	/**
	* Returns array of file attachment assetids
	*
	* @return array
	* @access public
	*/
	function _getAttachments()
	{
		$questions = $this->getAllQuestions();
		$db = MatrixDAL::getDb();

		$file_assets = Array();
		$filenames = Array();

		foreach ($questions as $q_assetid => $question) {
			if ($question['question_type_code'] == 'form_question_type_file_upload') {
				$q_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($q_assetid);
				if (!is_null($q_asset) && $q_asset->attr('attach_to_mail')) {
					$file_asset_id = $q_asset->getUploadedFileId();
					if (!empty($file_asset_id)) {
						$file_assets[] = $file_asset_id;
					} else {
						// We have a permanent storage
						if (isset($q_asset->extra_data['filesystem_path']) && is_file($q_asset->extra_data['filesystem_path'])) {
							$filenames[] = $q_asset->extra_data['filesystem_path'];
						}
					}
				}
			}
		}

		return Array(
				'assets'	=> $file_assets,
				'filenames'	=> $filenames,
			   );

	}//end _getAttachments()


	/**
	* Mail a receipt to friends if a STAF field has been assigned in Email Options
	*
	* @return boolean
	* @access public
	*/
	function sendToFriends()
	{
		$email_field_assetid = $this->attr('staf_field');
		if (empty($email_field_assetid)) return FALSE;

		$email_field = $GLOBALS['SQ_SYSTEM']->am->getAsset($email_field_assetid);
		if (empty($email_field)) return FALSE;
		$addresses = preg_split('/[\s;,]+/',$email_field->getValue());

		$email_format_attr = $this->getAttribute('staf_format');

		// append the 'To:' address with the address from the e-mail field
		$current_value = @unserialize($email_format_attr->value);
		$current_value['to'] = array_merge($current_value['to'], $addresses);
		$email_format_attr->value = serialize($current_value);

		$this->sendEmail($email_format_attr);

		return TRUE;

	}//end sendToFriends()


	/**
	* Run selective emails application
	*
	* @return boolean
	* @access public
	*/
	function runSelectiveEmails()
	{
		// attribute which holds all the selective email rules
		$selective_emails = $this->attr('selective_emails');

		// these are where we store the recipients of each respective type of email...
		// if rules pass then this will contain who is receiving each email
		$recipients	= Array();
		$receipts	= Array();
		$stafs		= Array();

		for (reset($selective_emails); NULL !== ($k = key($selective_emails)); next($selective_emails)) {
			$sel_email =& $selective_emails[$k];

			// check to see whether it is valid in the first place
			$valid = TRUE;

			if (empty($sel_email['rules'])) $valid = FALSE;				// no rule parts!
			if (empty($sel_email['send'])) $valid = FALSE;				// type of mail is empty
			if (empty($sel_email['address'])) $valid = FALSE;			// address to send to is empty

			if ($valid) {

				// Short circuiting rules for this loop. If we are requiring ALL rule parts to succeed
				// then we start with a Pass outcome and exit on the first Fail. If we are only needing
				// ANY then we start with a Fail and exit on the first Pass.
				$require_all = ($sel_email['require'] == 'all');
				$passed = $require_all;

				// find the rule parts
				$rule_parts =& $sel_email['rules'];

				for (reset($rule_parts); NULL !== ($rule_key = key($rule_parts)); next($rule_parts)) {
					$rule =& $rule_parts[$rule_key];
					$q_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($rule['assetid']);

					// if this rule is broken due to a deleted question, the rule
					// will automatically fail
					if (!$q_asset) {
						$passed = FALSE;
						break;
					}

					$answer = $q_asset->getValue();

					$type_code = 'form_question_rule_type_'.$rule['name'];
					$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
					$rule_asset = new $type_code();
					$rule_passed = $rule_asset->evaluate($answer, $rule, $q_asset);

					// Short Circuit evaluation, see note above this loop.
					if ($require_all && !$rule_passed) {
						// require ALL, rule part FAILED => fail whole rule
						$passed = FALSE;
						break;
					} else if (!$require_all && $rule_passed) {
						// require ANY, rule part PASSED => pass whole rule
						$passed = TRUE;
						break;
					}

				}//end for rule parts

				if ($passed) {
					// parse addresses into an array in case multiples are sent
					$addresses = preg_split('/[\s;,]+/',$sel_email['address']);

					switch ($sel_email['send']) {
						case 'recipient':
							$recipients	= array_merge($recipients, $addresses);
						break;
						case 'receipt':
							$receipts	= array_merge($receipts, $addresses);
						break;
						case 'staf':
							$stafs		= array_merge($stafs, $addresses);
						break;
					}
				}

			}//end if valid

		}//end for selective email rules


		// recipient emails
		if (!empty($recipients)) {
			// only send one of each type of email to each recipient
			$recipients = array_unique($recipients);

			$email_format = $this->getAttribute('recipient_emails_format');

			// substitute generic keywords with email format-specific keywords
			$current_value = @unserialize($email_format->value);

			if (!empty($current_value)) {
				$current_value['to'] = $recipients;
				$email_format->value = serialize($current_value);
				$this->sendEmail($email_format);
			}
		}


		// receipt emails
		if (!empty($receipts)) {
			// only send one of each type of email to each recipient
			$receipts = array_unique($receipts);

			$email_format = $this->getAttribute('receipt_emails_format');

			// substitute generic keywords with email format-specific keywords
			$current_value = @unserialize($email_format->value);

			if (!empty($current_value)) {
				$current_value['to'] = $receipts;
				$email_format->value = serialize($current_value);

				$additional_reps = Array('receipt_address' => '');
				$this->sendEmail($email_format, $additional_reps);
			}
		}


		// send to a friend
		if (!empty($stafs)) {
			// only send one of each type of email to each recipient
			$stafs = array_unique($stafs);

			$email_format = $this->getAttribute('staf_format');

			// substitute generic keywords with email format-specific keywords
			$current_value = @unserialize($email_format->value);

			if (!empty($current_value)) {
				$current_value['to'] = $stafs;
				$email_format->value = serialize($current_value);
				$this->sendEmail($email_format);
			}
		}

		return TRUE;

	}//end runSelectiveEmails()


	/**
	* Common function for sending away an email format
	*
	* This function automatically replaces the common Thank You/email bodycopy
	* keywords. Additional keywords to replace can be sent through the
	* $additional_reps parameter.
	*
	* @param object	&$email_format_attr	(Asset_Attribute_Email_Format)
	*									The email format to send off
	* @param array	$replacements		(string => string) Additional
	*									replacements for a certain
	*									email (eg. receipt address)
	*
	* @return void
	* @access private
	*/
	function sendEmail(&$email_format_attr, $replacements=Array())
	{
		$current_value = @unserialize($email_format_attr->value);
		$filenames = Array();

		// return false if there is no-one to send to
		if (empty($current_value['to']) && empty($current_value['to_assetids'])) {
			return FALSE;
		}

		// substitute generic keywords with email format-specific keywords
		$current_value['html_format'] = str_replace(
			Array('%form_info%',      '%form_summary%'),
			Array('%form_info_html%', '%form_summary_html%'), $current_value['html_format']);
		$current_value['text_format'] = str_replace(
			Array('%form_info%',      '%form_summary%'),
			Array('%form_info_text%', '%form_summary_text%'), $current_value['text_format']);

		// add attachments to emails
		$current_value['attachments'] = Array();
		$filenames = Array();
		if (array_get_index($current_value, 'inc_attachments', FALSE)) {
			$attachments = $this->_getAttachments();
			$current_value['attachments'] = $attachments['assets'];
			$filenames = $attachments['filenames'];
		}

		$all_text = $current_value['html_format']
					.' '.$current_value['text_format']
					.' '.implode(', ', $current_value['to'])
					.' '.$current_value['from']
					.' '.array_get_index($current_value, 'reply_to', '')
					.' '.$current_value['subject'];

		$all_keywords = extract_keywords($all_text);

		if (!is_array($replacements)) $replacements = Array();

		foreach ($all_keywords as $keyword) {
			if (isset($replacements[$keyword])) continue;
			$replacements[$keyword] = $this->_getThankYouKeywordReplacement($keyword);
		}

		$email_format_attr->value = serialize($current_value);
		$email_format_attr->sendMail($replacements, $filenames);

	}//end sendEmail()


	/**
	* Perform any submission actions that are valid and active
	*
	* @param boolean	$before_submit	perform action before form gets submitted?
	* @return void
	*/
	public function performSubmissionActions($before_submit = FALSE)
	{
		$success = TRUE;
		$actions = $this->attr('actions');
		foreach ($actions as $action_id => $action) {
			$action_type = $action['type_code'];
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($action_type);

			// If need to perform 'before_submit' actions, skip those 'after_submit' action
			if ($before_submit && !isset($action['settings']['before_submit'])) continue;
			// If need to perform 'after_submit' actions, skip those 'before_submit' action
			if (!$before_submit && isset($action['settings']['before_submit'])) continue;

			if ($action['active'] && call_user_func(Array($action_type, 'isValid'), $this, $action['settings'])) {
				$success &= call_user_func(Array($action_type, 'execute'), $this, $action['settings'], $action_id);
			}
		}

		return $success;

	}//end performSubmissionActions()


	/**
	* Get appropriate keywords for Thank You bodycopy and emails
	*
	* Since referral assets have always been added at the Form_Email level,
	* additional keywords should be added here instead
	*
	* @param string	$keyword	keyword to replace
	*
	* @return string
	*/
	public function _getThankYouKeywordReplacement($keyword)
	{
		$full_keyword = $keyword;
		$keyword = parse_keyword($keyword, $modifiers);
		$prefix = $this->getPrefix();
		$replacement = NULL;

		// Referrer is used by several keywords below
		$referrer = (isset($_POST[$prefix.'_referral_url'])) ? $_POST[$prefix.'_referral_url'] : '';

		if ($keyword == 'referral_url') {
			$replacement = $referrer;
		} else if (preg_match('|^referring_asset_(.*)|', $keyword, $matches)) {
			// referring asset keyword, if we have a Matrix asset URL
			// eg. %referring_asset_assetid%
			if (!empty($referrer)) {
				// use the url to work out the referring asset
				$referrer = clean_url($referrer);

				$url_parts = parse_url($referrer);
				$protocol = array_get_index($url_parts, 'scheme');
				$path = array_get_index($url_parts, 'host').array_get_index($url_parts, 'path');

				$referrer_asset = $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL($protocol, $path, TRUE, TRUE);
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

		} else if ($keyword == 'youtube_video_link'){
			$replacement = (isset($this->form_action_keywords['youtube_video_link'])) ? $this->form_action_keywords['youtube_video_link'] : '';
		} else if ($keyword == 'youtube_video_id'){
			$replacement = (isset($this->form_action_keywords['youtube_video_id'])) ? $this->form_action_keywords['youtube_video_id'] : '';
		} else if ($keyword == 'youtube_video_published_date'){
			$replacement = (isset($this->form_action_keywords['youtube_video_published_date'])) ? $this->form_action_keywords['youtube_video_published_date'] : '';
		} else {
			$replacement = parent::_getThankYouKeywordReplacement($keyword);
		}

		// If keyword was not replaced, put it back to be replaced later
		if ($replacement == '%'.$keyword.'%') {
			$replacement = '%'.$full_keyword.'%';
		} else {
			$replace_keywords = Array(
									'assetid' 	=> $this->id,
									'call_fns'	=> Array ('_getThankYouKeywordReplacement', 'getKeywordReplacement'),
								);
			apply_keyword_modifiers($replacement, $modifiers, $replace_keywords);
		}

		return $replacement;

	}//end _getThankYouKeywordReplacement()

}//end class

?>
