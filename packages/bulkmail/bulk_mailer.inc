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
* $Id: bulk_mailer.inc,v 1.22 2005/11/17 22:46:08 rong Exp $
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
* @version $Revision: 1.22 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class Bulk_Mailer
{


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


	/**
	* Signal handler for the pcntl fork process
	*
	* @param int	$signo	the signals that are supported by the PHP Process Control functions
	*
	* @return void
	* @access public
	*/
	function sigHandler($signo)
	{
		switch ($signo) {
			case SIGTERM:
				// handle shutdown tasks
				exit;
			break;
			case SIGHUP:
				// handle restart tasks
			break;
			default:
				// handle all other signals
		}
	}//end sigHandler()


		// setup signal handlers
		pcntl_signal(SIGTERM, 'sigHandler');
		pcntl_signal(SIGHUP, 'sigHandler');

		// login as root user
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$root_user =& $am->getSystemAsset('root_user');
		if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
			trigger_localised_error('BML0003', E_USER_WARNING, $root_user->name, $root_user->id);
			exit(1);
		}

		//mem_check(); //echo 'Process starts @ '.date('r')."\n";
		$recip_per_chunk = 1;
		$timeout_threshold = 60;

		// get a scheduled job from the database table and process it
		$bm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');
		$results = $bm->getQueuedJobs();

		// pick up a sleeping job (higher priority than a timeout job)
		foreach ($results as $job_id => $info) {
			if ($info['status'] == BML_JOB_STATE_SLEEPING) {
				$progress_info = $bm->getJobProgress($job_id);
				//echo "current time ".time()." vs wakeuptime $progress_info[wake_up_time], time diff = ".($progress_info['wake_up_time']-time())."\n";
				if (time() > $progress_info['wake_up_time']) {
					//echo "wake up!\n";
					// excluded time threshold rule
					if ($this->checkThreshold('excluded_time', $info['details']['threshold'])) {
						//echo " BulkMailer->Start Job #$job_id\n";
						$bm->updateJob($job_id, BML_JOB_STATE_RUNNING);
						$this->chunkJob($job_id, $recip_per_chunk);
						$this->processJob($job_id, $recip_per_chunk);
						return;
					}
				}
			}
		}

		// check for crashed or timeout process (higher priority than a new job)
		foreach ($results as $job_id => $info) {
			if ($info['status'] == BML_JOB_STATE_RUNNING) {

				$progress_info = $bm->getJobProgress($job_id);
				// not recovering from a pause, check timeout
				if ($progress_info['last_updated'] != 0) {
					$time_diff = time() - $progress_info['last_updated'];
					//echo "time diff=$time_diff, threshold=$timeout_threshold\n";
					// timeout threshold reached, take over
					if ($time_diff > $timeout_threshold) {
						if ($this->checkThreshold('excluded_time', $info['details']['threshold'])) {
							//echo " BulkMailer->timeout! taking over Job #$job_id\n";
							$this->chunkJob($job_id, $recip_per_chunk);
							$this->processJob($job_id, $recip_per_chunk);
							// exit bulkmailer (dont run the next for loop)
							return;
						}
					} else {
						//echo "Timeout threshold ok for Job #$job_id\n";
					}
				}
			}
		}

		// pick up a normal new job/ or a resumed job and run it (lowest priority)
		foreach ($results as $job_id => $info) {
			if ($info['status'] == BML_JOB_STATE_NOT_RUNNING) {
				// excluded time threshold rule
				if ($this->checkThreshold('excluded_time', $info['details']['threshold'])) {
					//echo " BulkMailer->Start Job #$job_id\n";
					$bm->updateJob($job_id, BML_JOB_STATE_RUNNING);
					$this->chunkJob($job_id, $recip_per_chunk);
					$this->processJob($job_id, $recip_per_chunk);
					exit;
				}
			}
		}

		//echo "Finished checking db table, exit\n";

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
				$info[$job_id]['progress']['total_count'] = count($result);
				$output = '<?php $progress_info = '.var_export($info[$job_id]['progress'], true).'; ?>';
				string_to_file($output, $progress_path);

				// cache recipients into chunks
				create_directory($recipient_path);
				$chunks = array_chunk($result, $recip_per_chunk, true);
				foreach ($chunks as $index => $chunk) {
					$output = '<?php $recipient_chunk = '.var_export($chunk, true).'; ?>';
					string_to_file($output, $recipient_path.'/recipient_'.$index);
				}
				//echo "chunkJob : Done chunkJob\n";
				exit;
			}

		} else {
			//echo "chunkJob : Recipient already cached\n";
		}

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
		while (file_exists($recipient_path.'/recipient_'.$progress_info['current_chunk'])) {
			$this->_reconnectDB();
			// check if job state is paused/cancelled at the beginning of each chunk
			$info = $bm->getQueuedJobs($job_id);
			$state = $info[$job_id]['status'];
			if ($state == BML_JOB_STATE_PAUSED) {
				//echo "processJob : job paused\n";
				exit;
			} else if ($state == BML_JOB_STATE_NOT_RUNNING) {
				//echo "processJob : job cancelled\n";
				$bm->deleteJob($job_id);
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
			$progress_info = $info[$job_id]['progress'];
			include($recipient_path.'/recipient_'.$progress_info['current_chunk']);
			//echo "\nfork process - recipient chunk ".$progress_info['current_chunk']."\n";

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
							$content = $this->generateBmailContent($details['content_id'], $details['content_gen'], $details['content_design'], $details['content_layout'], $job_path, $user_id);
							// only queue the mail if we can generate content
							if (!empty($content)) {
								$mime->setHtmlBody($content);
								//$mime->setTXTBody($content);
								$body = @$mime->get();
								// pass in custom_id as extra header details, for queue file naming
								$header_details['custom_id'] = $user_id;
								$headers = @$mime->headers($header_details);
								$mail_id = $mail_queue->put('', $email, $headers, $body);
							}
							$this->updateProgress($progress_path, $user_id);
							sleep(3);
							exit;
						}

					} else {
						//echo " $user_id already in queue/sent (skip, using file_exists)\n";
					}
				}
				//echo "processJob : sendMailsInQueue\n";
				$status = $mail_queue->sendMailsInQueue();
				if (PEAR::isError($status)) {
					trigger_localised_error('BML0001', E_USER_WARNING, $status->getMessage());
					exit;
				}
				// proceed to next chunk
				$this->nextChunk($progress_path);
				exit;

			} else {
				$this->_reconnectDB();
				$progress_info = $bm->getJobProgress($job_id);
				//echo "processJob : Parent $pid exits\n";
			}

		}//end while()

		$this->_reconnectDB();
		$progress_info = $bm->getJobProgress($job_id);
		if ($progress_info['current_count'] >= $progress_info['total_count']) {
			//echo "  BulkMailer->Finished processing Job #$job_id\n";
			$bm->deleteJob($job_id);
		}

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
			// need to empty conns to reconnect
			unset($GLOBALS['SQ_SYSTEM']->_db_conns);
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

	}//end _reconnectDB()


	/**
	* Writes the progress info to a file after a mail has been added to the queue
	*
	* @param string	$progress_path	path to the progress info file
	* @param int	$user_id		current recipient assetid
	*
	* @return boolean
	* @access public
	*/
	function updateProgress($progress_path, $user_id)
	{
		$progress_info = Array();
		if (file_exists($progress_path)) {
			include $progress_path;
		} else {
			trigger_localised_error('BML0009', E_USER_WARNING, $progress_path);
			return false;
		}

		$progress_info['current_count']++;
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
			//echo 'nextChunk : current_chunk = '.$progress_info['current_chunk']."\n";
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
		$group_ids = Array();

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
					} else if (is_a($recipient, 'user_group')) {
						$group_ids[] = $recipient_id;
					}
				}
				$am->forgetAsset($recipient);
			} else {
				// PROBLEMATIC USER/GROUP: no read access to content
			}
		}

		// expand groups and get all users/bulkmail_users from a query
		$result = Array();
		if (!empty($group_ids)) {
			$type_codes = Array(
							'user'			=> false,
							'bulkmail_user'	=> true,
						  );
			$query = $this->generateGetRecipientsQuery($group_ids, $type_codes);
			$result = $GLOBALS['SQ_SYSTEM']->db->getAssoc($query);
			assert_valid_db_result($result);
		}

		// combine group/user results
		$result += $user_list;

		// code for multiplying the recipients, eg. turn 10 actual users into 500 users
		// for testing purposes
		/*foreach ($result as $user_id => $email) {
			$count = 0;
			while ($count<5) {
				$result += Array( ($user_id * $count) => $email.'.'.$count.'.net');
				$count++;
			}
		}*/

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
			////echo "cacheContent: Cached\n";
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
		//if ($user_id) $user =& $am->getAsset($user_id);

		// always login and generate content for per user basis,
		// unless the user is a subscriber (bulkmail user) with only an email address
		if ($content_gen == 'each_user' && !is_null($user) && !is_a($user, 'Bulkmail_User')) {
			// login as this user and print page content
			if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($user)) {
				// PROBLEMATIC USER: in trash or not live
				// trigger_localised_error('BML0003', E_USER_WARNING, $user->name, $user->id);
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
		if ($type == 'excluded_time') {
			return $this->checkExcludedTime($thresholds);
		} else if ($type == 'send_rate') {
			return $this->checkSendRate($thresholds, $extra);
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
						//echo "current time is in excluded time\n";
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
		// if the time allocated for each mail (in seconds) is greater than this value,
		// the script will quit, and a wake up time will be stored in the progress file
		// otherwise, the script will sleep for allocated time before it runs again
		// (for spreading the job across a certain amount of time, and balancing the server load)
		$threshold_to_sleep = 60;

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
					//echo "send rate (time allocated) is $progress_info[threshold_rate] sec per mail\n";
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
			if ($progress_info['threshold_rate'] > $threshold_to_sleep) {
				// if the time allocated to each mail is greater than the threshold to sleep
				// set a wake up time in the progress file, and pick up this job again after the wake up time
				$progress_info['wake_up_time'] = time() + $progress_info['threshold_rate'];
				$output = '<?php $progress_info = '.var_export($progress_info, true).'; ?>';
				string_to_file($output, $progress_path);
				//echo "stored wake up time $progress_info[wake_up_time]\n";
				return false;
			} else {
				// if time allocated for each mail is 45 sec, sleep for 44 sec and process one mail
				//echo "sleep $progress_info[threshold_rate]-1 sec\n";
				sleep($progress_info['threshold_rate']-1);
				return true;
			}
		}

	}//end checkSendRate()


}//end class


?>