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
* $Id: maps.inc,v 1.4 2004/06/25 04:04:10 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


define('SQ_MAPS_DOWNLOADED' , 'D');
define('SQ_MAPS_INSTALLED'  , 'I');



/**
* Maps (MySource Automated Patching System)
*
* Purpose
*
*    Looks aftet the automated patching of the system
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
* @subpackage MAPS
*/
class Maps extends MySource_Object
{


	/**
	* Constructor
	*
	*/
	function Maps()
	{
		$this->MySource_Object();

	}//end constructor


	/**
	* Paints the backend interface for installing patches
	*
	* @param object Backend	$backend	Reference to the backend object
	*
	* @access public
	* @return void
	*/
	function paintBackend(&$backend)
	{
		// if we dont have access, get lost
		if (!$GLOBALS['SQ_SYSTEM']->userRoot()) {
			$GLOBALS['SQ_SYSTEM']->paintLogin('Login', 'You do not have permission to access the <i>Automated Patching System</i>');
			exit();
		}

		$o = &$backend->out;

		$o->setHeading('Automated Patching System', '<script language="JavaScript" type="text/javascript">sq_print_icon("'.$o->filesPath('/images/icons/header/maps.png').'", "20", "20", "Automated Patching System");</script>');
		$o->addHiddenField('form_posted', '1');


		  /////////////////////
		 //  ACTIVE SCREEN  //
		/////////////////////
		if (empty($_REQUEST['maps_screen'])) {
			$active_screen = 'patches';
			$_REQUEST['maps_screen'] = $active_screen;
		} else {
			$active_screen = $_REQUEST['maps_screen'];
		}

		// make sure that we have the screen for next time
		$o->addFormActionGetVar('maps_screen', $active_screen);


		  ///////////////////
		 //  SCREEN MENU  //
		///////////////////
		$current_location = $o->getCurrentLocation();
		$current_location .= (strstr($current_location, '?')) ? '&' : '?';

		$maps_screens = Array(
								'patches'	=> 'Patching',
								'locks'		=> 'Active Locks',
							);

		foreach ($maps_screens as $screen_code => $screen_name) {
			$url = $current_location.'maps_screen='.rawurlencode($screen_code);
			$o->addStaticScreen($url, $screen_name);
		}

		$o->setCurrentScreen($current_location.'maps_screen='.rawurlencode($active_screen));


		switch($active_screen) {

			case 'patches' :
				if (!empty($_POST['form_posted'])) $this->processPatches($o);
				$this->paintPatches($o);
			break;

			case 'locks' :
				if (!empty($_POST['form_posted'])) $this->processActiveLocks($o);
				$this->paintActiveLocks($o);
			break;

			default :
				trigger_error('Active Screen "'.$active_screen.'" not known', E_USER_WARNING);
		}//end switch

	}//end paint()


	/**
	* Paint a list of patches that are available for download
	*
	* @param object Backend_Outputter	&$o		reference to the backend outputter
	*
	* @access public
	* @return void
	*/
	function paintPatches(&$o)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT patchid, installed
				FROM '.SQ_TABLE_PREFIX.'patches';

		$db_patches = $db->getAssoc($sql);
		if (DB::isError($db_patches)) {
			$o->openSection();
				$o->openField('WARNING');
					?>
					<span class="sq-backend-warning">
						<b>The MySource Automated Patching System does not appear to be installed properly.</b>
					</span>
					<?php
				$o->closeField();
			$o->closeSection();
		}

