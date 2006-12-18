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
* $Id: bulk_mailer.inc,v 1.51.2.3 2006/12/18 05:47:25 rong Exp $
*
*/

require_once dirname(__FILE__).'/../../core/include/init.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';
require_once 'Mail/Queue.php';


/**
* Bulk_Mailer
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Rayn Ong <rong@squiz.net>
*
* @version $Revision: 1.51.2.3 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class Bulk_Mailer
{

	var $error_log_file_name = 'bulkmail_errors';


	/**
	* Constructor
	*
	*/
	function Bulk_Mailer()
	{

	}//end constructor


	/**
	* Starts the process, gets called from run.php
	*
	* @return array
	* @access public
	*/
	function start()
	{
		declare(ticks=1);

		// setup signal handlers
		pcntl_signal(SIGTERM, Array(&$this,'_sigHandler'));
		pcntl_signal(SIGHUP, Array(&$this,'_sigHandler'));

		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$bm =& $am->getSystemAsset('bulkmail_manager');

		// setup the log manager to log bulkmail errors
		$old_log_errors = ini_set('log_errors', '1');
		$old_error_log  = ini_set('error_log',  SQ_LOG_PATH.'/'.$this->error_log_file_name.SQ_CONF_LOG_EXTENSION);
		set_error_handler(Array(&$this, '_errorHandler'));

		// login as root user
		$root_user =& $am->getSystemAsset('root_user');
		if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
			trigger_localised_error('BML0003', E_USER_WARNING, $root_user->name, $root_user->id);
			exit(1);
		}

		// the allowed range for mail per minute (send rate)
		$GLOBALS['min_mpm_threshold'] = 1;
		$GLOBALS['max_mpm_threshold'] = 600;
		// if the time allocated for each mail is greater than 30 minutes
		// the script will quit, and a wake up time will be stored in the progress file
		$GLOBALS['sleep_threshold'] = 1800;
		// when the last_updated field in the progress file did not get updated for 1 hour
		// job is considered as timeout-ed and we will take over this job
		$GLOBALS['timeout_threshold'] = 3600;

		// get a scheduled job from the database table and process it
		$results = $bm->getQueuedJobs();

		// pick up a timeout-ed job (higher priority than a new job)
		foreach ($results as $job_id => $info) {
			if ($info['status'] == BML_JOB_STATE_RUNNING) {
				$progress_info = $bm->getJobProgress($job_id);
				// check timeout and excluded time, make sure we take over this job if timeout_threshold is reached
				if ($this->checkThreshold('time_out', Array(), $progress_info)) {
					if ($this->checkThreshold('excluded_time', $info['details']['threshold'])) {
						$this->chunkJob($job_id);
						$this->processJob($job_id);
						return;
					}
				}
			}
		}

		// pick up a new job or a resume a paused job (normal priority)
		foreach ($results as $job_id => $info) {
			if ($info['status'] == BML_JOB_STATE_NOT_RUNNING) {
				// excluded time threshold rule
				if ($this->checkThreshold('excluded_time', $info['details']['threshold'])) {
					$bm->updateJob($job_id, BML_JOB_STATE_RUNNING);
					$this->chunkJob($job_id);
					$this->processJob($job_id);
					break;
				}
			}
		}

	}//end start()


	/**
	* Fork this server process
	*
	* @return string
	* @access public
	*/
	function fork()
	{
		$child_pid = pcntl_fork();
		switch ($child_pid) {
			case -1:
				trigger_localised_error('BML0004', E_USER_WARNING);
				return NULL;
			break;
			case 0:
				return $child_pid;
			break;
			default :
				$status = NULL;
				pcntl_waitpid(-1, $status);
				return $child_pid;
			break;
		}

	}//end fork()


	/**
	* Cache recipients into chunks using a fork process
	*
	* @param int	$job_id	the unique id of the job entry
	*
	* @return void
	* @access public
	*/
	function chunkJob($job_id)
	{
		$pid = $this->fork();
		if (!$pid) {
			// reconnect because child disconnects DB when it exits
			$this->_reconnectDB();
			$bm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');
			$info = $bm->getQueuedJobs($job_id);
			$job_path = $bm->getJobDataPath($job_id);
			$recipient_path = $job_path.'/cache/recipient';
			$progress_path = $job_path.'/progress_info';
			$recip_per_chunk = $this->calculateChunkSize($info[$job_id]['details']['threshold'], $progress_path);
			$progress_info = Array();
			if (file_exists($progress_path)) {
				include $progress_path;
			} else {
				trigger_localised_error('BML0009', E_USER_WARNING, $progress_path);
			}

			// start caching if not previously cached
			if (!is_dir($recipient_path)) {
				$result = $this->getAllRecipients($info[$job_id]['details']['content_id'], $info[$job_id]['details']['recipients']);
				// update total_count and list of problematic recipients in the progress file
				$progress_info['total_count'] = count($result['recipient']);
				$progress_info['problematic'] = $result['problematic'];
				if (!array_to_file($progress_info, 'progress_info', $progress_path)) {
					trigger_localised_error('BML0008', E_USER_WARNING, $progress_path);
				}

				// cache recipients into smaller chunks
				create_directory($recipient_path);
				$chunks = array_chunk($result['recipient'], $recip_per_chunk, TRUE);
				foreach ($chunks as $index => $chunk) {
					if (!array_to_file($chunk, 'recipient_chunk', $recipient_path.'/recipient_'.$index)) {
						trigger_localised_error('BML0008', E_USER_WARNING, $recipient_path.'/recipient_'.$index);
					}
				}
			}
			exit;
		}//end if

	}//end chunkJob()


	/**
	* Process the active bulkmail job
	*
	* @param int	$job_id	the unique id of the job entry
	*
	* @return boolean
	* @access public
	*/
	function processJob($job_id)
	{
		$this->_reconnectDB();
		$bm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');
		$info = $bm->getQueuedJobs($job_id);
		$job_path = $bm->getJobDataPath($job_id);
		$progress_path = $job_path.'/progress_info';
		$recipient_path = $job_path.'/cache/recipient';

		// load required information
		$progress_info = $info[$job_id]['progress'];
		$details = $info[$job_id]['details'];
		$header_details = $details['header_details'];
		$header_details['from'] = $details['from'];

		require_once SQ_FUDGE_PATH.'/general/text.inc';
		$keywords = retrieve_keywords_replacements($details['subject']);

		$replacements = Array();
		$content_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($details['content_id']);
		foreach ($keywords as $word) {
			$replacements[$word] = $content_asset->getKeywordReplacement($word);
		}
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($content_asset);

		$details['subject'] = replace_keywords($details['subject'], $replacements);
		$header_details['subject'] = $details['subject'];

		// create the /queue and /sent directories for mail queue
		$queue_path = $details['queue_details']['dir'];
		$sent_path = $queue_path.'/sent';
		if (!is_dir($sent_path)) create_directory($sent_path);

		$GLOBALS['chunk_start'] = get_microtime();
		// read chunks from cache and process them
		while (file_exists($recipient_path.'/recipient_'.$progress_info['current_chunk'])) {

			// check if job state is paused/cancelled at the beginning of each chunk
			if (!isset($info[$job_id])) {
				// job removed from queue table
				exit;
			} else {
				$state = $info[$job_id]['status'];
				if ($state == BML_JOB_STATE_PAUSED) {
					// job paused
					exit;
				} else if ($state == BML_JOB_STATE_NOT_RUNNING) {
					// job cancelled
					exit;
				}
			}
			// threshold checking
			if (!$this->checkThreshold('excluded_time', $details['threshold'])) {
				// current time is in excluded time range
				$bm->updateJob($job_id, BML_JOB_STATE_NOT_RUNNING);
				exit;
			}

			// load the current recipient chunk
			include($recipient_path.'/recipient_'.$progress_info['current_chunk']);

			$pid = $this->fork();
			if (!$pid) {
				$mime =& new Mail_mime("\n");
				$mail_queue =& new Mail_Queue($details['queue_details'], $details['server_details']);

				for ($recipient_chunk; NULL !== ($user_id = key($recipient_chunk)); next($recipient_chunk)) {
					$email = $recipient_chunk[$user_id];
					$this->_reconnectDB();

					// make sure user is not already in mail queue/sent directory
					$id = md5($user_id);
					if (!file_exists($queue_path.'/'.$id) && !file_exists($sent_path.'/'.$id)) {
						// fork since we cannot call printFrontend multiple times
						$pid = $this->fork();
						if (!$pid) {
							$content = $this->generateContent($details['content_id'], $details['content_gen_as'], $details['content_design'], $details['content_layout'], $job_path, $user_id);
							$user =& $GLOBALS['SQ_SYSTEM']->am->getAsset($user_id);
							$header_details['subject'] = $this->replaceContentKeywords($header_details['subject'], $GLOBALS['SQ_SYSTEM']->user);
							// only queue the mail if we can generate content
							$problematic = (empty($content)) ? TRUE : FALSE;
							if (!$problematic) {
								$mime->setHtmlBody($content);
								$body = @$mime->get();
								// pass in custom_id as extra header details, for queue file naming
								$header_details['custom_id'] = $user_id;
								$headers = @$mime->headers($header_details);
								$mail_id = $mail_queue->put('', $email, $headers, $body);
							}
							$this->updateProgress($progress_path, $user_id, $problematic);
							$this->enforceSendRate($details['threshold'], $progress_path);
							exit;
						}
					}

				}
				$status = $mail_queue->sendMailsInQueue();
				if (PEAR::isError($status)) {
					trigger_localised_error('BML0001', E_USER_WARNING, $status->getMessage());
					exit;
				}
				// proceed to next chunk
				$this->nextChunk($progress_path);
				exit;

			} else {
				// parent process, get updated info for next loop
				$this->_reconnectDB();
				$progress_info = $bm->getJobProgress($job_id);
				$info = $bm->getQueuedJobs($job_id);
			}

		}//end while

		// job completed
		$this->_reconnectDB();
		$progress_info = $bm->getJobProgress($job_id);
		if ($progress_info['current_count'] >= $progress_info['total_count']) {

			// remove job entry from the db queue table
			$bm->deleteJob($job_id);

			// if this is not an ad-hoc job, log an internal message
			$job_id_parts = explode(':', $job_id);
			if (!isset($job_id_parts[1])) {

				if (count($progress_info['problematic']) > 0) {
					trigger_localised_error('BML0010', E_USER_WARNING, implode(',', $progress_info['problematic']));
					$message_type = 'bulkmail.job.completed.error';
				} else {
					$message_type = 'bulkmail.job.completed';
				}

				// format recipient count with grouped thousands
				$msg_reps = Array('num_recip'	=> number_format($progress_info['total_count']));
				$ms =& $GLOBALS['SQ_SYSTEM']->getMessagingService();
				$message = $ms->newMessage(Array(), $message_type, $msg_reps);
				$message->parameters['assetid'] = $job_id;
				$message->send();
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($message);
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($ms);
			}

		}
		return TRUE;

	}//end processJob()


