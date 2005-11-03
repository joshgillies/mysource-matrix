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
* $Id: bulk_mailer.inc,v 1.10 2005/11/03 05:26:08 rong Exp $
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
* @version $Revision: 1.10 $
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
			trigger_error('Failed to log in as the root user');
			exit(1);
		}

		mem_check(); echo 'Process starts @ '.date('r')."\n";
		$recip_per_chunk = 1;
		$timeout_threshold = 60;

		// get a scheduled job from the database table and process it
		$bm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');
		$results = $bm->getQueuedJobs();
		//echo "getQueuedJobs\n"; bam($results);

		// check for crashed or timeout process (higher priority that new jobs)
		foreach ($results as $job_id => $info) {
			if ($info['status'] == BML_JOB_STATE_RUNNING) {

				$progress_info = $bm->getJobProgress($job_id);
				// not recovering from a pause, check timeout
				if ($progress_info['last_updated'] != 0) {
					$time_diff = time() - $progress_info['last_updated'];
					echo "time diff=$time_diff, threshold=$timeout_threshold\n";
					// timeout threshold reached, take over
					if ($time_diff > $timeout_threshold) {
						echo " BulkMailer->timeout! taking over Job #$job_id\n";
						$this->chunkJob($job_id, $recip_per_chunk);
						$this->processJob($job_id, $recip_per_chunk);
						// exit bulkmailer (dont run the next for loop)
						return;
					} else {
						echo "Timeout threshold ok for Job #$job_id\n";
					}
				}
			}
		}

		// pick up a normal new job/ or a resumed job and run it
		foreach ($results as $job_id => $info) {
			if ($info['status'] == BML_JOB_STATE_NOT_RUNNING) {
				echo " BulkMailer->Start Job #$job_id\n";
				$bm->updateJob($job_id, BML_JOB_STATE_RUNNING);
				$this->chunkJob($job_id, $recip_per_chunk);
				$this->processJob($job_id, $recip_per_chunk);
				break;
			}
		}

		echo "Finished checking db table, exit\n";

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
				trigger_error('Failed to fork');
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
				$result = $this->getAllRecipients($info[$job_id]['details']['recipients']);
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
				echo "chunkJob : Done chunkJob\n";
				exit;
			}

		} else {
			echo "chunkJob : Recipient already cached\n";
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
		$queue_details = $info[$job_id]['details']['queue'];
		$content_details = $info[$job_id]['details']['content'];
		$header_details = $info[$job_id]['details']['header'];
		$server_details = $info[$job_id]['details']['server'];

		// create the /queue and /sent directories for mail queue
		$queue_path = $queue_details['dir'];
		$sent_path = $queue_path.'/sent';
		if (!is_dir($sent_path)) create_directory($sent_path);

		// update the cached content before we start
		// fork this process so that we can call printFrontend again
		$pid = $this->fork();
		if (!$pid) {
			$this->cacheContent($content_details, $job_path);
			exit;
		}

		// read chunks from cache and process them
		while (file_exists($recipient_path.'/recipient_'.$progress_info['current_chunk'])) {
			$this->_reconnectDB();
			// check if job state is paused/cancelled at the beginning of each chunk
			$info = $bm->getQueuedJobs($job_id);
			$state = $info[$job_id]['status'];
			if ($state == BML_JOB_STATE_PAUSED) {
				echo "processJob : job paused\n";
				exit;
			} else if ($state == BML_JOB_STATE_NOT_RUNNING) {
				echo "processJob : job cancelled\n";
				// remove db entry, set under construction, purge data dir, wotever is needed
				$bm->deleteJob($job_id);
				exit;
			}

			// load up current chunk
			$progress_info = $info[$job_id]['progress'];
			include($recipient_path.'/recipient_'.$progress_info['current_chunk']);
			echo "\nfork process - recipient chunk ".$progress_info['current_chunk']."\n";

			$pid = $this->fork();
			if (!$pid) {
				$mime =& new Mail_mime();
				$mail_queue =& new Mail_Queue($queue_details, $server_details);

				for ($recipient_chunk; null !== ($user_id = key($recipient_chunk)); next($recipient_chunk)) {
					$email = $recipient_chunk[$user_id];
					$this->_reconnectDB();

					// make sure user is not already in mail queue/sent directory
					$id = md5($user_id);
					if (!file_exists($queue_path.'/'.$id) && !file_exists($sent_path.'/'.$id)) {
						$to = 'rong@squiz.net'; // TODO: reomove and uncomment next line
						//$to = $email;
						// fork since we cannot call printFrontend multiple times
						$pid = $this->fork();
						if (!$pid) {
							$content = $this->generateBmailContent($content_details, $job_path, $user_id);
							$mime->setHtmlBody($content);
							//$mime->setTXTBody($content);
							$body = @$mime->get();
							// pass in custom_id as extra header details, for queue file naming
							$header_details['custom_id'] = $user_id;
							$headers = @$mime->headers($header_details);
							// note: 'from' field must be defined in the header details
							$mail_id = $mail_queue->put('', $to, $headers, $body);
							$this->updateProgress($progress_path, $user_id);
							exit;
						}

					} else {
						echo " $user_id already in queue/sent (skip, using file_exists)\n";
					}
				}
				echo "processJob : sendMailsInQueue\n";
				$mail_queue->sendMailsInQueue();
				// proceed to next chunk
				$this->nextChunk($progress_path);
				exit;

			} else {
				$this->_reconnectDB();
				$progress_info = $bm->getJobProgress($job_id);
				echo "processJob : Parent $pid exits\n";
			}

		}//end while()

		$this->_reconnectDB();
		$progress_info = $bm->getJobProgress($job_id);
		if ($progress_info['current_count'] == $progress_info['total_count']) {
			echo "  BulkMailer->Finished processing Job #$job_id\n";
			$bm->deleteJob($job_id);
		}

		return true;

	}//end processJob()


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
			trigger_error("updateProgress : Progress info file not found. \"$progress_path\"\n");
			return false;
		}

		$progress_info['current_count']++;
		$progress_info['current_recip_id'] = $user_id;
		$progress_info['last_updated'] = time();

		$output = '<?php $progress_info = '.var_export($progress_info, true).'; ?>';
		if (!string_to_file($output, $progress_path)) {
			trigger_error("updateProgress : Unable to write to progress file. \"$progress_path\"\n");
			return false;
		} else {
			echo 'updateProgress : #'.$progress_info['current_count']." User (Id #$user_id)\n";
			sleep(10); // TODO: remove this
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
			trigger_error("nextChunk : Progress info file not found. \"$progress_path\"\n");
			return false;
		}

		$progress_info['current_chunk']++;
		$output = '<?php $progress_info = '.var_export($progress_info, true).'; ?>';
		if (!string_to_file($output, $progress_path)) {
			trigger_error("nextChunk : Unable to write to progress file. \"$progress_path\"\n");
			return false;
		} else {
			echo 'nextChunk : current_chunk = '.$progress_info['current_chunk']."\n";
			return true;
		}

		return $progress_info;

	}//end nextChunk()


	/**
	* Cache all recipients (user_id => email) for a job
	*
	* Warning: $result array can be huge
	* Expands user_group to get all user/bulkmail_user
	* Skips those recipients that is in .queue or .sent directory
	*
	* @param array	$recipients	the job's 'recipients' attribute stripped from vars
	*
	* @return array
	* @access public
	*/
	function getAllRecipients($recipients)
	{
		// handle groups and users/bulkmail_users separately
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$user_list = Array();
		$group_ids = Array();

		foreach ($recipients as $recipient_id) {
			$recipient =& $am->getAsset($recipient_id);
			// store id and email of all users/bulkmail_users
			if (!is_null($recipient)) {
				if (is_a($recipient, 'user') || is_a($recipient, 'bulkmail_user')) {
					$user_list += Array($recipient_id => $recipient->attr('email'));
				}
			// store all group ids
			} else if (!is_null($recipient) && is_a($recipient, 'user_group')) {
				$group_ids[] = $recipient_id;
			}
			$am->forgetAsset($recipient);
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
		/*foreach($result as $user_id => $email) {
			$count = 0;
			while($count<100){
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


	/**
	* Caches the bulkmail page content as the pre-selected user, returns false if any error occurs
	*
	* @param array	$content_details	information related to content generation
	* @param string	$job_path			path to the effective job data dir
	*
	* @return boolean
	* @access public
	*/
	function cacheContent($content_details, $job_path)
	{
		// note: must run isValidJob() before calling this function
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$page =& $am->getAsset($content_details['content_page']);
		$user =& $am->getAsset($content_details['pre_selected_user']);

		// login as pre-selected user and print page content
		$GLOBALS['SQ_SYSTEM']->setCurrentUser($user);

		ob_start();
			// dont want any notice/warning here
			// TODO: add design/layout handling
			@$page->printFrontend();
			$content = ob_get_contents();
		ob_end_clean();

		// cache html content
		if (!string_to_file($content, $job_path.'/cache/content_file.html')) {
			trigger_error("cacheContent : Unable to write to cache file. \"$job_path/cache/content_file.html\"\n");
			return false;
		} else {
			echo "cacheContent: Cached\n";
			return true;
		}

	}//end cacheContent()


	/**
	* Generates the html bulkmail content based on the user and content generation type
	*
	* Used in previewing mail and sending actual bmails (custom content or per user)
	* Pre-selected user is used when we cannot login bulkmail_users
	*
	* @param array	$content_details	information related to content generation
	* @param string	$job_path			path to the effective job data dir
	* @param int	$user_id			the recipient of this email, zero is for preview mail
	*
	* @return string
	* @access public
	*/
	function generateBmailContent($content_details, $job_path, $user_id=0)
	{

		$am = $GLOBALS['SQ_SYSTEM']->am;
		$type = array_get_index($content_details, 'type', 'each_user');
		$user = null;

		// load this user, for content generation and keyword replacement
		if ($user_id) $user =& $am->getAsset($user_id);

		// always login and generate content for per user basis,
		// unless the user is a subscriber (bulkmail user) with only an email address
		if ($type == 'each_user' && !is_null($user) && !is_a($user, 'Bulkmail_User')) {
			// login as this user and print page content
			if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($user)) {
				// happens when the user is not live or in trash
				trigger_error("generateBmailContent : cannot login User $user->name (Id # $user->id)\n");
				return "generateBmailContent: cannot login User $user->name (Id # $user->id)\n";
			}
			$page =& $am->getAsset($content_details['content_page']);

			ob_start();
				// dont want any notice/warning here
				// TODO: add design/layout handling
				@$page->printFrontend();
				$content = ob_get_contents();
			ob_end_clean();
		}

		// if we cannot generate content, load the cached content,
		// note: must run cacheContent() before calling this function
		if (empty($content)) {
			if (file_exists($job_path.'/cache/content_file.html')) {
				$content = file_to_string($job_path.'/cache/content_file.html');
			} else {
				trigger_error("generateBmailContent : Unable to read from cache file. \"$job_path/cache/content_file.html\"\n");
				return "generateBmailContent: Unable to read from cache file.\n";
			}
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


}//end class


?>