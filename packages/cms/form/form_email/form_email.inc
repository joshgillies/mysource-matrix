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
* $Id: form_email.inc,v 1.24 2004/07/01 00:52:18 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once  dirname(__FILE__).'/../form/form.inc';
require_once SQ_FUDGE_PATH .'/general/www.inc';
require_once SQ_FUDGE_PATH .'/general/file_system.inc';

/**
* Form_Email
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
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
	
	}//end Form_Email()


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

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return $linkid;

	}//end create()


	/**
	* Returns name of the asset
	*
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @see Asset::_getName()
	* @access private
	*/
	function _getName($short_name=false)
	{
		return $this->attr('name');
	
	}//end _getName();


	/**
	* sends all the emails when the form is completed
	*
	* @access public
	* @return boolean
	*/
	function finishForm()
	{
		parent::finishForm();
		$this->mailRecipients();
		$this->mailReceipt();
		$this->sendToFriends();
		$this->runSelectiveEmails();
		$this->logSubmission();

		return true;

	}//end finishForm()


	/**
	* returns form information (site name, site id, page name, page id, url, submission date)
	*
	* @param boolean	$html	return html formatted or not 
	*
	* @access public
	* @return string $information
	*/
	function getFormInfo($html = false)
	{
		$info = parent::getFormInfo();
		$title = 'Form Information'; 
		$information  = ($html) ? '<b>'.$title.'</b><br />' : $title."\n\n";
		$information .= ($html) ? "<table>" : '';
		
		foreach ($info as $name => $value) {
			$information .= ($html) ? '<tr><td>'.ucwords(str_replace('_', ' ', $name)).'</td>' : ucwords(str_replace('_', ' ', $name))." : ";
			$information .= ($html) ? '<td>'.$value.'</td></tr>' : $value."\n";
		}
		$information .= ($html) ? "</table>" : '';
		return $information;

	}//end getFormInfo()


	/**
	* mails a receipt to the user if a receipt email field has been assigned in Email Options
	*
	* @access public
	* @return boolean
	*/
	function mailReceipt()
	{
		$email_field_assetid = $this->attr('receipt_field');

		if (empty($email_field_assetid)) {
			return false;
		} else {
			$email_field = $GLOBALS['SQ_SYSTEM']->am->getAsset($email_field_assetid);
			$address = $email_field->getValue();

			// get the form info (in both formats)
			$info = $this->getFormInfo(true);
			$info_text = $this->getFormInfo(false);

			// get the form summary (both formats)
			ob_start();
			$this->printSummary(true);
			$body = ob_get_contents();
			ob_end_clean();

			ob_start();
			$this->printSummary(false);
			$body_text = ob_get_contents();
			ob_end_clean();

			$reps = Array('receipt_address' => $address, 'form_summary_html' => $body, 'form_summary_text' => $body_text, 'form_info_html' => $info, 'form_info_text' => $info_text);

			$email_format_attr = $this->getAttribute('receipt_emails_format');

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

			$email_format_attr->value = serialize($current_value);

			$email_format_attr->sendMail($reps);
		}

		return true;

	}//end mailReceipt()

	
	/**
	* sends mail to recipients set in the backend
	* 
	* @access public
	* @return boolean
	*/
	function mailRecipients()
	{
		$email_format_attr = $this->getAttribute('receipt_emails_format');

		$current_value = @unserialize($email_format_attr->value);

		// return false if there is no-one to send to
		if (empty($current_value['to'])) return false;

		// get the form info (in both formats)
		$info = $this->getFormInfo(true);
		$info_text = $this->getFormInfo(false);

		// get the form summary
		ob_start();
		$this->printSummary(true);
		$body = ob_get_contents();
		ob_end_clean();

		ob_start();
		$this->printSummary(false);
		$body_text = ob_get_contents();
		ob_end_clean();


		$reps = Array('form_summary_html' => $body, 'form_summary_text' => $body_text, 'form_info_html' => $info, 'form_info_text' => $info_text);

		// substitute generic keywords with email format-specific keywords
		$current_value['html_format'] = str_replace(
			Array('%form_info%', '%form_summary%'), 
			Array('%form_info_html%', '%form_summary_html%'), $current_value['html_format']);
		$current_value['text_format'] = str_replace(
			Array('%form_info%', '%form_summary%'), 
			Array('%form_info_text%', '%form_summary_text%'), $current_value['text_format']);

		$email_format_attr->value = serialize($current_value);

		$email_format_attr->sendMail($reps);

		return true; 

	}//end mailRecipients()


	/**
	* mails a receipt to friends if a STAF field has been assigned in Email Options
	*
	* @access public
	* @return boolean
	*/
	function sendToFriends()
	{
		$email_field_assetid = $this->attr('staf_field');

		if (empty($email_field_assetid)) {
			return false;
		} else {
			$email_field = $GLOBALS['SQ_SYSTEM']->am->getAsset($email_field_assetid);
			$addresses = preg_split('/[\s;,]+/',$email_field->getValue());

			// get the form info
			$info = $this->getFormInfo(true);
			$info_text = $this->getFormInfo(false);

			// get the form summary
			ob_start();
			$this->printSummary(true);
			$body = ob_get_contents();
			ob_end_clean();

			ob_start();
			$this->printSummary(false);
			$body_text = ob_get_contents();
			ob_end_clean();

			$reps = Array('form_summary_html' => $body, 'form_summary_text' => $body_text, 'form_info_html' => $info, 'form_info_text' => $info_text);

			$email_format_attr = $this->getAttribute('staf_format');

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

			$email_format_attr->value = serialize($current_value);

			$email_format_attr->sendMail($reps);
		}

		return true;

	}//end sendToFriends()


	/**
	* runs selective emails application
	*
	* @access public
	* @return boolean
	*/
	function runSelectiveEmails()
	{
		// Attribute which holds all the selective email rules
		$selective_emails = $this->attr('selective_emails');

		// These are where we store the recipients of each respective type of email...
		// IF rules pass then this will contain who is receiving each email
		$recipients	= Array();
		$receipts	= Array();
		$stafs		= Array();

		for (reset($selective_emails); null !== ($k = key($selective_emails)); next($selective_emails)) {
			$sel_email =& $selective_emails[$k];
			
			// check to see whether it is valid in the first place
			$valid = true;

			if (empty($sel_email['rules'])) $valid = false;				// no rule parts!
			if (empty($sel_email['send'])) $valid = false;				// type of mail is empty
			if (empty($sel_email['address'])) $valid = false;			// address to send to is empty

			// we are still sweet, play on
			if ($valid) {

				// Short circuiting rules for this loop. If we are requiring ALL rule parts to succeed
				// then we start with a Pass outcome and exit on the first Fail. If we are only needing
				// ANY then we start with a Fail and exit on the first Pass.
				$require_all = ($sel_email['require'] == 'all');
				$passed = ($require_all ? true : false);
				
				// find the rule parts
				$rule_parts =& $sel_email['rules'];

				for (reset($rule_parts); null !== ($rule_key = key($rule_parts)); next($rule_parts)) {
					$rule =& $rule_parts[$rule_key];
					$q_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($rule['assetid']);
					$answer = $q_asset->getValue();

					$type_code = 'form_question_rule_type_' . $rule['name'];

					if ($rule['comparison_question_id']) {
						$comparison = &$GLOBALS['SQ_SYSTEM']->am->getAsset($rule['comparison_question_id']);
					} else {
						$comparison = $rule['value'];
					}

					$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
					$attributes = eval('return '.$type_code.'::getAttributes();');

					if (!$attributes['no_comparison']) {		// comparison value required!
						$rule_passed = eval('return '.$type_code.'::valueTest($answer, $rule["operator"], $comparison);');
					} else {
						$rule_passed = eval('return '.$type_code.'::valueTest($answer, $rule["operator"]);');
					}

					// Short Circuit evaluation, see note above this loop.
					if ($require_all && !$rule_passed) {
						// Require ALL, rule part FAILED => fail whole rule
						$passed = false;
						break;
					} elseif (!$require_all && $rule_passed) {
						// Require ANY, rule part PASSED => pass whole rule
						$passed = true;
						break;
					}

				}//end for-rule parts

				// Rule Passed, OMG!!
				if ($passed) {
					// parse addresses into an array in case multiples are sent
					$addresses = preg_split('/[\s;,]+/',$sel_email['address']);

					switch($sel_email['send']) {
						case 'recipient':
							$recipients	= array_merge($recipients, $addresses);
							break;
						case 'receipt':
							$receipts	= array_merge($receipts, $addresses);
							break;
						case 'staf':
							$stafs		= array_merge($stafs, $addresses);
							break;
					}//end switch

				}//end if passed

			}//end if valid

		}//end selective email rules

		// Form info will be common amongst all emails so we might as well capture it up the top
		// of this thing

		// get the form info
		$info = $this->getFormInfo(true);
		$info_text = $this->getFormInfo(false);

		// get the form summary
		ob_start();
		$this->printSummary(true);
		$body = ob_get_contents();
		ob_end_clean();

		ob_start();
		$this->printSummary(false);
		$body_text = ob_get_contents();
		ob_end_clean();

		// keyword replacements
		$reps = Array('form_summary_html' => $body, 'form_summary_text' => $body_text, 'form_info_html' => $info, 'form_info_text' => $info_text);

		// recipient emails
		if (!empty($recipients)) {
			// only send one of each type of email to each recipient
			$recipients = array_unique($recipients);

			$email_format = $this->getAttribute('recipient_emails_format');

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

				$email_format->value = serialize($current_value);

				$email_format->sendMail($reps);
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

				$current_value['html_format'] = str_replace(
					Array('%form_info%', '%form_summary%'), 
					Array('%form_info_html%', '%form_summary_html%'), $current_value['html_format']);
				$current_value['text_format'] = str_replace(
					Array('%form_info%', '%form_summary%'), 
					Array('%form_info_text%', '%form_summary_text%'), $current_value['text_format']);

				$email_format->value = serialize($current_value);

				$email_format->sendMail($reps);
			
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

				$current_value['html_format'] = str_replace(
					Array('%form_info%', '%form_summary%'), 
					Array('%form_info_html%', '%form_summary_html%'), $current_value['html_format']);
				$current_value['text_format'] = str_replace(
					Array('%form_info%', '%form_summary%'), 
					Array('%form_info_text%', '%form_summary_text%'), $current_value['text_format']);

				$email_format->value = serialize($current_value);

				$email_format->sendMail($reps);

			}

		}

		return true;

	}//end runSelectiveEmails()


}//end class
?>
