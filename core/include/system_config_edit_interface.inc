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
* $Id: system_config_edit_interface.inc,v 1.47 2013/07/25 23:23:49 lwright Exp $
*
*/


/**
* System_Config_Edit_Interface
*
* Purpose
*
*    Looks after the editing of the system config
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.47 $
* @package MySource_Matrix
*/
class System_Config_Edit_Interface extends MySource_Object
{


	/**
	* Constructor
	*
	*/
	function System_Config_Edit_Interface()
	{

	}//end constructor


	/**
	* Lets print the conf backend
	*
	* @param object Backend	$backend	Reference to the backend object
	*
	* @return void
	*/
	function paint(&$backend)
	{
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

		// if we dont have access, go away
		if (!$is_admin) {
			$GLOBALS['SQ_SYSTEM']->paintLogin(translate('login'), translate('cannot_access_asset'));
			exit();
		}

		$o = $backend->out;

		$o->setHeading(translate('system_configuration'), sq_get_icon($o->filesPath('/images/icons/header/system_config.png'), '', '', 'System Configuration'));
		$o->setPageTitle(translate('system_configuration'));
		$o->addHiddenField('config_posted', '1');


		  /////////////////////
		 //  ACTIVE SCREEN  //
		/////////////////////
		if (empty($_REQUEST['config_screen'])) {
			$active_screen = 'system_config';
			$_REQUEST['config_screen'] = $active_screen;
		} else {
			$active_screen = $_REQUEST['config_screen'];
		}

		// make sure that we have the screen for next time
		$o->addFormActionGetVar('config_screen', $active_screen);


		  ///////////////////
		 //  SCREEN MENU  //
		///////////////////
		$o->addFormActionGetVar('config_screen', $active_screen, TRUE);
		$current_location = $o->getCurrentLocation();
		list($current_loc_base, $current_loc_query) = explode('?', $current_location);
		$config_screens = Array(
							'system_config'					=> translate('system_configuration'),
							'squiz_server_config'			=> translate('squiz_server_configuration'),
							'apache_config'					=> translate('apache_configuration'),
							'hipo_config'					=> translate('hipo_configuration'),
							'external_tools_config'			=> translate('external_tools_configuration'),
							'user_prefs'					=> translate('global_preferences'),
							'password_rules_config'			=> translate('password_rules_configuration'),
							'messaging_service_config'		=> translate('messaging_service_configuration'),
							'proxy_authentication_config'	=> translate('proxy_configuration'),
							'context_config'				=> translate('context_configuration'),
							'locks'							=> translate('active_locks'),
						  );

		foreach ($config_screens as $screen_code => $screen_name) {
			$url = replace_query_string_vars(Array('config_screen' => rawurlencode($screen_code)), $current_loc_base, $current_loc_query);
			$o->addStaticScreen($url, $screen_name);
		}


		switch ($active_screen) {
			case 'system_config' :
			case 'squiz_server_config' :
			case 'apache_config' :
			case 'hipo_config' :
			case 'external_tools_config' :
			case 'password_rules_config' :
			case 'session_rules_config' :
			case 'messaging_service_config' :
			case 'proxy_authentication_config' :
			case 'context_config':

				$config = null;

				switch ($active_screen) {
					case 'system_config' :
						require_once SQ_INCLUDE_PATH.'/system_config.inc';
						$config = new System_Config();
					break;
					case 'squiz_server_config' :
						require_once SQ_SYSTEM_ROOT.'/core/server/squiz_server_config.inc';
						$config = new Squiz_Server_Config();
					break;
					case 'apache_config' :
						require_once SQ_INCLUDE_PATH.'/apache_config.inc';
						$config = new Apache_Config();
					break;
					case 'hipo_config' :
						require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_config.inc';
						$config = new HIPO_Config();
					break;
					case 'external_tools_config' :
						require_once SQ_INCLUDE_PATH.'/external_tools_config.inc';
						$config = new External_Tools_Config();
					break;
					case 'password_rules_config' :
						require_once SQ_INCLUDE_PATH.'/password_rules_config.inc';
						$config = new Password_Rules_Config();
					break;
					case 'session_rules_config' :
						require_once SQ_INCLUDE_PATH.'/session_rules_config.inc';
						$config = new Session_Rules_Config();
					break;
					case 'messaging_service_config' :
						require_once SQ_INCLUDE_PATH.'/messaging_service_config.inc';
						$config = new Messaging_Service_Config();
					break;
					case 'proxy_authentication_config' :
						require_once SQ_INCLUDE_PATH.'/proxy_authentication_config.inc';
						$config = new Proxy_Authentication_Config();
					break;
					case 'context_config' :
						require_once SQ_INCLUDE_PATH.'/context_config.inc';
						$config = new Context_Config();
					break;

				}// end switch
				if (!empty($_POST['config_posted'])) {
					$config->processBackend($o);
				}

				$config->paintBackend($o);

				return;

				break;

			case 'user_prefs' :

				if (!empty($_POST['config_posted'])) {
					$this->processUserPrefs($o);
				}

				$this->paintUserPrefs($o);

				return;

			case 'locks' :
				$this->paintActiveLocks($o);
				return;
			default :
				trigger_localised_error('SYS0076', E_USER_WARNING, $active_screen);
		}// end switch

	}//end paint()