//--        PROGRESS        --//


	/**
	* Reconnects to the DB as child process disconnects DB when it exits
	*
	* @return void
	* @access private
	*/
	function _reconnectDB()
	{
		// need to reset both, so that other processes wont break
		if (!empty($GLOBALS['SQ_SYSTEM']->_db_conns['db2'])) {
			$GLOBALS['SQ_SYSTEM']->_db_conns['db2']->disconnect();
		}
		$GLOBALS['SQ_SYSTEM']->_db_conns['db']->disconnect();
		$GLOBALS['SQ_SYSTEM']->_db_stack = Array();
		$GLOBALS['SQ_SYSTEM']->_db_conns = Array();
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db');

	}//end _reconnectDB()


	/**
	* Writes the progress info to a file after a mail has been added to the queue
	*
	* @param string		$progress_path	path to the progress info file
	* @param int		$user_id		current recipient assetid
	* @param boolean	$problematic	whether current user is a problematic one or not
	*
	* @return boolean
	* @access public
	*/
	function updateProgress($progress_path, $user_id, $problematic)
	{
		$progress_info = Array();
		if (file_exists($progress_path)) {
			include $progress_path;
		} else {
			trigger_localised_error('BML0009', E_USER_WARNING, $progress_path);
			return FALSE;
		}

		if ($problematic) {
			$progress_info['total_count']--;
			$progress_info['problematic'][] = $user_id;
		} else {
			$progress_info['current_count']++;
		}
		$progress_info['current_recip_id'] = $user_id;
		$progress_info['last_updated'] = time();

		if (!array_to_file($progress_info, 'progress_info', $progress_path)) {
			trigger_localised_error('BML0008', E_USER_WARNING, $progress_path);
			return FALSE;
		} else {
			return TRUE;
		}

	}//end updateProgress()


	/**
	* Proceed to next recipient chunk
	*
	* @param string	$progress_path	path to the progress info file
	*
	* @return boolean
	* @access public
	*/
	function nextChunk($progress_path)
	{
		$progress_info = Array();
		if (file_exists($progress_path)) {
			include $progress_path;
		} else {
			trigger_localised_error('BML0009', E_USER_WARNING, $progress_path);
			return FALSE;
		}

		$progress_info['current_chunk']++;
		// record the time taken to process one recipient chunk
		if (!$progress_info['time_per_chunk']) {
			$progress_info['time_per_chunk'] = get_microtime() - $GLOBALS['chunk_start'];
		}

		if (!array_to_file($progress_info, 'progress_info', $progress_path)) {
			trigger_localised_error('BML0008', E_USER_WARNING, $progress_path);
			return FALSE;
		} else {
			return TRUE;
		}

	}//end nextChunk()


