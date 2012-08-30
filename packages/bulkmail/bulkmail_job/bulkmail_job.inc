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
* $Id: bulkmail_job.inc,v 1.44 2012/08/30 00:57:13 ewang Exp $
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
* @version $Revision: 1.44 $
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
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

	}//end constructor


	/**
	* Returns parent asset Bulkmail Post Office or system default Post Office
	*
	* @return object
	* @access public
	*/
	function &getPostOffice()
	{
		$post_office = NULL;
		$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'bulkmail_post_office', TRUE);
		if (!empty($parents)) {
			$array_keys = array_keys($parents);
			$assetid = array_pop($array_keys);
			// make sure the parent post office is not in trash
			if (!$GLOBALS['SQ_SYSTEM']->am->assetInTrash($assetid)) {
				$post_office = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			}
		}

		// use the system default post office if none is selected
		if (is_null($post_office)) {
			$post_office = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');
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
		$bm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');
		$details_info = $bm->generateJobDetails($this, $post_office);
		$errors = $bm->isValidJob($details_info, TRUE);
		if (!empty($errors)) {
			trigger_localised_error('BML0001', E_USER_WARNING, $errors[0]);
			return FALSE;
		}

		// create the mail object
		$server_details = $post_office->attr('server_details');
		$driver = array_get_index($server_details, 'driver', '');
		$mf = new Mail();
		$mail_object = $mf->factory($driver, $server_details);

		// generate content as pre-selected user
		$data_dir = $post_office->data_path.'/.data/'.$this->id;
		require_once SQ_PACKAGES_PATH.'/bulkmail/bulk_mailer.inc';
		$bm = new Bulk_Mailer();
		$content = $bm->generateContent($details_info, $data_dir);

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

		require_once SQ_FUDGE_PATH.'/general/text.inc';
		$keywords = retrieve_keywords_replacements($subject);
		$replacements = Array();
		$content_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($details_info['content_id']);
		foreach ($keywords as $word) {
			$replacements[$word] = $content_asset->getKeywordReplacement($word);
		}
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($content_asset);
		replace_keywords($subject, $replacements);

		$subject = Bulk_Mailer::replaceContentKeywords($subject, $GLOBALS['SQ_SYSTEM']->user);

		$header_details['subject'] = $subject;
		$header_details['To'] = $recipient; // FR: 2515

		// add 'Preview' to the subject header for preview mails
		if (!empty($header_details['subject'])) {
			$header_details['subject'] .= ' (Preview)';
		}

		$text_only_content = html_entity_decode(strip_tags($content));

		// set $crlf to "\n" since we are sending the content using PEAR Mail
		$mime = new Mail_mime("\n");
		if (!$this->attr('content_text_only')) {
			$mime->setHTMLBody($content);
		}
		$mime->setTxtBody($text_only_content);
		$param = Array(
						'head_charset'	=> SQ_CONF_DEFAULT_CHARACTER_SET,
						'text_charset'	=> SQ_CONF_DEFAULT_CHARACTER_SET,
						'html_charset'	=> SQ_CONF_DEFAULT_CHARACTER_SET,
					  );
		$body = @$mime->get($param);
		$headers = @$mime->headers($header_details);
		$status = @$mail_object->send($recipient, $headers, $body);
		if ($status instanceof PEAR_Error) {
			// unable to send bulkmail
			trigger_localised_error('BML0002', E_USER_WARNING, $status->getMessage());
			return FALSE;
		}

		return TRUE;

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
	function processStatusChange($new_status, $update_parents=TRUE)
	{
		// get the current state from the bmail queue db table
		$bm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');
		$queued_jobs = $bm->getQueuedJobs($this->id);

		// cannot change the status of the job is in the db table
		if (!empty($queued_jobs)) return FALSE;

		switch ($new_status) {
			case SQ_STATUS_LIVE:
				// user set the job to live, do validation in addJob()
				if (!$bm->addJob($this)) return FALSE;
			break;
			case SQ_STATUS_UNDER_CONSTRUCTION:
				$job_path = $bm->getJobDataPath($this->id);
				clear_directory($job_path);
			break;
		}

		return parent::processStatusChange($new_status, $update_parents);

	}//end processStatusChange()


	/**
	* Generate the raw content to be sent, ignoring user switching and recpient keyword replacement
	*
	* @param array	$details	The details of the job
	*
	* @return string
	* @access public
	*/
	public function generateRawContent($details)
	{
		// moved functionality to asset_manager.inc for reusing the codes in triggers
		return $GLOBALS['SQ_SYSTEM']->am->generateRawContent($details);

	}//end generateRawContent()


}//end class
?>
