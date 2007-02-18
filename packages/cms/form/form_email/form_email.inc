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
* $Id: form_email.inc,v 1.59 2007/02/18 23:35:12 rhoward Exp $
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
* @version $Revision: 1.59 $
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

		$additional_reps = Array(
							'receipt_address'	=> $address,
						   );

		$email_format_attr =& $this->getAttribute('receipt_emails_format');

		// append the 'To:' address with the address from the e-mail field
		$current_value = @unserialize($email_format_attr->value);
		$current_value['to'][] = $address;
		$email_format_attr->value = serialize($current_value);

		$this->_sendEmail($email_format_attr, $additional_reps);

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
		$this->_sendEmail($email_format_attr);
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

		$email_format_attr =& $this->getAttribute('staf_format');

		// append the 'To:' address with the address from the e-mail field
		$current_value = @unserialize($email_format_attr->value);
		$current_value['to'] = array_merge($current_value['to'], $addresses);
		$email_format_attr->value = serialize($current_value);

		$this->_sendEmail($email_format_attr);

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


		// recipient emails
		if (!empty($recipients)) {
			// only send one of each type of email to each recipient
			$recipients = array_unique($recipients);

			$email_format =& $this->getAttribute('recipient_emails_format');

			// substitute generic keywords with email format-specific keywords
			$current_value = @unserialize($email_format->value);

			if (!empty($current_value)) {
				$current_value['to'] = $recipients;
				$email_format->value = serialize($current_value);
				$this->_sendEmail($email_format);
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
				$email_format->value = serialize($current_value);

				$additional_reps = Array('receipt_address' => '');
				$this->_sendEmail($email_format, $additional_reps);
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
				$email_format->value = serialize($current_value);
				$this->_sendEmail($email_format);
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
	* @param array	$additional_reps	(string => string)
	*									Additional replacements for a certain
	*									email (eg. receipt address)
	*
	* @return void
	* @access private
	*/
	function _sendEmail(&$email_format_attr, $additional_reps=Array())
	{
		$current_value = @unserialize($email_format_attr->value);

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
		$current_value['attachments'] = $this->_getAttachments();

		// Get list of replacements
		$reps_list = array_merge(
									retrieve_keywords_replacements($current_value['html_format']),
									retrieve_keywords_replacements($current_value['text_format']),
									retrieve_keywords_replacements(implode(', ', $current_value['to'])),
									retrieve_keywords_replacements($current_value['from']),
									retrieve_keywords_replacements($current_value['subject'])
								);

		$reps_list = array_unique($reps_list);

		// ...and find their replacements
		foreach ($reps_list as $rep_name) {
			$reps[$rep_name] = $this->_getThankYouKeywordReplacement($rep_name);
		}

		$reps = array_merge($reps, $additional_reps);

		$email_format_attr->value = serialize($current_value);
		$email_format_attr->sendMail($reps);

	}//end _sendEmail()


}//end class

?>
