<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: hipo_job_uninstall_patch.inc,v 1.18 2006/12/05 04:58:44 bcaldwell Exp $
*
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
* @version $Revision: 1.18 $
* @package MySource_Matrix
* @subpackage MAPS
*/
class HIPO_Job_Uninstall_Patch extends HIPO_Job
{


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
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('hipo_name_uninstall_patch');

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
						'name'			=> translate('hipo_checking_asset_types'),
						'function_call'	=> Array(
												'paint_function'   => 'paintPleaseWait',
												'process_function' => 'processCheckAssetTypes',
										   ),
						'running_mode'	=> 'server',
						'auto_step'		=> true,
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
						'allow_cancel'  => true,
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
						'allow_cancel'  => true,
					),
					Array(
						'name'			=> translate('hipo_reversing_file_updates'),
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
			trigger_localised_error('MAPS0058', E_USER_WARNING);
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

		$o->openSection(translate('please_wait'));
			$o->openField(translate('please_wait').' - '.strtolower($step_data['name']).'...');
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

		$o->openSection(translate('important_notice'));
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
					<b><?php echo translate('maps_hipo_ensure_backup'); ?></b><br /><br />
					<?php check_box('maps_backup_confirm', '1', false, 'Javascript: setNextButtonStatus();'); ?> <b><?php echo translate('maps_hipo_backup_confirm'); ?>/b>
					</span>
				<?php
			$o->closeField();
		$o->closeSection();

		$o->openSection(translate('uninstall_options'));
			$o->openField('');
				?>
				<table>
					<tr>
						<td class="sq-backend-field" align="center">&nbsp;</td>
						<td class="sq-backend-field"><?php echo translate('name'); ?></td>
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

		$o->openSection(translate('please_note'));
			$o->openField('');
				echo translate('maps_confirm_uninstall_patch');
			$o->closeField();
			$o->openField(translate('nb'));
				echo '<b>'.$patch_info['reverse_description'].'</b>';
			$o->closeField();
		$o->closeSection();

		$o->openSection(translate('details_of_original_patch'));
			$o->openField(translate('severity'));
				echo $patch_info['severity'];
			$o->closeField();
			$o->openField('');
				echo '<b>'.translate('maps_file_updates').':</b>&nbsp;';
				echo (
						empty($patch_info['updated_files']) &&
						empty($patch_info['new_files']) &&
						empty($patch_info['deleted_files'])
					  ) ? translate('no'): translate('yes');
				echo '&nbsp;&nbsp;<b>'.translate('maps_database_changes').':</b>&nbsp;';
				echo (empty($patch_info['queries'])) ? translate('no'): translate('yes');
				echo '&nbsp;&nbsp;<b>'.translate('maps_reversible').':</b>&nbsp;';
				echo ($patch_info['reversible']) ? translate('yes') : translate('no');
			$o->closeField();
			$o->openField(translate('description'));
				echo $patch_info['patch_description'];
			$o->closeField();
		$o->closeSection();

		$o->openSection(translate('change_log_of_original_patch'));
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
	* Checks that any asset types being deleted can be deleted
	*
	* @param array	&$step_data		the step data for this job
	* @param string	$prefix			prefix to be used with this job
	*
	* @access public
	* @return boolean
	*/
	function processCheckAssetTypes(&$step_data, $prefix)
	{
		$patch_info = $this->_getPatchDetails($this->_running_vars['patchid']);

		if (isset($patch_info['requirements']['assets'])) {
			foreach ($patch_info['requirements']['assets'] as $type_code => $versions) {
				if (empty($versions['old_version'])) {
					// we are going to be deleting this asset type that was installed
					// so make sure we can actually delete it
					$assetid_count = count($GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($type_code, false));
					if ($assetid_count > 0) {
						trigger_localised_error('MAPS0016', E_USER_ERROR, $type_code, $assetid_count);
						return false;
					} else {
						$type_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);
						if (substr(SQ_SYSTEM_ROOT.'/'.$type_info['dir'], 0, strlen(SQ_CORE_PACKAGE_PATH)) == SQ_CORE_PACKAGE_PATH) {
							trigger_localised_error('MAPS0015', E_USER_ERROR, $type_code);
							return false;
						}
					}
				}
			}
		}

		$step_data['percent_done'] = 100;
		$step_data['complete']     = true;
		return true;

	}//end processCheckAssetTypes()


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
				trigger_localised_error('MAPS0017', E_USER_ERROR, $patch_info['requirements']['system']['new_version'], SQ_SYSTEM_VERSION);
				return false;
			}
		}

		if (isset($patch_info['requirements']['packages'])) {
			foreach ($patch_info['requirements']['packages'] as $package_code => $versions) {
				$package_path = SQ_PACKAGES_PATH.'/'.$package_code.'/package_manager_'.$package_code.'.inc';
				if (!is_file($package_path)) {
					trigger_localised_error('MAPS0018', E_USER_ERROR, $versions['new_version'], strtoupper($package_code));
					return false;
				}
				require_once $package_path;
				$package_class = 'package_manager_'.$package_code;
				$package = new $package_class();

				if ($package->version != $versions['new_version']) {
					trigger_localised_error('MAPS0019', E_USER_ERROR, $versions['new_version'], strtoupper($package_code), $package->version);
					return false;
				}
			}
		}

		if (isset($patch_info['requirements']['assets'])) {
			foreach ($patch_info['requirements']['assets'] as $type_code => $versions) {
				$installed_version = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code, 'version');
				if ($installed_version != $versions['new_version']) {
					trigger_localised_error('MAPS0020', E_USER_ERROR, $versions['new_version'], $type_code, $installed_version);
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
						trigger_localised_error('MAPS0039', E_USER_ERROR, $from);
						return false;
					}
				}

			} else {

				// should be a regular file
				if (!$this->_running_vars['option_ignore_file_checks']) {
					if (!is_file($from)) {
						trigger_localised_error('MAPS0037', E_USER_ERROR, $from);
						return false;
					}

					// check the MD5 sum of the file to ensure it has not been changed since installing
					if ($file_info['md5'] != md5(file_get_contents($from))) {
						trigger_localised_error('MAPS0038', E_USER_ERROR, $from);
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
				trigger_localised_error('MAPS0030', E_USER_ERROR, $to);
				return false;
			}
			if (!$this->_running_vars['option_ignore_file_checks']) {
				if (!is_file($to)) {
					trigger_localised_error('MAPS0031', E_USER_ERROR, $to);
					return false;
				}

				// check the MD5 sum of the files to ensure they have not been changed since installing
				if ($file_info['md5'] != md5(file_get_contents($to))) {
					trigger_localised_error('MAPS0027', E_USER_ERROR, $to);
					return false;
				}
				if ($file_info['local_md5'] != md5(file_get_contents($from))) {
					trigger_localised_error('MAPS0032', E_USER_ERROR, $to);
					return false;
				}
			}
		}

		// check the files we be adding
		foreach ($patch_info['deleted_files'] as $path => $file_info) {
			$to   = SQ_SYSTEM_ROOT.'/'.$path;
			$from = $backup_path.'/'.$path;

			if (!is_file($from)) {
				trigger_localised_error('MAPS0034', E_USER_ERROR, $to);
				return false;
			}

			if (!$this->_running_vars['option_ignore_file_checks']) {
				if (is_file($to)) {
					trigger_localised_error('MAPS0035', E_USER_ERROR, $to);
					return false;
				}

				if ($file_info['local_md5'] != md5(file_get_contents($from))) {
					trigger_localised_error('MAPS0059', E_USER_ERROR, $to);
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
					trigger_localised_error('MAPS0040', E_USER_ERROR, $from);
					return false;
				}
			}

			// we check that this file exists again here because if we have deleted
			// the directory it was in, it will already have been deleted for us
			if (is_file($from) && !unlink($from)) {
				trigger_localised_error('MAPS0036', E_USER_ERROR, $from);
				return false;
			}
		}

		foreach ($patch_info['updated_files'] as $path => $file_info) {
			$to   = SQ_SYSTEM_ROOT.'/'.$path;
			$from = $backup_path.'/'.$path;

			if (!copy($from, $to)) {
				trigger_localised_error('MAPS0029', E_USER_ERROR, $to);
				return false;
			}
		}

		foreach ($patch_info['deleted_files'] as $path => $file_info) {
			$to   = SQ_SYSTEM_ROOT.'/'.$path;
			$from = $backup_path.'/'.$path;

			if (!copy($from, $to)) {
				trigger_localised_error('MAPS0033', E_USER_ERROR, $to);
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

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		foreach ($patch_info['reverse_queries'] as $query) {
			// replace table prefix replacements in query
			$query = str_replace('TABLE_PREFIX_', 'sq_', $query);
			$query = str_replace('ROLLBACK_PREFIX_', 'sq_rb_', $query);

			$result = $db->query($query);
			if (!assert_valid_db_result($result, '', false, false)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return false;
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		$step_data['percent_done'] = 100;
		$step_data['complete']     = true;
		return true;

	}//end processDatabaseUpdates()


	/**
	* Run step 3 & 4 of the install if required
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
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$patch_info = $this->_getPatchDetails($this->_running_vars['patchid']);
		$backup_path = SQ_DATA_PATH.'/private/maps/installed_patches/'.$patch_info['id'];

		// remove the DB entry for this patch
		$db  = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'DELETE FROM sq_patches WHERE patchid = '.$db->quote($patch_info['id']);
		$result = $db->query($sql);
		if (!assert_valid_db_result($result, '', false, false)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');

		}
		// dont actually die because we cant exactly abort at this point
		// we'll sort it out later another way

		// remove the backup directory for this patch because we dont need that any more
		require_once(SQ_FUDGE_PATH.'/general/file_system.inc');
		if (!delete_directory($backup_path)) {
			trigger_localised_error('MAPS0014', E_USER_WARNING, $backup_path);
			// dont actually die because we cant exactly abort at this point
			// we'll sort it out later another way
		}

		$step_data['percent_done'] = 100;
		$step_data['complete']     = true;
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return true;

	}//end processCleanUp()


}//end class

?>