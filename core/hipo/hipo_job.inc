<?php

/**
* Highly Intensive Processing Object (HIPO)
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
* @subpackage HIPO
*/
class HIPO_Job
{

	/**
	* The name of this HIPO job (should not be modified by code)
	* @var string
	*/
	var $_hipo_name = '';

	/**
	* Array that stores some variables that the HIPO steps use
	* @var Array()
	*/
	var $_hipo_vars = Array('current_step' => 1,
							'percent_done' => 0,
							'message'      => '',
							'errors'       => Array(),
							);

	/**
	* Some global options that affect how the HIPO does things
	* @var Array
	*/
	var $_options = Array('on_complete_url' => '',
						  'auto_complete'   => true,
						  );

	/**
	* Array that stores any running variables that any of the processing functions use
	* @var Array()
	*/
	var $_running_vars = Array();

	/**
	* Flag that represents if the HIPO has completed all its steps
	* @var Array
	*/
	var $_complete = false;

	/**
	* Array of steps to perform in this HIPO
	* @var Array
	*/
	var $_steps = Array();


	/**
	* Constructor
	*
	*/
	function HIPO_Job()
	{

	}//end constructor


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* @return string
	* @access public
	*/
	function getCodeName()
	{
		return strtolower(get_class($this));

	}//end getCodeName()


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
		if ($this->_complete) {
			// this HIPO is finsihed
			return $this->_options['on_complete_url'];
		} else {
			return $_SERVER['PHP_SELF'].'?SQ_ACTION=hipo&hipo_action=edit_permissions&hipo_codename='.$this->getCodeName();
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
		if (!empty($error)) $this->_hipo_vars['errors'][] = Array('time'    => time(),
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
	* @return boolean
	* @access public
	*/
	function initialise()
	{
		$steps     = serialize($this->_steps);
		$vars      = serialize($this->_running_vars);
		$options   = serialize($this->_options);
		$hipo_vars = serialize($this->_hipo_vars);
		$codename  = $this->getCodeName();
		$message   = '';

		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$sql = 'INSERT INTO '.SQ_TABLE_PREFIX.'hipo_running_vars 
				(code_name, hipo_vars, options, steps, running_vars)
				VALUES ('.$db->quote($codename).',
						'.$db->quote($hipo_vars).',
						'.$db->quote($options).',
						'.$db->quote($steps).',
						'.$db->quote($vars).'
						)';

		$result = $db->query($sql);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		return true;

	}//end initialise()


	/**
	* Load the HIPO vars from the database
	*
	* @return boolean
	* @access public
	*/
	function load($codename)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT code_name, hipo_vars, options, steps, running_vars
				FROM '.SQ_TABLE_PREFIX.'hipo_running_vars 
				WHERE code_name = '.$db->quote($codename);

		$result = $db->getRow($sql);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		$this->_code_name    = $result['code_name'];
		$this->_hipo_vars    = unserialize($result['hipo_vars']);
		$this->_options      = unserialize($result['options']);
		$this->_steps        = unserialize($result['steps']);
		$this->_running_vars = unserialize($result['running_vars']);

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
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		if ($this->_complete) {
			$sql = 'DELETE FROM '.SQ_TABLE_PREFIX.'hipo_running_vars WHERE code_name = '.$db->quote($this->_code_name);
		} else {
			$sql = 'UPDATE '.SQ_TABLE_PREFIX.'hipo_running_vars
					SET hipo_vars    = '.$db->quote(serialize($this->_hipo_vars)).',
						options      = '.$db->quote(serialize($this->_options)).',
						steps        = '.$db->quote(serialize($this->_steps)).',
						running_vars = '.$db->quote(serialize($this->_running_vars)).'
					WHERE code_name  = '.$db->quote($this->_code_name);
		}
		$result = $db->query($sql);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

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
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$sql = 'DELETE FROM '.SQ_TABLE_PREFIX.'hipo_running_vars WHERE code_name = '.$db->quote($this->_code_name);
		$result = $db->query($sql);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
		return true;

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
		$step = $this->_hipo_vars['current_step'];

		$form_name = 'HIPO_'.$this->getCodeName();
		$url = $this->getProcessURL();
		$auto_step = false;

		if (!isset($this->_steps[$step])) {
			// this step doesnt exist - probably reached the end of the HIPO
			if ($this->_complete) $status_msg = $this->_hipo_name.' HIPO Complete';
			else                  $status_msg = 'Unknown Error Occured';
			$auto_step = $this->_options['auto_complete'];
		} else {
			$fn = $this->_steps[$step]['paint_function'];
			$status_msg = $this->_steps[$step]['name'];
			$auto_step  = $this->_steps[$step]['auto_step'];
		}

		if ($this->_complete && !empty($this->_hipo_vars['errors'])) $auto_step = false;

		if ($auto_step) {
			if ($this->_complete) $onload = 'onLoad="self.location=\''.$url.'\'"';
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
								<td>
								<table border="0" cellspacing="2" cellpadding="2" width="100%" bgcolor="#735B7D">
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
										if ($this->_complete && !empty($this->_hipo_vars['errors'])) {
											?><p><?php
											$this->_paintErrorReport();
											?></p><?php
										}
										?>

										<p>
										<?php
										if (isset($this->_steps[$step])) {
											$prefix = $this->getCodeName().'step'.$step;
											$this->$fn($this->_steps[$step], $prefix);
										}
										?>
										</p>

										<?php
										$buttons_to_print = Array();

										if (!$this->_complete) {
											$buttons_to_print['cancel'] = Array('value' => 'Cancel', 'action' => "if (confirm('Are you sure you want to cancel?')) { this.form.cancel_hipo.value = '1'; this.form.submit(); }");
										}

										// print the NEXT or FINSIH button if we are not auto stepping through the HIPO
										if (!$auto_step) {
											if ($this->_complete) $button_text = 'Finish';
											else                  $button_text = 'Next >>';
											// they dont want us to refresh automatically, so we need to provide a manual NEXT button
											$buttons_to_print['next'] = Array('value' => $button_text, 'action' => 'this.form.submit()');
										}

										if (count($buttons_to_print)) {
											include_once SQ_LIB_PATH.'/html_form/html_form.inc';
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
	* This function calls the process function for the step currently being run, sets the _completed 
	* flag of the HIPO to indicate the current running state, and saves the information to the database
	*
	* @return void
	* @access public
	*/
	function process()
	{
		if (isset($_POST['cancel_hipo']) && $_POST['cancel_hipo']) {
			// they want to stop all processing and cancel the HIPO
			if (!$this->abort()) return false;
			$this->_complete = true;
			$this->setOption('auto_complete', true);
			$this->_hipo_vars['current_step'] = count($this->_steps) + 1;
			return true;
		}

		$step =& $this->_hipo_vars['current_step'];
		$fn = $this->_steps[$step]['process_function'];
		$prefix = $this->getCodeName().'step'.$step;
		
		set_error_handler(Array(&$this, '_errorHandler'));
		if (!$this->$fn($this->_steps[$step], $prefix)) {
			restore_error_handler();
			$hipo->abort();
			return false;
		}
		restore_error_handler();

		$prev_step_per = (($step - 1) * 100) / count($this->_steps);
		$this_step_per = (1 / count($this->_steps)) * $this->_steps[$step]['percent_done'];
		$this->_hipo_vars['percent_done'] = round($prev_step_per + $this_step_per, 2);

		if ($this->_steps[$step]['complete']) $this->_hipo_vars['current_step']++;

		if ($this->_hipo_vars['current_step'] > count($this->_steps)) $this->_complete = true;
		else $this->_complete = false;
		
		return $this->_save();

	}//end process()


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
	}


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