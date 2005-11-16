<?php

require_once SQ_PACKAGES_PATH.'/bulkmail/bulkmail_post_office/bulkmail_post_office.inc';
require_once SQ_PACKAGES_PATH.'/bulkmail/bulkmail_constant.inc';

/**
* Bulkmail_Manager
*
* Purpose
*
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Rayn Ong <rong@squiz.net>
*
* @version $Revision: 1.17 $
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
	function Bulkmail_Manager($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Asset($assetid);

	}//end constructor


	/**
	* Return a name to use to describe this instance of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	*/
	function _getName($short_name=false)
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
			return false;
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($linkid = parent::create($link)) {
			if (!system_asset_fns_create_cleanup($this)) {
				$linkid = false;
			}
		}

		if ($linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		return $linkid;

	}//end create()


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
		return false;

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
			trigger_localised_error('CORE0118', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
			return false;

	}//end deleteLink()


	/**
	* Can this asset have its last significant link removed (putting it in the trash)?
	*
	* @return boolean
	* @access public
	*/
	function canDelete()
	{
			return false;

	}//end canDelete()


	/**
	* returns FALSE always because we don't want to be able to clone a bulkmail manager
	*
	* @return boolean
	* @access public
	*/
	function canClone()
	{
			return false;

	}//end canClone()


