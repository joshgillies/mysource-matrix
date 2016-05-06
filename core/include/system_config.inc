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
* $Id: system_config.inc,v 1.157 2013/09/03 03:49:53 ewang Exp $
*
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
* @version $Revision: 1.157 $
* @package MySource_Matrix
*/
class System_Config extends Config
{


	/**
	* Holds the definitions for all the constants managed by this configuration object
	* In the form : Array([var name] => Array('editable' => [boolean], 'default' => [mixed scalar]))
	* @var array
	*/
	public $config_vars = Array(
						'SQ_CONF_PEAR_PATH'						=> Array('editable' => 0, 'default' => ''),
						'SQ_CONF_BACKEND_SUFFIX'				=> Array('editable' => 1, 'default' => '_admin'),
						'SQ_CONF_LIMBO_SUFFIX'					=> Array('editable' => 1, 'default' => '_edit'),
						'SQ_CONF_LOGIN_SUFFIX'					=> Array('editable' => 1, 'default' => '_login'),
						'SQ_CONF_NOCACHE_SUFFIX'				=> Array('editable' => 1, 'default' => '_nocache'),
						'SQ_CONF_NOPROXYCACHE_SUFFIX'			=> Array('editable' => 1, 'default' => '_noproxycache'),
						'SQ_CONF_RECACHE_SUFFIX'				=> Array('editable' => 1, 'default' => '_recache'),
						'SQ_CONF_PERFORMANCE_SUFFIX'			=> Array('editable' => 1, 'default' => '_performance'),
						'SQ_CONF_PERFORMANCE_TIMING_SUFFIX'		=> Array('editable' => 1, 'default' => '_performance_timing'),
						'SQ_CONF_PERFORMANCE_RESULT_SUFFIX'		=> Array('editable' => 1, 'default' => '_performance_result'),
						'SQ_CONF_ASSET_TREE_BASE'				=> Array('editable' => 0, 'default' => 64),
						'SQ_CONF_ASSET_TREE_SIZE'				=> Array('editable' => 0, 'default' =>  4),
						'SQ_CONF_ASSET_CACHE_SIZE_WEB'			=> Array('editable' => 0, 'default' =>  -1),
						'SQ_CONF_ASSET_CACHE_SIZE_CLI'			=> Array('editable' => 0, 'default' =>  -1),
						'SQ_CONF_ROLLBACK_ENABLED'				=> Array('editable' => 1, 'default' => '0'),
						'SQ_CONF_DEBUG'							=> Array('editable' => 1, 'default' => 0),
						'SQ_CONF_SYSTEM_NAME'					=> Array('editable' => 1, 'default' => 'The Squiz Matrix System'),
						'SQ_CONF_SYSTEM_OWNER'					=> Array('editable' => 1, 'default' => ''),
						'SQ_CONF_SYSTEM_ROOT_URLS'				=> Array('editable' => 1, 'default' => ''),
						'SQ_CONF_SYSTEM_PARENT_DOMAINS'			=> Array('editable' => 1, 'default' => ''),
						'SQ_CONF_STATIC_ROOT_URL'				=> Array('editable' => 1, 'default' => ''),
						'SQ_CONF_WEB_PATH_SEPARATOR'			=> Array('editable'	=> 1, 'default'	=> '-'),
						'SQ_CONF_REDIRECT_URL_WITH_TRAILING_SLASH'	=> Array('editable' => 1, 'default' => '1'),
						'SQ_CONF_STATIC_ROOT_HTTP'				=> Array('editable' => 1, 'default' => TRUE),
						'SQ_CONF_STATIC_ROOT_HTTPS'				=> Array('editable' => 1, 'default' => FALSE),
						'SQ_CONF_DEFAULT_EMAIL'					=> Array('editable' => 1, 'default' => ''),
						'SQ_CONF_TECH_EMAIL'					=> Array('editable' => 1, 'default' => ''),
						'SQ_CONF_MAX_LOGIN_ATTEMPTS'			=> Array('editable' => 1, 'default' => 3),
						'SQ_CONF_REFRESH_INTERVAL'				=> Array('editable' => 1, 'default' => 120),
						'SQ_CONF_LOCK_LENGTH'					=> Array('editable' => 1, 'default' => 600),
						'SQ_CONF_ASSET_MAP_ASSET_LIMIT'			=> Array('editable' => 1, 'default' => 50),
						'SQ_CONF_ASSET_MAP_ASSET_DISPLAY_NAME'	=> Array('editable' => 1, 'default' => '%asset_short_name%  [%asset_assetid%]'),
						'SQ_CONF_SEND_CACHEABLE_HEADER'			=> Array('editable' => 1, 'default' => '1'),
						'SQ_CONF_SEND_LAST_MODIFIED_HEADER'		=> Array('editable' => 1, 'default' => '0'),
						'SQ_CONF_SEND_NOT_MODIFIED_HEADER'		=> Array('editable' => 1, 'default' => '0'),
						'SQ_CONF_SEND_NO_CACHE_HEADER'			=> Array('editable' => 1, 'default' => '1'),
						'SQ_CONF_SEND_404_CACHEABLE_HEADER'		=> Array('editable' => 1, 'default' => '1'),
						'SQ_CONF_X_UA_COMPATIBLE_HEADER'	=> Array('editable' => 1, 'default' => '1'),
						'SQ_CONF_USE_HTTP_X_FORWARDED_FOR'		=> Array('editable' => 1, 'default' => '0'),
						'SQ_CONF_FORWARDED_IP_PROXY_LIST'		=> Array('editable' => 1, 'default' => ''),
						'SQ_CONF_ERRORS_LOG_TO_SYSLOG'			=> Array('editable' => 1, 'default' => FALSE),
						'SQ_CONF_ERRORS_SYSLOG_FACILITY'		=> Array('editable' => 1, 'default' => 'user'),
						'SQ_CONF_ERRORS_HIDE_FRONTEND'			=> Array('editable' => 1, 'default' => FALSE),
						'SQ_CONF_DEFAULT_FRONTEND_LANGUAGE'		=> Array('editable' => 1, 'default' => 'en'),
						'SQ_CONF_DEFAULT_BACKEND_LOCALE'		=> Array('editable' => 1, 'default' => 'en_AU'),
						'SQ_CONF_DEFAULT_CHARACTER_SET'			=> Array('editable' => 0, 'default' => 'utf-8'),
						'SQ_CONF_USE_LANG_CHAR_MAP'				=> Array('editable' => 1, 'default' => FALSE),
						'SQ_CONF_ALLOW_IP_CHANGE'				=> Array('editable' => 1, 'default' => '0'),
						'SQ_CONF_ALLOW_HTTP_LOGIN'				=> Array('editable' => 1, 'default' => '0'),
						'SQ_CONF_ENABLE_HTTP_LOGIN'				=> Array('editable' => 1, 'default' => '0'),
						'SQ_CONF_ENABLE_EXTERNAL_AUTH_SYSTEMS'	=> Array('editable' => 1, 'default' => TRUE),
						'SQ_CONF_HTTP_LOGIN_VAR'				=> Array('editable' => 1, 'default' => 'REMOTE_USER'),
						'SQ_CONF_ACCEPT_HTTP_CREDS'				=> Array('editable' => 1, 'default' => '0'),
						'SQ_CONF_COMMIT_BUTTON_TEXT'			=> Array('editable' => 1, 'default' => 'Commit'),
						'SQ_CONF_CONFIRM_SAVE_CHANGES'			=> Array('editable' => 1, 'default' => '1'),
						'SQ_CONF_LOG_EXTENSION'					=> Array('editable' => 0, 'default' => '.log'),
						'SQ_CONF_LOG_FILE_SYSTEM'				=> Array('editable' => 0, 'default' => 'system'),
						'SQ_CONF_LOG_FILE_ERROR'				=> Array('editable' => 0, 'default' => 'error'),
						'SQ_CONF_LOG_ERRORS'					=> Array('editable' => 1, 'default' => TRUE),
						'SQ_CONF_LOCKING_METHOD'				=> Array('editable' => 0, 'default' => 'default'),
						'SQ_CONF_SESSION_HANDLER'				=> Array('editable' => 0, 'default' => ''),
						'SQ_CONF_SESSION_GC_MAXLIFETIME'		=> Array('editable' => 1, 'default'	=> 604800),
						'SQ_CONF_EDITING_TIME'					=> Array('editable' => 0, 'default' => 3600),
						'SQ_CONF_NUM_DATA_DIRS'					=> Array('editable' => 0, 'default' => 60),
						'SQ_CONF_FORCE_SECURE_LOGIN_URLS'		=> Array('editable' => 1, 'default' => ''),
						'SQ_CONF_WEB_MEMORY_LIMIT'				=> Array('editable' => 1, 'default' => 64),
						'SQ_CONF_CRON_MEMORY_LIMIT'				=> Array('editable' => 1, 'default' => 64),
						'SQ_CONF_USE_DEFAULT_SESSION_SAVE_PATH'	=> Array('editable' => 1, 'default' => FALSE),
						'SQ_CONF_CUSTOM_SESSION_SAVE_PATH'		=> Array('editable' => 1, 'default' => ''),
						'SQ_CONF_ENABLE_ROLES_PERM_SYSTEM'		=> Array('editable' => 1, 'default' => FALSE),
						'SQ_CONF_ENABLE_ROLES_WF_SYSTEM'		=> Array('editable' => 1, 'default' => FALSE),
						'SQ_CONF_ENABLE_GLOBAL_ROLES'			=> Array('editable' => 1, 'default' => FALSE),
						'SQ_CONF_NO_ASSET_SELF_LINKS'			=> Array('editable' => 1, 'default' => FALSE),
						'SQ_CONF_TIMEZONE'						=> Array('editable' => 1, 'default' => 'Australia/Sydney'),
						'SQ_CONF_FORCE_LOWERCASE_USERNAME'		=> Array('editable' => 0, 'default' => FALSE),
						'SQ_LICENCE_INSTALL_KEY'				=> Array('editable' => 1, 'default' => ''),
						'SQ_CONF_COOKIE_OPTION_HTTP_ONLY'		=> Array('editable' => 1, 'default' => TRUE),
						'SQ_CONF_COOKIE_OPTION_SECURE'			=> Array('editable' => 1, 'default' => FALSE),
						'SQ_CONF_ROOT_USER_DISABLED'			=> Array('editable' => 1, 'default' => '0'),
						'SQ_CONF_DISABLE_ATTRIBUTION'			=> Array('editable' => 1, 'default' => '0'),
						'SQ_CONF_ENFORCE_FRAME_SAMEORIGIN'		=> Array('editable' => 1, 'default' => '1'),
						'SQ_CONF_ENFORCE_FRAME_SAMEORIGIN_FRONTEND'	=> Array('editable' => 1, 'default' => '1'),
						'SQ_CONF_ENABLE_CSRF_TOKEN_REQUEST'		=> Array('editable' => 1, 'default' => '0'),
						'SQ_CONF_VISITED_PAGES_MAX_ENTRIES'		=> Array('editable' => 1, 'default' => '10'),
						'SQ_FILE_TYPE_RESTRICT_ON_STATIC_URL'	=> Array('editable' => 1, 'default' => ''),
						'SQ_CONFIG_STRIP_MATRIX_COMMENTS'		=> Array('editable' => 1, 'default' => TRUE),
						'SQ_CONF_FUNNELBACK_INDEXING_USER'      => Array('editable' => 1, 'default' => ''),
					   );