	/**
	* Allow editing of the user preferece settings
	*
	* @param object Backend_Outputter	&$o			reference to the backend outputter
	*
	* @return void
	* @access public
	*/
	function paintUserPrefs(&$o)
	{
		$lock_code = get_class($this).'_user_prefs';
		$lock = $GLOBALS['SQ_SYSTEM']->getLockInfo($lock_code);
		$have_lock = (!empty($lock) && $GLOBALS['SQ_SYSTEM']->currentUserId() == $lock['userid']);

		$is_admin = ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

		$o->openRaw();
		?>
			<div class="sq-backend-section-div-wrapper section custom first">
				<div class="sq-backend-section-div">
					<div id="sq-aquire-locks-wrapper" class="clearfix">
						<?php
						echo '<div class="sq-locks-button-wrapper">';
							if (!empty($lock)) {
								if ($have_lock) {
									submit_button('sq_lock_release_manual', translate('release_lock_button'), 'set_hidden_field("process_form", "0");', 'accesskey="r" class="sq-form-field sq-btn-large sq-btn-blue sq-lock-button release"');
								}
							} else {
								submit_button('sq_lock_acquire', translate('acquire_lock_button'), 'set_hidden_field("process_form", "0");', 'accesskey="a" class="sq-form-field sq-btn-large sq-btn-blue sq-lock-button acquire"');
							}
						echo '</div>';
						echo '<p class="sq-lock-message">';
							if (!empty($lock)) {
								// this asset is currently locked
								// so display message to the user
								$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($lock['userid']);

								$now = time();

								require_once SQ_FUDGE_PATH.'/general/datetime.inc';
								$expires_in = easy_time_total(($lock['expires'] - $now), true);
								if (!$expires_in) $expires_in = '1 second';
								$expires_in = sprintf(translate('due_to_expire'), $expires_in);
								if ($have_lock) {
									echo sprintf(translate('release_lock'), translate('release_lock_button'), translate('user_preferences'));
								}
								if ($user->id != $GLOBALS['SQ_SYSTEM']->currentUserId()) {
									echo sprintf(translate('lock_held_by_user'), htmlspecialchars($user->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)).'. ';
								}
								echo ' '.$expires_in.'.';

							} else {
								echo sprintf(translate('acquire_lock'), translate('acquire_lock_button'), translate('user_preferences'));
							}
						echo '</p>';
						?>
					</div><!--#sq-aquire-locks-wrapper-->
				</div><!--.sq-backend-section-div-->
			</div><!--.sq-backend-section-div-wrapper-->
		<?php
		$o->closeRaw();

		// Disable OPCache so that we get the updated preferences file
		if (!empty($_POST['process_form']) && ini_get('opcache.enable')) {
			ini_set('opcache.enable', '0');
		}

		include SQ_DATA_PATH.'/private/conf/preferences.inc';

		foreach ($preferences as $type_code => $pref_vars) {
			$o->openSection($GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code, 'name').' '.translate('preferences'));

				// We need to include the asset the prefs are for to make sure all locales are imported for it
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);

