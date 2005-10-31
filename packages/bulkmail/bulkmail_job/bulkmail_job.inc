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
* $Id: bulkmail_job.inc,v 1.7 2005/10/31 04:20:34 rong Exp $
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
* @version $Revision: 1.7 $
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

		// if we cant find a post office, use the system default post office
		if (is_null($post_office)) {
			//$post_office =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');
		}

		return $post_office;

	}//end getPostOffice()


	/**
	* Returns the progress path to the progress info file
	*
	* @return string
	* @access public
	*/
	function getProgressPath()
	{
		// ad-hoc job with assetid of zero, return data-dir path of the default system post office
		// use a unique id to differentiate each ad-hoc job
		if (!$this->id) {
			$unique_id = uniqid('');
			// TODO: create and use a default system asset - default_bulkmail_post_office, underneath bulkmail_manager
			//$default_post_office =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');
			//echo ' data path of this ad-hoc job => '.$default_post_office->data_path."$unique_id/.data/progress_info\n";
			//return $default_post_office->data_path."$unique_id/.data/progress_info";
			return '';

		// normal job, store info in its data-dir
		} else {
			//echo ' data path of this normal job => '.$this->data_path."/.data/progress_info\n";
			return $this->data_path.'/.data/progress_info';
		}

	}//end getProgressPath()


 /**
		* Sends the bulkmail message using given PEAR mail driver params
		*
		* @param object &$asset         the asset to which we belong
		* @param mixed  $recipient      an array or a string with comma separated recipients.
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
	* Get the job's or post office's server details
	*
	* @return array
	* @access public
	*/
	function getServerDetails()
	{
		$post_office =& $this->getPostOffice();
		if (!is_null($post_office)) {
			return $post_office->attr('server_details');
		}

		return Array();

	}//end getServerDetails()


	/**
	* Get the job's or post office's header details
	*
	* @return array
	* @access public
	*/
	function getHeaderDetails()
	{
		$details_setting = $this->attr('details_setting');
		$asset = null;

		// determine whether to use job's or post office's header
		if ($details_setting['use_post_office_header']) {
			$asset =& $this->getPostOffice();
		} else {
			$asset =& $this;
		}

		$header_details = Array();
		if (!is_null($asset)) {
			$header_details = $asset->attr('header_details');
			// removes fields with empty values, eg. empty 'From' field will cause problem
			foreach ($header_details as $field => $value) {
				if (empty($value)) unset($header_details[$field]);
			}
		}
		return $header_details;

	}//end getHeaderDetails()


	/**
	* Get queue options for mail queue
	*
	* @return array
	* @access public
	*/
	function getQueueOptions()
	{
		return Array(
				'type'	=> 'flatfile',
				'dir'	=> $this->data_path.'/.data/queue',
			   );

	}//end getQueueOptions()


	/**
	* Cache all recipients (user_id => email)
	*
	* Warning: $result array can be huge
	* Expands user_group to get all user/bulkmail_user
	* Skips those recipients that is in .queue or .sent directory
	*
	* @return array
	* @access public
	*/
	function getRecipients()
	{
		// handle groups and users/bulkmail_users separately
		$recipients = $this->attr('recipients');
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$user_list = Array();
		$group_ids = Array();

		foreach ($recipients as $recipient_id) {
			$recipient =& $am->getAsset($recipient_id);

			// store all users/bulkmail_users (user_id => email)
			if (!is_null($recipient) && (is_a($recipient, 'user') || is_a($recipient, 'bulkmail_user'))) {
				$user_list += Array($recipient_id => $recipient->attr('email'));

			// store all group ids
			} else if (!is_null($recipient) && is_a($recipient, 'user_group')) {
				$group_ids[] = $recipient_id;
			}

			$am->forgetAsset($recipient);
		}

		// expand groups and get all users/bulkmail_users from a query
		$type_codes = Array(
						'user'			=> false,
						'bulkmail_user'	=> true,
					  );
		$query = $this->generateGetRecipientsQuery($group_ids, $type_codes);
		$result = $GLOBALS['SQ_SYSTEM']->db->getAssoc($query);
		assert_valid_db_result($result);

		// combine group/user results
		$result += $user_list;
		//bam($result);

		// code for multiplying the recipients, eg. turn 10 actual users into 500 users
		// for testing purposes
		/*foreach($result as $user_id => $email) {
			$count = 0;
			while($count<100){
				$result += Array( ($user_id * $count) => $email.'.'.$count.'.net');
				$count++;
			}
		}*/

		return $result;

	}//end getRecipients()


	/**
	* Generates the query to get chunk of recipient ids and email addresses
	*
	* @param array	$group_ids	the id of the group assets to get their children for
	* @param array	$type_codes	the type of asset that is linked
	*							and whether we are finding assets that are just a $type_code (strict)
	*							or $type_code and any of it's sub-classes (not strict)
	*							format: type_code => strict ? true : false
	*
	* @return string
	* @access public
	*/
	function generateGetRecipientsQuery($group_ids, $type_codes)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$select = 'SELECT DISTINCT l.minorid, val.custom_val';
		$from   = 'FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree t
						 INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l ON t.linkid = l.linkid
						 INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a ON l.minorid = a.assetid
						 INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_attr_val val ON a.assetid = val.assetid
						 INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_attr attr ON attr.attrid = val.attrid
				  ';

		// get all link tree ids of the groups
		$treeids = Array();
		foreach ($group_ids as $group_id) {
			$where = 'l.minorid = '.$db->quote($group_id);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
			$sql = 'SELECT t.treeid
					FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree t INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l ON t.linkid = l.linkid
					'.$where;
			$sql = $db->modifyLimitQuery($sql, 0, 1);
			$treeid = $db->getOne($sql);
			assert_valid_db_result($treeid);
			$treeids[] = $treeid;
		}
		// generate the where clause to match link tree id for each group
		$where = 'WHERE attr.name = \'email\' ';
		if (!empty($treeids)) {
			$where .= ' AND (';
			$count = 0;
			foreach ($treeids as $treeid) {
				if ($count) $where .= 'OR ';
				$where .= '(t.treeid LIKE '.$db->quote($treeid.'%').'AND t.treeid > '.$db->quote($treeid).') ';
				$count++;
			}
			$where .= ')';
		}
		// generate where condition to match all type_codes (either strict or not strict)
		if (!empty($type_codes)) {
			$where .= ' AND (';
			$count = 0;
			for (reset($type_codes); null !== ($key = key($type_codes)); next($type_codes)) {
				$type_codes_cond = '= '.$db->quote($key);
				if ($count) $where .= 'OR ';
				// is strict type_codes
				if ($type_codes[$key]) {
					$where .= '(a.type_code '.$type_codes_cond.')';
				} else {
					$where .= '(a.type_code IN (
											SELECT type_code
											FROM sq_ast_typ_inhd
											WHERE inhd_type_code '.$type_codes_cond.'
											))';
				}
				$count++;
			}
			$where .= ')';
		}//end if

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
		$query = Array(
					'select'	=> $select,
					'from'		=> $from,
					'where'		=> $where,
				 );

		return implode(' ', $query);
	}//end generateGetRecipientsQuery()


	/**
	* Checks if mail has already been queued for this user
	*
	* @param int	$user_id	the user assetid that we want to check
	*
	* @return boolean
	* @access public
	*/
	function isUserInQueue($user_id)
	{
		$path = $this->data_path.'/.data/queue/';
		$id = md5($user_id);
		if (file_exists($path.$id)) return true;

		return false;

	}//end isUserInQueue()


	/**
	* Checks if mail has already been sent for this user
	*
	* @param int	$user_id	the user assetid that we want to check
	*
	* @return boolean
	* @access public
	*/
	function isUserInSent($user_id)
	{
		$path = $this->data_path.'/.data/queue/sent/';
		$id = md5($user_id);
		if (file_exists($path.$id)) return true;

		return false;

	}//end isUserInSent()


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
		$old_status = $this->status;
		$state = $this->attr('current_state');
		// once the job is live, the only way to change it's status is by cancelling this job from the backend
		if ($old_status == SQ_STATUS_LIVE) {
			// maybe if the user set job to under construction, we cancel the job automatically?
			if ($state == BML_JOB_STATE_NOT_LIVE) {
				return parent::processStatusChange($new_status, $update_parents);
			}

		// if the job is not live, only set it to live if the job is valid
		} else {
			$error = $this->canSetLive();
			// can set live
			if (empty($error)) {
				// purge the data directory
				if (is_dir($this->data_path.'/.data')) {
					delete_directory($this->data_path.'/.data');
				}
				// set the job to live and running
				if ($state == BML_JOB_STATE_NOT_LIVE) {
					$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
					$this->setAttrValue('current_state', BML_JOB_STATE_RUNNING);
					$this->saveAttributes();
					$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
				}
				return parent::processStatusChange($new_status, $update_parents);
			}
		}

		return false;

	}//end processStatusChange()


	/**
	* Gets a custom attribute value directly from the database, as attr() doesn't return the updated value
	*
	* @param string	$name	the name of the custom attribute
	*
	* @return string
	* @access public
	*/
	function getAttrDB($name)
	{
		$sql = 'SELECT val.custom_val ';
		$sql .= 'FROM sq_ast_attr attr INNER JOIN sq_ast_attr_val val ON attr.attrid = val.attrid ';
		$sql .= "WHERE val.assetid = '".$this->id."' AND attr.name = '$name'";
		$result = $GLOBALS['SQ_SYSTEM']->db->getOne($sql);
		assert_valid_db_result($result);

		return $result;

	}//end getAttrDB()


	/**
	* Static function, returns the assetid of the current live job (if any)
	*
	* @return mixed int|boolean
	* @access public
	*/
	function getLiveJob()
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$strict = true;

		// get all bulkmail post offices and jobs that are currently in the trash folder
		$trash_folder = &$am->getSystemAsset('trash_folder');
		$in_trash = $am->getChildren($trash_folder->id, 'bulkmail_post_office', !$strict);

		// get all post offices that are not in trash
		// note: a post office may be in trash while the job is not, and we dont want that
		$post_offices = $am->getTypeAssetids('bulkmail_post_office', $strict, true);
		$post_offices = array_diff_assoc($post_offices, $in_trash);

		// get all jobs that are linked under a post office, and not in trash
		$jobs = Array();
		foreach ($post_offices as $post_office_assetid => $type_code) {
			$jobs += $am->getChildren($post_office_assetid, 'bulkmail_job', $strict);
		}
		$jobs = array_diff_assoc($jobs, $in_trash);

		// get live job (only one live job at any given point of time)
		foreach ($jobs as $job_assetid => $type_code) {
			$status = Bulkmail_Job::getStatusDB($job_assetid);
			if ($status == SQ_STATUS_LIVE) return $job_assetid;
		}

		return false;

	}//end getLiveJob()


	/**
	* Returns an array of errors or empty array if we can set this job to live
	*
	* @return array
	* @access public
	*/
	function canSetLive()
	{
		$error = Array();

		/*
		// db table allows multiple live jobs running at the same time
		// is there another live job running?
		// probably check this part when setting the status to live for real
		$current_live_job = Bulkmail_Job::getLiveJob();
		if (!empty($current_live_job) && $current_live_job != $this->id) {
			//$error[] = "Another Bulkmail Job (Id #$current_live_job) is currently Live.\n";
			$error[] = translate('bulkmail_warning_job_live', $current_live_job);
		}
		*/

		// is this job linked under a post office?
		$post_office =& $this->getPostOffice();
		if (is_null($post_office)) {
			//$error[] = "Post Office Error: Post Office not set.\n";
			// also Default System Post Office not set
			$error[] = translate('bulkmail_warning_po_post_office_not_set');
		} else {
			// get server details from post office
			$server_details = $this->getServerDetails();
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
		$header_details = $this->getHeaderDetails();
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
			if (!$user->canLogin()){
				$type = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($user->type(), 'name');
				//$error[] = "Content Details Error: Unable to generate content as $type \"".$user->name."\" (Id #$generate_as), user is either in trash or not live.\n";
				$error[] = translate('bulkmail_warning_content_generate_as_login_failed', $type, $user->name, $user->id);
			}
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($user);
		}

		return $error;

	}//end canSetLive()


