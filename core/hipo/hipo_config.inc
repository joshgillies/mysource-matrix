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
* $Id: hipo_config.inc,v 1.41 2012/08/30 01:09:18 ewang Exp $
*
*/


require_once SQ_LIB_PATH.'/config/config.inc';

/**
* HIPO_Config
*
* Purpose
*
*    Looks after the creation and editing of the hipo config
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.41 $
* @package MySource_Matrix
*/
class HIPO_Config extends Config
{
	/**
	* Holds all the vars for this that config vars for this config
	* In the form : Array([var name] => Array('editable' => [boolean], 'default' => [mixed scalar]))
	* @var Array()
	*/
	var $config_vars = Array(
						'SQ_HIPO_USE_SERVER'					=> Array('editable' => 1, 'default' => FALSE),
						'SQ_HIPO_SERVER_STATUS_CHECK_THRESHOLD'	=> Array('editable'	=> 1, 'default'	=> 30),
						'SQ_HIPO_TOTAL_THRESHOLD'				=> Array('editable' => 1, 'default' => 80),
					   );


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		$this->config_file = SQ_DATA_PATH.'/private/conf/hipo.inc';
		$this->_loadJobConfigVars();

	}//end constructor


	/**
	* Attempts to load the extra config vars that the jobs have into our config_vars array
	* Doesn't do anything if the config_file doesn't exist
	*
	* @return void
	* @access private
	*/
	function _loadJobConfigVars()
	{
		// if our config file doesn't exist then we can't load the job's because they require it
		if (!file_exists($this->config_file)) return;
		
		// ask each of the jobs for their config vars
		$packages = $GLOBALS['SQ_SYSTEM']->getInstalledPackages();
		
		foreach($packages as $package) {
			if ($package['code_name'] == '__core__') {
				$hipo_job_path = SQ_SYSTEM_ROOT.'/core/hipo/jobs';
			} else {
				$hipo_job_path = SQ_SYSTEM_ROOT.'/packages/'.strtolower($package['code_name']).'/hipo_jobs';
			}

			if (is_dir($hipo_job_path)) {
				require_once SQ_FUDGE_PATH.'/general/file_system.inc';
				$d = dir($hipo_job_path);
				while (FALSE !== ($file = $d->read())) {
					if ($file == '.' || $file == '..') {
						continue;
						}
					// skip hidden files
					if (strpos($file, '.') === 0) continue;

					if (!is_dir($hipo_job_path.'/'.$file)) {
						if (get_file_type($file) != 'inc') continue;
						require_once $hipo_job_path.'/'.$file;
						$type_code = substr($file, 0, strpos($file, '.'));

						if ($type_code) {
							$vars = call_user_func(Array($type_code, 'getConfigVars'));
							$this->config_vars = array_merge($this->config_vars, $vars);
						}
					}
				}//end while

				$d->close();

			}//end if

		}//end foreach

	}//end _loadJobConfigVars()


	/**
	* Rewrites the conf file with current variables. It returns the boolean value
	* indicating whether the file was written
	*
	* @param array		$vars				the array with any new values (that are allowed to be edited)
	*										Array('[config_var_name]' => [scalar value])
	* @param boolean	$backup_existing	whether we should backup the existing config file first
	*
	* @return boolean
	* @access public
	*/
	function save($vars, $backup_existing=FALSE)
	{
		// if the config file exists, just do a normal save
		if (file_exists($this->config_file)) {
			return parent::save($vars, $backup_existing);

		// else this is the first time that we have saved a config, things need to be done a little differently
		} else {

			// first we save as normal, but we won't have the jobs config vars in there
			// (no need to backup either because the file doesn't exist)
			if (!parent::save($vars, FALSE)) return FALSE;
			// then we load all jobs configs (which can happen now that there is a
			// config file that they can require) and save again to put them in the file
			$this->_loadJobConfigVars();
			return parent::save($vars, FALSE);

		}//end if

	}//end save()


	/**
	* Whether the current user can acquire the lock
	*
	* @return boolean
	* @access public
	* @see MySource::acquireLock()
	*/
	function canAcquireLock()
	{
		// need to be root
		return $GLOBALS['SQ_SYSTEM']->userRoot();

	}//end canAcquireLock()


	/**
	* Does the current user have writeAccess() to this config ?
	*
	* @return boolean
	* @access public
	*/
	function writeAccess()
	{
		if (!parent::writeAccess()) return FALSE;
		if (!$GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS)) {
			return TRUE;
		}

		// need to be root
		return $GLOBALS['SQ_SYSTEM']->userRoot();

	}//end writeAccess()


	/**
	* Paints the interface for our config options.
	*
	* If you (as a sub-class) put your config vars so that when they are submitted appear in
	* $_POST[get_class($this)] then you probably won't need to override processBackend()
	*
	* @param object	&$o	Backend_Outputter, reference to the backend outputter
	*
	* @return void
	* @access public
	* @see processBackend()
	*/
	function paintBackend(&$o)
	{
		parent::paintBackend($o);

		$write_access = $this->writeAccess();
		$class = get_class_lower($this);

		$o->openSection(translate('hipo_server_configuration'));

			// writing out standard fields, which are not specific to any single job
			$o->openField(translate('run_on_squiz_server'));
			echo '<span class="sq-inline-fields-wrapper">';
				if ($write_access) {
					combo_box($class.'[SQ_HIPO_USE_SERVER]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_HIPO_USE_SERVER);
				} else {
					echo (SQ_HIPO_USE_SERVER) ? translate('yes') : translate('no');
				}
			echo '</span>';
			$o->closeField();

			// server check threshold config entry
			$o->openField(translate('hipo_status_check_threshold'));
			echo '<span class="sq-inline-fields-wrapper">';
				if ($write_access) {
					text_box($class.'[SQ_HIPO_SERVER_STATUS_CHECK_THRESHOLD]', SQ_HIPO_SERVER_STATUS_CHECK_THRESHOLD, 1);
					echo '&nbsp;';
				} else {
					echo SQ_HIPO_SERVER_STATUS_CHECK_THRESHOLD;
				}
				echo translate('seconds');
				$o->note(translate('hipo_status_check_threshold_note'));
			echo '</span>';
			$o->closeField();

			// total threshold HIPO config entry
			$o->openField(translate('hipo_freestyle_threashold'));
			echo '<span class="sq-inline-fields-wrapper">';
				if ($write_access) {
					text_box($class.'[SQ_HIPO_TOTAL_THRESHOLD]', SQ_HIPO_TOTAL_THRESHOLD, 1);
					echo '&nbsp;';
				} else {
					echo SQ_HIPO_TOTAL_THRESHOLD;
				}
				echo translate('percent');
				$o->note(translate('hipo_threshold_note'));
			echo '</span>';
			$o->closeField();

			// looping through job class files and painting their backends
			$this->paintJobsBackends($o, $class);

		$o->closeSection();

		if ($write_access) $o->commitButton('', TRUE);

	}//end paintBackend()


	/**
	* One crazy function which calls paintBackend function of each job
	*
	* @param object	&$o		Backend_Outputter, reference to the backend outputter
	* @param string	$class	The name of the job class
	*
	* @return void
	* @access private
	*/
	function paintJobsBackends(&$o, $class)
	{
		$packages = $GLOBALS['SQ_SYSTEM']->getInstalledPackages();

		foreach($packages as $package) {
			if ($package['code_name'] == '__core__') {
				$hipo_job_path = SQ_SYSTEM_ROOT.'/core/hipo/jobs';
			} else {
				$hipo_job_path = SQ_SYSTEM_ROOT.'/packages/'.$package['code_name'].'/hipo_jobs';
				//include locale strings for hipo jobs because they are not assets and the strings are in package locale strings
				$GLOBALS['SQ_SYSTEM']->lm->includePackageStrings($package['code_name']);
			}

			if (is_dir($hipo_job_path)) {
				$d = dir($hipo_job_path);

				while (FALSE !== ($file = $d->read())) {
					if ($file == '.' || $file == '..') {
						continue;
					}
					// skip hidden files
					if (strpos($file, '.') === 0) continue;

					if (!is_dir($hipo_job_path.'/'.$file)) {
						if (get_file_type($file) != 'inc') continue;
						require_once $hipo_job_path.'/'.$file;
						$type_code = substr($file, 0, strpos($file, '.'));

						$write_access = $this->writeAccess();
						if ($type_code) {
							eval($type_code.'::paintConfig($o, $class, $write_access);');
						}
					}
				}//end while

				$d->close();

			}//end if

		}//end foreach

	}//end paintJobsBackends()


}//end class

?>