	/**
	* Holds all the valid debug settings and their descriptions for SQ_CONF_DEBUG
	* The keys are the bit elements and the values are the descriptions
	*
	* Due to being translated text, initialised in constructor.
	*
	* @var array
	*/
	private $debug_options = Array();


	/**
	* Constructor
	*
	* @return object
	* @access public
	*/
	function __construct()
	{
		parent::__construct();
		$this->config_file = SQ_DATA_PATH.'/private/conf/main.inc';
		$this->name = translate('System Configuration');
		$this->debug_options = Array(
			1	=> translate('Show File and Line number in error messages'),
			2	=> translate('Show Stack Trace in error messages'),
			4	=> translate('Show additional information about memory and performance'),
			8	=> translate('Show current frontend assetid and url where the error message is triggered from'),
		 );

	}//end constructor


	/**
	* Rewrites the conf file with current variables
	*
	* @param array		$vars				the array with any new values (that are allowed to be edited)
	*										Array('[config_var_name]' => [scalar value])
	* @param boolean	$backup_existing	whether we should backup the existing config file first
	* @param boolean	$send_message		whether to send a message using the messaging system
	*
	* @return boolean
	* @access public
	*/
	public function save($vars, $backup_existing=FALSE, $send_message=TRUE)
	{
		if (isset($GLOBALS['SQ_SYSTEM'])) {
			if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS)) {

				$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
				$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

				// if we dont have access, go away
				if (!$is_admin) {
					trigger_localised_error('SYS0267', sprintf(translate('You need to be either the Root User or a System Administrator to save the %s'), $this->name), E_USER_WARNING);
					return FALSE;
				}
			} else {
				$is_root = TRUE;
				$is_admin = TRUE;
			}//end if checking permissions

