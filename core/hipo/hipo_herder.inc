<?php

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) Herder (Manager ;P)
*
* @author  Blair Robertson <brobertson@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
* @subpackage HIPO
*/
class HIPO_Herder extends Resolve_Object
{

	/**
	* @var array of jobs that have been loaded
	*/
	var $_jobs = Array();

	/**
	* Constructor
	*
	*/
	function HIPO_Herder()
	{
	}//end constructor


	/**
	* Returns tha stack of jobs that are running for passed user and where initiated because
	* of the hipo job represented by source_code_name
	*
	* @param string	$code_name	the codename of the job
	* @param string	$job_type	the type of job (supply if known for speedier access)
	*
	* @return object HIPO_Job|null	returns the requested hipo job OR null on error
	* @access public
	*/
	function &getJob($code_name, $job_type='')
	{
		if (empty($this->_jobs[$code_name])) {
			if (!$job_type) {
				$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
				$db = &$GLOBALS['SQ_SYSTEM']->db;
				$sql = 'SELECT job_type
						FROM '.SQ_TABLE_PREFIX.'hipo_job
						WHERE code_name = '.$db->quote($code_name);
				$job_type = $db->getOne($sql);
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

				if (DB::isError($job_type)) {
					trigger_error($job_type->getMessage().'<br/>'.$job_type->getUserInfo(), E_USER_WARNING);
					$null = null; // needed for ref
					return $null;
				}
				// no job exists ?
				if (!$job_type) {
					$null = null; // needed for ref
					return $null;
				}

			}// end if

			require_once SQ_SYSTEM_ROOT.'/core/hipo/jobs/'.$job_type.'.inc';
			$this->_jobs[$code_name] = new $job_type($code_name);

			// something buggered up, not our problem
			if (!$this->_jobs[$code_name]->code_name) {
				$null = null; // needed for ref
				return $null;
			}

		}// endif
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
		if (!empty($_POST['cancel_hipo'])) { // && $_POST['cancel_hipo'] == 1) {
			if ($source_job->abort()) { 
				$_POST['aborting_hipo'] = $source_job->code_name;
				$_POST['aborting_hipo_url'] = $source_job->getOption('on_complete_url');
			}
			return true;
		}

		if (!$source_job->process()) {
			trigger_error('HIPO failed to process', E_USER_WARNING);
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

		$onload = '';
		$delay  = false;
		$auto_step = false;


		// if the source job is null it means that an abort finally finished
		// (we couldn't get here otherwise) so print that and get out of here
		if (is_null($source_job)) {
			$form_name = 'HIPO_abort';
			$url = $_POST['aborting_hipo_url'];
			$onload = 'self.location = "'.$url.'";';
			$auto_step = true;
		} else {

			$form_name = 'HIPO_'.$source_job->code_name;
			$url = $this->getProcessURL($source_job->code_name);
			$auto_step = $source_job->auto_step();

			if ($source_job->complete()) {
				$onload = 'self.location = "'.$url.'";';
			} else {
				$delay = ($source_job->getRunningMode() == 'server');
				$onload = 'document.'.$form_name.'.submit();';
			}

		} // end if

		?>
		<html>
			<head>
				<style>
					pre {
						color: #000000;
					}
					.sq-hipo-normal, p, td {
						font-size: 10px;
						font-family: Verdana;
						color: #FFFFFF;
					}
					.sq-hipo-small {
						font-size: 8px;
						font-family: Verdana;
						color: #FFFFFF;
					}
				</style>
				<script language="JavaScript" type="text/javascript">
					var ON_LOAD_TIME_OUT = null;
					function init() {
				<?php
				if ($auto_step) {
					if ($delay)  {
				?>
						ON_LOAD_TIME_OUT = setTimeout("next_page();", 2000);
				<?php
					} else {
				?>
							next_page();
				<?php
					}// end if
				}// end if
				?>
					}// end init()

					function next_page() {
						//if (confirm('Refresh')) {
						<?php echo $onload; ?>
						//}
					}// end next_page()

					function send_abort() {
						if (ON_LOAD_TIME_OUT != null) {
							clearTimeout(ON_LOAD_TIME_OUT);
						}
						if (confirm("Are you sure you want to cancel?")) {
							document.<?php echo $form_name; ?>.cancel_hipo.value = "1";
							document.<?php echo $form_name; ?>.submit();

						} else {
							next_page();
						}
					}// end send_abort();
				</script>
			</head>
			<body bgcolor="#FFFFFF" onLoad="javascript: init();" class="sq-hipo-normal">
				<form name="<?php echo $form_name; ?>" method="post" action="<?php echo $url; ?>">
				<table border="0" cellspacing="0" cellpadding="0" width="100%" height="100%">
					<tr>
						<td valign="middle" align="center">
						<table border="0" cellspacing="2" cellpadding="0" width="90%" bgcolor="#000000">
							<tr>
								<td valign="top" bgcolor="#E0E0E0" nowrap>
								<table>
									<tr>
										<td>
<pre style="color:#412F49; font-size:11px;">
<b>
            _,..,_ (, )
         .,'      `,./
       .' :`.----.': `,
      :   : ^    ^ :   ;
     :   :  6    6  :   ;
     :   :          :   ;
     :   :    __    :   ;
      :   `:'.--.`:'   ;
       `.  : o  o :  .'
        :   `----'   :
        : .  :'`:  . :
        `.:.'    `.:.'
   Powered by HIPO Technology
    ( packed with ::Fudge:: )
</b>
</pre>
										</td>
									</tr>
								</table>
								</td>
								<td valign="top" width="100%" bgcolor="#735B7D">
								<table border="0" cellspacing="2" cellpadding="2" width="100%" height="100%">
									<tr>
										<td>
									<?php

									// if we aren't waiting for an abort, print the progress stuff
									if (empty($_POST['aborting_hipo'])) {

										// Paint the source job, this will take care of any sub jobs if needed
										$source_job->paint();

										$buttons_to_print = Array();
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

										if (count($buttons_to_print)) {
											?>
											<p>
											<table border="0" width="100%" cellspacing="2" cellpadding="1">
												<tr>
													<td width="100%">&nbsp;</td>
												<?php
												foreach ($buttons_to_print as $btn_code => $btn_data) {
													?><td valign="top" align="center"><?php
													normal_button($btn_code, $btn_data['value'], $btn_data['action']);
													?></td><?php
												}
												?>
												</tr>
											</table>
											</p>
											<?php
										}// end if

									// we are aborting from the server
									} else {

										hidden_field('aborting_hipo',     $_POST['aborting_hipo']);
										hidden_field('aborting_hipo_url', $_POST['aborting_hipo_url']);

										// if the source job is null it means that an abort finally finished
										// (we couldn't get here otherwise) so print that and get out of here
										if (is_null($source_job)) {
											echo 'Abort Complete';
										} else {
											echo 'Aborting...';
										}

									}// end if
									?>
										</td>
									</tr>
								</table>
								</td>
							</tr>
						</table>
						</td>
					</tr>
				</table>
				</form>
			</body>
		</html>
		<?php


	}//end paintWeb()


	/**
	* Called by the daemon script to inform the hipo manager that it is about to start processing
	* This is needed so that we can check the the input from the web script and
	* return an ACK or an error to it before we start processing
	*
	* @param object XML_Tree	$input_root		root node of the data sent from the web script
	* @param object XML_Tree	$output_root	root node of the data to be sent back to the web script
	* @param string				$taskid			the id of the task on the server (used if we need to abort)
	*
	* @return boolean	indicating whether processing should start or not
	* @access public
	*/
	function prepareProcessServer(&$input_root, &$output_root, $taskid)
	{
		$this->_tmp['prepared_server'] = Array();

		if ($input_root->name != 'hipo_job') {
			$output_root->name = 'error';
			$output_root->content = 'Input Root Node not named "hipo_job"';
			return false;
		}

		if (empty($input_root->attributes['userid'])) {
			$output_root->name    = 'error';
			$output_root->content = 'userid not supplied on Input Root Node';
			return false;
		}

		$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($input_root->attributes['userid']);
		if (is_null($user) || !$GLOBALS['SQ_SYSTEM']->setCurrentUser($user)) {
			$output_root->name    = 'error';
			$output_root->content = 'Unable to find User #'.$userid.' to run as or unable to set them to the current user';
			return false;
		}

		if (empty($input_root->attributes['source_code_name'])) {
			$output_root->name    = 'error';
			$output_root->content = 'source_code_name not supplied on Input Root Node';
			return false;
		}

		$source_job = &$this->getJob($input_root->attributes['source_code_name']);
		if (is_null($source_job)) {
			$output_root->name    = 'error';
			$output_root->content = 'Unable to find a source job with the codename "'.$input_root->attributes['source_code_name'].'"';
			return false;
		}

		if (!$source_job->setRunningMode('server', $taskid)) {
			$output_root->name    = 'error';
			$output_root->content = 'Unable to set the running mode to server for "'.$source_job->code_name.'"';
			return false;
		}

		unset($source_job);
		unset($this->_jobs[$input_root->attributes['source_code_name']]);
		$source_job = &$this->getJob($input_root->attributes['source_code_name']);
		pre_echo("AFTER SET : ".print_r($source_job->_hipo_vars, true));

		$this->_tmp['prepared_server']['source_code_name'] = $source_job->code_name;

		pcntl_signal(SIGTERM, Array(&$this, 'sigHandler'));

		$output_root->name = 'ack';
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
	* @access public
	*/
	function processServer()
	{
		if (empty($this->_tmp['prepared_server'])) {
			return false;
		}

		$source_job = &$this->getJob($this->_tmp['prepared_server']['source_code_name']);
		if (is_null($source_job)) {
			trigger_error('Unable to find a source job with the codename "'.$input_root->attributes['source_code_name'].'"', E_USER_WARNING);
			return false;
		}


		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
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

		if ($ret_val) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return true;
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

	}//end processServer()


	/**
	* The handler for the any signals from the system
	*
	* @return int		$signo	the signal number
	* @access public
	*/
	function sigHandler($signo)
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
				break;
			default:
				trigger_error('Signal '.$signo.' not handled', E_USER_NOTICE);
				break;
		}// end switch

	}// end sigHandler()


}//end class

?>