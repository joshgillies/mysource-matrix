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
* $Id: hipo_job_install_patch.inc,v 1.4 2004/10/06 01:54:14 mnyeholt Exp $
* $Name: not supported by cvs2svn $
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
* @version $Version$ - 1.0
* @package MySource_Matrix
* @subpackage MAPS
*/
class HIPO_Job_Install_Patch extends HIPO_Job
{

	/**
	* The name of this HIPO job (should not be modified by code)
	* @var string
	*/
	var $hipo_name = 'Install Patch';

	/**
	* Array of steps to perform in this HIPO
	* @var Array
	*/
	var $_steps = Array(
					Array(
						'name'			=> 'Confirmation',
						'function_call'		=> Array(
												'paint_function'   => 'paintConfirmation',
												'process_function' => 'processConfirmation',
												),
						'running_mode'	=> 'web',
						'auto_step'		=> false,
						'skip_step'		=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
					Array(
						'name'			=> 'Checking Versions',
						'function_call'		=> Array(
												'paint_function'   => 'paintPleaseWait',
												'process_function' => 'processCheckVersions',
												),
						'running_mode'	=> 'server',
						'auto_step'		=> true,
						'skip_step'		=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
					Array(
						'name'			=> 'Checking File Updates',
						'function_call'		=> Array(
												'paint_function'   => 'paintPleaseWait',
												'process_function' => 'processCheckFiles',
												),
						'running_mode'	=> 'server',
						'auto_step'		=> true,
						'skip_step'		=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
					Array(
						'name'			=> 'Backing Up Modified Files',
						'function_call'		=> Array(
												'paint_function'   => 'paintPleaseWait',
												'process_function' => 'processBackupFiles',
												),
						'running_mode'	=> 'server',
						'auto_step'		=> true,
						'skip_step'		=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
					Array(
						'name'			=> 'Updating Database',
						'function_call'		=> Array(
												'paint_function'   => 'paintPleaseWait',
												'process_function' => 'processDatabaseUpdates',
												),
						'running_mode'	=> 'server',
						'auto_step'		=> true,
						'skip_step'		=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
					Array(
						'name'			=> 'Installing File Updates',
						'function_call'		=> Array(
												'paint_function'   => 'paintPleaseWait',
												'process_function' => 'processInstallFiles',
												),
						'running_mode'	=> 'server',
						'auto_step'		=> true,
						'skip_step'		=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
					Array(
						'name'			=> 'Running Installation Script',
						'function_call'		=> Array(
												'paint_function'   => 'paintPleaseWait',
												'process_function' => 'processInstallScript',
												),
						'running_mode'	=> 'web',
						'auto_step'		=> true,
						'skip_step'		=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
					Array(
						'name'			=> 'Cleaning Up',
						'function_call'		=> Array(
												'paint_function'   => 'paintPleaseWait',
												'process_function' => 'processCleanUp',
												),
						'running_mode'	=> 'server',
						'auto_step'		=> true,
						'skip_step'		=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
				);


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
			trigger_error('Cannot initialise Install Patch HIPO, no patch ID specified', E_USER_WARNING);
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
		
		$o->openSection('Please Wait');
			$o->openField('Please wait - '.strtolower($step_data['name']).'...');
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
					<b>Please ensure you have backed up your system before continuing. You must tick the box below to confirm and continue installation of the patch.</b><br /><br />
					<?php check_box('maps_backup_confirm', '1', false, 'Javascript: setNextButtonStatus();'); ?> <b>Yes, I have backed up my system</b>
					</span>
				<?php
			$o->closeField();
		$o->closeSection();
		
		$o->openSection('Install Options');
			$o->openField('');
				?>
				<table>
					<tr>
						<td class="sq-backend-field" align="center">&nbsp;</td>
						<td class="sq-backend-field">Name</td>
						<td class="sq-backend-field">Description</td>
						<td class="sq-backend-field" align="center">Danger</td>
					</tr>
					<tr>
						<td class="sq-backend-data" align="center" valign="top">
							<?php check_box('maps_ignore_file_checks'); ?>
						</td>
						<td class="sq-backend-data" nowrap="nowrap" valign="top">
							Ignore File Checks
						</td>
						<td class="sq-backend-data" valign="top">
							Ignore checking of local files in favour of directly replacing them with patched version
						</td>
						<td class="sq-backend-data" valign="top" align="center">
							<span style="color: #FF0000;"><b>High</b></span>
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
		
		$o->openSection('Patch Details');
			$o->openField('Severity');
				echo $patch_info['severity'];
			$o->closeField();
			$o->openField('');
				echo '<b>File Updates:</b>&nbsp;';
				echo (
						empty($patch_info['updated_files']) &&
						empty($patch_info['new_files']) &&
						empty($patch_info['deleted_files'])
					  ) ? 'No' : 'Yes';
				echo '&nbsp;&nbsp;<b>Database Changes:</b>&nbsp;';
				echo (empty($patch_info['queries'])) ? 'No' : 'Yes';
				echo '&nbsp;&nbsp;<b>Reversible:</b>&nbsp;';
				echo ($patch_info['reversible']) ? 'Yes' : 'No';
			$o->closeField();
			$o->openField('Description');
				echo $patch_info['patch_description'];
			$o->closeField();
		$o->closeSection();
		
		$o->openSection('Change Log');
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
					trigger_error('This patch can only be applied to version '.$patch_info['requirements']['system']['old_version'].' of the system (current version is '.SQ_SYSTEM_VERSION.')', E_USER_ERROR);
					return false;
				}
			} else if ($patch_info['requirements']['system']['old_version'] != SQ_SYSTEM_VERSION) {
				// the system version must be exactly equal to what is defined
				// in the patch or else the patch is patching an untested system
				trigger_error('This patch can only be applied to version '.$patch_info['requirements']['system']['old_version'].' of the system (current version is '.SQ_SYSTEM_VERSION.')', E_USER_ERROR);
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

						trigger_error('This patch can only be applied to a system without a current "'.strtoupper($package_code).'" package (current system has version '.$package->version.' installed)', E_USER_ERROR);
						return false;
					}
				} else if ($versions['old_version'] == $versions['new_version']) {
					// if the new and old versions are the same, make sure the 
					// package version is at least that of the required version.
					require_once $package_path;
					$package = new $package_class();
					
					// if the current version is lower than the minimum required version
					// then the requirement fails.
					if (version_compare($package->version,$versions['old_version']) < 0) {
						trigger_error('This patch can only be applied to version '.$patch_info['requirements']['system']['old_version'].' of the system (current version is '.SQ_SYSTEM_VERSION.')', E_USER_ERROR);
						return false;
					}
					
				} else {
					if (!is_file($package_path)) {
						trigger_error('This patch can only be applied to version '.$versions['old_version'].' of package "'.strtoupper($package_code).'" (current version is '.$package->version.')', E_USER_ERROR);
						return false;
					}
					require_once $package_path;
					$package = new $package_class();

					if ($package->version != $versions['old_version']) {
						trigger_error('This patch can only be applied to version '.$versions['old_version'].' of package "'.strtoupper($package_code).'" (current version is '.$package->version.')', E_USER_ERROR);
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
						trigger_error('This patch can only be applied to a system without a current "'.$type_code.'" asset type installed (current system has version '.$installed_version.' installed)', E_USER_ERROR);
						return false;
					}
				} else if ($versions['old_version'] == $versions['new_version']) {
					// if the new and old versions are the same, make sure the 
					// asset version is at least that of the required version.
					$installed_version = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code, 'version');
					
					// if the current version is lower than the minimum required version
					// then the requirement fails.
					if (version_compare($installed_version,$versions['old_version']) < 0) {
						trigger_error('This patch can only be applied to version '.$versions['old_version'].' of asset type "'.$type_code.'" (current version is '.$installed_version.')', E_USER_ERROR);
						return false;
					}
					
				} else {
					$installed_version = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code, 'version');
					if ($installed_version != $versions['old_version']) {
						trigger_error('This patch can only be applied to version '.$versions['old_version'].' of asset type "'.$type_code.'" (current version is '.$installed_version.')', E_USER_ERROR);
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
					trigger_error('Could not install new directory "'.$from.'" - directory does not exist in patch', E_USER_ERROR);
					return false;
				}

				if (!$this->_running_vars['option_ignore_file_checks']) {
					// check that the directory does not already exist in the system
					if (is_dir($to)) {
						trigger_error('Could not install new directory "'.$to.'" - a directory with the same name already exists in your MySource installation', E_USER_ERROR);
						return false;
					}
				}

			} else {
				// this should be a regular file
				if (!is_file($from)) {
					trigger_error('Could not install new file "'.$from.'" - file does not exist in patch', E_USER_ERROR);
					return false;
				}

				if (!$this->_running_vars['option_ignore_file_checks']) {
					// check that there is no file already installed
					if (is_file($to)) {
						trigger_error('Could not install new file "'.$to.'" - a file with the same name already exists in your MySource installation', E_USER_ERROR);
						return false;
					}
				}

				// check the MD5 sum of the file to ensure it has not been changed since downloading
				if ($file_info['md5'] != md5(file_get_contents($from))) {
					trigger_error('Could not update file "'.$to.'" - MD5 strings did not match - downloded file may have been modified', E_USER_ERROR);
					return false;
				}

			}//end if directory
		}

		// check the files we will be updating
		foreach ($patch_info['updated_files'] as $path => $file_info) {
			$to   = SQ_SYSTEM_ROOT.'/'.$path;
			$from = $patch_path.'/'.$path;

			if (!is_file($from)) {
				trigger_error('Could not install update file "'.$from.'" - file does not exist in patch', E_USER_ERROR);
				return false;
			}

			if (!$this->_running_vars['option_ignore_file_checks']) {
				// make sure there is a file to replace
				if (!is_file($to)) {
					trigger_error('Could not update file "'.$to.'" - no file with that name exists in your MySource installation', E_USER_ERROR);
					return false;
				}

				// check the MD5 sum of the file to ensure it has not been changed locally
				if ($file_info['local_md5'] != md5(file_get_contents($to))) {
					trigger_error('Could not update file "'.$to.'" - MD5 strings did not match - local file may have been modified', E_USER_ERROR);
					return false;
				}
			}

			// check the MD5 sum of the file to ensure it has not been changed since downloading
			if ($file_info['md5'] != md5(file_get_contents($from))) {
				trigger_error('Could not update file "'.$to.'" - MD5 strings did not match - downloded file may have been modified', E_USER_ERROR);
				return false;
			}
		}

		// check the files we will be deleting
		foreach ($patch_info['deleted_files'] as $path => $file_info) {
			$from = SQ_SYSTEM_ROOT.'/'.$path;

			if (!$this->_running_vars['option_ignore_file_checks']) {
				// make sure there is a file to delete
				if (!is_file($from)) {
					trigger_error('Could not delete file "'.$from.'" - no file with that name exists in your MySource installation', E_USER_ERROR);
					return false;
				}

				// check the MD5 sum of the file to ensure it has not been changed locally
				if ($file_info['local_md5'] != md5(file_get_contents($from))) {
					trigger_error('Could not delete file "'.$from.'" - MD5 strings did not match - local file may have been modified', E_USER_ERROR);
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
			trigger_error('Failed to create backup directory "'.$backup_path.'" for patch "'.$patch_info['id'].'" - upgrade aborted', E_USER_ERROR);
			return false;
		}

		foreach ($patch_info['new_files'] as $path => $file_info) {
			$from = SQ_SYSTEM_ROOT.'/'.$path;
			$to   = $backup_path.'/'.$path;
			
			// back up any new files if there is already a file in in the way
			if (!empty($file_info['md5']) && is_file($to)) {
				// try and create the backup directory for this file if it doesnt exist
				if (!is_dir(dirname($to)) && !create_directory(dirname($to))) {
					trigger_error('Failed to create backup directory "'.$path.'" for patch "'.$patch_info['id'].'" - upgrade aborted', E_USER_ERROR);
					return false;
				}
				// backup the existing file
				if (!copy($from, $to)) {
					trigger_error('Failed to backup file "'.$from.'" - upgrade aborted', E_USER_ERROR);
					return false;
				}
			}
		}

		foreach ($patch_info['updated_files'] as $path => $file_info) {
			$from = SQ_SYSTEM_ROOT.'/'.$path;
			$to   = $backup_path.'/'.$path;

			// try and create the backup directory for this file if it doesnt exist
			if (!is_dir(dirname($to)) && !create_directory(dirname($to))) {
				trigger_error('Failed to create backup directory "'.$path.'" for patch "'.$patch_info['id'].'" - upgrade aborted', E_USER_ERROR);
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
				trigger_error('Failed to backup file "'.$from.'" - upgrade aborted', E_USER_ERROR);
				return false;
			}
		}

		foreach ($patch_info['deleted_files'] as $path => $file_info) {
			$from = SQ_SYSTEM_ROOT.'/'.$path;
			$to   = $backup_path.'/'.$path;

			// try and create the backup directory for this file if it doesnt exist
			if (!is_dir(dirname($to)) && !create_directory(dirname($to))) {
				trigger_error('Failed to create backup directory "'.$path.'" for patch "'.$patch_info['id'].'" - upgrade aborted', E_USER_ERROR);
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
					trigger_error('Failed to backup file "'.$from.'" - upgrade aborted', E_USER_ERROR);
					return false;
				} else {
					trigger_error('Failed to backup file "'.$from.'" - upgrade continuing');
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
					trigger_error('Failed to install new directory "'.$path.'" - upgrade aborted', E_USER_ERROR);
					return false;
				}
			} else {
				if (!copy($from, $to)) {
					trigger_error('Failed to install new file "'.$path.'" - upgrade aborted', E_USER_ERROR);
					return false;
				}
			}
		}

		foreach ($patch_info['updated_files'] as $path => $file_info) {
			$to   = SQ_SYSTEM_ROOT.'/'.$path;
			$from = $patch_path.'/'.$path;

			if (!copy($from, $to)) {
				trigger_error('Failed to update file "'.$path.'" - upgrade aborted', E_USER_ERROR);
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
				trigger_error('Failed to delete file "'.$path.'" - upgrade aborted', E_USER_ERROR);
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

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		foreach ($patch_info['queries'] as $query) {
			// replace table prefix replacements in query
			$query = str_replace('TABLE_PREFIX_', SQ_TABLE_PREFIX, $query);
			$query = str_replace('ROLLBACK_PREFIX_', SQ_TABLE_ROLLBACK_PREFIX, $query);

			$result = $db->query($query);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		$step_data['percent_done'] = 100;
		$step_data['complete']     = true;
		return true;

	}//end processDatabaseUpdates()


	/**
	* Run step 3 of the install if required
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
		$patch_info = $this->_getPatchDetails($this->_running_vars['patchid']);

		if ($patch_info['run_script'] === '1') {
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
		$patch_info = $this->_getPatchDetails($this->_running_vars['patchid']);

		// write an entry to the DB for this patch to say it has been installed
		$db  = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'INSERT INTO '.SQ_TABLE_PREFIX.'patches
				(patchid, installed)
				VALUES ('.$db->quote($patch_info['id']).', '.$db->quote(ts_iso8601(time())).')
				';

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
		}

		$step_data['percent_done'] = 100;
		$step_data['complete']     = true;
		return true;
	
	}//end processCleanUp()


}//end class

?>