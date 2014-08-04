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
						'SQ_CONF_SEND_CACHEABLE_HEADER'			=> Array('editable' => 1, 'default' => '0'),
						'SQ_CONF_SEND_LAST_MODIFIED_HEADER'		=> Array('editable' => 1, 'default' => '0'),
						'SQ_CONF_SEND_NOT_MODIFIED_HEADER'		=> Array('editable' => 1, 'default' => '0'),
						'SQ_CONF_SEND_NO_CACHE_HEADER'			=> Array('editable' => 1, 'default' => '0'),
						'SQ_CONF_SEND_404_CACHEABLE_HEADER'		=> Array('editable' => 1, 'default' => '0'),
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

		$o->openSection(translate('system_settings'));

			$o->openField(translate('system_name'));
				if ($write_access) {
					text_box($class.'[SQ_CONF_SYSTEM_NAME]', SQ_CONF_SYSTEM_NAME, 30);
				} else {
					echo SQ_CONF_SYSTEM_NAME;
				}
			$o->closeField();

			$o->openField(translate('system_owner'));
			if ($write_access) {
				text_box($class.'[SQ_CONF_SYSTEM_OWNER]', SQ_CONF_SYSTEM_OWNER, 30);
			} else {
				echo SQ_CONF_SYSTEM_OWNER;
			}
			$o->closeField();

			$o->openField(translate('system_backend_suffix'));
			if ($write_access) {
				text_box($class.'[SQ_CONF_BACKEND_SUFFIX]', SQ_CONF_BACKEND_SUFFIX, 15);
			} else {
				echo nl2br(SQ_CONF_BACKEND_SUFFIX);
			}

			$o->openField(translate('system_simple_edit_suffix'));
			if ($write_access) {
				text_box($class.'[SQ_CONF_LIMBO_SUFFIX]', SQ_CONF_LIMBO_SUFFIX, 15);
			} else {
				echo nl2br(SQ_CONF_LIMBO_SUFFIX);
			}

			$o->openField(translate('system_login_suffix'));
			if ($write_access) {
				text_box($class.'[SQ_CONF_LOGIN_SUFFIX]', SQ_CONF_LOGIN_SUFFIX, 15);
			} else {
				echo nl2br(SQ_CONF_LOGIN_SUFFIX);
			}

			$o->openField(translate('system_nocache_suffix'));
			if ($write_access) {
				text_box($class.'[SQ_CONF_NOCACHE_SUFFIX]', SQ_CONF_NOCACHE_SUFFIX, 15);
			} else {
				echo nl2br(SQ_CONF_NOCACHE_SUFFIX);
			}

			$o->openField(translate('system_recache_suffix'));
			if ($write_access) {
				text_box($class.'[SQ_CONF_RECACHE_SUFFIX]', SQ_CONF_RECACHE_SUFFIX, 15);
			} else {
				echo nl2br(SQ_CONF_RECACHE_SUFFIX);
			}
			
			$o->openField(translate('system_performance_suffix'));
			if ($write_access) {
				text_box($class.'[SQ_CONF_PERFORMANCE_SUFFIX]', SQ_CONF_PERFORMANCE_SUFFIX, 15);
			} else {
				echo nl2br(SQ_CONF_PERFORMANCE_SUFFIX);
			}

			$o->openField(translate('system_timezone'));
			if ($write_access) {
				text_box($class.'[SQ_CONF_TIMEZONE]', SQ_CONF_TIMEZONE, 15);
			} else {
				echo nl2br(SQ_CONF_TIMEZONE);
			}

			$o->openField(translate('disable_attribution'));
			if ($write_access) {
				combo_box($class.'[SQ_CONF_DISABLE_ATTRIBUTION]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_CONF_DISABLE_ATTRIBUTION);
				$o->note(translate('disable_attribution_note'));
			} else {
				echo (SQ_CONF_DISABLE_ATTRIBUTION) ? translate('yes') : translate('no');
			}
			
			$o->openField(translate('enforce_frame_sameorigin'));
			if ($write_access) {
				combo_box($class.'[SQ_CONF_ENFORCE_FRAME_SAMEORIGIN]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_CONF_ENFORCE_FRAME_SAMEORIGIN);
				$o->note(translate('enforce_frame_sameorigin_note'));
			} else {
				echo (SQ_CONF_ENFORCE_FRAME_SAMEORIGIN) ? translate('yes') : translate('no');
			}
		$o->closeSection();

		$o->openSection(translate('system_urls'));

			require_once SQ_ATTRIBUTES_PATH.'/option_list/option_list.inc';

			$option_list = new Asset_Attribute_Option_List(0, NULL, SQ_OPTION_LIST_DELIMITER_UNIX);
			$option_list->setEditParam('width', 60);

			$o->openField(translate('system_root_urls'));
				if ($write_access) {
					$option_list->value = SQ_CONF_SYSTEM_ROOT_URLS;
					$option_list->paint('SQ_CONF_SYSTEM_ROOT_URLS');
				} else {
					echo nl2br(SQ_CONF_SYSTEM_ROOT_URLS);
				}
			$o->closeField();

			$o->openField(translate('system_parent_domains'));
				if ($write_access) {
					$option_list->value = SQ_CONF_SYSTEM_PARENT_DOMAINS;
					$option_list->paint('SQ_CONF_SYSTEM_PARENT_DOMAINS');
				} else {
					if (SQ_CONF_SYSTEM_PARENT_DOMAINS == '') {
						echo translate('none');
					} else {
						echo nl2br(SQ_CONF_SYSTEM_PARENT_DOMAINS);
					}
				}
				$o->note(translate('system_parent_domains_note'));
			$o->closeField();

			$o->openField(translate('system_static_url'));
				if ($write_access && $is_root) {
					text_box($class.'[SQ_CONF_STATIC_ROOT_URL]', SQ_CONF_STATIC_ROOT_URL, 80);
				} else {
					if (SQ_CONF_STATIC_ROOT_URL == '') {
						echo translate('not_set');
					} else {
						echo SQ_CONF_STATIC_ROOT_URL;
					}
				}
				$o->note(translate('system_static_url_note'));
			$o->closeField();

			$o->openField(translate('system_static_url_http'));
				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_STATIC_ROOT_HTTP]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_CONF_STATIC_ROOT_HTTP);
				} else {
					echo (SQ_CONF_STATIC_ROOT_HTTP) ? translate('yes') : translate('no');
				}
			$o->closeField();

			$o->openField(translate('system_static_url_https'));
				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_STATIC_ROOT_HTTPS]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_CONF_STATIC_ROOT_HTTPS);
				} else {
					echo (SQ_CONF_STATIC_ROOT_HTTPS) ? translate('yes') : translate('no');
				}
			$o->closeField();

			$o->openField(translate('system_web_path_separator'));
			if ($write_access) {
				text_box($class.'[SQ_CONF_WEB_PATH_SEPARATOR]', SQ_CONF_WEB_PATH_SEPARATOR, 1, 1);
			} else {
				echo nl2br(SQ_CONF_WEB_PATH_SEPARATOR);
			}
			$o->closeField();
			
			$o->openField(translate('system_redirect_url_with_trailing_slash'));
				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_REDIRECT_URL_WITH_TRAILING_SLASH]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_CONF_REDIRECT_URL_WITH_TRAILING_SLASH);
				} else {
					echo (SQ_CONF_REDIRECT_URL_WITH_TRAILING_SLASH) ? translate('yes') : translate('no');
				}
			$o->note(translate('system_redirect_url_with_trailing_slash_note'));
			$o->closeField();

		$o->closeSection();

		$o->openSection(translate('rollback_settings'));

			$o->openField(translate('rollback_enabled'));
				echo (SQ_CONF_ROLLBACK_ENABLED) ? translate('yes') : translate('no');
				$o->note(translate('must_enable_rollback'));
			$o->closeField();

		$o->closeSection();

		$o->openSection(translate('email_settings'));

			$o->openField(translate('default_email'));
				if ($write_access) {
					text_box($class.'[SQ_CONF_DEFAULT_EMAIL]', SQ_CONF_DEFAULT_EMAIL, 50);
				} else {
					echo SQ_CONF_DEFAULT_EMAIL;
				}
			$o->closeField();

			$o->openField(translate('tech_email'));
				if ($write_access) {
					text_box($class.'[SQ_CONF_TECH_EMAIL]', SQ_CONF_TECH_EMAIL, 50);
				} else {
					echo SQ_CONF_TECH_EMAIL;
				}
			$o->closeField();

		$o->closeSection();

		$o->openSection(translate('login/session_settings'));

			$o->openField(translate('force_secure_login_urls'));
				$root_urls = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);
				$force_secure_urls = explode("\n", SQ_CONF_FORCE_SECURE_LOGIN_URLS);
				?>
				<table class="sq-backend-table" style="width: auto">
					<thead>
						<tr>
							<th><?php echo translate('url'); ?></th>
							<th><?php echo translate('require_secure_login'); ?></th>
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
			$o->note(translate('force_secure_login_urls_note'));

			$o->openField(translate('max_login_attempts'));
			if ($write_access && $is_root) {
				text_box($class.'[SQ_CONF_MAX_LOGIN_ATTEMPTS]', SQ_CONF_MAX_LOGIN_ATTEMPTS, 5, 5);
			} else {
				echo SQ_CONF_MAX_LOGIN_ATTEMPTS;
			}
			$o->note(translate('0_is_unlimited_attempts'));

			$o->openField(translate('allow_ip_change'));
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_CONF_ALLOW_IP_CHANGE]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_CONF_ALLOW_IP_CHANGE);
			} else {
				echo (SQ_CONF_ALLOW_IP_CHANGE) ? translate('yes') : translate('no');
			}
			$o->note(translate('allow_user_ip_to_fluctuate'));

			$o->openField(translate('process_php_credentials'));
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_CONF_ACCEPT_HTTP_CREDS]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_CONF_ACCEPT_HTTP_CREDS);
			} else {
				echo (SQ_CONF_ACCEPT_HTTP_CREDS) ? translate('yes') : translate('no');
			}
			$o->note(translate('process_php_credentials_note'));

			$o->openField(translate('enable_http_authentication'));
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_CONF_ENABLE_HTTP_LOGIN]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_CONF_ENABLE_HTTP_LOGIN);
			} else {
				echo (SQ_CONF_ENABLE_HTTP_LOGIN) ? translate('yes') : translate('no');
			}
			$o->note(translate('enable_http_authentication_note'));

			$o->openField(translate('allow_single_sign_in'));
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_CONF_ALLOW_HTTP_LOGIN]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_CONF_ALLOW_HTTP_LOGIN);
			} else {
				echo (SQ_CONF_ALLOW_HTTP_LOGIN) ? translate('yes') : translate('no');
			}
			$o->note(translate('allow_single_sign_in_note'));

			$o->openField(translate('http_login_var'));
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
				<span id="sq-conf-http-login-var-toggle" class="clickable" onclick=toggle_http_login_var();><?php echo translate('http_login_var_show_server_var');?></span>
				<div id="sq-conf-http-login-var-list" style="display:none;">
					<table class="sq-backend-table" width="400">
						<tr>
							<th colspan="2">
							<?php echo translate('http_login_var_list_header');?>
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

			$o->openField(translate('enable_external_auth_systems'));
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_CONF_ENABLE_EXTERNAL_AUTH_SYSTEMS]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_CONF_ENABLE_EXTERNAL_AUTH_SYSTEMS);
			} else {
				echo (SQ_CONF_ENABLE_EXTERNAL_AUTH_SYSTEMS) ? translate('yes') : translate('no');
			}
			$o->note(translate('enable_external_auth_systems_note'));

			$o->openField(translate('choose_session_save_path'));
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_CONF_USE_DEFAULT_SESSION_SAVE_PATH]', Array('1' => translate('true'), '0' => translate('false')), FALSE, SQ_CONF_USE_DEFAULT_SESSION_SAVE_PATH);
			} else {
				echo (SQ_CONF_USE_DEFAULT_SESSION_SAVE_PATH) ? translate('true') : translate('false');
			}
			$o->note(translate('choose_session_save_path_note', session_save_path()));

		$o->closeSection();

		$o->openSection(translate('intervals'));

			$o->openField(translate('refresh_interval'));
			if ($write_access) {
				text_box($class.'[SQ_CONF_REFRESH_INTERVAL]', SQ_CONF_REFRESH_INTERVAL, 5, 5);
			} else {
				echo SQ_CONF_REFRESH_INTERVAL;
			}
			$o->note(translate('header_frame_mail_lock'));

			$o->openField(translate('lock_length'));
			if ($write_access) {
				text_box($class.'[SQ_CONF_LOCK_LENGTH]', SQ_CONF_LOCK_LENGTH, 5, 5);
			} else {
				echo SQ_CONF_LOCK_LENGTH;
			}
			$o->closeField(translate('lock_length_time'));

		$o->closeSection();

		$o->openSection(translate('php_config_settings'));

			$o->openField(translate('php_config_web_memory_limit'));
			if ($write_access && $is_root) {
				int_text_box($class.'[SQ_CONF_WEB_MEMORY_LIMIT]', SQ_CONF_WEB_MEMORY_LIMIT, FALSE, 5, NULL, NULL, 3);
				echo ' MB';
			} else {
				echo SQ_CONF_WEB_MEMORY_LIMIT.'MB';
			}
			$o->note(translate('php_config_web_memory_limit_note'));
			$o->closeField();

			$o->openField(translate('php_config_cron_memory_limit'));
			if ($write_access && $is_root) {
				int_text_box($class.'[SQ_CONF_CRON_MEMORY_LIMIT]', SQ_CONF_CRON_MEMORY_LIMIT, FALSE, 5, NULL, NULL, 3);
				echo ' MB';
			} else {
				echo SQ_CONF_CRON_MEMORY_LIMIT.'MB';
			}
			$o->note(translate('php_config_cron_memory_limit_note'));
			$o->closeField();

		$o->closeSection();


		$o->openSection(translate('error/debug_settings'));

			$o->openField(translate('log_errors'));
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_CONF_LOG_ERRORS]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_CONF_LOG_ERRORS);
			} else {
				echo (SQ_CONF_LOG_ERRORS) ? translate('yes') : translate('no');
			}
			$o->closeField();

			$o->openField(translate('hide_errors_on_the_frontend'));
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_CONF_ERRORS_HIDE_FRONTEND]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_CONF_ERRORS_HIDE_FRONTEND);
			} else {
				echo (SQ_CONF_ERRORS_HIDE_FRONTEND) ? translate('yes') : translate('no');
			}
			$o->closeField();

			$o->openField(translate('log_errors_to_syslog'));
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_CONF_ERRORS_LOG_TO_SYSLOG]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_CONF_ERRORS_LOG_TO_SYSLOG);
			} else {
				echo (SQ_CONF_ERRORS_LOG_TO_SYSLOG) ? translate('yes') : translate('no');
			}
			$o->note(translate('log_errors_to_syslog_note'));
			$o->closeField();

			$o->openField(translate('log_errors_syslog_facility'));
			// We only provide the common
			$options = Array(
						'user'		=> translate('syslog_facility_user'),
						'local0'	=> translate('syslog_facility_local', 0),
						'local1'	=> translate('syslog_facility_local', 1),
						'local2'	=> translate('syslog_facility_local', 2),
						'local3'	=> translate('syslog_facility_local', 3),
						'local4'	=> translate('syslog_facility_local', 4),
						'local5'	=> translate('syslog_facility_local', 5),
						'local6'	=> translate('syslog_facility_local', 6),
						'local7'	=> translate('syslog_facility_local', 7),
					   );
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_CONF_ERRORS_SYSLOG_FACILITY]', $options, FALSE, SQ_CONF_ERRORS_SYSLOG_FACILITY);
			} else {
				echo $options[SQ_CONF_ERRORS_SYSLOG_FACILITY];
			}

			$o->openField(translate('debug_settings'));
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

		$o->openSection(translate('internationalisation_settings'));

			$o->openField(translate('default_frontend_language'));
			require SQ_FUDGE_PATH.'/standards_lists/languages.inc';
			if ($write_access) {
				combo_box($class.'[SQ_CONF_DEFAULT_FRONTEND_LANGUAGE]', $standards_lists_languages, FALSE, SQ_CONF_DEFAULT_FRONTEND_LANGUAGE);
			} else {
				echo $standards_lists_languages[SQ_CONF_DEFAULT_FRONTEND_LANGUAGE];
			}

			$o->openField(translate('default_backend_locale'));
			require SQ_FUDGE_PATH.'/standards_lists/locales.inc';
			// default to English (Australia) in upgraded systems
			$current_locale = !defined('SQ_CONF_DEFAULT_BACKEND_LOCALE') ? 'en_AU' : SQ_CONF_DEFAULT_BACKEND_LOCALE;
			if ($write_access) {
				combo_box($class.'[SQ_CONF_DEFAULT_BACKEND_LOCALE]', $standards_lists_locales, FALSE, $current_locale);
			} else {
				echo $standards_lists_locales[$current_locale];
			}

			$o->openField(translate('default_character_set'));
			require SQ_FUDGE_PATH.'/standards_lists/character_sets.inc';
			echo $standards_lists_character_sets[SQ_CONF_DEFAULT_CHARACTER_SET];

			$o->openField(translate('use_lang_character_map'));
			if ($write_access) {
				$options = Array(
							'0'	=> translate('no'),
							'1'	=> translate('yes'),
						   );
				combo_box($class.'[SQ_CONF_USE_LANG_CHAR_MAP]', $options, FALSE, !defined('SQ_CONF_USE_LANG_CHAR_MAP') ? '0' : SQ_CONF_USE_LANG_CHAR_MAP);
			} else {
				echo (SQ_CONF_USE_LANG_CHAR_MAP) ? translate('yes') : translate('no');
			}

		$o->closeSection();

		$o->openSection(translate('editing_interface_settings'));

			$o->openField(translate('commit_button_text'));
			if ($write_access) {
				text_box($class.'[SQ_CONF_COMMIT_BUTTON_TEXT]', !defined('SQ_CONF_COMMIT_BUTTON_TEXT') ? 'Commit' : SQ_CONF_COMMIT_BUTTON_TEXT, 30);
			} else {
				echo !defined('SQ_CONF_COMMIT_BUTTON_TEXT') ? 'Commit' : SQ_CONF_COMMIT_BUTTON_TEXT;
			}
			$o->note(translate('rename_commit_button'));
			$o->closeField();

			$o->openField(translate('confirm_save_changes'));
			if ($write_access) {
				$options = Array(
							'0'	=> translate('no'),
							'1'	=> translate('yes'),
						   );
				combo_box($class.'[SQ_CONF_CONFIRM_SAVE_CHANGES]', $options, FALSE, !defined('SQ_CONF_CONFIRM_SAVE_CHANGES') ? '1' : SQ_CONF_CONFIRM_SAVE_CHANGES);
			} else {
				if (defined('SQ_CONF_CONFIRM_SAVE_CHANGES')) {
					?>
					<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo (SQ_CONF_CONFIRM_SAVE_CHANGES) ? 'tick' : 'cross'; ?>.gif" width="15" height="15" />
					<?php
				} else {
					echo translate('not_set');
				}//end if
			}//end if
			$o->note(translate('confirm_save_changes_msg'));
			$o->closeField();

		$o->closeSection();

		$o->openSection(translate('asset_map_settings'));

			$o->openField(translate('asset_limit_per_node'));
			if ($write_access) {
				text_box($class.'[SQ_CONF_ASSET_MAP_ASSET_LIMIT]', !defined('SQ_CONF_ASSET_MAP_ASSET_LIMIT') ? '50' : SQ_CONF_ASSET_MAP_ASSET_LIMIT, 30);
			} else {
				echo !defined('SQ_CONF_ASSET_MAP_ASSET_LIMIT') ? '50' : SQ_CONF_ASSET_MAP_ASSET_LIMIT;
			}
			$o->note(translate('asset_limit_per_node_note'));
			$o->closeField();

			$o->openField(translate('asset_display_name'));
			if ($write_access) {
				text_box($class.'[SQ_CONF_ASSET_MAP_ASSET_DISPLAY_NAME]', !defined('SQ_CONF_ASSET_MAP_ASSET_DISPLAY_NAME') ?  $this->config_vars['SQ_CONF_ASSET_MAP_ASSET_DISPLAY_NAME']['default']  : SQ_CONF_ASSET_MAP_ASSET_DISPLAY_NAME, 50);
			} else {
				echo !defined('SQ_CONF_ASSET_MAP_ASSET_DISPLAY_NAME') ? $this->config_vars['SQ_CONF_ASSET_MAP_ASSET_DISPLAY_NAME']['default'] : SQ_CONF_ASSET_MAP_ASSET_DISPLAY_NAME;
			}
			$o->note(translate('asset_display_name_note'));
			$o->closeField();

		$o->closeSection();

		$o->openSection(translate('http_header_section_name'));

			$o->openField(translate('http_header_cacheable_field'));
				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_SEND_CACHEABLE_HEADER]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_CONF_SEND_CACHEABLE_HEADER);
				} else {
					echo (SQ_CONF_SEND_CACHEABLE_HEADER) ? translate('yes') : translate('no');
				}
				$o->note(translate('http_header_cacheable_field_note'));
			$o->closeField();

			$o->openField(translate('http_header_last_modified_field'));
				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_SEND_LAST_MODIFIED_HEADER]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_CONF_SEND_LAST_MODIFIED_HEADER);
				} else {
					echo (SQ_CONF_SEND_LAST_MODIFIED_HEADER) ? translate('yes') : translate('no');
				}
				$o->note(translate('http_header_last_modified_field_note'));
			$o->closeField();

			$o->openField(translate('http_header_not_modified_field'));
				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_SEND_NOT_MODIFIED_HEADER]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_CONF_SEND_NOT_MODIFIED_HEADER);
				} else {
					echo (SQ_CONF_SEND_NOT_MODIFIED_HEADER) ? translate('yes') : translate('no');
				}
				$o->note(translate('http_header_not_modified_field_note'));
				$o->note(translate('http_header_conditional_note'));
			$o->closeField();

			$o->openField(translate('http_header_send_no_cache_header_field'));
				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_SEND_NO_CACHE_HEADER]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_CONF_SEND_NO_CACHE_HEADER);
				} else {
					echo (SQ_CONF_SEND_NO_CACHE_HEADER) ? translate('yes') : translate('no');
				}
				$o->note(translate('http_header_send_no_cache_header_field_note'));
			$o->closeField();

			$o->openField(translate('http_header_send_404_cacheable_header_field'));
				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_SEND_404_CACHEABLE_HEADER]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_CONF_SEND_404_CACHEABLE_HEADER);
				} else {
					echo (SQ_CONF_SEND_404_CACHEABLE_HEADER) ? translate('yes') : translate('no');
				}
				$o->note(translate('http_header_send_404_cacheable_header_field_note'));
			$o->closeField();
			
			$o->openField(translate('http_header_use_http_x_forwarded_for'));
				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_USE_HTTP_X_FORWARDED_FOR]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_CONF_USE_HTTP_X_FORWARDED_FOR);
				} else {	
					echo (SQ_CONF_USE_HTTP_X_FORWARDED_FOR) ? translate('yes') : translate('no');
				}
				$o->note(translate('http_header_use_http_x_forwarded_for_note'));
			
				$option_list->setEditParam('width', 30);
				if ($write_access) {
					$option_list->value = SQ_CONF_FORWARDED_IP_PROXY_LIST;
					$option_list->paint('SQ_CONF_FORWARDED_IP_PROXY_LIST');
				} else {
					if (SQ_CONF_FORWARDED_IP_PROXY_LIST == '') {
						echo translate('none');
					} else {
						echo nl2br(SQ_CONF_FORWARDED_IP_PROXY_LIST);
					}
				}
			$o->closeField();

			$o->openField(translate('http_header_cookie_http_only_field_name'));
				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_COOKIE_OPTION_HTTP_ONLY]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_CONF_COOKIE_OPTION_HTTP_ONLY);
				} else {	
					echo (SQ_CONF_COOKIE_OPTION_HTTP_ONLY) ? translate('yes') : translate('no');
				}
				$o->note(translate('http_header_cookie_http_only_note'));
			
			$o->closeField();

			$o->openField(translate('http_header_cookie_secure_option_field_name'));
				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_COOKIE_OPTION_SECURE]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_CONF_COOKIE_OPTION_SECURE);
				} else {	
					echo (SQ_CONF_COOKIE_OPTION_SECURE) ? translate('yes') : translate('no');
				}
				$o->note(translate('http_header_cookie_secure_option_note'));
			
			$o->closeField();

		
		$o->closeSection();


		$o->openSection(translate('roles_configuration_section_name'));

			$o->openField(translate('roles_configuration_enable_roles_perm_system'));
				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_ENABLE_ROLES_PERM_SYSTEM]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_CONF_ENABLE_ROLES_PERM_SYSTEM);
				} else {
					echo (SQ_CONF_ENABLE_ROLES_PERM_SYSTEM) ? translate('yes') : translate('no');
				}
			$o->closeField();

			$o->openField(translate('roles_configuration_enable_roles_wf_system'));
				if ($write_access && $is_root) {
					combo_box($class.'[SQ_CONF_ENABLE_ROLES_WF_SYSTEM]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_CONF_ENABLE_ROLES_WF_SYSTEM);
				} else {
					echo (SQ_CONF_ENABLE_ROLES_WF_SYSTEM) ? translate('yes') : translate('no');
				}
			$o->closeField();

			if ( ((defined('SQ_CONF_ENABLE_ROLES_PERM_SYSTEM') && (SQ_CONF_ENABLE_ROLES_PERM_SYSTEM == '1')) || (defined('SQ_CONF_ENABLE_ROLES_WF_SYSTEM') && (SQ_CONF_ENABLE_ROLES_WF_SYSTEM == '1')) ) || (!defined('SQ_CONF_ENABLE_ROLES_PERM_SYSTEM') && !defined('SQ_CONF_ENABLE_ROLES_WF_SYSTEM') )) {
				$o->openField(translate('roles_configuration_enable_global_roles'));
					if ($write_access && $is_root) {
						combo_box($class.'[SQ_CONF_ENABLE_GLOBAL_ROLES]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_CONF_ENABLE_GLOBAL_ROLES);
					} else {
						echo (SQ_CONF_ENABLE_GLOBAL_ROLES) ? translate('yes') : translate('no');
					}
				$o->closeField();
			}

		$o->closeSection();

		$o->openSection(translate('search_engine_optimizations'));

			$o->openField(translate('remove_self_links'));
				if ($write_access) {
					$options = Array(
								'0'	=> translate('no'),
								'1'	=> translate('yes'),
							   );
					combo_box($class.'[SQ_CONF_NO_ASSET_SELF_LINKS]', $options, FALSE, !defined('SQ_CONF_NO_ASSET_SELF_LINKS') ? '0' : SQ_CONF_NO_ASSET_SELF_LINKS);
				} else {
					echo (SQ_CONF_NO_ASSET_SELF_LINKS) ? translate('yes') : translate('no');
				}
				$o->note(translate('remove_self_links_note'));
			$o->closeField();

		$o->closeSection();

		$o->openSection(translate('miscellaneous_config'));
			$o->openField(translate('visited_pages_config_max_entries'));
			if ($write_access && $is_root) {
				text_box($class.'[SQ_CONF_VISITED_PAGES_MAX_ENTRIES]', !defined('SQ_CONF_VISITED_PAGES_MAX_ENTRIES') ? '10' : SQ_CONF_VISITED_PAGES_MAX_ENTRIES, 30);
			} else {
				echo !defined('SQ_CONF_VISITED_PAGES_MAX_ENTRIES') ? '10' : SQ_CONF_VISITED_PAGES_MAX_ENTRIES;
			}
				$o->note(translate('visited_pages_config_note'));
			$o->closeField();
		$o->closeSection();

		$o->openSection(translate('install_key'));

			$o->openField(translate('install_key'));
				echo SQ_LICENCE_INSTALL_KEY;
				$o->note(translate('system_config_install_key_note'));
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
