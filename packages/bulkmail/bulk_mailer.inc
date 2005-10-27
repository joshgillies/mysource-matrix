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
* $Id: bulk_mailer.inc,v 1.3 2005/10/27 01:38:15 ndvries Exp $
*
*/

require dirname(__FILE__).'/../../core/include/init.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';
require_once  'Mail/Queue.php';

// global job asset
$job = null;

/**
* Bulk_Mailer
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Rayn Ong <rong@squiz.net>
*
* @version $Revision: 1.3 $
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


	function start()
	{
		declare(ticks=1);
		function sig_handler($signo)
		{
			global $job;
			switch ($signo) {
				case SIGTERM:
					// handle shutdown tasks
					$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
					$job->setAttrValue('last_processed', 0);
					$job->saveAttributes();
					$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
					exit;
					break;
				case SIGHUP:
					// handle restart tasks
					$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
					$job->setAttrValue('last_processed', 0);
					$job->saveAttributes();
					$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
					break;
				default:
					// handle all other signals
			}
		}
		// setup signal handlers
		pcntl_signal(SIGTERM, "sig_handler");
		pcntl_signal(SIGHUP, "sig_handler");

		global $job;
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$root_user =& $am->getSystemAsset('root_user');
		if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
			trigger_error('Failed to log in as the root user');
			exit(1);
		}

		//mem_check();
		echo "  start ".date('r')."\n";
		$chunk_count = 4;
		$timeout_threshold = 5;

		require_once SQ_PACKAGES_PATH.'/bulkmail/bulkmail_job/bulkmail_job.inc';
		$job_id = Bulkmail_Job::getLiveJob();
		// no bulkmail job is scheduled to run, exit
		if (!$job_id) {
			echo "no current active bulkmail job\n";
			exit;
		}
		echo "getLiveJob returns Id#$job_id\n";

		// job is paused, exit
		$job = $am->getAsset($job_id); // cannot use & (global?)
		$state = $job->getAttrDB('current_state');
		if ($state == BML_JOB_STATE_PAUSED) exit;

		// job is running, check for timeout threshold
		if ($state == BML_JOB_STATE_RUNNING) {
			$last_processed = $job->getAttrDB('last_processed');
			if($last_processed != 0) {
				$time_diff = time() - $last_processed;
				echo " last processed $last_processed\ntime difference $time_diff\n";

				// if timeout threshold not reached, we are safe, exit
				if($time_diff < $timeout_threshold) {
					exit;

				// if timeout, kill old process and take over
				} else {
					echo "timeout!\n";
					$parent_pid = $job->getAttrDB('parent_process_pid');
					$child_pids = unserialize($job->getAttrDB('child_process_pids'));
					if(!$this->_killOldProcess($parent_pid, $child_pids)) {
						// probably exited abnormally, take over anyway
						echo "process does not exist\n";
					}
					echo "took over\n";
					$this->_setParentPid();
					$this->chunkJob($chunk_count);
					$this->processJob($chunk_count);
				}

			// if timestamp is not set+, we are starting a new process
			} else {
				// store the parent process id and timestamp
				$this->_setParentPid();
				mem_check();echo "  - getAsset(\$job_id)\n";
				$this->chunkJob($chunk_count);
				mem_check();echo "  - chunkJob\n";
				$this->processJob($chunk_count);
				mem_check();echo "  - processJob\n";
			}
		}

	}// end start()


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

	}// end fork()


	/**
	* Cache recipients into chunks using a fork process
	*
	* @param object	&$job			the bulkmail job asset that we are processing
	* @param int	$chunk_count	number of recipients in each chunk
	*
	* @return void
	* @access public
	*/
	function chunkJob($chunk_count)
	{
		global $job;
		$recipient_path = $job->data_path.'/.data/cache/recipient';
		$info_path = $job->data_path.'/.data/info';

		// skip if cached
		if(!file_exists($recipient_path.'/marker')) {

			// chunkRecipients will load up a big array
			// fork and exit to free up memory
			$pid = $this->fork();

			// is a child process
			if (!$pid) {

				// child disconnects DB when it exits, so reconnect here
				$this->_reconnectDB();

				// create the directory for chunk cache
				$recipient_path = $job->data_path.'/.data/cache/recipient';
				if (!create_directory($recipient_path)) {
					create_directory($recipient_path);
				}

				$result = $job->getRecipients();
				// initialise progress information
				$total_recipient = count($result);
				$total_chunk = ceil($total_recipient / $chunk_count);
				$info = Array(
							'total_recipient'	=> $total_recipient,
							'total_sent'		=> 0,
							'current_user'		=> 0,
							'paused'			=> false,
						);
				$output = '<?php $info = '.var_export($info, true).'; ?>';
				string_to_file($output, $info_path);
				$output = '<?php $marker = 0; ?>';
				string_to_file($output, $recipient_path.'/marker');

				// cache recipients into chunks
				$chunks = array_chunk($result, $chunk_count, true);
				foreach ($chunks as $marker => $chunk) {
					$output = '<?php $recipient_chunk = '.var_export($chunk, true).'; ?>';
					string_to_file($output, $recipient_path.'/recipient_'.$marker);
				}

				exit;
			}
		} else {
			echo "recipient already cached\n";
		}

	}//end chunkJob()


	/**
	* Process the active bulkmail job
	*
	* Also handles the pause/resume/skip processed recipients
	*
	* @param object	&$job			the bulkmail job asset that we are processing
	* @param int	$chunk_count	number of recipients in each chunk
	*
	* @return boolean
	* @access public
	*/
	function processJob($chunk_count)
	{
		global $job;
		$this->_reconnectDB();
		//echo "Processing ".$job->name." (Id #".$job->id.") in chunk of $chunk_count, starting with PID ".posix_getpid(), "\n";

		// information required to initialise mail queue
		$queue_options = $job->getQueueOptions();
		$mail_options = $job->getServerDetails();
		$queue_path = $queue_options['dir'];
		if (!create_directory($queue_path)) {
			create_directory($queue_path);
		}
		if (!create_directory($queue_path.'/sent')) {
			create_directory($queue_path.'/sent');
		}

		// other mail details
		$content_details = $job->attr('content_details');
		$content_details['content_page'] = $job->attr('content_assetid');
		$hdrs = $job->getHeaderDetails();
		$mime = new Mail_mime();
		$hdrs = $mime->headers($hdrs);

		// start or resume, load and process the correct chunk
		$marker = 0;
		$recipient_path = $job->data_path.'/.data/cache/recipient';
		$info_path = $job->data_path.'/.data/info';
		if(file_exists($recipient_path.'/marker')) include $recipient_path.'/marker';

		// update the cached content each time we run the script
		// do this in a fork process, so that we cant call printFrontend again in the child process
		$pid = $this->fork();
		if (!$pid) {
			$job->cacheContent($job, $content_details);
			echo "cached content!\n";
			exit;
		} else {
			echo "Parent $pid exits, after cache content\n";
		}

		// read chunks from cache and process them
		while(file_exists($recipient_path.'/recipient_'.$marker)) {

			$this->_reconnectDB();
			// if not live and running, exit (paused/cancelled)
			$status = Bulkmail_Job::getStatusDB($job->id);
			$state = $job->getAttrDB('current_state');
			if($status != SQ_STATUS_LIVE || $state != BML_JOB_STATE_RUNNING) {
				echo "NOT LIVE OR RUNNING, EXIT\n";
				exit;
			}

			// load up current chunk
			include($recipient_path.'/recipient_'.$marker);
			echo "\nfork process - recipient chunk $marker\n";
			bam($recipient_chunk);

			// resuming from a paused job, skip all processed recipients
			if (file_exists($info_path)) {
				include $info_path;
				if($info['paused']) {
					// load previous chunk of recipients
					$marker--;
					$recipient_chunk = $this->_resumeJob($info, $marker, $info_path, $recipient_path);
				}
			}

			$pid = $this->fork();
			if (!$pid) {
				// store the child process id and timestamp for each chunk
				$this->_setChildPid();
				// update cache each time
				$mail_queue =& new Mail_Queue($queue_options, $mail_options);

				for ($recipient_chunk; null !== ($user_id = key($recipient_chunk)); next($recipient_chunk)) {
					$email = $recipient_chunk[$user_id];

					if(!$job->isUserInQueue($user_id) && !$job->isUserInSent($user_id)) {

						// for testing purposes, set this to my email address
						// use $email instead of $to
						$to = 'rong@squiz.net';
						// pass in custom_id as extra header details, for queue file naming
						$hdrs['custom_id'] = $user_id;

						// fork since we cannot call printFrontend multiple times
						$pid = $this->fork();
						if (!$pid) {
							$content = $job->generateBmailContent($job, $content_details, $user_id);
							$mime->setTXTBody($content);
							$body = @$mime->get();
							// note: 'from' field must be defined in the header details
							$mail_id = $mail_queue->put('', $to, $hdrs, $body);
							exit;
						}

						$this->_reconnectDB();
						// update progress counter
						if (file_exists($info_path)) include $info_path;
						$info['total_sent']++;
						$info['current_user'] = $user_id;
						$output = '<?php $info = '.var_export($info, true).'; ?>';
						string_to_file($output, $info_path);
						echo " processed ".$info['total_sent']." (Id #$user_id) $email  ... sleep(5)\n"; //sleep(5);

						// pause the job if status is not live, or current state is paused
						$status = Bulkmail_Job::getStatusDB($job->id);
						$state = $job->getAttrDB('current_state');
						if($status != SQ_STATUS_LIVE || $state == BML_JOB_STATE_PAUSED) {
							$this->_pauseJob($info, $info_path);
							break;
						}
					} else {
						echo " $user_id already in queue/sent (skip, using file_exists)\n";
					}
//sleep(1);
				}
				// proceed to next chunk in the next loop
				$marker++;
				$output = "<?php \$marker = $marker; ?>";
				string_to_file($output, $recipient_path.'/marker');
				echo " sendMailsInQueue\n";
				$mail_queue->sendMailsInQueue();
				exit;

			} else {
				// load the updated marker
				if(file_exists($recipient_path.'/marker')) include $recipient_path.'/marker';
				echo "Parent $pid exits\n";
			}

		}//end while()

		// mark this job as finished, and set status to under construction
		$this->_reconnectDB();
		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
		$job->setAttrValue('current_state', BML_JOB_STATE_FINISHED);
		$job->processStatusChange(SQ_STATUS_UNDER_CONSTRUCTION, false);
		$job->setAttrValue('parent_process_pid', 0);
		$job->setAttrValue('child_process_pids', Array());
		$job->setAttrValue('last_processed', 0);
		$job->saveAttributes();
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

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
	* Pause job
	*
	* @param array	$info			the progress information
	* @param string	$info_path		the file path to the progress info
	*
	* @return void
	* @access private
	*/
	function _pauseJob($info, $info_path)
	{
		echo "Paused at user ".$info['current_user'].", child exit\n";
		// set the pause flag
		$info['paused'] = true;
		$output = '<?php $info = '.var_export($info, true).'; ?>';
		string_to_file($output, $info_path);

		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
		$job->setAttrValue('last_processed', 0);
		$job->saveAttributes();
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

	}//end _pauseJob()


	/**
	* Resume job
	*
	* Returns the current chunk pointing to the current user or false on error
	*
	* @param array	$info			the progress information
	* @param int	$marker			current chunk we are up to
	* @param string	$info_path		the file path to the progress info
	* @param string	$recipient_path	the file path to the recipient cache
	*
	* @return mixed boolean|array
	* @access private
	*/
	function _resumeJob($info, $marker, $info_path, $recipient_path)
	{
		echo "Resuming Job at chunk $marker - user ".$info['current_user']."\n";

		// unset the pause flag
		$info['paused'] = false;
		$output = '<?php $info = '.var_export($info, true).'; ?>';
		string_to_file($output, $info_path);

		// load the chunk we were up to
		include($recipient_path.'/recipient_'.$marker);

		// move the array internal pointer to where we paused
		while (key($recipient_chunk) != $info['current_user']) {
			echo " ".key($recipient_chunk)." already processed (skip, using array next) ... sleep(3)\n";
			next($recipient_chunk);
			//sleep(3);
		}

		return $recipient_chunk;

	}//end _resumeJob()


	/**
	* Sets the parent process pid and the last processed timestamp
	*
	* @return void
	* @access private
	*/
	function _setParentPid()
	{
		global $job;
		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
		$job->setAttrValue('parent_process_pid', posix_getpid());
		$job->setAttrValue('child_process_pids', Array());
		$job->setAttrValue('last_processed', time());
		$job->saveAttributes();
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

	}//end _setParentPid()


	/**
	* Adds the child process pid to the array and sets the last processed timestamp
	*
	* @return void
	* @access private
	*/
	function _setChildPid()
	{
		global $job;
		// keep maximum of 5 child pids
		$max_child = 5;
		$array = $job->getAttrDB('child_process_pids');
		if(!empty($array)) $array = unserialize($array);
		$array[] = posix_getpid();
		if(count($array) > $max_child) array_shift($array);

		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
		$job->setAttrValue('child_process_pids', $array);
		$job->setAttrValue('last_processed', time());
		$job->saveAttributes();
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

	}//end _setChildPid()


	/**
	* Kills the old process (parent and child)
	*
	* @param int	$parent_pid	parent process id
	* @param array	$child_pids	an array of child process ids
	*
	* @return boolean
	* @access private
	*/
	function _killOldProcess($parent_pid, $child_pids)
	{
		$error = false;

		if(!posix_kill($parent_pid, SIGTERM)) {
			echo "error: unable to kill parent process (pid=$parent_pid)\n";
			$error = true;
		} else{
			echo "killed parent process (pid=$parent_pid)\n";
		}

		$child_pid = array_pop($child_pids);
		if(!posix_kill($child_pid, SIGTERM)) {
			echo "error: unable to kill child process (pid=$child_pid)\n";
			$error = true;
		} else{
			echo "killed child process (pid=$child_pid)\n";
		}

		return ($error) ? false : true;

	}//end _killOldProcess()


}//end class


?>