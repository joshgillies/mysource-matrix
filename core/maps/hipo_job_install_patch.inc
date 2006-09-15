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
* $Id: hipo_job_install_patch.inc,v 1.18 2006/09/15 01:32:13 rhoward Exp $
*
*/


require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to install a system patch
*
* <pre>
* Initial Running Vars :
*    int    patchid  - the ID of the patch we are installing
* </pre>
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.18 $
* @package MySource_Matrix
* @subpackage MAPS
*/
class HIPO_Job_Install_Patch extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name		a unique codename the HIPO
	*/
	function HIPO_Job_Install_Patch($code_name='')
	{
		$this->HIPO_Job($code_name);

	}//end constructor


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* @return string
	* @access public
	*/
	function getCodeName()
	{
		return parent::getCodeName().$this->_running_vars['patchid'];

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('hipo_name_install_patch');

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
						'name'			=> translate('hipo_confirmation'),
						'function_call'	=> Array(
												'paint_function'   => 'paintConfirmation',
												'process_function' => 'processConfirmation',
										   ),
						'running_mode'	=> 'web',
						'auto_step'		=> false,
						'skip_step'		=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
						'allow_cancel'  => true,
					),
					Array(
						'name'			=> translate('hipo_checking_versions'),
						'function_call'	=> Array(
												'paint_function'   => 'paintPleaseWait',
												'process_function' => 'processCheckVersions',
										   ),
						'running_mode'	=> 'server',
						'auto_step'		=> true,
						'skip_step'		=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
						'allow_cancel'     => true,
					),
					Array(
						'name'			=> translate('hipo_checking_file_updates'),
						'function_call'	=> Array(
												'paint_function'   => 'paintPleaseWait',
												'process_function' => 'processCheckFiles',
										   ),
						'running_mode'	=> 'server',
						'auto_step'		=> true,
						'skip_step'		=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
						'allow_cancel'  => false,
					),
					Array(
						'name'			=> translate('hipo_backing_up_modified_files'),
						'function_call'	=> Array(
												'paint_function'   => 'paintPleaseWait',
												'process_function' => 'processBackupFiles',
										   ),
						'running_mode'	=> 'server',
						'auto_step'		=> true,
						'skip_step'		=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
						'allow_cancel'  => false,
					),
					Array(
						'name'			=> translate('hipo_updating_database'),
						'function_call'	=> Array(
												'paint_function'   => 'paintPleaseWait',
												'process_function' => 'processDatabaseUpdates',
										   ),
						'running_mode'	=> 'server',
						'auto_step'		=> true,
						'skip_step'		=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
						'allow_cancel'  => false,
					),
					Array(
						'name'			=> translate('hipo_installing_file_updates'),
						'function_call'	=> Array(
												'paint_function'   => 'paintPleaseWait',
												'process_function' => 'processInstallFiles',
										   ),
						'running_mode'	=> 'server',
						'auto_step'		=> true,
						'skip_step'		=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
						'allow_cancel'  => false,
					),
					Array(
						'name'			=> translate('hipo_running_installation_script'),
						'function_call'	=> Array(
												'paint_function'   => 'paintPleaseWait',
												'process_function' => 'processInstallScript',
										   ),
						'running_mode'	=> 'web',
						'auto_step'		=> true,
						'skip_step'		=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
						'allow_cancel'  => false,
					),
					Array(
						'name'			=> translate('hipo_cleaning_up'),
						'function_call'	=> Array(
												'paint_function'   => 'paintPleaseWait',
												'process_function' => 'processCleanUp',
										   ),
						'running_mode'	=> 'server',
						'auto_step'		=> true,
						'skip_step'		=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
						'allow_cancel'  => false,
					),
			   );

	}//end getInitialStepData()


	/**
	* Set up vars and database information that the HIPO will need to run
	*
	* @param string	$source_code_name	the code name of another job that wants to use this job
	*
	* @return string	returns the code_name for the new job
	* @access public
	*/
	function initialise($source_code_name=null)
	{
		if (!isset($this->_running_vars['patchid'])) {
			trigger_localised_error('HIPO0060', E_USER_WARNING);
			return '';
		}

		return parent::initialise($source_code_name);

	}//end initialise()


	/**
	* Get details about a specific patch
	*
	* @param string $patchid	the ID of the patch being uninstalled
	*
	* @return array
	* @access private
	*/
	function _getPatchDetails($patchid)
	{
		require_once SQ_SYSTEM_ROOT.'/core/maps/maps.inc';
		return MAPS::getPatchDetails($patchid);

	}//end _getPatchDetails()


	/**
	* Paints a generic 'please wait' screen for install steps
	*
	* @param array								&$step_data	a reference to the array of information about the current step
	* @param object Hipo_Backend_Outputter		&$o			the backend outputter class
	* @param string								$prefix		prefix for form vars
	*
	* @access public
	* @return boolean
	*/
	function paintPleaseWait(&$step_data, &$o, $prefix)
	{

		$o->openSection(translate('please_wait'));
			$o->openField(translate('please_wait').' - '.strtolower($step_data['name']).'...');
			$o->closeField();
		$o->closeSection();
		return true;

	}//end paintPleaseWait()


	/**
	* Paints a confirmation screen for installing the patch
	*
	* This allows the user to back out if they want to, as well as be presented
	* with any information/warnings regarding the new patch
	*
	* @param array								&$step_data	a reference to the array of information about the current step
	* @param object Hipo_Backend_Outputter		&$o			the backend outputter class
	* @param string								$prefix		prefix for form vars
	*
	* @access public
	* @return boolean
	*/
	function paintConfirmation(&$step_data, &$o, $prefix)
	{
		$o->addHiddenField('form_submitted', 1);
		$o->addOnLoad('setNextButtonStatus();');

		$patch_info = $this->_getPatchDetails($this->_running_vars['patchid']);

		$o->openSection('Important Notice');
			$o->openField('');
				?>
					<script language="Javascript">
						function setNextButtonStatus() {
							var nextButton = document.getElementById('next');
							var backupBox  = document.getElementById('maps_backup_confirm');
							if (backupBox.checked) {
								nextButton.disabled = false;
								nextButton.className = 'sq-hipo-button-enabled';
							} else {
								nextButton.disabled = true;
								nextButton.className = 'sq-hipo-button-disabled';
							}
						}
					</script>
					<img style="float: left;" src="<?php echo $o->filesPath('/images/maps/install.png'); ?>" wdth="70" height="69" />
					<span class="sq-backend-warning">
					<b><?php echo translate('maps_hipo_ensure_backup'); ?></b><br /><br />
					<?php check_box('maps_backup_confirm', '1', false, 'Javascript: setNextButtonStatus();'); ?> <b><?php echo translate('maps_hipo_backup_confirm'); ?></b>
					</span>
				<?php
			$o->closeField();
		$o->closeSection();

		$o->openSection(translate('install_options'));
			$o->openField('');
				?>
				<table>
					<tr>
						<td class="sq-backend-field" align="center">&nbsp;</td>
						<td class="sq-backend-field"><?php translate('name'); ?></td>
						<td class="sq-backend-field"><?php echo translate('description'); ?></td>
						<td class="sq-backend-field" align="center"><?php echo translate('danger'); ?></td>
					</tr>
					<tr>
						<td class="sq-backend-data" align="center" valign="top">
							<?php check_box('maps_ignore_file_checks'); ?>
						</td>
						<td class="sq-backend-data" nowrap="nowrap" valign="top">
							<?php echo translate('maps_ignore_filechecks'); ?>
						</td>
						<td class="sq-backend-data" valign="top">
							<?php echo translate('maps_ignore_local_filechecks'); ?>
						</td>
						<td class="sq-backend-data" valign="top" align="center">
							<span style="color: #FF0000;"><b><?php echo translate('high'); ?></b></span>
						</td>
					</tr>
				</table>
				<?php
			$o->closeField();
		$o->closeSection();

		$o->openSection('Please Note');
			$o->openField('');
				echo 'The details of the patch you have selected to install are shown below.<br />Please confirm that you wish to install this patch.';
			$o->closeField();
		$o->closeSection();

		$o->openSection(translate('patch_details'));
			$o->openField('Severity');
				echo $patch_info['severity'];
			$o->closeField();
			$o->openField('');
				echo '<b>'.translate('maps_file_updates').':</b>&nbsp;';
				echo (
						empty($patch_info['updated_files']) &&
						empty($patch_info['new_files']) &&
						empty($patch_info['deleted_files'])
					  ) ? translate('no') : translate('yes');
				echo '&nbsp;&nbsp;<b>'.translate('maps_database_changes').':</b>&nbsp;';
				echo (empty($patch_info['queries'])) ? translate('no') : translate('yes');
				echo '&nbsp;&nbsp;<b>'.translate('reversible').':</b>&nbsp;';
				echo ($patch_info['reversible']) ? translate('yes') : translate('no');
			$o->closeField();
			$o->openField(translate('description'));
				echo $patch_info['patch_description'];
			$o->closeField();
		$o->closeSection();

		$o->openSection(translate('change_log'));
			$i = 1;
			foreach ($patch_info['changelog'] as $change) {
				$o->openField($i.'.');
					echo $change;
				$o->closeField();
				$i++;
			}
		$o->closeSection();

		return true;

	}//end paintConfirmation()


	/**
	* Process the user's confirmation that they want to create the new link
	*
	* @param	&array	$step_data		the step data for this job
	* @param	string	$prefix			prefix to be used with this job
	*
	* @access public
	* @return boolean
	*/
	function processConfirmation(&$step_data, $prefix)
	{
		// they have to tick the box to confirm they have backed yp their system before continuing
		if (!isset($_POST['maps_backup_confirm']) || !$_POST['maps_backup_confirm']) return true;

		// process install options
		if (isset($_POST['maps_ignore_file_checks']) && $_POST['maps_ignore_file_checks']) {
			$this->_running_vars['option_ignore_file_checks'] = true;
		} else {
			$this->_running_vars['option_ignore_file_checks'] = false;
		}

		// has the next button been clicked ?
		if (isset($_REQUEST['form_submitted'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete'] = true;
		}

		return true;

	}//end processConfirmation()


	/**
	* Checks the required system and asset versions for the patch
	*
	* @param	&array	$step_data		the step data for this job
	* @param	string	$prefix			prefix to be used with this job
	*
	* @access public
	* @return boolean
	*/
	function processCheckVersions(&$step_data, $prefix)
	{
		$patch_info = $this->_getPatchDetails($this->_running_vars['patchid']);

		// To allow for a patch to be applied which doesn't change the
		// version number, any requirement that has a new_version == old_version
		// will only check to make sure that the old version was at LEAST
		// the required version.
		// For example, if your system is on RC2, but you want to patch
		// something that only needs RC1, the patch's requirement says
		// old_version = RC1, new_version=RC1, which will make the system
		// only check that the old version was at LEAST RC1. This lets us
		// not need to update the version for something just to be able
		// to add a patch (ie CMS is still version 0.1, nothing has changed)

		if (isset($patch_info['requirements']['system'])) {

			// if new and old are the same, check that old version is at least
			// sq_system_version
			if ($patch_info['requirements']['system']['old_version'] ==
				$patch_info['requirements']['system']['new_version']) {
				// if the current version is lower than the minimum required version
				if (version_compare(SQ_SYSTEM_VERSION,$patch_info['requirements']['system']['old_version']) < 0) {
					trigger_localised_error('MAPS0021', E_USER_ERROR, $patch_info['requirements']['system']['old_version'], SQ_SYSTEM_VERSION);
					return false;
				}
			} else if ($patch_info['requirements']['system']['old_version'] != SQ_SYSTEM_VERSION) {
				// the system version must be exactly equal to what is defined
				// in the patch or else the patch is patching an untested system
				trigger_localised_error('MAPS0021', E_USER_ERROR, $patch_info['requirements']['system']['old_version'], SQ_SYSTEM_VERSION);
				return false;
			}
		}

		if (isset($patch_info['requirements']['packages'])) {
			foreach ($patch_info['requirements']['packages'] as $package_code => $versions) {
				$package_path = SQ_PACKAGES_PATH.'/'.$package_code.'/package_manager_'.$package_code.'.inc';
				$package_class = 'package_manager_'.$package_code;

				if (empty($versions['old_version'])) {
					// we are installing a new package, so a previous version should not be installed
					if (is_file($package_path)) {
						// package is already installed, get its version to report to the user
						require_once $package_path;
						$package = new $package_class();

						trigger_localised_error('MAPS0025', E_USER_ERROR, strtoupper($package_code), $package->version);
						return false;
					}
				} else if ($versions['old_version'] == $versions['new_version']) {
					// if the new and old versions are the same, make sure the
					// package version is at least that of the required version.
					if (!is_file($package_path)) {
						trigger_localised_error('MAPS0022', E_USER_ERROR, $versions['old_version'], strtoupper($package_code));
						return false;
					}
					require_once $package_path;
					$package = new $package_class();

					// if the current version is lower than the minimum required version
					// then the requirement fails.
					if (version_compare($package->version,$versions['old_version']) < 0) {
						trigger_localised_error('MAPS0021', E_USER_ERROR, $patch_info['requirements']['system']['old_version'], SQ_SYSTEM_VERSION);
						return false;
					}

				} else {
					if (!is_file($package_path)) {
						trigger_localised_error('MAPS0022', E_USER_ERROR, $versions['old_version'], strtoupper($package_code));
						return false;
					}
					require_once $package_path;
					$package = new $package_class();

					if ($package->version != $versions['old_version']) {
						trigger_localised_error('MAPS0023', E_USER_ERROR, $versions['old_version'], strtoupper($package_code), $package->version);
						return false;
					}
				}
			}
		}

		if (isset($patch_info['requirements']['assets'])) {
			foreach ($patch_info['requirements']['assets'] as $type_code => $versions) {
				if (empty($versions['old_version'])) {
					// we are installing a new assset type, so a previous version should not be installed
					if ($GLOBALS['SQ_SYSTEM']->am->installed($type_code)) {
						$installed_version = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code, 'version');
						trigger_localised_error('MAPS0026', E_USER_ERROR, $type_code, $installed_version);
						return false;
					}
				} else if ($versions['old_version'] == $versions['new_version']) {
					// if the new and old versions are the same, make sure the
					// asset version is at least that of the required version.
					$installed_version = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code, 'version');

					// if the current version is lower than the minimum required version
					// then the requirement fails.
					if (version_compare($installed_version,$versions['old_version']) < 0) {
						trigger_localised_error('MAPS0024', E_USER_ERROR, $versions['old_version'], $type_code, $installed_version);
						return false;
					}

				} else {
					$installed_version = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code, 'version');
					if ($installed_version != $versions['old_version']) {
						trigger_localised_error('MAPS0024', E_USER_ERROR, $versions['old_version'], $type_code, $installed_version);
						return false;
					}
				}
			}
		}

		$step_data['percent_done'] = 100;
		$step_data['complete']     = true;
		return true;

	}//end processCheckVersions()


	/**
	* Check all files to be installed to ensure they are valid
	*
	* @param	&array	$step_data		the step data for this job
	* @param	string	$prefix			prefix to be used with this job
	*
	* @access public
	* @return boolean
	*/
	function processCheckFiles(&$step_data, $prefix)
	{
		$patch_info = $this->_getPatchDetails($this->_running_vars['patchid']);
		$patch_path = dirname($patch_info['file_path']);

		// check the new files that we will be installing
		foreach ($patch_info['new_files'] as $path => $file_info) {
			$to   = SQ_SYSTEM_ROOT.'/'.$path;
			$from = $patch_path.'/'.$path;

			if (empty($file_info['md5'])) {

				// this should be a directory
				if (!is_dir($from)) {
					trigger_localised_error('MAPS0054', E_USER_ERROR, $from);
					return false;
				}

				if (!$this->_running_vars['option_ignore_file_checks']) {
					// check that the directory does not already exist in the system
					if (is_dir($to)) {
						trigger_localised_error('MAPS0055', E_USER_ERROR, $to);
						return false;
					}
				}

			} else {
				// this should be a regular file
				if (!is_file($from)) {
					trigger_localised_error('MAPS0053', E_USER_ERROR, $from);
					return false;
				}

				if (!$this->_running_vars['option_ignore_file_checks']) {
					// check that there is no file already installed
					if (is_file($to)) {
						trigger_localised_error('MAPS0052', E_USER_ERROR, $to);
						return false;
					}
				}

				// check the MD5 sum of the file to ensure it has not been changed since downloading
				if ($file_info['md5'] != md5(file_get_contents($from))) {
					trigger_localised_error('MAPS0050', E_USER_ERROR, $to);
					return false;
				}

			}//end if directory
		}

		// check the files we will be updating
		foreach ($patch_info['updated_files'] as $path => $file_info) {
			$to   = SQ_SYSTEM_ROOT.'/'.$path;
			$from = $patch_path.'/'.$path;

			if (!is_file($from)) {
				trigger_localised_error('MAPS0051', E_USER_ERROR, $from);
				return false;
			}

			if (!$this->_running_vars['option_ignore_file_checks']) {
				// make sure there is a file to replace
				if (!is_file($to)) {
					trigger_localised_error('MAPS0048', E_USER_ERROR, $to);
					return false;
				}

				// check the MD5 sum of the file to ensure it has not been changed locally
				if ($file_info['local_md5'] != md5(file_get_contents($to))) {
					trigger_localised_error('MAPS0049', E_USER_ERROR, $to);
					return false;
				}
			}

			// check the MD5 sum of the file to ensure it has not been changed since downloading
			if ($file_info['md5'] != md5(file_get_contents($from))) {
				trigger_localised_error('MAPS0050', E_USER_ERROR, $to);
				return false;
			}
		}

		// check the files we will be deleting
		foreach ($patch_info['deleted_files'] as $path => $file_info) {
			$from = SQ_SYSTEM_ROOT.'/'.$path;

			if (!$this->_running_vars['option_ignore_file_checks']) {
				// make sure there is a file to delete
				if (!is_file($from)) {
					trigger_localised_error('MAPS0056', E_USER_ERROR, $from);
					return false;
				}

				// check the MD5 sum of the file to ensure it has not been changed locally
				if ($file_info['local_md5'] != md5(file_get_contents($from))) {
					trigger_localised_error('MAPS0057', E_USER_ERROR, $from);
					return false;
				}
			}
		}

		$step_data['percent_done'] = 100;
		$step_data['complete']     = true;
		return true;

	}//end processCheckFiles()


	/**
	* Back up all files that are being modified
	*
	* Note that at this step we dont have to do extra checking as we already
	* know that the files are valid (done in the last step)
	*
	* @param	&array	$step_data		the step data for this job
	* @param	string	$prefix			prefix to be used with this job
	*
	* @access public
	* @return boolean
	*/
	function processBackupFiles(&$step_data, $prefix)
	{
		$patch_info = $this->_getPatchDetails($this->_running_vars['patchid']);
		$patch_path = dirname($patch_info['file_path']);
		$backup_path = SQ_DATA_PATH.'/private/maps/installed_patches/'.$patch_info['id'];

		// make the backup directory
		require_once(SQ_FUDGE_PATH.'/general/file_system.inc');
		if (!is_dir($backup_path) && !create_directory($backup_path)) {
			trigger_localised_error('MAPS0044', E_USER_ERROR, $backup_path, $patch_info['id']);
			return false;
		}

		foreach ($patch_info['new_files'] as $path => $file_info) {
			$from = SQ_SYSTEM_ROOT.'/'.$path;
			$to   = $backup_path.'/'.$path;

			// back up any new files if there is already a file in in the way
			if (!empty($file_info['md5']) && is_file($to)) {
				// try and create the backup directory for this file if it doesnt exist
				if (!is_dir(dirname($to)) && !create_directory(dirname($to))) {
					trigger_localised_error('MAPS0044', E_USER_ERROR, $path, $patch_info['id']);
					return false;
				}
				// backup the existing file
				if (!copy($from, $to)) {
					trigger_localised_error('MAPS0046', E_USER_ERROR, $from);
					return false;
				}
			}
		}

		foreach ($patch_info['updated_files'] as $path => $file_info) {
			$from = SQ_SYSTEM_ROOT.'/'.$path;
			$to   = $backup_path.'/'.$path;

			// try and create the backup directory for this file if it doesnt exist
			if (!is_dir(dirname($to)) && !create_directory(dirname($to))) {
				trigger_localised_error('MAPS0044', E_USER_ERROR, $path, $patch_info['id']);
				return false;
			}

			// backup the existing file
			if ($this->_running_vars['option_ignore_file_checks'] && !is_file($from)) {
				// if there is no file to backup, use the one from the patch instead to we can
				// still reverse it if required - except if there is already a backup up version of
				// the file from a previous run
				if (is_file($to)) continue;
				$from = $patch_path.'/'.$path;
			}

			if (!copy($from, $to)) {
				trigger_localised_error('MAPS0046', E_USER_ERROR, $from);
				return false;
			}
		}

		foreach ($patch_info['deleted_files'] as $path => $file_info) {
			$from = SQ_SYSTEM_ROOT.'/'.$path;
			$to   = $backup_path.'/'.$path;

			// try and create the backup directory for this file if it doesnt exist
			if (!is_dir(dirname($to)) && !create_directory(dirname($to))) {
				trigger_localised_error('MAPS0044', E_USER_ERROR, $path, $patch_info['id']);
				return false;
			}

			// backup the existing file
			if ($this->_running_vars['option_ignore_file_checks'] && !is_file($from)) {
				// if there is no file to backup, use the one from the patch instead to we can
				// still reverse it if required - except if there is already a backup up version of
				// the file from a previous run
				if (is_file($to)) continue;
				$from = $patch_path.'/'.$path;
			}

			require_once SQ_FUDGE_PATH.'/general/file_system.inc';
			if (!copy_file($from, $to)) {
				if (!$this->_running_vars['option_ignore_file_checks']) {
					trigger_localised_error('MAPS0046', E_USER_ERROR, $from);
					return false;
				} else {
					trigger_localised_error('MAPS0045', E_USER_NOTICE, $from);
				}
			}
		}

		$step_data['percent_done'] = 100;
		$step_data['complete']     = true;
		return true;

	}//end processBackupFiles()


	/**
	* Update files from the patch
	*
	* @param	&array	$step_data		the step data for this job
	* @param	string	$prefix			prefix to be used with this job
	*
	* @access public
	* @return boolean
	*/
	function processInstallFiles(&$step_data, $prefix)
	{
		$patch_info = $this->_getPatchDetails($this->_running_vars['patchid']);
		$patch_path = dirname($patch_info['file_path']);

		foreach ($patch_info['new_files'] as $path => $file_info) {
			$to   = SQ_SYSTEM_ROOT.'/'.$path;
			$from = $patch_path.'/'.$path;

			if (empty($file_info['md5'])) {
				if (!mkdir($to)) {
					trigger_localised_error('MAPS0042', E_USER_ERROR, $path);
					return false;
				}
			} else {
				if (!copy($from, $to)) {
					trigger_localised_error('MAPS0041', E_USER_ERROR, $path);
					return false;
				}
			}
		}

		foreach ($patch_info['updated_files'] as $path => $file_info) {
			$to   = SQ_SYSTEM_ROOT.'/'.$path;
			$from = $patch_path.'/'.$path;

			if (!copy($from, $to)) {
				trigger_localised_error('MAPS0028', E_USER_ERROR, $path);
				return false;
			}
		}

		foreach ($patch_info['deleted_files'] as $path => $file_info) {
			$from = SQ_SYSTEM_ROOT.'/'.$path;

			// if we are ignoring file integrity checks, dont throw an error if the
			// deleted file doesnt exist
			if ($this->_running_vars['option_ignore_file_checks']) {
				if (!is_file($from)) continue;
			}

			if (!unlink($from)) {
				trigger_localised_error('MAPS0043', E_USER_ERROR, $path);
				return false;
			}
		}

		$step_data['percent_done'] = 100;
		$step_data['complete']     = true;
		return true;

	}//end processInstallFiles()


	/**
	* Run update queries over the database
	*
	* @param	&array	$step_data		the step data for this job
	* @param	string	$prefix			prefix to be used with this job
	*
	* @access public
	* @return boolean
	*/
	function processDatabaseUpdates(&$step_data, $prefix)
	{
		$patch_info = $this->_getPatchDetails($this->_running_vars['patchid']);

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		foreach ($patch_info['queries'] as $query) {
			// replace table prefix replacements in query
			$query = str_replace('TABLE_PREFIX_', 'sq_', $query);
			$query = str_replace('ROLLBACK_PREFIX_', 'sq_rb_', $query);

			$result = $db->query($query);
			assert_valid_db_result($result);
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		$step_data['percent_done'] = 100;
		$step_data['complete']     = true;
		return true;

	}//end processDatabaseUpdates()


	/**
	* Run step 3 of the install if required. Will analyse the files being
	* added in this patch first to see if it should only selectively run
	* for certain packages.
	*
	* We dont die on errors because it's a bit too late for that
	*
	* @param	array	&$step_data		the step data for this job
	* @param	string	$prefix			prefix to be used with this job
	*
	* @access public
	* @return boolean
	*/
	function processInstallScript(&$step_data, $prefix)
	{
		$package_reg = "|packages/(\w+)/(\w+)/.|is";

		$patch_info = $this->_getPatchDetails($this->_running_vars['patchid']);

		if ($patch_info['run_script'] === '1') {
			$package_list = isset($patch_info['runscriptfor']) ? $patch_info['runscriptfor'] : Array();
			$_GET['SYSTEM_ROOT'] = SQ_SYSTEM_ROOT;
			ob_start();
				include SQ_SYSTEM_ROOT.'/install/step_03.php';
			ob_end_clean();
		}

		$step_data['percent_done'] = 100;
		$step_data['complete']     = true;
		return true;

	}//end processInstallScript()


	/**
	* Clean up after ourselves
	*
	* @param	array	&$step_data		the step data for this job
	* @param	string	$prefix			prefix to be used with this job
	*
	* @access public
	* @return boolean
	*/
	function processCleanUp(&$step_data, $prefix)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$patch_info = $this->_getPatchDetails($this->_running_vars['patchid']);

		// write an entry to the DB for this patch to say it has been installed
		$db  = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'INSERT INTO sq_patches
				(patchid, installed)
				VALUES ('.$db->quote($patch_info['id']).', '.$db->quote(ts_iso8601(time())).')
				';

		$result = $db->query($sql);
		assert_valid_db_result($result);

		$step_data['percent_done'] = 100;
		$step_data['complete']     = true;

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return true;

	}//end processCleanUp()


}//end class

?>
