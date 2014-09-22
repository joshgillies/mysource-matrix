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
						'SQ_CONF_NUM_DATA_DIRS'					=> Array('editable' => 0, 'default' => 20),
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
						'SQ_CONF_VISITED_PAGES_MAX_ENTRIES'		=> Array('editable' => 1, 'default' => '10'),
						'SQ_FILE_TYPE_RESTRICT_ON_STATIC_URL'	=> Array('editable' => 1, 'default' => ''),
						'SQ_CONFIG_STRIP_MATRIX_COMMENTS'		=> Array('editable' => 1, 'default' => TRUE),
					   );

	/**
	* Holds all the valid debug settings and their descriptions for SQ_CONF_DEBUG
	* The keys are the bit elements and the values are the descriptions
	* @var array
	*/
	// TODO: TRANSLATE: these need to be either moved somewhere else, or implement some sort of global code
	private $debug_options = Array(
							1	=> 'Show File and Line number in error messages',
							2	=> 'Show Stack Trace in error messages',
							4	=> 'Show additional information about memory and performance',
							8	=> 'Show current frontend assetid and url where the error message is triggered from',
						 );


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
					trigger_localised_error('SYS0267', E_USER_WARNING, $this->name);
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
			$o->closeField();

			$o->openField(translate('System Backend Suffix'));

			if ($write_access) {
				text_box($class.'[SQ_CONF_BACKEND_SUFFIX]', SQ_CONF_BACKEND_SUFFIX, 15);
			} else {
				echo nl2br(SQ_CONF_BACKEND_SUFFIX);
			}

			$o->openField(translate('System Simple Edit Suffix'));

			if ($write_access) {
				text_box($class.'[SQ_CONF_LIMBO_SUFFIX]', SQ_CONF_LIMBO_SUFFIX, 15);
			} else {
				echo nl2br(SQ_CONF_LIMBO_SUFFIX);
			}

			$o->openField(translate('System Login Suffix'));

			if ($write_access) {
				text_box($class.'[SQ_CONF_LOGIN_SUFFIX]', SQ_CONF_LOGIN_SUFFIX, 15);
			} else {
				echo nl2br(SQ_CONF_LOGIN_SUFFIX);
			}

			$o->openField(translate('System Bypass Cache Suffix'));

			if ($write_access) {
				text_box($class.'[SQ_CONF_NOCACHE_SUFFIX]', SQ_CONF_NOCACHE_SUFFIX, 15);
			} else {
				echo nl2br(SQ_CONF_NOCACHE_SUFFIX);
			}

			$o->openField(translate('System Bypass Proxy Cache Suffix'));

			if ($write_access) {
				text_box($class.'[SQ_CONF_NOPROXYCACHE_SUFFIX]', SQ_CONF_NOPROXYCACHE_SUFFIX, 15);
			} else {
				echo nl2br(SQ_CONF_NOPROXYCACHE_SUFFIX);
			}


			$o->openField(translate('System Clear Cache Suffix'));

			if ($write_access) {
				text_box($class.'[SQ_CONF_RECACHE_SUFFIX]', SQ_CONF_RECACHE_SUFFIX, 15);
			} else {
				echo nl2br(SQ_CONF_RECACHE_SUFFIX);
			}

			$o->openField(translate('System Performance Mode Suffix'));

			if ($write_access) {
				text_box($class.'[SQ_CONF_PERFORMANCE_SUFFIX]', SQ_CONF_PERFORMANCE_SUFFIX, 15);
			} else {
				echo nl2br(SQ_CONF_PERFORMANCE_SUFFIX);
			}

			$o->openField(translate('System Timezone'));

			if ($write_access) {
				text_box($class.'[SQ_CONF_TIMEZONE]', SQ_CONF_TIMEZONE, 15);
			} else {
				echo nl2br(SQ_CONF_TIMEZONE);
			}

			$o->openField(translate('Disable Attribution'));

			if ($write_access) {
				combo_box($class.'[SQ_CONF_DISABLE_ATTRIBUTION]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_CONF_DISABLE_ATTRIBUTION);


				$o->note(translate('<b>NOTE:</b> Editing this option will need the designs to be regenerated before this option takes affect.'));

			} else {
				echo (SQ_CONF_DISABLE_ATTRIBUTION) ? translate('Yes') : translate('No');


			}

			$o->openField(translate('Enforce Same Origin Frame'));

			if ($write_access) {
				combo_box($class.'[SQ_CONF_ENFORCE_FRAME_SAMEORIGIN]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_CONF_ENFORCE_FRAME_SAMEORIGIN);


				$o->note(translate('If Matrix is nested in a frame of a page, the page must be hosted on same domain of Matrix.'));

			} else {
				echo (SQ_CONF_ENFORCE_FRAME_SAMEORIGIN) ? translate('Yes') : translate('No');


			}
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

			$o->openField(translate('Static URL uses HTTP'));

				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_STATIC_ROOT_HTTP]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_CONF_STATIC_ROOT_HTTP);


				} else {
					echo (SQ_CONF_STATIC_ROOT_HTTP) ? translate('Yes') : translate('No');


				}
			$o->closeField();

			$o->openField(translate('Static URL uses HTTPS'));

				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_STATIC_ROOT_HTTPS]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_CONF_STATIC_ROOT_HTTPS);


				} else {
					echo (SQ_CONF_STATIC_ROOT_HTTPS) ? translate('Yes') : translate('No');


				}
			$o->closeField();

			$o->openField(translate('System Web Path Separator'));

			if ($write_access) {
				text_box($class.'[SQ_CONF_WEB_PATH_SEPARATOR]', SQ_CONF_WEB_PATH_SEPARATOR, 1, 1);
			} else {
				echo nl2br(SQ_CONF_WEB_PATH_SEPARATOR);
			}
			$o->closeField();

			$o->openField(translate('Redirect URL with Trailing Slash'));

				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_REDIRECT_URL_WITH_TRAILING_SLASH]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_CONF_REDIRECT_URL_WITH_TRAILING_SLASH);


				} else {
					echo (SQ_CONF_REDIRECT_URL_WITH_TRAILING_SLASH) ? translate('Yes') : translate('No');


				}
			$o->note(translate('This will redirect front-end URL requests (excluding System Root URLs) to a URL with no trailing slashes.'));

			$o->closeField();

		$o->closeSection();

		$o->openSection(translate('Rollback Settings'));


			$o->openField(translate('Rollback Enabled'));

				echo (SQ_CONF_ROLLBACK_ENABLED) ? translate('Yes') : translate('No');


				$o->note(translate('Rollback must be enabled and disabled using the Rollback Management Tool, or the "rollback_management.php" script in the MySource "scripts" directory'));

			$o->closeField();

		$o->closeSection();

		$o->openSection(translate('Email Settings'));


			$o->openField(translate('Default Email'));

				if ($write_access) {
					text_box($class.'[SQ_CONF_DEFAULT_EMAIL]', SQ_CONF_DEFAULT_EMAIL, 50);
				} else {
					echo SQ_CONF_DEFAULT_EMAIL;
				}
			$o->closeField();

			$o->openField(translate('Tech Email'));

				if ($write_access) {
					text_box($class.'[SQ_CONF_TECH_EMAIL]', SQ_CONF_TECH_EMAIL, 50);
				} else {
					echo SQ_CONF_TECH_EMAIL;
				}
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
									echo '<img src="'.sq_web_path('lib').'/web/images/'.$filename.'.gif" />';
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
			$o->note(translate('Zero (0) is unlimited attempts.'));


			$o->openField(translate('Allow IP Change'));

			if ($write_access && $is_root) {
				combo_box($class.'[SQ_CONF_ALLOW_IP_CHANGE]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_CONF_ALLOW_IP_CHANGE);


			} else {
				echo (SQ_CONF_ALLOW_IP_CHANGE) ? translate('Yes') : translate('No');


			}
			$o->note(translate('Allow the IP of a user to fluctuate?'));


			$o->openField(translate('Process PHP Credentials'));

			if ($write_access && $is_root) {
				combo_box($class.'[SQ_CONF_ACCEPT_HTTP_CREDS]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_CONF_ACCEPT_HTTP_CREDS);


			} else {
				echo (SQ_CONF_ACCEPT_HTTP_CREDS) ? translate('Yes') : translate('No');


			}
			$o->note(translate('If this is enabled and a username and password are supplied in PHP_AUTH_USR and PHP_AUTH_PW, the system will try to authenticate a user with those credentials.'));


			$o->openField(translate('Enable HTTP Authentication'));

			if ($write_access && $is_root) {
				combo_box($class.'[SQ_CONF_ENABLE_HTTP_LOGIN]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_CONF_ENABLE_HTTP_LOGIN);


			} else {
				echo (SQ_CONF_ENABLE_HTTP_LOGIN) ? translate('Yes') : translate('No');


			}
			$o->note(translate('If this option is enabled, Squiz Matrix will generate an HTTP Authentication dialog box instead of painting the applied login design. This will allow external tools to login to Squiz Matrix by appending USE_HTTP_LOGIN=1 to the URL. Process PHP Credentials option has to be also enabled for HTTP Authentication to work.'));


			$o->openField(translate('Accept HTTP Authentication'));

			if ($write_access && $is_root) {
				combo_box($class.'[SQ_CONF_ALLOW_HTTP_LOGIN]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_CONF_ALLOW_HTTP_LOGIN);


			} else {
				echo (SQ_CONF_ALLOW_HTTP_LOGIN) ? translate('Yes') : translate('No');


			}
			$o->note(translate('If this option is enabled and a username is found in the server variable specified below, the system will try to automatically log in a user with that username.  (It will assume that the user has already been authenticated by some external mechanism.)'));


			$o->openField(translate('HTTP Authentication Variable'));

			if ($write_access && $is_root) {

				$server = array_keys($_SERVER);

				?>

				<script language="JavaScript" type="text/javascript">
				//<![CDATA[
					function toggle_http_login_var() {
						list = document.getElementById('sq-conf-http-login-var-list');
						txt = document.getElementById('sq-conf-http-login-var-toggle');
						if (txt && list) {
							list.style.display = (list.style.display == 'none' ? 'block' : 'none');
							txt.innerHTML = js_translate('http_login_var_' + (list.style.display == 'none' ? 'show' : 'hide') + '_server_var');
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

			$o->openField(translate('Enable External Authentication Systems'));

			if ($write_access && $is_root) {
				combo_box($class.'[SQ_CONF_ENABLE_EXTERNAL_AUTH_SYSTEMS]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_CONF_ENABLE_EXTERNAL_AUTH_SYSTEMS);


			} else {
				echo (SQ_CONF_ENABLE_EXTERNAL_AUTH_SYSTEMS) ? translate('Yes') : translate('No');


			}
			$o->note(translate('Set this to \'No\' to only allow users with accounts in the default matrix user system, rather than LDAP, IPB etc.'));


			$o->openField(translate('Use Default PHP session file save path'));

			if ($write_access && $is_root) {
				combo_box($class.'[SQ_CONF_USE_DEFAULT_SESSION_SAVE_PATH]', Array('1' => translate('True'), '0' => translate('False')), FALSE, SQ_CONF_USE_DEFAULT_SESSION_SAVE_PATH);


			} else {
				echo (SQ_CONF_USE_DEFAULT_SESSION_SAVE_PATH) ? translate('True') : translate('False');


			}
			$o->note(sprintf(translate('Using the default PHP session file may be required for system with multiple servers using shared file storage. However, enabling this option will prevent Site Network assets from operating correctly. <strong>This option should only be changed by advanced users: if you change this option all users who are currently logged in will be immediately logged out of their session.</strong> <br />The current session file save path is: <strong>%s</strong>'), session_save_path()));


		$o->closeSection();

		$o->openSection(translate('Intervals'));


			$o->openField(translate('Refresh Interval'));

			if ($write_access) {
				text_box($class.'[SQ_CONF_REFRESH_INTERVAL]', SQ_CONF_REFRESH_INTERVAL, 5, 5);
			} else {
				echo SQ_CONF_REFRESH_INTERVAL;
			}
			$o->note(translate('How long (in seconds) between refreshes of the top (header) frame - this frame keeps the lock on long edits and checks for new mail - Min: 60.'));


			$o->openField(translate('Lock Length'));

			if ($write_access) {
				text_box($class.'[SQ_CONF_LOCK_LENGTH]', SQ_CONF_LOCK_LENGTH, 5, 5);
			} else {
				echo SQ_CONF_LOCK_LENGTH;
			}
			$o->closeField(translate('The length of time after which a lock is considered to have expired. In seconds - Min: 60'));


		$o->closeSection();

		$o->openSection(translate('PHP Configuration'));


			$o->openField(translate('Web Memory Limit'));

			if ($write_access && $is_root) {
				int_text_box($class.'[SQ_CONF_WEB_MEMORY_LIMIT]', SQ_CONF_WEB_MEMORY_LIMIT, FALSE, 5, NULL, NULL, 3);
				echo ' MB';
			} else {
				echo SQ_CONF_WEB_MEMORY_LIMIT.'MB';
			}
			$o->note(translate('The maximum amount of system memory allocated to each instance of Squiz Matrix web system.'));

			$o->closeField();

			$o->openField(translate('Cron Memory Limit'));

			if ($write_access && $is_root) {
				int_text_box($class.'[SQ_CONF_CRON_MEMORY_LIMIT]', SQ_CONF_CRON_MEMORY_LIMIT, FALSE, 5, NULL, NULL, 3);
				echo ' MB';
			} else {
				echo SQ_CONF_CRON_MEMORY_LIMIT.'MB';
			}
			$o->note(translate('The maximum amount of system memory allocated to each instance of the Squiz Matrix cron system.'));

			$o->closeField();

		$o->closeSection();


		$o->openSection(translate('Error/Debug Settings'));


			$o->openField(translate('Log Errors'));

			if ($write_access && $is_root) {
				combo_box($class.'[SQ_CONF_LOG_ERRORS]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_CONF_LOG_ERRORS);


			} else {
				echo (SQ_CONF_LOG_ERRORS) ? translate('Yes') : translate('No');


			}
			$o->closeField();

			$o->openField(translate('Hide Errors on the Frontend'));

			if ($write_access && $is_root) {
				combo_box($class.'[SQ_CONF_ERRORS_HIDE_FRONTEND]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_CONF_ERRORS_HIDE_FRONTEND);


			} else {
				echo (SQ_CONF_ERRORS_HIDE_FRONTEND) ? translate('Yes') : translate('No');


			}
			$o->closeField();

			$o->openField(translate('Log Errors to Syslog?'));

			if ($write_access && $is_root) {
				combo_box($class.'[SQ_CONF_ERRORS_LOG_TO_SYSLOG]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_CONF_ERRORS_LOG_TO_SYSLOG);


			} else {
				echo (SQ_CONF_ERRORS_LOG_TO_SYSLOG) ? translate('Yes') : translate('No');


			}
			$o->note(translate('If enabled, errors will be sent to the *nix Syslog. Please contact your system administrator for appropriate settings if you are to use this feature.'));

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
					<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo (SQ_CONF_DEBUG & $bit) ? 'tick' : 'cross'; ?>.gif" width="15" height="15" />
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

			$o->openField(translate('Use language character map'));

			if ($write_access) {
				$options = Array(
							'0'	=> translate('No'),

							'1'	=> translate('Yes'),

						   );
				combo_box($class.'[SQ_CONF_USE_LANG_CHAR_MAP]', $options, FALSE, !defined('SQ_CONF_USE_LANG_CHAR_MAP') ? '0' : SQ_CONF_USE_LANG_CHAR_MAP);
			} else {
				echo (SQ_CONF_USE_LANG_CHAR_MAP) ? translate('Yes') : translate('No');


			}

		$o->closeSection();

		$o->openSection(translate('Editing Interface Settings'));


			$o->openField(translate('Commit Button Text'));

			if ($write_access) {
				text_box($class.'[SQ_CONF_COMMIT_BUTTON_TEXT]', !defined('SQ_CONF_COMMIT_BUTTON_TEXT') ? 'Commit' : SQ_CONF_COMMIT_BUTTON_TEXT, 30);
			} else {
				echo !defined('SQ_CONF_COMMIT_BUTTON_TEXT') ? 'Commit' : SQ_CONF_COMMIT_BUTTON_TEXT;
			}
			$o->note(translate('This setting renames the button used to commit edited information into the Squiz Matrix system - default is \'Commit\'.'));

			$o->closeField();

			$o->openField(translate('Confirm Save Changes'));

			if ($write_access) {
				$options = Array(
							'0'	=> translate('No'),

							'1'	=> translate('Yes'),

						   );
				combo_box($class.'[SQ_CONF_CONFIRM_SAVE_CHANGES]', $options, FALSE, !defined('SQ_CONF_CONFIRM_SAVE_CHANGES') ? '1' : SQ_CONF_CONFIRM_SAVE_CHANGES);
			} else {
				if (defined('SQ_CONF_CONFIRM_SAVE_CHANGES')) {
					?>
					<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo (SQ_CONF_CONFIRM_SAVE_CHANGES) ? 'tick' : 'cross'; ?>.gif" width="15" height="15" />
					<?php
				} else {
					echo translate('Not Set');

				}//end if
			}//end if
			$o->note(translate('This setting, if set, warns a user when changes are made but not committed.'));

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


			$o->openField(translate('Send Cacheable Header'));

				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_SEND_CACHEABLE_HEADER]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_CONF_SEND_CACHEABLE_HEADER);


				} else {
					echo (SQ_CONF_SEND_CACHEABLE_HEADER) ? translate('Yes') : translate('No');


				}
				$o->note(translate('If enabled, server will send cacheable Cache-Control and Pragma headers.'));

			$o->closeField();

			$o->openField(translate('Send Last-Modified Header'));

				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_SEND_LAST_MODIFIED_HEADER]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_CONF_SEND_LAST_MODIFIED_HEADER);


				} else {
					echo (SQ_CONF_SEND_LAST_MODIFIED_HEADER) ? translate('Yes') : translate('No');


				}
				$o->note(translate('If enabled, server will send a timestamp indicating when the asset was last modified.'));

			$o->closeField();

			$o->openField(translate('Send Not Modified Status-Code'));

				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_SEND_NOT_MODIFIED_HEADER]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_CONF_SEND_NOT_MODIFIED_HEADER);


				} else {
					echo (SQ_CONF_SEND_NOT_MODIFIED_HEADER) ? translate('Yes') : translate('No');


				}
				$o->note(translate('If enabled, server will send a "304 Not Modified" Status-Code indicating nothing has been changed since last visit.'));

				$o->note(translate('Note: Only applicable to Live assets that are accessible by public users.'));

			$o->closeField();

			$o->openField(translate('Send no-cache header for File assets'));

				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_SEND_NO_CACHE_HEADER]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_CONF_SEND_NO_CACHE_HEADER);


				} else {
					echo (SQ_CONF_SEND_NO_CACHE_HEADER) ? translate('Yes') : translate('No');


				}
				$o->note(translate('If enabled, the "no-cache" Cache-Control option will be sent when serving File asset types. This option can be used to resolve inline file display issues in Internet Explorer.'));

			$o->closeField();

			$o->openField(translate('Send 404 Cacheable Header'));

				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_SEND_404_CACHEABLE_HEADER]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_CONF_SEND_404_CACHEABLE_HEADER);


				} else {
					echo (SQ_CONF_SEND_404_CACHEABLE_HEADER) ? translate('Yes') : translate('No');


				}
				$o->note(translate('If enabled, server will send cacheable Cache-Control and Pragma headers for 404 response to allow Proxy server to cache.'));

			$o->closeField();

			$o->openField(translate('Use "X-Forwarded-For" Header'));

				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_USE_HTTP_X_FORWARDED_FOR]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_CONF_USE_HTTP_X_FORWARDED_FOR);


				} else {
					echo (SQ_CONF_USE_HTTP_X_FORWARDED_FOR) ? translate('Yes') : translate('No');


				}
				$o->note(translate('Only enable to help obtain User IP when behind a proxy. System will attempt to use "X-Forwarded-For" header. By default the REMOTE_ADDR server variable is used. This will only work in conjunction with a trusted/known Proxy IP list below.'));


				$option_list->setEditParam('width', 30);
				if ($write_access) {
					$option_list->value = SQ_CONF_FORWARDED_IP_PROXY_LIST;
					$option_list->paint('SQ_CONF_FORWARDED_IP_PROXY_LIST');
				} else {
					if (SQ_CONF_FORWARDED_IP_PROXY_LIST == '') {
						echo translate('None');

					} else {
						echo nl2br(SQ_CONF_FORWARDED_IP_PROXY_LIST);
					}
				}
			$o->closeField();

			$o->openField(translate('HTTP only for cookie'));

				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_COOKIE_OPTION_HTTP_ONLY]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_CONF_COOKIE_OPTION_HTTP_ONLY);


				} else {
					echo (SQ_CONF_COOKIE_OPTION_HTTP_ONLY) ? translate('Yes') : translate('No');


				}
				$o->note(translate('Enable to set \'HttpOnly\' option for the cookies. When a cookie is HttpOnly the web browser should not allow client side scripts such as JavaScript to have access to the cookie. This can help mitigate the effects of cross site scripting attacks.<i><b> (Available only for PHP Version greater than 5.2.0)</i></b>'));


			$o->closeField();

			$o->openField(translate('Enable \'Secure\' Cookie option'));

				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_COOKIE_OPTION_SECURE]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_CONF_COOKIE_OPTION_SECURE);


				} else {
					echo (SQ_CONF_COOKIE_OPTION_SECURE) ? translate('Yes') : translate('No');


				}
				$o->note(translate('Specifies whether or not to transmit the secure cookie flag when a connection is made over HTTPS. Enabling this will cause browsers to not share the session cookie between HTTP and HTTPS.'));


			$o->closeField();


		$o->closeSection();


		$o->openSection(translate('Roles Configuration'));


			$o->openField(translate('Enable Permission Roles System'));

				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_ENABLE_ROLES_PERM_SYSTEM]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_CONF_ENABLE_ROLES_PERM_SYSTEM);


				} else {
					echo (SQ_CONF_ENABLE_ROLES_PERM_SYSTEM) ? translate('Yes') : translate('No');


				}
			$o->closeField();

			$o->openField(translate('Enable Workflow Roles System'));

				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_ENABLE_ROLES_WF_SYSTEM]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_CONF_ENABLE_ROLES_WF_SYSTEM);


				} else {
					echo (SQ_CONF_ENABLE_ROLES_WF_SYSTEM) ? translate('Yes') : translate('No');


				}
			$o->closeField();

			if ( ((defined('SQ_CONF_ENABLE_ROLES_PERM_SYSTEM') && (SQ_CONF_ENABLE_ROLES_PERM_SYSTEM == '1')) || (defined('SQ_CONF_ENABLE_ROLES_WF_SYSTEM') && (SQ_CONF_ENABLE_ROLES_WF_SYSTEM == '1')) ) || (!defined('SQ_CONF_ENABLE_ROLES_PERM_SYSTEM') && !defined('SQ_CONF_ENABLE_ROLES_WF_SYSTEM') )) {
				$o->openField(translate('Enable Global Roles'));

					if ($write_access && $is_root) {
						combo_box($class.'[SQ_CONF_ENABLE_GLOBAL_ROLES]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_CONF_ENABLE_GLOBAL_ROLES);


					} else {
						echo (SQ_CONF_ENABLE_GLOBAL_ROLES) ? translate('Yes') : translate('No');


					}
				$o->closeField();
			}

		$o->closeSection();

		$o->openSection(translate('Search Engine Optimizations'));


			$o->openField(translate('Remove self links'));

				if ($write_access) {
					$options = Array(
								'0'	=> translate('No'),

								'1'	=> translate('Yes'),

							   );
					combo_box($class.'[SQ_CONF_NO_ASSET_SELF_LINKS]', $options, FALSE, !defined('SQ_CONF_NO_ASSET_SELF_LINKS') ? '0' : SQ_CONF_NO_ASSET_SELF_LINKS);
				} else {
					echo (SQ_CONF_NO_ASSET_SELF_LINKS) ? translate('Yes') : translate('No');


				}
				$o->note(translate('Select \'Yes\' if you want all links directing to currently viewed asset removed from content.'));

			$o->closeField();

		$o->closeSection();

		$o->openSection(translate('Miscellaneous Settings'));

			$o->openField(translate('Visited Pages Maximum Entries'));

			if ($write_access && $is_root) {
				text_box($class.'[SQ_CONF_VISITED_PAGES_MAX_ENTRIES]', !defined('SQ_CONF_VISITED_PAGES_MAX_ENTRIES') ? '10' : SQ_CONF_VISITED_PAGES_MAX_ENTRIES, 30);
			} else {
				echo !defined('SQ_CONF_VISITED_PAGES_MAX_ENTRIES') ? '10' : SQ_CONF_VISITED_PAGES_MAX_ENTRIES;
			}
				$o->note(translate('Maximum number of visited URLs entires to store in the session. Visited URLs and asset ids can be accessed via global session var "visited_urls" and "visited_assets" respectively.'));

			$o->closeField();

			$o->openField(translate('Strip Matrix Comments'));

				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONFIG_STRIP_MATRIX_COMMENTS]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_CONFIG_STRIP_MATRIX_COMMENTS);


				} else {
					echo (SQ_CONF_STATIC_ROOT_HTTP) ? translate('Yes') : translate('No');


				}
				$o->note(htmlspecialchars(translate('Strip out special Matrix comments, "<!--@@ @@-->" and "/*@@ @@*/" from the output.')));

			$o->closeField();

		$o->closeSection();

		$o->openSection(translate('Install Key'));


			$o->openField(translate('Install Key'));

				echo SQ_LICENCE_INSTALL_KEY;
				$o->note(translate('The install key is automatically generated when the <em>step_03.php</em> script is run to reconfigure your system - it cannot be changed here.'));

			$o->closeField();
		$o->closeSection();

		if ($write_access) $o->commitButton('', TRUE);

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
