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
* $Id: hipo_herder.inc,v 1.111 2013/10/18 03:57:25 ewang Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) Herder (Manager ;P)
*
* @author  Blair Robertson <brobertson@squiz.net>
* @version $Revision: 1.111 $
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
	var $ho = NULL;

	/**
	* @var integer
	* the current threshold value
	*/
	var $threshold = 0;


	/**
	* Constructor
	*
	* @return object
	* @access public
	 */
	function HIPO_Herder()
	{

	}//end constructor


	/**
	* Backend outputter for hipos
	*
	* @return object
	* @access public
	*/
	function &getBackendOutputter()
	{
		if (!isset($this->ho) || get_class($this->ho) != 'Hipo_Backend_Outputter') {
			require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_backend_outputter.inc';
			$this->ho = new Hipo_Backend_Outputter();
		}
		return $this->ho;

	}//end getBackendOutputter()


	/**
	* Returns the job for the passed code name
	*
	* Returns the requested hipo job OR null on error
	*
	* @param string	$code_name	the codename of the job
	*
	* @return object
	* @access public
	*/
	function &getJob($code_name)
	{
		if (empty($this->_jobs[$code_name])) {

			// we need to get the hipo_dir from the db so we know
			// where to look for the include file
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db3');

			try {
				$bind_vars['code_name'] = $code_name;
				$hipo_vars_all = MatrixDAL::executeAssoc('core', 'getHipoJobByCodeName', $bind_vars);
				$hipo_vars = Array();
				if (!empty($hipo_vars_all) && isset($hipo_vars_all[0])) {
					$hipo_vars = $hipo_vars_all[0];
				}
			} catch (Exception $e) {
				throw new Exception('Unable to get HIPO job from code name: '.$code_name.$e->getMessage());
			}//end try catch

			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

			if (!isset($hipo_vars['job_type']) || empty($hipo_vars['job_type'])) {
				$null = NULL;
				return $null;
			}
			$job_type = strtolower($hipo_vars['job_type']);

			$vars = unserialize($hipo_vars['hipo_vars']);
			if (empty($vars['job_dir'])) {
				$job_dir = SQ_SYSTEM_ROOT.'/core/hipo/jobs';
			} else {
				$job_dir = strpos($vars['job_dir'], '/') === 0 ? $vars['job_dir'] : SQ_SYSTEM_ROOT.'/'.$vars['job_dir'];
			}

			$this->_includeHipoJobLocales($job_dir);

			require_once $job_dir.'/'.$job_type.'.inc';

			$this->_jobs[$code_name] = new $job_type($code_name);

			if (!$this->_jobs[$code_name]->code_name) {
				// something broke, not our problem
				$null = NULL;
				return $null;
			}

		}//end if

		return $this->_jobs[$code_name];

	}//end getJob()


	/**
	* De-caches a job from the job list, so it can be run again in the same script cleanly
	*
	* Mainly to do with freestylin' jobs that spawn a dependant sub-job - it needs to generate
	* a code name, and eventually hits the job cache. If the same job is run the same session,
	* it will re-use the job, even if complete.
	*
	* No checking of hipo job status performed. Probably not a good idea to run until a job
	* is complete (whether through abortion or natural circumstances).
	*
	* @param string	$code_name	The code name of the job to uncache
	*
	* @return boolean
	*/
	public function uncacheJob($code_name)
	{
		$job = $this->getJob($code_name);
		if ($job === NULL) {
			return FALSE;
		}

		unset($this->_jobs[$code_name]);
		return TRUE;

	}//end uncacheJob()


	/**
	* Return the URL that will continue the processing of the HIPO
	*
	* If the HIPO is finsihed, the URL returned will be the on_complete_url in the
	* _options array of the HIPO so that the HIPO can redirect back to where it started
	*
	* @param string	$code_name	the codename of the job
	*
	* @return string
	* @access public
	*/
	function getProcessURL($code_name)
	{
		$hipo = $this->getJob($code_name);
		if (is_null($hipo)) return '';

		if ($hipo->complete()) {
			// this HIPO is finsihed
			return $hipo->getOption('on_complete_url');
		} else {
			$host = array_get_index($_SERVER, 'HTTP_HOST', '');
			return current_protocol().'://'.$host.$_SERVER['PHP_SELF'].'?SQ_ACTION=hipo&hipo_source_code_name='.$hipo->source_code_name;
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
			trigger_localised_error('HIPO0058', translate('HIPO source codename not specified'), E_USER_WARNING);
			return FALSE;
		}

		$source_job = $this->getJob($_GET['hipo_source_code_name']);
		if (is_null($source_job)) {
			// if we aren't actually waiting for an aborted hipo to remove itself then, display an error
			if (empty($_POST['aborting_hipo'])) {
				trigger_localised_error('HIPO0057', sprintf(translate('Unable to find a source job with the codename "%s"'), $_GET['hipo_source_code_name']), E_USER_WARNING);
				return FALSE;
			// else the server has finally killed the job so we are now done
			} else {
				return TRUE;
			}
		}

		// they want to stop all processing and cancel the HIPO
		if (!empty($_POST['cancel_hipo'])) {
			if ($source_job->abort()) {
				$_POST['aborting_hipo'] = $source_job->code_name;
				$_POST['aborting_hipo_url'] = $source_job->getOption('on_complete_url');
			}
			return TRUE;
		}

		if (!empty($_GET['return_to_herder'])) {
			$source_job->_options['on_complete_url'] = './?SQ_BACKEND_PAGE=main&backend_section=hipo_herder';
		}

		$o = $this->getBackendOutputter();

		if (!$source_job->process()) {
			$error_details = '';
			$o->setFormData('', '');
			$o->openRaw();
				$source_job->_paintErrorReport();
			$o->closeRaw();
			$o->paint();

			$source_job->abort();

			return FALSE;
		}

		return TRUE;

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
			trigger_localised_error('HIPO0058', translate('HIPO source codename not specified'), E_USER_WARNING);
			return FALSE;
		}

		$source_job = $this->getJob($_GET['hipo_source_code_name']);
		if (is_null($source_job) && empty($_POST['aborting_hipo'])) {
			trigger_localised_error('HIPO0057', sprintf(translate('Unable to find a source job with the codename "%s"'), $_GET['hipo_source_code_name']), E_USER_WARNING);
			return FALSE;
		}

		// if this job is completed but has errors, just paint the job to print the error
		// report because we dont need the additonal content
		if (!is_null($source_job) && $source_job->complete() && !empty($source_job->_hipo_vars['errors'])) {
			$o = $this->getBackendOutputter();
			$o->setFormData('main_form', '');
			$o->openRaw();
			$source_job->paint($o);
			$o->closeRaw();
			$o->paint();
			return;
		}

		$onload = '';
		$delay  = FALSE;
		$auto_step = FALSE;
		$form_name = 'main_form';

		// if the source job is null it means that an abort finally finished
		// (we couldn't get here otherwise) so print that and get out of here
		if (is_null($source_job)) {
			$url = $_POST['aborting_hipo_url'];
			$onload = 'self.location = "'.$url.'";';
			$auto_step = TRUE;
			$percent_done = 100;
			$status_msg = 'HIPO Aborted';
		} else {
			$url = $this->getProcessURL($source_job->code_name);
			$auto_step = $source_job->autoStep();
			$percent_done = $source_job->percentDone();

			if ($source_job->complete()) {
				$onload = 'self.location = "'.$url.'";';
			} else {
				$delay = ($source_job->getRunningMode() == 'server');
				$onload = 'document.'.$form_name.'.submit();';
			}
			$status_msg = $source_job->complete() ? sprintf(translate('%s HIPO Complete'), $source_job->getHipoName()) : sprintf(translate('%s HIPO Progress'), $source_job->getHipoName());


		}

		$o = $this->getBackendOutputter();
		$o->setFormData($form_name, $url);

		$o->openRaw();
		?>
		<script language="JavaScript" type="text/javascript">
			function next_page() {
				<?php echo str_replace('\\', '\\\\', $onload); ?>
			}//end next_page()

			function send_abort() {
				if (ON_LOAD_TIME_OUT != null) {
					clearTimeout(ON_LOAD_TIME_OUT);
				}
				if (confirm(js_translate('Are you sure you want to cancel?'))) {

					document.<?php echo $form_name; ?>.cancel_hipo.value = "1";
					document.<?php echo $form_name; ?>.submit();
				} else {
					<?php
					if (!is_null($source_job) && $source_job->getRunningMode() == 'server') {
						?>next_page();<?php
					} else {
						?>alert(js_translate('This action cannot be performed'));<?php

					}
					?>
				}
			}//end send_abort()
		</script>
		<?php

		if ($auto_step) {
			if ($delay) {
				$o->addOnLoad('ON_LOAD_TIME_OUT = setTimeout("next_page();", 2000);');
			} else {
				$o->addOnLoad('ON_LOAD_TIME_OUT = setTimeout("next_page();", 100);');
			}
		}

		?>
		<div class="sq-standalone-wrapper sq-hipo-job">
			<table class="sq-standalone-table">
				<tr class="sq-standalone-header">
					<td colspan="2">
						<img src="<?php echo sq_web_path('lib').'/web/images/icons/hipo.png'; ?>" alt="Powered by Highly Intensive Processing Object (HIPO) Technology - Version 5" /> <strong>HIPO</strong>
					</td>
				</tr>
				<tr class="sq-standalone-body">
					<td colspan="2">
						<div class="sq-standalone-content sq-bg-blue">
							<?php HIPO_Job::paintProgressBar($percent_done, $status_msg, 'sq-hipo-header-progress-bar-label', 'sq-hipo-header-progress-bar-percent', 'sq-hipo-header-progress-bar-main', 'sq-hipo-header-progress-bar-done');?>
						</div>
						<?php
						$buttons_to_print = Array();

						// if we aren't waiting for an abort, print the progress stuff
						if (empty($_POST['aborting_hipo'])) {

							// Paint the source job, this will take care of any sub jobs if needed
							?>

							<div class="sq-standalone-content first-white">
								<?php
								$o->closeRaw();
								$source_job->paint($o);
								$o->openRaw();
								?>
							</div>

							<?php

							if (!$source_job->complete()) {
								// basically what happens here is that if cancel has already been pressed we increment the counter
								// this means that the cancel button only get's pressed once
								hidden_field('cancel_hipo', (empty($_POST['cancel_hipo'])) ? 0 : ((int) $_POST['cancel_hipo'] + 1));
								$allow_cancel = $source_job->_steps[$source_job->_hipo_vars['current_step']]['allow_cancel'];
								if ($allow_cancel) {
									$buttons_to_print['cancel'] = Array('value' => 'Cancel', 'action' => 'send_abort();');
								}
							}

							// print the NEXT or FINISH button if we are not auto stepping through the HIPO
							if (!$auto_step) {
								// they dont want us to refresh automatically, so we need to provide a manual NEXT button
								$button_text = ($source_job->complete()) ? 'Finish' : 'Next';
								$buttons_to_print['next'] = Array('value' => $button_text, 'action' => 'next_page();');
							}

						} else {
							// we are aborting from the server

							hidden_field('aborting_hipo',     $_POST['aborting_hipo']);
							hidden_field('aborting_hipo_url', $_POST['aborting_hipo_url']);

							// if the source job is null it means that an abort finally finished
							// (we couldn't get here otherwise) so print that and get out of here
							if (is_null($source_job)) {
								echo translate('Abort Complete');

								$button_text = 'OK';
							} else {
								echo translate('Aborting...');

								$button_text = 'Next';
							}

							// print the NEXT or FINISH button if we are not auto stepping through the HIPO
							if (!$auto_step) {
								// they dont want us to refresh automatically, so we need to provide a manual NEXT button
								$buttons_to_print['next'] = Array('value' => $button_text, 'action' => 'next_page();');
							}

						}// end if
						?>
						</div>
					</td>
				</tr>

				<?php
				if (count($buttons_to_print)) {
				?>
				<tr class="sq-standalone-footer">
						<?php

						// work out if we should print the 'Next >>' button
						$print_next = TRUE;
						if (isset($source_job->_hipo_vars['no_next']) && $source_job->_hipo_vars['no_next']) {
							$print_next = FALSE;
						}
						foreach ($buttons_to_print as $btn_code => $btn_data) {
							?><span class="<?php echo $btn_code; ?>"><?php
							// access key 'n' for next button
							if ($btn_code == 'next') {
								// if we should no proceed any further, then do not print the next button
								if ($print_next) {
									echo ('<td style="text-align: right">');
									normal_button($btn_code, $btn_data['value'], $btn_data['action'], 'class="sq-btn-blue sq-hipo-button-enabled '.$btn_code.'"accesskey="n"');
									echo ('</td>');
								}
							} else {
								echo ('<td style="text-align: left">');
								normal_button($btn_code, $btn_data['value'], $btn_data['action'], 'class="sq-hipo-button-enabled '.$btn_code.'"');
								echo ('</td>');
							}
							?></span><?php
						}
						?>
					</td>
				</tr>
				<?php
				}//end if
				?>
			</table>
		</div>
		<?php

		$o->closeRaw();
		$o->paint();

	}//end paintWeb()


	/**
	* Called by the daemon script to inform the hipo manager that it is about to start processing
	* This is needed so that we can check the the input from the web script and
	* returns an ACK or an error, indicating whether processing should start or not
	*
	* @param array	&$input_data	data sent from the web script
	* @param array	&$output_data	data to be sent back to the web script
	* @param string	$taskid			the id of the task on the server (used if we need to abort later on)
	*
	* @return boolean
	* @access public
	*/
	function prepareProcessServer(&$input_data, &$output_data, $taskid)
	{
		$this->_tmp['prepared_server'] = Array();

		if (empty($input_data['userid'])) {
			$output_data['result'] = 'error';
			$output_data['msg']    = 'userid not supplied on Input Data';
			return FALSE;
		}

		$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($input_data['userid']);
		if (is_null($user) || !$GLOBALS['SQ_SYSTEM']->setCurrentUser($user)) {
			$output_data['result'] = 'error';
			$output_data['msg']    = 'Unable to find User #'.$input_data['userid'].' to run as or unable to set them to the current user';
			return FALSE;
		}

		if (empty($input_data['source_code_name'])) {
			$output_data['result'] = 'error';
			$output_data['msg']    = 'source_code_name not supplied on Input Data';
			return FALSE;
		}

		$source_job = $this->getJob($input_data['source_code_name']);
		if (is_null($source_job)) {
			$output_data['result'] = 'error';
			$output_data['msg']    = 'Unable to find a source job with the codename "'.$input_data['source_code_name'].'"';
			return FALSE;
		}

		if (!$source_job->setRunningMode('server', $taskid)) {
			$output_data['result'] = 'error';
			$output_data['msg']    = 'Unable to set the running mode to server for "'.$source_job->code_name.'"';
			return FALSE;
		}

		$this->_tmp['prepared_server']['source_code_name'] = $source_job->code_name;

		pcntl_signal(SIGTERM, Array(&$this, 'serverSigHandler'));
		pcntl_signal(SIGINT,  Array(&$this, 'serverSigHandler'));

		$output_data['result'] = 'ack';
		return TRUE;

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
		// oracle + fork has some problems with commiting changes, have to force it
		if (MatrixDAL::getDbType() === 'oci') {
			MatrixDAL::setOciCommitOnSuccess(TRUE);
		}

		if (empty($this->_tmp['prepared_server'])) {
			return FALSE;
		}

		$source_job = $this->getJob($this->_tmp['prepared_server']['source_code_name']);
		if (is_null($source_job)) {
			trigger_localised_error('HIPO0057', sprintf(translate('Unable to find a source job with the codename "%s"'), $this->_tmp['prepared_server']['source_code_name']), E_USER_WARNING);
			return FALSE;
		}

		if ($source_job->uses_trans) {	// only encase in a transaction if source job wants it
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		}

		$this->_tmp['processing_server'] = TRUE;

		$ret_val = TRUE;
		while (!$source_job->complete() && $source_job->getRunningMode() == 'server') {
			if (!$source_job->process()) {
				$ret_val = FALSE;
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
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		}

		if (MatrixDAL::getDbType() === 'oci') {
			MatrixDAL::setOciCommitOnSuccess(FALSE);
		}

		return $ret_val;

	}//end processServer()


	/**
	* The handler for the any signals from the system
	*
	* @param int	$signo	the signal number
	*
	* @return void
	* @access public
	*/
	function serverSigHandler($signo)
	{
		switch ($signo) {
			case SIGINT:
			case SIGTERM:
				if (!empty($this->_tmp['processing_server'])) {
					$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db'); // just to make sure
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				}
				if (!empty($this->_tmp['prepared_server'])) {
					$source_job = $this->getJob($this->_tmp['prepared_server']['source_code_name']);
					if (!is_null($source_job)) $source_job->abort();
				}// end if
				exit();
			default:
				trigger_localised_error('HIPO0056', sprintf(translate('Signal %s not handled'), $signo), E_USER_NOTICE);
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
	* @return boolean
	* @access public
	*/
	function queueHipo($job_type, $vars=Array(), $on_complete_url='', $job_dir='')
	{
		if (empty($on_complete_url)) {
			$on_complete_url = $_SERVER['REQUEST_URI'];
		}
		if (empty($job_dir)) {
			$job_dir = SQ_SYSTEM_ROOT.'/core/hipo/jobs';
		}

		$this->_includeHipoJobLocales($job_dir);

		require_once $job_dir.'/'.$job_type.'.inc';
		$init_hipo = new $job_type();
		$init_hipo->setRunningVars($vars);

		// prepare() check the integrity of the the running vars
		if (!$init_hipo->prepare()) return FALSE;
		$init_hipo->_hipo_vars['job_dir'] = str_replace(SQ_SYSTEM_ROOT.'/', '', $job_dir);
		$job_threshold = $init_hipo->getThresholdPercentageRequired();

		// if the job has not explictly asked to be run in hipo mode
		// see if we can freestyle it
		if ($job_threshold > 0) {
			if (($this->threshold + $job_threshold) <= SQ_HIPO_TOTAL_THRESHOLD) {
				$this->threshold += $job_threshold;
				$init_hipo->_steps = $init_hipo->getInitialStepData();
				return $init_hipo->freestyle();
			}
		}

		$init_hipo->setOption('on_complete_url', $on_complete_url);

		// check if this hipo has already been queued up by this person
		// may happen when trying to perform the same task multiple times
		// in a single script execution
		$hipo_code = $init_hipo->getCodeName();
		if (isset($this->_queued_jobs[$GLOBALS['SQ_SYSTEM']->currentUserId()])) {
			if (in_array($hipo_code, $this->_queued_jobs[$GLOBALS['SQ_SYSTEM']->currentUserId()])) {
				return TRUE;
			}
		}

		if (!$init_hipo->initialise()) return FALSE;

		$this->_queued_jobs[$GLOBALS['SQ_SYSTEM']->currentUserId()][] = $hipo_code;
		return TRUE;

	}//end queueHipo()


	/**
	* Prepare the HIPO Jobs that have been queued for the current user
	*
	* @param string	$on_complete_url	the URL to redirect to once all HIPOs are done
	*									(can be left blank to use the current URL when the
	*									HIPO jobs were queued)
	*
	* @return mixed string|array
	* @access public
	*/
	function runQueuedJobs($on_complete_url='')
	{
		// note that we do not check rollback tables here because the
		// hipo job table doesn't need one and doesn't want one!!
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$db = MatrixDAL::getDb();
		try {
			$bind_vars['current_user'] = $GLOBALS['SQ_SYSTEM']->currentUserId();
			$queued_jobs = MatrixDAL::executeAssoc('core', 'getQueuedHipoJobs', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to get current queued jobs for user: '.$GLOBALS['SQ_SYSTEM']->currentUserId().' due to database error: '.$e->getMessage());
			return '';
		}

		if (empty($queued_jobs)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return '';
		}

		$job_codes = Array();

		for ($i = 0; $i < count($queued_jobs); $i++) {
			$job_data = $queued_jobs[$i];
			$job = $this->getJob($job_data['code_name']);
			$job_codes[] = $job_data['code_name'];

			if (isset($queued_jobs[($i+1)])) {
				$next_url = $this->getProcessURL($queued_jobs[($i+1)]['code_name']);
				$job->setOption('on_complete_url', $next_url);
				if (!$job->save()) {
					trigger_localised_error('HIPO0055', translate('HIPO Job failed to save'), E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return '';
				}
			} else if (!empty($on_complete_url)) {
				$job->setOption('on_complete_url', $on_complete_url);
				if (!$job->save()) {
					trigger_localised_error('HIPO0055', translate('HIPO Job failed to save'), E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return '';
				}
			}
		}

		try {
			// run query from DAL
			$bind_vars = Array(
							'running'		=> 1,
							'code_names'	=> $job_codes,
						 );
			$result = MatrixDAL::executeQuery('core', 'updateHipoRunningStatus', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to update HIPO jobs table due to database error: '.$e->getMessage());
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return $this->getProcessURL($queued_jobs[0]['code_name']);

	}//end runQueuedJobs()


	/**
	* Run the HIPO Job right now
	*
	* @param string	$job_type	the type of job (hipo_job_BLAH)
	* @param array	&$vars		HIPO vars to send to the job
	* @param string	$job_dir	where to find this hipo job include files
	*							(can be left blank to use default hipo job directory)
	*
	* @return array
	* @access public
	*/
	function freestyleHipo($job_type, &$vars, $job_dir='')
	{
		if (empty($job_dir)) {
			$job_dir = SQ_SYSTEM_ROOT.'/core/hipo/jobs';
		}

		require_once $job_dir.'/'.$job_type.'.inc';
		$init_hipo = new $job_type();
		$init_hipo->setRunningVars($vars);
		$init_hipo->_steps = $init_hipo->getInitialStepData();

		// prepare() check the integrity of the the running vars
		if (!$init_hipo->prepare()) return FALSE;
		set_error_handler(Array(&$init_hipo, '_errorHandler'));
			$init_hipo->freestyle();
		restore_error_handler();
		return $init_hipo->getErrors();

	}//end freestyleHipo()


	/**
	* Include the locales for a hipo job
	*
	* @param string	$job_dir	where to find this hipo job include files
	*
	* @return void
	* @access private
	*/
	function _includeHipoJobLocales($job_dir)
	{
		// if a default hipo job location, nothing to do
		if ($job_dir == SQ_SYSTEM_ROOT.'/core/hipo/jobs') {
			return;
		}

		// If this is not in the core/hipo/jobs it has to be under one of the packages
		// Because the hipo jobs are not assets the locale strings will be part of
		// the packages locales
		if (strpos($job_dir, str_replace('\\', '/', SQ_CORE_PACKAGE_PATH)) !== FALSE) {
			$GLOBALS['SQ_SYSTEM']->lm->includePackageStrings('__core__');
		} else {
			$stripped_dir = str_replace(str_replace('\\', '/', SQ_PACKAGES_PATH).'/', '', $job_dir);
			$package = substr($stripped_dir, 0, strpos($stripped_dir, '/'));
			$GLOBALS['SQ_SYSTEM']->lm->includePackageStrings($package);
		}


	}//end _includeHipoJobLocales()


	/**
	* Print out the backend interface for the Herder
	*
	* @param object	&$backend	reference to the backend object
	*
	* @return void
	* @access public
	*/
	function paintBackend(&$backend)
	{
		$hipo_admin      = $GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin();
		$current_user_id = $GLOBALS['SQ_SYSTEM']->currentUserId();

		$lock = $GLOBALS['SQ_SYSTEM']->getLockInfo(get_class_lower($this));
		$have_lock = (!empty($lock) && ($current_user_id == $lock['userid']));

		$o =& $backend->out;

		if (!empty($_POST['herder_posted'])) {

			if ($have_lock && !empty($_POST['process_form']) && !empty($_POST['force_kill'])) {
				$message_body = '';
				foreach ($_POST['force_kill'] as $source_code_name) {
					$source_job = $this->getJob($source_code_name);
					if (is_null($source_job)) continue;

					if ($hipo_admin) {
						// if the user is admin, temporarily pretend that
						// the current user is the owner of the job
						$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($source_job->userid);
						if (is_null($user)) continue;
						$GLOBALS['SQ_SYSTEM']->setCurrentUser($user);
						$source_job->abort();
						$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();
					} else if ($source_job->userid == $current_user_id) {
						$source_job->abort();
					} else {
						continue;
					}

					$message_body .= 'FORCE KILL '.$source_code_name."\n";

				}

				$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
				$msg_reps = Array(
								'hipo_code_name'	=> $source_code_name,
							);
				$message = $ms->newMessage(Array(), 'hipo.force_kill', $msg_reps);
				$message->subject = translate('HIPO Herder');
				$message->body    = translate('FORCE KILL %hipo_code_name%');
				$message->send();

			}//end if have lock, and forece kill is set

			// we want to remove some job records from the database, that is all
			if ($have_lock && !empty($_POST['process_form']) && !empty($_POST['remove_job'])) {
				$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db3');
				$db = $GLOBALS['SQ_SYSTEM']->db;

				$jobs_to_remove = Array();
				foreach ($_POST['remove_job'] as $code_name) {
					$jobs_to_remove[] = MatrixDAL::quote($code_name);
				}
				$sql = 'DELETE FROM sq_hipo_job
						WHERE code_name IN ('.implode(',', $jobs_to_remove).')';
				try {
					$query = MatrixDAL::preparePdoQuery($sql);
					$result = MatrixDAL::execPdoQuery($query);
				} catch (Exception $e) {
					throw new Exception('Unable to delete HIPO job(s) due to database error: '.$e->getMessage());
				}
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

				// log each job that was cleared
				$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
				foreach ($_POST['remove_job'] as $code_name) {
					$msg_reps = Array(
									'hipo_code_name'	=> $code_name,
								);
					$message = $ms->newMessage(Array(), 'hipo.orphan_kill', $msg_reps);
                    $message->subject = translate('HIPO Herder');
                    $message->body    = translate('ORPHAN KILL %hipo_code_name%');
					$message->send();
				}

			}// end if have lock, and removing orphaned jobs

			if (!empty($_POST['sq_lock_release']) || !empty($_POST['sq_lock_release_manual'])) {
				$GLOBALS['SQ_SYSTEM']->releaseLock(get_class_lower($this));
			}

			// if there is no lock currently and we want it
			if (!empty($_POST['sq_lock_acquire'])) {
				$GLOBALS['SQ_SYSTEM']->acquireLock(get_class_lower($this));
			}

			// just to make sure
			$lock      = $GLOBALS['SQ_SYSTEM']->getLockInfo(get_class_lower($this));
			$have_lock = (!empty($lock) && ($current_user_id == $lock['userid']));

		}//end if not empty herder posted

		require_once SQ_FUDGE_PATH.'/general/datetime.inc';
		$o->setHeading('HIPO Herder', sq_get_icon($o->filesPath('/images/icons/header/hipo_herder.png'), '', '', 'HIPO Herder'));
		$o->setPageTitle('HIPO Herder');
		$o->addHiddenField('herder_posted', '1');

		// lets tell the user if the asset is locked for editing
		// or if they can lock it to edit it

		$o->openRaw();
		?>
			<div class="sq-backend-section-div-wrapper section custom first">
				<div class="sq-backend-section-div">
					<div id="sq-aquire-locks-wrapper" class="clearfix">
						<?php
						echo '<div class="sq-locks-button-wrapper">';
							if (!empty($lock)) {
								if ($have_lock) {
									submit_button('sq_lock_release_manual', translate('Release Lock(s)'), 'set_hidden_field("process_form", "0");', 'accesskey="r" class="sq-form-field sq-btn-large sq-btn-blue sq-lock-button release"');

								}
							} else {
								submit_button('sq_lock_acquire', translate('Acquire Lock(s)'), 'set_hidden_field("process_form", "0");', 'accesskey="a" class="sq-form-field sq-btn-large sq-btn-blue sq-lock-button acquire"');

							}
						echo '</div>';
						echo '<p class="sq-lock-message">';
							if (!empty($lock)) {
								// this asset is currently locked
								// so display message to the user
								$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($lock['userid']);
								$now = time();

								require_once SQ_FUDGE_PATH.'/general/datetime.inc';
								$expires_in = easy_time_total(($lock['expires'] - $now), TRUE);
								if (!$expires_in) $expires_in = '1 second';
								$expires_in = sprintf(translate('The lock is due to expire in %s'), $expires_in);

								if ($have_lock) {
									echo sprintf(translate('Press the <span class="sq-backend-button-fake">%1$s</span> button to release the locks you hold on "<strong>%2$s</strong>".'), translate('Release Lock(s)'), translate('HIPO Herder'));



								}
								if ($user->id != $GLOBALS['SQ_SYSTEM']->currentUserId()) {
									echo sprintf(translate('Held by user "%s"'), htmlspecialchars($user->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)).'. ';

								}
								echo ' '.$expires_in.'.';

							} else {
								echo sprintf(translate('Press the <span class="sq-backend-button-fake">%1$s</span> button to lock "<strong>%2$s</strong>" for editing.'), translate('Acquire Lock(s)'), translate('HIPO Herder'));



							}
						echo '</p>';
						?>
					</div><!--#sq-aquire-locks-wrapper-->
				</div><!--.sq-backend-section-div-->
			</div><!--.sq-backend-section-div-wrapper-->
		<?php
		$o->closeRaw();

		$o->openSection(translate('Current HIPO Jobs'));

			$o->openField('', 'wide_col');

			$source_jobs = Array();

			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db3');
			$db = MatrixDAL::getDb();
			$sql = 'SELECT source_code_name, code_name, job_type
					FROM sq_hipo_job';

			// regular users can only see own HIPO
			if (!$hipo_admin) {
				$sql .= ' WHERE userid = :current_user_id';
			}
			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				if (!$hipo_admin) {
					MatrixDAL::bindValueToPdo($query, 'current_user_id', $current_user_id);
				}
				$result = MatrixDAL::executePdoAssoc($query);
			} catch (Exception $e) {
				throw new Exception('Unable to get HIPO jobs for user: '.$current_user_id.' due to database error: '.$e->getMessage());
			}
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			foreach ($result as $key => $row) {
				$source_code_name = ($row['source_code_name'] == $row['code_name']) ? '' : $row['source_code_name'];
				if (!isset($source_jobs[$source_code_name])) {
					$source_jobs[$source_code_name] = Array();
				}
				$source_jobs[$source_code_name][$row['code_name']] = $row['job_type'];
			}

			if (empty($source_jobs[''])) {
				echo translate('There are currently no HIPO Jobs on the system');


			} else {
				?>
				<table class="sq-backend-table">
					<tr>
						<?php
							if ($have_lock) {
								?>
									<th class="sq-backend-table-header" style="width: 30px;"><?php echo translate('Resume'); ?></th>

								<?php
							}
						?>
						<th class="sq-backend-table-header"><?php echo translate('Job Type'); ?></th>

						<th class="sq-backend-table-header"><?php echo translate('Owner'); ?></th>

						<th class="sq-backend-table-header"><?php echo translate('Last Updated'); ?></th>

						<th class="sq-backend-table-header" style="text-align: center;"><?php echo translate('Percent Complete'); ?></th>

				<?php
					if ($have_lock) {
						?>
							<th class="sq-backend-table-header" style="text-align: center;"><?php echo translate('Delete?'); ?></th>

						<?php
					} // end if have lock
				?>
					</tr>
				<?php
				$now = time();
				$current_user_id = $GLOBALS['SQ_SYSTEM']->currentUserId();
				foreach ($source_jobs[''] as $source_code_name => $source_job_type) {
					$source_job = $this->getJob($source_code_name);
					if (is_null($source_job)) continue;
				?>
					<tr>
						<?php
							if ($have_lock) {
							?>
							<td class="sq-backend-table-cell" style="text-align: center;">
								<?php
									// if the current user is the owner of the hipo job or the current user is a sysadmin or a root
									// then display the resume button
									// also once the user clicked on the resume button, the hipo job will be resumed in a popup window
									// and the resume button will be hidden
									if (($source_job->userid === $current_user_id) || $GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
									?>
										<a href="./?SQ_ACTION=hipo&hipo_source_code_name=<?php echo $source_code_name; ?>&return_to_herder=1" style="text-decoration: none;">
											<img style="border: none;" src="<?php echo sq_web_path('lib'); ?>/web/images/hipo_resume.png" title="<?php echo translate('Resume this HIPO Job'); ?>" alt="Resume <?php echo $source_code_name; ?>" />

										</a>
										&nbsp;
										<a href="#" onclick="var result = window.open('./?SQ_ACTION=hipo&hipo_source_code_name=<?php echo $source_code_name; ?>', 'hipo_job<?php echo $source_code_name; ?>', 'width=650,height=400,scrollbars=1,toolbar=0,menubar=0,location=0,resizable=1');return false;" style="text-decoration: none;" >
											<img style="border: none;" src="<?php echo sq_web_path('lib'); ?>/web/images/hipo_resume_popup.png" title="<?php echo translate('Resume this HIPO Job in a new window'); ?>" alt="Resume <?php echo $source_code_name; ?>" />

										</a>
									<?php
									}
								?>
							</td>
							<?php
							}
						?>
						<td class="sq-backend-table-cell">
							<?php echo ucwords(str_replace('_', ' ', $source_job_type)); ?>
						</td>
						<td class="sq-backend-table-cell">
						<?php
							$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($source_job->userid);
							echo $user->name.' (Id #'.$user->id.')';
						?>
						</td>
						<td class="sq-backend-table-cell">
							<?php echo easy_time_total($source_job->last_updated - $now, TRUE); ?>

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
						foreach ($source_jobs[$source_code_name] as $code_name => $job_type) {
							$job = $this->getJob($code_name);
							if (is_null($job)) continue;
						?>
							<tr>
								<?php
								if ($have_lock) {
								?>
								<td class="sq-backend-table-cell">
									&nbsp;
								</td>
								<?php
								}
								?>
								<td class="sq-backend-table-cell">
									<img src="<?php echo sq_web_path('lib'); ?>/web/images/tree/branch.gif" width="16" height="16" alt="branch">&nbsp;
									<?php echo ucwords(str_replace('_', ' ', $job_type)); ?>
								</td>
								<td class="sq-backend-table-cell">
									&nbsp;
								</td>
								<td class="sq-backend-table-cell">
									<?php echo easy_time_total($source_job->last_updated - $now, TRUE); ?>
								</td>
								<td class="sq-backend-table-cell" style="text-align: center;">
									<?php echo $job->percentDone(); ?>%
								</td>
								<?php
								if ($have_lock) {
								?>
								<td class="sq-backend-table-cell">
									&nbsp;
								</td>
								<?php
								}
								?>
							</tr>
						<?php

						}// end foreach
					}// end if
				}//end foreach
				?>

				</table>
				<?php
			}//end else


		$o->closeSection();

		// get jobs that have a non-existant source job
		$o->openSection(translate('Orphaned HIPO Jobs'));

			$o->openField('', 'wide_col');

				$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db3');
				$db = MatrixDAL::getDb();

				$sql = 'SELECT code_name, source_code_name, job_type, userid, last_updated
						FROM sq_hipo_job
						WHERE source_code_name NOT IN (SELECT code_name FROM sq_hipo_job)';

				// regular users can only see own HIPO
				if (!$hipo_admin) {
					$sql .= ' AND userid = :current_user_id';
				}

				try {
					$query = MatrixDAL::preparePdoQuery($sql);
					if (!$hipo_admin) {
						MatrixDAL::bindValueToPdo($query, 'current_user_id', $current_user_id);
					}
					$orphaned_jobs = MatrixDAL::executePdoAssoc($query);
				} catch (Exception $e) {
					throw new Exception('Unable to get orphaned jobs for user: '.$current_user_id.' due to database error: '.$e->getMessage());
				}

				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

				// no orphaned jobs, yay
				if (empty($orphaned_jobs)) {
					echo translate('There are currently no orphaned HIPO Jobs on the system');

				} else {
					?>
					<table class="sq-backend-table">
						<tr>
							<td class="sq-backend-table-header"><?php echo translate('Job Type'); ?></td>

							<td class="sq-backend-table-header"><?php echo translate('Parent Job Type'); ?></td>

							<td class="sq-backend-table-header"><?php echo translate('Owner'); ?></td>

							<td class="sq-backend-table-header"><?php echo translate('Last Updated'); ?></td>

					<?php
					if ($have_lock) {
						?>
							<td class="sq-backend-table-header" style="text-align: center;"><?php echo translate('Delete?'); ?></td>

						<?php
					} // end if have lock
					?>
						</tr>
					<?php

					// display some information on each orphaned job. we only use the info
					// we pulled earlier because calling something like getJob() may result in
					// attempt to abort the job in hipo_job::load(). we don't really want any
					// error messages to be displayed, just a way to remove this from the db
					foreach ($orphaned_jobs as $job_info) {
						?>
						<tr>
							<td><?php echo ucwords(str_replace('_', ' ', $job_info['job_type'])); ?></td>
							<td><?php
								$parent_job_type = reset(explode('-', $job_info['source_code_name']));
								echo ucwords(str_replace('_', ' ', $parent_job_type));
							?></td>
							<td><?php
								$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($job_info['userid']);
								echo $user->name.' (Id #'.$user->id.')';
							?></td>
							<td><?php
								$now = time();
								echo easy_time_total(strtotime($job_info['last_updated']) - $now, TRUE);
							?></td>
							<?php
							if ($have_lock) {
								?>
								<td class="sq-backend-table-cell" style="text-align: center;">
									<?php check_box('remove_job[]', $job_info['code_name']); ?>
								</td>
								<?php
							}// end if have_lock
						?>
						</tr>
						<?php
					}
					?>
					</table>
					<?php
				}//end else

			$o->closeField();
		$o->closeSection();

		if ($have_lock) $o->commitButton('', TRUE);

	}//end paintBackend()


	/**
	* Gets the jobs with percentage done for specified user
	*
	* @param int	$userid	the user id
	*
	* @return array
	* @access public
	*/
	function getJobsForUser($userid)
	{
		try {
			$bind_vars['userid'] = $userid;
			$jobsforuser = MatrixDAL::executeAssoc('core', 'selectJobsForUser', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Failed to get HIPO jobs for user due to database error: '.$e->getMessage());
		}

		// now get the percentage done for each job code we have
		for ($i = 0; $i < count($jobsforuser); $i++) {
			$source_job = $this->getJob($jobsforuser[$i]['code_name']);
			$jobsforuser[$i]['percent_done'] = $source_job->percentDone();
		}
		return $jobsforuser;

	}//end getJobsForUser()


}//end class

?>
