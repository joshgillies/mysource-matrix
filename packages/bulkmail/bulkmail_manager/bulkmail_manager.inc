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
* @version $Revision: 1.1 $
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
		// if no post office is supplied, work out which one to use
		if (is_null($post_office)) {
			if ($job->id == 0) {
				$post_office =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_post_office_default');
			} else {
				$post_office =& $job->getPostOffice();
			}
		}

		// check isValidJob here

		// work out the unique id for this job in the db table
		if ($job->id == 0) {
			// use random id based on current timestamp for ad-hoc jobs
			// check if this id has been taken by another ad-hoc job
			do {
				$unique_id = md5(uniqid(''));
				$data_dir = $post_office->data_path.'/.data/'.$unique_id.'/';
			} while (is_dir($data_dir));
		} else {
			$unique_id = $job->id;
		}

		// get the effective data path for progress file
		// initialise the progress file, set total count to zero
		$data_dir = $post_office->data_path.'/.data/'.$unique_id.'/';
		echo "data dir = $data_dir\n";
		$this->initProgressFile($data_dir);

		// dump vars to file

		// add entry to the database table
		$db_name = SQ_TABLE_RUNNING_PREFIX.'bulkmail_queue';
		$fields = Array('id', 'post_office_id', 'status');
		$db_values = "'$unique_id', '".$post_office->id."', '".BML_JOB_STATE_NOT_RUNNING."'";
		$db_fields = implode(', ', $fields);
		$sql = "INSERT INTO $db_name ($db_fields) VALUES ($db_values);";
		echo $sql;
		$result = $GLOBALS['SQ_SYSTEM']->db->query($sql);

		return assert_valid_db_result($result);

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
		$db_name = SQ_TABLE_RUNNING_PREFIX.'bulkmail_queue';
		$sql = "DELETE FROM $db_name WHERE id = $job_id;";
		$result = $GLOBALS['SQ_SYSTEM']->db->query($sql);
		echo " deleted $job_id\n";

		return assert_valid_db_result($result);

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
		$db_name = SQ_TABLE_RUNNING_PREFIX.'bulkmail_queue';
		$sql = "UPDATE $db_name SET status = $state WHERE id = $job_id;";
		$result = $GLOBALS['SQ_SYSTEM']->db->query($sql);
		echo " updated state to $state\n";

		return assert_valid_db_result($result);

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
		$db_name = SQ_TABLE_RUNNING_PREFIX.'bulkmail_queue';
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$where = '';
		if (!is_null($job_id)) {
			$where = ' WHERE id = '.$db->quote($job_id);
		} else if (!is_null($post_office_id)) {
			$where = ' WHERE post_office_id = '.$db->quote($post_office_id);
		}
		$sql = 'SELECT id, post_office_id, status FROM sq_bulkmail_queue'.$where.';';

		$result = $db->getAssoc($sql);
		// add progress info here

		assert_valid_db_result($result);

		return $result;

	}//end getQueuedJobs()


	/**
	* Returns an array of job ids and their states for a particular post office
	*
	* @param int	$job_id	the unique id of the job entry
	* @param int	$po_id	the asset id of the post office
	*
	* @return array
	* @access public
	*/
	function getJobInfo($job_id, $po_id)
	{
		$uniq_id = md5($job_id);
		$post_office =& $GLOBALS['SQ_SYSTEM']->am->getAsset($po_id);
		$data_dir = $post_office->data_path.'/'.$uniq_id.'/.data/progress_info';
		echo "$data_dir\n";
		return '';

	}//end getJobInfo()


//--        PROGRESS        --//


	/**
	* Initialises the progress info file
	*
	* @param string	$progress_path	path to the directory containing the progress info file
	*								use job's data-dir path if it is a normal job
	*								use system default post office data-dir path for an ad-hoc job
	*
	* @return boolean
	* @access public
	*/
	function initProgressFile($progress_path)
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
		$progress_info = Array();
		$progress_info['current_count'] = 0;
		$progress_info['current_recip_id'] = 0;
		$progress_info['current_chunk'] = 0;
		$progress_info['last_updated'] = 0;
		$progress_info['total_count'] = 0;

		$output = '<?php $progress_info = '.var_export($progress_info, true).'; ?>';
		if (!create_directory($progress_path)) {
			create_directory($progress_path);
		}
		if (!string_to_file($output, $progress_path.'/progress_info')) {
			echo " error: failed to init progress file\n";
			return false;
		} else {
			echo " init progress file\n";
			return true;
		}

	}//end initProgressFile()


	/**
	* Reads the progress info from a file
	*
	* @param string	$progress_path	path to the progress info file
	*
	* @return array
	* @access public
	*/
	function readProgressFile($progress_path)
	{
		$progress_info = Array();
		if (file_exists($progress_path.'/progress_info')) {
			include $progress_path.'/progress_info';
		}

		return $progress_info;

	}//end readProgressFile()


}//end class
?>
