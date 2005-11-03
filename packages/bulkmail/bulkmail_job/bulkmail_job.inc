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
* $Id: bulkmail_job.inc,v 1.8 2005/11/03 01:08:02 rong Exp $
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
* @version $Revision: 1.8 $
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
	* Sends the bulkmail message using given PEAR mail driver params
	*
	* @param object	&$asset		the asset to which we belong
	* @param mixed	$recipient	an array or a string with comma separated recipients.
	*
	* @return boolean
	* @access public
	*/
	function sendPreviewMail(&$asset, $recipient)
	{
			$post_office = $this->getPostOffice();
			if (!is_null($post_office)) {
					return $post_office->sendPreviewMail($this, $recipient);
			} else {
					// error handling in parent's sendPreviewMail
					return parent::sendPreviewMail($this, $recipient);
			}

	}//end sendPreviewMail()


	/**
	* Returns an array of errors or empty array if we can set this job to live
	*
	* @return array
	* @access public
	*/
	function isValidJob()
	{
		$error = Array();

		// is this job linked under a post office?
		$post_office =& $this->getPostOffice();
		if (is_null($post_office)) {
			//$error[] = "Post Office Error: Post Office not set.\n";
			// also Default System Post Office not set
			$error[] = translate('bulkmail_warning_po_post_office_not_set');
		} else {
			// get server details from post office
			$server_details = $post_office->attr('server_details');
			$driver = array_get_index($server_details, 'driver', '');
			$mail_object =& Mail::factory($driver, $server_details);
			if (PEAR::isError($mail_object)) {
				// cannot create a mail instance
				//$error[] = "Post Office Server Details Error: ".$mail_object->getMessage().".\n";
				$type = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($post_office->type(), 'name');
				$error[] = translate('bulkmail_warning_po_server_details', $mail_object->getMessage(), $type, $post_office->name, $post_office->id);
			}
		}

		// no recipient
		// note: if expand $recipient, its equivalent to calling getRecipients() again (intensive process)
		$recipients = $this->attr('recipients');
		if (empty($recipients)) {
			//$error[] = "Post Office Error: Recipients no set.\n";
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
			//$error[] = "Header Details Error: Missing 'From' field\n";
			$error[] = translate('bulkmail_warning_header_from_field');
		}

		$content_details = $this->attr('content_details');
		$page_id = array_get_index($content_details, 'content_page', 0);
		$type = array_get_index($content_details, 'type', '');
		$generate_as = array_get_index($content_details, 'pre_selected_user', '');

		// no content page
		if (!$page_id) {
			//$error[] = "Content Details Error: Content Page not set.\n";
			$error[] = translate('bulkmail_warning_content_page_not_set');
		}
		// pre-selected user not set
		if (!$generate_as) {
			//$error[] = "Content Details Error: Bulkmail User not set for group-based content generation method.\n";
			$error[] = translate('bulkmail_warning_content_generate_as_not_set');
		} else {
			$user =& $GLOBALS['SQ_SYSTEM']->am->getAsset($generate_as);
			if (!$user->canLogin()) {
				$type = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($user->type(), 'name');
				//$error[] = "Content Details Error: Unable to generate content as $type \"".$user->name."\" (Id #$generate_as), user is either in trash or not live.\n";
				$error[] = translate('bulkmail_warning_content_generate_as_login_failed', $type, $user->name, $user->id);
			}
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($user);
		}

		return $error;

	}//end isValidJob()


	/**
	* Provides extra checking and disable invalid status change once the job is live and running
	*
	* @param int		$new_status		the new status to set for this asset
	* @param boolean	$update_parents	Specify if we want to update the parent as well
	*
	* @return boolean
	* @access public
	*/
	function processStatusChange($new_status, $update_parents=true)
	{
		// TODO: more robust pause/resume handling here
		$old_status = $this->status;

		// get the current state from the bmail queue db table
		$bm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');
		$table = $bm->getQueuedJobs($this->id);
		if (!empty($table)) {
			$state = $table[$this->id]['status'];
		} else {
			$state = BML_JOB_STATE_NOT_RUNNING;
		}

		// once the job is live, the only way to change it's status is by cancelling this job from the backend
		if ($old_status == SQ_STATUS_LIVE && $state == BML_JOB_STATE_NOT_RUNNING) {
			return parent::processStatusChange($new_status, $update_parents);

		// if the job is not live, only set it to live if it is valid
		} else {
			$errors = $this->isValidJob();
			if (empty($errors)) {
				// add entry to db
				if ($state == BML_JOB_STATE_NOT_RUNNING) {
					if (!$bm->addJob($this)) {
						trigger_error("processStatusChange: addJob($this->id) failed.\n");
						return false;
					}
				}
				// set the job to live
				return parent::processStatusChange($new_status, $update_parents);
			}
		}

		return false;

	}//end processStatusChange()


}//end class
?>