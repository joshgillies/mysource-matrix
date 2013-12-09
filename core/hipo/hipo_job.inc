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
* $Id: hipo_job.inc,v 1.113 2013/07/25 23:23:49 lwright Exp $
*
*/


// include the hipo system config file
require_once SQ_DATA_PATH.'/private/conf/hipo.inc';

/**
* Highly Intensive Processing Object (HIPO)
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.113 $
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
	* @deprecated 	in favour of getHipoName(); please translate() the HIPO name
	* 				inside that, unless no translation is required
	*/
	var $hipo_name = '';

	/**
	* Flag to see if we are prepared or not
	* @var boolean
	*/
	var $is_prepared = FALSE;

	/**
	* Flag which determines whether HIPO Herder wraps the whole job in a transaction
	*
	* If this is turned OFF (FALSE), we rely on this job to manage its own transactions!
	*
	* @var boolean
	*/
	var $uses_trans = TRUE;

	/**
	* Array that stores some variables that the HIPO steps use
	* @var array()
	*/
	var $_hipo_vars = Array(
						'current_step'		=> 0,
						'percent_done'		=> 0,
						'initialise_mode'	=> '',      // the mode that this job was initialised() in,
															// used to decide what to do when we are completed
						'running_mode'		=> '',      // the mode that is currently doing the processing
						'aborted'			=> FALSE,   // whether we have been aborted or not
						'server_taskid'		=> '',      // id the HIPO server gave the task when it went
															// into running_mode='server'
						'message'			=> '',
						'job_dir'			=> '',      // the directory where the jobs include file is located
						'errors'			=> Array(),
					  );

	/**
	* Some global options that affect how the HIPO does things
	* @var array()
	*/
	var $_options = Array(
						'on_complete_url'	=> '',
						'auto_complete'		=> TRUE,
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
	*
	* Initialising the step data here is DEPRECATED as a result of translation,
	* please use getInitialStepData() and translate() all required strings. The
	* variable itself is NOT deprecated, however - still use as previously.
	*
	* If no translation is required then you can still initialise stuff here.
	*
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
	*			'auto_step'		=> [TRUE|FALSE],
	*			'skip_step'		=> [TRUE|FALSE],
	*			'allow_cancel'	=> [TRUE|FALSE],
	*			'percent_done'	=> 0,
	*			'complete'		=> FALSE,
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
	*
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
	* Returns the (localised) HIPO name
	*
	* This will return the hipo_name variable for backward compatibility reasons.
	* Please override this with a translate()'d call with the string code
	* containing the HIPO name.
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return $this->hipo_name;

	}//end getHipoName()


	/**
	* Returns the steps in this hipo, possibly with localised step names
	* and messages
	*
	* This will return the steps member variable for backward compatibilty
	* purposes. Please override this with the proper step data, which should be
	* the same as the old steps variable but with all appropriate fields
	* translate()'d.
	*
	* @return string
	* @access public
	*/
	function getInitialStepData()
	{
		return $this->_steps;

	}//end getInitialStepData()


	/**
	* Determines whether the HIPO job has finished its run (ie. is complete or was aborted)
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
	* @return array
	* @access public
	*/
	function &getRunningVars()
	{
		return $this->_running_vars;

	}//end getRunningVars()


	/**
	* Resets the vars that the HIPO steps use
	*
	* @param array	&$vars	variables used in hipo job
	*
	* @return void
	* @access public
	*/
	function setRunningVars(&$vars)
	{
		if (!is_array($vars)) {
			trigger_localised_error('HIPO0045', E_USER_WARNING);
			return;
		}
		$this->_running_vars =& $vars;

	}//end setRunningVars()


	/**
	* Set one of the global options for the HIPO
	*
	* @param string	$name	name of the option
	* @param string	$value	new value to assign it
	*
	* @return void
	* @access public
	*/
	function setOption($name, $value)
	{
		if (!isset($this->_options[$name])) {
			trigger_localised_error('HIPO0044', E_USER_WARNING, $name);
			return;
		}
		$this->_options[$name] = $value;

	}//end setOption()


	/**
	* Returns one of the global options for the HIPO
	*
	* @param string	$name	name of the option
	*
	* @return mixed NULL|string
	* @access public
	*/
	function getOption($name)
	{
		if (!isset($this->_options[$name])) {
			trigger_localised_error('HIPO0044', E_USER_WARNING, $name);
			return NULL;
		}
		return $this->_options[$name];

	}//end getOption()


	/**
	* Set one of the global hipo vars for the HIPO
	*
	* @param string	$name	name of the hipo var
	* @param string	$value	new value to assign it
	*
	* @return void
	* @access public
	*/
	function setHipoVar($name, $value)
	{
		if (!isset($this->_hipo_vars[$name])) {
			trigger_localised_error('HIPO0044', E_USER_WARNING, $name);
			return;
		}
		$this->_hipo_vars[$name] = $value;

	}//end setHipoVar()


	/**
	* Returns one of the global hipo vars for the HIPO
	*
	* @param string	$name	name of the hipo var
	*
	* @return mixed NULL|string
	* @access public
	*/
	function getHipoVar($name)
	{
		if (!isset($this->_hipo_vars[$name])) {
			trigger_localised_error('HIPO0044', E_USER_WARNING, $name);
			return NULL;
		}
		return $this->_hipo_vars[$name];

	}//end getHipoVar()


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
	function _addError($error, $warning=FALSE)
	{
		$error = trim($error);
		if (!empty($error)) {
			$this->_hipo_vars['errors'][] = Array(
												'time'		=> time(),
												'message'	=> $error,
												'warning'	=> $warning,
											);
		}

	}//end _addError()


	/**
	* Get all the errors that have been produced by this running of this HIPO_Job
	*
	* @return array
	* @access public
	*/
	function getErrors()
	{
		return $this->_hipo_vars['errors'];

	}//end getErrors()


	/**
	* Prepares and sets up the running vars of this hipo.
	* This is run before initialise(), because initialise() may not be run at all if this Job can be freestyle()'d
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		$this->is_prepared = TRUE;
		return TRUE;

	}//end prepare()


	/**
	* Set database information that the HIPO will need to run
	*
	* This function should be run before any painting or processing of the HIPO. It will
	* write the initial database entry for the HIPO so that the vars needed by the HIPO
	* are available on successive reloads
	* This function returns the code_name for the new job
	*
	* @param string	$source_code_name	the code name of the top level hipo job that this job is being run under
	*
	* @return string
	* @access public
	*/
	function initialise($source_code_name=NULL)
	{
		if (!$this->is_prepared) {
			if (!$this->prepare()) return FALSE;
		}

		$this->_hipo_vars['initialise_mode'] = $this->_current_mode;
		$this->_hipo_vars['running_mode']    = $this->_current_mode;

		if (empty($source_code_name)) {
			$source_initialise_mode = $this->_current_mode;
		} else {
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$source_job = $hh->getJob($source_code_name);
			if (is_null($source_job) || ($source_job->source_code_name != $source_job->code_name)) {
				return '';
			}
			$source_initialise_mode = $source_job->getInitialiseMode();
		}

		// get the initial set of steps
		$init_steps = $this->getInitialStepData();

		for ($i = 0; $i < count($init_steps); $i++) {
			// if we are starting from a server script but this job needs the web to run,
			// then we are in trouble, so get out of here
			if ($source_initialise_mode == 'server' && $init_steps[$i]['running_mode'] == 'web') {
				trigger_localised_error('HIPO0043', E_USER_WARNING);
				return '';
			// if we aren't using the server and we are in a web script
			// make sure that all the steps are changed accordingly
			} else if ($this->_current_mode == 'web' && !SQ_HIPO_USE_SERVER) {
				$init_steps[$i]['running_mode'] = 'web';

			} else if ($init_steps[$i]['running_mode'] == 'server' && empty($init_steps[$i]['auto_step'])) {
				trigger_localised_error('HIPO0042', E_USER_WARNING);
				$init_steps[$i]['auto_step'] = TRUE;

			}// end if
		}// end for

		// set the step member variable now that it's all translated and converted
		$this->_steps = $init_steps;

		$steps     = serialize($this->_steps);
		$vars      = serialize($this->_running_vars);
		$options   = serialize($this->_options);
		$hipo_vars = serialize($this->_hipo_vars);
		$code_name = $this->getCodeName();
		$message   = '';

		if (empty($source_code_name)) {
			$source_code_name = $code_name;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db3');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$db = MatrixDAL::getDb();

		try {
			$bind_vars['code_name'] = $code_name;
			$result = MatrixDAL::executeAssoc('core', 'getHipoJobsUser', NULL, $bind_vars);
			$existing =  array_get_index($result, 0, Array());
		} catch (Exception $e) {
			throw new Exception('Unable to get HIPO job code name: '.$code_name.' due to database error: '.$e->getMessage());
		}

		if (!empty($existing)) {
			$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($existing['userid']);
			trigger_localised_error('HIPO0041', E_USER_WARNING, $user->name, $user->id, $GLOBALS['SQ_SYSTEM']->datetime(iso8601_ts($existing['last_updated'])));
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return '';
		}

		$now = time();

		try {
			$bind_vars = Array(
							'code_name'			=> $code_name,
							'job_type'			=> get_class($this),
							'userid'			=> $GLOBALS['SQ_SYSTEM']->currentUserId(),
							'running'			=> 0,
							'source_code_name'	=> $source_code_name,
							'hipo_vars'			=> $hipo_vars,
							'options'			=> $options,
							'steps'				=> $steps,
							'running_vars'		=> $vars,
							'last_updated'		=> ts_iso8601($now),
						 );

			MatrixDAL::executeQuery('core', 'insertHipoJob', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to insert new HIPO job due to database error: '.$e->getMessage());
		}

		try {
			$result = MatrixDAL::executeOne('core', 'getHipoJobsUser', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to get HIPO job code name: '.$code_name.' due to database error: '.$e->getMessage());
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		$this->userid       = $GLOBALS['SQ_SYSTEM']->currentUserId();
		$this->last_updated = $now;

		return $code_name;

	}//end initialise()


	/**
	* Paints backend of this job
	*
	* @param object		&$o				Backend_Outputter, reference to the backend outputter
	* @param string		$class			calling class name
	* @param boolean	$write_access	write access priviledge to the current asset
	*
	* @return void
	* @access private
	*/
	public static function paintConfig(&$o, $class, $write_access)
	{

	}//end paintConfig()


	/**
	* Gets the config vars that this hipo requires.
	*
	* Config vars should be in the format:
	* <PRE>
	* Array(
	*    'SQ_DEFINE_1'   => Array('editable' => 0, 'default' => FALSE),
	*    'SQ_DEFINE_2'   => Array('editable' => 1, 'default' => ''),
	* );
	* </PRE>
	* This method is called statically.
	*
	* @return array
	* @access public
	* @static
	*/
	public static function getConfigVars()
	{
		return Array();

	}//end getConfigVars()


	/**
	* Returns the maximum number of assets we can process in FreeStyle mode
	*
	* @return int
	* @access public
	*/
	function getThreshold()
	{
		$config_vars = $this->getConfigVars();
		if (empty($config_vars)) return 1;
		foreach ($config_vars as $name => $value) {
			if (strpos($name, 'THRESHOLD') !== FALSE) {
				return constant($name);
			}
		}
		return 1;

	}//end getThreshold()


	/**
	* Returns what percentage of our threshold value is needed for this job
	*
	* @return int
	* @access public
	*/
	function getThresholdPercentageRequired()
	{
		return 0;

	}//end getThresholdPercentageRequired()


	/**
	* Performs the duties of this hipo, without showing any output on the frontend, in the one execution cycle.
	*
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		return TRUE;

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
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db3');
		$db  = $GLOBALS['SQ_SYSTEM']->db;
		$db  = MatrixDAL::getDb();
		try {
			$bind_vars['code_name'] = $code_name;
			$result = MatrixDAL::executeAssoc('core', 'loadHipoVars', $bind_vars);
			$result = $result[0];
		} catch (Exception $e) {
			throw new Exception('Unable to load the HIPO vars for: '.$code_name.' due to database error: '.$e->getMessage());
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		if (empty($result)) return FALSE;

		if ($result['job_type'] != get_class($this)) {
			trigger_localised_error('HIPO0040', E_USER_WARNING, $result['job_type'], get_class($this));
			return FALSE;
		}

		$this->code_name        = $code_name;
		$this->source_code_name = $result['source_code_name'];
		$this->userid           = $result['userid'];
		$this->_hipo_vars       = unserialize($result['hipo_vars']);
		$this->_options         = unserialize($result['options']);
		$this->_steps           = unserialize($result['steps']);
		$this->_running_vars    = unserialize($result['running_vars']);
		$this->last_updated     = iso8601_ts($result['last_updated']);

		// previous last_updated - the last_updated var from the last time load() was called
		$prev_last_updated = array_get_index($this->_hipo_vars, 'timeout_prev_last_updated', $this->last_updated);

		// If this task utilises server processing
		$running_at_server = isset($this->_steps[$this->_hipo_vars['current_step']]) && isset($this->_steps[$this->_hipo_vars['current_step']]['running_mode']) && $this->_steps[$this->_hipo_vars['current_step']]['running_mode'] == 'server';

		if ($this->_current_mode != 'server' && $running_at_server && SQ_HIPO_USE_SERVER) {
			if ($this->last_updated > $prev_last_updated) {
				// keep the last_updated record up to date
				$prev_last_updated = $this->last_updated;

			} else {
				$time_since_update = time() - $prev_last_updated;

				// is it time to send a request yet?
				// check whether we've failed to update
				if ($time_since_update > SQ_HIPO_SERVER_STATUS_CHECK_THRESHOLD) {
					// check whether the process is still alive - send a request to the server

					// if there is no system taskid then there is nothing we can do
					if (empty($this->_hipo_vars['server_taskid'])) {
						return FALSE;
					}

					require_once SQ_SYSTEM_ROOT.'/core/server/squiz_server_client_mysource3.inc';

					$data = Array(
								'command'			=> 'alive',
								'userid'			=> $GLOBALS['SQ_SYSTEM']->currentUserId(),
								'source_code_name'	=> $this->source_code_name,
								'taskid'			=> $this->_hipo_vars['server_taskid'],
							);

					$alive = FALSE;
					$client = new Squiz_Server_Client_MySource3();
					if ($client->connect()) {
						if ($client->write('HIPO_Job', $data)) {
							$ack = $client->read();
							if (!is_null($ack)) {
								switch ($ack['result']) {
									case 'ack':
										$this->_renewLastUpdated();
										$prev_last_updated = $this->last_updated;
										$alive = TRUE;
									break;

									case 'error':
									case 'dead':
										$alive = FALSE;
										trigger_localised_error('HIPO0048', E_USER_WARNING, $ack['msg'].' (TaskID #'.$this->_hipo_vars['server_taskid'].')');
									break;

									default :
										trigger_localised_error('HIPO0047', E_USER_WARNING, print_r($ack, TRUE).' (TaskID #'.$this->_hipo_vars['server_taskid'].')');
								}//end switch ($ack['result'])
							}//end if (!is_null($ack))
						}//end if ($client->write())
						$client->disconnect();
					}//end if ($client->connect())

					// not alive, die
					if (!$alive) {
						$this->abort();
						return FALSE;
					}

				}//end if ($time_since_update > SQ_HIPO_SERVER_STATUS_CHECK_THRESHOLD)
			}//end else ($this->last_updated > $prev_last_updated)

			$this->_hipo_vars['timeout_prev_last_update'] = $prev_last_updated;

		}//end if ($this->_current_mode != 'server')

		return TRUE;

	}//end load()


	/**
	* Save the HIPO vars to the database OR delete from database if complete
	*
	* @param boolean	$ignore_running_mode	whether to ignore the running mode check or not
	*											(SHOULD ONLY BE USED BY setRunningMode())
	*
	* @return boolean
	* @access public
	*/
	function save($ignore_running_mode=FALSE)
	{
		if ((!$GLOBALS['SQ_SYSTEM']->userRoot()) && (!$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) && ($this->userid != $GLOBALS['SQ_SYSTEM']->currentUserId())) {
			trigger_localised_error('HIPO0039', E_USER_WARNING, $this->code_name);
			return FALSE;
		}

		// if we have called save and we aren't the running version, dont save
		if (!$ignore_running_mode && $this->_current_mode != $this->_hipo_vars['running_mode']) {
			return FALSE;
		}

		$this->_updateProgress();

		// Second let's decide, depening on the circumstances, whether to delete or update
		$delete = FALSE;
		// if we have aborted, delete right now
		if ($this->_hipo_vars['aborted']) {
			$delete = TRUE;
		} else if ($this->complete()) {
			// if we are running from the web, we can delete the DB entry
			// because we are now done with it
			if ($this->_current_mode == 'web') {
				$delete = TRUE;

			// running from the server/cmdline
			} else {

				// The idea here is that if we are the top hipo job AND we were initialised in a web script
				// then we need to leave the DB entry so that the refreshing we script can get the data one last time
				if ($this->code_name != $this->source_code_name || $this->_hipo_vars['initialise_mode'] != 'web') {
					$delete = TRUE;
				}

			}// endif

		}// end if

		$now = time();

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db3');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$db = MatrixDAL::getDb();
		// now we just do what we are told
		if ($delete) {
			try {
				$bind_vars['code_name'] = $this->code_name;
				MatrixDAL::executeQuery('core', 'deleteHipoJobFromCodeName', $bind_vars);
			} catch (Exception $e) {
				throw new Exception('Unable to delete HIPO job with code name: '.$code_name.' due to database error: '.$e->getMessage());
			}
		} else {

			try {
				$bind_vars	= Array (
								'hipo_vars'		=> serialize($this->_hipo_vars),
								'options'		=> serialize($this->_options),
								'steps'			=> serialize($this->_steps),
								'running_vars'	=> serialize($this->_running_vars),
								'last_updated'	=> ts_iso8601($now),
								'code_name'		=> $this->code_name,
							  );
				MatrixDAL::executeQuery('core', 'updateHipoJobVars', $bind_vars);
			} catch (Exception $e) {
				throw new Exception('Unable to update HIPO job vars due to database error: '.$e->getMessage());
			}

		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		$this->last_updated = $now;

		return TRUE;

	}//end save()


	/**
	* Update progress
	*
	* @return void
	* @access private
	*/
	function _updateProgress()
	{
		// First let's make sure that everything is in order with the hipo vars and our complete status
		if (!$this->complete()) {
			$step = $this->_hipo_vars['current_step'];

			$prev_step_per = ($step / count($this->_steps)) * 100;
			$this_step_per = ($this->complete()) ? 0 : ((1 / count($this->_steps)) * $this->_steps[$step]['percent_done']);
			$this->_hipo_vars['percent_done'] = round($prev_step_per + $this_step_per, 2);

			if (!empty($this->_steps[$step]['complete'])) {
				$this->getNextStep();
			}

		}// end if

		// if we have completed then we need to re-set the running mode back to the init mode
		if ($this->complete()) {
			$this->_hipo_vars['running_mode'] = $this->_hipo_vars['initialise_mode'];
		}

	}//end _updateProgress()


	/**
	* Updates the last_updated time directly in the database, rather than
	* modifying the hipo_job object and waiting for it to save()
	*
	* @param int	$new_time	the time to set the last_updated time to - NULL defaults to the current time
	*
	* @return void
	* @access private
	*/
	function _renewLastUpdated($new_time=NULL)
	{
		if (is_null($new_time)) $new_time = time();

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db3');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$db = $GLOBALS['SQ_SYSTEM']->db;

		$sql = 	'UPDATE
					sq_hipo_job
				SET
					last_updated	= :new_time
				WHERE
					code_name	= :code_name';

		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'new_time',  ts_iso8601($new_time));
		MatrixDAL::bindValueToPdo($query, 'code_name', $this->code_name);
		MatrixDAL::execPdoQuery($query);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

	}//end _renewLastUpdated()


	/**
	* If we are currently in a hipo_job step then it returns the job that we are executing
	* Otherwise returns NULL
	*
	* @return mixed NULL|object
	* @access private
	*/
	function &_getSubJob()
	{
		$step = $this->_hipo_vars['current_step'];
		if (isset($this->_steps[$step])) {
			if (!empty($this->_steps[$step]['hipo_job']) && !empty($this->_steps[$step]['hipo_job']['code_name']) && !empty($this->_steps[$step]['hipo_job']['job_type'])) {
				$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
				$job = $hh->getJob($this->_steps[$step]['hipo_job']['code_name']);
				return $job;
			}// end if
		}// endif

		$null = NULL; // because we need to return by ref
		return $null;

	}//end _getSubJob()


	/**
	* Returns the mode that this task was initialised in
	*
	* @return string
	* @access public
	*/
	function getInitialiseMode()
	{
		return $this->_hipo_vars['initialise_mode'];

	}//end getInitialiseMode()


	/**
	* Set the mode that we are running (ie processing) in
	* Returns whether or not the set was successful
	*
	* @param string	$mode			[web|server]
	* @param string	$server_taskid	when the $mode == 'server' this must be supplied, it is the taskid given to it by the server
	*
	* @return boolean
	* @access public
	*/
	function setRunningMode($mode, $server_taskid='')
	{
		if ($mode != 'web' && $mode != 'server') {
			return FALSE;
		}

		$sub_job = $this->_getSubJob();
		if (!is_null($sub_job)) {
			if (!$sub_job->setRunningMode($mode)) return FALSE;
		}// end if

		$this->_hipo_vars['running_mode']  = $mode;
		$this->_hipo_vars['server_taskid'] = $server_taskid;
		if ($this->save(TRUE)) {
			return TRUE;
		} else {
			return FALSE;
		}

	}//end setRunningMode()


	/**
	* Returns the running mode of this task
	*
	* @return string
	* @access public
	*/
	function getRunningMode()
	{
		return $this->_hipo_vars['running_mode'];

	}//end getRunningMode()


	/**
	* Whether we want to auto_step in the paint mode for this job or not
	*
	* @return boolean
	* @access public
	*/
	function autoStep()
	{
		$step = $this->_hipo_vars['current_step'];
		if ($this->complete()) {
			// if we are complete, but there are errors, we need to pause for them to be seen
			if (!empty($this->_hipo_vars['errors'])) {
				return FALSE;
			} else {
				return $this->_options['auto_complete'];
			}

		} else if (!empty($this->_steps[$step]['function_call'])) {
			return $this->_steps[$step]['auto_step'];

		} else if (!empty($this->_steps[$step]['hipo_job'])) {

			$sub_job = $this->_getSubJob();
			// if we are processing a sub job, it's it's decision whether to auto step or not
			if (!is_null($sub_job)) {
				return $sub_job->autoStep();

			// otherwise do what's set for this step
			} else {
				return $this->_steps[$step]['auto_step'];

			}// end if

		}// end if

		return FALSE;

	}//end autoStep()


	/**
	* Return the next step to be executed by the hipo job,
	*
	* @return int
	* @access public
	*/
	function getNextStep()
	{
		$this->_hipo_vars['current_step']++;
		for ($i = $this->_hipo_vars['current_step']; $i < count($this->_steps); $i++) {
			if (isset($this->_steps[$i]['skip_step']) && $this->_steps[$i]['skip_step']) {
				$this->_steps[$i]['complete'] = TRUE;
				$this->_steps[$i]['percent_done'] = 100;
				$this->_hipo_vars['current_step']++;

				continue;
			} else {
				break;
			}
		}

		return $this->_hipo_vars['current_step'];

	}//end getNextStep()


	/**
	* called when a step in the hip job process needs to be skipped
	*
	* @param int	$stepid	the array index of the step
	*
	* @return boolean
	* @access public
	*/
	function skipStep($stepid=0)
	{
		if (!isset($this->_steps[$stepid])) return FALSE;
		$this->_steps[$stepid]['skip_step'] = TRUE;

		return TRUE;

	}//end skipStep()


	/**
	* Paint information about the process of the HIPO
	*
	* This function calls the paint function for the step currently being run and also outputs
	* a progress bar to indicate to the user where they are in the HIPO process
	*
	* @param object		&$o		Backend_Outputter, reference to the backend outputter
	* @param boolean	$nested	TRUE if the steps are nested
	*
	* @return void
	* @access public
	*/
	function paint(&$o, $nested=FALSE)
	{
		if ($this->_current_mode != 'web') {
			trigger_localised_error('HIPO0038', E_USER_WARNING);
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
				$status_msg = ($this->complete()) ? translate('completed') : translate('unknown_error_occurred');
			}

			$o->openSection($this->getHipoName());
				$o->openField('&nbsp;');
				HIPO_Job::paintProgressBar($this->_hipo_vars['percent_done'], translate('hipo_progress', $this->getHipoName()).' - '.$status_msg);
				$o->closeField();
			$o->closeSection();
		}

		// print the error report if there are errors
		if ($this->complete() && !empty($this->_hipo_vars['errors'])) {
			echo '<p>';
			$this->_paintErrorReport(TRUE);
			echo '</p>';
			return;
		}

		if ($step_data) {
			if (!empty($this->_steps[$step]['function_call'])) {
				$fn = (empty($this->_steps[$step]['function_call']['paint_function'])) ? 'paintStepDefault' : $this->_steps[$step]['function_call']['paint_function'];
				$prefix = $this->code_name.'step'.$step;
				$this->$fn($this->_steps[$step], $o, $prefix);
			} else if (!empty($step_data['hipo_job'])) {
				$sub_job = $this->_getSubJob();
				if (!is_null($sub_job)) {
					$o->closeRaw();
						$sub_job->paint($o, TRUE);
					$o->openRaw();
				}
			} else {
				trigger_localised_error('HIPO0037', E_USER_WARNING, $this->_hipo_vars['current_step']);
			}
		}//end if

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
		if ((!$GLOBALS['SQ_SYSTEM']->userRoot()) && (!$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) && ($this->userid != $GLOBALS['SQ_SYSTEM']->currentUserId())) {
			trigger_localised_error('HIPO0036', E_USER_WARNING);
			return FALSE;
		}

		// if we are is a display mode different from the running mode, we should not be
		// doing any processing or saving - so just return TRUE here
		if ($this->_current_mode != $this->_hipo_vars['running_mode']) {
			return TRUE;
		}

		// if we are complete, then just save (which will clear the DB if needed)
		if ($this->complete()) return $this->save();

		$step = $this->_hipo_vars['current_step'];
		if ($this->_steps[$step]['running_mode'] == $this->_hipo_vars['running_mode']) {

			// call the correct process function based on the running mode
			if ($this->_hipo_vars['running_mode'] == 'web') {
				// web mode
				set_error_handler(Array(&$this, '_errorHandler'));
				$max_assets_to_process = max($this->getThreshold(), 1);
				$i = 0;
				$ret_val = TRUE;
				while (($i < $max_assets_to_process) && (!$this->_steps[$step]['complete'])) {
					$prefix = $this->code_name.'step'.$step;
					$ret_val = $ret_val && $this->processWeb($this->_steps[$step], $prefix);
					$i++;
				}
				restore_error_handler();
				return $ret_val;

			} else {
				// server mode
				$prefix = $this->code_name.'step'.$step;
				set_error_handler(Array(&$this, '_errorHandler'));
				$ret_val = $this->processServer($this->_steps[$step], $prefix);
				restore_error_handler();
				return $ret_val;
			}

		// if we are using the server
		} else if (SQ_HIPO_USE_SERVER) {

			switch ($this->_steps[$step]['running_mode']) {
				// We are currently running in a web script, but this step utilises server processing
				// let's put a call through the HIPO phone to the server
				case 'server' :

					require_once SQ_SYSTEM_ROOT.'/core/server/squiz_server_client_mysource3.inc';

					$data = Array(
								'command'			=> 'start',
								'userid'			=> $GLOBALS['SQ_SYSTEM']->currentUserId(),
								'source_code_name'	=> $this->source_code_name,
							);

					$ret_val = FALSE;
					$client = new Squiz_Server_Client_MySource3();
					if ($client->connect()) {
						if ($client->write('HIPO_Job', $data)) {
							$ack = $client->read();
							if (!is_null($ack)) {
								switch ($ack['result']) {
									case 'ack' :
										$ret_val = TRUE;
									break;

									case 'error' :
										trigger_localised_error('HIPO0054', E_USER_WARNING, $ack['msg']);
									break;

									default :
										trigger_localised_error('HIPO0053', E_USER_WARNING, print_r($ack, TRUE));

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
					// pre_echo(__FUNCTION__."::".__LINE__.">>>>>>>>>>>>>>>>> RESET RUNNING MODE TO web <<<<<<<<<<<<<<<<<<<");
					log_error('The Squiz Server reset the running mode to WEB for User ID #'.$GLOBALS['SQ_SYSTEM']->currentUserId().' and Job '.$this->source_code_name);
					return $this->setRunningMode('web');
				break;

				default :
					trigger_localised_error('HIPO0052', E_USER_WARNING, $this->_steps[$step]['running_mode']);
					$this->abort();
					return FALSE;

			}//end switch

		// if we aren't using the server this should never happen (because of what happens in initialise())
		} else {
			trigger_localised_error('HIPO0051', E_USER_ERROR);

		}// endif

	}//end process()


	/**
	* Process the current step of a HIPO in the web interface
	*
	* This function calls the process function for the step currently being run - and that's it.
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processWeb(&$step_data, $prefix)
	{
		$process_ok = FALSE;
		if (!empty($step_data['function_call'])) {
			$fn = $step_data['function_call']['process_function'];
			$process_ok = $this->$fn($step_data, $prefix);

		} else if (!empty($step_data['hipo_job'])) {
			if (empty($step_data['hipo_job']['code_name']) || empty($step_data['hipo_job']['job_type'])) {
				$fn = $step_data['hipo_job']['init_details_function'];
				// First let's get the info needed to create this
				$job_type = '';
				$running_vars = Array();
				$options = Array();
				$hipo_vars = Array();
				$this->$fn($job_type, $running_vars, $options, $hipo_vars);

				// so they have decided to ignore this step ? that's cool
				if (empty($job_type)) {
					$process_ok = TRUE;

				} else {
					if (empty($hipo_vars)) {
						require_once SQ_SYSTEM_ROOT.'/core/hipo/jobs/'.$job_type.'.inc';
					} else {
						require_once $hipo_vars['job_dir'].'/'.$job_type.'.inc';
					}

					$init_hipo = new $job_type();

					$init_hipo->setRunningVars($running_vars);
					foreach ($options as $k => $v) {
						$init_hipo->setOption($k, $v);
					}
					foreach ($hipo_vars as $k => $v) {
						$init_hipo->setHipoVar($k, $v);
					}

					$code_name = $init_hipo->initialise($this->source_code_name);
					if ($code_name) {
						$step_data['hipo_job']['code_name'] = $code_name;
						$step_data['hipo_job']['job_type']  = $job_type;
						$process_ok = TRUE;
					}

				}// end if

			} else {
				$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
				$hipo = $hh->getJob($step_data['hipo_job']['code_name']);
				if (!is_null($hipo) && $hipo->process()) {
					$step_data['running_mode'] = $hipo->getRunningMode();
					$step_data['percent_done'] = $hipo->percentDone();
					$step_data['complete']     = $hipo->complete();
					$process_ok = TRUE;
				}

			}// end if

		} else {
			trigger_localised_error('HIPO0050', E_USER_WARNING, $this->_hipo_vars['current_step']);

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
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processServer(&$step_data, $prefix)
	{
		if (!empty($step_data['function_call'])) {
			$fn = $step_data['function_call']['process_function'];
			$prev_percent = (int) $this->_hipo_vars['percent_done'];
			while (!$step_data['complete']) {
				if (!$this->$fn($step_data, $prefix)) {
					$this->save();
					return FALSE;
				}

				$current_percent = (int) $this->_hipo_vars['percent_done'];
				if ((($current_percent != $prev_percent) && ($current_percent % 3 == 0)) || $step_data['complete']) {
					// Full save point or job is finished
					$this->save(FALSE);
					$prev_percent = $current_percent;
				} else {
					// Else progress to the next step
					$this->_updateProgress();
				}//end if
			}//end while
			return TRUE;

		} else if (!empty($step_data['hipo_job'])) {

			$fn = $step_data['hipo_job']['init_details_function'];

			// First let's get the info needed to create this
			$job_type     = '';
			$running_vars = Array();
			$options      = Array();
			$hipo_vars    = Array();
			$this->$fn($job_type, $running_vars, $options, $hipo_vars);

			if (empty($job_type)) return FALSE;

			if (empty($hipo_vars)) {
				require_once SQ_SYSTEM_ROOT.'/core/hipo/jobs/'.$job_type.'.inc';
			} else {
				require_once $hipo_vars['job_dir'].'/'.$job_type.'.inc';
			}

			$init_hipo = new $job_type();

			$init_hipo->setRunningVars($running_vars);
			foreach ($options as $k => $v) {
				$init_hipo->setOption($k, $v);
			}

			$code_name = $init_hipo->initialise($this->source_code_name);
			if (!$code_name) return FALSE;
			unset($init_hipo);

			$step_data['hipo_job']['code_name'] = $code_name;
			$step_data['hipo_job']['job_type']  = $job_type;
			$this->save();

			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$hipo = $hh->getJob($code_name, $job_type);
			if (is_null($hipo)) return FALSE;

			while (!$step_data['complete'] && $step_data['running_mode'] == 'server') {
				if (!$hipo->process()) return FALSE;
				$step_data['running_mode'] = $hipo->getRunningMode();
				$step_data['percent_done'] = $hipo->percentDone();
				$step_data['complete']     = $hipo->complete();
				$this->save();
			}

			return TRUE;

		} else {
			trigger_localised_error('HIPO0050', E_USER_WARNING, $this->_hipo_vars['current_step']);
			return FALSE;

		}// end if

	}//end processServer()


	/**
	* Cancel the HIPO and cleanup
	*
	* @param boolean $abort_source		Whether to abort the job's source job, if any
	* @param boolean $abort_dependant	Whether to abort the job's dependant job, if any
	*
	* @return boolean
	* @access public
	*/
	function abort($abort_source=TRUE, $abort_dependant=TRUE)
	{
		// If "source_code_name" and "code_name" are differnet then this is a dependant job
		// Therefore get the job's source and abort it too
		if ($abort_source && $this->source_code_name != $this->code_name) {
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$source_job = $hh->getJob($this->source_code_name);
			if (is_null($source_job) || ($source_job->source_code_name != $source_job->code_name)) {
				return FALSE;
			}
			$source_job->abort(TRUE, FALSE);
		}
		// If the job has spawned a dependant job, abort it (Bug #5838)
		if ($abort_dependant) {
			$dependant_job = $this->getDependantHipoJob();
			if (!is_null($dependant_job)) {
				$dependant_job->abort(FALSE, TRUE);
			}
		}

		// if aborting through a web script, but the processing is running on the server we need to inform the server to abort
		if ($this->_current_mode == 'web' && $this->_hipo_vars['running_mode'] == 'server') {

			// if there is no system taskid then there is nothing we can do
			if (empty($this->_hipo_vars['server_taskid'])) {
				return FALSE;
			}

			require_once SQ_SYSTEM_ROOT.'/core/server/squiz_server_client_mysource3.inc';

			$data = Array(
						'command'			=> 'abort',
						'userid'			=> $GLOBALS['SQ_SYSTEM']->currentUserId(),
						'source_code_name'	=> $this->source_code_name,
						'taskid'			=> $this->_hipo_vars['server_taskid'],
					);

			$ret_val = FALSE;
			$client = new Squiz_Server_Client_MySource3();
			if ($client->connect()) {
				if ($client->write('HIPO_Job', $data)) {
					$ack = $client->read();
					if (!is_null($ack)) {
						switch ($ack['result']) {
							case 'ack' :
								$ret_val = TRUE;
							break;

							case 'error' :
								trigger_localised_error('HIPO0048', E_USER_WARNING, $ack['msg'].' (TaskID #'.$this->_hipo_vars['server_taskid'].')');
							break;

							default :
								trigger_localised_error('HIPO0047', E_USER_WARNING, print_r($ack, TRUE).' (TaskID #'.$this->_hipo_vars['server_taskid'].')');

						}// end switch

					}// end if
				}// end if

				$client->disconnect();
			}// endif

			if ($ret_val) return TRUE;

			trigger_localised_error('HIPO0046', E_USER_WARNING);
			// fall through and delete this job, set the running mode to allow the saving
			$this->setRunningMode('web');

		}//end if

		$this->jobAborted();

		$sub_job = $this->_getSubJob();
		if (!is_null($sub_job)) $sub_job->jobAborted();

		return TRUE;

	}//end abort()


	/**
	* Called to do any clean-up for this job
	*
	* @return boolean
	* @access public
	*/
	function jobAborted()
	{
		$this->setOption('auto_complete', TRUE);
		$this->_hipo_vars['aborted'] = TRUE;
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
		require_once SQ_FUDGE_PATH.'/general/datetime.inc';

		?>
		<div class="sq-standalone-wrapper sq-error-report">
			<table class="sq-standalone-table">
				<tr class="sq-standalone-header">
					<td colspan="2">
						<?php echo translate('hipo_error_report', $this->getHipoName());?> 
						<span><?php echo translate('hipo_herder_completion', (!$this->complete()) ? translate('not') : ''); ?></span>
					</td>
				</tr>
				<tr class="sq-standalone-body">
					<td colspan="2" style="padding: 0;">
						<table class="sq-error-report-list">
							<tr>
								<th class="sq-error-report-type">Type</th>
								<th class="sq-error-report-description">Description</th>
								<th class="sq-error-report-reported"><?php echo translate('reported'); ?></th>
							</tr>
						<?php
							foreach ($this->_hipo_vars['errors'] as $error) {
								$err_type = ($error['warning']) ? translate('warning') : translate('error');
								$err_type_lowercase = strtolower($err_type);
								?>
								<tr>
									<td class="sq-error-report-type <?php echo $err_type_lowercase; ?>"><span title="<?php echo $err_type; ?>">
										<span><?php echo $err_type; ?></span>
										<!--img src="<?php echo sq_web_path('lib').'/web/images/'.$err_type_lowercase.'.png'; ?>" alt="<?php echo $err_type; ?>" title="<?php echo $err_type; ?>"/-->
									</td>
									<td class="sq-error-report-description"><?php echo $error['message']; ?></td>
									<td class="sq-error-report-reported"><?php echo readable_datetime($error['time']);?></td>
								</tr>
								<?php
							}
						?>
						</table>
					</td>
				</tr>
				<tr class="sq-standalone-footer">
					<td style="text-align:left;">
						&nbsp;
					</td>
					<td style="text-align:right;">
						<a class="sq-btn-link sq-btn-large sq-btn-blue" href="<?php echo $this->_options['on_complete_url']; ?>"><?php echo translate('continue'); ?></a>
					</td>
				</tr>
			</table>
		</div>
		<?php

	}//end _paintErrorReport()


	/**
	* A default function for painting the progess of a step
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param object	&$o			Hipo_Backend_Outputter, the backend outputter class
	* @param string	$prefix		prefix for form vars
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
	* @param float	$percent_done	the percentage of progress completed
	* @param string	$message		a status message to display above the progress bar
	* @param string	$label_class	the css class for the progress bar label ($message)
	* @param string	$percent_class	the css class for the percentage done figure ($percentage)
	* @param string	$bar_main_class	the css class for the base of the progress bar
	* @param string	$bar_done_class	the css class for the completed part of the progress bar
	*
	* @return void
	* @access public
	* @static
	*/
	public static function paintProgressBar($percent_done, $message='', $label_class='sq-hipo-progress-bar-label', $percent_class='sq-hipo-progress-bar-percent', $bar_main_class='sq-hipo-progress-bar-main', $bar_done_class='sq-hipo-progress-bar-done')
	{
		?>
		<div>
			<table class="sq-hipo-progress-bar-table <?php echo $bar_main_class; ?>">
				<tr>
					<td class="<?php echo $label_class; ?>"><b><?php echo nl2br($message); ?></b></td>
					<td class="<?php echo $percent_class; ?>"><b><?php echo round($percent_done); ?>%</b></td>
				</tr>
				<tr>
					<td colspan="2">
						<div class="sq-hipo-progress-bar-wrapper">
							<div class="sq-hipo-progress-bar-outer">
								<div class="sq-hipo-progress-bar-inner <?php echo $bar_done_class; ?>" style="width: <?php echo ($percent_done) ? round($percent_done) : '1'; ?>%">
								</div>
							</div>
						</div>
					</td>
				</tr>
			</table>
		</div>
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
	* @return void
	* @access private
	* @see HIPO_Job::process()
	*/
	function _errorHandler($err_no, $err_msg, $err_file, $err_line)
	{
		$terminate = ((E_USER_ERROR | E_ERROR) & $err_no);

		// Treat PHP 5.2 catchable fatals as PHP 5.1 fatals
		if (defined('E_RECOVERABLE_ERROR') && ($err_no == E_RECOVERABLE_ERROR)) {
			$terminate = TRUE;
		}

		// if the function didn't have an '@' prepended OR if we are about to terminate
		// catch the error
		if ((error_reporting() & $err_no) || $terminate) {
			// Strip out the file path begining
			$err_file = hide_system_root($err_file);
			$err_msg  = hide_system_root($err_msg);

			$text_msg = strip_tags(preg_replace(Array('/<br\\/?>/i', '/<p[^>]*>/i'), Array("\n", "\n\n"), $err_msg));

			// send a report to the system error log
			if (ini_get('log_errors')) {
				log_error($text_msg, $err_no, $err_file, $err_line);
			}

			$msg = $text_msg;
			if (SQ_CONF_DEBUG & 1) {
				$msg .= ' ('.$err_file.':'.$err_line.')';
			}
			$this->_addError($msg, !$terminate);

			// the hipo job is about to die, make sure the errors are saved to be discovered later
			if ($terminate) $this->save();

		}//end error_reporting

	}//end _errorHandler()


	/**
	* Returns the dependent HIPO jobs spawned by this job, if any
	*
	* @return object|null
	* @access public
	*/
	function getDependantHipoJob()
	{
		$dependant_job = NULL;
		if (!empty($this->_running_vars['dependant_hipo_job'])) {
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$dependant_job = $hh->getJob($this->_running_vars['dependant_hipo_job']);
		}

		return $dependant_job;

	}//end getDependantHipoJob()

}//end class
?>