//--        PROGRESS        --//


	/**
	* Initialises the progress info file
	*
	* @param string		$progress_path	path to the progress info file
	*									use job's data-dir path if it is a normal job
	*									use system default post office data-dir path for an ad-hoc job
	* @param int		$total_count	total number of recipients for a job
	*
	* @return boolean
	* @access private
	*/
	function _initProgressFile($progress_path, $total_count)
	{
		/*
			available fields:
			current_count		processed this number of mails
			current_recip_id	assetid of the current recipient/user
								keep track of this as recipients are not in numerical order
			current_chunk		currently up to this chunk of recipient
			last_updated		timestamp of when we last added a mail to the queue
			total_count			total number of mails to send
		*/
		$progress_info = Array();
		$progress_info['current_count'] = 0;
		$progress_info['current_recip_id'] = 0;
		$progress_info['current_chunk'] = 0;
		$progress_info['last_updated'] = 0;
		$progress_info['total_count'] = $total_count;

		$output = '<?php $progress_info = '.var_export($progress_info, true).'; ?>';
		if (!string_to_file($output, $progress_path)) {
			return false;
		} else {
			echo " init progress file\n";
			return true;
		}

	}//end _initProgressFile()


	/**
	* Writes the progress info to a file after a mail has been added to the queue
	*
	* @param string		$progress_path	path to the progress info file
	* @param int		$user_id		current recipient assetid
	* @param string		$email			current recipient email address
	*
	* @return boolean
	* @access private
	*/
	function _updateProgressFile($progress_path, $user_id, $email)
	{
		if (file_exists($progress_path)) include $progress_path;
		$progress_info['current_count']++;
		$progress_info['current_recip_id'] = $user_id;
		//$progress_info['current_recip_email'] = $email;
		$progress_info['last_updated'] = time();

		$output = '<?php $progress_info = '.var_export($progress_info, true).'; ?>';
		if (!string_to_file($output, $progress_path)) {
			echo "bulk_mailer.inc:_updateProgressFile - Unable to write to file \"$progress_path\"\n";
			return false;
		} else {
			echo ' processed '.$progress_info['current_count']." (Id #$user_id) $email  ... sleep(5)\n";
			sleep(5); // for testing purposes
			return true;
		}

	}//end _updateProgressFile()


	/**
	* Reads the progress info from a file
	*
	* @param string	$progress_path	path to the progress info file
	*
	* @return array
	* @access private
	*/
	function _readProgressFile($progress_path)
	{
		$progress_info = Array();
		if (file_exists($progress_path)) include $progress_path;

		return $progress_info;

	}//end _readProgressFile()


	/**
	* Proceed to next recipient chunk
	*
	* @param string	$progress_path	path to the progress info file
	*
	* @return boolean
	* @access private
	*/
	function _nextChunk($progress_path)
	{
		$progress_info = Array();
		if (file_exists($progress_path)) include $progress_path;
		$progress_info['current_chunk']++;
		$output = '<?php $progress_info = '.var_export($progress_info, true).'; ?>';
		if (!string_to_file($output, $progress_path)) {
			echo "bulk_mailer.inc:_nextChunk - Unable to write to file \"$progress_path\"\n";
			return false;
		} else {
			echo ' proceed to next chunk'.$progress_info['current_chunk']."\n";
			return true;
		}

		return $progress_info;

	}//end _nextChunk()


}//end class
?>