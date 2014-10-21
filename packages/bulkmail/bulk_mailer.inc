<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: bulk_mailer.inc,v 1.106 2013/03/05 05:12:26 akarelia Exp $
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
* @version $Revision: 1.106 $
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
	function __construct()
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
		//We will intensively use fork from now on
		//therefore set oci_commit_on_success mode on to avoid end-of-communication issue for oracle + fork
		if (MatrixDAL::getDbType() === 'oci') {
			MatrixDAL::setOciCommitOnSuccess(TRUE);
		}
		// setup signal handlers
		pcntl_signal(SIGTERM, Array(&$this,'_sigHandler'));
		pcntl_signal(SIGHUP, Array(&$this,'_sigHandler'));

		$am = $GLOBALS['SQ_SYSTEM']->am;
		$bm = $am->getSystemAsset('bulkmail_manager');

		// setup the log manager to log bulkmail errors
		$old_log_errors = ini_set('log_errors', '1');
		$old_error_log  = ini_set('error_log',  SQ_LOG_PATH.'/'.$this->error_log_file_name.SQ_CONF_LOG_EXTENSION);
		set_error_handler(Array(&$this, '_errorHandler'));

		// login as root user
		$root_user = $am->getSystemAsset('root_user');
		if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
			trigger_localised_error('BML0003', sprintf(translate('Unable to set %1$s (Id #%2$s) as the current user.'), $root_user->name, $root_user->id), E_USER_WARNING);
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
				}
			}
		}
		if (MatrixDAL::getDbType() === 'oci') {
			MatrixDAL::setOciCommitOnSuccess (FALSE);
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
				trigger_localised_error('BML0004', translate('Failed to fork.'), E_USER_WARNING);
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
			$bm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');
			$info = $bm->getQueuedJobs($job_id);
			$job_path = $bm->getJobDataPath($job_id);

			$recipient_path = $job_path.'/cache/recipient';
			$progress_path = $job_path.'/progress_info';
			$recip_per_chunk = $this->calculateChunkSize($info[$job_id]['details']['threshold'], $progress_path);
			$progress_info = Array();
			if (file_exists($progress_path)) {
				include $progress_path;
			} else {
				trigger_localised_error('BML0009', sprintf(translate('File "%s" is not readable.'), $progress_path), E_USER_WARNING);
			}

			// start caching if not previously cached
			if (!is_dir($recipient_path)) {
				$result = $this->getAllRecipients($info[$job_id]['details']['content_id'], $info[$job_id]['details']['recipients']);
				// update total_count and list of problematic recipients in the progress file
				$progress_info['total_count'] = count($result['recipient']);
				$progress_info['problematic'] = $result['problematic'];
				if (!array_to_file($progress_info, 'progress_info', $progress_path)) {
					trigger_localised_error('BML0008', sprintf(translate('File "%s" is not writable.'), $progress_path), E_USER_WARNING);
				}

				// cache recipients into smaller chunks
				create_directory($recipient_path);
				$chunks = array_chunk($result['recipient'], $recip_per_chunk, TRUE);
				foreach ($chunks as $index => $chunk) {
					if (!array_to_file($chunk, 'recipient_chunk', $recipient_path.'/recipient_'.$index)) {
						trigger_localised_error('BML0008', sprintf(translate('File "%s" is not writable.'), $recipient_path.'/recipient_'.$index), E_USER_WARNING);
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
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$this->_reconnectDB();
		$bm = $am->getSystemAsset('bulkmail_manager');
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
		$content_asset = $am->getAsset($details['content_id']);

		$details['search_page'] = NULL;
		$details['user_subscriptions'] = NULL;

		if ((get_class($content_asset) == 'Search_Page') && !$details['user_subscriptions_status']) {
			trigger_localised_error('BML0020', translate('Asset to send is configured as a Search Page but the User subscription for the bulkmail Job is disabled.'), E_USER_WARNING);
			return FALSE;
		} else if ((get_class($content_asset) != 'Search_Page') && $details['user_subscriptions_status']) {
			trigger_localised_error('BML0021', translate('User subscription for the Bulkmail Job is enabled but the asset to send isn\'t set to a Search Page Asset.'), E_USER_WARNING);
			return FALSE;
		} else if ((get_class($content_asset) == 'Search_Page') && $details['user_subscriptions_status'] && empty($details['subscriptions'])) {
			trigger_localised_error('BML0023', translate('No dynamic Subscription configured for the Bulkmail Job'), E_USER_WARNING);
			return FALSE;
		} else if ((get_class($content_asset) == 'Search_Page') && $details['user_subscriptions_status']) {
			// the settings seems to be correct
			$details['search_page'] = $content_asset;
			$details['user_subscriptions'] = $details['subscriptions'];
		}

		// If the content asset has a different character set to the default,
		// then use that instead, otherwise use the default set in System Config
		if (!empty($content_asset->charset)) {
			$content_charset = $content_asset->charset;
		} else {
			$content_charset = SQ_CONF_DEFAULT_CHARACTER_SET;
		}

		$mime_param = Array(
						'head_charset'	=> $content_charset,
						'text_charset'	=> $content_charset,
						'html_charset'	=> $content_charset,
					  );

		foreach ($keywords as $word) {
			if (0 === strpos($word, 'recipient_')) {
				// keep recipient keyword until later
				$replacements[$word] = "%$word%";
			} else {
				$replacements[$word] = $content_asset->getKeywordReplacement($word);
			}
		}
		$am->forgetAsset($content_asset);

		replace_keywords($details['subject'], $replacements);
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
				$mime = new Mail_mime("\n");
				$mail_queue = new Mail_Queue($details['queue_details'], $details['server_details']);

				if (strpos($job_id, ':') === FALSE) {
					$job = $am->getAsset($job_id);
					$current_post_office = $job->getPostOffice();
				} else {
					$job = $current_post_office = NULL;
				}

				if ($details['content_text_only']) {
					unset($details['content_design']);
					unset($details['content_layout']);
				}

				foreach ($recipient_chunk as $r_info) {
					$this->_reconnectDB();
					$user_id = $r_info['assetid'];
					$email = $r_info['custom_val'];
					$user  = $am->getAsset($user_id, '', TRUE);

					// do not add the email to queue if there is no recipient's email address. This will
					// make sure that the sendMailsInQueue() method of Mail_Queue class does not throw an
					// exception when the first email in the chunk of 100 emails is empty which may leaves
					// 99 valid email addresses not being sent to. Further to that bug fx #5847 Bulkmail
					// job does not correctly check for duplicate mails in its' queue if the email has
					// leading/trailing white spaces or line breaks then that should be termed as invalid
					// and don't allow it to be queued up or else PEAR package will go in infinite loop
					if ((is_string($email) && (trim($email) == '')) || ($email != trim($email) || !valid_email($email))
						|| (is_array($email) && (trim($email['email']) == '')) || (array_get_index($email, 'email', FALSE) && ($email['email'] != trim($email['email']) || !valid_email($email['email'])))) {
						$this->updateProgress($progress_path, $user_id, FALSE, FALSE);
						continue;
					}

					// make sure user is not already in mail queue/sent directory
					$id = md5($user_id);

					if (!file_exists($queue_path.'/'.$id) && !file_exists($sent_path.'/'.$id)) {
						// fork since we cannot call printFrontend multiple times
						$pid = $this->fork();
						if (!$pid) {
							$problematic = NULL;
							$filtered = NULL;

							if (!is_null($details['search_page']) && !is_null($details['user_subscriptions'])) {
								$search_asset_list = Array();
								$user_metadata = NULL;
								$matches = Array();
								if (!is_null($user)) {
									foreach ($details['user_subscriptions'] as $sub_name => $sub_info) {
										$matches[$sub_name] = Array();
										foreach ($sub_info as $index => $info) {
											$match = FALSE;
											$category_type = array_keys($info);
											$user_metadata = $mm->getMetadataValueByAssetid($user->id, $info[$category_type[0]]['assetid']);
											$metadata_field= $GLOBALS['SQ_SYSTEM']->am->getAsset($info[$category_type[0]]['assetid']);
											$rule_text = $info[$category_type[0]]['rule_text'];
											$old_frontend_asset = $GLOBALS['SQ_SYSTEM']->frontend_asset;

											$GLOBALS['SQ_SYSTEM']->frontend_asset = $job;
											$GLOBALS['SQ_SYSTEM']->replaceKeyword($rule_text);
											$GLOBALS['SQ_SYSTEM']->frontend_asset = $old_frontend_asset;

											if ($info[$category_type[0]]['rule_cond'] == 'contains') {
												if ($metadata_field instanceof Metadata_Field_Select) {
													$rule_text_array = explode(';', $rule_text);

													foreach ($rule_text_array as $individial_text) {
														$match = (stripos($user_metadata, trim($individial_text)) !== FALSE);
														if ($match) break;
													}
												} else {
													$match = (stripos($user_metadata, $rule_text) !== FALSE);
												}
											} else {
												// needs a exact match
												$match = (strtolower($user_metadata) == strtolower($rule_text));
											}

											if ($match && !in_array($user_metadata, $matches[$sub_name])) {
												if ($metadata_field instanceof Metadata_Field_Select && strpos($user_metadata, '\\;') !== FALSE) {
													$matches[$sub_name] = explode('\;', $user_metadata);
												} else {
													$matches[$sub_name][] = $user_metadata;
												}
											}
										}//end foreach
									}//end foreach
								}//end if

								// check to see if the user is a bulkmail user
								// we can't set current user as a bulkmail user
								if ($user instanceof Bulkmail_User) {
									$user = NULL;
									$user = $am->getAsset($details['content_gen_as']);
								}

								$GLOBALS['SQ_SYSTEM']->setCurrentUser($user);
								$old_get_vars = $_GET;
								$search_result = '';
								$_GET = $good_query = Array();
								foreach ($matches as $get_var => $strings_to_srch) {
									foreach ($strings_to_srch as $srch_string) {
										$_REQUEST['mode'] = 'results';
										$_GET[$get_var] = $srch_string;
										$search_vars    = $content_asset->processSearchVars();

										$this->_reconnectDB(TRUE);

										$search_res = $content_asset->processSearch($search_vars);

										unset($_GET[$get_var]);
										if (!empty($search_res)) {
											$good_query[$get_var][] = $srch_string;
										}

										$this->_reconnectDB();
									}//end foreach
									$_GET = $good_query;
								}//end foreach
								$this->_reconnectDB(TRUE);

								if (!empty($_GET)) $search_result .= $this->generateContent($details, $job_path, $user_id, $email, TRUE);

								$this->_reconnectDB();

								$_GET = $old_get_vars;
								$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();

								$details['content_id'] = NULL;
								$details['search_content'] = $search_result;
							}//end if

							// if no search results were found then update the progress because we have already processed this user
							if (empty($details)) $this->updateProgress($progress_path, $user_id, $problematic, $filtered);

							if (!isset($details['search_content'])) {
								$content = $this->generateContent($details, $job_path, $user_id, $email);
							} else {
								$content = $details['search_content'];
							}
							$text_only_content = html_entity_decode(strip_tags($content));

							// If there is a filter set, only continue if the
							// filter matches (or is ineffective due to empty
							// text field).
							if (array_get_index($details, 'content_body_filter')) {
								$match       = array_get_index($details, 'content_body_filter_match', TRUE);
								$filter_text = array_get_index($details, 'content_body_filter_text', '');

								if (!empty($filter_text)) {
									$target_content = ($details['content_text_only'] ? $text_only_content : $content);
									$ok = (strpos($target_content, $filter_text) !== FALSE);
									if ($ok != $match) $filtered = TRUE;
								}
							}

							if (is_array($email)) {
								$header_details['subject'] = $this->replaceContentKeywords($header_details['subject'], $email);
							} else {
								$user = $am->getAsset($user_id);
								$header_details['subject'] = $this->replaceContentKeywords($header_details['subject'], $user);
							}//end if

							// only queue the mail if we can generate content
							$search_content_empty = FALSE;
							if (empty($content) && isset($details['search_content'])) {
								$search_content_empty = TRUE;
							} else if (empty($content)) {
								$problematic = TRUE;
							}

							if (!$problematic && !$filtered && !$search_content_empty) {
								// Check that the email is not the email address array
								if (is_array($email)) $email = $email['email'];

								// Always set text version, and set html version
								// if turned on.
								$mime->setTxtBody($text_only_content);
								if (!$details['content_text_only']) {
									$mime->setHTMLBody($content);
								}

								$body = @$mime->get($mime_param);
								// pass in custom_id as extra header detail, for queue file naming
								$header_details['custom_id'] = $user_id;
								$header_details['To'] = $email; // FR: 2515
								// Check for Test Mode
								if (!is_null($current_post_office) && $current_post_office->attr('bulkmail_mode') == 'test') {
									// Log the emails
									$message_type = 'bulkmail.job.testmode';
									$msg_reps = Array(
													'from'		=> $header_details['from'],
													'to'		=> $header_details['To'],
													'subject'	=> $header_details['subject'],
												);
									$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
									$message = $ms->newMessage(Array(), $message_type, $msg_reps);
									$message->subject = translate('Bulkmail Test Mode');
									$message->body    = translate('Mail request for %to% with a subject of \'%subject%\'');
									$message->parameters['assetid'] = $job_id;
									$message->send();

									$am->forgetAsset($message);
									$am->forgetAsset($ms);
								} else {
									$headers = @$mime->headers($header_details);
									$mail_id = $mail_queue->put('', $email, $headers, $body);
								}
							}
							$this->updateProgress($progress_path, $user_id, $problematic, $filtered);
							$this->enforceSendRate($details['threshold'], $progress_path);

							exit;
						}//end if
					}//end if

				}//end foreach

				if (!is_null($job)) $am->forgetAsset($job);
				if (!is_null($current_post_office)) $am->forgetAsset($current_post_office);
				try {
					$status = $mail_queue->sendMailsInQueue();
				} catch (Exception $e) {
					trigger_localised_error('BML0001', sprintf(translate('Invalid job. %s'), $e->getMessage()), E_USER_WARNING);
					$this->nextChunk($progress_path);
					exit;
				}

				if ($status instanceof PEAR_Error) {
					trigger_localised_error('BML0001', sprintf(translate('Invalid job. %s'), $status->getMessage()), E_USER_WARNING);
					$this->nextChunk($progress_path);
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
			if (strpos($job_id, ':') === FALSE) {
				$job = $am->getAsset($job_id);
				$current_post_office = $job->getPostOffice();
			} else {
				$job = $current_post_office = NULL;
			}

			// remove job entry from the db queue table
			$bm->deleteJob($job_id);

			// if this is not an ad-hoc job, log an internal message
			$job_id_parts = explode(':', $job_id);
			if (!isset($job_id_parts[1])) {

				if (!is_null($current_post_office) && $current_post_office->attr('bulkmail_mode') == 'test') {
					$message_type = 'bulkmail.job.testmode.completed';
					$msg_subject  = translate('Bulkmail Test Mode Completed');
					$msg_body     = translate('Bulkmail Test mode was completed. Mail was requested for %num_recip% recipients.');
					$msg_reps = Array(
									'num_recip'		=> number_format($progress_info['total_count']),
									'num_problem'	=> number_format(count($progress_info['problematic'])),
									'num_filtered'	=> isset($progress_info['filtered']) ? number_format(count($progress_info['filtered'])) : 0,
								);
				} else {
					$subtype = '';
					$msg_subject = translate('Bulkmail Job Completed');
					$msg_body    = translate('<strong>Mail successfully sent to %num_recip% recipients.</strong>');
					if (count($progress_info['problematic']) > 0) {
						trigger_localised_error('BML0010', sprintf(translate('Problematic recipient id found: %s.', implode(','), $progress_info['problematic'])), E_USER_WARNING);
						$subtype = '.error';
                        $msg_subject = translate('Bulkmail Job Completed with Error');
                        $msg_body   .= '<br/>'.translate('%num_problem% recipients were skipped due to problems in generation.');
					}

					if (isset($progress_info['filtered']) && (count($progress_info['filtered']) > 0)) {
                        $msg_body .= '<br/>'.translate('%num_filtered% recipients were also skipped as their bulkmail\'s contents failed to match the enabled Body filter.');
						trigger_localised_error('BML0019', sprintf(translate('Recipient id skipped due to body filter failing to match: %s.', implode(','), $progress_info['filtered'])), E_USER_WARNING);
						if (empty($subtype)) {
							$subtype     = '.filtered';
							$msg_subject = translate('Bulkmail Job Completed, some Filtered');
                        } else {
							$subtype    .= '+filtered';
							$msg_subject = translate('Bulkmail Job Completed with Error and some Filtered');
						}
					}

					if ($subtype !== '') {
					     $msg_body .= ' '.translate('Please check the Bulkmail Error log for details.');
					}
					$message_type = 'bulkmail.job.completed'.$subtype;

					// format recipient count with grouped thousands
					$msg_reps = Array(
									'num_recip'		=> number_format($progress_info['total_count']),
									'num_problem'	=> number_format(count($progress_info['problematic'])),
									'num_filtered'	=> isset($progress_info['filtered']) ? number_format(count($progress_info['filtered'])) : 0,
								);
				}//end if

				// Send the message
				$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
				$message = $ms->newMessage(Array(), $message_type, $msg_reps);
				$message->subject = $msg_subject;
				$message->body    = $msg_body;
				$message->parameters['assetid'] = $job_id;
				$message->send();
				$am->forgetAsset($message);
				$am->forgetAsset($ms);
			}
			if (!is_null($job)) $am->forgetAsset($job);
			if (!is_null($current_post_office)) $am->forgetAsset($current_post_office);

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
	function _reconnectDB($dbsearch=FALSE)
	{
		if (!$dbsearch) {
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection(TRUE);
			//force a new oci connection to avoid end-of-communication issue of oracle + fork
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2', TRUE);
		} else {
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection(TRUE);
			//force a new oci connection to avoid end-of-communication issue of oracle + fork
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('dbsearch', TRUE);
		}

	}//end _reconnectDB()


	/**
	* Writes the progress info to a file after a mail has been added to the queue
	*
	* If a user was both problematic AND filtered (probable if enabled), it will
	* be marked as the former.
	*
	* @param string		$progress_path	path to the progress info file
	* @param int		$user_id		current recipient assetid
	* @param boolean	$problematic	TRUE means we couldn't generate bmail for this user
	* @param boolean	$filtered		TRUE means user's bmail was filtered out
	*
	* @return boolean
	* @access public
	*/
	function updateProgress($progress_path, $user_id, $problematic, $filtered)
	{
		$progress_info = Array();
		if (file_exists($progress_path)) {
			include $progress_path;
		} else {
			trigger_localised_error('BML0009', sprintf(translate('File "%s" is not readable.'), $progress_path), E_USER_WARNING);
			return FALSE;
		}

		if ($problematic) {
			$progress_info['total_count']--;
			$progress_info['problematic'][] = $user_id;
		} else if ($filtered) {
			$progress_info['total_count']--;
			$progress_info['filtered'][] = $user_id;
		} else {
			$progress_info['current_count']++;
		}
		$progress_info['current_recip_id'] = $user_id;
		$progress_info['last_updated'] = time();

		if (!array_to_file($progress_info, 'progress_info', $progress_path)) {
			trigger_localised_error('BML0008', sprintf(translate('File "%s" is not writable.'), $progress_path), E_USER_WARNING);
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
			trigger_localised_error('BML0009', sprintf(translate('File "%s" is not readable.'), $progress_path), E_USER_WARNING);
			return FALSE;
		}

		$progress_info['current_chunk']++;
		// record the time taken to process one recipient chunk
		if (!$progress_info['time_per_chunk']) {
			$progress_info['time_per_chunk'] = get_microtime() - $GLOBALS['chunk_start'];
		}

		if (!array_to_file($progress_info, 'progress_info', $progress_path)) {
			trigger_localised_error('BML0008', sprintf(translate('File "%s" is not writable.'), $progress_path), E_USER_WARNING);
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
	* @param int	$asset_to_send_id	the id of the asset to send
	* @param array	$recipients			the job's 'recipients' attribute stripped from vars
	*
	* @return array
	* @access public
	*/
	function getAllRecipients($asset_to_send_id, $recipients)
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;

		if (!is_array($recipients)) {
			$recipients = Array($recipients);
		}

		$asset_to_send = $am->getAsset($asset_to_send_id);
		$recipients_expanded = Array();
		$problematic_list = Array();

		foreach ($recipients as $recipient_key => $recipient_id) {
			// Test for email address
			if (isset($recipient_id['email']) && preg_match('/(.*)\@(.*)/is', $recipient_id['email'])) {
				$recipients_expanded[$recipient_key] = $recipients[$recipient_key];
			} else {
				// Otherwise it should be an asset id
				$recipient = $am->getAsset($recipient_id);

				// If the asset is not a user, we should check for children
				if (!($recipient instanceOf User) && !($recipient instanceOf Bulkmail_User)) {
					// It is no longer just user group
					// If it is a search folder, do something, otherwise just try to get children
					if ($recipient->type() == 'search_folder') {
						$map_links	= $recipient->getAssetMapLinks();
						foreach ($map_links as $map_link) {
							// We only add this asset if it is a user or bulkmail user
							$all_user_types	= Array();
							$user_types				= $am->getTypeDescendants('user', TRUE);
							$bulkmail_user_types	= $am->getTypeDescendants('bulkmail_user', TRUE);
							$all_user_types	= array_merge($user_types, $bulkmail_user_types);
							if (in_array($map_link['type_code'], $all_user_types)) {
								$id_parts		= explode(':', $map_link['assetid']);
								// We are so sure that it has shadow id, but just incase
								if (isset($id_parts[1])) {
									$link_assetid	= $id_parts[1];
									$recipients_expanded[$link_assetid]	= $map_link['type_code'];
								}//end if
							}//end if
						}//end foreach
					} else {
						$recipients_expanded += $am->getChildren($recipient->id, 'user', FALSE, FALSE);
						$recipients_expanded += $am->getChildren($recipient->id, 'bulkmail_user', FALSE, FALSE);
					}//end else
				} else {
					// Else it is a user, add single recipient
					$recipients_expanded[$recipient->id] = $recipient->type();
				}

				$am->forgetAsset($recipient);

			}//end if
		}//end foreach

		$recipients = Array();
		$recipients_query = Array();
		$permission_denied = $am->getPermission($asset_to_send_id, SQ_PERMISSION_READ, FALSE, FALSE);
		$public_user = $am->getSystemAsset('public_user');
		$public_user_id = $public_user->id;
		for ($recipients_expanded; NULL != ($recipient_id = key($recipients_expanded)); next($recipients_expanded)) {
			//if it has right permission, but it is not an email address
			if (!in_array($recipient_id, $permission_denied) &&  (!is_array($recipients_expanded[$recipient_id]) || !array_key_exists('email',$recipients_expanded[$recipient_id]))) {

				$recipient_id_parts = explode(':', $recipient_id);
				if (isset($recipient_id_parts[1])) {
					$recipient = $am->getAsset($recipient_id);
					// calls to getChildren() on bridge assets do not always return User assets
					if ($recipient instanceof User) {
						$recipients[] = Array(
											'assetid'	 => $recipient->id,
											'custom_val' => $recipient->attr('email'),
										);
					}
					$am->forgetAsset($recipient);
				} else {
					// Only do assets here
					$recipients_query[] = $recipient_id;
				}//end if
			} else if (!in_array($public_user_id, $permission_denied)) {
				// Make sure the public user has access, then process the email addresses
				if (isset($recipients_expanded[$recipient_id]['email']) && preg_match('/(.*)\@(.*)/is', $recipients_expanded[$recipient_id]['email'])) {
					// Do the email addresses here
					$recipients[] = Array(
										'assetid'		=> $recipient_id,
										'custom_val'	=> Array(
															'first_name'	=> $recipients_expanded[$recipient_id]['first_name'],
															'last_name'		=> $recipients_expanded[$recipient_id]['last_name'],
															'email'			=> $recipients_expanded[$recipient_id]['email'],
														   ),
									);
				}
			} else {
				// read access denied for this recipient
				$problematic_list[] = $recipient_id;
			}//end if
		}//end for

		$query_results = Array();
		if (!empty($recipients_query)) {
			// Eliminate any duplicate recipients before chunking to obtain optimal bite-sized portions
			$recipients_query = array_unique($recipients_query);

			// Chunkifise our query to get all the email addresses of REAL assets
			// User assets without an email will be filtered out here
			$chunk_size = 1000;
			$recipient_chunks = array_chunk($recipients_query, $chunk_size, TRUE);

			foreach ($recipient_chunks as $recipient_chunk) {
				try {
					$bind_vars['recipient_list'] = $recipient_chunk;
					$bind_vars['contextid'] = $GLOBALS['SQ_SYSTEM']->getContextId();
					$query_result = MatrixDAL::executeAssoc('bulkmail_package', 'getAllRecipients', $bind_vars);
					$query_results = array_merge($query_results, $query_result);
				} catch (Exception $e) {
					throw new Exception('Unable to get recipient list due to database error: '.$e->getMessage());
				}
			}
		}

		$recipients = array_merge($recipients, $query_results);
		$result['recipient'] = $recipients;
		$result['problematic'] = $problematic_list;

		return $result;

	}//end getAllRecipients()


//--        CONTENT        --//


	/**
	* Generates the html bulkmail content based on the user and content generation type
	*
	* Used in previewing mail and sending actual bmail (use each user or one user)
	* Pre-selected user is used when we cannot login bulkmail_users
	*
	* @param array	$job_details	Details of the job
	* @param string	$job_path		path to the effective job data dir
	* @param int	$recipient_id	the recipient of this email, empty is for preview mail
	* @param array	$email			the email of the recipient (or the email details), empty is for preview mail
	* @param boolen $force_regen	to force regen the content instead of using the cache for bulkmail user
	*
	* @return string
	* @access public
	*/
	public function generateContent($job_details, $job_path, $recipient_id=NULL, $email=NULL, $force_regen=FALSE)
	{

		$am = $GLOBALS['SQ_SYSTEM']->am;
		if (isset($job_details['content_id']) && $job_details['content_id']) {
			$content_asset = $am->getAsset($job_details['content_id']);
			$GLOBALS['SQ_SYSTEM']->frontend_asset = &$content_asset;

			$am->forgetAsset($content_asset);
		}

		if (!empty($recipient_id)) {
			if (!is_null($email) && is_array($email)) {
				$recipient = Array($recipient_id => $email);
			} else {
				$recipient = $am->getAsset($recipient_id);
			}
		} else {
			$recipient = $GLOBALS['SQ_SYSTEM']->user;
		}//end if

		// Create an instance of the job to load the content
		if (isset($job_details['job_id']) && !empty($job_details['job_id'])) {
			$job = $am->getAsset($job_details['job_id']);
		} else {
			$job = NULL;
		}//end if

		if (($job_details['content_gen'] == 'one_user') || ($recipient instanceof Bulkmail_User)) {

			// Same content for everyone, apart from recipient keywords but only if we are not using the user subscription
			// as the subscription using search page will be dynamic and there is no point in getting it outta cache, unless
			// we're previewing
			if ((file_exists($job_path.'/cache/content_file_'.$job_details['content_id'].'.html')) && !empty($recipient_id) && !$force_regen) {

				$content = file_to_string($job_path.'/cache/content_file_'.$job_details['content_id'].'.html');

			} else {

				// Log in as generator
				$generator_user = $am->getAsset($job_details['content_gen_as']);
				if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($generator_user)) {
					trigger_localised_error('BML0017', translate('Could not change to the correct user for content generation'), E_USER_WARNING);
					return '';
				}

				// Do the actual generation
				if (is_null($job)) {
					$content = Bulk_Mailer::generateRawContent($job_details);
				} else {
					$content = $job->generateRawContent($job_details, $recipient);
				}//end if

				// Log out generator
				$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();

				// Save to cache, unless previewing or not sending to actual users
				if (!empty($recipient_id) || !($recipient instanceof Bulkmail_User)) {
					string_to_file($content, $job_path.'/cache/content_file_'.$job_details['content_id'].'.html');
				}


			}//end else file doesn't exist or recipient empty

		} else {

			// We are generating content on a per-recipient basis
			// If the recipient is a non-user, we ask for the Raw Content based on the recipient passed
			if (is_array($recipient)) {
				// Do the actual generation
				if (is_null($job)) {
					$content = Bulk_Mailer::generateRawContent($job_details);
				} else {
					$content = $job->generateRawContent($job_details, $recipient);
				}//end if

				return Bulk_Mailer::replaceContentKeywords($content, $recipient);
			}//end if

			// Make the recipient the current user, unless we are previewing (empty recipient id)
			// in which case the current user is already the recipient
			if (!empty($recipient_id)) {
				if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($recipient)) {
					trigger_localised_error('BML0017', translate('Could not change to the correct user for content generation'), E_USER_WARNING);
					return '';
				}
			}

			// Do the actual generation
			if (is_null($job)) {
				$content = Bulk_Mailer::generateRawContent($job_details);
			} else {
				$content = $job->generateRawContent($job_details, $recipient);
			}//end if

			// If we logged in the recipient, log them out again
			if (!empty($recipient_id)) {
				$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();
			}

		}//end else

		return Bulk_Mailer::replaceContentKeywords($content, $recipient);

	}//end generateContent()


	/**
	* Generate the raw content to be sent, ignoring user switching and recpient keyword replacement
	*
	* @param array	$details	The details of the job
	*
	* @return string
	* @access private
	* @static
	*/
	public static function generateRawContent($details)
	{
		// moved functionality to asset_manager.inc for reusing the codes in triggers
		return $GLOBALS['SQ_SYSTEM']->am->generateRawContent($details);

	}//end generateRawContent()


	/**
	* Keyword replacement for customised email content of a user
	*
	* @param string	&$content	page content before keyword replacement
	* @param int	&$user		personalise content for this user
	*
	* @return string
	* @access public
	*/
	public static function replaceContentKeywords(&$content, &$user)
	{
		// to insert user keywords in the 'Asset to Send' content,
		// add 'recipient_' prefix in front of normal keywords
		require_once SQ_FUDGE_PATH.'/general/text.inc';

		$keywords = retrieve_keywords_replacements($content);
		$keyword_replacements = Array();
		foreach ($keywords as $word) {
			if (0 === strpos($word, 'recipient_')) {
				$user_keyword = substr($word, 10);
				if (is_array($user)) {
					// Process based on the first name, last name and email address supplied
					foreach ($user as $user_login => $user_details) {
						if ($user_keyword == 'login') {
							$keyword_replacements[$word] = $user_login;
						} else if (isset($user_details[$user_keyword])) {
							$keyword_replacements[$word] = $user_details[$user_keyword];
						} else {
							$keyword_replacements[$word] = "%$word%";
						}//end if
					}//end foreach
				} else {
					$user_replacement = $user->getKeywordReplacement($user_keyword);
					if ($user_replacement == "%$user_keyword%") {
						// Unknown user keyword, do not replace
						$keyword_replacements[$word] = "%$word%";
					} else {
						$keyword_replacements[$word] = $user_replacement;
					}
				}//end if
			} else {
				$keyword_replacements[$word] = '';
			}
		}
		require_once SQ_INCLUDE_PATH.'/general.inc';
		replace_global_keywords($content);
		replace_keywords($content, $keyword_replacements);
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
			if ($threshold['type'] == translate('Send Rate')) {

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
			$bm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');
			$mpm = $bm->attr('chunk_size');
		}

		// store mpm value in progress file
		$progress_info = Array();
		if (file_exists($progress_path)) {
			include $progress_path;
		} else {
			trigger_localised_error('BML0009', sprintf(translate('File "%s" is not readable.'), $progress_path), E_USER_WARNING);
		}
		$progress_info['mpm'] = $mpm;
		if (!array_to_file($progress_info, 'progress_info', $progress_path)) {
			trigger_localised_error('BML0008', sprintf(translate('File "%s" is not writable.'), $progress_path), E_USER_WARNING);
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
			trigger_localised_error('BML0009', sprintf(translate('File "%s" is not readable.'), $progress_path), E_USER_WARNING);
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
				trigger_localised_error('BML0008', sprintf(translate('File "%s" is not writable.'), $progress_path), E_USER_WARNING);
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
			if ($threshold['type'] == translate('Excluded Time')) {

				$excluded_days = Array();
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
		if ((error_reporting() & $err_no) || $terminate) {

			$bt = debug_backtrace();
			if (count($bt) > 1) {
				$real_bt_index = 0;
				// note the use of '_errorhandler' instead of '_errorHandler' in the last condition
				while (($real_bt_index < count($bt)-1) && ((FALSE !== strpos(array_get_index($bt[$real_bt_index], 'class'), 'locale_manager')) || (FALSE !== strpos(array_get_index($bt[$real_bt_index], 'file'), 'locale_manager')) || ($bt[$real_bt_index]['function'] == '_errorhandler'))) {
					$real_bt_index++;
				}
				if (isset($bt[$real_bt_index]['file'])) {
					$err_file = hide_system_root($bt[$real_bt_index]['file']);
				}
				if (isset($bt[$real_bt_index]['line'])) {
					$err_line = $bt[$real_bt_index]['line'];
				}
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