//--        RECIPIENT        --//


	/**
	* Cache all recipients (user_id => email) for a job
	*
	* Warning: $result array can be huge
	* Expands user_group to get all user/bulkmail_user
	* Skips those recipients that is in .queue or .sent directory
	*
	* @param int	$asset_to_send	the id of the asset to send
	* @param array	$recipients		the job's 'recipients' attribute stripped from vars
	*
	* @return array
	* @access public
	*/
	function getAllRecipients($asset_to_send, $recipients)
	{
		// handle groups and users/bulkmail_users separately
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$user_list = Array();
		$group_list = Array();
		$problematic_list = Array();

		if (!is_array($recipients)) {
			$recipients = Array($recipients);
		}

		// an array of userids that have read access to the asset to send (content page)
		// if public user has read access, then everyone should be able to read it
		$read_permission = $am->getPermission($asset_to_send, SQ_PERMISSION_READ, TRUE);
		$public_user =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('public_user');
		$public_read_access = in_array($public_user->id, $read_permission);

		foreach ($recipients as $recipient_id) {
			// only add to user list if this recipient has read permission
			if ($public_read_access || in_array($recipient_id, $read_permission)) {
				$recipient =& $am->getAsset($recipient_id);
				if (!is_null($recipient)) {
					if (is_a($recipient, 'user') || is_a($recipient, 'bulkmail_user')) {
						$user_list += Array($recipient_id => $recipient->attr('email'));
					// store all group ids
					} else if (is_a($recipient, 'user_group')) {
						$group_ids[] = $recipient_id;
					}
				}
				$am->forgetAsset($recipient);
			} else {
				// problematic recipient: no read access to content
				$problematic_list[] = $recipient_id;
			}
		}

		// expand groups and get all users/bulkmail_users from a query
		if (!empty($group_ids)) {
			$type_codes = Array(
							'user'			=> FALSE,
							'bulkmail_user'	=> TRUE,
						  );
			$query = $this->generateGetRecipientsQuery($group_ids, $type_codes);
			$group_list = $GLOBALS['SQ_SYSTEM']->db->getAssoc($query);
			assert_valid_db_result($group_list);
		}

		// combine group/user results
		$result['recipient'] = $group_list + $user_list;
		$result['problematic'] = $problematic_list;
		return $result;

	}//end getAllRecipients()


	/**
	* Generates the query to get chunk of recipient ids and email addresses
	*
	* @param array	$group_ids	the id of the group assets to get their children for
	* @param array	$type_codes	the type of asset that is linked
	*							and whether we are finding assets that are just a $type_code (strict)
	*							or $type_code and any of it's sub-classes (not strict)
	*							format: type_code => strict ? TRUE : FALSE
	*
	* @return string
	* @access public
	*/
	function generateGetRecipientsQuery($group_ids, $type_codes)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;
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
			for (reset($type_codes); NULL !== ($key = key($type_codes)); next($type_codes)) {
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


//--        CONTENT        --//


	/**
	* Generates the html bulkmail content based on the user and content generation type
	*
	* Used in previewing mail and sending actual bmail (use each user or one user)
	* Pre-selected user is used when we cannot login bulkmail_users
	*
	* @param int	$content_id		the assetid of the content page
	* @param string	$content_gen_as	the content generation type, use each user or use one user
	* @param int	$content_design	the assetid of the effective design
	* @param int	$content_layout	the assetid of the effective layout
	* @param string	$job_path		path to the effective job data dir
	* @param int	$recipient_id	the recipient of this email, zero is for preview mail
	*
	* @return string
	* @access public
	*/
	function generateContent($content_id, $content_gen_as, $content_design, $content_layout, $job_path, $recipient_id=0)
	{
		// Get the user before we change it
		$current_user =& $GLOBALS['SQ_SYSTEM']->user;

		$am = $GLOBALS['SQ_SYSTEM']->am;
		$generateUser =& $am->getAsset($content_gen_as);
		$replaceUser =& $generateUser;

		$cache = TRUE;
		if (!$recipient_id) {
			$replaceUser =& $current_user;
			$cache = FALSE;
		}

		if ($recipient_id) {
			$replaceUser =& $am->getAsset($recipient_id);
			if (!is_a($replaceUser, 'Bulkmail_User')) {
				$generateUser =& $replaceUser;
			}
		}

		if (is_null($generateUser) || is_null($replaceUser)) {
			return '';
		}

		if ((file_exists($job_path.'/cache/content_file.html')) && ($content_gen_as == $generateUser->id)) {
			$content = file_to_string($job_path.'/cache/content_file.html');
		} else {
			if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($generateUser)) {
				return '';
			}
			$page =& $am->getAsset($content_id);

			ob_start();
				// generate content based on design and paint layout supplied
				if ($content_design && $content_layout) {
					$GLOBALS['SQ_SYSTEM']->setGlobalDefine('SQ_PAINT_LAYOUT_ID', $content_layout);
					$design =& $am->getAsset($content_design);
					$design->paint($page);
				} else if ($content_design) {
					$design =& $am->getAsset($content_design);
					$design->paint($page);
				} else if ($content_layout) {
					$layout =& $am->getAsset($content_layout);
					$layout->paint($page);
				} else {
					@$page->printFrontend();
				}
				$content = ob_get_contents();
			ob_end_clean();

			if (($content_gen_as == $generateUser->id) && !$cache) {
				string_to_file($content, $job_path.'/cache/content_file.html');
			}
		}

		$content = Bulk_Mailer::replaceContentKeywords($content, $replaceUser);

		// Restore the old user again
		$GLOBALS['SQ_SYSTEM']->setCurrentUser($current_user);

		return $content;

	}//end generateContent()


	/**
	* Keyword replacement for customised email content of a user
	*
	* @param string	$content	page content before keyword replacement
	* @param int	$user		personalise content for this user
	*
	* @return string
	* @access public
	*/
	function replaceContentKeywords($content, $user)
	{
		// to insert user keywords in the 'Asset to Send' content,
		// add 'bulkmail_' prefix in front of normal keywords
		require_once SQ_FUDGE_PATH.'/general/text.inc';
		$keywords = $user->getAvailableKeywords();
		$keyword_replacements = Array();
		foreach ($keywords as $key => $val) {
			$keyword_replacements['recipient_'.$key] = $user->getKeywordReplacement($key);
		}
		$content = replace_keywords($content, $keyword_replacements);

		return $content;

	}//end replaceContentKeywords()


