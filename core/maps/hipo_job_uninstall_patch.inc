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
* $Id: hipo_job_uninstall_patch.inc,v 1.3 2004/06/25 04:04:10 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to uninstall a system patch
*
* <pre>
* Initial Running Vars :
*    int    patchid - the ID of the patch we are uninstalling
* </pre>
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
* @subpackage MAPS
*/
class HIPO_Job_Uninstall_Patch extends HIPO_Job
{

	/**
	* The name of this HIPO job (should not be modified by code)
	* @var string
	*/
	var $hipo_name = 'Uninstall Patch';

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
						'name'			=> 'Reversing File Updates',
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
	function HIPO_Job_Uninstall_Patch($code_name='')
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
	* Paints a generic 'please wait' screen for uninstall steps
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
	* Paints a confirmation screen for uninstalling the patch
	*
	* This allows the user to back out if they want to, as well as be presented
	* with any information/warnings regarding the uninstall
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
					<img style="float: left;" src="<?php echo $o->filesPath('/images/maps/uninstall.png'); ?>" wdth="70" height="69" />
					<span class="sq-backend-warning">
					<b>Please ensure you have backed up your system before continuing. You must tick the box below to confirm and continue installation of the patch.</b><br /><br />
					<?php check_box('maps_backup_confirm', '1', false, 'Javascript: setNextButtonStatus();'); ?> <b>Yes, I have backed up my system</b>
					</span>
				<?php
			$o->closeField();
		$o->closeSection();

		$o->openSection('Uninstall Options');
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
				echo 'The details of the patch you have selected to uninstall are shown below.<br />Please confirm that you wish to uninstall this patch.';
			$o->closeField();
			$o->openField('NB');
				echo '<b>'.$patch_info['reverse_description'].'</b>';
			$o->closeField();
		$o->closeSection();

		$o->openSection('Details of Original Patch');
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

		$o->openSection('Change Log of Original Patch');
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
	* @param array	&$step_data		the step data for this job
	* @param string	$prefix			prefix to be used with this job
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
	* @param array	&$step_data		the step data for this job
	* @param string	$prefix			prefix to be used with this job
	*
	* @access public
	* @return boolean
	*/
	function processCheckVersions(&$step_data, $prefix)
	{
		$patch_info = $this->_getPatchDetails($this->_running_vars['patchid']);

		if (isset($patch_info['requirements']['system'])) {
			// the system version must be exactly equal to what is defined
			// in the patch or else the patch is unpatching an untested system
			if ($patch_info['requirements']['system']['new_version'] != SQ_SYSTEM_VERSION) {
				trigger_error('This patch can only be removed from version '.$patch_info['requirements']['system']['new_version'].' of the system (current version is '.SQ_SYSTEM_VERSION.')', E_USER_ERROR);
				return false;
			}
		}
		
		if (isset($patch_info['requirements']['packages'])) {
			foreach ($patch_info['requirements']['packages'] as $package_code => $versions) {
				$package_path = SQ_PACKAGES_PATH.'/'.$package_code.'/package_manager_'.$package_code.'.inc';
				if (!is_file($package_path)) {
					trigger_error('This patch can only be removed from version '.$versions['new_version'].' of package "'.strtoupper($package_code).'" (package is not installed on this system)', E_USER_ERROR);
					return false;
				}
				require_once $package_path;
				$package_class = 'package_manager_'.$package_code;
				$package = new $package_class();

				if ($package->version != $versions['new_version']) {
					trigger_error('This patch can only be removed from version '.$versions['new_version'].' of package  "'.strtoupper($package_code).'" (current version is '.$package->version.')', E_USER_ERROR);
					return false;
				}
			}
		}

		if (isset($patch_info['requirements']['assets'])) {
			foreach ($patch_info['requirements']['assets'] as $type_code => $versions) {
				$installed_version = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code, 'version');
				if ($installed_version != $versions['new_version']) {
					trigger_error('This patch can only be removed from version '.$versions['new_version'].' of asset type "'.$type_code.'" (current version is '.$installed_version.')', E_USER_ERROR);
					return false;
				}
			}
		}

