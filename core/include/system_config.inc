<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: system_config.inc,v 1.8 2003/10/06 23:35:39 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_LIB_PATH.'/config/config.inc';

/**
* System_Config
*
* Purpose
*
*    Looks after the creation and editing of the system config
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class System_Config extends Config
{
	var $config_vars = Array(
		'SQ_CONF_DB_DSN'					=> Array('editable' => 0, 'default' => ''),
		'SQ_CONF_DB2_DSN'					=> Array('editable' => 0, 'default' => ''),
		'SQ_CONF_PEAR_PATH'					=> Array('editable' => 0, 'default' => ''),
		'SQ_CONF_BACKEND_SUFFIX'			=> Array('editable' => 0, 'default' => '_edit'),
/*
* Fixed Size of the number of digits that each link trees entry can be, run
*   pow(SQ_CONF_ASSET_TREE_BASE, SQ_CONF_ASSET_TREE_SIZE)
* to get max number of possible link entries per node
*/
		'SQ_CONF_ASSET_TREE_BASE'			=> Array('editable' => 0, 'default' => 64),
		'SQ_CONF_ASSET_TREE_SIZE'			=> Array('editable' => 0, 'default' =>  4),

		'SQ_CONF_ROLLBACK_ENABLED'			=> Array('editable' => 0, 'default' => '0'),


