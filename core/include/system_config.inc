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
* $Id: system_config.inc,v 1.88 2006/11/14 04:02:22 bcaldwell Exp $
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
* @version $Revision: 1.88 $
* @package MySource_Matrix
*/
class System_Config extends Config
{

	/**
	* Holds the definitions for all the constants managed by this configuration object
	* In the form : Array([var name] => Array('editable' => [boolean], 'default' => [mixed scalar]))
	* @var array
	*/
	var $config_vars = Array(
						'SQ_CONF_DB_DSN'						=> Array('editable' => 1, 'default' => ''),
						'SQ_CONF_DB2_DSN'						=> Array('editable' => 1, 'default' => ''),
						'SQ_CONF_DB3_DSN'						=> Array('editable' => 1, 'default' => ''),
						'SQ_CONF_DBCACHE_DSN'					=> Array('editable' => 1, 'default' => ''),
						'SQ_CONF_PEAR_PATH'						=> Array('editable' => 0, 'default' => ''),
						'SQ_CONF_BACKEND_SUFFIX'				=> Array('editable' => 1, 'default' => '_admin'),
						'SQ_CONF_LIMBO_SUFFIX'					=> Array('editable' => 1, 'default' => '_edit'),
						'SQ_CONF_NOCACHE_SUFFIX'				=> Array('editable' => 1, 'default' => '_nocache'),
						'SQ_CONF_ASSET_TREE_BASE'				=> Array('editable' => 0, 'default' => 64),
						'SQ_CONF_ASSET_TREE_SIZE'				=> Array('editable' => 0, 'default' =>  4),
						'SQ_CONF_ASSET_CACHE_SIZE_WEB'			=> Array('editable' => 0, 'default' =>  -1),
						'SQ_CONF_ASSET_CACHE_SIZE_CLI'			=> Array('editable' => 0, 'default' =>  -1),
						'SQ_CONF_ROLLBACK_ENABLED'				=> Array('editable' => 0, 'default' => '0'),
						'SQ_CONF_DEBUG'							=> Array('editable' => 1, 'default' => 0),
						'SQ_CONF_SYSTEM_NAME'					=> Array('editable' => 1, 'default' => 'The System'),
						'SQ_CONF_SYSTEM_OWNER'					=> Array('editable' => 1, 'default' => ''),
						'SQ_CONF_SYSTEM_ROOT_URLS'				=> Array('editable' => 1, 'default' => ''),
						'SQ_CONF_SYSTEM_PARENT_DOMAINS'			=> Array('editable' => 1, 'default' => ''),
						'SQ_CONF_STATIC_ROOT_URL'				=> Array('editable' => 1, 'default' => ''),
						'SQ_CONF_WEB_PATH_SEPARATOR'			=> Array('editable'	=> 1, 'default'	=> '_'),
						'SQ_CONF_STATIC_ROOT_HTTP'				=> Array('editable' => 1, 'default' => TRUE),
						'SQ_CONF_STATIC_ROOT_HTTPS'				=> Array('editable' => 1, 'default' => FALSE),
						'SQ_CONF_DEFAULT_EMAIL'					=> Array('editable' => 1, 'default' => ''),
						'SQ_CONF_TECH_EMAIL'					=> Array('editable' => 1, 'default' => ''),
						'SQ_CONF_MAX_LOGIN_ATTEMPTS'			=> Array('editable' => 1, 'default' => 3),
						'SQ_CONF_REFRESH_INTERVAL'				=> Array('editable' => 1, 'default' => 120),
						'SQ_CONF_LOCK_LENGTH'					=> Array('editable' => 1, 'default' => 600),
						'SQ_CONF_ASSET_MAP_ASSET_LIMIT'			=> Array('editable' => 1, 'default' => 50),
						'SQ_CONF_ASSET_MAP_ASSET_DISPLAY_NAME'	=> Array('editable' => 1, 'default' => '%asset_short_name%'),
						'SQ_CONF_SEND_CACHEABLE_HEADER'			=> Array('editable' => 1, 'default' => '1'),
						'SQ_CONF_SEND_LAST_MODIFIED_HEADER'		=> Array('editable' => 1, 'default' => '0'),
						'SQ_CONF_SEND_NOT_MODIFIED_HEADER'		=> Array('editable' => 1, 'default' => '0'),
						'SQ_CONF_ERRORS_HIDE_FRONTEND'			=> Array('editable' => 1, 'default' => FALSE),
						'SQ_CONF_DEFAULT_FRONTEND_LANGUAGE'		=> Array('editable' => 1, 'default' => 'en'),
						'SQ_CONF_DEFAULT_BACKEND_LOCALE'		=> Array('editable' => 1, 'default' => 'en_AU'),
						'SQ_CONF_DEFAULT_CHARACTER_SET'			=> Array('editable' => 1, 'default' => 'iso-8859-1'),
						'SQ_CONF_ALLOW_IP_CHANGE'				=> Array('editable' => 1, 'default' => '0'),
						'SQ_CONF_ALLOW_HTTP_LOGIN'				=> Array('editable' => 1, 'default' => '0'),
						'SQ_CONF_ENABLE_EXTERNAL_AUTH_SYSTEMS'	=> Array('editable' => 1, 'default' => TRUE),
						'SQ_CONF_HTTP_LOGIN_VAR'				=> Array('editable' => 1, 'default' => 'REMOTE_USER'),
						'SQ_CONF_ACCEPT_HTTP_CREDS'				=> Array('editable' => 1, 'default' => '0'),
						'SQ_CONF_COMMIT_BUTTON_TEXT'			=> Array('editable' => 1, 'default' => 'Commit'),
						'SQ_CONF_LOG_EXTENSION'					=> Array('editable' => 0, 'default' => '.log'),
						'SQ_CONF_LOG_FILE_SYSTEM'				=> Array('editable' => 0, 'default' => 'system'),
						'SQ_CONF_LOG_FILE_ERROR'				=> Array('editable' => 0, 'default' => 'error'),
						'SQ_CONF_LOG_ERRORS'					=> Array('editable' => 1, 'default' => TRUE),
						'SQ_CONF_SESSION_GC_MAXLIFETIME'		=> Array('editable' => 1, 'default'	=> 604800),
						'SQ_CONF_EDITING_TIME'					=> Array('editable' => 0, 'default' => 3600),
						'SQ_CONF_NUM_DATA_DIRS'					=> Array('editable' => 0, 'default' => 20),
						'SQ_CONF_FORCE_SECURE_LOGIN_URLS'		=> Array('editable' => 1, 'default' => ''),
					   );