			if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_DATA_VALIDATION)) {

				// now just make sure they aren't trying to set anything that they shouldn't
				if (!$is_root) {
					unset($vars['SQ_CONF_MAX_LOGIN_ATTEMPTS']);
					unset($vars['SQ_CONF_LOG_ERRORS']);
					unset($vars['SQ_CONF_ERRORS_HIDE_FRONTEND']);
					unset($vars['SQ_CONF_ERRORS_LOG_TO_SYSLOG']);
					unset($vars['SQ_CONF_BACKEND_SUFFIX']);
					unset($vars['SQ_CONF_LIMBO_SUFFIX']);
					unset($vars['SQ_CONF_LOGIN_SUFFIX']);
					unset($vars['SQ_CONF_ROLLBACK_ENABLED']);
					unset($vars['SQ_CONF_WEB_MEMORY_LIMIT']);
					unset($vars['SQ_CONF_CRON_MEMORY_LIMIT']);
				}

				if (isset($vars['SQ_CONF_FORCE_SECURE_LOGIN_URLS'])) {
					if(empty($vars['SQ_CONF_FORCE_SECURE_LOGIN_URLS'])) {
						$vars['SQ_CONF_FORCE_SECURE_LOGIN_URLS'] = '';
					}
					else {
						$vars['SQ_CONF_FORCE_SECURE_LOGIN_URLS'] = implode("\n", $vars['SQ_CONF_FORCE_SECURE_LOGIN_URLS']);
					}
				}

				if (isset($vars['SQ_CONF_STATIC_ROOT_URL']) && substr($vars['SQ_CONF_STATIC_ROOT_URL'], -1) == '/') {
					$vars['SQ_CONF_STATIC_ROOT_URL'] = substr($vars['SQ_CONF_STATIC_ROOT_URL'], 0, strlen($vars['SQ_CONF_STATIC_ROOT_URL']) -1);
				}
				foreach (Array('SQ_CONF_REFRESH_INTERVAL', 'SQ_CONF_LOCK_LENGTH') as $var_name) {
					if (isset($vars[$var_name])) {
						$vars[$var_name] = max(60, (int) $vars[$var_name]);
					}
				}
			}//end if validating values

		}//end if SQ_SYSTEM is set

		return parent::save($vars, $backup_existing, $send_message);

	}//end save()


	/**
	* Paints out the code for the config file for the passed config
	* This allows extra things to be added for special cases
	*
	* @param string		$var_name	the config var name
	* @param mixed		$value		the value that the config var has
	* @param boolean	$var_export	whether to run the value through var_export or not
	*
	* @return boolean
	*/
	protected function _paintConfigVar($var_name, $value, $var_export=TRUE)
	{
		switch ($var_name) {
			case 'SQ_CONF_PEAR_PATH' :
				// override the default empty value to populate this real default path
				if(empty($value)) $value = SQ_SYSTEM_ROOT.'/php_includes';
				// if this looks like a plain value and it starts with the system root, then put the system root constant in
				if ($var_export && substr($value, 0, strlen(SQ_SYSTEM_ROOT)) == SQ_SYSTEM_ROOT) {
					if (!parent::_paintConfigVar($var_name, 'SQ_SYSTEM_ROOT.'.var_export(substr($value, strlen(SQ_SYSTEM_ROOT)), TRUE), FALSE)) {
						return FALSE;
					}
				} else {
					if (!parent::_paintConfigVar($var_name, $value, $var_export)) {
						return FALSE;
					}
				}
				if ($value) {
					$sep = (substr(PHP_OS, 0, 3) == 'WIN') ? ';' : ':';

					// basically, if the first entry is '.' (ie the current directory) leave that,
					// and insert the pear path just after that, otherwise just right at the front of the
					// include path
					echo "\$inc_dir = ini_get('include_path');\n";
					echo "\$inc_dir = (substr(\$inc_dir, 0, 2) == '.".$sep."') ? '.".$sep."'.SQ_CONF_PEAR_PATH.'".$sep."'.substr(\$inc_dir, 2) : SQ_CONF_PEAR_PATH.'".$sep."'.\$inc_dir;\n";
					echo "ini_set('include_path', \$inc_dir);\n\n";
				}
				return TRUE;
			break;


			case 'SQ_CONF_LOG_ERRORS' :
				if (!parent::_paintConfigVar($var_name, $value, $var_export)) {
					return FALSE;
				}
				// NOTE: if they don't want to log errors we will leave
				//       them with whatever their PHP settings are
				if ($value) echo "ini_set('log_errors', '1');\n";

				return TRUE;
			break;


			case 'SQ_CONF_LOG_FILE_ERROR' :
				$value = trim($value);
				// NOTE: we're not checking if the error file can be created or written to
				// this is because the full filename is dynamic, and on first run the value of
				// SQ_CONF_LOG_EXTENSION is unknown so we cannot create a proper name for testing
				// persmissions of SQ_LOG_PATH should make sure that dir can be written to

				// NOTE: if the value is given as "logname.log", the resulting logname will be
				// "logname.log.log" because users do not have control over file extension

				// the following call is used as a basic prevention mechanism to stop anyone
				// from specifying paths with directories
				$value = basename($value);
				if (!empty($value)) {
					$value = $this->config_vars[$var_name]['default'];
				}

				if (!parent::_paintConfigVar($var_name, $value, $var_export)) {
					return FALSE;
				}

				echo "ini_set('error_log',  SQ_LOG_PATH.'/'.SQ_CONF_LOG_FILE_ERROR.SQ_CONF_LOG_EXTENSION);\n";
				return TRUE;
			break;


			case 'SQ_CONF_SYSTEM_ROOT_URLS' :
			case 'SQ_CONF_STATIC_ROOT_URL' :
			case 'SQ_CONF_FORWARDED_IP_PROXY_LIST' :
				return parent::_paintConfigVar($var_name, trim($value), $var_export);
			break;

			case 'SQ_CONF_SESSION_GC_MAXLIFETIME' :
				if (!parent::_paintConfigVar($var_name, $value, $var_export)) {
					return FALSE;
				}
				echo "ini_set('session.gc_maxlifetime', $value);\n";
				return TRUE;
			break;

			default :
				return parent::_paintConfigVar($var_name, $value, $var_export);

		}//end switch

	}//end _paintConfigVar()


	/**
	* Whether the current user can acquire the lock
	*
	* @return boolean
	* @access public
	* @see MySource::acquireLock()
	*/
	public function canAcquireLock()
	{
		// need to be root or a sys admin
		return ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

	}//end canAcquireLock()


	/**
	* Does the current user have writeAccess() to this config ?
	*
	* @return boolean
	* @access public
	*/
	public function writeAccess()
	{
		if (!parent::writeAccess()) return FALSE;
		if (!isset($GLOBALS['SQ_SYSTEM'])) return TRUE;
		if (!$GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS)) {
			return TRUE;
		}

		// need to be root or a sys admin
		return ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

	}//end writeAccess()


	/**
	* Paints the interface for our config options.
	*
	* If you (as a sub-class) put your config vars so that when they are submitted appear in
	* $_POST[get_class($this)] then you probably won't need to override processBackend()
	*
	* @param object	&$o	reference to the backend outputter
	*
	* @return void
	* @access public
	* @see processBackend()
	*/
	public function paintBackend(&$o)
	{
		parent::paintBackend($o);

		$yes_no_options = Array(
		    '1' => translate('Yes'),
		    '0' => translate('No'),
		);

		$write_access = $this->writeAccess();

		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();

		$class = get_class_lower($this);

		$o->openSection(translate('System Settings'));


			$o->openField(translate('System Name'));

				if ($write_access) {
					text_box($class.'[SQ_CONF_SYSTEM_NAME]', SQ_CONF_SYSTEM_NAME, 30);
				} else {
					echo SQ_CONF_SYSTEM_NAME;
				}
			$o->closeField();

			$o->openField(translate('System Owner'));

			if ($write_access) {
				text_box($class.'[SQ_CONF_SYSTEM_OWNER]', SQ_CONF_SYSTEM_OWNER, 30);
			} else {
				echo SQ_CONF_SYSTEM_OWNER;
			}
			$o->note(translate('The System Name and System Owner appear by default in the footer of internal messages sent from the system by email. This behaviour can be changed by setting a custom message footer on the Messaging Service Config screen.'));
			$o->closeField();

			$o->openField(translate('Admin Mode Suffix'));

			if ($write_access) {
				text_box($class.'[SQ_CONF_BACKEND_SUFFIX]', SQ_CONF_BACKEND_SUFFIX, 15);
			} else {
				echo nl2br(SQ_CONF_BACKEND_SUFFIX);
			}

			$o->note(translate('If this path is added to a URL served by this system, Matrix will launch the Administration Mode Interface. Users of level "Backend User" or above can access the Admin Mode. A login page will be provided to users not logged in or without proper privilege.'));
			$o->closeField();

			$o->openField(translate('Edit Mode Suffix'));

			if ($write_access) {
				text_box($class.'[SQ_CONF_LIMBO_SUFFIX]', SQ_CONF_LIMBO_SUFFIX, 15);
			} else {
				echo nl2br(SQ_CONF_LIMBO_SUFFIX);
			}

			$o->note(translate('If this path is added to a URL served by this system, Matrix will launch the Edit Mode Interface. Users of level "Simple Edit User" or above can access the Edit Mode. A login page will be provided to users not logged in or without proper privilege.'));
			$o->closeField();

			$o->openField(translate('System Login Suffix'));

			if ($write_access) {
				text_box($class.'[SQ_CONF_LOGIN_SUFFIX]', SQ_CONF_LOGIN_SUFFIX, 15);
			} else {
				echo nl2br(SQ_CONF_LOGIN_SUFFIX);
			}

			$o->note(translate('If this path is added to a URL served by this system, Matrix will provide a login page, without triggering a backend mode. After logging in through this interface, Matrix will return the user to the previous frontend asset.'));
			$o->closeField();

			$o->openField(translate('System Bypass Cache Suffix'));

			if ($write_access) {
				text_box($class.'[SQ_CONF_NOCACHE_SUFFIX]', SQ_CONF_NOCACHE_SUFFIX, 15);
			} else {
				echo nl2br(SQ_CONF_NOCACHE_SUFFIX);
			}

			$o->note(translate('If this path is added to a URL served by this system, Matrix will disable its own caching systems entirely. This means Matrix will provide a fresh copy of an asset\'s content even if caching is enabled, and also not save the result in its caches.'));
			$o->closeField();

			$o->openField(translate('System Bypass Proxy Cache Suffix'));

			if ($write_access) {
				text_box($class.'[SQ_CONF_NOPROXYCACHE_SUFFIX]', SQ_CONF_NOPROXYCACHE_SUFFIX, 15);
			} else {
				echo nl2br(SQ_CONF_NOPROXYCACHE_SUFFIX);
			}

			$o->note(translate('If this path is added to a URL served by this system, Matrix will ignore any "Send Cacheable Headers" settings and will not send relevant headers (such as Cache-Control, Expires, and Last-Modified) for that request only.'));
			$o->closeField();
			
			$o->openField(translate('System Clear Cache Suffix'));

			if ($write_access) {
				text_box($class.'[SQ_CONF_RECACHE_SUFFIX]', SQ_CONF_RECACHE_SUFFIX, 15);
			} else {
				echo nl2br(SQ_CONF_RECACHE_SUFFIX);
			}

			$o->note(translate('If this path is added to a URL served by this system, Matrix will provide a fresh copy of an asset\'s content, similar to the Bypass Cache suffix, but also reprime Matrix\'s cache system with the resulting contents.'));
			$o->closeField();

			$o->openField(translate('System Performance Mode Suffix'));

			if ($write_access) {
				text_box($class.'[SQ_CONF_PERFORMANCE_SUFFIX]', SQ_CONF_PERFORMANCE_SUFFIX, 15);
			} else {
				echo nl2br(SQ_CONF_PERFORMANCE_SUFFIX);
			}

			$o->note(translate('If this path is added to a URL served by this system, Matrix will enter Performance Mode, allowing you to view a list of assets - including designs and nested assets - that are requested during the painting of the URL and their speed of performing certain actions (such as painting), allowing you to analyse bottlenecks in page generation. You must be a Backend User at least, and have write access to an asset in order to access Performance Mode.'));
			$o->closeField();

			$o->openField(translate('System Timezone'));

			if ($write_access) {
				text_box($class.'[SQ_CONF_TIMEZONE]', SQ_CONF_TIMEZONE, 15);
			} else {
				echo nl2br(SQ_CONF_TIMEZONE);
			}

			$o->note(translate('This sets the default PHP timezone for Matrix.'));
			$o->closeField();

			$o->openField(translate('Disable Matrix Attribution in Designs?'));

			if ($write_access) {
				combo_box($class.'[SQ_CONF_DISABLE_ATTRIBUTION]', $yes_no_options, FALSE, SQ_CONF_DISABLE_ATTRIBUTION);			
			} else {
				echo $yes_no_options[SQ_CONF_DISABLE_ATTRIBUTION];
			}

			$o->note(translate('If set to "Yes", parsed Design assets will not contain the "Running Squiz Matrix" attribution comment. After changing this setting, Design assets need to be regenerated for the change to take effect.'));
			$o->closeField();

			$o->openField(translate('Enforce Same Origin Frame Security For Backend?'));

			if ($write_access) {
				combo_box($class.'[SQ_CONF_ENFORCE_FRAME_SAMEORIGIN]', $yes_no_options, FALSE, SQ_CONF_ENFORCE_FRAME_SAMEORIGIN);
			} else {
				echo $yes_no_options[SQ_CONF_ENFORCE_FRAME_SAMEORIGIN];
			}

			$o->note(translate('If set to "Yes", Matrix will add the header "X-Frame-Options: SAMEORIGIN" to pages within Admin Mode and Edit Mode. This means that only pages hosted on the same domain as this Matrix system will be able to nest the Admin and Edit Mode interfaces using frames. Virtually all recent browsers support this header.'));
			$o->closeField();

			$o->openField(translate('Enforce Same Origin Frame Security For Frontend?'));

			if ($write_access) {
				combo_box($class.'[SQ_CONF_ENFORCE_FRAME_SAMEORIGIN_FRONTEND]', $yes_no_options, FALSE, SQ_CONF_ENFORCE_FRAME_SAMEORIGIN_FRONTEND);			
			} else {
				echo $yes_no_options[SQ_CONF_ENFORCE_FRAME_SAMEORIGIN_FRONTEND];
			}
			$o->note(translate('If set to "Yes", Matrix will add the header "X-Frame-Options: SAMEORIGIN" to frontend pages. This means that all frontend pages served by this Matrix system cannot be nested in a frame, unless the parent page is hosted on the same domain. Virtually all recent browsers support this header.'));
			$o->closeField();

		$o->closeSection();

		$o->openSection(translate('System URL Settings'));

			require_once SQ_ATTRIBUTES_PATH.'/option_list/option_list.inc';

			$option_list = new Asset_Attribute_Option_List(0, NULL, SQ_OPTION_LIST_DELIMITER_UNIX);
			$option_list->setEditParam('width', 60);

			$o->openField(translate('System Root URLs'));

				if ($write_access) {
					$option_list->value = SQ_CONF_SYSTEM_ROOT_URLS;
					$option_list->paint('SQ_CONF_SYSTEM_ROOT_URLS');
				} else {
					echo nl2br(SQ_CONF_SYSTEM_ROOT_URLS);
				}

			$o->note(translate('Root URLs must be added here before they (or any more specific URL) can be used by a Site asset. There is no limit to the number of System Root URLs that can be registered.'));
			$o->closeField();

			$o->openField(translate('System Parent Domains'));

				if ($write_access) {
					$option_list->value = SQ_CONF_SYSTEM_PARENT_DOMAINS;
					$option_list->paint('SQ_CONF_SYSTEM_PARENT_DOMAINS');
				} else {
					if (SQ_CONF_SYSTEM_PARENT_DOMAINS == '') {
						echo translate('None');

					} else {
						echo nl2br(SQ_CONF_SYSTEM_PARENT_DOMAINS);
					}
				}
				$o->note(translate('Parent domains are examined when the system is setting the session cookie, and are useful if several system root URLs have a common parent domain (eg syd.example.com and melb.example.com).  If the current URL ends with one of the parent domains then the cookie will be set on the parent domain instead, with the result that the user\'s session will persist across the parent domain and all its subdomains.  Note that the common parent must be only one level up - alpha.syd.example.com and bravo.mel.example.com cannot share a cookie because their common parent domain is two levels up.'));

			$o->closeField();

			$o->openField(translate('System Static URL'));

				if ($write_access && $is_root) {
					text_box($class.'[SQ_CONF_STATIC_ROOT_URL]', SQ_CONF_STATIC_ROOT_URL, 80);
				} else {
					if (SQ_CONF_STATIC_ROOT_URL == '') {
						echo translate('Not Set');

					} else {
						echo SQ_CONF_STATIC_ROOT_URL;
					}
				}
				$o->note(translate('Static URLs can be used to offload delivery of static files and images to an alternative web server. This is an advanced configuration option and requires specific web server configuration to operate correctly.'));

			$o->closeField();

			$o->openField(translate('Restricted File Extensions via Static Root Domain'));

				if ($write_access && $is_root) {
					text_box($class.'[SQ_FILE_TYPE_RESTRICT_ON_STATIC_URL]', SQ_FILE_TYPE_RESTRICT_ON_STATIC_URL, 80);
				} else {
					if (SQ_FILE_TYPE_RESTRICT_ON_STATIC_URL == '') {
						echo translate('Not Set');

					} else {
						echo SQ_FILE_TYPE_RESTRICT_ON_STATIC_URL;
					}
				}
				$o->note(translate('Type the restricted file extensions with comma separated. For example: jpg,png,txt,mov, etc. If configured, the extensions specified won\'t be served via the Static URL.'));

			$o->closeField();

			$o->openField(translate('Allow HTTP for Static URL?'));
				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_STATIC_ROOT_HTTP]', $yes_no_options, FALSE, SQ_CONF_STATIC_ROOT_HTTP);
				} else {
					echo $yes_no_options[SQ_CONF_STATIC_ROOT_HTTP];
				}
			$o->note(translate('If set to "No", Static URLs for live, publicly-readable files will be served through a Secure HTTP (HTTPS) protocol, even if the connection for the asset referencing them is insecure.'));
			$o->closeField();

			$o->openField(translate('Allow Secure HTTP (HTTPS) for Static URL?'));
				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_STATIC_ROOT_HTTPS]', $yes_no_options, FALSE, SQ_CONF_STATIC_ROOT_HTTPS);
				} else {
					echo $yes_no_options[SQ_CONF_STATIC_ROOT_HTTPS];
				}
			$o->note(translate('If set to "No", Static URLs for live, publicly-readable files will be served through the standard (insecure) HTTP protocol, even if the connection for the asset referencing them is secure.'));
			$o->note(translate('At least one of the two settings above must be set to "Yes". If both are set to "Yes", then Static URLs will be served using the same protocol as the referencing asset.'));
			$o->closeField();

			$o->openField(translate('System Web Path Separator'));

			if ($write_access) {
				text_box($class.'[SQ_CONF_WEB_PATH_SEPARATOR]', SQ_CONF_WEB_PATH_SEPARATOR, 1, 1);
			} else {
				echo nl2br(SQ_CONF_WEB_PATH_SEPARATOR);
			}
			$o->note(translate('This character replaces white space when creating valid web paths.'));
			$o->closeField();

			$o->openField(translate('Redirect URLs with Trailing Slash?'));

				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_REDIRECT_URL_WITH_TRAILING_SLASH]', $yes_no_options, FALSE, SQ_CONF_REDIRECT_URL_WITH_TRAILING_SLASH);
				} else {
					echo $yes_no_options[SQ_CONF_REDIRECT_URL_WITH_TRAILING_SLASH];
				}
			$o->note(translate('If set to "Yes", Matrix will automatically redirect frontend URL requests (excluding System Root URLs) to a URL with no trailing slashes.'));

			$o->closeField();

		$o->closeSection();

		$o->openSection(translate('Rollback Settings'));


			$o->openField(translate('Rollback Enabled?'));

				echo $yes_no_options[SQ_CONF_ROLLBACK_ENABLED];

				$o->note(translate('If set to "Yes", Matrix will maintain a history of changes to assets, and allow the viewing of the Matrix system at a particular point in time through an asset\'s History screen.'));
				$o->note(translate('This setting cannot be changed through Admin Mode.').' '.
					translate('Rollback must be enabled and disabled using the Rollback Management Tool, or the "rollback_management.php" script in the Matrix "scripts" directory'));

			$o->closeField();

		$o->closeSection();

		$o->openSection(translate('Email Settings'));


			$o->openField(translate('Default Email'));

				if ($write_access) {
					text_box($class.'[SQ_CONF_DEFAULT_EMAIL]', SQ_CONF_DEFAULT_EMAIL, 50);
				} else {
					echo SQ_CONF_DEFAULT_EMAIL;
				}
			$o->note('The Default Email is used as the sender address on emails from Matrix that are sent "from the system", or where an email address can\'t be sourced from elsewhere, such as a user without an email address sending a message. The Default Email also receives messages not directed at any particular user (along with the Tech Email address).');
			$o->closeField();

			$o->openField(translate('Tech Email'));

				if ($write_access) {
					text_box($class.'[SQ_CONF_TECH_EMAIL]', SQ_CONF_TECH_EMAIL, 50);
				} else {
					echo SQ_CONF_TECH_EMAIL;
				}
			$o->note('The Tech Email is included on internal message emails by default as a "support" email address (along with the System Owner name). This email also receives messages not directed at any particular user (along with the Default Email address), and also receives emails from Matrix about fatal errors.');
			$o->closeField();

		$o->closeSection();

		$o->openSection(translate('Login/Session Settings'));


			$o->openField(translate('Root URLs Requiring Secure Login'));

				$root_urls = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);
				$force_secure_urls = explode("\n", SQ_CONF_FORCE_SECURE_LOGIN_URLS);
				?>
				<table class="sq-backend-table" style="width: auto">
					<thead>
						<tr>
							<th><?php echo translate('URL'); ?></th>

							<th><?php echo translate('Require Secure Login'); ?></th>

						</tr>
					</thead>
					<tbody>
					<?php
					foreach ($root_urls as $i => $url) {
						?>
						<tr>
							<td><?php echo $url; ?></td>
							<td>
								<?php
								if ($write_access) {
									check_box($class.'[SQ_CONF_FORCE_SECURE_LOGIN_URLS][]', $url, in_array($url, $force_secure_urls));
								} else {
									$filename = in_array($url, $force_secure_urls) ? 'tick' : 'cross';
									echo '<img src="'.sq_web_path('lib').'/web/images/'.$filename.'.png" />';
								}
								?>
							</td>
						</tr>
						<?php
					}
					?>
					</tbody>
				</table>
				<?php
			$o->note(translate('Any Login or Password Change screen presented at a URL based on a selected root URL will always use secure (HTTPS) protocol, regardless of any other protocol settings.'));


			$o->openField(translate('Max Login Attempts'));

			if ($write_access && $is_root) {
				text_box($class.'[SQ_CONF_MAX_LOGIN_ATTEMPTS]', SQ_CONF_MAX_LOGIN_ATTEMPTS, 5, 5);
			} else {
				echo SQ_CONF_MAX_LOGIN_ATTEMPTS;
			}
			$o->note(translate('Sets the maximum number of login attempts allowed before a user account is locked from further login. Most users are locked by setting their asset status to Under Construction; reset the status to Live to unlock. Setting to 0 (zero) disables user account locking.'));

			$o->openField(translate('Allow IP Changes?'));
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_CONF_ALLOW_IP_CHANGE]', $yes_no_options, FALSE, SQ_CONF_ALLOW_IP_CHANGE);
			} else {
				echo $yes_no_options[SQ_CONF_ALLOW_IP_CHANGE];
			}
			$o->note(translate('If set to "No", a Matrix user will be logged out if their IP address changes during a session.'));
			
			$o->openField(translate('Process PHP Auth Credentials?'));
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_CONF_ACCEPT_HTTP_CREDS]', $yes_no_options, FALSE, SQ_CONF_ACCEPT_HTTP_CREDS);
			} else {
				echo $yes_no_options[SQ_CONF_ACCEPT_HTTP_CREDS];
			}
			$o->note(translate('If set to "Yes", Matrix will use the PHP_AUTH_USER and PHP_AUTH_PW server variables to attempt to authenticate a user. These variables are generated by PHP when a HTTP Authentication dialog box is invoked in a script.'));
			$o->closeField();

			$o->openField(translate('Enable HTTP Authentication?'));
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_CONF_ENABLE_HTTP_LOGIN]', $yes_no_options, FALSE, SQ_CONF_ENABLE_HTTP_LOGIN);
			} else {
				echo $yes_no_options[SQ_CONF_ENABLE_HTTP_LOGIN];
			}

			$o->note(translate('If set to "Yes", Matrix will respond to requests with the query string "USE_HTTP_LOGIN=1" with a HTTP Authentication request (and dialog box) instead of a login design, allowing external tools to login to Matrix.'));
			$o->note(
				sprintf(
					translate('This setting will only generate the HTTP Authentication request. To actually process the credentials and login, the "%s" setting must also be set to "Yes".'),
					translate('Process PHP Auth Credentials?')
				)
			);
			$o->closeField();

			$o->openField(translate('Accept HTTP Authentication?'));
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_CONF_ALLOW_HTTP_LOGIN]', $yes_no_options, FALSE, SQ_CONF_ALLOW_HTTP_LOGIN);
			} else {
				echo $yes_no_options[SQ_CONF_ALLOW_HTTP_LOGIN];
			}
			
			$o->openField(translate('HTTP Authentication Variable'));
			if ($write_access && $is_root) {
				$server = array_keys($_SERVER);
				?>

				<script language="JavaScript" type="text/javascript">
				//<![CDATA[
					function toggle_http_login_var() {
						list = document.getElementById('sq-conf-http-login-var-list');
						txt = document.getElementById('sq-conf-http-login-var-toggle');

						var txtValues = {
							'none':  js_translate('Show Server Variables'),
							'block': js_translate('Hide Server Variables')
						}

						if (txt && list) {
							list.style.display = (list.style.display == 'none' ? 'block' : 'none');
							txt.innerHTML = txtValues[list.style.display];
						}
					}// end set_conf_debug
				//]]>
				</script>

				<?php
				text_box($class.'[SQ_CONF_HTTP_LOGIN_VAR]', SQ_CONF_HTTP_LOGIN_VAR, 40, '', FALSE , 'id="sq-conf-http-login-var"');
				?>

				<br />
				<span id="sq-conf-http-login-var-toggle" class="clickable" onclick=toggle_http_login_var();><?php echo translate('Show Server Variables');?></span>

				<div id="sq-conf-http-login-var-list" style="display:none;">
					<table class="sq-backend-table" width="400">
						<tr>
							<th colspan="2">
							<?php echo translate('Server Variables');?>

							</th>
						</tr>
						<tr>
							<td>
								<?php
									if (!empty($server)) {
										?><ul><?php
											for ($i=0; $i < sizeof($server); $i++) {
												// make the second column
												if ($i == round(sizeof($server)/2)) {
													echo '</ul></td><td ><ul>';
												}
												?><li>
													<span class="clickable" onclick="elt=document.getElementById('sq-conf-http-login-var'); if(elt) elt.value='<?php echo $server[$i];?>';">
														<?php echo $server[$i];?>
													</span>
												</li><?php
											}
										?></ul><?php
									}
								?>
							</td>
						</tr>
					</table>
				</div>
				<?php
			} else {
				echo SQ_CONF_HTTP_LOGIN_VAR;
			}

			$o->note(translate('If "Accept HTTP Authentication?" is set to "Yes", Matrix will attempt to automatically login a user with a username matching the server variable in "HTTP Authentication Variable".'));
			$o->note(translate('This allows Matrix to accept users through certain types of single sign-on systems, but is a potential security risk as it will assume that the user has already been authenticated.'));

			$o->openField(translate('Enable External Authentication Systems?'));
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_CONF_ENABLE_EXTERNAL_AUTH_SYSTEMS]', $yes_no_options, FALSE, SQ_CONF_ENABLE_EXTERNAL_AUTH_SYSTEMS);
			} else {
				echo $yes_no_options[SQ_CONF_ENABLE_EXTERNAL_AUTH_SYSTEMS];
			}
			$o->note(translate('If set to "No", only the Default Authentication system will be enabled for logging in users. This means that types of users that depend on another Authentication system - such as LDAP Users - will not be able to log in until this setting is reset to "Yes".'));


			$o->openField(translate('Use Default PHP Session File Save Path?'));

			$options = Array(
                '1' => translate('Yes'),
                '0' => translate('No'),
            );
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_CONF_USE_DEFAULT_SESSION_SAVE_PATH]', $options, FALSE, SQ_CONF_USE_DEFAULT_SESSION_SAVE_PATH);
			} else {
				echo $options[SQ_CONF_USE_DEFAULT_SESSION_SAVE_PATH];
			}
			$o->note(translate('If set to "Yes", Matrix will use the default PHP session file path instead of the Matrix "cache" directory. This may be required for system with multiple servers using shared file storage. However, this will prevent Site Network assets from operating correctly.'));
			$o->note('<strong>'.translate('This option should only be changed by advanced users. If you change this option, all users will be immediately logged out of their session.').'</strong>');
			$o->note(sprintf(translate('The current session file save path is: <strong>%s</strong>'), session_save_path()));
		$o->closeSection();

		$o->openSection(translate('Intervals'));
			require_once SQ_FUDGE_PATH.'/general/datetime.inc';

			$o->openField(translate('Header Frame Refresh Interval').' ('.translate('seconds').')');

			if ($write_access) {
				text_box($class.'[SQ_CONF_REFRESH_INTERVAL]', SQ_CONF_REFRESH_INTERVAL, 5, 5);
			} else {
				echo SQ_CONF_REFRESH_INTERVAL.' <em>('.easy_time_total(SQ_CONF_REFRESH_INTERVAL, TRUE).')</em>';
			}
			$o->note(translate('The length of time between refreshes of the header frame. Each refresh checks for new messages, and assists with long edits by renewing the current screen\'s locks. Minimum value: 60 seconds (one minute). To benefit from lock refreshing, this value should be less than the Lock Length.'));

			$o->openField(translate('Lock Length').' ('.translate('seconds').')');

			if ($write_access) {
				text_box($class.'[SQ_CONF_LOCK_LENGTH]', SQ_CONF_LOCK_LENGTH, 5, 5);
			} else {
				echo SQ_CONF_LOCK_LENGTH.' <em>('.easy_time_total(SQ_CONF_LOCK_LENGTH, TRUE).')</em>';
			}
			$o->note(translate('The length of time after which a lock is considered to have expired. Minimum value: 60 seconds (one minute).'));
			$o->closeField();


		$o->closeSection();

		$o->openSection(translate('PHP Configuration'));


			$o->openField(translate('Web Memory Limit'));

			if ($write_access && $is_root) {
				int_text_box($class.'[SQ_CONF_WEB_MEMORY_LIMIT]', SQ_CONF_WEB_MEMORY_LIMIT, FALSE, 5, NULL, NULL, 3);
				echo ' MB';
			} else {
				if ((int) SQ_CONF_WEB_MEMORY_LIMIT === -1) {
					echo translate('No limit');
				} else {
					echo SQ_CONF_WEB_MEMORY_LIMIT.'MB';
				}
			}
			$o->note(translate('The maximum amount of system memory allocated to each instance of Matrix web system.'));

			$o->closeField();

			$o->openField(translate('Cron Memory Limit'));

			if ($write_access && $is_root) {
				int_text_box($class.'[SQ_CONF_CRON_MEMORY_LIMIT]', SQ_CONF_CRON_MEMORY_LIMIT, FALSE, 5, NULL, NULL, 3);
				echo ' MB';
			} else {
				if ((int) SQ_CONF_CRON_MEMORY_LIMIT === -1) {
					echo translate('No limit');
				} else {
					echo SQ_CONF_CRON_MEMORY_LIMIT.'MB';
				}
			}
			$o->note(translate('The maximum amount of system memory allocated to each instance of the Matrix cron system.'));

			$o->closeField();

		$o->closeSection();


		$o->openSection(translate('Error/Debug Settings'));
			$o->openField(translate('Hide Errors on the Frontend?'));
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_CONF_ERRORS_HIDE_FRONTEND]', $yes_no_options, FALSE, SQ_CONF_ERRORS_HIDE_FRONTEND);
			} else {
				echo $yes_no_options[SQ_CONF_ERRORS_HIDE_FRONTEND];
			}
			$o->note(translate('If set to "Yes", errors produced by Matrix or PHP will not be shown on to frontend users. This is strongly recommended for production systems. Errors will still be logged to the appropriate error log, as controlled by the "log_errors" PHP directive.'));
			$o->closeField();

			$o->openField(translate('Log Errors to Syslog?'));

			if ($write_access && $is_root) {
				combo_box($class.'[SQ_CONF_ERRORS_LOG_TO_SYSLOG]', $yes_no_options, FALSE, SQ_CONF_ERRORS_LOG_TO_SYSLOG);
			} else {
				echo $yes_no_options[SQ_CONF_ERRORS_LOG_TO_SYSLOG];
			}
			$o->closeField();

			$o->openField(translate('Syslog Facility'));

			// We only provide the common
			$options = Array(
						'user'		=> translate('User'),
						'local0'	=> sprintf(translate('Local %s'), 0),
						'local1'	=> sprintf(translate('Local %s'), 1),
						'local2'	=> sprintf(translate('Local %s'), 2),
						'local3'	=> sprintf(translate('Local %s'), 3),
						'local4'	=> sprintf(translate('Local %s'), 4),
						'local5'	=> sprintf(translate('Local %s'), 5),
						'local6'	=> sprintf(translate('Local %s'), 6),
						'local7'	=> sprintf(translate('Local %s'), 7),
					   );
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_CONF_ERRORS_SYSLOG_FACILITY]', $options, FALSE, SQ_CONF_ERRORS_SYSLOG_FACILITY);
			} else {
				echo $options[SQ_CONF_ERRORS_SYSLOG_FACILITY];
			}

			$o->note(translate('If "Log Errors to Syslog?" to "Yes", Matrix and PHP errors will be sent to the "syslog" system logger. "Syslog Facility" defines the particular log facility that these messages should be sent to. Please contact your system administrator for appropriate settings if you are to use this feature.'));
			$o->closeField();

			$o->openField(translate('Debug Settings'));

			if ($write_access && $is_root) {
				?>
				<script language="JavaScript" type="text/javascript">
				<!--
					function set_conf_debug(adding, bit) {

						var current_debug = parseInt(get_form_element_value("<?php echo $class; ?>[SQ_CONF_DEBUG]"), 10);
						if (isNaN(current_debug)) return;
						// are we adding this bit to the list, or removing it ?
						if (adding) {
							current_debug |= bit;
						} else {
							current_debug &= ~bit;
						}

						set_hidden_field("<?php echo $class; ?>[SQ_CONF_DEBUG]", current_debug);

					}// end set_conf_debug

				//-->
				</script>
				<?php
				hidden_field($class.'[SQ_CONF_DEBUG]', SQ_CONF_DEBUG);
				foreach ($this->debug_options as $bit => $desc) {
					check_box($class.'_SQ_CONF_DEBUG_'.$bit, $bit, (bool)(SQ_CONF_DEBUG & $bit), 'set_conf_debug(this.checked, '.$bit.');');
					label($desc, $class.'_SQ_CONF_DEBUG_'.$bit);
					echo '<br />';
				}
			} else {
				foreach ($this->debug_options as $bit => $desc) {
					?>
					<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo (SQ_CONF_DEBUG & $bit) ? 'tick' : 'cross'; ?>.png" width="15" height="15" />
					<?php
					echo $desc, "<br />\n";
				}

			}

		$o->closeSection();

		$o->openSection(translate('Internationalisation Settings'));


			$o->openField(translate('Default Frontend Language'));

			require SQ_FUDGE_PATH.'/standards_lists/languages.inc';
			if ($write_access) {
				combo_box($class.'[SQ_CONF_DEFAULT_FRONTEND_LANGUAGE]', $standards_lists_languages, FALSE, SQ_CONF_DEFAULT_FRONTEND_LANGUAGE);
			} else {
				echo $standards_lists_languages[SQ_CONF_DEFAULT_FRONTEND_LANGUAGE];
			}

			$o->openField(translate('Default Backend Locale'));

			require SQ_FUDGE_PATH.'/standards_lists/locales.inc';
			// default to English (Australia) in upgraded systems
			$current_locale = !defined('SQ_CONF_DEFAULT_BACKEND_LOCALE') ? 'en_AU' : SQ_CONF_DEFAULT_BACKEND_LOCALE;
			if ($write_access) {
				combo_box($class.'[SQ_CONF_DEFAULT_BACKEND_LOCALE]', $standards_lists_locales, FALSE, $current_locale);
			} else {
				echo $standards_lists_locales[$current_locale];
			}

			$o->openField(translate('Default Character Set'));

			require SQ_FUDGE_PATH.'/standards_lists/character_sets.inc';
			echo $standards_lists_character_sets[SQ_CONF_DEFAULT_CHARACTER_SET];

			$o->openField(translate('Replace Accented Characters in Web Paths?'));

			if ($write_access) {
				combo_box($class.'[SQ_CONF_USE_LANG_CHAR_MAP]', $yes_no_options, FALSE, !defined('SQ_CONF_USE_LANG_CHAR_MAP') ? '0' : SQ_CONF_USE_LANG_CHAR_MAP);
			} else {
				echo $yes_no_options[SQ_CONF_USE_LANG_CHAR_MAP];
			}

			$o->note('If set to "Yes", a character map will be used for translating accented characters in web paths so it can be made valid. The character map used is determined by the default frontend language.');

		$o->closeSection();

		$o->openSection(translate('Editing Interface Settings'));


			$o->openField(translate('Commit Button Text'));

			if ($write_access) {
				text_box($class.'[SQ_CONF_COMMIT_BUTTON_TEXT]', !defined('SQ_CONF_COMMIT_BUTTON_TEXT') ? 'Commit' : SQ_CONF_COMMIT_BUTTON_TEXT, 30);
			} else {
				echo !defined('SQ_CONF_COMMIT_BUTTON_TEXT') ? 'Commit' : SQ_CONF_COMMIT_BUTTON_TEXT;
			}
			$o->note(translate('This setting renames the button used to commit edited information into the Matrix system - default is \'Commit\'.'));

			$o->closeField();

			$o->openField(translate('Confirm Save Changes'));
			if ($write_access) {
				combo_box($class.'[SQ_CONF_CONFIRM_SAVE_CHANGES]', $yes_no_options, FALSE, !defined('SQ_CONF_CONFIRM_SAVE_CHANGES') ? '1' : SQ_CONF_CONFIRM_SAVE_CHANGES);
			} else {
			    echo $yes_no_options[SQ_CONF_CONFIRM_SAVE_CHANGES];
			}//end if
			$o->note(translate('If set to "Yes", users will be warned when changes are made but not committed.'));

			$o->closeField();

		$o->closeSection();

		$o->openSection(translate('Asset Map Settings'));


			$o->openField(translate('Asset Limit Per Set'));

			if ($write_access) {
				text_box($class.'[SQ_CONF_ASSET_MAP_ASSET_LIMIT]', !defined('SQ_CONF_ASSET_MAP_ASSET_LIMIT') ? '50' : SQ_CONF_ASSET_MAP_ASSET_LIMIT, 30);
			} else {
				echo !defined('SQ_CONF_ASSET_MAP_ASSET_LIMIT') ? '50' : SQ_CONF_ASSET_MAP_ASSET_LIMIT;
			}
			$o->note(translate('This setting limits the number of child assets displayed under an asset. You will be able to navigate between sets by using next and previous buttons.'));

			$o->closeField();

			$o->openField(translate('Asset Display Name'));

			if ($write_access) {
				text_box($class.'[SQ_CONF_ASSET_MAP_ASSET_DISPLAY_NAME]', !defined('SQ_CONF_ASSET_MAP_ASSET_DISPLAY_NAME') ?  $this->config_vars['SQ_CONF_ASSET_MAP_ASSET_DISPLAY_NAME']['default']  : SQ_CONF_ASSET_MAP_ASSET_DISPLAY_NAME, 50);
			} else {
				echo !defined('SQ_CONF_ASSET_MAP_ASSET_DISPLAY_NAME') ? $this->config_vars['SQ_CONF_ASSET_MAP_ASSET_DISPLAY_NAME']['default'] : SQ_CONF_ASSET_MAP_ASSET_DISPLAY_NAME;
			}
			$o->note(translate('Keywords available for use in the asset display name are &#37;asset_assetid&#37;, &#37;asset_name&#37; &#37;asset_short_name&#37; &#37;asset_type_code&#37; &#37;asset_status&#37; and &#37;asset_num_kids&#37;. Other characters may also be used for formatting, eg [&#37;asset_assetid&#37;] - &#37;asset_short_name&#37;'));

			$o->closeField();

		$o->closeSection();

		$o->openSection(translate('HTTP Headers Settings'));


			$o->openField(translate('Send Cacheable Headers?'));

				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_SEND_CACHEABLE_HEADER]', $yes_no_options, FALSE, SQ_CONF_SEND_CACHEABLE_HEADER);
				} else {
					echo $yes_no_options[SQ_CONF_SEND_CACHEABLE_HEADER];
				}

				$o->note(translate('If set to "Yes", Matrix will send cacheable Cache-Control and Pragma headers when an asset is painted.'));

			$o->closeField();

			$o->openField(translate('Send Last-Modified Header?'));

				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_SEND_LAST_MODIFIED_HEADER]', $yes_no_options, FALSE, SQ_CONF_SEND_LAST_MODIFIED_HEADER);
				} else {
					echo $yes_no_options[SQ_CONF_SEND_LAST_MODIFIED_HEADER];
				}

				$o->note(translate('If set to "Yes", Matrix will send a "Last-Modified" header along with responses, which indicates when a page\'s content was last modified. Browser and proxy server caches can use this header to determine the age of the content, and minimise unnecessary future hits on the Matrix system.'));

			$o->closeField();

			$o->openField(translate('Send Not Modified Status Code?'));
				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_SEND_NOT_MODIFIED_HEADER]', $yes_no_options, FALSE, SQ_CONF_SEND_NOT_MODIFIED_HEADER);
				} else {
					echo $yes_no_options[SQ_CONF_SEND_NOT_MODIFIED_HEADER];
				}
				$o->note(translate('If set to "Yes", Matrix will test "If-Modified-Since" headers sent in requests to live, publicly readable assets. If the current asset\'s content is older than the passed "If-Modified-Since" date, Matrix will send a "304 Not Modified" status code instead of the actual content.'));
				$o->note(translate('This setting is recommended for heavily trafficked sites, as this can reduce the number of requests that reach Matrix (instead of using a cache), especially if your content does not change often.'));
			$o->closeField();

			$o->openField(translate('Send No-Cache Header for File assets?'));
				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_SEND_NO_CACHE_HEADER]', $yes_no_options, FALSE, SQ_CONF_SEND_NO_CACHE_HEADER);
				} else {
					echo $yes_no_options[SQ_CONF_SEND_NO_CACHE_HEADER];
				}
				$o->note(translate('If set to "Yes", the "no-cache" Cache-Control option will be sent when serving File asset types. This option can be used to resolve inline file display issues in Internet Explorer.'));
			$o->closeField();

			$o->openField(translate('Send Cacheable Headers for 404 Pages?'));
				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_SEND_404_CACHEABLE_HEADER]', $yes_no_options, FALSE, SQ_CONF_SEND_404_CACHEABLE_HEADER);
				} else {
					echo $yes_no_options[SQ_CONF_SEND_404_CACHEABLE_HEADER];
				}
				$o->note(translate('If set to "Yes", Matrix will send cacheable Cache-Control and Pragma headers for 404 (Not Found) pages.'));

			$o->closeField();

			$o->openField(translate('Use "X-Forwarded-For" Header?'));
				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_USE_HTTP_X_FORWARDED_FOR]', $yes_no_options, FALSE, SQ_CONF_USE_HTTP_X_FORWARDED_FOR);

					echo '<br/><br/><strong>'.translate('Only for these trusted proxies:').'</strong>';
					$option_list->value = SQ_CONF_FORWARDED_IP_PROXY_LIST;
					$option_list->paint('SQ_CONF_FORWARDED_IP_PROXY_LIST');
				} else {
					echo $yes_no_options[SQ_CONF_USE_HTTP_X_FORWARDED_FOR];
					if (SQ_CONF_USE_HTTP_X_FORWARDED_FOR === '1') {
						echo '<br/><br/><strong>'.translate('Only for these trusted proxies:').'</strong>';
                        if (SQ_CONF_FORWARDED_IP_PROXY_LIST == '') {
                            echo ' <span class="sq-backend-warning">'.translate('No trusted proxies set').'</span>';
                        } else {
                            echo '<ul><li>'.implode('</li><li>', explode("\n",SQ_CONF_FORWARDED_IP_PROXY_LIST)).'</li></ul>';
                        }
					}
				}

				$o->note(translate('Only enable to help obtain User IP when behind a proxy. System will attempt to use "X-Forwarded-For" header. By default the REMOTE_ADDR server variable is used. This will only work in conjunction with the above trusted/known Proxy IP list.'));
			$o->closeField();

			$o->openField(translate('Set \'HttpOnly\' flag for Session Cookies?'));
                if ($write_access && $is_root) {
                    combo_box($class.'[SQ_CONF_COOKIE_OPTION_HTTP_ONLY]', $yes_no_options, FALSE, SQ_CONF_COOKIE_OPTION_HTTP_ONLY);
                } else {
                    echo $yes_no_options[SQ_CONF_COOKIE_OPTION_HTTP_ONLY];
                }
				$o->note(translate('If set to "Yes", Matrix will set the \'HttpOnly\' flag on its session cookie. When a cookie is HttpOnly the web browser should not allow client side scripts such as JavaScript to have access to the cookie. This can help mitigate the effects of cross site scripting attacks.'));


			$o->closeField();

			$o->openField(translate('Set \'Secure\' flag for Session Cookies?'));
				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_COOKIE_OPTION_SECURE]', $yes_no_options, FALSE, SQ_CONF_COOKIE_OPTION_SECURE);
				} else {
					echo $yes_no_options[SQ_CONF_COOKIE_OPTION_SECURE];
				}
				$o->note(translate('If set to "Yes", Matrix will set the \'Secure\' flag on its session cookie when a connection is made over HTTPS. This flag signals browsers to not share the session cookie between HTTP and HTTPS.'));


			$o->closeField();

			$o->openField(translate('Send IE "X-UA-Compatible" header?'));
				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_X_UA_COMPATIBLE_HEADER]', $yes_no_options, FALSE, SQ_CONF_X_UA_COMPATIBLE_HEADER);
				} else {
					echo $yes_no_options[SQ_CONF_X_UA_COMPATIBLE_HEADER];
				}
				$o->note(translate('If set to "Yes", Matrix will send this header for all requests. Matrix\'s settings for the header request Internet Explorer 8-10 use its latest engine to render the page, even if it may otherwise be viewed in "Compatibility View". If the browser has the (now-retired) Chrome Frame installed, it will use that instead.'));

			$o->closeField();


		$o->closeSection();


		$o->openSection(translate('Roles Configuration'));


			$o->openField(translate('Enable Permission Roles System?'));

				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_ENABLE_ROLES_PERM_SYSTEM]', $yes_no_options, FALSE, SQ_CONF_ENABLE_ROLES_PERM_SYSTEM);


				} else {
					echo $yes_no_options[SQ_CONF_ENABLE_ROLES_PERM_SYSTEM];


				}
			$o->note(translate('If set to "Yes", the roles system will be enabled when working with the permissions system. If this is not required, setting to "No" may improve performance.'));
			$o->closeField();

			$o->openField(translate('Enable Workflow Roles System?'));

				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_ENABLE_ROLES_WF_SYSTEM]', $yes_no_options, FALSE, SQ_CONF_ENABLE_ROLES_WF_SYSTEM);


				} else {
					echo $yes_no_options[SQ_CONF_ENABLE_ROLES_WF_SYSTEM];


				}

			$o->note(translate('If set to "Yes", the roles system will be enabled when working with the workflow system.'));
			$o->closeField();

			if ( ((defined('SQ_CONF_ENABLE_ROLES_PERM_SYSTEM') && (SQ_CONF_ENABLE_ROLES_PERM_SYSTEM == '1')) || (defined('SQ_CONF_ENABLE_ROLES_WF_SYSTEM') && (SQ_CONF_ENABLE_ROLES_WF_SYSTEM == '1')) ) || (!defined('SQ_CONF_ENABLE_ROLES_PERM_SYSTEM') && !defined('SQ_CONF_ENABLE_ROLES_WF_SYSTEM') )) {
				$o->openField(translate('Enable Global Role Assignments?'));

					if ($write_access && $is_root) {
						combo_box($class.'[SQ_CONF_ENABLE_GLOBAL_ROLES]', $yes_no_options, FALSE, SQ_CONF_ENABLE_GLOBAL_ROLES);
					} else {
						echo $yes_no_options[SQ_CONF_ENABLE_GLOBAL_ROLES];
					}
				$o->note(translate('If set to "Yes", the roles systems will take into account "global" role assignments, created by linking users beneath Role assets. If set to "No", only roles assigned at an asset level (through the Roles screen) will take effect.'));
				$o->closeField();
			}

		$o->closeSection();

		$o->openSection(translate('Search Engine Optimizations'));


			$o->openField(translate('Remove Self Links from Content?'));

				if ($write_access) {
					combo_box($class.'[SQ_CONF_NO_ASSET_SELF_LINKS]', $yes_no_options, FALSE, !defined('SQ_CONF_NO_ASSET_SELF_LINKS') ? '0' : SQ_CONF_NO_ASSET_SELF_LINKS);
				} else {
					echo $yes_no_options[SQ_CONF_NO_ASSET_SELF_LINKS];
				}
				$o->note(translate('If set to "Yes", an asset\'s content will be stripped of links pointing to any of its own URLs when painted on the frontend.'));

			$o->closeField();

		$o->closeSection();

		$o->openSection(translate('Miscellaneous Settings'));

			$o->openField(translate('Visited Pages Maximum Entries'));

			if ($write_access && $is_root) {
				text_box($class.'[SQ_CONF_VISITED_PAGES_MAX_ENTRIES]', !defined('SQ_CONF_VISITED_PAGES_MAX_ENTRIES') ? '10' : SQ_CONF_VISITED_PAGES_MAX_ENTRIES, 30);
			} else {
				echo !defined('SQ_CONF_VISITED_PAGES_MAX_ENTRIES') ? '10' : SQ_CONF_VISITED_PAGES_MAX_ENTRIES;
			}
				$o->note(translate('Maximum number of visited URLs entires to store in the session. Visited URLs and asset IDs can be accessed via global session var "visited_urls" and "visited_assets" respectively.'));

			$o->closeField();

			$o->openField(translate('Strip Matrix Comments from Frontend?'));

				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONFIG_STRIP_MATRIX_COMMENTS]', $yes_no_options, FALSE, SQ_CONFIG_STRIP_MATRIX_COMMENTS);
				} else {
					echo $yes_no_options[SQ_CONFIG_STRIP_MATRIX_COMMENTS];
				}
				$o->note(htmlspecialchars(translate('If set to "Yes", the Design parsing process will strip specially-marked comments (wrapped in "<!--@@ @@-->" for Design parse files,  and "/*@@ @@*/" for CSS Design files) from the final output. This allows you to add comments to your designs that are not shown to frontend users viewing the page source.')));

			$o->closeField();

		$o->closeSection();

		$o->openSection(translate('Install Key'));


			$o->openField(translate('Install Key'));

				echo SQ_LICENCE_INSTALL_KEY;
				$o->note(translate('The install key is automatically generated when the <em>step_03.php</em> script is run to reconfigure your system - it cannot be changed here.'));

			$o->closeField();
		$o->closeSection();

        $o->openSection(translate('Funnelback'));
	            $o->openField(translate('Funnelback Indexing User'));
	                    if ($write_access) {
	                    text_box($class.'[SQ_CONF_FUNNELBACK_INDEXING_USER]', !defined('SQ_CONF_FUNNELBACK_INDEXING_USER') ? '' : SQ_CONF_FUNNELBACK_INDEXING_USER, 30);
	                    } else {
	                            echo !defined('SQ_CONF_FUNNELBACK_INDEXING_USER') ? '' : SQ_CONF_FUNNELBACK_INDEXING_USER;
	                    }
	                    $o->note(translate('Specify one or multiple Matrix User IDs(comma-separated) to be allowed for Funnelback Indexing. Funnelback Crawler will have to login as one of those users to receive "X-Funnelback" headers for extra page metadata.'));

	            $o->closeField();
	    $o->closeSection();

		if ($write_access) $o->commitButton('', FALSE);

	}//end paintBackend()


	/**
	* Saves the config settings that were submitted
	*
	* @param object	&$o	reference to the backend outputter
	*
	* @return boolean
	* @access public
	* @see paintBackend()
	*/
	public function processBackend(&$o)
	{
		$write_access = $this->writeAccess();
		if (!$write_access) return parent::processBackend($o);

		$class = get_class_lower($this);

		// Value restriction for Visited Pages Maximum Entries setting
		if(isset($_POST[$class]['SQ_CONF_VISITED_PAGES_MAX_ENTRIES'])) {
		    if ($_POST[$class]['SQ_CONF_VISITED_PAGES_MAX_ENTRIES'] < 0) {
			    $_POST[$class]['SQ_CONF_VISITED_PAGES_MAX_ENTRIES'] = 0;
		    } else if ($_POST[$class]['SQ_CONF_VISITED_PAGES_MAX_ENTRIES'] > 1000) {
			    $_POST[$class]['SQ_CONF_VISITED_PAGES_MAX_ENTRIES'] = 1000;
		    }
		}

		require_once SQ_ATTRIBUTES_PATH.'/option_list/option_list.inc';
		$option_list = new Asset_Attribute_Option_List(0, NULL, SQ_OPTION_LIST_DELIMITER_UNIX);

		$list_settings = Array(
							'SQ_CONF_SYSTEM_ROOT_URLS',
							'SQ_CONF_SYSTEM_PARENT_DOMAINS',
							'SQ_CONF_FORWARDED_IP_PROXY_LIST'
						 );

		foreach ($list_settings as $list_setting) {
			$option_list->process($list_setting, TRUE, TRUE);
			$_POST[$class][$list_setting] = $option_list->value;
		}

		$roles_changed = (isset($_POST['system_config']['SQ_CONF_ENABLE_ROLES_PERM_SYSTEM']) || isset($_POST['system_config']['SQ_CONF_ENABLE_ROLES_WF_SYSTEM']) || isset($_POST['system_config']['SQ_CONF_ENABLE_GLOBAL_ROLES']));

		$enable_roles_perm_system_setting = SQ_CONF_ENABLE_ROLES_PERM_SYSTEM;
		$enable_roles_wf_system_setting = SQ_CONF_ENABLE_ROLES_WF_SYSTEM;
		$enable_global_roles_setting = SQ_CONF_ENABLE_GLOBAL_ROLES;

		if ($roles_changed) {
			$enable_roles_perm_system_setting = (isset($_POST['system_config']['SQ_CONF_ENABLE_ROLES_PERM_SYSTEM'])) ? $_POST['system_config']['SQ_CONF_ENABLE_ROLES_PERM_SYSTEM'] : $enable_roles_perm_system_setting;
			$enable_roles_wf_system_setting = (isset($_POST['system_config']['SQ_CONF_ENABLE_ROLES_WF_SYSTEM'])) ? $_POST['system_config']['SQ_CONF_ENABLE_ROLES_WF_SYSTEM'] : $enable_roles_wf_system_setting;

			$enable_global_roles_setting = (isset($_POST['system_config']['SQ_CONF_ENABLE_GLOBAL_ROLES'])) ? $_POST['system_config']['SQ_CONF_ENABLE_GLOBAL_ROLES'] : $enable_global_roles_setting;
		}

		if (($enable_global_roles_setting != SQ_CONF_ENABLE_GLOBAL_ROLES) || ($enable_roles_perm_system_setting != SQ_CONF_ENABLE_ROLES_PERM_SYSTEM) || $enable_roles_wf_system_setting != SQ_CONF_ENABLE_ROLES_WF_SYSTEM) {
			$enabled = ($enable_roles_perm_system_setting || $enable_roles_wf_system_setting);
			$this->configureRoleTables($enabled, $enable_global_roles_setting);
		}

		if(!isset($_POST['system_config']['SQ_CONF_FORCE_SECURE_LOGIN_URLS'])) {
			// if not set, it means it should be blanked out
			$_POST['system_config']['SQ_CONF_FORCE_SECURE_LOGIN_URLS'] = '';
		}
		return parent::processBackend($o);

	}//end processBackend()


	/**
	* Configures the Permissions and Roles database views to reflect the specified roles state
	*
	* @param boolean	$enable_roles_system	whether to enable or disable the Roles system completely
	* @param boolean	$enable_global_roles	whether to enable or disable Global Roles
	*
	* @throws Exception when there is a database error.
	* @return boolean
	* @access public
	*/
	public function configureRoleTables($enable_roles_system=TRUE, $enable_global_roles=TRUE)
	{
		require_once SQ_LIB_PATH.'/db_install/db_install.inc';
		$roles_changed = FALSE;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');

		$db = MatrixDAL::getDb();
		$db_type = MatrixDAL::getDbType();

		/*
		* Default behaviour is to retain the standard Matrix Roles View unless modified, in which case we will define a simpler view
		* 1. Modify Roles View to remove Global Role associations
		* 2. Modify Permissions View to disable the Roles System entirely
		*/
		$create_role_view_sql = $this->_getCommonViewDefinition('views_roles', $db_type, 'sq_vw_ast_role '.((!$enable_global_roles) ? 'Disabled ' : '').'View');
		$create_perm_view_sql = $this->_getCommonViewDefinition('views_roles', $db_type, 'sq_vw_ast_perm '.((!$enable_roles_system) ? 'Disabled ' : '').'View');

		// Let's start a transaction for safety
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$drop_success = FALSE;

		// Attempt "DROP VIEW ... IF EXISTS" for PostGres, "DROP VIEW" for Oracle
		try {
			if ($db_type == 'pgsql') {
				// Drop the views first
				// If we can't drop or create views accordingly then we'll roll back the transaction in the "catch" block
				MatrixDAL::executeSql('DROP VIEW IF EXISTS sq_vw_ast_perm');
				MatrixDAL::executeSql('DROP VIEW IF EXISTS sq_vw_ast_role');

				$drop_success = TRUE;
			} else if ($db_type == 'oci') {
				// find out first if the view exists or else oracle will
				// throw warning oci_execute(): ORA-00942: table or view does not exist
				$perm_sql = "SELECT VIEW_NAME FROM ALL_VIEWS WHERE VIEW_NAME = 'SQ_VW_AST_PERM'";
				$role_sql = "SELECT VIEW_NAME FROM ALL_VIEWS WHERE VIEW_NAME = 'SQ_VW_AST_ROLE'";

				$perm_vw_query = MatrixDAL::preparePdoQuery($perm_sql);
				$role_vw_query = MatrixDAL::preparePdoQuery($role_sql);

				$perm_vw_exists = MatrixDAL::executePdoAssoc($perm_vw_query);
				$role_vw_exists = MatrixDAL::executePdoAssoc($role_vw_query);

				if (isset($perm_vw_exists['0']['view_name']) && $perm_vw_exists['0']['view_name'] == 'SQ_VW_AST_PERM') {
					MatrixDAL::executeSql('DROP VIEW sq_vw_ast_perm');
				}

				if (isset($role_vw_exists['0']['view_name']) && $role_vw_exists['0']['view_name'] == 'SQ_VW_AST_ROLE') {
					MatrixDAL::executeSql('DROP VIEW sq_vw_ast_role');
				}

				$drop_success = TRUE;
			}
		} catch (Exception $e) {
			// Don't let the world know, just keep on keeping on - we'll create the view in a sec
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');

			// A new transaction
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		}

		// If the "DROP VIEW ... IF EXISTS" failed under PostGres, then maybe we are running PostGres 8.1.
		// In this case, give it another go
		if (!$drop_success && ($db_type == 'pgsql')) {
			try {
				MatrixDAL::executeSql('DROP VIEW sq_vw_ast_perm');
				MatrixDAL::executeSql('DROP VIEW sq_vw_ast_role');

				$drop_success = TRUE;
			} catch (Exception $e) {
				// Que sera, sera
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');

				// A new transaction
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			}
		}

		// Assume that the views have been dropped, so let's re-create them again
		try {
			// Then create the new views
			MatrixDAL::executeSql($create_role_view_sql);
			MatrixDAL::executeSql($create_perm_view_sql);

			$roles_changed = TRUE;
		} catch (Exception $e) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			throw $e;
		}

		// Allow the secondary user to operate (select, update etc.) with the Roles views
		try {
			grant_secondary_user_perms(FALSE);
		} catch(Exception $e) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			throw $e;
		}

		// All is good, commit the transaction
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return $roles_changed;

	}//end configureRoleTables()


	/**
	* Obtains a database definition from the specified core or package XML file
	*
	* @param string		$relations	the type of relations to install. Eg. functions|views
	* @param string		$db_type	the database type (eg; 'pgsql')
	* @param boolean	$display_name	the alias to refer to in the XML file for the wanted query
	* @param string		$package	the Matrix Package in which the query can be found
	*
	* @return string
	* @access private
	*/
	private function _getCommonViewDefinition($relations, $db_type, $display_name, $package=NULL)
	{
		$sql = NULL;

		if (is_null($package)) {
			$subdir = 'install';
		} else {
			$subdir = 'packages/'.$package;
		}
		$fns_file = SQ_SYSTEM_ROOT.'/'.$subdir.'/'.$db_type.'_'.$relations.'.xml';

			// if the file does not exist, check to see if there is a common file
			// for all database types
		if (!file_exists($fns_file)) {
			$fns_file = SQ_SYSTEM_ROOT.'/'.$subdir.'/common_'.$relations.'.xml';
		}
		if (file_exists($fns_file)) {

			try {
				// Load stored relations file as SimpleXML
				$root = simplexml_load_string(file_get_contents($fns_file), 'SimpleXMLElement', LIBXML_NOCDATA);
			} catch (Exception $e) {
				throw new Exception('Could not parse stored relations file: '.$e->getMessage());
			}

			if ($root->getName() != 'sql') {
				throw new Exception('Cannot install stored relations file: expected root element "sql", found "'.$root->getName().'".');
				return FALSE;
			}

			$display_names = Array();

			foreach ($root->children() as $sql_element) {
				$element_name = $sql_element->getName();
				if ($sql_element->getName() != 'sql_element') {
					throw new Exception('Cannot install stored relations file: expected element "sql_element", found "'.$sql_element->getName().'".');
				}

				$display_names[] = isset($sql_element->attributes()->{'display_name'}) ? $sql_element->attributes()->{'display_name'} : '<Unknown Element>';
				if ($sql_element->attributes()->{'display_name'} == $display_name) {
					$sql = trim(xml_entity_decode((string)$sql_element));
				}
			}
		}

		return $sql;

	}//end _getCommonViewDefinition()


}//end class

?>