		$step_data['percent_done'] = 100;
		$step_data['complete']     = true;
		return true;
	
	}//end processCheckVersions()


	/**
	* Check all files to be uninstalled to ensure they are valid
	*
	* @param array	&$step_data		the step data for this job
	* @param string	$prefix			prefix to be used with this job
	*
	* @access public
	* @return boolean
	*/
	function processCheckFiles(&$step_data, $prefix)
	{
		$patch_info  = $this->_getPatchDetails($this->_running_vars['patchid']);
		$backup_path = SQ_DATA_PATH.'/private/maps/installed_patches/'.$patch_info['id'];
		
		// check the files that we will be removing
		foreach ($patch_info['new_files'] as $path => $file_info) {
			$from = SQ_SYSTEM_ROOT.'/'.$path;

			if (empty($file_info['md5'])) {

				// should be a directory
				if (!$this->_running_vars['option_ignore_file_checks']) {
					if (!is_dir($from)) {
						trigger_error('Could not remove directory "'.$from.'" - directory does not exist in your MySource installation', E_USER_ERROR);
						return false;
					}
				}

			} else {

				// should be a regular file
				if (!$this->_running_vars['option_ignore_file_checks']) {
					if (!is_file($from)) {
						trigger_error('Could not remove file "'.$from.'" - file does not exist in your MySource installation', E_USER_ERROR);
						return false;
					}
					
					// check the MD5 sum of the file to ensure it has not been changed since installing
					if ($file_info['md5'] != md5(file_get_contents($from))) {
						trigger_error('Could not remove file "'.$from.'" - MD5 strings did not match - local file may have been modified since being patched', E_USER_ERROR);
						return false;
					}
				}

			}//end if directory
		}

		// check the files we will be updating
		foreach ($patch_info['updated_files'] as $path => $file_info) {
			$to   = SQ_SYSTEM_ROOT.'/'.$path;
			$from = $backup_path.'/'.$path;

			if (!is_file($from)) {
				trigger_error('Could not unpatch file "'.$to.'" - old version does not exist in patch backup directory', E_USER_ERROR);
				return false;
			}
			if (!$this->_running_vars['option_ignore_file_checks']) {
				if (!is_file($to)) {
					trigger_error('Could not unpatch file "'.$to.'" - file does not exist in your MySource installation and it was not removed in this patch', E_USER_ERROR);
					return false;
				}

				// check the MD5 sum of the files to ensure they have not been changed since installing
				if ($file_info['md5'] != md5(file_get_contents($to))) {
					trigger_error('Could not update file "'.$to.'" - MD5 strings did not match - local file may have been modified since being patched', E_USER_ERROR);
					return false;
				}
				if ($file_info['local_md5'] != md5(file_get_contents($from))) {
					trigger_error('Could not unpatch file "'.$to.'" - MD5 strings did not match - backup file may have been modified since being backed up', E_USER_ERROR);
					return false;
				}
			}
		}

		// check the files we be adding
		foreach ($patch_info['deleted_files'] as $path => $file_info) {
			$to   = SQ_SYSTEM_ROOT.'/'.$path;
			$from = $backup_path.'/'.$path;

			if (!is_file($from)) {
				trigger_error('Could not restore file "'.$to.'" - old version does not exist in patch backup directory', E_USER_ERROR);
				return false;
			}

			if (!$this->_running_vars['option_ignore_file_checks']) {
				if (is_file($to)) {
					trigger_error('Could not restore file "'.$to.'" - a file with the same name exists in your MySource installation and it was removed in this patch', E_USER_ERROR);
					return false;
				}
	
				if ($file_info['local_md5'] != md5(file_get_contents($from))) {
					trigger_error('Could not restore file "'.$to.'" - MD5 strings did not match - backup file may have been modified since being backed up', E_USER_ERROR);
					return false;
				}
			}
		}

		$step_data['percent_done'] = 100;
		$step_data['complete']     = true;
		return true;
	
	}//end processCheckFiles()


	/**
	* Update files from the patch
	*
	* @param array	&$step_data		the step data for this job
	* @param string	$prefix			prefix to be used with this job
	*
	* @access public
	* @return boolean
	*/
	function processInstallFiles(&$step_data, $prefix)
	{
		$patch_info = $this->_getPatchDetails($this->_running_vars['patchid']);
		$backup_path = SQ_DATA_PATH.'/private/maps/installed_patches/'.$patch_info['id'];

		// need to reverse the new files (and dirs) we are deleting so we dont remove files
		// before they are sceduled to be removed
		$patch_info['new_files'] = array_reverse($patch_info['new_files']);
		foreach ($patch_info['new_files'] as $path => $file_info) {
			$from = SQ_SYSTEM_ROOT.'/'.$path;

			if (empty($file_info['md5'])) {
				if ($this->_running_vars['option_ignore_file_checks'] && !is_dir($from)) continue;

				include_once(SQ_FUDGE_PATH.'/general/file_system.inc');
				if (!delete_directory($from)) {
					trigger_error('Could not remove directory "'.$from.'" - patch uninstall aborted', E_USER_ERROR);
					return false;
				}
			}

			// we check that this file exists again here because if we have deleted
			// the directory it was in, it will already have been deleted for us
			if (is_file($from) && !unlink($from)) {
				trigger_error('Could not remove file "'.$from.'" - patch uninstall aborted', E_USER_ERROR);
				return false;
			}
		}

		foreach ($patch_info['updated_files'] as $path => $file_info) {
			$to   = SQ_SYSTEM_ROOT.'/'.$path;
			$from = $backup_path.'/'.$path;

			if (!copy($from, $to)) {
				trigger_error('Could not unpatch file "'.$to.'" - patch uninstall aborted', E_USER_ERROR);
				return false;
			}
		}

		foreach ($patch_info['deleted_files'] as $path => $file_info) {
			$to   = SQ_SYSTEM_ROOT.'/'.$path;
			$from = $backup_path.'/'.$path;

			if (!copy($from, $to)) {
				trigger_error('Could not restore file "'.$to.'" - patch uninstall aborted', E_USER_ERROR);
				return false;
			}
		}

		$step_data['percent_done'] = 100;
		$step_data['complete']     = true;
		return true;
	
	}//end processInstallFiles()
	

	/**
	* Run reverse update queries over the database
	*
	* @param array	&$step_data		the step data for this job
	* @param string	$prefix			prefix to be used with this job
	*
	* @access public
	* @return boolean
	*/
	function processDatabaseUpdates(&$step_data, $prefix)
	{
		$patch_info = $this->_getPatchDetails($this->_running_vars['patchid']);

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		foreach ($patch_info['reverse_queries'] as $query) {
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
	* @param array	&$step_data		the step data for this job
	* @param string	$prefix			prefix to be used with this job
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
	* Note that no errors here actually kill the HIPO because its a little
	* late to be backing out on the uninstall. We'll sort any errors out later (fingers crossed)
	*
	* @param array	&$step_data		the step data for this job
	* @param string	$prefix			prefix to be used with this job
	*
	* @access public
	* @return boolean
	*/
	function processCleanUp(&$step_data, $prefix)
	{
		$patch_info = $this->_getPatchDetails($this->_running_vars['patchid']);
		$backup_path = SQ_DATA_PATH.'/private/maps/installed_patches/'.$patch_info['id'];

		// remove the DB entry for this patch
		$db  = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'DELETE FROM '.SQ_TABLE_PREFIX.'patches WHERE patchid = '.$db->quote($patch_info['id']);
		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			// dont actually die because we cant exactly abort at this point
			// we'll sort it out later another way
		}
		
		// remove the backup directory for this patch because we dont need that any more
		require_once(SQ_FUDGE_PATH.'/general/file_system.inc');
		if (!delete_directory($backup_path)) {
			trigger_error('Unable to delete patch backup directory "'.$backup_path.'" - continuing', E_USER_WARNING);
			// dont actually die because we cant exactly abort at this point
			// we'll sort it out later another way
		}

		$step_data['percent_done'] = 100;
		$step_data['complete']     = true;
		return true;
	
	}//end processCleanUp()


}//end class

?>