//// START EDITABLE SETTINGS ////
		// System Settings
		'SQ_CONF_SYSTEM_NAME'				=> Array('editable' => 1, 'default' => 'The System'),
		'SQ_CONF_SYSTEM_OWNER'				=> Array('editable' => 1, 'default' => ''),
		'SQ_CONF_SYSTEM_ROOT_URLS'			=> Array('editable' => 1, 'default' => ''),

		// Email Settings
		'SQ_CONF_DEFAULT_EMAIL'				=> Array('editable' => 1, 'default' => ''),
		'SQ_CONF_TECH_EMAIL'				=> Array('editable' => 1, 'default' => ''),

		// Session/Login Settings
		'SQ_CONF_MAX_LOGIN_ATTEMPTS'		=> Array('editable' => 1, 'default' => 3),
		'SQ_CONF_MAX_IDLE_TIME'				=> Array('editable' => 1, 'default' => 6000),

		// Interval Settings
		'SQ_CONF_REFRESH_INTERVAL'			=> Array('editable' => 1, 'default' => 120),
		'SQ_CONF_LOCK_LENGTH'				=> Array('editable' => 1, 'default' => 600),

		// Error Settings
		'SQ_CONF_LOG_ERRORS'				=> Array('editable' => 1, 'default' => true),
		'SQ_CONF_ERRORS_HIDE_FRONTEND'		=> Array('editable' => 1, 'default' => false),

		// Internationalisation Settings
		'SQ_CONF_DEFAULT_FRONTEND_LANGUAGE'	=> Array('editable' => 1, 'default' => 'en'),
		'SQ_CONF_DEFAULT_CHARACTER_SET'		=> Array('editable' => 1, 'default' => 'iso-8859-1'),
	);


	/**
	* Constructor
	*
	*/
	function System_Config()
	{
		$this->Config();
		$this->config_file = SQ_DATA_PATH.'/private/conf/main.inc';
	}


	/**
	* Rewrites the conf file with current variables
	*
	* @param Array		$vars				the array with any new values (that are allowed to be edited) Array('[config_var_name]' => [scalar value])
	* @param boolean	$backup_existing	whether we should backup the existing config file first
	*
	* @return boolean	indicates whether the file was written
	* @access public
	*/
	function save($vars, $backup_existing=false)
	{

		if (empty($GLOBALS['SQ_INSTALL'])) {

			$lock = $GLOBALS['SQ_SYSTEM']->getLockInfo(get_class($this));
			// if we dont have the lock, piss off
			if (empty($lock) || $GLOBALS['SQ_SYSTEM']->currentUserId() != $lock['userid']) {
				trigger_error('To save the System Config, you need to be currently holding the lock', E_USER_WARNING);
				return false;
			}

			$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
			$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

			// if we dont have access, piss off
			if (!$is_admin) {
				trigger_error('You need to be either the Root User or a System Administrator to save the System Config', E_USER_WARNING);
				return false;
			}

			// Now just make sure they aren't trying to set anything that they shouldn't
			if (!$is_root) {
				unset($vars['SQ_CONF_MAX_LOGIN_ATTEMPTS']);
				unset($vars['SQ_CONF_MAX_IDLE_TIME']);
				unset($vars['SQ_CONF_LOG_ERRORS']);
				unset($vars['SQ_CONF_ERRORS_HIDE_FRONTEND']);
			}//end if 

			// Now do some validation
			if (isset($vars['SQ_CONF_SYSTEM_ROOT_URLS'])) {
				$vars['SQ_CONF_SYSTEM_ROOT_URLS'] = str_replace("\r", '', $vars['SQ_CONF_SYSTEM_ROOT_URLS']);
			}
			foreach(Array('SQ_CONF_MAX_IDLE_TIME', 'SQ_CONF_REFRESH_INTERVAL', 'SQ_CONF_LOCK_LENGTH') as $var_name) {
				if (isset($vars[$var_name])) {
					$vars[$var_name] = max(60, (int) $vars[$var_name]);
				}
			}

		}// end if

		return parent::save($vars, $backup_existing);

	}// end save()


	/**
	* Paint's out the code for the config file for the passed config
	* This allows extra things to be added for special cases
	*
	* @param string $name	the config var name
	* @param mixed  $value	the value that the config var has
	*
	* @return boolean	whether the value is valid or not
	*/
	function _paintConfigVar($var_name, $value)
	{
		switch($var_name) {
			case 'SQ_CONF_PEAR_PATH' :
				if (!parent::_paintConfigVar($var_name, $value)) return false;
				if ($value) {
					$sep = (substr(PHP_OS, 0, 3) == 'WIN') ? ';' : ':';

					// basically, if the first entry is '.' (ie the current directory) leave that,
					// and insert the pear path just after that, otherwise just right at the front of the
					// include path
					echo "\$inc_dir = ini_get('include_path');\n";
					echo "\$inc_dir = (substr(\$inc_dir, 0, 2) == '.".$sep."') ? '.".$sep.$value.$sep."'.substr(\$inc_dir, 2) : '".$value.$sep."'.\$inc_dir;\n";
					echo "ini_set('include_path', \$inc_dir);\n\n";
				}
				return true;
				break;

			case 'SQ_CONF_LOG_ERRORS' :
				if (!parent::_paintConfigVar($var_name, $value)) return false;
				// NOTE: if they don't want to log errors we will leave
				//       them with whatever their PHP settings are
				if ($value) {
					echo "ini_set('log_errors', '1');\n";
					echo "ini_set('error_log',  SQ_DATA_PATH.'/private/logs/error.log');\n";
				}
				return true;
				break;

			default :
				return parent::_paintConfigVar($var_name, $value);

		}// end switch

	}//end _paintConfigVar()


	/**
	* Lets print the conf backend
	*
	* @param object Backend	$backend	Reference to the backend object
	*/
	function paintBackend(&$backend)
	{
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

		// if we dont have access, piss off
		if (!$is_admin) {
			$GLOBALS['SQ_SYSTEM']->paintLogin('Login', 'You do not have permission to access the <i>System Config</i>');
			exit();
		}

		$lock = $GLOBALS['SQ_SYSTEM']->getLockInfo(get_class($this));
		$have_lock = (!empty($lock) && $GLOBALS['SQ_SYSTEM']->currentUserId() == $lock['userid']);

		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		$o = &$backend->out;

		if (!empty($_POST['config_posted'])) {

			$saved = false;
			if (!empty($_POST['process_form']) && !empty($_POST['config'])) {
				$saved = $this->save($_POST['config']);
			}

			if (!empty($_POST['sq_release_lock']) || !empty($_POST['sq_manual_release_lock'])) {
				$GLOBALS['SQ_SYSTEM']->releaseLock(get_class($this));
			}

			// if there is no lock currently and we want it
			if (!empty($_POST['sq_acquire_lock'])) {
				$GLOBALS['SQ_SYSTEM']->acquireLock(get_class($this));
			}

			// if the save was successful, we need to redirect because the constants
			// that are already defined cannot be redefined in this script execution
			if ($saved) {
				$o->setRedirect($o->getCurrentLocation());
				return;
			} 

			// just to make sure 
			$lock      = $GLOBALS['SQ_SYSTEM']->getLockInfo(get_class($this));
			$have_lock = (!empty($lock) && $GLOBALS['SQ_SYSTEM']->currentUserId() == $lock['userid']);

		}// endif

		$o->setHeading('System Configuration', 'create');
		$o->addHiddenField('config_posted', '1');

		// lets tell the user if the asset is locked for editing
		// or if they can lock it to edit it
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

						echo '<p class="sq-backend-locked">System Configuration is currently locked for editing by user "'.$user->name.'". <br>'.$expires_in.'.</p>';

					} else {
						echo '<p class="sq-backend-unlocked">The System Configuration is currently Unlocked.</p>';
					}
					?>
				</td>
				<td valign="top">
					<?php
					if (!empty($lock)) {
						if ($have_lock) {
							submit_button('sq_manual_release_lock', 'Release Lock', 'set_hidden_field("process_form", "0");');
						}
					} else {
						submit_button('sq_acquire_lock', 'Lock', 'set_hidden_field("process_form", "0");');
					}
					?>
				</td>
			</tr>
		</table>
		<?php

		$o->closeSection();

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
				combo_box('config[SQ_CONF_LOG_ERRORS]', Array(true => 'Yes', false => 'No'), false, SQ_CONF_LOG_ERRORS);
			} else {
				echo (SQ_CONF_LOG_ERRORS) ? 'Yes' : 'No';
			}

			$o->openField('Hide Errors on the Frontend');
			if($have_lock && $is_root) {
				combo_box('config[SQ_CONF_ERRORS_HIDE_FRONTEND]', Array(true => 'Yes', false => 'No'), false, SQ_CONF_ERRORS_HIDE_FRONTEND);
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

		if ($have_lock) {
			$o->commitButton('Commit', true);
		}

	}//end print_backend()

}//end class

?>
