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
* $Id: system_config.inc,v 1.19 2003/12/30 05:25:38 gsherwood Exp $
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
		'SQ_CONF_BACKEND_SUFFIX'			=> Array('editable' => 0, 'default' => '_admin'),
		'SQ_CONF_LIMBO_SUFFIX'				=> Array('editable' => 0, 'default' => '_edit'),
/*
* Fixed Size of the number of digits that each link trees entry can be, run
*   pow(SQ_CONF_ASSET_TREE_BASE, SQ_CONF_ASSET_TREE_SIZE)
* to get max number of possible link entries per node
*/
		'SQ_CONF_ASSET_TREE_BASE'			=> Array('editable' => 0, 'default' => 64),
		'SQ_CONF_ASSET_TREE_SIZE'			=> Array('editable' => 0, 'default' =>  4),

		'SQ_CONF_ROLLBACK_ENABLED'			=> Array('editable' => 0, 'default' => '0'),

		// Indexing Settings
		'SQ_CONF_INDEXING_ENABLED'			=> Array('editable' => 0, 'default' => '0'),

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
		
		// Allow IP to fluctuate ?
		'SQ_CONF_ALLOW_IP_CHANGE'			=> Array('editable' => 1, 'default' => '0'),

	);


	/**
	* Constructor
	*
	*/
	function System_Config()
	{
		$this->Config();
		$this->config_file = SQ_DATA_PATH.'/private/conf/main.inc';

	}//end constructor


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
		if (empty($GLOBALS['SQ_INSTALL'])) {

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

	}//end save()


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
					echo "\$inc_dir = (substr(\$inc_dir, 0, 2) == '.".$sep."') ? '.".$sep."'.SQ_CONF_PEAR_PATH.'".$sep."'.substr(\$inc_dir, 2) : SQ_CONF_PEAR_PATH.'".$sep."'.\$inc_dir;\n";
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


}//end class

?>