		$new_patches = Array();
		$installed_patches = Array();
		if ($handle = opendir(SQ_DATA_PATH.'/private/maps/downloaded_patches')) {
			while (false !== ($patchid = readdir($handle))) {
				if (!preg_match('|^[0-9]{12}$|', $patchid)) continue;
				$patch_details = $this->getPatchDetails($patchid);
				
				// is this an installed patch or not ?
				if (isset($db_patches[$patchid])) {
					$installed_date = date('F j, Y, g:i a', strtotime($db_patches[$patchid]));
				
					$installed_patches[$patchid] = Array(
													'description' => $patch_details['patch_description'],
													'installed'   => $installed_date,
													'reversible'  => $patch_details['reversible'],
													);
				} else {
					$new_patches[$patchid] = Array(
												'description' => $patch_details['patch_description'],
												'released'    => $patch_details['released'],
											);
				}
			}
		} else {
			trigger_error('Downloaded patches directory "'.SQ_DATA_PATH.'/private/maps/downloaded_patches'.'" does not exist.', E_USER_WARNING);
		}

		// look for installed patches that are not in the DB for some reason
		// so we can at least reverse them out and try again
		if ($handle = opendir(SQ_DATA_PATH.'/private/maps/installed_patches')) {
			while (false !== ($patchid = readdir($handle))) {
				if (!preg_match('|^[0-9]{12}$|', $patchid)) continue;
				$patch_details = $this->getPatchDetails($patchid);
				
				if (!isset($db_patches[$patchid])) {
				
					$installed_patches[$patchid] = Array(
													'description' => $patch_details['patch_description'],
													'installed'   => 'unknown',
													'reversible'  => $patch_details['reversible'],
													);
				}
			}
		} else {
			trigger_error('Installed patches directory "'.SQ_DATA_PATH.'/private/maps/installed_patches'.'" does not exist.', E_USER_WARNING);
		}


		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		$o->openSection('Upload Patch File');
			$o->openField('Upload Patch');
				file_upload('maps_upload_patch');
			$o->closeField();
			$o->openField('Note');
				?>
				Uploading a valid patch file here will not install the patch immediatly. It will be installed into your <i>download patches</i> directory and appear in the <b>Patches Available To Install</b> list below.
				<?php
			$o->closeField();
		$o->closeSection();



		$o->openSection('Patches Available To Install');
			$o->openField('');
				?>
				<table>
					<tr>
						<td valign="top"><img src="<?php echo $o->filesPath('/images/maps/install.png'); ?>" width="70" height="69" /></td>
						<td width="100%">
							<?php
							if (empty($new_patches)) {
								echo 'There are no patches available to be installed';
							} else {
								?>
								<table class="sq-backend-table">
									<tr>
										<td class="sq-backend-table-header">Description</td>
										<td class="sq-backend-table-header" width="120">Released</td>
										<td class="sq-backend-table-header" width="50" align="center">Install</td>
									</tr>
									<?php
									foreach ($new_patches as $patchid => $patch_info) {
										?>
										<tr>
											<td class="sq-backend-table-cell"><?php echo $patch_info['description']; ?></td>
											<td class="sq-backend-table-cell" nowrap="nowrap">
												<?php echo $patch_info['released']; ?>
											</td>
											<td class="sq-backend-table-cell" nowrap="nowrap" align="center">
												<?php check_box('maps_install_patch['.$patchid.']'); ?>
											</td>
										</tr>
										<?php
									}
									?>
								</table>
								<?php
							}
							?>
						</td>
					</tr>
				</table>
				<?php
			$o->closeField();
		$o->closeSection();


		$o->openSection('Installed Patches');
			$o->openField('');
				?>
				<table>
					<tr>
						<td valign="top"><img src="<?php echo $o->filesPath('/images/maps/uninstall.png'); ?>" width="70" height="69" /></td>
						<td width="100%">
							<?php
							if (empty($installed_patches)) {
								echo 'There are no patches currently installed';
							} else {
								?>
								<table class="sq-backend-table">
									<tr>
										<td class="sq-backend-table-header">Description</td>
										<td class="sq-backend-table-header" width="120">Install Date</td>
										<td class="sq-backend-table-header" width="50" align="center">Uninstall</td>
									</tr>
									<?php
									foreach ($installed_patches as $patchid => $patch_info) {
										?>
										<tr>
											<td class="sq-backend-table-cell"><?php echo $patch_info['description']; ?></td>
											<td class="sq-backend-table-cell" nowrap="nowrap">
												<?php echo $patch_info['installed']; ?>
											</td>
											<td class="sq-backend-table-cell" nowrap="nowrap" align="center">
												<?php
													if ($patch_info['reversible']) check_box('maps_uninstall_patch['.$patchid.']');
													else echo 'No';
												?>
											</td>
										</tr>
										<?php
									}
									?>
								</table>
								<?php
							}
							?>
						</td>
					</tr>
				</table>
				<?php
			$o->closeField();
		$o->closeSection();


