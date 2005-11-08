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
* $Id: bulkmail_job.inc,v 1.16 2005/11/08 01:42:07 rong Exp $
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
* @version $Revision: 1.16 $
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
			$post_office =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');
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
		// check if this job is valid
		$post_office = $this->getPostOffice();
		$data_dir = $post_office->data_path.'/.data/'.$this->id;
		if (!is_dir($data_dir)) create_directory($data_dir);
		$bm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');
		$bm->initDetailsFile($this, $post_office, $data_dir);
		if (!$bm->isValidJob($this->id)) {
			trigger_localised_error('BML0013', E_USER_WARNING, $this->id);
			return false;
		}

		// create the mail object
		$server_details = $post_office->attr('server_details');
		$driver = array_get_index($server_details, 'driver', '');
		$mail_object =& Mail::factory($driver, $server_details);

		// store current session info, so that current user will not get errors
		$old_session = $_SESSION;

		// generate content as pre-selected user
		$content_id = $this->attr('content_id');
		$content_gen_as = $this->attr('content_gen_as');
		$content_design = $this->attr('content_design');
		$content_layout = $this->attr('content_layout');
		require_once SQ_PACKAGES_PATH.'/bulkmail/bulk_mailer.inc';
		Bulk_Mailer::cacheContent($content_id, $content_gen_as, $content_design, $content_layout, $data_dir);
		$content = file_to_string($data_dir.'/cache/content_file.html');

		// restore session info
		$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();
		$_SESSION = $old_session;

		// get header details, required and normal headers
		$use_post_office_header = $this->attr('use_post_office_header');
		if ($use_post_office_header) {
			$from = $post_office->attr('from');
			$subject = $post_office->attr('subject');
			$header_details = $post_office->attr('header_details');
		} else {
			$from = $this->attr('from');
			$subject = $this->attr('subject');
			$header_details = $this->attr('header_details');
		}
		$header_details['from'] = $from;
		$header_details['subject'] = $subject;

		// add 'Preview' to the subject header for preview mails
		if (!empty($header_details['subject'])) {
			$header_details['subject'] .= ' (Preview)';
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
		$queued_jobs = $bm->getQueuedJobs($this->id);

		// cannot change the status of the job is in the db table
		if (!empty($queued_jobs)) return false;

		// user set the job to live, do validation in addJob()
		if ($new_status == SQ_STATUS_LIVE) {
			if (!$bm->addJob($this)) return false;
		}

		return parent::processStatusChange($new_status, $update_parents);

	}//end processStatusChange()


}//end class
?>