//--        THRESHOLD        --//


	/**
	* Calculate the recipient chunk size based on the 'send rate' threshold
	*
	* @param array	$thresholds		the threshold attribute of the post office
	* @param string	$progress_path	the progress path of the job
	*
	* @return boolean
	* @access public
	*/
	function calculateChunkSize($thresholds, $progress_path)
	{
		$mpm = 0;
		foreach ($thresholds as $threshold) {
			if ($threshold['type'] == translate('bulkmail_threshold_rule_send_rate')) {
				$value = $threshold['params']['value'];
				$unit = $threshold['params']['unit'];
				$mpm = ceil(($value/$unit) * 60);
				// mail per minute setting is not within allowed range
				if ($mpm < $GLOBALS['min_mpm_threshold'] || $mpm > $GLOBALS['max_mpm_threshold']) {
					$mpm = 0;
				}
			}
		}
		// use default chunk_size if 'send rate' rule is not defined or not achievable
		if (!$mpm) {
			$bm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');
			$mpm = $bm->attr('chunk_size');
		}

		// store mpm value in progress file
		$progress_info = Array();
		if (file_exists($progress_path)) {
			include $progress_path;
		} else {
			trigger_localised_error('BML0009', E_USER_WARNING, $progress_path);
		}
		$progress_info['mpm'] = $mpm;
		if (!array_to_file($progress_info, 'progress_info', $progress_path)) {
			trigger_localised_error('BML0008', E_USER_WARNING, $progress_path);
		}

		return $mpm;

	}//end calculateChunkSize()


	/**
	* Apply the send rate rule by putting the script to sleep after each recipient
	*
	* @param array	$thresholds		the threshold attribute of the post office
	* @param string	$progress_path	the progress path of the job
	*
	* @return boolean
	* @access public
	*/
	function enforceSendRate($thresholds, $progress_path)
	{
		$progress_info = Array();
		if (file_exists($progress_path)) {
			include $progress_path;
		} else {
			trigger_localised_error('BML0009', E_USER_WARNING, $progress_path);
			return FALSE;
		}
		if (!isset($progress_info['mpm'])) {
			// send rate rule is not defined
			return TRUE;
		}

		// finished processing one recipient chunk, work out the time to sleep between each recipient
		if (($progress_info['time_per_chunk'] != 0) && (!isset($progress_info['sleep_time']))) {
			$time_diff = 60 - $progress_info['time_per_chunk'];
			$progress_info['sleep_time'] = -1;
			if ($time_diff > 0) {
				// store the sleep time between each recipient (in microseconds)
				$chunk_size = $this->calculateChunkSize($thresholds, $progress_path);
				$progress_info['sleep_time'] = ($time_diff/$chunk_size) * 1000000;
				// sleep for the first chunk
				usleep($progress_info['sleep_time'] * $chunk_size);
			}

			// write the sleep time between recipient to the progress file
			if (!array_to_file($progress_info, 'progress_info', $progress_path)) {
				trigger_localised_error('BML0008', E_USER_WARNING, $progress_path);
			}
		}

		// sleep between each recipient
		if (isset($progress_info['sleep_time']) && ($progress_info['sleep_time'] != -1)) {
			usleep($progress_info['sleep_time']);
		}

		return TRUE;

	}//end enforceSendRate()


	/**
	* Checks a particular threshold rule
	*
	* @param string	$type		the type of the thresholding rule to check
	* @param array	$thresholds	the threshold attribute of the post office
	* @param array	$extra		extra param that needs to be passed in
	*
	* @return boolean
	* @access public
	*/
	function checkThreshold($type, $thresholds, $extra='')
	{
		switch ($type) {
			case 'excluded_time' :
				return $this->isExcludedTime($thresholds);
			break;
			case 'wakeup_time' :
				return $this->checkWakeUpTime($thresholds, $extra);
			break;
			case 'time_out' :
				return $this->checkTimeOut($thresholds, $extra);
			break;
			default:
				return FALSE;
			break;
		}

	}//end checkThreshold()


	/**
	* Check if current time is within excluded time threshold rule
	*
	* @param array	$thresholds	the threshold attribute of the post office
	*
	* @return boolean
	* @access public
	*/
	function isExcludedTime($thresholds)
	{

		$days_array = Array(
						0	=> 'Sun',
						1	=> 'Mon',
						2	=> 'Tue',
						3	=> 'Wed',
						4	=> 'Thu',
						5	=> 'Fri',
						6	=> 'Sat',
					  );

		foreach ($thresholds as $threshold) {
			if ($threshold['type'] == translate('bulkmail_threshold_rule_excluded_time')) {
				foreach ($threshold['params']['days'] as $excluded_day) {
					$excluded_days[] = $days_array[$excluded_day];
				}
				$now_day = date('D', time()); // A textual representation of a day, three letters, eg. 'Mon'
				$now_time = date('G', time()); // 24-hour format of an hour without leading zeros, eg. 8, 15
				if (in_array($now_day, $excluded_days)) {
					// range: [from, to)
					if ($now_time >= $threshold['params']['from'] && (($threshold['params']['to'] < $threshold['params']['from']) || ($now_time < $threshold['params']['to']))) {
						return FALSE;
					}
				} else if ($threshold['params']['from'] > $threshold['params']['to']) {
					// exclude from previous day X hour to today Y hour
					$yesterday_day = date('D', time()-86400);
					if (in_array($yesterday_day, $excluded_days)) {
						if ($now_time < $threshold['params']['to']) {
							return FALSE;
						}
					}
				}
			}
		}
		return TRUE;

	}//end isExcludedTime()


	/**
	* Check if a running job is timeout-ed
	*
	* @param array	$thresholds		the threshold attribute of the post office
	* @param array	$progress_info	the current progress info from the progress file
	*
	* @return boolean
	* @access public
	*/
	function checkTimeOut($thresholds, $progress_info)
	{
		if (isset($progress_info['last_updated']) && ($progress_info['last_updated'] != 0)) {
			$time_diff = time() - $progress_info['last_updated'];
			// timeout threshold reached, take over
			if ($time_diff > $GLOBALS['timeout_threshold']) {
				return TRUE;
			}
		}
		return FALSE;

	}//end checkTimeOut()