		$o->commitButton('Commit', true);

	}//end paintPatches()


	/**
	* Processess the list of patches that have been installed
	*
	* @param object Backend_Outputter	&$o		reference to the backend outputter
	*
	* @return void
	* @access public
	*/
	function processPatches(&$o)
	{
		// upload a new patch is we've been asked to
		$info = get_file_upload_info('maps_upload_patch');
		if (!empty($info)) {

			// if this is a tar archive, extract it
			if (preg_match('|^[0-9]{12}\.tar$|', $info['name'])) {
				// work out the patchid to see if it is already installed
				$patchid = substr($info['name'], 0, 12);
				if (is_dir(SQ_DATA_PATH.'/private/maps/downloaded_patches/'.$patchid)) {
					trigger_error('A patch with the same ID has already been uploaded', E_USER_WARNING);
				} else {
					require_once 'Archive/Tar.php';
					$tar = new Archive_Tar($info['tmp_name']);
					$tar->extract(SQ_DATA_PATH.'/private/maps/downloaded_patches');
				}
			} else {
				trigger_error('The uploaded file did not appear to be a valid patch', E_USER_WARNING);
			}
		}

		$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();

		// install new patches
		if (isset($_POST['maps_install_patch'])) {
			foreach ($_POST['maps_install_patch'] as $patchid => $on) {
				if ($on !== '1') continue;
				$hh->queueHipo('hipo_job_install_patch', Array('patchid' => trim($patchid)), '', SQ_SYSTEM_ROOT.'/core/maps');
			}
		}
		
		// uninstall patches
		if (isset($_POST['maps_uninstall_patch'])) {
			foreach ($_POST['maps_uninstall_patch'] as $patchid => $on) {
				if ($on !== '1') continue;
				$hh->queueHipo('hipo_job_uninstall_patch', Array('patchid' => trim($patchid)), '', SQ_SYSTEM_ROOT.'/core/maps');
			}
		}

		$url = $hh->runQueuedJobs();
		if (!empty($url)) $o->setRedirect($url);

	}//end processPatches()
	
	
	/**
	* Paint a list of active locks
	*
	* @param object Backend_Outputter	&$o		reference to the backend outputter
	*
	* @access public
	* @return void
	*/
	function paintActiveLocks(&$o)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT userid, lockid, source_lockid, expires
				FROM '.SQ_TABLE_PREFIX.'lock
				WHERE expires >= NOW()
				ORDER BY expires DESC';

		$db_locks = $db->getAll($sql);
		if (DB::isError($db_locks)) {
			trigger_error($db_locks->getMessage().'<br />'.$db_locks->getUserInfo(), E_USER_ERROR);
		}

		
		if (empty($db_locks)) {
			$o->openSection('Active Locks');
				$o->openField('There are no active locks in the system');
				$o->closeField();
			$o->openSection();
			return true;
		}
		
		$active_locks = Array();
		$asset_info_ids = Array();
		foreach ($db_locks as $lock) {
			$lockid = $lock['lockid']; 
			if (substr($lockid, 0, 6) != 'asset.') continue;
			$lockid = substr($lockid, 6);
			$lock_type = substr($lockid, strrpos($lockid, '.') + 1);
			$assetid = substr($lockid, 0, strrpos($lockid, '.'));
			$active_locks[$lock['userid']]['locks'][$assetid][$lock_type] = $lock['expires'];
			
			if (!isset($active_locks[$lock['userid']]['num_locks'])) $active_locks[$lock['userid']]['num_locks'] = 0;
			$active_locks[$lock['userid']]['num_locks']++;
			
			$asset_info_ids[$lock['userid']] = $lock['userid'];
			$asset_info_ids[$assetid] = $assetid;
		}
		
		$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($asset_info_ids));

		require_once SQ_FUDGE_PATH.'/general/datetime.inc';
		
		$o->openSection('Active Locks');
			$o->openField('');
				?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-header">User</td>
						<td class="sq-backend-table-header">Lock Type</td>
						<td class="sq-backend-table-header">Held At</td>
						<td class="sq-backend-table-header">Expires</td>
					</tr>
				<?php
				foreach ($active_locks as $userid => $locks) {
					$printed_header = false;
					foreach ($locks['locks'] as $assetid => $lock_info) {
						foreach ($lock_info as $lock_type => $expires) {
							?>
							<tr>
								<?php
								if (!$printed_header) {
									?>
									<td class="sq-backend-table-cell" rowspan="<?php echo $locks['num_locks']; ?>">
										<?php
										if (isset($asset_info[$userid])) {
											$name = $asset_info[$userid]['name'];
											$type = $asset_info[$userid]['type_code'];
										} else {
											$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($userid);
											$name = $asset->name;
											$type = $asset->type();
											$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
										}
										?>
										<script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo $GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($type); ?>", "16", "16", "");</script>
										&nbsp;<acronym title="<?php echo ucwords(str_replace('_', ' ', $type)); ?> - <?php echo $userid; ?>"><?php echo $name; ?></acronym>
									</td>
									<?php
									$printed_header = true;
								}
								?>
								<td class="sq-backend-table-cell">
									<?php echo $lock_type; ?>
								</td>
								<td class="sq-backend-table-cell">
									<?php
									if (isset($asset_info[$assetid])) {
										$name = $asset_info[$assetid]['name'];
										$type = $asset_info[$assetid]['type_code'];
									} else {
										$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
										$name = $asset->name;
										$type = $asset->type();
										$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
									}
									?>
									<script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo $GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($type); ?>", "16", "16", "");</script>
									&nbsp;<acronym title="<?php echo ucwords(str_replace('_', ' ', $type)); ?> - <?php echo $assetid; ?>"><?php echo $name; ?></acronym>
								</td>
								<td class="sq-backend-table-cell">
									<?php
									$expires_in = easy_time_total((strtotime($expires) - time()), true);
									if (!$expires_in) $expires_in = '1 second';
									echo 'The lock is due to expire in '.$expires_in;
									?>
								</td>
							</tr>
							<?php
						}
					}
				}
				?>
				</table>
				<?php
			$o->closeField();
		$o->closeSection();

	}//end paintActiveLocks()


	/**
	* Processess the list of active locks
	*
	* @param object Backend_Outputter	&$o		reference to the backend outputter
	*
	* @return void
	* @access public
	*/
	function processActiveLocks(&$o)
	{

	}//end processActiveLocks()


	/**
	* Get details about a specific patch
	*
	* Note that the patch must have been downloaded and now reside in the
	* downloaded_patches directory
	*
	* @param string $patchid	the 12 digit ID of the patch
	*
	* @access private
	* @return array
	* @static
	*/
	function getPatchDetails($patchid)
	{
		require_once 'XML/Tree.php';
		
		$patch_file = SQ_DATA_PATH.'/private/maps/downloaded_patches/'.$patchid.'/patch'.$patchid.'.xml';
		$input = new XML_Tree($patch_file);
		$root  = &$input->getTreeFromFile();
		if (PEAR::isError($root)) {
			trigger_error('Unable to Read File : '.$patch_file."\n".$root->getMessage()."\n".$root->getUserInfo(), E_USER_WARNING);
			return;
		}

		$patch_info = Array('id' => $patchid, 'file_path' => $patch_file);
		$patch_info['reversible'] = $root->attributes['reversible'];
		$patch_info['severity']   = $root->attributes['severity'];
		$patch_info['run_script'] = $root->attributes['run_script'];
		
		foreach ($root->children as $node) {
			switch (strtolower($node->name)) {
				case 'requirements' :
					$patch_info['requirements'] = Array();
					foreach ($node->children as $requirement) {
						if (strtolower($requirement->name) != 'requirement') continue;
						if (!isset($requirement->attributes['type'])) continue;

						$code = '';
						$version = '';
						$new_version = '';
						foreach ($requirement->children as $require) {
							if (strtolower($require->name) == 'code') $code = $require->content;
							else if (strtolower($require->name) == 'version') $version = $require->content;
							else if (strtolower($require->name) == 'new_version') $new_version = $require->content;
						}
						if (empty($code)) continue;

						switch (strtolower(trim($requirement->attributes['type']))) {
							case 'system' :
								$patch_info['requirements']['system']['old_version'] = $version;
								$patch_info['requirements']['system']['new_version'] = $new_version;
							break;

							case 'package' :
								$patch_info['requirements']['packages'][$code]['old_version'] = $version;
								$patch_info['requirements']['packages'][$code]['new_version'] = $new_version;
							break;

							case 'asset'  :
								$patch_info['requirements']['assets'][$code]['old_version'] = $version;
								$patch_info['requirements']['assets'][$code]['new_version'] = $new_version;
							break;
						}
					}
				break;

				case 'patch_description'   :
				case 'reverse_description' :
					$patch_info[strtolower($node->name)] = trim($node->content);
				break;

				case 'changelog' :
					$patch_info['changelog'] = Array();
					foreach ($node->children as $change) {
						if (strtolower($change->name) != 'change') continue;
						$patch_info['changelog'][] = $change->content;
					}
				break;

				case 'files' :
					$patch_info['new_files']     = Array();
					$patch_info['updated_files'] = Array();
					$patch_info['deleted_files'] = Array();
					foreach ($node->children as $file) {
						if (strtolower($file->name) != 'file') continue;
						$name = trim($file->content);
						if (!isset($file->attributes['local_md5'])) {
							$patch_info['new_files'][$name] = Array();
							$patch_info['new_files'][$name]['md5'] = $file->attributes['md5'];
						} else if (!isset($file->attributes['md5'])) {
							$patch_info['deleted_files'][$name] = Array();
							$patch_info['deleted_files'][$name]['local_md5'] = $file->attributes['local_md5'];
						} else {
							$patch_info['updated_files'][$name] = Array();
							$patch_info['updated_files'][$name]['md5'] = $file->attributes['md5'];
							$patch_info['updated_files'][$name]['local_md5'] = $file->attributes['local_md5'];
						}
					}
				break;

				case 'queries'         :
				case 'reverse_queries' :
					$patch_info[strtolower($node->name)] = Array();
					foreach ($node->children as $query) {
						if (strtolower($query->name) != 'query') continue;
						if ($query->attributes['db'] == 'all' || $query->attributes['db'] ==  $GLOBALS['SQ_SYSTEM']->db->phptype) {
							$patch_info[strtolower($node->name)][] = $query->content;
						}
					}
				break;

			}//end switch
		}//end foreach

		$released = substr($patchid, 0, 4).'-'.substr($patchid, 4, 2).'-'.substr($patchid, 6, 2).' '.substr($patchid, 8, 2).':'.substr($patchid, 10, 2).':00';
		$patch_info['released'] = date('F j, Y, g:i a', strtotime($released));

		return $patch_info;

	}//end getPatchDetails()


}//end class

?>
