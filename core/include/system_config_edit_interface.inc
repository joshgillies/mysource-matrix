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
* $Id: system_config_edit_interface.inc,v 1.1 2003/12/30 05:25:38 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


/**
* System_Config_Edit_Interface
*
* Purpose
*
*    Looks after the editing of the system config
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
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

		// if we dont have access, piss off
		if (!$is_admin) {
			$GLOBALS['SQ_SYSTEM']->paintLogin('Login', 'You do not have permission to access the <i>System Configuration</i>');
			exit();
		}

		$o = &$backend->out;

		$o->setHeading('System Configuration', '<script language="JavaScript" type="text/javascript">sq_print_icon("'.$o->filesPath('/images/icons/header/system_config.png').'", "20", "20", "System Configuration");</script>');
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
		$current_location = $o->getCurrentLocation();
		$current_location .= (strstr($current_location, '?')) ? '&' : '?';

		$config_screens = Array(
									'system_config' => 'System Configuration',
									'hipo_config'   => 'HIPO Configuration',
									'user_prefs'    => 'Global Preferences',
								);

		foreach ($config_screens as $screen_code => $screen_name) {
			$url = $current_location.'config_screen='.rawurlencode($screen_code);
			$o->addStaticScreen($url, $screen_name);
		}

		$o->setCurrentScreen($current_location.'config_screen='.rawurlencode($active_screen));


		  ///////////////
		 //  LOCKING  //
		///////////////
		$lock_code = get_class($this).rawurlencode($active_screen);
		$lock = $GLOBALS['SQ_SYSTEM']->getLockInfo($lock_code);
		$have_lock = (!empty($lock) && $GLOBALS['SQ_SYSTEM']->currentUserId() == $lock['userid']);

		if (!empty($_POST['config_posted'])) {

			$saved = false;
			if (!empty($_POST['process_form'])) {
				$lock = $GLOBALS['SQ_SYSTEM']->getLockInfo($lock_code);
				if (empty($lock) || $GLOBALS['SQ_SYSTEM']->currentUserId() != $lock['userid']) {
					trigger_error('To save the '.$config_screens[$active_screen].', you need to be currently holding the lock', E_USER_WARNING);
				} else {
					$fn = 'process'.str_replace('_', '', $active_screen);
					$saved = $this->$fn($o, $have_lock);
				}
			}

			if (!empty($_POST['sq_lock_release']) || !empty($_POST['sq_lock_release_manual'])) {
				$GLOBALS['SQ_SYSTEM']->releaseLock($lock_code);
			}

			// if there is no lock currently and we want it
			if (!empty($_POST['sq_lock_acquire'])) {
				$GLOBALS['SQ_SYSTEM']->acquireLock($lock_code);
			}

			// if the save was successful, we need to redirect because the constants
			// that are already defined cannot be redefined in this script execution
			if ($saved) {
				$o->setRedirect($o->getCurrentLocation());
				return;
			}

			// just to make sure 
			$lock      = $GLOBALS['SQ_SYSTEM']->getLockInfo($lock_code);
			$have_lock = (!empty($lock) && $GLOBALS['SQ_SYSTEM']->currentUserId() == $lock['userid']);

		}// endif


		$o->openSection('Locking / Editing');
		$o->openField('&nbsp;');
		?>
		<table border="0" cellspacing="3" cellpadding="1">
			<tr>
				<td valign="top" width="30">
					<script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo sq_web_path('lib'); ?>/web/images/icons/<?php echo ((empty($lock)) ? 'un' : ''); ?>locked.png", "16", "16", "");</script>
				</td>
				<td valign="top">
					<?php
					if (!empty($lock)) {
						// this asset is currently locked
						// so display message to the user
						$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($lock['userid']);

						$now = time();

						require_once SQ_FUDGE_PATH.'/general/datetime.inc';
						$expires_in = easy_time_total(($lock['expires'] - $now), true);
						if (!$expires_in) $expires_in = '1 second';
						$expires_in = 'The lock is due to expire in '.$expires_in;

						?>
						<p class="sq-backend-locked-by-<?php echo ($GLOBALS['SQ_SYSTEM']->currentUser($user)) ? 'user' : 'someone-else'; ?>">
							The <?php echo $config_screens[$active_screen]; ?> screen is currently locked for editing by user "<?php echo $user->name; ?>".<br/>
							<?php echo $expires_in; ?>
						</p>
						<?php

					} else {
						?>
						<p class="sq-backend-unlocked">
							The <?php echo $config_screens[$active_screen]; ?> screen is currently Unlocked.
						</p>
						<?php
					}
					?>
				</td>
				<td valign="top">
					<?php
					if (!empty($lock)) {
						if ($have_lock) {
							submit_button('sq_lock_release_manual', 'Release Lock', 'set_hidden_field("process_form", "0");');
						}
					} else {
						submit_button('sq_lock_acquire', 'Lock', 'set_hidden_field("process_form", "0");');
					}
					?>
				</td>
			</tr>
		</table>
		<?php

		$o->closeSection();

		$fn = 'paint'.str_replace('_', '', $active_screen);
		$this->$fn($o, $have_lock);

		if ($have_lock) $o->commitButton('Commit', true);

	}//end paint()


	/**
	* Paints the config options for the main system
	*
	* @param object Backend_Outputter	&$o			reference to the backend outputter
	* @param boolean					$have_lock	do we have the lock to edit these cofnig settings or not
	*
	* @return void
	* @access public
	*/
	function paintSystemConfig(&$o, $have_lock)
	{
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

		$o->openSection('System Settings');

			$o->openField('System Name');
			if($have_lock) {
				text_box('config[SQ_CONF_SYSTEM_NAME]', SQ_CONF_SYSTEM_NAME, 30);
			} else {
				echo SQ_CONF_SYSTEM_NAME;
			}

			$o->openField('System Owner');
			if($have_lock) {
				text_box('config[SQ_CONF_SYSTEM_OWNER]', SQ_CONF_SYSTEM_OWNER, 30);
			} else {
				echo SQ_CONF_SYSTEM_OWNER;
			}

			$o->openField('System Root URLs');
			if($have_lock) {
				text_area('config[SQ_CONF_SYSTEM_ROOT_URLS]', SQ_CONF_SYSTEM_ROOT_URLS, 40, 4);
			} else {
				echo nl2br(SQ_CONF_SYSTEM_ROOT_URLS);
			}

		$o->closeSection();

		$o->openSection('Email Settings');

			$o->openField('Default Email');
			if($have_lock) {
				text_box('config[SQ_CONF_DEFAULT_EMAIL]', SQ_CONF_DEFAULT_EMAIL, 50);
			} else {
				echo SQ_CONF_DEFAULT_EMAIL;
			}

			$o->openField('Tech Email');
			if($have_lock) {
				text_box('config[SQ_CONF_TECH_EMAIL]', SQ_CONF_TECH_EMAIL, 50);
			} else {
				echo SQ_CONF_TECH_EMAIL;
			}

		$o->closeSection();

		$o->openSection('Login/Session Settings');

			$o->openField('Max Login Attempts', '', 'Zero (0) is unlimited attempts');
			if($have_lock && $is_root) {
				text_box('config[SQ_CONF_MAX_LOGIN_ATTEMPTS]', SQ_CONF_MAX_LOGIN_ATTEMPTS, 5, 5);
			} else {
				echo SQ_CONF_MAX_LOGIN_ATTEMPTS;
			}

			$o->openField('Max Idle Time', '', 'In seconds - Min: 60 or zero (0) for unlimited idle time');

			if($have_lock && $is_root) {
				text_box('config[SQ_CONF_MAX_IDLE_TIME]', SQ_CONF_MAX_IDLE_TIME, 10, 10);
			} else {
				echo SQ_CONF_MAX_IDLE_TIME;
			}

		$o->closeSection();


		$o->openSection('Intervals');

			$o->openField('Refresh Interval', '', 'How long (in seconds) between refreshes of the top (header) frame - this frame keeps the lock on long edits and checks for new mail - Min: 60');
			if($have_lock) {
				text_box('config[SQ_CONF_REFRESH_INTERVAL]', SQ_CONF_REFRESH_INTERVAL, 5, 5);
			} else {
				echo SQ_CONF_REFRESH_INTERVAL;
			}

			$o->openField('Lock Length', '', 'The length of time after which a lock is considered to have expired. In seconds - Min: 60');
			if($have_lock) {
				text_box('config[SQ_CONF_LOCK_LENGTH]', SQ_CONF_LOCK_LENGTH, 5, 5);
			} else {
				echo SQ_CONF_LOCK_LENGTH;
			}

		$o->closeSection();


		$o->openSection('Error Settings');

			$o->openField('Log Errors');
			if($have_lock && $is_root) {
				combo_box('config[SQ_CONF_LOG_ERRORS]', Array('1' => 'Yes', '0' => 'No'), false, SQ_CONF_LOG_ERRORS);
			} else {
				echo (SQ_CONF_LOG_ERRORS) ? 'Yes' : 'No';
			}

			$o->openField('Hide Errors on the Frontend');
			if($have_lock && $is_root) {
				combo_box('config[SQ_CONF_ERRORS_HIDE_FRONTEND]', Array('1' => 'Yes', '0' => 'No'), false, SQ_CONF_ERRORS_HIDE_FRONTEND);
			} else {
				echo (SQ_CONF_ERRORS_HIDE_FRONTEND) ? 'Yes' : 'No';
			}

		$o->closeSection();

		$o->openSection('Internationalisation Settings');

			$o->openField('Default Frontend Language');
			require SQ_FUDGE_PATH.'/standards_lists/languages.inc';
			if($have_lock) {
				combo_box('config[SQ_CONF_DEFAULT_FRONTEND_LANGUAGE]', $standards_lists_languages, false, SQ_CONF_DEFAULT_FRONTEND_LANGUAGE);
			} else {
				echo $standards_lists_languages[SQ_CONF_DEFAULT_FRONTEND_LANGUAGE];
			}

			$o->openField('Default Character Set');
			require SQ_FUDGE_PATH.'/standards_lists/character_sets.inc';
			if($have_lock) {
				combo_box('config[SQ_CONF_DEFAULT_CHARACTER_SET]', $standards_lists_character_sets, false, SQ_CONF_DEFAULT_CHARACTER_SET);
			} else {
				echo $standards_lists_character_sets[SQ_CONF_DEFAULT_CHARACTER_SET];
			}

		$o->closeSection();

	}//end paintSystemConfig()


	/**
	* Saves the system config settings that were submitted
	*
	* @param object Backend_Outputter	&$o			reference to the backend outputter
	* @param boolean					$have_lock	do we have the lock to edit these cofnig settings or not
	*
	* @return boolean
	* @access public
	*/
	function processSystemConfig(&$o, $have_lock)
	{
		if (empty($_POST['config'])) return false;

		require_once SQ_INCLUDE_PATH.'/system_config.inc';
		$config = new System_Config();

		return $config->save($_POST['config']);

	}//end processSystemConfig()


	/**
	* Paints the config options for the HIPO server
	*
	* @param object Backend_Outputter	&$o			reference to the backend outputter
	* @param boolean					$have_lock	do we have the lock to edit these cofnig settings or not
	*
	* @return void
	* @access public
	*/
	function paintHipoConfig(&$o, $have_lock)
	{
		require_once SQ_DATA_PATH.'/private/conf/hipo.inc';

		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

		$o->openSection('HIPO Server Configuration');

			$o->openField('Use HIPO Server');
			if($have_lock && $is_root) {
				combo_box('config[SQ_HIPO_USE_SERVER]', Array('1' => 'Yes', '0' => 'No'), false, SQ_HIPO_USE_SERVER);
			} else {
				echo (SQ_HIPO_USE_SERVER) ? 'Yes' : 'No';
			}

			$o->openField('HIPO Server Domain');
			if($have_lock) {
				text_box('config[SQ_HIPO_SERVER_DOMAIN]', SQ_HIPO_SERVER_DOMAIN, 30);
			} else {
				echo SQ_HIPO_SERVER_DOMAIN;
			}

			$o->openField('HIPO Server Port');
			if($have_lock) {
				text_box('config[SQ_HIPO_SERVER_PORT]', SQ_HIPO_SERVER_PORT, 10);
			} else {
				echo SQ_HIPO_SERVER_PORT;
			}

			$o->openField('HIPO Server System ID');
			if($have_lock) {
				text_box('config[SQ_HIPO_SERVER_SYSTEMID]', SQ_HIPO_SERVER_SYSTEMID, 30);
			} else {
				echo SQ_HIPO_SERVER_SYSTEMID;
			}

		$o->closeSection();

	}//end paintHipoConfig()


	/**
	* Saves the HIPO config settings that were submitted
	*
	* @param object Backend_Outputter	&$o			reference to the backend outputter
	* @param boolean					$have_lock	do we have the lock to edit these cofnig settings or not
	*
	* @return boolean
	* @access public
	*/
	function processHipoConfig(&$o, $have_lock)
	{
		if (empty($_POST['config'])) return false;

		require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_config.inc';
		$hipo_config = new HIPO_Config();
		return $hipo_config->save($_POST['config']);

	}//end processHipoConfig()


	/**
	* Allow editing of the user preferece settings
	*
	* @param object Backend_Outputter	&$o			reference to the backend outputter
	* @param boolean					$have_lock	do we have the lock to edit these cofnig settings or not
	*
	* @return void
	* @access public
	*/
	function paintUserPrefs(&$o, $have_lock)
	{
		include SQ_DATA_PATH.'/private/conf/preferences.inc';

		foreach ($preferences as $type_code => $pref_vars) {
			$o->openSection($GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code, 'name').' Preferences');
				$path = SQ_SYSTEM_ROOT.'/'.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code, 'dir').'/'.$type_code.'_prefs.inc';
				require_once $path;
				$pref_name = $type_code.'_Prefs';
				$prefs = new $pref_name();
				$prefs->paintBackend($o, $have_lock);
			$o->closeSection();
		}

	}//end paintUserPrefs()


	/**
	* Saves the user preference settings that were submitted
	*
	* @param object Backend_Outputter	&$o			reference to the backend outputter
	* @param boolean					$have_lock	do we have the lock to edit these cofnig settings or not
	*
	* @return boolean
	* @access public
	*/
	function processUserPrefs(&$o, $have_lock)
	{
		include SQ_DATA_PATH.'/private/conf/preferences.inc';
		$new_preferences = $preferences;

		foreach ($preferences as $type_code => $pref_vars) {
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
		foreach($preferences as $type_code => $pref_data) {
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
		if (!string_to_file($str, SQ_DATA_PATH.'/private/conf/preferences.inc')) return false;

		if ($changed_vars) {
			$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
			$message_body = '';
			foreach($changed_vars as $var_name => $values) {
				$message_body .= ucwords(str_replace('_', ' ', $values['type_code'])).' preference variable "'.$var_name.'" changed from '.var_export($values['old'], true).' to '.var_export($values['new'], true)."\n";
			}
			$message = $ms->newMessage(Array(), 'System Preferences Updated', $message_body, 'prefs.system');
			$message->send();

		}// end if

		return true;

	}//end processUserPrefs()


}//end class

?>