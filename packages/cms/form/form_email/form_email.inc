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
* $Id: form_email.inc,v 1.54 2006/11/30 22:11:56 lwright Exp $
*
*/


require_once dirname(__FILE__).'/../form/form.inc';
require_once SQ_FUDGE_PATH.'/general/www.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

/**
* Form_Email
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Revision: 1.54 $
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
	*
	* @return string
	* @access private
	*/
	function _getName($short_name=FALSE)
	{
		return $this->attr('name');

	}//end _getName()


	/**
	* Prints the standard form i.e not a wizard
	*
	* @return boolean
	* @access public
	*/
	function printStandard()
	{
		$prefix = $this->getPrefix();
		$referer = (isset($_SERVER['HTTP_REFERER'])) ? $_SERVER['HTTP_REFERER'] : '';
		?><div><input type="hidden" name="<?php echo $prefix; ?>_referral_url" value="<?php echo $referer; ?>" /></div><?php
		return parent::printStandard();

	}//end printStandard()


	/**
	* Send all the emails when the form is completed
	*
	* @return boolean
	* @access public
	*/
	function finishForm()
	{
		parent::finishForm();
		$this->mailRecipients();
		$this->mailReceipt();
		$this->sendToFriends();
		$this->runSelectiveEmails();

		return TRUE;

	}//end finishForm()


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
		$information .= ($html) ? '<tr><td valign="top">Submission Time</td>' : 'Submission Time : ';
		$information .= ($html) ? '<td valign="top">'.$now.'</td></tr>' : $now."\n";

		$information .= ($html) ? '</table>' : '';
		return $information;

	}//end getFormInfo()


	/**
	* Clones certain specified components of the asset
	*
	* @param object	&$clone		the clone asset shell
	* @param array	$components	the wanted components to clone eg.
	*							Array(
	*							'attributes',
	*							'metadata_shemas',
	*							'metadata',
	*							'workflow',
	*							'permissions',
	*							'data'
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

		// Remove metadata schemas from form submission clone components
		$metadata_schemas_key = array_search('metadata_schemas', $components);
		if ($metadata_schemas_key !== FALSE) {
			unset($components[$metadata_schemas_key]);
		}

		if (!parent::cloneComponents($clone, $components)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		if (in_array('attributes', $components) || in_array('all', $components)) {
			// send to a friend format
			$recip = unserialize($this->attr('recipient_emails_format'));
			$recip = str_replace('%response_'.$this->id.'_', '%response_'.$clone->id.'_', $recip);
			$rec = unserialize($this->attr('receipt_emails_format'));
			$rec = str_replace('%response_'.$this->id.'_', '%response_'.$clone->id.'_', $rec);
			$staf = unserialize($this->attr('staf_format'));
			$staf = str_replace('%response_'.$this->id.'_', '%response_'.$clone->id.'_', $staf);
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

		}

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

		$email_field =& $GLOBALS['SQ_SYSTEM']->am->getAsset($email_field_assetid);
		if (empty($email_field)) return FALSE;
		$address = $email_field->getValue();

		// get the form info (in both formats)
		$info = $this->getFormInfo(TRUE);
		$info_text = $this->getFormInfo(FALSE);

		// get the form summary (both formats)
		ob_start();
			$this->printSummary(TRUE);
			$body = ob_get_contents();
		ob_end_clean();

		ob_start();
			$this->printSummary(FALSE);
			$body_text = ob_get_contents();
		ob_end_clean();

		$reps = Array(
					'receipt_address'	=> $address,
					'form_summary_html'	=> $body,
					'form_summary_text'	=> $body_text,
					'form_info_html'	=> $info,
					'form_info_text'	=> $info_text,
				);

		if (!is_null($this->submission_asset)) {
			$reps['form_submission_id'] = $this->submission_asset->id;
		}

		// replacements of question responses
		$questions = $this->getQuestions();
		$sections =& $this->getSections();

		foreach ($questions as $q_id => $question) {
			$q_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($this->id.':q'.$q_id);
			$q_name = $q_asset->attr('name');
			$reps['response_'.$this->id.'_q'.$q_id] = $q_asset->getSummary();
		}

		foreach ($sections as $section) {
			$reps['section_title_'.$section->id] = $section->attr('name');
			$questions = $section->getQuestions();
			foreach ($questions as $q_id => $question) {
				$q_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($section->id.':q'.$q_id);
				$q_name = $section->attr('name').': '.$q_asset->attr('name');
				$reps['response_'.$section->id.'_q'.$q_id] = $q_asset->getSummary();
			}
		}

		$email_format_attr =& $this->getAttribute('receipt_emails_format');

		// append the 'To:' address with the address from the e-mail field
		$current_value = @unserialize($email_format_attr->value);
		$current_value['to'][] = $address;

		// substitute generic keywords with email format-specific keywords
		$current_value['html_format'] = str_replace(
			Array('%form_info%', '%form_summary%'),
			Array('%form_info_html%', '%form_summary_html%'), $current_value['html_format']);
		$current_value['text_format'] = str_replace(
			Array('%form_info%', '%form_summary%'),
			Array('%form_info_text%', '%form_summary_text%'), $current_value['text_format']);


		// add attachments to emails
		$current_value['attachments'] = Array();
		$current_value['attachments'] = $this->_getAttachments();

		$email_format_attr->value = serialize($current_value);

		$email_format_attr->sendMail($reps);

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
		$email_format_attr =& $this->getAttribute('recipient_emails_format');

		$current_value = @unserialize($email_format_attr->value);

		// return false if there is no-one to send to
		if (empty($current_value['to']) && empty($current_value['to_assetids'])) {
			return FALSE;
		}

		// get the form info (in both formats)
		$info = $this->getFormInfo(TRUE);
		$info_text = $this->getFormInfo(FALSE);

		// get the form summary
		ob_start();
			$this->printSummary(TRUE);
			$body = ob_get_contents();
		ob_end_clean();

		ob_start();
			$this->printSummary(FALSE);
			$body_text = ob_get_contents();
		ob_end_clean();

		$reps = Array(
					'form_summary_html'	=> $body,
					'form_summary_text'	=> $body_text,
					'form_info_html'	=> $info,
					'form_info_text'	=> $info_text,
				);

		if (!is_null($this->submission_asset)) {
			$reps['form_submission_id'] = $this->submission_asset->id;
		}

		// replacements of question responses
		$questions = $this->getQuestions();
		$sections =& $this->getSections();

		foreach ($questions as $q_id => $question) {

			$q_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($this->id.':q'.$q_id);
			$q_name = $q_asset->attr('name');
			$reps['response_'.$this->id.'_q'.$q_id] = $q_asset->getSummary();
		}

		foreach ($sections as $section) {
			$reps['section_title_'.$section->id] = $section->attr('name');
			$questions = $section->getQuestions();
			foreach ($questions as $q_id => $question) {
				$q_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($section->id.':q'.$q_id);
				$q_name = $section->attr('name').': '.$q_asset->attr('name');
				$reps['response_'.$section->id.'_q'.$q_id] = $q_asset->getSummary();
			}
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
		$current_value['attachments'] = $this->_getAttachments();

		$email_format_attr->value = serialize($current_value);

		$email_format_attr->sendMail($reps);

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
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$file_assets = Array();
		foreach ($questions as $q_assetid => $question) {
			if ($question['question_type_code'] == 'form_question_type_file_upload') {
				$q_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($q_assetid);
				if (!is_null($q_asset) && $q_asset->attr('attach_to_mail')) {
					$file_asset_id = $q_asset->getUploadedFileId();
					if (!is_null($file_asset_id)) {
						$file_assets[] = $file_asset_id;
					}
				}
			}
		}

		return $file_assets;

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

		$email_field =& $GLOBALS['SQ_SYSTEM']->am->getAsset($email_field_assetid);
		if (empty($email_field)) return FALSE;
		$addresses = preg_split('/[\s;,]+/',$email_field->getValue());

		// get the form info
		$info = $this->getFormInfo(TRUE);
		$info_text = $this->getFormInfo(FALSE);

		// get the form summary
		ob_start();
			$this->printSummary(TRUE);
			$body = ob_get_contents();
		ob_end_clean();

		ob_start();
			$this->printSummary(FALSE);
			$body_text = ob_get_contents();
		ob_end_clean();

		$prefix = $this->getPrefix();
		$referer = (isset($_POST[$prefix.'_referral_url'])) ? $_POST[$prefix.'_referral_url'] : '';

		$reps = Array(
					'form_summary_html'	=> $body,
					'form_summary_text'	=> $body_text,
					'form_info_html'	=> $info,
					'form_info_text'	=> $info_text,
					'referral_url'		=> $referer,
				);

		if (!is_null($this->submission_asset)) {
			$reps['form_submission_id'] = $this->submission_asset->id;
		}

		// replacements of question responses
		$questions = $this->getQuestions();
		$sections =& $this->getSections();

		foreach ($questions as $q_id => $question) {
			$q_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($this->id.':q'.$q_id);
			$q_name = $q_asset->attr('name');
			$reps['response_'.$this->id.'_q'.$q_id] = $q_asset->getSummary();
		}

		foreach ($sections as $section) {
			$reps['section_title_'.$section->id] = $section->attr('name');
			$questions = $section->getQuestions();
			foreach ($questions as $q_id => $question) {
				$q_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($section->id.':q'.$q_id);
				$q_name = $section->attr('name').': '.$q_asset->attr('name');
				$reps['response_'.$section->id.'_q'.$q_id] = $q_asset->getSummary();
			}
		}

		$email_format_attr =& $this->getAttribute('staf_format');

		// append the 'To:' address with the address from the e-mail field
		$current_value = @unserialize($email_format_attr->value);
		$current_value['to'] = array_merge($current_value['to'], $addresses);

		// substitute generic keywords with email format-specific keywords
		$current_value['html_format'] = str_replace(
			Array('%form_info%', '%form_summary%'),
			Array('%form_info_html%', '%form_summary_html%'), $current_value['html_format']);
		$current_value['text_format'] = str_replace(
			Array('%form_info%', '%form_summary%'),
			Array('%form_info_text%', '%form_summary_text%'), $current_value['text_format']);

		// add attachments to emails
		$current_value['attachments'] = Array();
		$current_value['attachments'] = $this->_getAttachments();

		$email_format_attr->value = serialize($current_value);

		$email_format_attr->sendMail($reps);

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
					$q_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($rule['assetid']);

					// if this rule is broken due to a deleted question, the rule
					// will automatically fail
					if (!$q_asset) {
						$passed = FALSE;
						break;
					}

					$answer = $q_asset->getValue();

					$type_code = 'form_question_rule_type_'.$rule['name'];
					$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
					$rule_asset =& new $type_code();
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

		// form info will be common amongst all emails so we might as well
		// capture it up the top of this thing

		// get the form info
		$info = $this->getFormInfo(TRUE);
		$info_text = $this->getFormInfo(FALSE);

		// get the form summary
		ob_start();
			$this->printSummary(TRUE);
			$body = ob_get_contents();
		ob_end_clean();

		ob_start();
			$this->printSummary(FALSE);
			$body_text = ob_get_contents();
		ob_end_clean();

		// keyword replacements
		$reps = Array(
					'form_summary_html'	=> $body,
					'form_summary_text'	=> $body_text,
					'form_info_html'	=> $info,
					'form_info_text'	=> $info_text,
				);

		if (!is_null($this->submission_asset)) {
			$reps['form_submission_id'] = $this->submission_asset->id;
		}

		// replacements of question responses
		$questions = $this->getQuestions();
		$sections =& $this->getSections();

		foreach ($questions as $q_id => $question) {
			$q_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($this->id.':q'.$q_id);
			$q_name = $q_asset->attr('name');
			$reps['response_'.$this->id.'_q'.$q_id] = $q_asset->getSummary();
		}

		foreach ($sections as $section) {
			$reps['section_title_'.$section->id] = $section->attr('name');
			$questions = $section->getQuestions();
			foreach ($questions as $q_id => $question) {
				$q_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($section->id.':q'.$q_id);
				$q_name = $section->attr('name').': '.$q_asset->attr('name');
				$reps['response_'.$section->id.'_q'.$q_id] = $q_asset->getSummary();
			}
		}

		// recipient emails
		if (!empty($recipients)) {
			// only send one of each type of email to each recipient
			$recipients = array_unique($recipients);

			$email_format =& $this->getAttribute('recipient_emails_format');

			// substitute generic keywords with email format-specific keywords
			$current_value = @unserialize($email_format->value);

			if (!empty($current_value)) {
				$current_value['to'] = $recipients;

				$current_value['html_format'] = str_replace(
					Array('%form_info%', '%form_summary%'),
					Array('%form_info_html%', '%form_summary_html%'), $current_value['html_format']);
				$current_value['text_format'] = str_replace(
					Array('%form_info%', '%form_summary%'),
					Array('%form_info_text%', '%form_summary_text%'), $current_value['text_format']);

				// add attachments to emails
				$current_value['attachments'] = Array();
				$current_value['attachments'] = $this->_getAttachments();

				$email_format->value = serialize($current_value);

				$email_format->sendMail($reps);
			}
		}

		// receipt emails
		if (!empty($receipts)) {
			// only send one of each type of email to each recipient
			$receipts = array_unique($receipts);

			$email_format =& $this->getAttribute('receipt_emails_format');

			// substitute generic keywords with email format-specific keywords
			$current_value = @unserialize($email_format->value);

			if (!empty($current_value)) {
				$current_value['to'] = $receipts;

				$current_value['html_format'] = str_replace(
					Array('%form_info%', '%form_summary%'),
					Array('%form_info_html%', '%form_summary_html%'), $current_value['html_format']);
				$current_value['text_format'] = str_replace(
					Array('%form_info%', '%form_summary%'),
					Array('%form_info_text%', '%form_summary_text%'), $current_value['text_format']);

				// add attachments to emails
				$current_value['attachments'] = Array();
				$current_value['attachments'] = $this->_getAttachments();

				$email_format->value = serialize($current_value);

				$email_format->sendMail($reps);
			}
		}

		// send to a friend
		if (!empty($stafs)) {
			// only send one of each type of email to each recipient
			$stafs = array_unique($stafs);

			$email_format =& $this->getAttribute('staf_format');

			// substitute generic keywords with email format-specific keywords
			$current_value = @unserialize($email_format->value);

			if (!empty($current_value)) {
				$current_value['to'] = $stafs;

				$current_value['html_format'] = str_replace(
					Array('%form_info%', '%form_summary%'),
					Array('%form_info_html%', '%form_summary_html%'), $current_value['html_format']);
				$current_value['text_format'] = str_replace(
					Array('%form_info%', '%form_summary%'),
					Array('%form_info_text%', '%form_summary_text%'), $current_value['text_format']);

				// add attachments to emails
				$current_value['attachments'] = Array();
				$current_value['attachments'] = $this->_getAttachments();

				$email_format->value = serialize($current_value);

				$email_format->sendMail($reps);
			}
		}

		return TRUE;

	}//end runSelectiveEmails()


}//end class

?>
