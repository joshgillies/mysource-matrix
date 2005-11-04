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
* $Id: bulkmail_job.inc,v 1.12 2005/11/04 02:50:23 rong Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/bulkmail/bulkmail_post_office/bulkmail_post_office.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';


/**
* Bulkmail_Job
*
* Purpose
*
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Rayn Ong <rong@squiz.net>
*
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class Bulkmail_Job extends Bulkmail_Post_Office
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Bulkmail_Job($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Asset($assetid);

	}//end constructor


	/**
	* Returns parent asset Bulkmail Post Office or system default Post Office
	*
	* @return object
	* @access public
	*/
	function &getPostOffice()
	{
		$post_office = null;
		$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'bulkmail_post_office', true);
		if (!empty($parents)) {
			$assetid = array_pop(array_keys($parents));
			// make sure the parent post office is not in trash
			if (!$GLOBALS['SQ_SYSTEM']->am->assetInTrash($assetid)) {
				$post_office =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			}
		}

		// use the system default post office if none is selected
		if (is_null($post_office)) {
			$post_office =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_post_office_default');
		}

		return $post_office;

	}//end getPostOffice()


	/**
	* Sends the preview bulkmail content using given PEAR mail
	*
	* @param array	$recipient	array of email addresses, passed in from the backend interface
	*
	* @return boolean
	* @access public
	*/
	function sendPreviewMail($recipient)
	{
		if (!$this->isValidJob()) {
			trigger_localised_error('BML0013', E_USER_WARNING, $this->id);
			return false;
		}

		// Create the mail object using server_details as parameter
		$post_office = $this->getPostOffice();
		$server_details = $post_office->attr('server_details');
		$driver = array_get_index($server_details, 'driver', '');
		$mail_object =& Mail::factory($driver, $server_details);

		// unable to create mail object
		if (PEAR::isError($mail_object)) {
			trigger_localised_error('BML0001', E_USER_WARNING, $mail_object->getMessage());
			return false;
		}

		// store current session info, so that current user will not get errors
		$old_session = $_SESSION;

		// generate content as pre-selected user
		$content_details = $this->attr('content_details');
		$job_path = $post_office->data_path.'/.data/'.$this->id;
		require_once SQ_PACKAGES_PATH.'/bulkmail/bulk_mailer.inc';
		Bulk_Mailer::cacheContent($content_details, $job_path);
		$content = file_to_string($job_path.'/cache/content_file.html');

		// restore session info
		$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();
		$_SESSION = $old_session;

		// get header details
		$setting = $this->attr('details_setting');
		$use_post_office_header = array_get_index($setting, 'use_post_office_header', 0);
		if ($use_post_office_header) {
			$header_details = $post_office->attr('header_details');
		} else {
			$header_details = $this->attr('header_details');
		}
		if (isset($header_details['Subject'])) {
			// add 'Preview' to the subject header
			$header_details['Subject'] = $header_details['Subject'].' (Preview)';
		}

		$mime =& new Mail_mime();
		//$mime->setTxtBody($textMessage);
		$mime->setHtmlBody($content);
		$body = @$mime->get();
		$headers = @$mime->headers($header_details);
		$status = $mail_object->send($recipient, $headers, $body);
		if (PEAR::isError($status)) {
			// unable to send bulkmail
			trigger_localised_error('BML0002', E_USER_WARNING, $status->getMessage());
			return false;
		}

		return true;

	}//end sendPreviewMail()


	/**
	* Returns an array of errors if job is not valid or true if we can set this job to live
	*
	* @return mixed array|boolean
	* @access public
	*/
	function isValidJob()
	{
		$error = Array();

		// if parent not set, use the system default post office
		$post_office =& $this->getPostOffice();
		if (!is_null($post_office)) {
			// get server details from post office
			$server_details = $post_office->attr('server_details');
			$driver = array_get_index($server_details, 'driver', '');
			$mail_object =& Mail::factory($driver, $server_details);
			if (PEAR::isError($mail_object)) {
				// cannot create a mail instance
				$type = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($post_office->type(), 'name');
				$error[] = translate('bulkmail_warning_po_server_details', $mail_object->getMessage(), $type, $post_office->name, $post_office->id);
			}
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($mail_object);
		}

		// no recipient
		// note: if expand $recipient, its equivalent to calling getAllRecipients() (intensive process)
		$recipients = $this->attr('recipients');
		if (empty($recipients)) {
			$error[] = translate('bulkmail_warning_po_recipient_not_set');
		}

		// get header details from post office or job
		// check if minimum required fields (from field) are supplied
		$setting = $this->attr('details_setting');
		$use_post_office_header = array_get_index($setting, 'use_post_office_header', 0);
		if ($use_post_office_header) {
			$header_details = $post_office->attr('header_details');
		} else {
			$header_details = $this->attr('header_details');
		}
		if (!isset($header_details['from']) && !isset($header_details['From'])) {
			$error[] = translate('bulkmail_warning_header_from_field');
		}

		$content_details = $this->attr('content_details');
		$page_id = array_get_index($content_details, 'content_page', 0);
		$type = array_get_index($content_details, 'type', '');
		$generate_as = array_get_index($content_details, 'pre_selected_user', '');

		// no content page
		if (!$page_id) {
			$error[] = translate('bulkmail_warning_content_page_not_set');
		}
		// pre-selected user not set
		if (!$generate_as) {
			$error[] = translate('bulkmail_warning_content_generate_as_not_set');
		} else {
			$user =& $GLOBALS['SQ_SYSTEM']->am->getAsset($generate_as);
			if (!$user->canLogin()) {
				$type = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($user->type(), 'name');
				$error[] = translate('bulkmail_warning_content_generate_as_login_failed', $type, $user->name, $user->id);
			}
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($user);
		}
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($post_office);

		// pass the error array for error reporting if we need it
		return (empty($error)) ? true : false;

	}//end isValidJob()


	/**
	* Provides extra checking and disable invalid status change once the job is in progress
	*
	* @param int		$new_status		the new status to set for this asset
	* @param boolean	$update_parents	Specify if we want to update the parent as well
	*
	* @return boolean
	* @access public
	*/
	function processStatusChange($new_status, $update_parents=true)
	{
		// get the current state from the bmail queue db table
		$bm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');
		$table = $bm->getQueuedJobs($this->id);

		// cannot change the status of the job is in the db table
		if (!empty($table)) return false;

		// user set the job to live
		if ($new_status == SQ_STATUS_LIVE) {
			// only set job to live if it is valid
			if ($this->isValidJob()) {
				if (!$bm->addJob($this)) {
					trigger_localised_error('BML0012', E_USER_WARNING, $this->id);
					return false;
				}
			} else {
				return false;
			}
		}

		return parent::processStatusChange($new_status, $update_parents);

	}//end processStatusChange()


}//end class
?>