//--        HANDLER        --//


	/**
	* Signal handler for the pcntl fork process
	*
	* @param int	$signo	the signals that are supported by the PHP Process Control functions
	*
	* @return void
	* @access private
	*/
	function _sigHandler($signo)
	{
		switch ($signo) {
			case SIGTERM:
				exit;
			break;
			case SIGHUP:
			case SIGUSR1:
			default:
		}

	}//end _sigHandler()


	/**
	* This function captures all errors and warnings that occur during the execution of start()
	*
	* @param int	$err_no		The type of error (E_*)
	* @param string	$err_msg	The error message
	* @param string	$err_file	The file the error occured in
	* @param string	$err_line	The line the error occured on
	*
	* @return void
	* @access private
	* @see Cron_Manager::_errorHandler()
	*/
	function _errorHandler($err_no, $err_msg, $err_file, $err_line)
	{
		$terminate = ((E_USER_ERROR | E_ERROR) & $err_no);

		// if the function didn't have an '@' prepended OR if we are about to terminate
		// catch the error
		if (error_reporting() || $terminate) {

			$bt = debug_backtrace();
			if (count($bt) > 1) {
				$real_bt_index = 0;
				// note the use of '_errorhandler' instead of '_errorHandler' in the last condition
				while (($real_bt_index < count($bt)-1) && ((FALSE !== strpos(array_get_index($bt[$real_bt_index], 'class'), 'locale_manager')) || (FALSE !== strpos(array_get_index($bt[$real_bt_index], 'file'), 'locale_manager')) || ($bt[$real_bt_index]['function'] == '_errorhandler'))) {
					$real_bt_index++;
				}
				$err_file = hide_system_root($bt[$real_bt_index]['file']);
				$err_line = $bt[$real_bt_index]['line'];
				$err_msg  = hide_system_root($err_msg);
			}

			$text_msg = strip_tags(preg_replace(Array('/<br\\/?>/i', '/<p[^>]*>/i'), Array("\n", "\n\n"), $err_msg));

			// send a report to the system error log
			if (ini_get('log_errors')) {
				log_error($text_msg, $err_no, $err_file, $err_line, $this->error_log_file_name);
			}

		}//end error_reporting

		if ($terminate) exit(1);

	}//end _errorHandler()


}//end class


?>
