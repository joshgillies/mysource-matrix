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
* $Id: hipo_config.inc,v 1.17 2004/06/10 11:10:19 brobertson Exp $
* $Name: not supported by cvs2svn $
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
* @version $Version$ - 1.0
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
		'SQ_HIPO_USE_SERVER'					=> Array('editable' => 1, 'default' => false),
		'SQ_HIPO_TOTAL_THRESHOLD'				=> Array('editable' => 1, 'default' => 80),
	);

	/**
	* Constructor
	*
	*/
	function HIPO_Config()
	{
		$this->Config();
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
		$hipo_job_path = SQ_SYSTEM_ROOT.'/core/hipo/jobs';
		if (is_dir($hipo_job_path)) {
			require_once SQ_FUDGE_PATH.'/general/file_system.inc';
			$d = dir($hipo_job_path);
			while (false !== ($file = $d->read())) {
				if ($file == '.' || $file == '..') continue;
				// skip hidden files
				if (strpos($file, '.') == 0) continue;

				if (!is_dir($hipo_job_path.'/'.$file)) {
					require_once $hipo_job_path.'/'.$file;
					if (get_file_type($file) != 'inc') continue;
					$type_code = substr($file, 0, strpos($file, '.'));

					if ($type_code) {
						eval('$vars = '.$type_code.'::getConfigVars();');
						$this->config_vars = array_merge($this->config_vars, $vars);
					}
				}
			}// end while

			$d->close();

		}// end if

	}//end _loadJobConfigVars();


	/**
	* Rewrites the conf file with current variables
	*
	* @param Array		$vars				the array with any new values (that are allowed to be edited)
	*										Array('[config_var_name]' => [scalar value])
	* @param boolean	$backup_existing	whether we should backup the existing config file first
	*
	* @return boolean	indicates whether the file was written
	* @access public
	*/
	function save($vars, $backup_existing=false)
	{
		// if the config file exists, just do a normal save
		if (file_exists($this->config_file)) {
			return parent::save($vars, $backup_existing);

		// else this is the first time that we have saved a config, things need to be done a little differently
		} else {

			// first we save as normal, but we won't have the jobs config vars in there
			// (no need to backup either because the file doesn't exist)
			if (!parent::save($vars, false)) return false;
			// then we load all jobs configs (which can happen now that there is a 
			// config file that they can require) and save again to put them in the file
			$this->_loadJobConfigVars();
			return parent::save($vars, false);

		}// end if

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
		if (!parent::writeAccess()) return false;
		if (!empty($GLOBALS['SQ_INSTALL'])) return true;
		// need to be root
		return $GLOBALS['SQ_SYSTEM']->userRoot();

	}//end writeAccess()


	/**
	* Paints the interface for our config options.
	*
	* If you (as a sub-class) put your config vars so that when they are submitted appear in 
	* $_POST[get_class($this)] then you probably won't need to override processBackend()
	*
	* @param object Backend_Outputter	&$o			reference to the backend outputter
	*
	* @return void
	* @access public
	* @see processBackend()
	*/
	function paintBackend(&$o)
	{
		parent::paintBackend($o);

		$write_access = $this->writeAccess();
		$class = get_class($this);

		$o->openSection('HIPO Server Configuration');

			// writing out standard fields, which are not specific to any single job
			$o->openField('Use HIPO Server');
			if($write_access) {
				combo_box($class.'[SQ_HIPO_USE_SERVER]', Array('1' => 'Yes', '0' => 'No'), false, SQ_HIPO_USE_SERVER);
			} else {
				echo (SQ_HIPO_USE_SERVER) ? 'Yes' : 'No';
			}
			$o->closeField();

			// total threshold HIPO config entry
			$o->openField('Total Threshold');
			if($write_access) {
				text_box($class.'[SQ_HIPO_TOTAL_THRESHOLD]', SQ_HIPO_TOTAL_THRESHOLD, 5);
			} else {
				echo SQ_HIPO_TOTAL_THRESHOLD;
			}
			$o->closeField();

			// looping through job class files and painting their backends
			$this->paintJobsBackends($o, $class);

		$o->closeSection();

		if ($write_access) $o->commitButton('Commit', true);

	}//end paintBackend()


	/**
	* One crazy function which calls paintBackend function of each job
	*
	* @return void
	* @access private
	*/
	function paintJobsBackends(&$o, $class)
	{
		$hipo_job_path = SQ_SYSTEM_ROOT.'/core/hipo/jobs';
		if (is_dir($hipo_job_path)) {
			//require_once SQ_FUDGE_PATH.'/general/file_system.inc';
			$d = dir($hipo_job_path);
			while (false !== ($file = $d->read())) {
				if ($file == '.' || $file == '..') continue;
				// skip hidden files
				if (strpos($file, '.') == 0) continue;

				if (!is_dir($hipo_job_path.'/'.$file)) {
					require_once $hipo_job_path.'/'.$file;
					if (get_file_type($file) != 'inc') continue;
					$type_code = substr($file, 0, strpos($file, '.'));

					if ($type_code) {
						eval($type_code.'::paintConfig($o, $class);');
					}
				}
			}// end while

			$d->close();

		}// end if

	}//end paintJobsBackends();


}//end class

?>
