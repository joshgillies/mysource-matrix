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
* $Id: form_email.inc,v 1.13 2004/05/17 05:12:34 lwright Exp $
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
	* @access public
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
		$this->logSubmission();
		//$this->mailReceipt();
		//$this->SendToFriends();

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
	* mails a receipt to the user if a email special infomation field has been assigned
	*
	* @access public
	* @return boolean
	*/
	function mailReceipt()
	{
		if (!$this->attr('receipt')) return false;
		if (!$to_address = $this->getSpecialInformation('email')) return false;

		$from_address = $this->attr('receipt_from_address');
		$subject      = ($this->attr('receipt_subject')) ? $this->attr('receipt_subject') : 'Form Submission';
		$allow_html   = ($this->attr('receipt_allow_html')) ? true : false;
		$body         = $this->attr('receipt_body');
	
		if ($this->attr('receipt_send_answers')) {
			ob_start();
			$this->printSummary($allow_html);
			$body .= ob_get_contents();
			ob_end_clean();
		}
		$extras = ($allow_html) ? "\nContent-Type: text/html; charset=iso-8859-1" : '';
		
		mail($to_address, $subject, $body, 'From :'.$from_address.$extras);

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
		// return false if there is no-one to send to
		if (!$this->attr('recipient_emails')) return false;

		$addresses = split("\r\n", $this->attr('recipient_emails'));
		
		$allow_html = ($this->attr('recipient_allow_html')) ? true : false;
		$terminator = $allow_html ? '<BR />' : "\n"; // i'll be back

		// if this email is in html, add some nice html tags
		$body = ($allow_html) ? "<html>\n\t<head><title></title></head>\n\t</body>" : '';
		$body .= $this->attr('recipient_body').$terminator.$terminator;

		// get the form info if they want it
		if ($this->attr('recipient_send_info')) {
			$body .= $this->getFormInfo($allow_html).$terminator;
		}

		// check to see if we want to send the answers to the recipient
		if ($this->attr('recipient_send_answers')) {
			ob_start();
			$this->printSummary($allow_html);
			$body .= ob_get_contents();
			ob_end_clean();
		}
		$body .= $terminator.$terminator;

		// end the html
		$body .= ($allow_html) ? "</body></table>" : '';

		// send a html header if we need to
		$extras = ($allow_html) ? "\nContent-Type: text/html; charset=iso-8859-1" : '';

		$subject = ($this->attr('recipient_subject')) ? $this->attr('recipient_subject') : 'Form Submission';
	
		// special information address
		//$si_address = $this->getSpecialInformation('email'); 
		/*if ($si_address === '')*/ $si_address = false; 

		// spam... baby, spam!
		foreach ($addresses as $address) {
			// make sure these are valid emails, otherwise, just strip them out
			if (!valid_email(trim($address))) continue;
			
			$from_address = ($si_address) ? $si_address : trim($address);
			mail(trim($address), $subject, $body, 'From: '.$from_address.$extras);
		}
		
		return true;

	}//end mailRecipients()


	/**
	* Sends a message to a friend if there is a special information field set up to do so
	*
	* @access public
	* @return boolean
	*/
	function sendToFriends()
	{
		if ($friends = $this->getSpecialInformation('friend')) {
			$message = $this->attr('staf_message');
			$subject = ($this->attr('staf_subject')) ? $this->attr('staf_subject') : 'Message From a Friend';
			$from_address = $this->attr('staf_from_address');

			if (!$message) return false;

			$friends = explode(',', $friends);

			foreach ($friends as $friend) {
				if (!valid_email(trim($friend))) continue;
				mail(trim($friend), $subject, $message, 'From: '.$from_address);
			}
			return true;
		}
		return false;
	
	}//end sendToFriends()


}//end class
?>
