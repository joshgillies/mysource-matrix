<?php

// include the hipo system config file
require_once SQ_DATA_PATH.'/private/conf/hipo.inc';

/**
* Highly Intensive Processing Object (HIPO)
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
* @subpackage HIPO
*/
class HIPO_Job extends Resolve_Object
{

	/**
	* The name of this HIPO job (should not be modified by code)
	* @var string
	*/
	var $code_name = '';

	/**
	* The code name of the hipo job that this job is being run under
	* @var string
	*/
	var $_source_code_name = '';

	/**
	* The name of this HIPO job (should not be modified by code)
	* @var string
	*/
	var $_hipo_name = '';

	/**
	* Array that stores some variables that the HIPO steps use
	* @var array()
	*/
	var $_hipo_vars = Array(
						'current_step'		=> 0,
						'percent_done'		=> 0,
						'initialise_mode'	=> '',		// the mode that this job was initialised() in, used to decide what to do when we are completed
						'running_mode'		=> '',		// the mode that is currently doing the processing
						'aborted'			=> false,	// whether we have been aborted or not
						'message'			=> '',
						'errors'			=> Array(),
					);

	/**
	* Some global options that affect how the HIPO does things
	* @var array()
	*/
	var $_options = Array(
						'on_complete_url'	=> '',
						'auto_complete'		=> true,
					);

	/**
	* Array that stores any running variables that any of the processing functions use
	* @var array()
	*/
	var $_running_vars = Array();

	/**
	* Flag that represents if the HIPO has completed all its steps
	* @var boolean
	*/
	var $_complete = false;

	/**
	* Flag that represents what mode the HIPO is being run in
	* @var string
	*/
	var $_current_mode = 'web';

	/**
	* Zero Indexed Array of steps to perform in this HIPO
	* Form :
	* Array(
	*		Array(
	*			'name'			=> [name of the step],
	*		[
	*			'function_call'	=> Array(
	*									'paint_function'   => 'paintSomething(&$step_data, $prefix)',
	*									'process_function' => 'processSomething(&$step_data, $prefix)',
	*								),
	*		|
	*			'hipo_job'		=> Array(
	*									'init_details_function' => 'getJobDetails(&$job_type, &$running_vars, &$options)',
	*								),
	*		]
	*			'running_mode'	   => [web|server],
	*			'auto_step'		   => [true|false],
	*			'percent_done'	   => 0,
	*			'complete'		   => false,
	*			'message'		   => '',
	*		),
	*		...
	* );
	*
	*
	*
	* @var array()
	*/
	var $_steps = Array();


	/**
	* Constructor
	*
	* @param string	$code_name		a unique code_name the HIPO
	* @param string	$current_mode	the mode this HIPO is being displayed in
	*								(web = browser, server = HIPO server)
	*/
	function HIPO_Job($code_name='')
	{
		$this->_current_mode = (SQ_PHP_CLI) ? 'server' : 'web';
		if (!empty($code_name)) $this->load($code_name);

	}//end constructor


	/**
	* Returns a unique code_name the HIPO can use to ensure it is not being run twice
	*
	* @return string
	* @access private
	*/
	function _getCodeName()
	{
		return strtolower(get_class($this));

	}//end _getCodeName()


	/**
	* Returns a unique code_name the HIPO can use to ensure it is not being run twice
	*
	* @return boolean
	* @access public
	*/
	function complete()
	{
		return ($this->_hipo_vars['current_step'] > count($this->_steps) || $this->_hipo_vars['aborted']);
	}//end complete()


	/**
	* How much of it's work has this HIPO_Job done
	*
	* @return int
	* @access public
	*/
	function percentDone()
	{
		return (int) $this->_hipo_vars['percent_done'];
	}//end percentDone()


	/**
	* Return the URL that will continue the processing of the HIPO
	*
	* If the HIPO is finsihed, the URL returned will be the on_complete_url in the
	* _options array of the HIPO so that the HIPO can redirect back to where it started
	*
	* @return string
	* @access public
	*/
	function getProcessURL()
	{
		if ($this->complete()) {
			// this HIPO is finsihed
			return $this->_options['on_complete_url'];
		} else {
			return $_SERVER['PHP_SELF'].'?SQ_ACTION=hipo&hipo_source_code_name='.$this->_source_code_name;
		}

	}//end getProcessURL()


