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
* $Id: bulkmail_manager.inc,v 1.56 2012/09/14 05:51:57 akarelia Exp $
*
*/

require_once SQ_PACKAGES_PATH.'/bulkmail/bulkmail_post_office/bulkmail_post_office.inc';
require_once SQ_PACKAGES_PATH.'/bulkmail/bulkmail_constant.inc';
require_once SQ_FUDGE_PATH.'/general/www.inc';


/**
* Bulkmail_Manager
*
* Purpose
*
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Rayn Ong <rong@squiz.net>
*
* @version $Revision: 1.56 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class Bulkmail_Manager extends Bulkmail_Post_Office
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
	* Return a name to use to describe this instance of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	*/
	function _getName($short_name=FALSE)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name');

	}//end _getName()


	/**
	* Create this asset
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return mixed int|boolean
	* @access public
	* @see Asset::create()
	*/
	function create(&$link)
	{
		require_once SQ_CORE_PACKAGE_PATH.'/system/system_asset_fns.inc';
		if (!system_asset_fns_create_pre_check($this)) {
			return FALSE;
		}
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($linkid = parent::create($link)) {
			if (!system_asset_fns_create_cleanup($this)) {
				$linkid = FALSE;
			}
		}

		if ($linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return $linkid;

	}//end create()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		return Array(
				SQ_LINK_TYPE_1	=> Array(),
				SQ_LINK_TYPE_2	=> Array(),
				SQ_LINK_TYPE_3	=> Array(),
				SQ_LINK_NOTICE	=> Array(
									'image'		=> Array('card' => 1, 'exclusive' => FALSE),
									'design'	=> Array('card' => 'M', 'exclusive' => FALSE),
								   ),
			   );

	}//end _getAllowedLinks()


	/**
	* Not able to move this system asset around
	*
	* @param object	&$minor		the minor asset that we are linking to
	* @param object	&$old_major	the major asset that we are being moved from
	* @param string	$link_type	the type of link this is
	*
	* @return mixed boolean|string
	* @access public
	*/
	function canMoveLink(&$minor, &$old_major, $link_type)
	{
		return FALSE;

	}//end canMoveLink()


	/**
	* Removes a link where this asset is the major player
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return boolean
	* @access public
	*/
	function deleteLink($linkid)
	{
			trigger_localised_error('CORE0118', sprintf(translate('You cannot delete anything from the %s'), $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name')), E_USER_WARNING)
			return FALSE;

	}//end deleteLink()


	/**
	* Can this asset have its last significant link removed (putting it in the trash)?
	*
	* @return boolean
	* @access public
	*/
	function canDelete()
	{
			return FALSE;

	}//end canDelete()


	/**
	* returns FALSE always because we don't want to be able to clone a bulkmail manager
	*
	* @return boolean
	* @access public
	*/
	function canClone()
	{
			return FALSE;

	}//end canClone()


//--        JOB        --//


	/**
	* Interface function for adding a non-scheduled (DO IT NOW!) bulkmail to the
	* bulkmail queue.
	*
	* @param string	$from			Address of the sender, eg "John Doe <johndoe@example.com>"
	* @param array	$recipients		AssetIDs of the recipients
	* @param string	$subject		Subject of the email
	* @param int	$content_id		AssetID of the asset you want to send as content
	* @param int	$content_gen_as	AssetID of the user you want the mail generated as
	* @param array	$headers		Addition headers to add to the mail in the form:
	*								Array('key'	=> 'value')
	* @param object	$post_office_id	AssetID of the post office to use. If this is NULL, you MUST supply
	*								server details
	* @param array	$server_details	Mail options to create a post office on the fly:
	* <pre>
	*								Array(
	*									'driver'	=> 'sendmail',
	*									'path'		=> '/usr/sbin/sendmail',
	*								)
	*
	*								OR
	*
	*								Array(
	*									'driver'	=> 'smtp',
	*									'host'		=> 'mail.yourcompany.com',
	*									'port'		=> 25,
	*									'auth'		=> FALSE,
	*									'username'	=> '',
	*									'password'	=> '',
	*								)
	* </pre>
	* @param int	$design_id		AssetID of the design to use with the content
	* @param int	$layout_id		AssetID of the paint layout to use with the content (requires a design as well)
	*
	* @return boolean
	* @access public
	*/
	function createAdHocJob($from, $recipients, $subject, $content_id, $content_gen_as, $headers=Array(), $post_office_id=0, $server_details=Array(), $design_id=0, $layout_id=0, $subscriptions=Array())
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;

		if (!is_array($recipients)) {
			$recipients = Array($recipients);
		}

		$recipients = $am->assetExists($recipients);
		if (empty($recipients)) {
			trigger_localised_error('BML0010', translate('Problematic recipient id found: %s.'), E_USER_ERROR)
			return FALSE;
		}

		$am->includeAsset('bulkmail_job');
		$bulkmail_job = new Bulkmail_Job();

		$bulkmail_job->setAttrValue('from', $from);
		$bulkmail_job->setAttrValue('recipients', $recipients);
		$bulkmail_job->setAttrValue('subject', $subject);
		$bulkmail_job->setAttrValue('content_id', $content_id);
		$bulkmail_job->setAttrValue('content_gen_as', $content_gen_as);
		$bulkmail_job->setAttrValue('header_details', $headers);

		$bulkmail_job->setAttrValue('content_design', $design_id);
		$bulkmail_job->setAttrValue('content_layout', $layout_id);

		$subscription_status = empty($subscriptions) ? FALSE : TRUE;
		$bulkmail_job->setAttrValue('user_subscriptions_status', $subscription_status);
		$bulkmail_job->setAttrValue('subscriptions', $subscriptions);

		$post_office = NULL;
		if (empty($post_office_id) && !empty($server_details)) {
			$post_office = new Bulkmail_Post_Office();
			$post_office->setAttrValue('server_details', $server_details);
		} else if ($post_office_id) {
			$post_office = $am->getAsset($post_office_id, 'bulkmail_post_office');
			if (is_null($post_office)) return FALSE;
		}

		return $this->addJob($bulkmail_job, $post_office);

	}//end createAdHocJob()


	/**
	* Adds a new job entry to the database, set up the datapath and progress info file
	*
	* Note: existing entry in the table with the same job id will cause a HIPO lock
	*
	* @param object	$job			job object
	* @param object	$post_office	post office object where we get the server/header details from
	*
	* @return boolean
	* @access private
	*/
	function addJob($job, $post_office=NULL)
	{
		// if no post office is supplied, work out which one to use
		if (is_null($post_office)) {
			if ($job->id == 0) {
				$post_office = $this;
			} else {
				$post_office = $job->getPostOffice();
			}
		}

		// validate normal/ad-hoc jobs before adding entry to db
		$details_info = $this->generateJobDetails($job, $post_office);
		$errors = $this->isValidJob($details_info, TRUE);
		if (!empty($errors)) {
			trigger_localised_error('BML0001', sprintf(translate('Invalid job. %s'), $errors[0]), E_USER_WARNING)
			return FALSE;
		}

		// work out the unique id for this job in the db table
		if ($job->id == 0) {
			// keep generating an ID until we find a new one (probably straight away)
			do {
				$unique_id = $post_office->id.':'.md5(uniqid(''));
				$data_dir = $this->getJobDataPath($unique_id);
			} while (is_dir($data_dir));
		} else {
			$unique_id = $job->id;
			$data_dir = $this->getJobDataPath($unique_id);

			// purge and refresh dir
			if (is_dir($data_dir)) delete_directory($data_dir);
		}

		// store progress and details information in files
		create_directory($data_dir);
		$this->initProgressFile($data_dir);
		$this->initDetailsFile($details_info, $data_dir);

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// add a new entry to the database table
		try {
			$bind_vars = Array(
							'id'		=> $unique_id,
							'po_id'		=> $post_office->id,
							'status'	=> BML_JOB_STATE_NOT_RUNNING,
						 );
			$result = MatrixDAL::executeQuery('bulkmail_package', 'addJob', $bind_vars);
		} catch (Exception $e) {
			throw new Exception("Unable to add bulkmail job (#$unique_id) due to database error: ".$e->getMessage());
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end addJob()


	/**
	* Deletes a job entry from the DB table when it's cancelled/completed
	*
	* Also removes the data directory if the job is an ad-hoc job
	*
	* @param int	$job_id	the unique id of the job entry
	*
	* @return boolean
	* @access public
	*/
	function deleteJob($job_id)
	{
		try {
			$bind_vars['job_id'] = $job_id;
			$result = MatrixDAL::executeQuery('bulkmail_package', 'deleteJob', $bind_vars);
		} catch (Exception $e) {
			throw new Exception("Unable to delete bulkmail job (#$job_id) due to database error: ".$e->getMessage());
		}

		// purge the data dir if it is an ad-hoc job
		$job_id_parts = explode(':', $job_id);
		if (isset($job_id_parts[1])) {
			$post_office = $GLOBALS['SQ_SYSTEM']->am->getAsset($job_id_parts[0]);
			$path = $post_office->data_path.'/.data/'.$job_id_parts[1];
			if (is_dir($path)) delete_directory($path);
		}

		return TRUE;

	}//end deleteJob()


	/**
	* Updates the current state of a job
	*
	* @param int	$job_id	the unique id of the job entry
	* @param string	$state	new job state
	*
	* @return boolean
	* @access public
	*/
	function updateJob($job_id, $state)
	{

		// set last_updated to zero to skip timeout checking when we resume the job
		if ($state == BML_JOB_STATE_NOT_RUNNING	) {
			$progress_info = $this->getJobProgress($job_id);
			$progress_info['last_updated'] = 0;
			$data_dir = $this->getJobDataPath($job_id);
			if (!array_to_file($progress_info, 'progress_info', $data_dir.'/progress_info')) {
				trigger_localised_error('BML0008', sprintf(translate('File "%s" is not writable.'), $data_dir.'/progress_info'), E_USER_WARNING)
				return FALSE;
			}
		}

		try {
				$bind_vars = Array(
								'status'	=> $state,
								'job_id'	=> $job_id,
						 	 );
			$result = MatrixDAL::executeQuery('bulkmail_package', 'updateJob', $bind_vars);
		} catch (Exception $e) {
			throw new Exception("Unable to update bulkmail job (#$job_id) due to database error: ".$e->getMessage());
		}
		return TRUE;

	}//end updateJob()


	/**
	* Returns jobs from the db table, if post office id is supplied, also returns all children
	*
	* @param int	$job_id			the unique id of the job entry
	* @param int	$post_office_id	the id of the parent post office
	*
	* @return array
	* @access public
	*/
	function getQueuedJobs($job_id=NULL, $post_office_id=NULL)
	{
		if (is_null($job_id) && is_null($post_office_id)) {
			try {
				$results = MatrixDAL::executeGrouped('bulkmail_package', 'getAllQueuedJobs');
			} catch (Exception $e) {
				throw new Exception('Unable to get queued bulkmail jobs: '.$e->getMessage());
			}
		} else if (!is_null($job_id)) {
			try {
				$bind_vars['job_id'] = $job_id;
				$results = MatrixDAL::executeGrouped('bulkmail_package', 'getQueuedJobsByJob', $bind_vars);
			} catch (Exception $e) {
				throw new Exception('Unable to get queued bulkmail jobs: '.$e->getMessage());
			}
		} else if (!is_null($post_office_id)) {
			try {
				$bind_vars['post_office_id'] = $post_office_id;
				$results = MatrixDAL::executeGrouped('bulkmail_package', 'getQueuedJobsByPostOffice', $bind_vars);
			} catch (Exception $e) {
				throw new Exception('Unable to get queued bulkmail jobs: '.$e->getMessage());
			}
		}

		// add progress/details info to each job result
		foreach ($results as $job_id => $result) {
			$job_id_actual = strtok($job_id, ':');
			if ($GLOBALS['SQ_SYSTEM']->am->assetExists($job_id_actual)) {
				$progress_info = $this->getJobProgress($job_id);
				$results[$job_id]['progress'] = $progress_info;
				$details_info = $this->getJobDetails($job_id);
				$results[$job_id]['details'] = $details_info;
				// do this since we cannot use executeGroupAssoc
				$results[$job_id]['po_id'] = $result[0][0];
				$results[$job_id]['status'] = $result[0][1];
			} else {
				// if this job has been removed, then delete it from the bmail queue too
				$this->deleteJob($job_id);
			}
		}

		return $results;

	}//end getQueuedJobs()


	/**
	* Gets the data path of a job
	*
	* @param int	$job_id	the unique id of the job entry
	*
	* @return string
	* @access public
	*/
	function getJobDataPath($job_id)
	{
		$job_id_parts = explode(':', $job_id);

		if (isset($job_id_parts[1])) {
			$post_office = $GLOBALS['SQ_SYSTEM']->am->getAsset($job_id_parts[0]);
			$path = $post_office->data_path.'/.data/'.$job_id_parts[1];
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($job);
		} else {
			$job = $GLOBALS['SQ_SYSTEM']->am->getAsset($job_id);
			$post_office = $job->getPostOffice();
			$path = $post_office->data_path.'/.data/'.$job_id;
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($job);
		}
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($post_office);

		return $path;

	}//end getJobDataPath()


	/**
	* Gets the post office of a bulkmail job
	*
	* @param string	$job_id	the unique id of the job entry
	*
	* @return string
	* @access public
	*/
	function &getPostOffice($job_id)
	{
		$job_id_parts = explode(':', $job_id);
		if (isset($job_id_parts[1])) {
			$post_office = $GLOBALS['SQ_SYSTEM']->am->getAsset($job_id_parts[0]);
		} else {
			$job = $GLOBALS['SQ_SYSTEM']->am->getAsset($job_id, 'bulkmail_job');
			$post_office = $job->getPostOffice();
		}

		return $post_office;

	}//end getPostOffice()


//--        PROGRESS        --//


	/**
	* Initialises the progress info file
	*
	* @param string	$data_dir	path to the directory containing the progress info file
	*							use job's data-dir path if it is a normal job
	*							use system default post office data-dir path for an ad-hoc job
	*
	* @return boolean
	* @access public
	*/
	function initProgressFile($data_dir)
	{
		/*
			available fields:
			current_count		processed this number of mails
			current_recip_id	assetid of the current recipient/user
								keep track of this as recipients are not in numerical order
			current_chunk		currently up to this chunk of recipient
			last_updated		timestamp of when we last added a mail to the queue
			total_count			total number of mails to send (to be updated when chunking recipient)
			time_per_chunk		to be used for thresholding the send rate rule
			problematic			an array of problematic recipient ids
		*/
		$progress_info = Array(
							'current_count'		=> 0,
							'current_recip_id'	=> 0,
							'current_chunk'		=> 0,
							'last_updated'		=> 0,
							'total_count'		=> 0,
							'time_per_chunk'	=> 0,
							'problematic'		=> Array(),
						 );

		if (!array_to_file($progress_info, 'progress_info', $data_dir.'/progress_info')) {
			trigger_localised_error('BML0007', sprintf(translate('Unable to create file: %s.'), $data_dir.'/progress_info'), E_USER_WARNING)
			return FALSE;
		} else {
			return TRUE;
		}

	}//end initProgressFile()


	/**
	* Reads the progress info from a file
	*
	* @param int	$job_id	the unique id of the job entry
	*
	* @return array
	* @access public
	*/
	function getJobProgress($job_id)
	{
		$job_path = $this->getJobDataPath($job_id);
		$progress_path = $job_path.'/progress_info';
		$progress_info = Array();
		if (file_exists($progress_path)) {
			include $progress_path;
		}

		return $progress_info;

	}//end getJobProgress()


	/**
	* Generates the relevant job details, also work out whether to use post office attribute
	*
	* @param object	$job			the job object
	* @param object	$post_office	the post office object
	*
	* @return array
	* @access public
	*/
	function generateJobDetails($job, $post_office)
	{
		$details_info = Array();

		// Pass the ID of the Job, so we can load the job back up to generate the content
		$details_info['job_id'] = $job->id;

		foreach ($job->vars as $var_name  => $var_info) {
			switch ($var_name) {
				case 'from' :
				case 'subject' :
				case 'header_details':
					if ($job->attr('use_post_office_header')) {
						$details_info[$var_name] = $post_office->attr($var_name);
					} else {
						$details_info[$var_name] = $job->attr($var_name);
					}
				break;
				case 'server_details':
					$details_info[$var_name] = $post_office->attr($var_name);
				break;
				case 'threshold':
					// whether default post office overrides the threshold settings
					if ($this->attr('use_bm_threshold')) {
						$details_info[$var_name] = $this->attr($var_name);
					} else {
						$details_info[$var_name] = $post_office->attr($var_name);
					}
				break;
				case 'bulkmail_mode':
					// Select the bulkmail mode
					$mode = $post_office->attr('bulkmail_mode');
					if (!empty($mode)) {
						$details_info[$var_name] = $mode;
					} else {
						$details_info[$var_name] = $this->attr($var_name);
					}
				break;
				default:
					$details_info[$var_name] = $job->attr($var_name);
				break;
			}
		}

		return $details_info;

	}//end generateJobDetails()


	/**
	* Stores all the job details we need to a file
	*
	* @param array	$details_info	job details generated by generateJobDetails()
	* @param string	$data_dir		path to job's effective data directory
	*
	* @return boolean
	* @access public
	*/
	function initDetailsFile($details_info, $data_dir)
	{
		// mail queue options
		$details_info['queue_details'] = Array(
											'type'	=> 'flatfile',
											'dir'	=> $data_dir.'/queue',
										 );

		if (!array_to_file($details_info, 'details_info', $data_dir.'/details_info')) {
			trigger_localised_error('BML0007', sprintf(translate('Unable to create file: %s.'), $data_dir.'/details_info'), E_USER_WARNING)
			return FALSE;
		} else {
			return TRUE;
		}

	}//end initDetailsFile()


	/**
	* Reads the details info from a file
	*
	* @param int	$job_id	the unique id of the job entry
	*
	* @return array
	* @access public
	*/
	function getJobDetails($job_id)
	{
		$job_path = $this->getJobDataPath($job_id);
		$details_path = $job_path.'/details_info';
		$details_info = Array();
		if (file_exists($details_path)) include $details_path;

		return $details_info;

	}//end getJobDetails()


	/**
	* Check if job is not valid or TRUE if we can set this job to live
	*
	* Returns an array of error messages if extra parameter is passed in, otherwise returns a boolean
	*
	* @param array		$details_info	job details generated by generateJobDetails()
	* @param boolean	$report_error	returns an array of error messages if TRUE
	*
	* @return mixed boolean|array
	* @access public
	*/
	function isValidJob($details_info, $report_error=FALSE)
	{
		$errors = Array();

		// cannot create a PEAR mail instance
		$driver = array_get_index($details_info['server_details'], 'driver', '');
		$mf = new Mail();
		$mail_object = $mf->factory($driver, $details_info['server_details']);
		if ($mail_object instanceof PEAR_Error) {
			$errors[] = sprintf(translate('Server Details: "%s"'), $mail_object->getMessage());

		}
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($mail_object);

		// no recipient
		$recipients = $details_info['recipients'];
		if (empty($recipients)) {
			$errors[] = translate('Recipients: Recipients not set.');

		}

		// no content page
		$public_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('public_user');
		if (empty($details_info['content_id'])) {
			$errors[] = translate('Content: \'Asset to Send\' not set.');

		} else {
			// page is not live
			$content_page = $GLOBALS['SQ_SYSTEM']->am->getAsset($details_info['content_id']);
			if ($content_page->status != SQ_STATUS_LIVE) {
				$errors[] = sprintf(translate('Content: "Asset to Send", %1$s (Id #%2$s), is not live.'), $content_page->name, $content_page->id);

			} else {
				// no pre-selected user
				if (empty($details_info['content_gen_as'])) {
					$errors[] = translate('Content: \'Generate As\' not set.');

				} else {
					// cannot login as pre-selected user, is in trash or not live
					$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($details_info['content_gen_as']);
					if (!($user instanceof Public_User) && !$user->canLogin()) {
						$errors[] = sprintf(translate('Content: Unable to generate content as %1$s (Id #%2$s), user is either in trash or not live.'), $user->name, $user->id);

					}
					// pre-selected user does not have (effective) read permission for content page
					$ids = $GLOBALS['SQ_SYSTEM']->am->getParents($user->id, 'user_group', FALSE);
					$ids[$user->id] = 'content_generate_as';
					$content_gen_as_read_access = $content_page->readAccess(array_keys($ids));
					$public_read_access = $content_page->readAccess(Array($public_user->id));
					if (!$content_gen_as_read_access && !$public_read_access) {
						$errors[] = sprintf(translate('Content: %1$s (Id #%2$s) does not have Read Access to \'Asset to Send\' - %3$s (Id #%4$s).'), $user->name, $user->id, $content_page->name, $content_page->id);

					}
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($user);
				}
			}

			// check recipient read access (not expanding groups)
			$problematic_list = Array();
			$permission_denied = $GLOBALS['SQ_SYSTEM']->am->getPermission($details_info['content_id'], SQ_PERMISSION_READ, FALSE, FALSE);
			$public_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('public_user');
			$public_user_id = $public_user->id;
			foreach ($recipients as $recipient_key => $recipient_id) {
				if (isset($recipient_id['email']) && preg_match('/(.*)\@(.*)/is', $recipient_id['email'])) {
					// If the recipient is an email address, check for public permission
					if (!$content_page->readAccess(Array($public_user_id))) {
						$problematic_list[] = $recipients[$recipient_key];
					}//end if
				} else {
					// get parent user group and get the effective permission on the asset to send
					$ids = $GLOBALS['SQ_SYSTEM']->am->getParents($recipient_id, 'user_group', FALSE);
					$ids[$recipient_id] = 'recipient_id';
					if (!$content_page->readAccess(array_keys($ids))) {
						$problematic_list[] = $recipient_id;
					}
					if (in_array($recipient_id, $permission_denied)) {
						$problematic_list[] = $recipient_id;
					}
				}//end if
			}//end foreach
			if (!empty($problematic_list)) {
				asort($problematic_list);
				$errors[] = sprintf(translate('Content: The following recipients do not have Read Access to "%1$s" (ID #%2$s): %3$s.'), $content_page->name, $content_page->id, implode(', ', $problematic_list));

			}

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($content_page);
		}//end else

		// design does not have public read access
		if (!empty($details_info['content_design'])) {
			$design = $GLOBALS['SQ_SYSTEM']->am->getAsset($details_info['content_design']);
			$public_read_access = $design->readAccess(Array($public_user->id));
			if (!$public_read_access) {
				$errors[] = sprintf(translate('Content: %1$s (Id #%2$s) does not have public user read access. Check the status and permissions.'), $design->name, $design->id);

			}
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($design);
		}

		// layout does not have public read access
		if (!empty($details_info['content_layout'])) {
			$layout = $GLOBALS['SQ_SYSTEM']->am->getAsset($details_info['content_layout']);
			$public_read_access = $layout->readAccess(Array($public_user->id));
			if (!$public_read_access) {
				$errors[] = sprintf(translate('Content: %1$s (Id #%2$s) does not have public user read access. Check the status and permissions.'), $layout->name, $layout->id);

			}
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($layout);
		}

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($public_user);

		// no 'from' field
		if (empty($details_info['from'])) {
			$errors[] = translate('Header Details: Missing \'From\' field.');

		// 'from' field is not a valid email address
		} else {
			if (!valid_email($details_info['from'])) {
				$errors[] = translate('Header Details: Email address provided in \'From\' field is invalid.');

			}
		}

		// no 'subject' field
		if (empty($details_info['subject'])) {
			$errors[] = translate('Header Details: Missing \'Subject\' field');

		}

		if ($report_error) {
			return $errors;
		} else {
			return (empty($errors) ? TRUE : FALSE);
		}

	}//end isValidJob()


}//end class
?>
