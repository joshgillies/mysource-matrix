<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: hipo_herder.inc,v 1.39 2004/05/27 00:27:19 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) Herder (Manager ;P)
*
* @author  Blair Robertson <brobertson@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Herder extends MySource_Object
{

	/**
	* @var array of jobs that have been loaded
	*/
	var $_jobs = Array();

	/**
	* @var array of jobs that are queued to be run
	*/
	var $_queued_jobs = Array();

	/**
	* @var &object Hipo_backend_Outputter
	* Reference to the hipo backend outputter
	*/
	var $ho = null;

	/**
	* @var integer
	* the current threshold value
	*/
	var $threshold = 0;


	/**
	* Constructor
	*
	*/
	function HIPO_Herder()
	{
		
	}//end constructor


	/**
	* Backend outputter for hipos
	*
	* @return object Hipo_Backend_Outputter
	* @access public
	*/
	function &getBackendOutputter()
	{
		if(!isset($this->ho) || get_class($this->ho) != 'hipo_backend_outputter') {
			require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_backend_outputter.inc';
			$this->ho = new Hipo_Backend_Outputter();
		}
		return $this->ho;

	}//end getBackendOutputter()


	/**
	* Returns the job for the passed code name
	*
	* @param string	$code_name	the codename of the job
	*
	* @return object HIPO_Job | null	returns the requested hipo job OR null on error
	* @access public
	*/
	function &getJob($code_name)
	{
		if (empty($this->_jobs[$code_name])) {
			
			// we need to get the hipo_dir from the db so we know
			// where to look for the include file
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$db = &$GLOBALS['SQ_SYSTEM']->db;
			$sql = 'SELECT job_type, hipo_vars
					FROM '.SQ_TABLE_PREFIX.'hipo_job
					WHERE code_name = '.$db->quote($code_name);
			$hipo_vars = $db->getRow($sql);
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

			if (DB::isError($hipo_vars)) {
				trigger_error($hipo_vars->getMessage().'<br/>'.$hipo_vars->getUserInfo(), E_USER_WARNING);
				$null = null;
				return $null;
			}

			$job_type = $hipo_vars['job_type'];
			if (empty($job_type)) {
				$null = null;
				return $null;
			}
			
			$vars = unserialize($hipo_vars['hipo_vars']);
			if (!isset($vars['job_dir']) || empty($vars['job_dir'])) {
				$job_dir = SQ_SYSTEM_ROOT.'/core/hipo/jobs';
			} else {
				$job_dir = $vars['job_dir'];
			}

			require_once $job_dir.'/'.$job_type.'.inc';
			$this->_jobs[$code_name] = new $job_type($code_name);

			if (!$this->_jobs[$code_name]->code_name) {
				// something buggered up, not our problem
				$null = null;
				return $null;
			}

		}//end if

		return $this->_jobs[$code_name];

	}//end getJobStack()


	/**
	* Return the URL that will continue the processing of the HIPO
	*
	* If the HIPO is finsihed, the URL returned will be the on_complete_url in the
	* _options array of the HIPO so that the HIPO can redirect back to where it started
	*
	* @return string
	* @access public
	*/
	function getProcessURL($code_name)
	{
		$hipo = &$this->getJob($code_name);
		if (is_null($hipo)) return '';

		if ($hipo->complete()) {
			// this HIPO is finsihed
			return $hipo->getOption('on_complete_url');
		} else {
			return $_SERVER['PHP_SELF'].'?SQ_ACTION=hipo&hipo_source_code_name='.$hipo->source_code_name;
		}

	}//end getProcessURL()


	/**
	* Called by the web script to inform the hipo manager that it can start processing
	*
	* @return boolean
	* @access public
	*/
	function processWeb()
	{
		if (empty($_GET['hipo_source_code_name'])) {
			trigger_error('HIPO source codename not specified', E_USER_WARNING);
			return false;
		}

		$source_job = &$this->getJob($_GET['hipo_source_code_name']);
		if (is_null($source_job)) {
			// if we aren't actually waiting for an aborted hipo to remove itself then, display an error
			if (empty($_POST['aborting_hipo'])) {
				trigger_error('Unable to find a source job with the codename "'.$_GET['hipo_source_code_name'].'"', E_USER_WARNING);
				return false;
			// else the server has finally killed the job so we are now done
			} else {
				return true;
			}
		}

		// they want to stop all processing and cancel the HIPO
		if (!empty($_POST['cancel_hipo'])) {
			if ($source_job->abort()) {
				$_POST['aborting_hipo'] = $source_job->code_name;
				$_POST['aborting_hipo_url'] = $source_job->getOption('on_complete_url');
			}
			return true;
		}

		if (!$source_job->process()) {
			$error_details = '';

			$o = &$this->getBackendOutputter();
			$o->setFormData('', '');
			$o->openRaw();
				$source_job->_paintErrorReport();
			$o->closeRaw();
			$o->paint();

			$source_job->abort();

			return false;
		}

		return true;

	}//end processWeb()


	/**
	* Called by the web script to paint the current process bar
	*
	* @return void
	* @access public
	*/
	function paintWeb()
	{
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';

		if (empty($_GET['hipo_source_code_name'])) {
			trigger_error('HIPO source codename not specified', E_USER_WARNING);
			return false;
		}

		$source_job = &$this->getJob($_GET['hipo_source_code_name']);
		if (is_null($source_job) && empty($_POST['aborting_hipo'])) {
			trigger_error('Unable to find a source job with the codename "'.$_GET['hipo_source_code_name'].'"', E_USER_WARNING);
			return false;
		}
		
		// if this job is completed but has errors, just paint the job to print the error
		// report because we dont need the addtional content
		if ($source_job->complete() && !empty($source_job->_hipo_vars['errors'])) {
			$o = &$this->getBackendOutputter();
			$o->setFormData('main_form', '');
			$o->openRaw();
			$source_job->paint($o);
			$o->closeRaw();
			$o->paint();
			return;
		}

		$onload = '';
		$delay  = false;
		$auto_step = false;
		$form_name = 'main_form';

		// if the source job is null it means that an abort finally finished
		// (we couldn't get here otherwise) so print that and get out of here
		if (is_null($source_job)) {
			$url = $_POST['aborting_hipo_url'];
			$onload = 'self.location = "'.$url.'";';
			$auto_step = true;
			$percent_done = 100;
			$status_msg = 'HIPO Aborted';
		} else {
			$url = $this->getProcessURL($source_job->code_name);
			$auto_step = $source_job->auto_step();
			$percent_done = $source_job->percentDone();

			if ($source_job->complete()) {
				$onload = 'self.location = "'.$url.'";';
			} else {
				$delay = ($source_job->getRunningMode() == 'server');
				$onload = 'document.'.$form_name.'.submit();';
			}
			$status_msg = $source_job->hipo_name.' HIPO '.(($source_job->complete()) ? 'Complete' : 'Progress');
		}

		$o = &$this->getBackendOutputter();
		$o->setFormData($form_name, $url);
		if ($auto_step) {
			if ($delay) {
				$o->addOnLoad('ON_LOAD_TIME_OUT = setTimeout("next_page();", 2000)');
			} else {
				$o->addOnLoad('next_page()');
			}
		}
		
		$o->openRaw();
		?>
		<script language="JavaScript" type="text/javascript">
			function next_page() {
				<?php echo $onload; ?>
			}// end next_page()

			function send_abort() {
				if (ON_LOAD_TIME_OUT != null) {
					clearTimeout(ON_LOAD_TIME_OUT);
				}
				if (confirm("Are you sure you want to cancel?")) {
					document.<?php echo $form_name; ?>.cancel_hipo.value = "1";
					document.<?php echo $form_name; ?>.submit();
				} else {
					<?php
					if ($source_job->getRunningMode() == 'server') {
						?>next_page();<?php
					}
					?>
				}
			}// end send_abort()
		</script>
		<?php
	

		?>
		<table border="0" cellspacing="0" cellpadding="0" width="500">
			<tr>
				<td class="sq-hipo-header">
					<table class="sq-hipo-header">
						<tr>
							<td valign="middle" width="100%"><img src="<?php echo sq_web_path('lib').'/web/images/icons/hipo.gif'; ?>" width="71" height="26" alt="Powered by Highly Intensive Processing Object (HIPO) Technology" /></td>
						</tr>
						<tr>
							<td class="sq-hipo-header">
								<?php HIPO_Job::paintProgressBar($percent_done, $status_msg, 'sq-hipo-header-progress-bar-label', 'sq-hipo-header-progress-bar-percent', 'sq-hipo-header-progress-bar-main', 'sq-hipo-header-progress-bar-done'); ?>
							</td>
						</tr>
					</table>
				</td>
			</tr>
			<tr>
				<td class="sq-hipo-main" valign="top">
				<?php
				$buttons_to_print = Array();

				// if we aren't waiting for an abort, print the progress stuff
				if (empty($_POST['aborting_hipo'])) {

					// Paint the source job, this will take care of any sub jobs if needed
					$o->closeRaw();
					$source_job->paint($o);
					$o->openRaw();

					if (!$source_job->complete()) {
						// basically what happens here is that if cancel has already been pressed we increment the counter
						// this means that the cancel button only get's pressed once
						hidden_field('cancel_hipo', (empty($_POST['cancel_hipo'])) ? 0 : ((int) $_POST['cancel_hipo'] + 1));
						$buttons_to_print['cancel'] = Array('value' => 'Cancel', 'action' => 'send_abort();');
					}

					// print the NEXT or FINISH button if we are not auto stepping through the HIPO
					if (!$auto_step) {
						// they dont want us to refresh automatically, so we need to provide a manual NEXT button
						$button_text = ($source_job->complete()) ? 'Finish' : 'Next >>';
						$buttons_to_print['next'] = Array('value' => $button_text, 'action' => 'next_page();');
					}

				// we are aborting from the server
				} else {

					hidden_field('aborting_hipo',     $_POST['aborting_hipo']);
					hidden_field('aborting_hipo_url', $_POST['aborting_hipo_url']);

					// if the source job is null it means that an abort finally finished
					// (we couldn't get here otherwise) so print that and get out of here
					if (is_null($source_job)) {
						echo 'Abort Complete';
						$button_text = 'OK';
					} else {
						echo 'Aborting...';
						$button_text = 'Next >>';
					}

					// print the NEXT or FINISH button if we are not auto stepping through the HIPO
					if (!$auto_step) {
						// they dont want us to refresh automatically, so we need to provide a manual NEXT button
						$buttons_to_print['next'] = Array('value' => $button_text, 'action' => 'next_page();');
					}

				}// end if

				if (count($buttons_to_print)) {
					?>
					<p>
					<table border="0" width="100%" cellspacing="2" cellpadding="1">
						<tr>
							<td width="100%">&nbsp;</td>
							<?php
							foreach ($buttons_to_print as $btn_code => $btn_data) {
								?><td valign="top" align="center"><?php
								normal_button($btn_code, $btn_data['value'], $btn_data['action'], 'class="sq-hipo-button-enabled"');
								?></td><?php
							}
							?>
						</tr>
					</table>
					</p>
					<?php
				}// end if
				?>
				</td>
			</tr>
		</table>
		<?php

		$o->closeRaw();
		$o->paint();

	}//end paintWeb()


	/**
	* Called by the daemon script to inform the hipo manager that it is about to start processing
	* This is needed so that we can check the the input from the web script and
	* return an ACK or an error to it before we start processing
	*
	* @param Array	$input_data		data sent from the web script
	* @param Array 	$output_data	data to be sent back to the web script - Array ('result' => [ack|error], 'msg' => '');
	* @param string	$taskid			the id of the task on the server (used if we need to abort later on)
	*
	* @return boolean	indicating whether processing should start or not
	* @access public
	*/
	function prepareProcessServer(&$input_data, &$output_data, $taskid)
	{
		$this->_tmp['prepared_server'] = Array();

		if (empty($input_data['userid'])) {
			$output_data['result'] = 'error';
			$output_data['msg']    = 'userid not supplied on Input Data';
			return false;
		}

		$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($input_data['userid']);
		if (is_null($user) || !$GLOBALS['SQ_SYSTEM']->setCurrentUser($user)) {
			$output_data['result'] = 'error';
			$output_data['msg']    = 'Unable to find User #'.$input_data['userid'].' to run as or unable to set them to the current user';
			return false;
		}

		if (empty($input_data['source_code_name'])) {
			$output_data['result'] = 'error';
			$output_data['msg']    = 'source_code_name not supplied on Input Data';
			return false;
		}

		$source_job = &$this->getJob($input_data['source_code_name']);
		if (is_null($source_job)) {
			$output_data['result'] = 'error';
			$output_data['msg']    = 'Unable to find a source job with the codename "'.$input_data['source_code_name'].'"';
			return false;
		}

		if (!$source_job->setRunningMode('server', $taskid)) {
			$output_data['result'] = 'error';
			$output_data['msg']    = 'Unable to set the running mode to server for "'.$source_job->code_name.'"';
			return false;
		}

		$this->_tmp['prepared_server']['source_code_name'] = $source_job->code_name;

		pcntl_signal(SIGTERM, Array(&$this, 'serverSigHandler'));
		pcntl_signal(SIGINT,  Array(&$this, 'serverSigHandler'));

		$output_data['result'] = 'ack';
		return true;

	}//end prepareProcessServer()


	/**
	* Called by the daemon script to inform the hipo manager that it can start processing
	* Process the current step of a HIPO in a server environment
	*
	* This function should only be called by a HIPO running on the HIPO server, so we assume
	* no timeouts here and continually call the process function of the current step until
	* it is completed.
	*
	* @return boolean
	* @access public
	*/
	function processServer()
	{
		if (empty($this->_tmp['prepared_server'])) return false;

		$source_job = &$this->getJob($this->_tmp['prepared_server']['source_code_name']);
		if (is_null($source_job)) {
			trigger_error('Unable to find a source job with the codename "'.$this->_tmp['prepared_server']['source_code_name'].'"', E_USER_WARNING);
			return false;
		}

		if ($source_job->uses_trans) {	// only encase in a transaction if source job wants it
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		}

		$this->_tmp['processing_server'] = true;

		$ret_val = true;
		while(!$source_job->complete() && $source_job->getRunningMode() == 'server') {
			if (!$source_job->process()) {
				$ret_val = false;
				break;
			}
		}// end while

		unset($this->_tmp['prepared_server']);
		unset($this->_tmp['processing_server']);

		if ($source_job->uses_trans) {	// complete transaction - only if desired by job
			if ($ret_val) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			} else {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			}
		}

		return $ret_val;

	}//end processServer()


	/**
	* The handler for the any signals from the system
	*
	* @return int		$signo	the signal number
	* @access public
	*/
	function serverSigHandler($signo)
	{
		switch($signo) {
			case SIGINT:
			case SIGTERM:
				error_log('Aborting Job...');
				if (!empty($this->_tmp['processing_server'])) {
					$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db'); // just to make sure
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				}
				if (!empty($this->_tmp['prepared_server'])) {
					$source_job = &$this->getJob($this->_tmp['prepared_server']['source_code_name']);
					if (!is_null($source_job)) {
						error_log("ABORT JOB --->");
						$source_job->abort();
					}
				}// end if
				exit();
			default:
				trigger_error('Signal '.$signo.' not handled', E_USER_NOTICE);
				break;
		}// end switch

	}//end serverSigHandler()


	/**
	* Queue up a HIPO Job to run at the next refresh
	*
	* @param string	$job_type			the type of job (hipo_job_BLAH)
	* @param array	$vars				HIPO vars to send to the job
	* @param string	$on_complete_url	the URL to redirect to once all HIPOs are done
	*									(can be left blank to use the current URL)
	* @param string	$job_dir			where to find this hipo job include files
	*									(can be left blank to use default hipo job directory)
	*
	* @return string
	* @access public
	*/
	function queueHipo($job_type, $vars=Array(), $on_complete_url='', $job_dir='')
	{
		if (empty($on_complete_url)) $on_complete_url = $_SERVER['REQUEST_URI'];
		if (empty($job_dir)) $job_dir = SQ_SYSTEM_ROOT.'/core/hipo/jobs';

		require_once $job_dir.'/'.$job_type.'.inc';
		$init_hipo = new $job_type();
		$init_hipo->setRunningVars($vars);

		// prepare() check the integrity of the the running vars
		if (!$init_hipo->prepare()) return false;
		$init_hipo->_hipo_vars['job_dir'] = $job_dir;
		$job_threshold = (float) $init_hipo->getThreshold();
		
		// if the job has not explictly asked to be run in hipo mode
		// see if we can freestyle it
		if ($job_threshold > 0) {
			if (($this->threshold + $job_threshold) <= SQ_HIPO_TOTAL_THRESHOLD) {
				$this->threshold += $job_threshold;
				set_error_handler(Array(&$init_hipo, '_errorHandler'));
				$init_hipo->freestyle();
				restore_error_handler();
				return $init_hipo->getErrors();
			}
		}

		$init_hipo->setOption('on_complete_url', $on_complete_url);

		// check if this hipo has already been queued up by this person
		// may happen when trying to perform the same task multiple times
		// in a single script execution
		$hipo_code = $init_hipo->getCodeName();
		if (isset($this->_queued_jobs[$GLOBALS['SQ_SYSTEM']->currentUserId()])) {
			if (in_array($hipo_code, $this->_queued_jobs[$GLOBALS['SQ_SYSTEM']->currentUserId()])) return true;
		}

		if (!$init_hipo->initialise()) return false;

		$this->_queued_jobs[$GLOBALS['SQ_SYSTEM']->currentUserId()][] = $hipo_code;
		return true;

	}//end queueHipo()


	/**
	* Prepare the HIPO Jobs that have been queued for the current user
	*
	* @param string	$on_complete_url	the URL to redirect to once all HIPOs are done
	*									(can be left blank to use the current URL when the
	*									HIPO jobs were queued)
	*
	* @return string | Array()
	* @access public
	*/
	function runQueuedJobs($on_complete_url='')
	{
		// note that we do not check rollback tables here because the
		// hipo job table doesn't need one and doesn't want one!!
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT code_name, job_type
				FROM '.SQ_TABLE_PREFIX.'hipo_job
				WHERE userid = '.$db->quote($GLOBALS['SQ_SYSTEM']->currentUserId()).'
				  AND running = 0';
		$queued_jobs = $db->getAll($sql);

		if (DB::isError($queued_jobs)) {
			trigger_error($queued_jobs->getMessage().'<br/>'.$queued_jobs->getUserInfo(), E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return '';
		}

		if (empty($queued_jobs)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return '';
		}

		$job_codes = Array();

		for ($i = 0; $i < count($queued_jobs); $i++) {
			$job_data = $queued_jobs[$i];
			$job = &$this->getJob($job_data['code_name']);
			$job_codes[] = $db->quote($job_data['code_name']);

			if (isset($queued_jobs[($i+1)])) {
				$next_url = $this->getProcessURL($queued_jobs[($i+1)]['code_name']);
				$job->setOption('on_complete_url', $next_url);
				if (!$job->save()) {
					trigger_error('HIPO Job failed to save', E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return '';
				}
			} else if (!empty($on_complete_url)) {
				$job->setOption('on_complete_url', $on_complete_url);
				if (!$job->save()) {
					trigger_error('HIPO Job failed to save', E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return '';
				}
			}
		}

		$sql = 'UPDATE '.SQ_TABLE_PREFIX.'hipo_job 
				SET running = 1
				WHERE code_name IN ('.implode(', ',$job_codes).')';

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return '';
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return $this->getProcessURL($queued_jobs[0]['code_name']);

	}//end runQueuedJobs()


	/**
	* Run the HIPO Job right now
	*
	* @param string	$job_type	the type of job (hipo_job_BLAH)
	* @param array	$vars		HIPO vars to send to the job
	* @param string	$job_dir	where to find this hipo job include files
	*							(can be left blank to use default hipo job directory)
	*
	* @return Array()
	* @access public
	*/
	function freestyleHipo($job_type, $vars=Array(), $job_dir='')
	{
		if (empty($job_dir)) $job_dir = SQ_SYSTEM_ROOT.'/core/hipo/jobs';
		
		require_once $job_dir.'/'.$job_type.'.inc';
		$init_hipo = new $job_type();
		$init_hipo->setRunningVars($vars);

		// prepare() check the integrity of the the running vars
		if (!$init_hipo->prepare()) return false;
		set_error_handler(Array(&$init_hipo, '_errorHandler'));
			$init_hipo->freestyle();
		restore_error_handler();
		return $init_hipo->getErrors();

	}//end freestyleHipo()


	/**
	* Print out the backend interface for the Herder
	*
	* @param object Backend	$backend	reference to the backend object
	*
	* @return void
	*/
	function paintBackend(&$backend)
	{
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();

		// if we dont have access, piss off
		if (!$is_root) {
			$GLOBALS['SQ_SYSTEM']->paintLogin('Login', 'You do not have permission to access the <i>HIPO Herder</i>');
			exit();
		}

		$lock = $GLOBALS['SQ_SYSTEM']->getLockInfo(get_class($this));
		$have_lock = (!empty($lock) && $GLOBALS['SQ_SYSTEM']->currentUserId() == $lock['userid']);

		$o = &$backend->out;

		if (!empty($_POST['herder_posted'])) {

			if ($have_lock && !empty($_POST['process_form']) && !empty($_POST['force_kill'])) {
				$message_body = '';
				foreach($_POST['force_kill'] as $source_code_name) {
					$source_job = &$this->getJob($source_code_name);
					if (is_null($source_job)) continue;
					// OK special case here, because to be able to save a hipo job, 
					// the current user needs to be the owner of the job
					$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($source_job->userid);
					if (is_null($user)) continue;
					$GLOBALS['SQ_SYSTEM']->setCurrentUser($user);
					$source_job->abort();
					$message_body .= 'FORCE KILL '.$source_code_name."\n";
					$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();
				}

				$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
				$message = $ms->newMessage(Array(), 'HIPO Herder', $message_body, 'hipo.force_kill');
				$message->send();

			}

			if (!empty($_POST['sq_lock_release']) || !empty($_POST['sq_lock_release_manual'])) {
				$GLOBALS['SQ_SYSTEM']->releaseLock(get_class($this));
			}

			// if there is no lock currently and we want it
			if (!empty($_POST['sq_lock_acquire'])) {
				$GLOBALS['SQ_SYSTEM']->acquireLock(get_class($this));
			}

			// just to make sure
			$lock      = $GLOBALS['SQ_SYSTEM']->getLockInfo(get_class($this));
			$have_lock = (!empty($lock) && $GLOBALS['SQ_SYSTEM']->currentUserId() == $lock['userid']);

		}// endif

		require_once SQ_FUDGE_PATH.'/general/datetime.inc';
		$o->setHeading('HIPO Herder', '<script language="JavaScript" type="text/javascript">sq_print_icon("'.$o->filesPath('/images/icons/header/hipo_herder.png').'", "20", "20", "HIPO Icon");</script>');
		$o->addHiddenField('herder_posted', '1');

		// lets tell the user if the asset is locked for editing
		// or if they can lock it to edit it
		$o->openSection('Locking / Editing');
		$o->openField('&nbsp;');

		?>
		<table border="0" cellspacing="3" cellpadding="1">
			<tr>
				<td valign="top" width="30">
					<script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo sq_web_path('lib'); ?>/web/images/icons/<?php echo ((empty($lock)) ? 'un' : ''); ?>locked.png", "16", "16", "");</script>
				</td>
				<td valign="top">
					<?php
					if (!empty($lock)) {
						// this asset is currently locked
						// so display message to the user
						$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($lock['userid']);

						$now = time();

						$expires_in = easy_time_total(($lock['expires'] - $now), true);
						if (!$expires_in) $expires_in = '1 second';
						$expires_in = 'The lock is due to expire in '.$expires_in;

						?>
						<p class="sq-backend-locked-by-<?php echo ($GLOBALS['SQ_SYSTEM']->currentUser($user)) ? 'user' : 'someone-else'; ?>">
							The HIPO Herder is currently locked for editing by user "<?php echo $user->name; ?>".<br>
							<?php echo $expires_in; ?>
						</p>
						<?php

					} else {
						?>
						<p class="sq-backend-unlocked">
							The HIPO Herder is currently Unlocked.
						</p>
						<?php
					}
					?>
				</td>
				<td valign="top">
					<?php
					if (!empty($lock)) {
						if ($have_lock) {
							submit_button('sq_lock_release_manual', 'Release Lock', 'set_hidden_field("process_form", "0");');
						}
					} else {
						submit_button('sq_lock_acquire', 'Lock', 'set_hidden_field("process_form", "0");');
					}
					?>
				</td>
			</tr>
		</table>
		<?php

		$o->closeSection();

		$o->openSection('Current HIPO Jobs');
			$o->openField('', 'new_line');

			$source_jobs = Array();

			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$db = &$GLOBALS['SQ_SYSTEM']->db;
			$sql = 'SELECT source_code_name, code_name, job_type
					FROM '.SQ_TABLE_PREFIX.'hipo_job';
			$result = $db->query($sql);
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			} else {
				while(DB_OK === $result->fetchInto($row)) {
					$source_code_name = ($row['source_code_name'] == $row['code_name']) ? '' : $row['source_code_name'];
					if (!isset($source_jobs[$source_code_name])) {
						$source_jobs[$source_code_name] = Array();
					}
					$source_jobs[$source_code_name][$row['code_name']] = $row['job_type'];
				}
			}

			if (empty($source_jobs[''])) {
				echo 'There are currently no HIPO Jobs on the system';

			} else {
				?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-header">Source Job Code Name</td>
						<td class="sq-backend-table-header">Code Name</td>
						<td class="sq-backend-table-header">Owner</td>
						<td class="sq-backend-table-header">Last Updated</td>
						<td class="sq-backend-table-header" style="text-align: center;">Percent Complete</td>
				<?php
					if ($have_lock) {
				?>
						<td class="sq-backend-table-header" style="text-align: center;">Delete ?</td>
				<?php
					} // end if have lock
				?>
					</tr>
				<?php
				$now = time();
				foreach($source_jobs[''] as $source_code_name => $source_job_type) {
					$source_job = &$this->getJob($source_code_name);
					if (is_null($source_job)) continue;
				?>
					<tr>
						<td class="sq-backend-table-cell">
							<?php echo $source_job->code_name; ?>
						</td>
						<td class="sq-backend-table-cell">
							<?php echo $source_job->code_name; ?>
						</td>
						<td class="sq-backend-table-cell">
						<?php 
							$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($source_job->userid);
							echo $user->name.'<br>(Id #'.$user->id.')';
						?>
						</td>
						<td class="sq-backend-table-cell">
							<?php echo $GLOBALS['SQ_SYSTEM']->datetime($source_job->last_updated); ?><br>
							<?php echo easy_time_total($source_job->last_updated - $now, true); ?>

						</td>
						<td class="sq-backend-table-cell" style="text-align: center;">
							<?php echo $source_job->percentDone(); ?>%
						</td>
				<?php
					if ($have_lock) {
				?>
						<td class="sq-backend-table-cell" style="text-align: center;">
							<?php check_box('force_kill[]', $source_job->code_name); ?>
						</td>
				<?php
					}// end if have_lock
				?>
					</tr>
				<?php

					if (!empty($source_jobs[$source_code_name])) {
						foreach($source_jobs[$source_code_name] as $code_name => $job_type) {
							$job = &$this->getJob($code_name);
						?>
							<tr>
								<td class="sq-backend-table-cell">
									&nbsp;
								</td>
								<td class="sq-backend-table-cell">
									<?php echo $job->code_name; ?>
								</td>
								<td class="sq-backend-table-cell">
									&nbsp;
								</td>
								<td class="sq-backend-table-cell">
									<?php echo $GLOBALS['SQ_SYSTEM']->datetime($job->last_updated); ?><br>
									<?php echo easy_time_total($source_job->last_updated - $now, true); ?>
								</td>
								<td class="sq-backend-table-cell" style="text-align: center;">
									<?php echo $job->percentDone(); ?>%
								</td>
								<td class="sq-backend-table-cell" style="text-align: center;">
									&nbsp;
								</td>
							</tr>
						<?php

						}// end foreach
					}// end if
				}// end foreach
				?>

				</table>
				<?php
			}// end if


		$o->closeSection();

		if ($have_lock) {
			$o->commitButton('Commit', true);
		}

	}//end printBackend()


}//end class

?>