//--        JOB        --//


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
	function addJob($job, $post_office=null)
	{
		// open the transaction
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// if no post office is supplied, work out which one to use
		if (is_null($post_office)) {
			if ($job->id == 0) {
				$post_office =& $this;
			} else {
				$post_office =& $job->getPostOffice();
			}
		}

		// validate normal/ad-hoc jobs before adding entry to db
		$details_info = $this->generateJobDetails($job, $post_office);
		$errors = $this->isValidJob($details_info, true);
		if (!empty($errors)) {
			trigger_localised_error('BML0001', E_USER_WARNING, $errors[0]);
			return false;
		}

		// work out the unique id for this job in the db table
		if ($job->id == 0) {
			// use random id based on current timestamp for ad-hoc jobs
			// check if this id has been taken by another ad-hoc job
			do {
				$unique_id = md5(uniqid(''));
			} while (is_dir($post_office->data_path.'/.data/'.$unique_id));
		} else {
			$unique_id = $job->id;
		}

		// purge and refresh dir
		$data_dir = $this->getJobDataPath($post_office->id.':'.$unique_id);
		if (is_dir($data_dir)) delete_directory($data_dir);

		// store progress and details information in files
		create_directory($data_dir);
		$this->initProgressFile($data_dir);
		$this->initDetailsFile($details_info, $data_dir);

		// add a new entry to the database table
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$sql = 'INSERT INTO
					sq_bulkmail_queue
					(
						id,
						post_office_id,
						status
					)
					VALUES
					(
						'.$db->quoteSmart($unique_id).',
						'.$db->quoteSmart($post_office->id).',
						'.$db->quoteSmart(BML_JOB_STATE_NOT_RUNNING).'
					)';

		$result = $db->query($sql);
		assert_valid_db_result($result);
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return true;

	}//end addJob()


	/**
	* Deletes a job entry from the DB table when it's cancelled/finished
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
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$sql = 'DELETE FROM
					sq_bulkmail_queue
				WHERE
					id = '.$db->quoteSmart($job_id);
		$result = $db->query($sql);
		assert_valid_db_result($result);
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		// purge the data dir if it is an ad-hoc job
		// TODO: make sure this part works, once we can add ad-hoc job
		$job_id_parts = explode(':', $job_id);
		if (isset($job_id_parts[1])) {
			$post_office =& $GLOBALS['SQ_SYSTEM']->am->getAsset($job_id_parts[0]);
			$path = $post_office->data_path.'/.data/'.$job_id_parts[1];
			if (is_dir($path)) delete_directory($path);
		}

		return true;

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
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// set last_updated to zero to skip timeout checking when we resume the job
		if ($state == BML_JOB_STATE_NOT_RUNNING	) {
			$progress_info = $this->getJobProgress($job_id);
			$progress_info['last_updated'] = 0;
			$data_dir = $this->getJobDataPath($job_id);
			$output = '<?php $progress_info = '.var_export($progress_info, true).'; ?>';
			if (!string_to_file($output, $data_dir.'/progress_info')) {
				trigger_localised_error('BML0008', E_USER_WARNING, $data_dir.'/progress_info');
				return false;
			}
		}

		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$sql = 'UPDATE
					sq_bulkmail_queue
				SET
					status = '.$db->quoteSmart($state).'
				WHERE
					id = '.$db->quoteSmart($job_id);
		$result = $db->query($sql);
		assert_valid_db_result($result);
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return true;

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
	function getQueuedJobs($job_id=null, $post_office_id=null)
	{
		$db_table = SQ_TABLE_RUNNING_PREFIX.'bulkmail_queue';
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$where = '';
		if (!is_null($job_id)) {
			$where = ' WHERE
						id = '.$db->quoteSmart($job_id);
		} else if (!is_null($post_office_id)) {
			$where = ' WHERE
						post_office_id = '.$db->quoteSmart($post_office_id);
		}

		$sql = 'SELECT id, post_office_id, status
				FROM sq_bulkmail_queue
				'.$where;

		$results = $db->getAssoc($sql);
		assert_valid_db_result($results);

		// add progress/details info to each job result
		foreach ($results as $job_id => $result) {
			$progress_info = $this->getJobProgress($job_id);
			$results[$job_id]['progress'] = $progress_info;
			$details_info = $this->getJobDetails($job_id);
			$results[$job_id]['details'] = $details_info;
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
			$post_office =& $GLOBALS['SQ_SYSTEM']->am->getAsset($job_id_parts[0]);
			$path = $post_office->data_path.'/.data/'.$job_id_parts[1];
		} else {
			$job =& $GLOBALS['SQ_SYSTEM']->am->getAsset($job_id, 'bulkmail_job');
			$post_office =& $job->getPostOffice();
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
			$post_office =& $GLOBALS['SQ_SYSTEM']->am->getAsset($job_id_parts[0]);
		} else {
			$job =& $GLOBALS['SQ_SYSTEM']->am->getAsset($job_id, 'bulkmail_job');
			$post_office =& $job->getPostOffice();
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
		*/
		$progress_info = Array(
							'current_count'		=> 0,
							'current_recip_id'	=> 0,
							'current_chunk'		=> 0,
							'last_updated'		=> 0,
							'total_count'		=> 0,
						 );

		$output = '<?php $progress_info = '.var_export($progress_info, true).'; ?>';
		if (!string_to_file($output, $data_dir.'/progress_info')) {
			trigger_localised_error('BML0007', E_USER_WARNING, $data_dir.'/progress_info');
			return false;
		} else {
			return true;
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
				case 'threshold':
					$details_info[$var_name] = $post_office->attr($var_name);
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

		$output = ' <?php $details_info = '.var_export($details_info, true).'; ?>';
		if (!string_to_file($output, $data_dir.'/details_info')) {
			trigger_localised_error('BML0007', E_USER_WARNING, $data_dir.'/details_info');
			return false;
		} else {
			return true;
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
	* Check if job is not valid or true if we can set this job to live
	*
	* Returns an array of error messages if extra parameter is passed in, otherwise returns a boolean
	*
	* @param array		$details_info	job details generated by generateJobDetails()
	* @param boolean	$report_error	returns an array of error messages if true
	*
	* @return mixed boolean|array
	* @access public
	*/
	function isValidJob($details_info, $report_error=false)
	{
		$errors = Array();

		// cannot create a PEAR mail instance
		$driver = array_get_index($details_info['server_details'], 'driver', '');
		$mail_object =& Mail::factory($driver, $details_info['server_details']);
		if (PEAR::isError($mail_object)) {
			$errors[] = translate('bm_warning_server_details_error', $mail_object->getMessage());
		}
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($mail_object);

		// no recipient
		$recipients = $details_info['recipients'];
		if (empty($recipients)) {
			$errors[] = translate('bm_warning_no_recipient');
		}

		// no 'from' and 'subject' header field
		if (empty($details_info['from'])) {
			$errors[] = translate('bm_warning_no_from_field');
		}
		if (empty($details_info['subject'])) {
			$errors[] = translate('bm_warning_no_subject_field');
		}

		// no content page
		if (empty($details_info['content_id'])) {
			$errors[] = translate('bm_warning_no_asset_to_send');
		} else {
			// page is not live
			$content_page =& $GLOBALS['SQ_SYSTEM']->am->getAsset($details_info['content_id']);
			if ($content_page->status != SQ_STATUS_LIVE) {
				$errors[] = translate('bm_warning_asset_to_send_not_live', $content_page->name, $content_page->id);
			} else {
				// no pre-selected user
				if (empty($details_info['content_gen_as'])) {
					$errors[] = translate('bm_warning_no_generate_as');
				} else {
					// cannot login as pre-selected user, is in trash or not live
					$user =& $GLOBALS['SQ_SYSTEM']->am->getAsset($details_info['content_gen_as']);
					if (!$user->canLogin()) {
						$errors[] = translate('bm_warning_generate_as_login_error', $user->name, $user->id);
					}
					// pre-selected user does not have read permission for content page
					$read_access = $content_page->readAccess(Array($user->id));
					if (!$read_access) {
						$errors[] = translate('bm_warning_generate_as_no_read_access', $user->name, $user->id, $content_page->name, $content_page->id);
					}
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($user);
				}
			}
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($content_page);
		}

		// design does not have public read access
		if (!empty($details_info['content_design'])) {
			$public_user =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('public_user');
			$design =& $GLOBALS['SQ_SYSTEM']->am->getAsset($details_info['content_design']);
			$public_read_access = $design->readAccess(Array($public_user->id));
			if (!$public_read_access) {
				$errors[] = translate('bm_warning_design_no_public_read_access', $design->name, $design->id);
			}
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($design);
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($public_user);
		}

		if ($report_error) {
			return $errors;
		} else {
			return (empty($errors) ? true : false);
		}

	}//end isValidJob()


}//end class
?>
