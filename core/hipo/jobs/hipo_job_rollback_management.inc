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
* $Id: hipo_job_rollback_management.inc,v 1.5 2012/08/30 01:09:19 ewang Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';
require_once SQ_INCLUDE_PATH.'/rollback_management.inc';

/**
* Highly Intensive Processing Object (HIPO) to delete and/or update Rollback data
*
* <pre>
* Initial Running Vars :
*	enable_rollback		- When set, rollback will be enabled
*	disable_rollback	- When set, rollback will be disabled
*	reset_rollback		- When set, rollback data will be reset
*	rollback_date		- Rollback data will be set to the specified date when enabled / disabled
*	purge_fv_date		- File versioning entries will be purged before the specified date
*	purge_rb_date		- Rollback entries will be purged before the specified date
* </pre>
*
* @author  Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Rollback_Management extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name	a unique codename the HIPO
	*/
	function HIPO_Job_Rollback_Management($code_name='')
	{
		// We'll do our own transactions thanks
		$this->uses_trans = FALSE;

		$this->HIPO_Job($code_name);

	}//end constructor


	/**
	* Paints backend of this job
	*
	* @param object	&$o		reference to the backend outputter
	* @param string	$class	calling class name
	*
	* @return void
	* @access private
	*/
	public static function paintConfig(&$o, $class, $write_access)
	{

	}//end paintConfig()


	/**
	* Sets some info about thresholds that this hipo requires.
	*
	* @return array
	* @access public
	*/
	public static function getConfigVars()
	{
		return Array(
				'SQ_HIPO_ROLLBACK_MANAGEMENT_THRESHOLD'	=> Array('editable'	=> 1, 'default'	=> 1),
			   );

	}//end getConfigVars()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('Rollback Management');


	}//end getHipoName()


	/**
	* Returns the steps in this hipo, possibly with localised step names
	* and messages
	*
	* @return string
	* @access public
	*/
	function getInitialStepData()
	{
		return Array(
				Array(
					'name'			=> translate('Changing Rollback System Config Setting'),

					'function_call'	=> Array(
										'process_function'	=> 'changeRollbackSetting',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'allow_cancel'	=> FALSE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('Configuring Database Triggers'),

					'function_call'	=> Array(
										'process_function'	=> 'configureDBTriggers',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'allow_cancel'	=> FALSE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('Opening Database Transaction'),

					'function_call'	=> Array(
										'process_function'	=> 'openTransaction',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'allow_cancel'	=> FALSE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('Purging File Versioning'),

					'function_call'	=> Array(
										'process_function'	=> 'purgeFileVersioning',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> ''	,
				),
				Array(
					'name'			=> translate('Resetting Rollback'),

					'function_call'	=> Array(
										'process_function'	=> 'resetRollback',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('Toggling Rollback State'),

					'function_call'	=> Array(
										'process_function'	=> 'toggleRollbackState',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('Deleting Rollback Entries'),

					'function_call'	=> Array(
										'process_function'	=> 'deleteRollbackEntries',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('Aliging Rollback Entries'),

					'function_call'	=> Array(
										'process_function'	=> 'alignRollbackEntries',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('Deleting Redundant Rollback Entries'),

					'function_call'	=> Array(
										'process_function'	=> 'deleteRedundantRollbackEntries',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('Completing Database Transaction'),

					'function_call'	=> Array(
										'process_function'	=> 'completeTransaction',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'allow_cancel'	=> FALSE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
			   );

	}//end getInitialStepData()


	/**
	* Returns the current threshold
	*
	* Returns a value that represents the total percentage that this hipo
	* would run in freestyle mode in its current state
	*
	* @return float
	* @access public
	*/
	function getThresholdPercentageRequired()
	{
		return 100;

	}//end getThresholdPercentageRequired()


	/**
	* Prepares the running vars
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		require SQ_DATA_PATH.'/private/db/table_columns.inc';

		$this->_running_vars['rollback_table_columns'] = $tables;
		$this->_running_vars['rollback_table_names'] = get_rollback_table_names();
		$this->_running_vars['start_job'] = TRUE;

		return parent::prepare();

	}//end prepare()


	/**
	* Performs the duties of this hipo, without showing any output
	*
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		if (!$this->changeRollbackSetting($this->_steps[0], $this->getCodeName())) {
			return FALSE;
		}

		if (!$this->configureDBTriggers($this->_steps[1], $this->getCodeName())) {
			return FALSE;
		}

		if (!$this->openTransaction($this->_steps[2], $this->getCodeName())) {
			return FALSE;
		}

		if (!$this->purgeFileVersioning($this->_steps[3], $this->getCodeName())) {
			return FALSE;
		}

		if (!$this->resetRollback($this->_steps[4], $this->getCodeName())) {
			return FALSE;
		}

		if (!$this->toggleRollbackState($this->_steps[5], $this->getCodeName())) {
			return FALSE;
		}

		if (!$this->deleteRollbackEntries($this->_steps[6], $this->getCodeName())) {
			return FALSE;
		}

		if (!$this->alignRollbackEntries($this->_steps[7], $this->getCodeName())) {
			return FALSE;
		}

		if (!$this->deleteRedundantRollbackEntries($this->_steps[8], $this->getCodeName())) {
			return FALSE;
		}

		if (!$this->completeTransaction($this->_steps[9], $this->getCodeName())) {
			return FALSE;
		}

		return TRUE;

	}//end freestyle()


	/**
	* Modifies the SQ_CONF_ROLLBACK_ENABLED value in the System Configuration
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function changeRollbackSetting(&$step_data, $prefix)
	{
		$saved = TRUE;

		// Set SQ_CONF_ROLLBACK_ENABLED in the System Config if we are about to change state
		if (($this->_running_vars['enable_rollback']) || ($this->_running_vars['disable_rollback'])) {
			$saved = FALSE;

			include_once(SQ_INCLUDE_PATH.'/system_config.inc');
			$system_conf = new System_Config();
			if (!$system_conf->canAcquireLock()) {
				trigger_localised_error('CORE0233', E_USER_NOTICE);
				// Fail
				return FALSE;
			}

			$enable_rollback = (($this->_running_vars['enable_rollback']) ? '1' : '0');

			$system_conf->acquireLock();
			$saved = $system_conf->save(Array('SQ_CONF_ROLLBACK_ENABLED' => $enable_rollback), FALSE, FALSE);
			$system_conf->releaseLock();
		} else {
			$this->skipStep(2);
		}

		$step_data['percent_done'] = 100;
		$step_data['complete']     = TRUE;

		return $saved;

	}//end changeRollbackSetting()


	/**
	* Configures the appropriate database triggers for Rollback.
	* In essence, this function mimics the "Step 2" installation script
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function configureDBTriggers(&$step_data, $prefix)
	{
		$success = TRUE;

		if (($this->_running_vars['enable_rollback']) || ($this->_running_vars['disable_rollback'])) {
			$success = FALSE;

			ob_start();

			require_once SQ_SYSTEM_ROOT.'/install/install.inc';
			require_once SQ_LIB_PATH.'/db_install/db_install.inc';
			require_once SQ_INCLUDE_PATH.'/system_config.inc';
			require_once SQ_LIB_PATH.'/file_versioning/file_versioning.inc';
			require_once SQ_FUDGE_PATH.'/general/file_system.inc';

			$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

			$cached_table_columns = Array();

			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			install_stored_relations('functions');

			if (file_exists(SQ_DATA_PATH.'/private/db/table_columns.inc')) {
			        unlink(SQ_DATA_PATH.'/private/db/table_columns.inc');
			}

			if (!db_install(SQ_CORE_PACKAGE_PATH.'/tables.xml')) {
			        $GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			        trigger_error('TABLE INSTALL FAILURE', E_USER_ERROR);
			}

			// install any tables needed by the packages
			$packages = get_package_list();

			foreach ($packages as $package) {
				$xml_file = SQ_PACKAGES_PATH.'/'.$package.'/tables.xml';
				if (file_exists($xml_file)) {
					if (!db_install($xml_file)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						trigger_error('TABLE INSTALL FAILURE', E_USER_ERROR);
					}
					install_stored_relations('functions', $package);
				}
			}

			install_stored_relations('views');

			// grant permissions to the tables for the secondary user
			grant_secondary_user_perms();

			$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();

			if (!$fv->initRepository()) {
			        trigger_error('Unable to initialise File Versioning Repository', E_USER_ERROR);
			}

			ob_end_clean();

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

			$success = TRUE;
		}//end if

		$step_data['percent_done'] = 100;
		$step_data['complete']     = TRUE;

		return $success;

	}//end configureDBTriggers()


	/**
	* Opens a sunny-fresh database transaction in which all modifications by this HIPO will be performed
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function openTransaction(&$step_data, $prefix)
	{
		$step_data['percent_done'] = 100;
		$step_data['complete']     = TRUE;

		// Start a transaction. Why not?
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		return TRUE;

	}//end openTransaction()


	/**
	* Purges file versioning entries up to a specified date
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function purgeFileVersioning(&$step_data, $prefix)
	{
		if (!empty($this->_running_vars['purge_fv_date'])) {
			if ($this->_running_vars['start_job']) {
				$this->_running_vars['start_job'] = FALSE;
			}

			// Purge file versioning entries, 50 at a time
			$affected_rows = purge_file_versioning($this->_running_vars['purge_fv_date'], 50);

			if ($affected_rows == 0) {
				$step_data['percent_done'] = 100;
				$step_data['complete']     = TRUE;
				$this->_running_vars['start_job'] = TRUE;
				$this->skipStep(3);
				$this->skipStep(4);
				$this->skipStep(5);
				$this->skipStep(6);
			} else {
				$step_data['percent_done'] = 50;
				$step_data['complete']     = FALSE;
			}
		} else {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
			$this->_running_vars['start_job'] = TRUE;
		}

		return TRUE;

	}//end purgeFileVersioning()


	/**
	* Truncates rollback entries
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function resetRollback(&$step_data, $prefix)
	{
		if ($this->_running_vars['reset_rollback']) {
			if ($this->_running_vars['start_job']) {
				$this->_running_vars['start_job'] = FALSE;
				$this->_running_vars['to_check'] = $this->_running_vars['rollback_table_names'];
			}

			if (!empty($this->_running_vars['to_check'])) {
				reset($this->_running_vars['to_check']);
				list($key, $table_name) = each($this->_running_vars['to_check']);
				unset($this->_running_vars['to_check'][$key]);

				$step_data['message'] = sprintf(translate('Truncating rollback entries in %s'), $table_name);

				truncate_rollback_entries($table_name);
			}

			if (empty($this->_running_vars['to_check'])) {
				$step_data['percent_done'] = 100;
				$step_data['complete']     = TRUE;
				$this->_running_vars['start_job'] = TRUE;
			} else {
				$step_data['percent_done'] = (int)(((count($this->_running_vars['rollback_table_names']) - count($this->_running_vars['to_check'])) / count($this->_running_vars['rollback_table_names'])) * 100);
				$step_data['complete']     = FALSE;
			}
		} else {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
			$this->_running_vars['start_job'] = TRUE;
		}

		return TRUE;

	}//end resetRollback()


	/**
	* Enables or disables Rollback depending on the state change requested
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function toggleRollbackState(&$step_data, $prefix)
	{
		if ($this->_running_vars['start_job']) {
			$this->_running_vars['start_job'] = FALSE;
			$this->_running_vars['to_check'] = $this->_running_vars['rollback_table_names'];

			if ($this->_running_vars['enable_rollback']) {
				$rbfound = rollback_found($this->_running_vars['to_check']);
				if ($rbfound) {
					$step_data['percent_done'] = 100;
					$step_data['complete']     = TRUE;
					$this->_running_vars['start_job'] = TRUE;
					$this->skipStep(5);
					$this->skipStep(6);
					return TRUE;
				}
			}
		}

		if ((!empty($this->_running_vars['to_check'])) && (($this->_running_vars['enable_rollback']) || ($this->_running_vars['disable_rollback']))) {
			reset($this->_running_vars['to_check']);
			list($key, $table_name) = each($this->_running_vars['to_check']);
			unset($this->_running_vars['to_check'][$key]);

			if ($this->_running_vars['enable_rollback']) {
				$step_data['message'] = sprintf(translate('Opening rollback entries in %s'), $table_name);

				open_rollback_entries($table_name, $this->_running_vars['rollback_table_columns'], $this->_running_vars['rollback_date']);
			} else if ($this->_running_vars['disable_rollback']) {
				$step_data['message'] = sprintf(translate('Closing rollback entries in %s'), $table_name);

				close_rollback_entries($table_name, $this->_running_vars['rollback_date']);
			}

			if (empty($this->_running_vars['to_check'])) {
				$step_data['percent_done'] = 100;
				$step_data['complete']     = TRUE;
				$this->_running_vars['start_job'] = TRUE;
				$this->skipStep(5);
				$this->skipStep(6);
			} else {
				$step_data['percent_done'] = (int)(((count($this->_running_vars['rollback_table_names']) - count($this->_running_vars['to_check'])) / count($this->_running_vars['rollback_table_names'])) * 100);
				$step_data['complete']     = FALSE;
			}
		} else {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
			$this->_running_vars['start_job'] = TRUE;
		}

		return TRUE;

	}//end toggleRollbackState()


	/**
	* Deletes rollback entries up to a specified date
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function deleteRollbackEntries(&$step_data, $prefix)
	{
		if (!empty($this->_running_vars['purge_rb_date'])) {
			if ($this->_running_vars['start_job']) {
				$this->_running_vars['start_job'] = FALSE;
				$this->_running_vars['to_check'] = $this->_running_vars['rollback_table_names'];
			}

			reset($this->_running_vars['to_check']);
			list($key, $table_name) = each($this->_running_vars['to_check']);
			unset($this->_running_vars['to_check'][$key]);

			// Delete rollback entries from the specified table
			$step_data['message'] = sprintf(translate('Deleting rollback entries from %s'), $table_name);

			$affected_rows = delete_rollback_entries($table_name, $this->_running_vars['purge_rb_date']);

			// No more tables? Then we're done!
			if (empty($this->_running_vars['to_check'])) {
				$step_data['percent_done'] = 100;
				$step_data['complete']     = TRUE;
				$this->_running_vars['start_job'] = TRUE;
			} else {
				// Report the correct percentage before proceeding with this step
				$step_data['percent_done'] = (int)(((count($this->_running_vars['rollback_table_names']) - count($this->_running_vars['to_check'])) / count($this->_running_vars['rollback_table_names'])) * 100);

				$step_data['complete']     = FALSE;
			}
		} else {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
			$this->_running_vars['start_job'] = TRUE;
		}

		return TRUE;

	}//end deleteRollbackEntries()


	/**
	* Aligns rollback entries to a specified date
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function alignRollbackEntries(&$step_data, $prefix)
	{
		if (!empty($this->_running_vars['purge_rb_date'])) {
			if ($this->_running_vars['start_job']) {
				$this->_running_vars['start_job'] = FALSE;
				$this->_running_vars['to_check'] = $this->_running_vars['rollback_table_names'];
			}

			reset($this->_running_vars['to_check']);
			list($key, $table_name) = each($this->_running_vars['to_check']);
			unset($this->_running_vars['to_check'][$key]);

			// Align rollback entries in the specified table
			$affected_rows = align_rollback_entries($table_name, $this->_running_vars['purge_rb_date']);
			$step_data['message'] = sprintf(translate('Aligning rollback entries in %s'), $table_name);


			// No more tables? Then we're done!
			if (empty($this->_running_vars['to_check'])) {
				$step_data['percent_done'] = 100;
				$step_data['complete']     = TRUE;
				$this->_running_vars['start_job'] = TRUE;
			} else {
				// Report the correct percentage before proceeding with this step
				$step_data['percent_done'] = (int)(((count($this->_running_vars['rollback_table_names']) - count($this->_running_vars['to_check'])) / count($this->_running_vars['rollback_table_names'])) * 100);

				$step_data['complete']     = FALSE;
			}
		} else {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
			$this->_running_vars['start_job'] = TRUE;
		}

		return TRUE;

	}//end alignRollbackEntries()


	/**
	* Delete redundant entries from the Rollback table
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function deleteRedundantRollbackEntries(&$step_data, $prefix)
	{
		if (!empty($this->_running_vars['delete_redundant_entries'])) {
			if ($this->_running_vars['start_job']) {
				$this->_running_vars['start_job'] = FALSE;
				$this->_running_vars['to_check'] = $this->_running_vars['rollback_table_names'];
			}

			if (!empty($this->_running_vars['to_check'])) {
				reset($this->_running_vars['to_check']);
				list($key, $table_name) = each($this->_running_vars['to_check']);
				unset($this->_running_vars['to_check'][$key]);

				$step_data['message'] = sprintf(translate('Deleting redundant rollback entries in %s'), $table_name);

				delete_redundant_rollback_entries($table_name);
			}


			if (empty($this->_running_vars['to_check'])) {
				$step_data['percent_done'] = 100;
				$step_data['complete']     = TRUE;
				$this->_running_vars['start_job'] = TRUE;
			} else {
				$step_data['percent_done'] = (int)(((count($this->_running_vars['rollback_table_names']) - count($this->_running_vars['to_check'])) / count($this->_running_vars['rollback_table_names'])) * 100);
				$step_data['complete']     = FALSE;
			}
		} else {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
			$this->_running_vars['start_job'] = TRUE;
		}

		return TRUE;

	}//end deleteRedundantRollbackEntries()


	/**
	* Completes the database transaction started by this HIPO
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function completeTransaction(&$step_data, $prefix)
	{
		$step_data['percent_done'] = 100;
		$step_data['complete']     = TRUE;

		// Transaction processed
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end completeTransaction()


}//end class
?>