				$path = SQ_SYSTEM_ROOT.'/'.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code, 'dir').'/'.$type_code.'_prefs.inc';
				require_once $path;
				$pref_name = $type_code.'_Prefs';
				$prefs = new $pref_name();
				$prefs->paintBackend($o, $have_lock);
			$o->closeSection();
		}

		if ($have_lock) $o->commitButton('', true);

	}//end paintUserPrefs()


	/**
	* Saves the user preference settings that were submitted
	*
	* @param object Backend_Outputter	&$o			reference to the backend outputter
	*
	* @return boolean
	* @access public
	*/
	function processUserPrefs(&$o)
	{
		$is_admin = ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

		$lock_code = get_class($this).'_user_prefs';
		$lock = $GLOBALS['SQ_SYSTEM']->getLockInfo($lock_code);
		$have_lock = (!empty($lock) && $GLOBALS['SQ_SYSTEM']->currentUserId() == $lock['userid']);

		if (!empty($_POST['process_form'])) {

			include SQ_DATA_PATH.'/private/conf/preferences.inc';
			$new_preferences = $preferences;

			foreach ($preferences as $type_code => $pref_vars) {
				// We need to include the asset the prefs are for to make sure all locales are imported for it
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);

				$path = SQ_SYSTEM_ROOT.'/'.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code, 'dir').'/'.$type_code.'_prefs.inc';
				require_once $path;
				$pref_name = $type_code.'_Prefs';
				$prefs = new $pref_name();
				if ($prefs->processBackend($o, $have_lock)) {
					$new_preferences[$type_code] = $prefs->pref_vars;
				}
			}


			// go through and work out which vars have changed
			$changed_vars = Array();
			foreach ($preferences as $type_code => $pref_data) {
				foreach ($pref_data as $var_name => $var_data) {
					$current_value = $preferences[$type_code][$var_name]['default'];
					$new_value = $new_preferences[$type_code][$var_name]['default'];

					if ($current_value !== $new_value) {
						$changed_vars[$var_name] = Array('old' => $current_value, 'new' => $new_value, 'type_code' => $type_code);
					}
				}
			}

			// output the new preference file
			$str = '<'.'?php $preferences = '.var_export($new_preferences, true).'; ?'.'>';
			require_once SQ_FUDGE_PATH.'/general/file_system.inc';
			if (!string_to_file($str, SQ_DATA_PATH.'/private/conf/preferences.inc')) {
				return false;
			}

			if ($changed_vars) {
				$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
				$ms->openQueue();
				foreach ($changed_vars as $var_name => $values) {
					$msg_reps = Array(
									'type_code'	=> ucwords(str_replace('_', ' ', $values['type_code'])),
									'setting'	=> $var_name,
									'old_value'	=> var_export($values['old'], true),
									'new_value'	=> var_export($values['new'], true),
								);
					$message = $ms->newMessage(Array(), 'prefs.system', $msg_reps);
					$ms->enqueueMessage($message);
				}
				$ms->closeQueue();


			}// end if


		}// end if process_form

		if (!empty($_POST['sq_lock_release']) || !empty($_POST['sq_lock_release_manual'])) {
			$GLOBALS['SQ_SYSTEM']->releaseLock($lock_code);
		}

		// if there is no lock currently and we want it (and we are an admin)
		if (!empty($_POST['sq_lock_acquire']) && $is_admin) {
			$GLOBALS['SQ_SYSTEM']->acquireLock($lock_code);
		}


		return true;

	}//end processUserPrefs()


	/**
	* Paint a list of active locks
	*
	* @param object Backend_Outputter   &$o     reference to the backend outputter
	*
	* @access public
	* @return void
	*/
	function paintActiveLocks(&$o)
	{
		$class_name = 'locking_method_'.SQ_CONF_LOCKING_METHOD;
		$GLOBALS['SQ_SYSTEM']->am->includeAsset($class_name);
		$current_time = time();

		try {
			if (eval('return '.$class_name.'::supportsGetActiveLocks();') === FALSE) {
				$o->openSection(translate('active_locks'));
					$o->openField(sprintf(translate('system_config_active_locks_no_support'), SQ_CONF_LOCKING_METHOD));
					$o->closeField();
				$o->openSection();
				return TRUE;
			} else {
				$db_locks = eval('return '.$class_name.'::getActiveLocks(NULL);');
			}
		} catch (Exception $e) {
			return 'Unable to get active locks, '.$e->getMessage();
		}

		if (empty($db_locks)) {
			$o->openSection(translate('active_locks'));
				$o->openField('','wide_col');
					echo '<em>'.translate('no_active_locks').'.</em>';
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

			// Using assetExists() will not work for shadow assets to check if asset exists in system
			if (!$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, '', TRUE)) continue;

			$active_locks[$lock['userid']]['locks'][$assetid][$lock_type] = $lock['expires'];

			if (!isset($active_locks[$lock['userid']]['num_locks'])) $active_locks[$lock['userid']]['num_locks'] = 0;
			$active_locks[$lock['userid']]['num_locks']++;

			$asset_info_ids[$lock['userid']] = $lock['userid'];
			$asset_info_ids[$assetid] = $assetid;
		}

		$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($asset_info_ids));

		require_once SQ_FUDGE_PATH.'/general/datetime.inc';

		$o->openSection(translate('active_locks'));
			$o->openField('', 'wide_col');
				?>
				<table class="sq-backend-table">
					<tr>
						<th class="sq-backend-table-header"><?php echo translate('user'); ?></th>
						<th class="sq-backend-table-header"><?php echo translate('lock_type'); ?></th>
						<th class="sq-backend-table-header"><?php echo translate('maps_held_at'); ?></th>
						<th class="sq-backend-table-header"><?php echo translate('asset_type'); ?></th>
						<th class="sq-backend-table-header"><?php echo translate('expires'); ?></th>
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
									<th class="sq-backend-table-header" rowspan="<?php echo $locks['num_locks']; ?>">
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
									</th>
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
										$type = $asset_info[$assetid]['type_code'];
										$name = $asset_info[$assetid]['name'];
									} else {
										$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
										$name = $asset->name;
										$type = $asset->type();
										$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
									}
									echo get_asset_tag_line($assetid);
									?>
								</td>
								<td class="sq-backend-table-cell">
									<acronym title="<?php echo $type; ?> - <?php echo $assetid; ?>">
										<?php sq_print_icon($GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($type), 16, 16, '', NULL); ?>
										<?php echo ucwords(str_replace('_', ' ', $type)); ?>
									</acronym>
								</td>
								<td class="sq-backend-table-cell">
									<?php
									$expires_in = easy_time_total((strtotime($expires) - $current_time), true);
									if (!$expires_in) $expires_in = '1 second';
									echo sprintf(translate('due_to_expire'), $expires_in);
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
	* @param object Backend_Outputter   &$o     reference to the backend outputter
	*
	* @return void
	* @access public
	*/
	function processActiveLocks(&$o)
	{

	}//end processActiveLocks()


}//end class

?>
