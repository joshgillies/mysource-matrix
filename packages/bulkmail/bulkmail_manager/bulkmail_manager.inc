<?php

require_once SQ_INCLUDE_PATH.'/asset.inc';
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
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class Bulkmail_Manager extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Bulkmail_Manager($assetid=0)
	{
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
	* Only the default post office system asset can be linked underneath a bulkmail manager
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		return Array(
				SQ_LINK_TYPE_1	=> Array(
									'bulkmail_post_office_default'	=> Array(
																		'card'		=> 'M',
																		'exclusive'	=> false,
																	   ),
								   ),
				SQ_LINK_TYPE_2	=> Array(
									'bulkmail_post_office_default'	=> Array(
																		'card'		=> 'M',
																		'exclusive'	=> false,
																	   ),
								   ),
				SQ_LINK_NOTICE	=> Array(
									'bulkmail_post_office_default'	=> Array(
																		'card'		=> 'M',
																		'exclusive'	=> false,
																	   ),
								   ),
			   );

	}//end _getAllowedLinks()


	/**
	* Not able to move default post office around
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
				$post_office =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_post_office_default');
			} else {
				$post_office =& $job->getPostOffice();
			}
		}

		// check isValidJob here IMPORTANT!

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
		$this->initDetailsFile($job, $post_office, $data_dir);

		// add a new entry to the database table
		$db_table = SQ_TABLE_RUNNING_PREFIX.'bulkmail_queue';
		$fields = Array('id', 'post_office_id', 'status');
		$db_values = "'$unique_id', '".$post_office->id."', '".BML_JOB_STATE_NOT_RUNNING."'";
		$db_fields = implode(', ', $fields);
		$sql = "INSERT INTO $db_table ($db_fields) VALUES ($db_values);";
		$result = $GLOBALS['SQ_SYSTEM']->db->query($sql);

		if (!assert_valid_db_result($result)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// commit transaction if everything is ok
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		echo " Job #$job->id : addJob\n";
		return true;

	}//end addJob()


	/**
	* Deletes a job entry from the DB table when it's cancelled/finished
	*
	* @param int	$job_id	the unique id of the job entry
	*
	* @return boolean
	* @access public
	*/
	function deleteJob($job_id)
	{
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$db_table = SQ_TABLE_RUNNING_PREFIX.'bulkmail_queue';
		$sql = "DELETE FROM $db_table WHERE id = $job_id;";
		$result = $GLOBALS['SQ_SYSTEM']->db->query($sql);

		if (!assert_valid_db_result($result)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		echo " Job #$job_id : deleteJob\n";
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
		if ($state == BML_JOB_STATE_PAUSED) {
			$progress_info = getJobProgress($job_id);
			$progress_info['last_updated'] = 0;
			$data_dir = $this->getJobDataPath($job_id);
			$output = '<?php $progress_info = '.var_export($progress_info, true).'; ?>';
			if (!string_to_file($output, $data_dir.'/progress_info')) {
				trigger_error("updateJob : failed to update progress file.\n");
				return false;
			}
		}

		$db_table = SQ_TABLE_RUNNING_PREFIX.'bulkmail_queue';
		$sql = "UPDATE $db_table SET status = $state WHERE id = $job_id;";
		$result = $GLOBALS['SQ_SYSTEM']->db->query($sql);
		if (!assert_valid_db_result($result)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		echo " Job #$job_id : updateJob, state=$state\n";
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end updateJob()


	/**
	* Returns jobs from the db table, if post office id is supplied, also returns all children
	*
	* @param int	$job_id			the unique id of the job entry
	* @param int	$post_office_id	the id of the parent post office
	*
	* @return boolean
	* @access public
	*/
	function getQueuedJobs($job_id=null, $post_office_id=null)
	{
		$db_table = SQ_TABLE_RUNNING_PREFIX.'bulkmail_queue';
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$where = '';
		if (!is_null($job_id)) {
			$where = ' WHERE id = '.$db->quote($job_id);
		} else if (!is_null($post_office_id)) {
			$where = ' WHERE post_office_id = '.$db->quote($post_office_id);
		}

		$sql = "SELECT id, post_office_id, status FROM $db_table $where;";
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
			trigger_error("initProgressFile : failed to init progress file.\n");
			return false;
		} else {
			echo " init progress file\n";
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
	* Stores all the job details we need to a file
	*
	* @param object	$job			the job object
	* @param object	$post_office	the post office object
	* @param string	$data_dir		path to job's effective data directory
	*
	* @return boolean
	* @access public
	*/
	function initDetailsFile($job, $post_office, $data_dir)
	{
		$details_info = Array();

		// mail queue options
		$details_info['queue'] = Array(
									'type'	=> 'flatfile',
									'dir'	=> $data_dir.'/queue',
								 );

		$details_info['recipients'] = $job->vars['recipients']['value'];
		$details_info['content'] = $job->vars['content_details']['value'];

		// whether to use post office's or job's header details
		if ($job->vars['details_setting']['value']['use_post_office_header']) {
			$details_info['header'] = $post_office->vars['header_details']['value'];
		} else {
			$details_info['header'] = $job->vars['header_details']['value'];
		}
		// removes fields with empty values, eg. empty 'From' field will cause problem
		foreach ($details_info['header'] as $field => $value) {
			if (empty($value)) {
				unset($details_info['header'][$field]);
			}
		}

		// details from post office
		$details_info['server'] = $post_office->vars['server_details']['value'];
		$details_info['threshold'] = 'not set';

		$output = ' <?php $details_info = '.var_export($details_info, true).'; ?>';
		if (!string_to_file($output, $data_dir.'/details_info')) {
			trigger_error("initDetailsFile : failed to init details file.\n");
			return false;
		} else {
			echo " init details file\n";
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


}//end class
?>