	/**
	* Returns the vars that the HIPO steps use
	*
	* @return Array()
	* @access public
	*/
	function getRunningVars()
	{
		return $this->_running_vars;

	}//end getRunningVars()


	/**
	* Resets the vars that the HIPO steps use
	*
	* @param Array	$vars
	*
	* @return void
	* @access public
	*/
	function setRunningVars($vars)
	{
		$this->_running_vars = $vars;

	}//end setRunningVars()


	/**
	* Set one of the global options for the HIPO
	*
	* @param string	$name		name of the option
	* @param string	$value		new value to assign it
	*
	* @return void
	* @access public
	*/
	function setOption($name, $value)
	{
		if (!isset($this->_options[$name])) {
			trigger_error('HIPO Option "'.$name.'" not known', E_USER_WARNING);
			return;
		}
		$this->_options[$name] = $value;

	}// end setOption()


	/**
	* Add an error to the HIPO job to be reported at the end
	*
	* This function will automatically log the time of the error for the report
	*
	* @param string		$error		the text of the error to report
	* @param boolean	$warning	set to TRUE if this is just a warning, not an error
	*
	* @return void
	* @access protected
	*/
	function _addError($error, $warning=false)
	{
		$error = trim($error);
		if (!empty($error)) $this->_hipo_vars['errors'][] = Array(
																'time'    => time(),
																'message' => $error,
																'warning' => $warning,
																);

	}//end _addError()


	/**
	* Set up vars and database information that the HIPO will need to run
	*
	* This function should be run before any painting or processing of the HIPO. It will
	* write the initial database entry for the HIPO so that the vars needed by the HIPO
	* are available on successive reloads.
	*
	* @param string	$source_code_name	the code name of another job that wants to use this job
	*
	* @return string	returns the code_name for the new job
	* @access public
	*/
	function initialise($source_code_name=null)
	{
		$this->_hipo_vars['initialise_mode'] = $this->_current_mode;
		$this->_hipo_vars['running_mode']    = $this->_current_mode;

		// if we aren't using the server make sure that all the steps are changed accordingly
		if (!SQ_HIPO_USE_SERVER) {
			for($i = 0; $i < count($this->_steps); $i++) {
				$this->_steps[$i]['running_mode'] = 'web';
			}
		}

		$steps     = serialize($this->_steps);
		$vars      = serialize($this->_running_vars);
		$options   = serialize($this->_options);
		$hipo_vars = serialize($this->_hipo_vars);
		$code_name = $this->_getCodeName();
		$message   = '';

		if (empty($source_code_name)) $source_code_name = $code_name;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db  = &$GLOBALS['SQ_SYSTEM']->db;
		$sql = 'INSERT INTO '.SQ_TABLE_PREFIX.'hipo_job
				(code_name, job_type, userid, source_code_name, hipo_vars, options, steps, running_vars)
				VALUES ('.$db->quote($code_name).',
						'.$db->quote(get_class($this)).',
						'.$db->quote($GLOBALS['SQ_SYSTEM']->currentUserId()).',
						'.$db->quote($source_code_name).',
						'.$db->quote($hipo_vars).',
						'.$db->quote($options).',
						'.$db->quote($steps).',
						'.$db->quote($vars).'
						)';
		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return '';
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return $code_name;

	}//end initialise()


	/**
	* Load the HIPO vars from the database
	*
	* @param string	$code_name	a unique code_name the HIPO
	*
	* @return boolean
	* @access public
	*/
	function load($code_name)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT job_type, userid, source_code_name, hipo_vars, options, steps, running_vars
				FROM '.SQ_TABLE_PREFIX.'hipo_job
				WHERE code_name = '.$db->quote($code_name);

		$result = $db->getRow($sql);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		if (empty($result)) return false;

		if ($result['job_type'] != get_class($this)) {
			trigger_error('Attempted to load "'.$result['job_type'].'" job into a "'.get_class($this).'" object', E_USER_WARNING);
			return false;
		}

		if ((int) $result['userid'] != $GLOBALS['SQ_SYSTEM']->currentUserId()) {
			trigger_error('Attempted to load "'.$result['job_type'].'" job that you do not own', E_USER_WARNING);
			return false;
		}

