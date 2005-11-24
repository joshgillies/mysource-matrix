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
* $Id: bulk_mailer.inc,v 1.34 2005/11/24 00:44:56 ndvries Exp $
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
* @version $Revision: 1.34 $
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
		//speed_check(); mem_check('start '.ts_iso8601(time()));

		// setup signal handlers
		pcntl_signal(SIGTERM, Array(&$this,'_sigHandler'));
		pcntl_signal(SIGHUP, Array(&$this,'_sigHandler'));

		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$bm =& $am->getSystemAsset('bulkmail_manager');
		//speed_check(); mem_check('bulkmail manager loaded');

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
		//speed_check(); mem_check('root user loaded');

		// constant definition
		$recip_per_chunk = 10;
		// if the time allocated for each mail (in seconds) is greater than this value,
		// the script will quit, and a wake up time will be stored in the progress file
		$GLOBALS['sleep_threshold'] = 60;
		// when the last_updated field in the progress file did not get updated for this amount of time
		// job is considered as timeout-ed and we will take over this job
		$GLOBALS['timeout_threshold'] = 3600;

		// get a scheduled job from the database table and process it
		$results = $bm->getQueuedJobs();
		//speed_check(); mem_check('getQueuedJobs');

		// pick up a sleeping job (higher priority than a timeout-ed job)
		foreach ($results as $job_id => $info) {
			switch ($info['status']) {
				case BML_JOB_STATE_SLEEPING:
					$progress_info = $bm->getJobProgress($job_id);
					// check timeout and excluded time, make sure we take over this job if timeout_threshold is reached
					if ($this->checkThreshold('time_out', Array(), $progress_info)) {
						if ($this->checkThreshold('excluded_time', $info['details']['threshold'])) {
							$this->chunkJob($job_id, $recip_per_chunk);
							$this->processJob($job_id, $recip_per_chunk);
							return;
						}
					}
				case BML_JOB_STATE_RUNNING:
					$progress_info = $bm->getJobProgress($job_id);
					// check timeout and excluded time, make sure we take over this job if timeout_threshold is reached
					if ($this->checkThreshold('time_out', Array(), $progress_info)) {
						if ($this->checkThreshold('excluded_time', $info['details']['threshold'])) {
							$this->chunkJob($job_id, $recip_per_chunk);
							$this->processJob($job_id, $recip_per_chunk);
							return;
						}
					}
				case BML_JOB_STATE_NOT_RUNNING:
					// excluded time threshold rule
					if ($this->checkThreshold('excluded_time', $info['details']['threshold'])) {
						$bm->updateJob($job_id, BML_JOB_STATE_RUNNING);
						$this->chunkJob($job_id, $recip_per_chunk);
						$this->processJob($job_id, $recip_per_chunk);
						return;
					}
				default:
				break;
			}
		}

		//speed_check(); mem_check('Done checking sq_bulkmail_queue');

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
				return null;
			break;
			case 0:
				return $child_pid;
			break;
			default :
				$status = null;
				pcntl_waitpid(-1, $status);
				return $child_pid;
			break;
		}

	}//end fork()


	/**
	* Cache recipients into chunks using a fork process
	*
	* @param int	$job_id				the unique id of the job entry
	* @param int	$recip_per_chunk	the number of recipients in each chunk
	*
	* @return void
	* @access public
	*/
	function chunkJob($job_id, $recip_per_chunk)
	{
		//speed_check(); mem_check('chunkJob start');
		$bm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');
		$info = $bm->getQueuedJobs($job_id);
		$job_path = $bm->getJobDataPath($job_id);
		$recipient_path = $job_path.'/cache/recipient';
		$progress_path = $job_path.'/progress_info';

		// start caching if not previously cached
		if (!is_dir($recipient_path)) {

			// do this in a fork process, to free up memory
			$pid = $this->fork();
			// is a child process
			if (!$pid) {
				// reconnect because child disconnects DB when it exits
				$this->_reconnectDB();
				$result = $this->getAllRecipients($info[$job_id]['details']['content_id'], $info[$job_id]['details']['recipients']);
				// update total_count in progress info file
				$info[$job_id]['progress']['total_count'] = count($result['recipient']);
				$info[$job_id]['progress']['problematic'] = $result['problematic'];
				$output = '<?php $progress_info = '.var_export($info[$job_id]['progress'], true).'; ?>';
				string_to_file($output, $progress_path);

				// cache recipients into chunks
				create_directory($recipient_path);
				$chunks = array_chunk($result['recipient'], $recip_per_chunk, true);
				foreach ($chunks as $index => $chunk) {
					$output = '<?php $recipient_chunk = '.var_export($chunk, true).'; ?>';
					string_to_file($output, $recipient_path.'/recipient_'.$index);
				}
				exit;
			}
		}
		//speed_check(); mem_check('chunkJob end');

	}//end chunkJob()


	/**
	* Process the active bulkmail job
	*
	* @param int	$job_id				the unique id of the job entry
	* @param int	$recip_per_chunk	number of recipients in each chunk
	*
	* @return boolean
	* @access public
	*/
	function processJob($job_id, $recip_per_chunk)
	{
		//speed_check(); mem_check('processJob start');
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

		// update the cached content before we start
		// fork this process so that we can call printFrontend again
		$pid = $this->fork();
		if (!$pid) {
			$this->cacheContent($details['content_id'], $details['content_gen_as'], $details['content_design'], $details['content_layout'], $job_path);
			exit;
		}

		// read chunks from cache and process them
		//speed_check(); mem_check('start whileloop');
		while (file_exists($recipient_path.'/recipient_'.$progress_info['current_chunk'])) {
			// echo $progress_info['current_chunk'].' ';
			////speed_check(); mem_check('processJob fork chunk '.$progress_info['current_chunk']);

			// check if job state is paused/cancelled at the beginning of each chunk
			$state = $info[$job_id]['status'];
			if ($state == BML_JOB_STATE_PAUSED) {
				// job paused
				exit;
			} else if ($state == BML_JOB_STATE_NOT_RUNNING) {
				// job cancelled
				exit;
			}

			// threshold checking
			if (!$this->checkThreshold('excluded_time', $details['threshold'])) {
				// current time is in excluded time range
				$bm->updateJob($job_id, BML_JOB_STATE_NOT_RUNNING);
				exit;
			}
			if (!$this->checkThreshold('send_rate', $details['threshold'], $progress_path)) {
				// threshold limit has been reached, set job to sleep
				$bm->updateJob($job_id, BML_JOB_STATE_SLEEPING);
				exit;
			}

			// load the current recipient chunk
			include($recipient_path.'/recipient_'.$progress_info['current_chunk']);

			$pid = $this->fork();
			if (!$pid) {
				$mime =& new Mail_mime();
				$mail_queue =& new Mail_Queue($details['queue_details'], $details['server_details']);

				for ($recipient_chunk; null !== ($user_id = key($recipient_chunk)); next($recipient_chunk)) {
					$email = $recipient_chunk[$user_id];
					$this->_reconnectDB();

					// make sure user is not already in mail queue/sent directory
					$id = md5($user_id);
					if (!file_exists($queue_path.'/'.$id) && !file_exists($sent_path.'/'.$id)) {
						// fork since we cannot call printFrontend multiple times
						$pid = $this->fork();
						if (!$pid) {
							$GLOBALS['mail_start'] = get_microtime();
							$content = $this->generateBmailContent($details['content_id'], $details['content_gen'], $details['content_design'], $details['content_layout'], $job_path, $user_id);
							// only queue the mail if we can generate content
							$problematic = (empty($content)) ? true : false;
							if (!$problematic) {
								$mime->setHtmlBody($content);
								//$mime->setTXTBody($content);
								$body = @$mime->get();
								// pass in custom_id as extra header details, for queue file naming
								$header_details['custom_id'] = $user_id;
								$headers = @$mime->headers($header_details);
								$mail_id = $mail_queue->put('', $email, $headers, $body);
							}
							$this->updateProgress($progress_path, $user_id, $problematic);
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

		}//end while()
		//speed_check(); mem_check('end whileloop');

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
		//speed_check(); mem_check('processJob end');
		return true;

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
			$GLOBALS['SQ_SYSTEM']->db->disconnect();
			unset($GLOBALS['SQ_SYSTEM']->_db_conns);
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

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
			return false;
		}

		if ($problematic) {
			$progress_info['total_count']--;
			$progress_info['problematic'][] = $user_id;
		} else {
			$progress_info['current_count']++;
			$time_taken = get_microtime() - $GLOBALS['mail_start'];
			$progress_info['total_time_taken'] += $time_taken;
		}
		$progress_info['current_recip_id'] = $user_id;
		$progress_info['last_updated'] = time();

		$output = '<?php $progress_info = '.var_export($progress_info, true).'; ?>';
		if (!string_to_file($output, $progress_path)) {
			trigger_localised_error('BML0008', E_USER_WARNING, $progress_path);
			return false;
		} else {
			return true;
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
			return false;
		}

		$progress_info['current_chunk']++;
		$output = '<?php $progress_info = '.var_export($progress_info, true).'; ?>';
		if (!string_to_file($output, $progress_path)) {
			trigger_localised_error('BML0008', E_USER_WARNING, $progress_path);
			return false;
		} else {
			return true;
		}

		return $progress_info;

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
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$user_list = Array();
		$group_list = Array();
		$problematic_list = Array();

		if (!is_array($recipients)) {
			$recipients = Array($recipients);
		}

		// an array of userids that have read access to the asset to send (content page)
		// if public user has read access, then everyone should be able to read it
		$read_permission = $am->getPermission($asset_to_send, SQ_PERMISSION_READ, true);
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
					} else if (is_a($recipient, 'user_group') || is_a($recipient, 'web_folder_users')) {
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
							'user'			=> false,
							'bulkmail_user'	=> true,
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


//--        CONTENT        --//


	/**
	* Caches the bulkmail page content as the pre-selected user, returns false if any error occurs
	*
	* @param int	$content_id		the assetid of the content page
	* @param int	$content_gen_as	the assetid of the pre-selected user
	* @param int	$content_design	the assetid of the effective design
	* @param int	$content_layout	the assetid of the effective layout
	* @param string	$job_path		path to the effective job data dir
	*
	* @return boolean
	* @access public
	*/
	function cacheContent($content_id, $content_gen_as, $content_design, $content_layout, $job_path)
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$page =& $am->getAsset($content_id);
		$user =& $am->getAsset($content_gen_as);

		// login as pre-selected user and print page content
		$GLOBALS['SQ_SYSTEM']->setCurrentUser($user);

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

		// cache html content
		if (!is_dir($job_path.'/cache/')) {
			create_directory($job_path.'/cache/');
		}
		if (!string_to_file($content, $job_path.'/cache/content_file.html')) {
			trigger_localised_error('BML0008', E_USER_WARNING, $job_path.'/cache/content_file.html');
			return false;
		} else {
			return true;
		}

	}//end cacheContent()


	/**
	* Generates the html bulkmail content based on the user and content generation type
	*
	* Used in previewing mail and sending actual bmail (use each user or one user)
	* Pre-selected user is used when we cannot login bulkmail_users
	*
	* @param int	$content_id		the assetid of the content page
	* @param string	$content_gen	the content generation type, use each user or use one user
	* @param int	$content_design	the assetid of the effective design
	* @param int	$content_layout	the assetid of the effective layout
	* @param string	$job_path		path to the effective job data dir
	* @param int	$user_id		the recipient of this email, zero is for preview mail
	*
	* @return string
	* @access public
	*/
	function generateBmailContent($content_id, $content_gen, $content_design, $content_layout, $job_path, $user_id=0)
	{
		// note: must run cacheContent() before calling this function
		if (file_exists($job_path.'/cache/content_file.html')) {
			$content = file_to_string($job_path.'/cache/content_file.html');
		}

		// load this user, for content generation and keyword replacement
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$user = null;
		if ($user_id) $user =& $am->getAsset($user_id);

		// always login and generate content for per user basis,
		// unless the user is a subscriber (bulkmail user) with only an email address
		if ($content_gen == 'each_user' && !is_null($user) && !is_a($user, 'Bulkmail_User')) {
			// login as this user and print page content
			if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($user)) {
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
		}

		// keyword replacement
		if ($user_id && !is_null($user)) {
			$content = $this->replaceContentKeyword($content, $user);
		}

		return $content;

	}//end generateBmailContent()


	/**
	* Keyword replacement for customised email content of a user
	*
	* @param string	$content	page content before keyword replacement
	* @param int	$user		personalise content for this user
	*
	* @return string
	* @access public
	*/
	function replaceContentKeyword($content, $user)
	{
		require_once SQ_FUDGE_PATH.'/general/text.inc';

		$name = '';
		if (is_a($user, 'User')) {
			$name = $user->attr('first_name').' '.$user->attr('last_name');
		} else if (is_a($user, 'Bulkmail_User')) {
			$name = $user->attr('name');
			// if use email address if name not found
			if (empty($name)) $name = $user->attr('email');
		}
		$keyword_replacements = Array(
									'bmail_assetid'	=> $user->id,
									'bmail_name'	=> $name,
									'bmail_email'	=> $user->attr('email'),
								);
		$content = replace_keywords($content, $keyword_replacements);

		return $content;

	}//end replaceContentKeyword()


//--        THRESHOLD        --//


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
		$result = false;
		switch ($type) {
			case 'excluded_time' :
				return $this->checkExcludedTime($thresholds);
			break;
			case 'send_rate' :
				return $this->checkSendRate($thresholds, $extra);
			break;
			case 'wakeup_time' :
				return $this->checkWakeUpTime($thresholds, $extra);
			break;
			case 'time_out' :
				return $this->checkTimeOut($thresholds, $extra);
			break;
			default:
				return false;
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
	function checkExcludedTime($thresholds)
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
					if ($now_time >= $threshold['params']['from'] && $now_time < $threshold['params']['to']) {
						return false;
					}
				}
			}
		}
		return true;

	}//end checkExcludedTime()


	/**
	* Check the mail per minute threshold rule
	*
	* @param array	$thresholds		the threshold attribute of the post office
	* @param string	$progress_path	the progress path of the job
	*
	* @return boolean
	* @access public
	*/
	function checkSendRate($thresholds, $progress_path)
	{
		$progress_info = Array();
		if (file_exists($progress_path)) {
			include $progress_path;
		} else {
			trigger_localised_error('BML0009', E_USER_WARNING, $progress_path);
			return false;
		}

		if (!isset($progress_info['threshold_rate'])) {
			foreach ($thresholds as $threshold) {
				if ($threshold['type'] == translate('bulkmail_threshold_rule_send_rate')) {
					$value = $threshold['params']['value'];
					$unit = $threshold['params']['unit'];
					// time interval allocated to each mail, minimum is 1 sec
					$progress_info['threshold_rate'] = ceil($unit/$value);
					break;
				}
			}
			// rate rule not defined
			if (!isset($progress_info['threshold_rate'])) {
				$progress_info['threshold_rate'] = -1;
			}
		}

		// check to see if the rate threshold has been reached
		if ($progress_info['threshold_rate'] != -1) {
			if (isset($progress_info['wake_up_time'])) {
				// just woke up, unset the wake up time field and process the next recipient
				unset($progress_info['wake_up_time']);
				$output = '<?php $progress_info = '.var_export($progress_info, true).'; ?>';
				string_to_file($output, $progress_path);
				return true;
			}
			if ($progress_info['threshold_rate'] > $GLOBALS['sleep_threshold']) {
				// if the time allocated to each mail is greater than the threshold to sleep
				// set a wake up time in the progress file, and pick up this job again after the wake up time
				$progress_info['wake_up_time'] = time() + $progress_info['threshold_rate'];
				$output = '<?php $progress_info = '.var_export($progress_info, true).'; ?>';
				string_to_file($output, $progress_path);
				return false;
			} else {
				// if time allocated for each mail is less than the threshold, sleep
				sleep($progress_info['threshold_rate']-1);
				return true;
			}
		}

		return true;

	}//end checkSendRate()


	/**
	* Check if a current sleeping job can wake up now by checking the wake up time
	*
	* @param array	$thresholds		the threshold attribute of the post office
	* @param array	$progress_info	the current progress info from the progress file
	*
	* @return boolean
	* @access public
	*/
	function checkWakeUpTime($thresholds, $progress_info)
	{
		if (isset($progress_info['wake_up_time']) && (time() > $progress_info['wake_up_time'])) {
			return true;
		}

		return false;

	}//end checkWakeUpTime()


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
		// rule cannot be applied, do not treat a sleeping job as a timeout-ed job
		// otherwise Bulk Mailer will take over and two same jobs will be running simultaneously
		if (isset($progress_info['threshold_rate'])) {
			if ($progress_info['threshold_rate'] > $GLOBALS['timeout_threshold']) {
				return false;
			}
		}

		if (isset($progress_info['last_updated']) && ($progress_info['last_updated'] != 0)) {
			$time_diff = time() - $progress_info['last_updated'];
			// timeout threshold reached, take over
			if ($time_diff > $GLOBALS['timeout_threshold']) {
				return true;
			}
		}
		return false;

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
				while (($real_bt_index < count($bt)-1) && ((false !== strpos(array_get_index($bt[$real_bt_index], 'class'), 'locale_manager')) || (false !== strpos(array_get_index($bt[$real_bt_index], 'file'), 'locale_manager')) || ($bt[$real_bt_index]['function'] == '_errorhandler'))) {
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