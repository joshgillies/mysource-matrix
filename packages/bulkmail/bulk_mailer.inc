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
* $Id: bulk_mailer.inc,v 1.8 2005/11/03 00:52:02 ndvries Exp $
*
*/

require dirname(__FILE__).'/../../core/include/init.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';
require_once  'Mail/Queue.php';

/**
* Bulk_Mailer
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Rayn Ong <rong@squiz.net>
*
* @version $Revision: 1.8 $
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
	* Create a prepare statement and return the results by running it with different values
	*
	* @param string	$table_name		perform query on this table
	* @param array	$table_fields	the relevant table fields for this query
	* @param array	$values			run this query with different values
	*
	* @return array
	* @access public
	*/
	function selectFields($table_name, $table_fields, $values)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$fields = implode(', ', $table_fields);
		$prepare_statement = $db->prepare("select $fields from $table_name where assetid = ?;");
		if (PEAR::isError($prepare_statement)) {
			echo $prepare_statement->getMessage();
			return Array();
		}

		$result_sets = Array();
		foreach ($values as $val) {
			// executeMultiple() returns an integer instead of all results
			$result =& $db->execute($prepare_statement, $val);
			if (PEAR::isError($result)) {
				echo $result->getMessage();
				return Array();
			}
			$result->fetchInto($row);
			$result_sets[] = $row;
		}

		return $result_sets;

	}//end selectFields()


	/**
	* Starts the process, gets called from run.php
	*
	* @return void
	* @access public
	*/
	function start()
	{
		/*
		$table_name = 'sq_ast';
		$table_fields = array('assetid', 'name', 'type_code');
		$values = array('1', '2', '12', '15');
		$results = $this->selectFields($table_name, $table_fields, $values);
		bam($results);
		die;
		*/
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

		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$root_user =& $am->getSystemAsset('root_user');
		if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
			trigger_error('Failed to log in as the root user');
			exit(1);
		}

		mem_check(); echo 'Process starts @ '.date('r')."\n";
		$chunk_count = 2;
		$timeout_threshold = 60;

		require_once SQ_PACKAGES_PATH.'/bulkmail/bulkmail_job/bulkmail_job.inc';
		$job_id = Bulkmail_Job::getLiveJob();

		// no bulkmail job is scheduled to run, exit
		if (!$job_id) {
			echo "no current active bulkmail job\n";
			exit;
		}
		echo "getLiveJob returns Id#$job_id\n";
		$GLOBALS['BULKMAIL_JOB'] =& $am->getAsset($job_id);
		$progress_info = Array();
		$progress_path = $GLOBALS['BULKMAIL_JOB']->getProgressPath();
		mem_check();echo "  - getAsset(\$job_id)\n";

		// if not live and running, exit (paused/cancelled)
		$status = Bulkmail_Job::getStatusDB($GLOBALS['BULKMAIL_JOB']->id);
		$state = $GLOBALS['BULKMAIL_JOB']->getAttrDB('current_state');
		if ($status != SQ_STATUS_LIVE || $state != BML_JOB_STATE_RUNNING) {
			echo " job is not live/running, exit\n";
			exit;
		}

		if ($state == BML_JOB_STATE_RUNNING) {

			$progress_info = $GLOBALS['BULKMAIL_JOB']->_readProgressFile($progress_path);

			// if timestamp is set, check for timeout threshold
			if (!empty($progress_info)) {
				if ($progress_info['last_updated'] != 0) {
					$time_diff = time() - $progress_info['last_updated'];
					echo " last updated ".$progress_info['last_updated'].".\n time difference $time_diff, threshold=$timeout_threshold\n";
					// timeout threshold not reached, we are safe, exit
					if ( $time_diff < $timeout_threshold) {
						exit;
					// if timeout, take over
					} else {
						echo " timeout! taking over processJob()\n";
						$this->chunkJob($chunk_count); //extra check in case recipients not yet cached
						$this->processJob($chunk_count);
					}
				}

			// if timestamp is not set, we are starting a new process
			} else {
				$this->chunkJob($chunk_count);
				mem_check();echo "  - chunkJob\n";
				$this->processJob($chunk_count);
				mem_check();echo "  - processJob\n";
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
	* @param int	$chunk_count	number of recipients in each chunk
	*
	* @return void
	* @access public
	*/
	function chunkJob($chunk_count)
	{
		$recipient_path = $GLOBALS['BULKMAIL_JOB']->data_path.'/.data/cache/recipient';
		$progress_path = $GLOBALS['BULKMAIL_JOB']->getProgressPath();

		// start caching if not previously cached
		if (!file_exists($progress_path)) {

			// do this in a fork process to free up memory
			$pid = $this->fork();

			// is a child process
			if (!$pid) {

				// child disconnects DB when it exits, so reconnect here
				$this->_reconnectDB();

				// create the cache directory for recipient chunks
				if (!create_directory($recipient_path)) {
					create_directory($recipient_path);
				}

				// initialise progress information with total number of recipients
				$result = $GLOBALS['BULKMAIL_JOB']->getRecipients();
				$total_count = count($result);
				$GLOBALS['BULKMAIL_JOB']->_initProgressFile($progress_path, $total_count);

				// cache recipients into chunks
				$chunks = array_chunk($result, $chunk_count, true);
				foreach ($chunks as $index => $chunk) {
					$output = '<?php $recipient_chunk = '.var_export($chunk, true).'; ?>';
					string_to_file($output, $recipient_path.'/recipient_'.$index);
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
	* @param int	$chunk_count	number of recipients in each chunk
	*
	* @return boolean
	* @access public
	*/
	function processJob($chunk_count)
	{
		$this->_reconnectDB();
		$progress_path = $GLOBALS['BULKMAIL_JOB']->getProgressPath();
		//echo "Processing ".$GLOBALS['BULKMAIL_JOB']->name." (Id #".$GLOBALS['BULKMAIL_JOB']->id.") in chunk of $chunk_count, starting with PID ".posix_getpid(), "\n";

		// information required to initialise mail queue
		$queue_options = $GLOBALS['BULKMAIL_JOB']->getQueueOptions();
		$mail_options = $GLOBALS['BULKMAIL_JOB']->getServerDetails();
		$queue_path = $queue_options['dir'];
		if (!create_directory($queue_path)) {
			create_directory($queue_path);
		}
		if (!create_directory($queue_path.'/sent')) {
			create_directory($queue_path.'/sent');
		}

		// other mail details
		$content_details = $GLOBALS['BULKMAIL_JOB']->attr('content_details');
		$header_details = $GLOBALS['BULKMAIL_JOB']->getHeaderDetails();
		$mime =& new Mail_mime();

		// start or resume, load and process the correct chunk
		//$marker = 0;
		$recipient_path = $GLOBALS['BULKMAIL_JOB']->data_path.'/.data/cache/recipient';
		//if (file_exists($recipient_path.'/marker')) {
		//	include $recipient_path.'/marker';
		//}

		$progress_info = $GLOBALS['BULKMAIL_JOB']->_readProgressFile($progress_path);

		// re-cache the content each time we run the script
		// do this in a fork process, so that we can call printFrontend multiple times
		$pid = $this->fork();
		if (!$pid) {
			$GLOBALS['BULKMAIL_JOB']->cacheContent($GLOBALS['BULKMAIL_JOB'], $content_details);
			echo "cached content!\n";
			exit;
		} else {
			echo "Parent $pid exits, after cache content\n";
		}

		// read chunks from cache and process them
		while (file_exists($recipient_path.'/recipient_'.$progress_info['current_chunk'])) {

			$this->_reconnectDB();
			// if not live and running, exit (paused/cancelled)
			$status = Bulkmail_Job::getStatusDB($GLOBALS['BULKMAIL_JOB']->id);
			$state = $GLOBALS['BULKMAIL_JOB']->getAttrDB('current_state');
			if ($status != SQ_STATUS_LIVE || $state != BML_JOB_STATE_RUNNING) {
				echo " job is not live/running, exit\n";
				exit;
			}

			// load up current chunk
			$progress_info = $GLOBALS['BULKMAIL_JOB']->_readProgressFile($progress_path);
			include($recipient_path.'/recipient_'.$progress_info['current_chunk']);
			echo "\nfork process - recipient chunk ".$progress_info['current_chunk']."\n";
			bam($recipient_chunk);

			$pid = $this->fork();
			if (!$pid) {
				$mail_queue =& new Mail_Queue($queue_options, $mail_options);

				for ($recipient_chunk; null !== ($user_id = key($recipient_chunk)); next($recipient_chunk)) {
					$email = $recipient_chunk[$user_id];
					$this->_reconnectDB();
					if (!$GLOBALS['BULKMAIL_JOB']->isUserInQueue($user_id) && !$GLOBALS['BULKMAIL_JOB']->isUserInSent($user_id)) {
						$to = 'rong@squiz.net'; // for testing purposes
						//$to = $email;
						// fork since we cannot call printFrontend multiple times
						$pid = $this->fork();
						if (!$pid) {
							$content = $GLOBALS['BULKMAIL_JOB']->generateBmailContent($GLOBALS['BULKMAIL_JOB'], $content_details, $user_id);
							$mime->setHtmlBody($content);
							//$mime->setTXTBody($content);
							$body = @$mime->get();
							// pass in custom_id as extra header details, for queue file naming
							$header_details['custom_id'] = $user_id;
							$hdrs = @$mime->headers($header_details);
							// note: 'from' field must be defined in the header details
							$mail_id = $mail_queue->put('', $to, $hdrs, $body);
							$GLOBALS['BULKMAIL_JOB']->_updateProgressFile($progress_path, $user_id, $email);
							exit;
						}

					} else {
						echo " $user_id already in queue/sent (skip, using file_exists)\n";
					}
				}
				// proceed to next chunk
				$GLOBALS['BULKMAIL_JOB']->_nextChunk($progress_path);
				echo " sendMailsInQueue\n";
				$mail_queue->sendMailsInQueue();
				exit;

			} else {
				$progress_info = $GLOBALS['BULKMAIL_JOB']->_readProgressFile($progress_path);
				echo "Parent $pid exits\n";
			}

		}//end while()

		$progress_info = $GLOBALS['BULKMAIL_JOB']->_readProgressFile($progress_path);

		if ($progress_info['current_count'] == $progress_info['total_count']) {
			echo "finished job\n";
			$this->_reconnectDB();
			// TODO: job is finished, remove it from the db table here
			// ie. job is live and not in the db table = finished

			// purge the data directory
			if (is_dir($GLOBALS['BULKMAIL_JOB']->data_path.'/.data')) {
				delete_directory($GLOBALS['BULKMAIL_JOB']->data_path.'/.data');
			}
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


}//end class


?>