		$this->code_name         = $code_name;
		$this->_source_code_name = $result['source_code_name'];
		$this->_hipo_vars        = unserialize($result['hipo_vars']);
		$this->_options          = unserialize($result['options']);
		$this->_steps            = unserialize($result['steps']);
		$this->_running_vars     = unserialize($result['running_vars']);

		return true;

	}//end load()


	/**
	* Save the HIPO vars to the database OR delete from database if complete
	*
	* @return boolean
	* @access private
	*/
	function _save()
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		// if we have called save and we aren't the running version, bugger off
		if ($this->_current_mode != $this->_hipo_vars['running_mode']) return true;

		// First let's make sure that everything is in order with the hipo vars and our complete status
		$step = $this->_hipo_vars['current_step'];
		if ($this->_steps[$step]['complete']) $this->_hipo_vars['current_step']++;

		$prev_step_per = ($step / count($this->_steps)) * 100;
		$this_step_per = ($this->complete()) ? 0 : ((1 / count($this->_steps)) * $this->_steps[$step]['percent_done']);
		$this->_hipo_vars['percent_done'] = round($prev_step_per + $this_step_per, 2);

		if ($this->_steps[$step]['complete']) $this->_hipo_vars['current_step']++;

		// Second let's decide, depening on the circumstances, whether to delete or update
		$delete = false;
		if ($this->complete()) {
			// if we are running from the web, we can delete the DB entry
			// because we are now done with it
			if ($this->_current_mode == 'web') {
				$delete = true;

			// running from the server/cmdline
			} else {

				// The idea here is that if we are the top hipo job AND we were initialised in a web script
				// then we need to leave the DB entry so that the refreshing we script can get the data one last time
				if ($this->code_name != $this->_source_code_name || $this->_hipo_vars['initialise_mode'] != 'web') {
					$delete = true;
				}

			}// endif

		}// end if

		// now we just do what we are told
		if ($delete) {
			$sql = 'DELETE FROM '.SQ_TABLE_PREFIX.'hipo_job
					WHERE code_name = '.$db->quote($this->code_name);

		} else {
			$sql = 'UPDATE '.SQ_TABLE_PREFIX.'hipo_job
					SET hipo_vars    = '.$db->quote(serialize($this->_hipo_vars)).',
						options      = '.$db->quote(serialize($this->_options)).',
						steps        = '.$db->quote(serialize($this->_steps)).',
						running_vars = '.$db->quote(serialize($this->_running_vars)).'
					WHERE code_name  = '.$db->quote($this->code_name);
		}
		$result = $db->query($sql);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return true;

	}//end _save()


	/**
	* Cancel the HIPO and cleanup
	*
	* @return boolean
	* @access public
	*/
	function abort()
	{
		$this->_hipo_vars['aborted'] = true;
		return $this->_save();
	}//end abort()


	/**
	* Paint information about the process of the HIPO
	*
	* This function calls the paint function for the step currently being run and also outputs
	* a progress bar to indicate to the user where they are in the HIPO process
	*
	* @return void
	* @access public
	*/
	function paint()
	{
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		$step = $this->_hipo_vars['current_step'];

		$form_name = 'HIPO_'.$this->code_name;
		$url = $this->getProcessURL();
		$auto_step = false;

		if (!isset($this->_steps[$step])) {
			// this step doesnt exist - probably reached the end of the HIPO
			if ($this->complete()) $status_msg = $this->_hipo_name.' HIPO Complete';
			else                  $status_msg = 'Unknown Error Occured';
			$auto_step = $this->_options['auto_complete'];
		} else {
			$fn = $this->_steps[$step]['function_call']['paint_function'];
			$status_msg = $this->_steps[$step]['name'];
			$auto_step  = $this->_steps[$step]['auto_step'];
		}

		if ($this->complete() && !empty($this->_hipo_vars['errors'])) $auto_step = false;

		if ($auto_step) {
			if ($this->complete()) $onload = 'onLoad="self.location=\''.$url.'\'"';
			else $onload = 'onLoad="document.'.$form_name.'.submit()"';
		} else {
			$onload = '';
		}

		?>
		<html>
			<head>
				<style>
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
			</head>
			<body bgcolor="#FFFFFF" <?php echo $onload;?> class="sq-hipo-normal">
				<form name="<?php echo $form_name; ?>" method="post" action="<?php echo $url; ?>">
				<input type="hidden" name="cancel_hipo" value="0">
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
										<p>
										<?php
										if (count($this->_steps) > 1) {
											$this->paintProgressBar($this->_hipo_vars['percent_done'], '#412F49', 'Total HIPO Progress - '.$status_msg);
										} else {
											echo '<b>'.$status_msg.'</b>';
										}
										?></p><?php

										// print the error report if there are errors
										if ($this->complete() && !empty($this->_hipo_vars['errors'])) {
											?><p><?php
											$this->_paintErrorReport();
											?></p><?php
										}
										?>

										<p>
										<?php
										if (isset($this->_steps[$step])) {
											$prefix = $this->code_name.'step'.$step;
											$this->$fn($this->_steps[$step], $prefix);
										}
										?>
										</p>

										<?php
										$buttons_to_print = Array();

										if (!$this->complete()) {
											$buttons_to_print['cancel'] = Array('value' => 'Cancel', 'action' => "if (confirm('Are you sure you want to cancel?')) { this.form.cancel_hipo.value = '1'; this.form.submit(); }");
										}

										// print the NEXT or FINSIH button if we are not auto stepping through the HIPO
										if (!$auto_step) {
											if ($this->complete()) $button_text = 'Finish';
											else                  $button_text = 'Next >>';
											// they dont want us to refresh automatically, so we need to provide a manual NEXT button
											$buttons_to_print['next'] = Array('value' => $button_text, 'action' => 'this.form.submit()');
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
										}
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

	}//end paint()


	/**
	* Process the current step of a HIPO
	*
	* <P>This function decides, based on the current running_mode, which process function to call. It will also
	* switch to server mode if required based on the step data. This function also handles cancel (abort) requests
	* from the user.</P>
	* <P>This function sets the _completed flag of the HIPO to indicate if the HIPO is completed and saves
	* all HIPO var information to the database.</P>
	*
	* @return boolean
	* @access public
	*/
	function process()
	{
		if (!empty($_POST['cancel_hipo'])) {
			// they want to stop all processing and cancel the HIPO
			if (!$this->abort()) return false;
			$this->setOption('auto_complete', true);
			$this->_hipo_vars['current_step'] = count($this->_steps);
			return true;
		}

		$step = $this->_hipo_vars['current_step'];

		if ($this->_steps[$step]['running_mode'] != $this->_hipo_vars['running_mode']) {

			if ($this->_steps[$step]['running_mode'] == 'server' && SQ_HIPO_USE_SERVER) {
				// we have reached a step that requires server processing, but we are not yet
				// running in server mode (ie. we are not running on the HIPO server) - so lets
				// put a call through the HIPO phone to the server
				trigger_error('CONNECT TO HIPO SERVER HERE AND WAIT FOR ACK', E_USER_NOTICE);

			} else {
				trigger_error('SET RUN MODE', E_USER_NOTICE);
				$this->_hipo_vars['running_mode'] = $this->_steps[$step]['running_mode'];
			}

		} else {

			// if we are is a display mode different from the running mode, we should not be
			// doing any processing or saving - so just return true here
			if ($this->_current_mode != $this->_hipo_vars['running_mode']) return true;

			// call the correct process function based on the running mode
			$fn = 'process'.ucfirst($this->_hipo_vars['running_mode']);
			$prefix = $this->code_name.'step'.$step;
			set_error_handler(Array(&$this, '_errorHandler'));
			if (!$this->$fn($this->_steps[$step], $prefix)) {
				$this->abort();
				restore_error_handler();
				return false;
			}
			restore_error_handler();

		}//end if

		return $this->_save();

	}//end process()


	/**
	* Process the current step of a HIPO in the web interface
	*
	* This function calls the process function for the step currently being run - and that's it.
	*
	* @param array()	&$step_data	a reference to the array of information about the current step
	* @param string		$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processWeb(&$step_data, $prefix)
	{
		if (!empty($step_data['function_call'])) {
			$fn = $step_data['function_call']['process_function'];
			return $this->$fn($step_data, $prefix);

		} elseif (!empty($step_data['hipo_job']['init_details_function'])) {

			if (empty($step_data['hipo_job']['code_name']) || empty($step_data['hipo_job']['job_type'])) {
				$fn = $step_data['hipo_job']['init_details_function'];
				// First let's get the info needed to create this
				$this->$fn($job_type='', $running_vars=Array(), $options=Array());
				if (empty($job_type)) return false;

				require_once SQ_SYSTEM_ROOT.'/core/hipo/jobs/'.$job_type.'.inc';
				$init_hipo = new $job_type();

				$init_hipo->setRunningVars($running_vars);
				foreach($options as $k => $v) {
					$init_hipo->setOption($k, $v);
				}

				$code_name = $init_hipo->initialise($this->code_name);
				if (!$code_name) return false;
				$step_data['hipo_job']['code_name'] = $code_name;
				$step_data['hipo_job']['job_type']  = $job_type;

			} else {
				$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
				$hipo = &$hh->getJob($step_data['hipo_job']['code_name'], $step_data['hipo_job']['job_type']);
				if (is_null($hipo)) return false;
				if (!$hipo->process()) return false;

				$step_data['percent_done'] = $hipo->percentDone();
				$step_data['complete']     = $hipo->complete();

				return true;

			}// end if

		} else {
			trigger_error('Step '.$this->_hipo_vars['current_step'].' does not have any processing ability specified', E_USER_WARNING);
			return false;

		}// end if

	}//end processWeb()


	/**
	* Process the current step of a HIPO in a server environment
	*
	* This function should only be called by a HIPO running on the HIPO server, so we assume
	* no timeouts here and continually call the process function of the current step until
	* it is completed.
	*
	* @param array()	&$step_data	a reference to the array of information about the current step
	* @param string		$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processServer(&$step_data, $prefix)
	{
		if (!empty($step_data['function_call'])) {
			$fn = $step_data['function_call']['process_function'];
			while (!$step_data['complete']){
				if (!$this->$fn($step_data, $prefix)) return false;
				$this->_save();
			}
			return true;

		} elseif (!empty($step_data['hipo_job']['init_details_function'])) {

			$fn = $step_data['hipo_job']['init_details_function'];
			// First let's get the info needed to create this
			$this->$fn($job_type='', $running_vars=Array(), $options=Array());
			if (empty($job_type)) return false;

			require_once SQ_SYSTEM_ROOT.'/core/hipo/jobs/'.$job_type.'.inc';
			$init_hipo = new $job_type();

			$init_hipo->setRunningVars($running_vars);
			foreach($options as $k => $v) {
				$init_hipo->setOption($k, $v);
			}

			$code_name = $init_hipo->initialise($this->code_name);
			if (!$code_name) return false;

			unset($init_hipo);

			$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$hipo = &$hh->getJob($code_name, $job_type);
			if (is_null($hipo)) return false;

			while (!$step_data['complete']){
				if (!$hipo->process()) return false;
				$step_data['percent_done'] = $hipo->percentDone();
				$step_data['complete']     = $hipo->complete();
				$this->_save();
			}// end if

			return true;

		} else {
			trigger_error('Step '.$this->_hipo_vars['current_step'].' does not have any processing ability specified', E_USER_WARNING);
			return false;

		}// end if

	}//end processServer()


	/**
	* Paint a report of errors that were triggered during HIPO execution
	*
	* @return void
	* @access private
	*/
	function _paintErrorReport()
	{
		require SQ_FUDGE_PATH.'/general/datetime.inc';
		?>
		<div align="center">
		<table border="0" cellspacing="1" cellpadding="1" bgcolor="#000000" width="95%">
			<tr>
				<td bgcolor="#412F49" align="center" width="100%"><b><?php echo $this->_hipo_name;?> HIPO Error Report</b></td>
			</tr>
			<tr>
				<td bgcolor="#9E86AA" width="100%">
					<table border="0" width="100%" cellspacing="2" cellpadding="1">
					<?php
						foreach ($this->_hipo_vars['errors'] as $error) {
							?>
							<tr>
								<td align="left" valign="top">
									<b>
									<?php
									if ($error['warning']) echo 'WARNING: ';
									else                   echo 'ERROR: ';
									?>
									</b>
									<?php echo $error['message']; ?>
									<br/>
									<span class="sq-hipo-small">Reported <?php echo readable_datetime($error['time']);?></span>
									<hr/>
								</td>
							</tr>
							<?php
						}
					?>
					</table>
				</td>
			</tr>
			<tr>
				<td bgcolor="#412F49" align="center" width="100%"><b>End of Report</b></td>
			</tr>
		</table>
		</div>
		<?php

	}//end _paintErrorReport()


	/**
	* Paint a progress bar for a HIPO
	*
	* @param float	$percent_done	the percentage of progress completed
	* @param string	$colour			the colour to paint the progress bar
	* @param string	$message		a status message to display above the progress bar
	*
	* @return void
	* @access public
	*/
	function paintProgressBar($percent_done, $colour='#412F49', $message='')
	{
		?>
		<table width="100%" border="0" cellspacing="2" cellpadding="1">
			<tr>
				<td style="vertical-align: bottom;"><b><?php echo nl2br($message); ?></b></td>
				<td style="text-align: right; vertical-alignment: top;"><b><?php echo round($percent_done); ?>%</b></td>
			</tr>
			<tr>
				<td colspan="2" bgcolor="#412F49" width="100%">
					<table bgcolor="#412F49" width="100%" cellspacing="1" cellpadding="0" border="0">
						<tr>
							<td width="100%" bgcolor="#FFFFFF">
							<?php
							if ($percent_done) {
								?>
								<table width="<?php echo round($percent_done); ?>%" cellspacing="0" cellpadding="0" border="0">
									<tr>
										<td bgcolor="<?php echo $colour; ?>">&nbsp;</td>
									</tr>
								</table>
								<?php
							} else {
								?>&nbsp;<?php
							}
							?>
							</td>
						</tr>
					</table>
				</td>
			</tr>
		</table>
		<?php

	}//end paintProgressBar()


	/**
	* This function captures all errors and warnings that occur during the execution of process()
	*
	* @param int	$err_no		The type of error (E_*)
	* @param string	$err_msg	The error message
	* @param string	$err_file	The file the error occured in
	* @param string	$err_line	The line the error occured on
	*
	* @returns void
	* @access private
	* @see HIPO_Job::process()
	*/
	function _errorHandler($err_no, $err_msg, $err_file, $err_line)
	{
		$terminate = ((E_USER_ERROR | E_ERROR) & $err_no);

		// if the function didn't have an '@' prepended OR if we are about to terminate
		// catch the error
		if (error_reporting() || $terminate) {

			// Strip out the file path begining
			$err_file = str_replace(Array("\r", SQ_SYSTEM_ROOT), Array('', '[SYSTEM_ROOT]'), $err_file);
			$err_msg  = str_replace(Array("\r", SQ_SYSTEM_ROOT), Array('', '[SYSTEM_ROOT]'), $err_msg);

			$text_msg = strip_tags(preg_replace(Array('/<br\\/?>/i', '/<p[^>]*>/i'), Array("\n", "\n\n"), $err_msg));

			// send a report to the system error log
			if (ini_get('log_errors')) {
				$err_name = '';
				switch($err_no) {
					case E_USER_ERROR	: $err_name = SQ_SYSTEM_SHORT_NAME.' Error';	break;
					case E_USER_WARNING	: $err_name = SQ_SYSTEM_SHORT_NAME.' Warning';	break;
					case E_USER_NOTICE	: $err_name = SQ_SYSTEM_SHORT_NAME.' Notice';	break;
					case E_ERROR		: $err_name = 'PHP Error';						break;
					case E_WARNING		: $err_name = 'PHP Warning';					break;
					case E_NOTICE		: $err_name = 'PHP Notice';						break;
					default				: $err_name = 'Unknown Error Type - '.$err_no;
				}

				$log_msg = date('D d/m/Y H:i:s').' ['.strtolower($err_name).'] ('.$err_file.':'.$err_line.') - '.$text_msg;
				$log_msg = str_replace('[SYSTEM_ROOT]', '', $log_msg);
				error_log($log_msg);
			}

			$msg = $text_msg.' ('.$err_file.':'.$err_line.')';
			$msg = str_replace('[SYSTEM_ROOT]', '', $msg);
			$this->_addError($msg, !$terminate);

		}// end error_reporting

	}// end _errorHandler()

}//end class

?>