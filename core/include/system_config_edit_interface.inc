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
* $Id: system_config_edit_interface.inc,v 1.2.2.2 2004/04/21 10:55:17 brobertson Exp $
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
									'system_config'			=> 'System Configuration',
									'user_prefs'			=> 'Global Preferences',
									'squiz_server_config'	=> 'Squiz Server Configuration',
									'hipo_config'			=> 'HIPO Configuration',
									'replication_config'	=> 'Replication Configuration',
								);

		foreach ($config_screens as $screen_code => $screen_name) {
			$url = $current_location.'config_screen='.rawurlencode($screen_code);
			$o->addStaticScreen($url, $screen_name);
		}

		$o->setCurrentScreen($current_location.'config_screen='.rawurlencode($active_screen));


		switch($active_screen) {
			case 'system_config' :
			case 'squiz_server_config' :
			case 'hipo_config' :
			case 'replication_config' :

				$config = null;

				switch($active_screen) {
					case 'system_config' :
						require_once SQ_INCLUDE_PATH.'/system_config.inc';
						$config = new System_Config();
						break;
					case 'squiz_server_config' :
						require_once SQ_SYSTEM_ROOT.'/core/server/squiz_server_config.inc';
						$config = new Squiz_Server_Config();
						break;
					case 'hipo_config' :
						require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_config.inc';
						$config = new HIPO_Config();
						break;
					case 'replication_config' :
						require_once SQ_SYSTEM_ROOT.'/core/replication/replication_config.inc';
						$config = new Replication_Config();
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

			default :
				trigger_error('Active Screen "'.$active_screen.'" not known', E_USER_WARNING);
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
							The User Preferences screen is currently locked for editing by user "<?php echo $user->name; ?>".<br/>
							<?php echo $expires_in; ?>
						</p>
						<?php

					} else {
						?>
						<p class="sq-backend-unlocked">
							The User Preferences screen is currently Unlocked.
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

		if ($have_lock) $o->commitButton('Commit', true);

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
			require_once SQ_FUDGE_PATH.'/general/file_system.inc';
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


}//end class

?>
