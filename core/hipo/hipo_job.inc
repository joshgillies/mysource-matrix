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
* $Id: hipo_job.inc,v 1.41 2004/02/11 18:47:49 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


// include the hipo system config file
require_once SQ_DATA_PATH.'/private/conf/hipo.inc';

/**
* Highly Intensive Processing Object (HIPO)
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job extends MySource_Object
{

	/**
	* The name of this HIPO job (should not be modified by code)
	* @var string
	*/
	var $code_name = '';

	/**
	* The code name of the top level hipo job that this job is being run under
	* @var string
	*/
	var $source_code_name = '';

	/**
	* The user that this HIPO Job is to be run as
	* @var string
	*/
	var $userid = 0;

	/**
	* When this job was last save()'d
	* @var string
	*/
	var $last_updated = 0;

	/**
	* The name of this HIPO job (should not be modified by code)
	* @var string
	*/
	var $hipo_name = '';

	/**
	* Flag to see if we are prepared or not
	* @var boolean
	*/
	var $is_prepared = false;

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
						'server_taskid'		=> '',		// id the HIPO server gave the task when it went into running_mode='server'
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
	* Flag that represents what mode the HIPO is being run in
	* @var string
	*/
	var $_current_mode = 'web';

	/**
	* Zero Indexed Array of steps to perform in this HIPO
	* Form :
	* <pre>
	* Array(
	*		Array(
	*			'name'			=> [name of the step],
	*		[
	*			'function_call'	=> Array(
	*									['paint_function'   => 'paintSomething(&$step_data, $prefix)',]
	*									'process_function' => 'processSomething(&$step_data, $prefix)',
	*								),
	*		|
	*			'hipo_job'		=> Array(
	*									'init_details_function' => 'getJobDetails(&$job_type, &$running_vars, &$options)',
	*								),
	*		]
	*			'running_mode'	=> [web|server],
	*			'auto_step'		=> [true|false],
	*			'percent_done'	=> 0,
	*			'complete'		=> false,
	*			'message'		=> '',
	*		),
	*		...
	* );
	* </pre>
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
	* @access public
	*/
	function getCodeName()
	{
		return strtolower(get_class($this));

	}//end getCodeName()


	/**
	* Returns a unique code_name the HIPO can use to ensure it is not being run twice
	*
	* @return boolean
	* @access public
	*/
	function complete()
	{
		return ($this->_hipo_vars['current_step'] >= count($this->_steps) || $this->_hipo_vars['aborted']);

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
		if (!is_array($vars)) {
			trigger_error('Running Vars needs to be an array', E_USER_WARNING);
			return;
		}
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

	}//end setOption()


	/**
	* Returns one of the global options for the HIPO
	*
	* @param string	$name		name of the option
	*
	* @return mixed
	* @access public
	*/
	function getOption($name)
	{
		if (!isset($this->_options[$name])) {
			trigger_error('HIPO Option "'.$name.'" not known', E_USER_WARNING);
			return null;
		}
		return $this->_options[$name];

	}//end getOption()


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
	* Get all the errors that have been produced by this running of this HIPO_Job
	*
	* @return Array()
	* @access 
	*/
	function getErrors()
	{
		return $this->_hipo_vars['errors'];
	}//end getErrors()


	/**
	* Set up vars and database information that the HIPO will need to run
	*
	* This function should be run before any painting or processing of the HIPO. It will
	* write the initial database entry for the HIPO so that the vars needed by the HIPO
	* are available on successive reloads.
	*
	* @param string	$source_code_name	the code name of the top level hipo job that this job is being run under
	*
	* @return string	returns the code_name for the new job
	* @access public
	*/
	function initialise($source_code_name=null)
	{
		$this->_hipo_vars['initialise_mode'] = $this->_current_mode;
		$this->_hipo_vars['running_mode']    = $this->_current_mode;

		if (empty($source_code_name)) {
			$source_initialise_mode = $this->_current_mode;
		} else {
			$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$source_job = &$hh->getJob($source_code_name);
			if (is_null($source_job) || ($source_job->source_code_name != $source_job->code_name)) return '';
			$source_initialise_mode = $source_job->getInitialiseMode();
		}

		for($i = 0; $i < count($this->_steps); $i++) {
			// if we are starting from a server script but this job needs the web to run, 
			// then we are fucked, so get out of here
			if ($source_initialise_mode == 'server' && $this->_steps[$i]['running_mode'] == 'web') {
				trigger_error('You can\'t run a hipo job that requires a "web" running mode when it\'s is being initialised from the server', E_USER_WARNING);
				return '';
			// if we aren't using the server and we are in a web script
			// make sure that all the steps are changed accordingly
			} else if ($this->_current_mode == 'web' && !SQ_HIPO_USE_SERVER) {
				$this->_steps[$i]['running_mode'] = 'web';

			} else if ($this->_steps[$i]['running_mode'] == 'server' && empty($this->_steps[$i]['auto_step'])) {
				trigger_error('You can\'t have auto_step set to false for a step that run\'s on the HIPO Server - it just does not make sense', E_USER_WARNING);
				$this->_steps[$i]['auto_step'] = true;

			}// end if
		}// end for

		$steps     = serialize($this->_steps);
		$vars      = serialize($this->_running_vars);
		$options   = serialize($this->_options);
		$hipo_vars = serialize($this->_hipo_vars);
		$code_name = $this->getCodeName();
		$message   = '';

		if (empty($source_code_name)) $source_code_name = $code_name;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db  = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT userid, last_updated
				FROM '.SQ_TABLE_PREFIX.'hipo_job
				WHERE code_name = '.$db->quote($code_name);
		$existing = $db->getRow($sql);
		if (DB::isError($existing)) {
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			trigger_error($existing->getMessage().'<br/>'.$existing->getUserInfo(), E_USER_WARNING);
			return '';
		}

		if (!empty($existing)) {
			$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($existing['userid']);
			trigger_error('A HIPO Job with the same code name is currently being run by "'.$user->name.'" (Id #'.$user->id.').'."\n".'It was last updated at '.$GLOBALS['SQ_SYSTEM']->datetime(iso8601_ts($existing['last_updated'])), E_USER_WARNING);
			return '';
		}

		$now = time();

		$sql = 'INSERT INTO '.SQ_TABLE_PREFIX.'hipo_job
				(code_name, job_type, userid, running, source_code_name, hipo_vars, options, steps, running_vars, last_updated)
				VALUES ('.$db->quote($code_name).',
						'.$db->quote(get_class($this)).',
						'.$db->quote($GLOBALS['SQ_SYSTEM']->currentUserId()).',
						0,
						'.$db->quote($source_code_name).',
						'.$db->quote($hipo_vars).',
						'.$db->quote($options).',
						'.$db->quote($steps).',
						'.$db->quote($vars).',
						'.$db->quote(ts_iso8601($now)).'
						)';
		$result = $db->query($sql);
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return '';
		}

		$this->userid       = $GLOBALS['SQ_SYSTEM']->currentUserId();
		$this->last_updated = $now;

		return $code_name;

	}//end initialise()


	/**
	* Paints backend of this job
	*
	* @param object Backend_Outputter	&$o			reference to the backend outputter
	* @param string						$class		calling class name
	*
	* @return void
	* @access private
	*/
	function paintConfig(&$o, $class)
	{
	}// end paintConfig()


	/**
	* Gets the config vars that this hipo requires.
	*
	* Config vars should be in the format:
	* <PRE>
	* Array(
	*    'SQ_DEFINE_1'   => Array('editable' => 0, 'default' => false),
	*    'SQ_DEFINE_2'   => Array('editable' => 1, 'default' => ''),
	* );
	* </PRE>
	* This method is called statically.
	* 
	* @access public
	* @return Array()
	* @static
	*/
	function getConfigVars()
	{
		return Array();
	
	}//end getConfigVars()


	/**
	* Prepares the running vars of this hipo.
	*
	* @access public
	* @return boolean
	*/
	function prepare()
	{
		$this->is_prepared = true;
		return true;
	
	}//end prepare()


	/**
	* Returns the current Threshold.
	*
	* Returns a value that represents the total percentage that this hipo would run in freestyle mode in its current state.
	* 
	* @return float
	* @access public
	*/
	function getThreshold()
	{
		return 0;
	
	}//end getThreshold()


	/**
	* Performs the duties of this hipo, without showing any output on the frontend, in the one execution cycle.
	* 
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		return true;

	}//end freestyle()


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
		$db  = &$GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT job_type, userid, running, source_code_name, hipo_vars, options, steps, running_vars, last_updated
				FROM '.SQ_TABLE_PREFIX.'hipo_job
				WHERE code_name = '.$db->quote($code_name);

		$result = $db->getRow($sql);
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		if (empty($result)) return false;

		if ($result['job_type'] != get_class($this)) {
			trigger_error('Attempted to load "'.$result['job_type'].'" job into a "'.get_class($this).'" object', E_USER_WARNING);
			return false;
		}

		$this->code_name        = $code_name;
		$this->source_code_name = $result['source_code_name'];
		$this->userid           = (int) $result['userid'];
		$this->_hipo_vars       = unserialize($result['hipo_vars']);
		$this->_options         = unserialize($result['options']);
		$this->_steps           = unserialize($result['steps']);
		$this->_running_vars    = unserialize($result['running_vars']);
		$this->last_updated     = iso8601_ts($result['last_updated']);

		return true;

	}//end load()


	/**
	* Save the HIPO vars to the database OR delete from database if complete
	*
	* @param boolean $ignore_running_mode	whether to ignore the running mode check or not
	*										(SHOULD ONLY BE USED BY setRunningMode())
	*
	* @return boolean
	* @access public
	*/
	function save($ignore_running_mode=false)
	{
		if ($this->userid != $GLOBALS['SQ_SYSTEM']->currentUserId()) {
			trigger_error('You cannot process "'.$this->code_name.'" HIPO job that you do not own', E_USER_WARNING);
			return false;
		}

		// if we have called save and we aren't the running version, bugger off
		if (!$ignore_running_mode && $this->_current_mode != $this->_hipo_vars['running_mode']) {
			return false;
		}

		// First let's make sure that everything is in order with the hipo vars and our complete status
		if (!$this->complete()) {
			$step = $this->_hipo_vars['current_step'];

			$prev_step_per = ($step / count($this->_steps)) * 100;
			$this_step_per = ($this->complete()) ? 0 : ((1 / count($this->_steps)) * $this->_steps[$step]['percent_done']);
			$this->_hipo_vars['percent_done'] = round($prev_step_per + $this_step_per, 2);
		
			if (!empty($this->_steps[$step]['complete'])) $this->getNextStep();

		}// end if

		// if we have completed then we need to re-set the running mode back to the init mode
		if ($this->complete()) {
			$this->_hipo_vars['running_mode'] = $this->_hipo_vars['initialise_mode'];
		}

		// Second let's decide, depening on the circumstances, whether to delete or update
		$delete = false;
		// if we have aborted, delete right now
		if ($this->_hipo_vars['aborted']) {
			$delete = true;
		} else if ($this->complete()) {
			// if we are running from the web, we can delete the DB entry
			// because we are now done with it
			if ($this->_current_mode == 'web') {
				$delete = true;

			// running from the server/cmdline
			} else {

				// The idea here is that if we are the top hipo job AND we were initialised in a web script
				// then we need to leave the DB entry so that the refreshing we script can get the data one last time
				if ($this->code_name != $this->source_code_name || $this->_hipo_vars['initialise_mode'] != 'web') {
					$delete = true;
				}

			}// endif

		}// end if

		$now = time();

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		// now we just do what we are told
		if ($delete) {
			$sql = 'DELETE FROM '.SQ_TABLE_PREFIX.'hipo_job
					WHERE code_name = '.$db->quote($this->code_name);

		} else {
			$sql = 'UPDATE '.SQ_TABLE_PREFIX.'hipo_job
					SET hipo_vars    = '.$db->quote(serialize($this->_hipo_vars)).',
						options      = '.$db->quote(serialize($this->_options)).',
						steps        = '.$db->quote(serialize($this->_steps)).',
						running_vars = '.$db->quote(serialize($this->_running_vars)).',
						last_updated = '.$db->quote(ts_iso8601($now)).'
					WHERE code_name  = '.$db->quote($this->code_name);
		}
		$result = $db->query($sql);
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		$this->last_updated = $now;

		return true;

	}//end save()


	/**
	* If we are currently in a hipo_job step then it returns the job that we are executing
	* Otherwise returns NULL
	*
	* @return null|object HIPO_Job
	* @access private
	*/
	function &_getSubJob()
	{
		$step = $this->_hipo_vars['current_step'];
		if (isset($this->_steps[$step])) {
			if (!empty($this->_steps[$step]['hipo_job']) && !empty($this->_steps[$step]['hipo_job']['code_name']) && !empty($this->_steps[$step]['hipo_job']['job_type'])) {
				$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
				return $hh->getJob($this->_steps[$step]['hipo_job']['code_name'], $this->_steps[$step]['hipo_job']['job_type']);
			}// end if
		}// endif

		$null = null; // because we need to return by ref
		return $null;

	}// end _getSubJob();


	/**
	* Returns the mode that this task was initialised in
	*
	* @return string
	* @access public
	*/
	function getInitialiseMode()
	{
		return $this->_hipo_vars['initialise_mode'];
	}// end getInitialiseMode();

	
	/**
	* Set the mode that we are running (ie processing) in
	*
	* @param string	$mode			[web|server]
	* @param string	$server_taskid	when the $mode == 'server' this must be supplied, it is the taskid given to it by the server
	*
	* @return boolean	whether set was successfull
	* @access public
	*/
	function setRunningMode($mode, $server_taskid='')
	{
		if ($mode != 'web' && $mode != 'server') {
			return false;
		}

		$sub_job = &$this->_getSubJob();
		if (!is_null($sub_job)) {
			if (!$sub_job->setRunningMode($mode)) {
				return false;
			}
		}// end if

		$this->_hipo_vars['running_mode']  = $mode;
		$this->_hipo_vars['server_taskid'] = $server_taskid;
		if ($this->save(true)) {
			return true;
		} else {
			return false;
		}

	}// end setRunningMode();


	/**
	* Returns the running mode of this task
	*
	* @return string
	* @access public
	*/
	function getRunningMode()
	{
		return $this->_hipo_vars['running_mode'];
	}// end getRunningMode();


	/**
	* Whether we want to auto_step in the paint mode for this job or not
	*
	* @return boolean
	* @access public
	*/
	function auto_step()
	{
		$step = $this->_hipo_vars['current_step'];
		if ($this->complete()) {
			// if we are complete, but there are errors, we need to pause for them to be seen
			if (!empty($this->_hipo_vars['errors'])) return false;
			else return $this->_options['auto_complete'];

		} elseif (!empty($this->_steps[$step]['function_call'])) {
			return $this->_steps[$step]['auto_step'];

		} elseif (!empty($this->_steps[$step]['hipo_job'])) {

			$sub_job = &$this->_getSubJob();
			// if we are processing a sub job, it's it's decision whether to auto step or not
			if (!is_null($sub_job)) {
				return $sub_job->auto_step();

			// otherwise do what's set for this step
			} else {
				return $this->_steps[$step]['auto_step'];
			
			}// end if

		}// end if

		return false;

	}//end auto_step()



	/**
	* returns the next step to be executed by the hipo job, 
	*
	* @access public
	* @return integer 
	*/
	function getNextStep()
	{
		$this->_hipo_vars['current_step']++;
		for ($i = $this->_hipo_vars['current_step']; $i < count($this->_steps); $i++) {
			if (isset($this->_steps[$i]['skip_step']) && $this->_steps[$i]['skip_step']) {
				$this->_steps[$i]['complete'] = true;
				$this->_steps[$i]['percent_done'] = 100;
				$this->_hipo_vars['current_step']++;

				continue;
			} else {
				break;
			}
		}

		return $this->_hipo_vars['current_step'];

	} // end getNextStep()


	/**
	* called when a step in the hip job process needs to be skipped
	*
	* @param integer $stepid the array index of the step
	*
	* @access public
	* @return boolean
	*/
	function skipStep($stepid=0)
	{
		if (!isset($this->_steps[$stepid])) return false;
		$this->_steps[$stepid]['skip_step'] = true;
		
		return true;
	
	} // end skipStep()


	/**
	* Paint information about the process of the HIPO
	*
	* This function calls the paint function for the step currently being run and also outputs
	* a progress bar to indicate to the user where they are in the HIPO process
	*
	* @return void
	* @access public
	*/
	function paint(&$o, $nested=false)
	{
		if ($this->_current_mode != 'web') {
			trigger_error('You cannot paint the job when you are not in Web Mode', E_USER_WARNING);
			return;
		}

		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		$step = $this->_hipo_vars['current_step'];
		$step_data = (isset($this->_steps[$step])) ? $this->_steps[$step] : Array();

		// if we have more than one step we need to "total" progress bar
		if ($nested && count($this->_steps) > 1) {
			if ($step_data) {
				$status_msg = $this->_steps[$step]['name'];
			} else {
				// this step doesnt exist - probably reached the end of the HIPO
				if ($this->complete()) $status_msg = 'Complete';
				else                   $status_msg = 'Unknown Error Occured';
			}

			$o->openSection($this->hipo_name);
				$o->openField('&nbsp;');
				HIPO_Job::paintProgressBar($this->_hipo_vars['percent_done'], $this->hipo_name.' HIPO Progress - '.$status_msg);
				$o->closeField();
			$o->closeSection();
		}

		// print the error report if there are errors
		if ($this->complete() && !empty($this->_hipo_vars['errors'])) {
			echo '<p>';
			$this->_paintErrorReport();
			echo '</p>';
		}

		if ($step_data) {

			if (!empty($this->_steps[$step]['function_call'])) {
				$fn = (empty($this->_steps[$step]['function_call']['paint_function'])) ? 'paintStepDefault' : $this->_steps[$step]['function_call']['paint_function'];
				$prefix = $this->code_name.'step'.$step;
				$this->$fn($this->_steps[$step], $o, $prefix);

			} elseif (!empty($step_data['hipo_job'])) {

				$sub_job = &$this->_getSubJob();
				if (!is_null($sub_job)) {
					$o->closeRaw();
					$sub_job->paint($o, true);
					$o->openRaw();
				}// end if

			} else {
				trigger_error('Step '.$this->_hipo_vars['current_step'].' does not have any processing ability specified', E_USER_WARNING);

			}// end if

		}// end if

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
		if ($this->userid != $GLOBALS['SQ_SYSTEM']->currentUserId()) {
			trigger_error('You cannot process "'.$result['job_type'].'" HIPO job that you do not own', E_USER_WARNING);
			return false;
		}
		
		// if we are is a display mode different from the running mode, we should not be
		// doing any processing or saving - so just return true here
		if ($this->_current_mode != $this->_hipo_vars['running_mode']) return true;

		// if we are complete, then just save (which will clear the DB if needed)
		if ($this->complete()) return $this->save();

		$step = $this->_hipo_vars['current_step'];

		if ($this->_steps[$step]['running_mode'] == $this->_hipo_vars['running_mode']) {

			// call the correct process function based on the running mode
			$fn = 'process'.$this->_hipo_vars['running_mode'];
			$prefix = $this->code_name.'step'.$step;
			set_error_handler(Array(&$this, '_errorHandler'));
			$ret_val = $this->$fn($this->_steps[$step], $prefix);
			restore_error_handler();
			return $ret_val;

		// if we are using the server
		} elseif (SQ_HIPO_USE_SERVER) {

			switch($this->_steps[$step]['running_mode']) {
				// We are currently running in a web script, but this step utilises server processing
				// let's put a call through the HIPO phone to the server
				case 'server' :

					require_once SQ_SYSTEM_ROOT.'/core/server/squiz_server_client_mysource3.inc';

					$data = Array(
								'command'			=> 'start', 
								'userid'			=> $GLOBALS['SQ_SYSTEM']->currentUserId(),
								'source_code_name'	=> $this->source_code_name,
							);

					$ret_val = false;
					$client = new Squiz_Server_Client_MySource3();
					if ($client->connect()) {
						if ($client->write('HIPO_Job', $data)) {
							$ack = $client->read();
							if (!is_null($ack)) {
								switch ($ack['result']) {
									case 'ack' :
										$ret_val = true;
										break;

									case 'error' :
										trigger_error('HIPO Server Error : '.$ack['msg'], E_USER_WARNING);
										break;

									default :
										trigger_error('Unexpected return from the HIPO Server : '.print_r($ack, true), E_USER_WARNING);

								}// end switch
							}// end if
						}// end if

						$client->disconnect();

					}// endif

					// return here because we don't want to save and override the DB value now that it's running 
					// on the server
					return $ret_val;

					break;

				// We are currently running on the server, but this step needs web processing
				// let's reset the running mode and get out of here
				case 'web' :
					// if we have reached a new step, that uses a different running mode to ourselves, then change our running mode
					pre_echo(__FUNCTION__."::".__LINE__.">>>>>>>>>>>>>>>>> RESET RUNNING MODE TO web <<<<<<<<<<<<<<<<<<<");
					return $this->setRunningMode('web');

					break;

				default :
					trigger_error('Unknown Running Mode "'.$this->_steps[$step]['running_mode'].'"', E_USER_WARNING);
					$this->abort();
					return false;
					
			}//end switch

		// if we aren't using the server this should never happen (because of what happens in initialise())
		} else {
			trigger_error('Something is not right with this HIPO, please consult your system administrator', E_USER_ERROR);

		}// endif

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
		$process_ok = false;
		if (!empty($step_data['function_call'])) {
			$fn = $step_data['function_call']['process_function'];
			$process_ok = $this->$fn($step_data, $prefix);
			

		} elseif (!empty($step_data['hipo_job'])) {
			if (empty($step_data['hipo_job']['code_name']) || empty($step_data['hipo_job']['job_type'])) {
				$fn = $step_data['hipo_job']['init_details_function'];
				// First let's get the info needed to create this
				$this->$fn($job_type='', $running_vars=Array(), $options=Array());

				// so they have decided to ignore this step ? that's cool
				if (empty($job_type)) {
					$process_ok = true;

				} else {

					require_once SQ_SYSTEM_ROOT.'/core/hipo/jobs/'.$job_type.'.inc';
					$init_hipo = new $job_type();

					$init_hipo->setRunningVars($running_vars);
					foreach($options as $k => $v) {
						$init_hipo->setOption($k, $v);
					}

					$code_name = $init_hipo->initialise($this->source_code_name);
					if ($code_name) {
						$step_data['hipo_job']['code_name'] = $code_name;
						$step_data['hipo_job']['job_type']  = $job_type;
						$process_ok = true;
					}

				}// end if

			} else {
				$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
				$hipo = &$hh->getJob($step_data['hipo_job']['code_name'], $step_data['hipo_job']['job_type']);
				if (!is_null($hipo) && $hipo->process()) {

					$step_data['running_mode'] = $hipo->getRunningMode();
					$step_data['percent_done'] = $hipo->percentDone();
					$step_data['complete']     = $hipo->complete();
					$process_ok = true;

				}


			}// end if

		} else {
			trigger_error('Step '.$this->_hipo_vars['current_step'].' does not have any processing ability specified', E_USER_WARNING);

		}// end if

		$save_ok = $this->save();
		return ($process_ok && $save_ok);

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
				if (!$this->$fn($step_data, $prefix)) {
					return false;
				}
				$this->save();
			}
			return true;

		} elseif (!empty($step_data['hipo_job'])) {

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

			$code_name = $init_hipo->initialise($this->source_code_name);
			if (!$code_name) return false;
			unset($init_hipo);

			$step_data['hipo_job']['code_name'] = $code_name;
			$step_data['hipo_job']['job_type']  = $job_type;
			$this->save();

			$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$hipo = &$hh->getJob($code_name, $job_type);
			if (is_null($hipo)) return false;

			while (!$step_data['complete'] && $step_data['running_mode'] == 'server'){
				if (!$hipo->process()) return false;
				$step_data['running_mode'] = $hipo->getRunningMode();
				$step_data['percent_done'] = $hipo->percentDone();
				$step_data['complete']     = $hipo->complete();
				$this->save();
			}// end if

			return true;

		} else {
			trigger_error('Step '.$this->_hipo_vars['current_step'].' does not have any processing ability specified', E_USER_WARNING);
			return false;

		}// end if

	}//end processServer()


	/**
	* Cancel the HIPO and cleanup
	*
	* @return boolean
	* @access public
	*/
	function abort()
	{
		if ($this->source_code_name != $this->code_name) {
			$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$source_job = &$hh->getJob($this->source_code_name);
			if (is_null($source_job) || ($source_job->source_code_name != $source_job->code_name)) return false;
			$source_job->abort();
		}

		// if aborting through a web script, but the processing is running on the server we need to inform the server to abort
		if ($this->_current_mode == 'web' && $this->_hipo_vars['running_mode'] == 'server') {

			// if there is no system taskid then there is nothing we can do
			if (empty($this->_hipo_vars['server_taskid'])) {
				trigger_error('Unable To Send Abort, No TaskID for the process on the server ', E_USER_NOTICE);
				return false;
			}

			require_once SQ_SYSTEM_ROOT.'/core/server/squiz_server_client_mysource3.inc';

			$data = Array(
						'command'			=> 'abort', 
						'userid'			=> $GLOBALS['SQ_SYSTEM']->currentUserId(),
						'source_code_name'	=> $this->source_code_name,
						'taskid'			=> $this->_hipo_vars['server_taskid'],
					);

			$ret_val = false;
			$client = new Squiz_Server_Client_MySource3();
			if ($client->connect()) {
				if ($client->write('HIPO_Job', $data)) {
					$ack = $client->read();
					if (!is_null($ack)) {
						switch ($ack['result']) {
							case 'ack' :
								$ret_val = true;
								break;

							case 'error' :
								trigger_error('HIPO Server Error : '.$ack['msg'], E_USER_WARNING);
								break;

							default :
								trigger_error('Unexpected return from the HIPO Server : '.print_r($ack, true), E_USER_WARNING);

						}// end switch

					}// end if
				}// end if

				$client->disconnect();
			}// endif

			if ($ret_val) return true;

			trigger_error('Forcing Abort of HIPO Job', E_USER_WARNING);
			// fall through and delete this bugger, set the running mode to allow the saving
			$this->setRunningMode('web');

		}// end if

		$this->jobAborted();

		$sub_job = &$this->_getSubJob();
		if (!is_null($sub_job)) {
			$sub_job->jobAborted();
		}// end if

		return true;

	}//end abort()


	/**
	* Called to do any clean-up for this job
	*
	* @return boolean
	* @access public
	*/
	function jobAborted()
	{
		$this->setOption('auto_complete', true);
		$this->_hipo_vars['aborted'] = true;
		return $this->save();

	}//end jobAborted()
	
	
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
				<td bgcolor="#412F49" align="center" width="100%"><b><?php echo $this->hipo_name;?> HIPO Error Report</b></td>
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
	* A default function for painting the progess of a step
	*
	* @param array()							&$step_data	a reference to the array of information about the current step
	* @param object Hipo_Backend_Outputter		&$o			the backend outputter class
	* @param string								$prefix		prefix for form vars
	*
	* @return void
	* @access public
	*/
	function paintStepDefault(&$step_data, &$o, $prefix)
	{
		$o->openSection($step_data['name']);
			$o->openField('&nbsp;');
			$this->paintProgressBar($step_data['percent_done'], $step_data['message']);
			$o->closeField();
		$o->closeSection();

	}//end paintStepDefault()

	
	/**
	* Paint a progress bar for a HIPO
	*
	* @param float	$percent_done		the percentage of progress completed
	* @param string	$message			a status message to display above the progress bar
	* @param string	$label_class		the css class for the progress bar label ($message)
	* @param string	$percent_class		the css class for the percentage done figure ($percentage)
	* @param string	$bar_main_class		the css class for the base of the progress bar
	* @param string	$bar_done_class		the css class for the completed part of the progress bar
	*
	* @return void
	* @access public
	* @static
	*/
	function paintProgressBar($percent_done, $message='', $label_class='sq-hipo-progress-bar-label', $percent_class='sq-hipo-progress-bar-percent', $bar_main_class='sq-hipo-progress-bar-main', $bar_done_class='sq-hipo-progress-bar-done')
	{
		?>
		<table width="100%" border="0" cellspacing="0" cellpadding="0">
			<tr>
				<td class="<?php echo $label_class; ?>"><b><?php echo nl2br($message); ?></b></td>
				<td class="<?php echo $percent_class; ?>"><b><?php echo round($percent_done); ?>%</b></td>
			</tr>
			<tr>
				<td colspan="2"><img src="<?php echo sq_web_path('lib').'/web/images/blank.gif'; ?>" width="1" height="10" alt="blank" /></td>
			<tr>
				<td colspan="2" width="100%">
					<table width="100%" cellspacing="0" cellpadding="0" border="0">
						<tr>
							<td class="<?php echo $bar_main_class; ?>" width="100%">
							<table width="<?php echo ($percent_done) ? round($percent_done) : '1'; ?>%" cellspacing="0" cellpadding="0" border="0">
								<tr>
									<td class="<?php echo $bar_done_class; ?>"><img src="<?php echo sq_web_path('lib').'/web/images/blank.gif'; ?>" width="1" height="8" alt="blank" /></td>
								</tr>
							</table>
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