	/**
	* Holds all the valid debug settings and their descriptions for SQ_CONF_DEBUG
	* The keys are the bit elements and the values are the descriptions
	* @var array
	*/
	// TODO: TRANSLATE: these need to be either moved somewhere else, or implement some sort of global code
	var $debug_options = Array(
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
	function System_Config()
	{
		$this->Config();
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
	function save($vars, $backup_existing=FALSE, $send_message=TRUE)
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
					unset($vars['SQ_CONF_BACKEND_SUFFIX']);
					unset($vars['SQ_CONF_LIMBO_SUFFIX']);
					unset($vars['SQ_CONF_ROLLBACK_ENABLED']);
				}

				if (isset($vars['SQ_CONF_FORCE_SECURE_LOGIN_URLS'])) {
					$vars['SQ_CONF_FORCE_SECURE_LOGIN_URLS'] = implode("\n", $vars['SQ_CONF_FORCE_SECURE_LOGIN_URLS']);
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
	* @access private
	*/
	function _paintConfigVar($var_name, $value, $var_export=TRUE)
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
	function canAcquireLock()
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
	function writeAccess()
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
	function paintBackend(&$o)
	{
		parent::paintBackend($o);

		$write_access = $this->writeAccess();

		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();

		$class = get_class($this);

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

			$o->openField(translate('system_nocache_suffix'));
			if ($write_access) {
				text_box($class.'[SQ_CONF_NOCACHE_SUFFIX]', SQ_CONF_NOCACHE_SUFFIX, 15);
			} else {
				echo nl2br(SQ_CONF_NOCACHE_SUFFIX);
			}

		$o->closeSection();

		$o->openSection(translate('system_urls'));

			require_once SQ_ATTRIBUTES_PATH.'/option_list/option_list.inc';

			$option_list =& new Asset_Attribute_Option_List(0, NULL, SQ_OPTION_LIST_DELIMITER_UNIX);
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

			$o->openField(translate('allow_single_sign_in'));
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_CONF_ALLOW_HTTP_LOGIN]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_CONF_ALLOW_HTTP_LOGIN);
			} else {
				echo (SQ_CONF_ALLOW_HTTP_LOGIN) ? translate('yes') : translate('no');
			}
			$o->note(translate('allow_single_sign_in_note'));

			$o->openField(translate('http_login_var'));
			if ($write_access && $is_root) {
				text_box($class.'[SQ_CONF_HTTP_LOGIN_VAR]', SQ_CONF_HTTP_LOGIN_VAR, 40);
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


		$o->openSection(translate('error/debug_settings'));

			$o->openField(translate('log_errors'));
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_CONF_LOG_ERRORS]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_CONF_LOG_ERRORS);
			} else {
				echo (SQ_CONF_LOG_ERRORS) ? translate('yes') : translate('no');
			}

			$o->openField(translate('hide_errors_on_the_frontend'));
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_CONF_ERRORS_HIDE_FRONTEND]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_CONF_ERRORS_HIDE_FRONTEND);
			} else {
				echo (SQ_CONF_ERRORS_HIDE_FRONTEND) ? translate('yes') : translate('no');
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
					echo $desc, "<br />\n";
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
			if ($write_access) {
				combo_box($class.'[SQ_CONF_DEFAULT_CHARACTER_SET]', $standards_lists_character_sets, FALSE, SQ_CONF_DEFAULT_CHARACTER_SET);
			} else {
				echo $standards_lists_character_sets[SQ_CONF_DEFAULT_CHARACTER_SET];
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
		$o->closeSection();

		if ($write_access) $o->commitButton('', TRUE);

	}//end paintBackend()


	/**
	* Saves the config settings that were submitted
	*
	* @param object Backend_Outputter	&$o			reference to the backend outputter
	*
	* @return boolean
	* @access public
	* @see paintBackend()
	*/
	function processBackend(&$o)
	{
		$write_access = $this->writeAccess();
		if (!$write_access) return parent::processBackend($o);

		$class = get_class($this);

		require_once SQ_ATTRIBUTES_PATH.'/option_list/option_list.inc';
		$option_list =& new Asset_Attribute_Option_List(0, NULL, SQ_OPTION_LIST_DELIMITER_UNIX);

		$list_settings = Array(
							'SQ_CONF_SYSTEM_ROOT_URLS',
							'SQ_CONF_SYSTEM_PARENT_DOMAINS',
						 );

		foreach ($list_settings as $list_setting) {
			$option_list->process($list_setting);
			$_POST[$class][$list_setting] = $option_list->value;
		}
		return parent::processBackend($o);

	}//end processBackend()



}//end class

?>
