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
* $Id: form_email.inc,v 1.14 2004/05/19 23:42:29 lwright Exp $
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
		$this->mailReceipt();
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
	* mails a receipt to the user if a email special infomation field has been assigned
	*
	* @access public
	* @return boolean
	*/
	function mailReceipt()
	{
		$email_format = $this->attr('receipt_emails_format');

		$email_field_assetid = $this->attr('receipt_field');

		if (empty($email_field_assetid)) {
			return false;
		} else {
			$email_field = $GLOBALS['SQ_SYSTEM']->am->getAsset($email_field_assetid);
			$address = $email_field->getValue();

			// get the form info
			$info = $this->getFormInfo(true);

			// get the form summary
			ob_start();
			$this->printSummary(true);
			$body = ob_get_contents();
			ob_end_clean();

			$reps = Array('receipt_address' => $address, 'form_summary' => $body, 'form_info' => $info);

			$email_format_attr = $this->getAttribute('receipt_emails_format');

			// append the 'To:' address with the address from the e-mail field
			$current_value = @unserialize($email_format_attr->value);
			$current_value['to'][] = $address;
			$email_format_attr->value = serialize($current_value);
			pre_echo($email_format_attr);

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
		$email_format = $this->attr('recipient_emails_format');

		// return false if there is no-one to send to
		if (empty($email_format['to'])) return false;

		// get the form info
		$info = $this->getFormInfo(true);

		// get the form summary
		ob_start();
		$this->printSummary(true);
		$body = ob_get_contents();
		ob_end_clean();


		$reps = Array('form_summary' => $body, 'form_info' => $info);

		$email_format = $this->getAttribute('recipient_emails_format');
		$email_format->sendMail($reps);

		return true;

	}//end mailRecipients()


}//end class
?>
