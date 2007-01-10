<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: mysource.inc,v 1.205.2.10 2007/01/10 01:02:55 lwright Exp $
*
*/

require_once 'DB.php';
require_once SQ_INCLUDE_PATH.'/mysource_object.inc';
require_once SQ_INCLUDE_PATH.'/asset_manager.inc';
require_once SQ_INCLUDE_PATH.'/locale_manager.inc';

/**
* MySource
*
* Purpose
*    This is the object from which everything else is run
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.205.2.10 $
* @package MySource_Matrix
*/
class MySource
{
	/**
	* The current DB object
	*
	* @var object DB
	*/
	var $db;

	/**
	* Holds all open DB objects
	*
	* @var array
	* @see changeDatabaseConnection(), restoreDatabaseConnection()
	*/
	var $_db_conns = Array();

	/**
	* The stack for DB connections
	*
	* @var array
	* @see changeDatabaseConnection(), restoreDatabaseConnection()
	*/
	var $_db_stack = Array();


	/**
	* The stack for run levels
	*
	* @var array
	* @see setRunLevel(), restoreRunLevel()
	*/
	var $_run_level_stack = Array();

	/**
	* The current Backend object
	*
	* @var object Backend
	*/
	var $backend;

	/**
	* @var object Asset_Manager
	*/
	var $am;

	/**
	* @var object Transaction_Manager
	*/
	var $tm;

	/**
	* @var object Messaging_Service
	*/
	var $ms;

	/**
	* @var object Metadata_Manager
	*/
	var $mm;

	/**
	* @var object HIPO_Herder
	*/
	var $hh;

	/**
	* @var object Workflow_Manager
	*/
	var $wm;

	/**
	* @var object File_Versioning
	*/
	var $fv;

	/**
	* @var object Event_Manager
	*/
	var $em;

	/**
	* @var object Tag_Manager
	*/
	var $tag_manager;

	/**
	* @var object Locale_Manager
	*/
	var $lm;

	/**
	* @var object Trigger_Manager
	*/
	var $trigger_manager;

	/**
	* @var object Log_Manager
	*/
	var $log_manager;

	/**
	* The logged in user
	*
	* @var object User
	*/
	var $user = NULL;

	/**
	* The object of the (main) asset being printed on the frontend
	*
	* @var object Asset
	*/
	var $frontend_asset;

	/**
	* Whether the logged in user is the root user or not
	*
	* @var boolean
	*/
	var $_user_is_root = FALSE;

	/**
	* Whether the logged in user is a system admin or not
	*
	* @var boolean
	*/
	var $_user_is_sys_admin = FALSE;

	/**
	* Whether the person using the system is a logged in user or not
	*
	* @var boolean
	*/
	var $_user_is_public = TRUE;


	/**
	* Array of system messages
	*
	* @var array(string)
	*/
	var $_msgs;


	/**
	* Array of globally defined variables
	*
	* @var array(string)
	*/
	var $_global_defines;


	/**
	* Constructor
	*
	*/
	function MySource()
	{

	}//end constructor


	/**
	* This initialises all the objects that we need to use, the reason that we can't do
	* this in the constructor is that the Asset_Manager uses the DB in it's constructor
	* in order to load the asset types but if initialise the Asset_Manager in our constructor
	* we don't exist and therefore cannot be accessed to get the DB for the Asset_Manager to use
	*
	* @return void
	* @access public
	*/
	function init()
	{
		$this->changeDatabaseConnection('db');

		// initialise the asset manager as we are going to need it pretty much everywhere
		$this->am =& new Asset_Manager();

		// Same with the locale manager; translations will be all around us
		$this->lm =& new Locale_Manager();

		$this->lm->setCurrentLocale(SQ_CONF_DEFAULT_BACKEND_LOCALE);

		if (!SQ_PHP_CLI && $GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS)) {
			// we are accessing from the web, and the security subsystem is enabled in this run level,
			// so we use cookies for security

			$proxy_conf_file = SQ_DATA_PATH.'/private/conf/proxy_authentication.inc';
			if (file_exists($proxy_conf_file)) {
				// proxy configuration exists, let's see if we need to do anything
				require_once($proxy_conf_file);

				if (SQ_PROXY_URL_PORT_STRIP) {
					$url_parts = parse_url($_SERVER['HTTP_HOST']);

					// remove port information from url and reconstruct it
					if (isset($url_parts['port'])) unset($url_parts['port']);
					$_SERVER['HTTP_HOST'] = implode('', $url_parts);
				}
			}

			$domain = strip_url(sq_web_path('root_url'), TRUE);
			$pos = strpos($domain, '/');
			$path = '/';
			if ($pos !== FALSE) {
				$path = substr($domain, $pos);
				$domain = substr($domain, 0, $pos);
			}

			// if there is a port at the end of the domain, we need to remove
			// it as domains cannot contain ports. You will not be able to log
			// in if the domain contains a port
			$domain = preg_replace('|:\d+$|', '', $domain);

			session_set_cookie_params(0, $path, $domain);
			session_name('SQ_SYSTEM_SESSION');
			session_save_path(SQ_SYSTEM_ROOT.'/cache');
			ini_set('session.gc_maxlifetime', SQ_CONF_SESSION_GC_MAXLIFETIME);

			$session_set = FALSE;
			if (isset($_GET['SESSION_KEY']) && isset($_GET['SESSION_ID'])) {
				// asset map or someone has sent session key through the GET
				session_id($_GET['SESSION_ID']);
				$session_set = TRUE;
			}
			session_start();

			// check to see if the session key is the same as that used to login
			if ($session_set && $this->getUniqueSessionKey() != $_GET['SESSION_KEY']) {
				define('SQ_ROLLBACK_VIEW', '0');
				define('SQ_TABLE_RUNNING_PREFIX', 'sq_');
				$ms =& $this->getMessagingService();
				$log = $ms->newMessage(Array(), 'system.security.alert', Array());
				$log->parameters['remote_addr'] = $_SERVER['REMOTE_ADDR'];
				$log->parameters['sessionid'] = session_id();
				$log->parameters['userid'] = $_SESSION['userid'];
				$log->send();
				exit();
			}

			// process rollback view mode global actions here so setting the rollback vars below
			if (!empty($_REQUEST['SQ_ACTION']) && strpos($_REQUEST['SQ_ACTION'], 'rollback_view') === 0) {
				$this->_processGlobalActions();
			}

			// check if we are in rollback view mode
			if (!empty($_SESSION['sq_rollback_view']) && (!isset($_REQUEST['SQ_ACTION']) || $_REQUEST['SQ_ACTION'] != 'logout')) {
				// we are viewing the system at some time in the past
				define('SQ_ROLLBACK_VIEW',        '1');
				define('SQ_TABLE_RUNNING_PREFIX', 'sq_rb_');

				// we need to let the database know what the current timestamp that we
				// are using for rollback so that and stored procedures that require
				// the rollback timestamp can acquire it
				$then = $_SESSION['sq_rollback_view']['rollback_time'];
				$db   =& $GLOBALS['SQ_SYSTEM']->db;

				if ($db->phptype == 'pgsql') {
					$sql  = 'SELECT sq_set_rollback_timestamp('.$db->quoteSmart($then).')';
				} else if ($db->phptype == 'oci8') {
					$sql = 'DECLARE BEGIN sq_common_pkg.sq_set_rollback_timestamp('.$db->quoteSmart($then).'); END;';
				}
				$result = $db->query($sql);
				assert_valid_db_result($result);
			} else {
				define('SQ_ROLLBACK_VIEW', '0');
				define('SQ_TABLE_RUNNING_PREFIX', 'sq_');
			}

			$this->setupUser();
			$this->_updateSessionCookie();
			$_SESSION['user_last_access'] = time();


			// if we are using the 'a' querystring arg, remove any trailing exclamation marks
			// that are used as termination characters in different kinds of content
			if (isset($_REQUEST['a'])) {
				$_REQUEST['a'] = rtrim($_REQUEST['a'], '$');
			}

		} else {

			// we are either in CLI mode or the run level doesn't enable permissions
			define('SQ_ROLLBACK_VIEW',        '0');
			define('SQ_TABLE_RUNNING_PREFIX', 'sq_');

		}//end if

		// process any global actions we have
		if (!empty($_REQUEST['SQ_ACTION'])) {
			$this->_processGlobalActions();
		}

	}//end init()


	/**
	* From here is where everything else happens, it starts the ball rolling
	*
	* @return void
	* @access public
	*/
	function start()
	{
		// if the user is Up For Review, prompt the user to change password
		$require_password_change = ($this->user && !is_a($this->user, 'public_user') && $this->user->status == SQ_STATUS_LIVE_APPROVAL);

		if (SQ_IN_BACKEND || SQ_IN_LIMBO) {

			if ($require_password_change) {
				$this->paintPasswordChange(translate('change_password'), translate('must_change_password_to_access_backend'));
				exit();
			}

			if ($this->user && !is_a($this->user, 'public_user')) {
				require_once SQ_INCLUDE_PATH.'/backend.inc';
				$backend =& new Backend();
				$this->backend =& $backend;
				$backend->paint();
			} else {
				$this->paintLogin(translate('login'), translate('must_login_to_access_backend'));
			}

		} else {
			// we are on the frontend

			// check for ./?a=xx type urls or normal
			if (isset($_REQUEST['a'])) {
				// requested asset is exclusively in the trash - don't display
				if ($this->am->assetInTrash($_REQUEST['a'], TRUE)) {
					$asset = NULL;
				} else {
					$asset =& $this->am->getAsset($_REQUEST['a'], '', TRUE);
				}
			} else {
				$asset =& $this->am->getAssetFromURL(NULL, NULL, TRUE, TRUE);
				if ($asset != NULL) {
					$valid_protocols = $asset->getValidProtocols();
					if (empty($valid_protocols)) {
						// if current protocol not valid and no other protocol exists for this URL, it's as if
						// this asset has not been found at all.  This would happen if it only had
						// an HTTP URL but force-secure = true for the asset.
						$asset = NULL;
					}
				}
			}

			if (!is_null($asset) && !$asset->effectiveUnrestricted() && $require_password_change) {
				$page_name = '"'.$asset->name.'"';
				$this->paintPasswordChange(translate('change_password'), translate('must_change_password_to_access_asset', $page_name));
				exit();
			}



			if (is_null($asset)) {

				$protocol = current_protocol();
				$url      = strip_url(current_url(FALSE, TRUE));

				$rm =& $this->am->getSystemAsset('remap_manager');
				if (!$rm->loadRemapFromURL($protocol, $url)) {

					// okay, the remap manager could not remap this url, so
					// check to see if we can find a 404 page from a site matching this url
					$root_url = $this->am->getRootURL($url);

					// if we can get a site from the url,
					// try to delegate the 404 to its not found page
					if (!empty($root_url)) {
						$site =& $this->am->getAsset($root_url['assetid']);
						if (!is_null($site)) {
							$page =& $site->getSpecialPage('not_found');
							if (!is_null($page)) {
								header('HTTP/1.0 404 Not Found');

								ob_start();
									$site->paintAsset($page);
								$content = ob_get_clean();

								$this->_replaceKeyword($content);
								echo $content;

								exit(1);
							}
						}
					}

					// if we can't get a site from the url, or the
					// site does not have a 404 page, then try asking
					// the remap manager to remap to its 404 page
					if (!$rm->paintPageNotFoundRemap()) {
						header('HTTP/1.0 404 Not Found');
						// last resort, display little green error
						trigger_localised_error('SYS0218', E_USER_NOTICE, $protocol.'://'.$url);
					}

				}//end if !load remap
				exit(1);

			}//end if is_null(asset)

			$this->frontend_asset = &$asset;
			$this->broadcastTriggerEvent('trigger_event_asset_accessed', $asset);

			// buffer the content and do keyword replacement
			ob_start();
				// caching is enabled and asset is readable by public user, send HTTP headers
				if (basename($_SERVER['PHP_SELF']) != SQ_CONF_NOCACHE_SUFFIX && is_a($GLOBALS['SQ_SYSTEM']->user, 'public_user')
					&& $asset->effectiveUnrestricted() && empty($_POST) && SQ_CONF_SEND_CACHEABLE_HEADER) {
					$this->_sendHTTPHeaders($asset);
				} else {
					$asset->printFrontend();
				}
				$content = ob_get_contents();
			ob_end_clean();

			$this->_replaceKeyword($content);
			echo $content;

		}//end else backend/limbo

		if (SQ_CONF_DEBUG & 4) {
			// report all the assets with a reference count > 0
			log_write($this->am->getForgottenItemsReport(), 'asset_cache', E_USER_NOTICE, FALSE);
		}

		$this->_setApacheNotes();

		$this->db->disconnect();

	}//end start()


	/**
	* Sets the Apache Request Notes with the set information filled on the the Apache Configuration Screen
	*
	* @return void
	* @access private
	*/
	function _setApacheNotes()
	{
		include SQ_DATA_PATH.'/private/conf/apache.inc';
		foreach ($request_notes as $note_name => $note_info) {
			// record request note for each type
			$note_value = '';
			switch ($note_info['type']) {
				case 'user':
					$note_value = $GLOBALS['SQ_SYSTEM']->user->getKeywordReplacement($note_info['keyword']);
				break;
				case 'asset':
					if (isset($this->frontend_asset)  && $this->frontend_asset->id) {
						$note_value = $this->frontend_asset->getKeywordReplacement($note_info['keyword']);
					}
				break;
			}
			if ((trim($note_value) !== '') && ($note_value !== '%'.$note_info['keyword'].'%')) {
				apache_note($note_name, $note_value);
			}
		}//end foreach

	}//end _setApacheNotes()


	/**
	* Sends appropiate HTTP Headers based on the system config
	*
	* @param object	&$asset	the asset being processed
	*
	* @return void
	* @access private
	*/
	function _sendHTTPHeaders(&$asset)
	{
		header('Pragma: cache');

		// save/load the cache information of this asset
		$cm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
		$cache_info = $cm->loadFromCache($asset->id, $asset->type(), 'cache_info');
		$cache_expiry = $cm->getExpiry($asset->type());
		header('Cache-Control: max-age='.$cache_expiry.', public');

		ob_start();
			$loaded_assetids = $GLOBALS['SQ_SYSTEM']->am->_asset_cache->getAssetKeys();
			$asset->printFrontend();
			$all_assetids = $GLOBALS['SQ_SYSTEM']->am->_asset_cache->getAssetKeys();
			$content = ob_get_contents();
		ob_end_clean();

		// not cached or has expired
		if (empty($cache_info)) {

			// work out the correct last updated time of this asset
			$assetids = array_diff($all_assetids, $loaded_assetids);
			$last_updated = $asset->getEffectiveLastUpdatedTime($assetids);

			$info = Array(
						'last_modified'	=> $last_updated,
						'expires'		=> (time() + $cache_expiry),
					);
			$cache_status = $cm->saveToCache($asset->id, $asset->type(), 'cache_info', serialize($info));
			// if cache_status is FALSE, then caching is probably disabled, expiry time = now
			$expiry_time = $cache_status ? $info['expires'] : time();
			header('Expires: '.gmdate('D, d M Y H:i:s', $expiry_time).' GMT');
			if (SQ_CONF_SEND_LAST_MODIFIED_HEADER && $cache_status) {
				header('Last-Modified: '.gmdate('D, d M Y H:i:s', $info['last_modified']).' GMT');
			}
			echo $content;

		// cached version found
		} else {
			$info = unserialize($cache_info);
			$last_modified = gmdate('D, d M Y H:i:s', $info['last_modified']).' GMT';
			$headers = $this->_getHeaders();
			$not_modified = FALSE;
			// if the last modified time matches the If-Modified-Since header in HEAD request
			if (isset($headers['If-Modified-Since']) && ($last_modified == $headers['If-Modified-Since'])) {
				$not_modified =TRUE;
			}

			header('Expires: '.gmdate('D, d M Y H:i:s', $info['expires']).' GMT');

			if (SQ_CONF_SEND_LAST_MODIFIED_HEADER) {
				header('Last-Modified: '.$last_modified);
			}
			if (SQ_CONF_SEND_NOT_MODIFIED_HEADER && $not_modified) {
				header('HTTP/1.1 304 Not Modified');
				exit(1);
			} else {
				// not sending 304 header, print the cached content
				echo $content;
			}
		}

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($cm);

	}//end _sendHTTPHeaders()


	/**
	* Highest level of keyword replacement is done here
	*
	* @param string	&$content					the text to replace keywords in
	* @param array	$additional_replacements	extra replacements to add
	*
	* @return void
	* @access private
	*/
	function _replaceKeyword(&$content, $additional_replacements=Array())
	{
		// Replace any global keywords first
		replace_global_keywords($content);

		if ($this->getGlobalDefine('SQ_REPLACE_MYSOURCE_LEVEL_KEYWORDS', TRUE)) {
			// Perform any additional replacements
			$keywords = retrieve_keywords_replacements($content);
			foreach ($keywords as $word) {
				if (!isset($additional_replacements[$word])) {
					$replacement = $this->frontend_asset->getKeywordReplacement($word);
					// still cannot replace it, blank it out
					if ($replacement == "%$word%") {
						$additional_replacements[$word] = '';
					} else {
						$additional_replacements[$word] = $replacement;
					}
				}
			}
			replace_keywords($content, $additional_replacements);
		}

	}//end _replaceKeyword()


//--        GLOBAL DEFINES        --//


	/**
	* Set a new global define, or overwrite the value of an existing one
	*
	* @param string	$index	the index of the variable
	* @param string	$value	the value of the variable
	*
	* @return void
	* @access public
	*/
	function setGlobalDefine($index, $value)
	{
		if (!empty($index)) {
			$this->_global_defines[$index] = $value;
		}

	}//end setGlobalDefine()


	/**
	* Unset a global define
	*
	* @param string	$index	the index of the variable
	*
	* @return void
	* @access public
	*/
	function unsetGlobalDefine($index)
	{
		unset($this->_global_defines[$index]);

	}//end unsetGlobalDefine()


	/**
	* Get the value of a global define, or $default if the variable doesn't exist
	*
	* @param string	$index		the index of the variable
	* @param string	$default	a default value to return if $index not set
	*
	* @return mixed int|float|string|boolean|array|object|void|NULL
	* @access public
	*/
	function getGlobalDefine($index, $default=NULL)
	{
		if (!is_array($this->_global_defines)) return $default;
		return $this->isGlobalDefineSet($index) ? $this->_global_defines[$index] : $default;

	}//end getGlobalDefine()


	/**
	* Check if a globally defined variable has been set
	*
	* @param string	$index	the index of the variable
	*
	* @return boolean
	* @access public
	*/
	function isGlobalDefineSet($index)
	{
		return (isset($this->_global_defines[$index])) ? TRUE : FALSE;

	}//end isGlobalDefineSet()


//--        LOGGING USERS IN, OUT AND ABOUT        --//


	/**
	* Paints the login form, based upon which url we are currently at
	*
	* @param string	$heading	a heading to print on the form
	* @param string	$msg		a msg to print with the form
	*
	* @return void
	* @access public
	*/
	function paintLogin($heading, $msg)
	{
		// Attempt to perform an auth redirect. If there is one, then the script
		// will terminate inside the do_redirect() function and redirect. If
		// not, this function will return FALSE and we will fall through to the
		// login box.
		$this->authRedirect();

		$db =& $this->db;
		header('HTTP/1.0 403 Forbidden');

		if (array_get_index($_REQUEST, 'SQ_BACKEND_PAGE') == 'header') {
			// we don't want to print a login box in the top frame, so reload the parent frame
			$redirect_location = replace_query_string_vars(Array('SQ_BACKEND_PAGE' => NULL));
			?>
			<script type="text/javascript">
				window.parent.document.location = '<?php echo $redirect_location; ?>';
			</script>
			<?php
			return;
		}

		if (current_protocol() == 'http') {
			// check if we require secure login for this root URL
			$current_url = current_url(FALSE, TRUE);
			$root_urls = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);
			$current_root_url = '';
			foreach ($root_urls as $url) {
				if (0 === strpos($current_url, $url)) {
					if (strlen($url) > strlen($current_root_url)) {
						$current_root_url = $url;
					}
				}
			}

			if (in_array($current_root_url, explode("\n", SQ_CONF_FORCE_SECURE_LOGIN_URLS))) {
				// Redirect to HTTPS
				$_SESSION['SQ_RETURN_TO_HTTP'] = 1;
				do_redirect('https://'.current_url(FALSE)); // exits
			}
		}//end if http

		$GLOBALS['SQ_LOGIN_FORM'] = Array('paint' => 'login', 'heading' => $heading, 'msg' => $msg);

		if (empty($_GET['FORCE_BACKUP_LOGIN'])) {

			$current_asset =& $this->am->getAssetFromURL(NULL, NULL, TRUE, TRUE);
			// if we can't find a current asset, use the root folder
			if (is_null($current_asset)) {
				$current_asset =& $this->am->getSystemAsset('root_folder');
				if (is_null($current_asset)) {
					trigger_localised_error('SYS0248', E_USER_ERROR);
				}
			}

			$design_info = $this->am->getDesignFromURL(strip_url(current_url(FALSE, TRUE)), 'design::system::login');

			// we have found the design to use
			if ($design_info) {
				$design =& $this->am->getAsset($design_info['designid'], $design_info['type_code']);
			} else {
				// OK, use the system login form
				$design =& $this->am->getSystemAsset('login_design');
			}

			if (!is_null($design)) {
				$design->paint($current_asset);
				return;
			}

		}//end if

		// we can't find a design, resort to our backup
		if ($this->_msgs) echo implode('<br />', $this->_msgs);
		require_once SQ_INCLUDE_PATH.'/login.inc';

	}//end paintLogin()


	/**
	* Paints the password change form, based upon which url we are currently at
	*
	* @param string	$heading	a heading to print on the form
	* @param string	$msg		a msg to print with the form
	*
	* @return void
	* @access public
	*/
	function paintPasswordChange($heading, $msg)
	{
		$db =& $this->db;

		$GLOBALS['SQ_PASSWORD_CHANGE_FORM'] = Array('paint' => 'password_change', 'heading' => $heading, 'msg' => $msg);

		if (empty($_GET['FORCE_BACKUP_PASSWORD_CHANGE'])) {

			$current_asset =& $this->am->getAssetFromURL(NULL, NULL, TRUE, TRUE);
			// if we can't find a current asset, use the root folder
			if (is_null($current_asset)) {
				$current_asset =& $this->am->getSystemAsset('root_folder');
				if (is_null($current_asset)) {
					trigger_localised_error('SYS0248', E_USER_ERROR);
				}
			}

			$design_info = $this->am->getDesignFromURL(strip_url(current_url(FALSE, TRUE)), 'design::system::password_change');

			// we have found the design to use
			if ($design_info) {
				$design =& $this->am->getAsset($design_info['designid'], $design_info['type_code']);
			} else {
				// OK, use the system password change form
				$design =& $this->am->getSystemAsset('password_change_design');
			}

			if (!is_null($design)) {
				$design->paint($current_asset);
				return;
			}

		}//end if

		// we can't find a design, resort to our backup
		if ($this->_msgs) echo implode('<br />', $this->_msgs);
		require_once SQ_INCLUDE_PATH.'/password_change.inc';

	}//end paintPasswordChange()


	/**
	* Set up the current user, reading from the session and checking that conditions are satisfied
	*
	* @return void
	* @access public
	*/
	function setupUser()
	{
		if (!SQ_CONF_ALLOW_IP_CHANGE && isset($_SESSION['remote_addr']) && ($_SESSION['remote_addr'] != $_SERVER['REMOTE_ADDR'])) {
			// IP address has changed and we don't allow that
			$this->_loginPublicUser();
			return;
		}

		$this->user = NULL;

		if (!empty($_SESSION['userid']) && !empty($_SESSION['user']) && !empty($_SESSION['user_serialised_ts'])) {
			$userid = $_SESSION['userid'];
			// There is a serialised user object in the session; if it's not stale, unserialise and use it
			$user_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($userid), $_SESSION['user_type_code'], TRUE, 'updated');
			$user_updated_date = array_get_index($user_info, $userid, FALSE);
			if ($user_updated_date) {
				if ($_SESSION['user_serialised_ts'] > iso8601_ts($user_updated_date)) {
					$GLOBALS['SQ_SYSTEM']->am->includeAsset($_SESSION['user_type_code']);
					$this->user = unserialize($_SESSION['user']);
					$GLOBALS['SQ_SYSTEM']->am->rememberAsset($this->user);
				}
			}
		}

		if (is_null($this->user) && !empty($_SESSION['userid'])) {
			// We failed to unserialise a user from the session, but there is nevertheless a userid
			// in the session, so the serialised version must have been stale.  So, let's load the asset
			// from the DB using the userid and re-save to session.
			$this->user =& $GLOBALS['SQ_SYSTEM']->am->getAsset($_SESSION['userid'], $_SESSION['user_type_code']);
			if (!is_null($this->user)) {
				$_SESSION['user'] = serialize($this->user);
				$_SESSION['user_serialised_ts'] = time();
			}
		}

		if (is_null($this->user)) {
			// See if we can log in an http-authenticated user
			if (defined('SQ_CONF_ALLOW_HTTP_LOGIN') && SQ_CONF_ALLOW_HTTP_LOGIN) {
				$this->_loginHttpAuthenticatedUser();
			}
		}

		if (is_null($this->user)) {
			// Have a go at using HTTP credentials
			if (defined('SQ_CONF_ACCEPT_HTTP_CREDS') && SQ_CONF_ACCEPT_HTTP_CREDS) {
				$this->_authenticateHttpUser();
			}
		}

		if (is_null($this->user)) {
			// Must be the public user
			$this->_loginPublicUser();
		}

		// Right, now we have a user of some sort; let's check their session preferences
		$security_actions = Array('login', 'logout', 'password_change', 'hipo', 'asset_map_request');
		$has_security_action = in_array(array_get_index($_REQUEST, 'SQ_ACTION'), $security_actions);
		if (array_get_index($_SESSION, 'user_last_access') && !$has_security_action) {

			$session_prefs = $this->getUserPrefs('user', 'SQ_USER_SESSION_PREFS', TRUE);

			// See if it's been too long since their last MatrixActivity(R)
			if (!empty($session_prefs['timeout'])) {
				if (time() - $_SESSION['user_last_access'] > $session_prefs['timeout']) {
					$this->_logSecurityMsg('system.security.logout.idle', Array('timeout' => $session_prefs['timeout']));
					$this->_clearSession();
					$this->_loginPublicUser();
					$this->generateLoginKey(TRUE);
					return;
				}
			}

			// See if it's been too long since their last authentication (or beginning of public user's session).
			// To avoid demanding authentication at an inconvenient time (eg when the user is submitting a big form),
			// the timeout is rounded to a number of days.  If the timeout was yesterday then we demand authentication
			// straight away.  If the timeout is today then we only ask for authentication if the user has been inactive
			// for a certain time (Which probably means they are not editing).
			if (array_get_index($_SESSION, 'user_last_authentication')) {
				if (!empty($session_prefs['max_length'])) {
					$session_expiry_ts = $_SESSION['user_last_authentication'] + $session_prefs['max_length'];
					$today_start_ts = strtotime('00:00:01 today');
					$tomorrow_start_ts = strtotime('00:00:01 tomorrow');
					if ($session_expiry_ts < $today_start_ts) {
						// it expired yesterday so clear the session / force a login right now
						$this->_logSecurityMsg('system.security.logout.staleauth', Array('timeout' => $session_prefs['max_length']));
						$this->_clearSession();
						$this->_loginPublicUser();
						$this->generateLoginKey(TRUE);
					} else if ($session_expiry_ts < $tomorrow_start_ts) {
						// it's meant to expire today...
						if (time() - $_SESSION['user_last_access'] > SQ_CONF_EDITING_TIME) {
							// because they haven't done anything for a while we assume it's safe to log them out
							// and ask for re-authentication
							$this->_logSecurityMsg('system.security.logout.staleauth', Array('timeout' => $session_prefs['max_length']));
							$this->_clearSession();
							$this->_loginPublicUser();
							$this->generateLoginKey(TRUE);
							return;
						}
					}
				}
			}
		}//end if

		$this->_user_is_root      = $_SESSION['user_is_root'];
		$this->_user_is_sys_admin = $_SESSION['user_is_sys_admin'];
		$this->_user_is_public    = $_SESSION['user_is_public'];

	}//end setupUser()


	/**
	* Processes any global actions that need to be taken care of
	*
	* @return void
	* @access private
	*/
	function _processGlobalActions()
	{
		switch ($_REQUEST['SQ_ACTION']) {
			case 'password_change' :
				$password_old     = array_get_index($_POST, 'SQ_PASSWORD_CHANGE_OLD');
				$password_new     = array_get_index($_POST, 'SQ_PASSWORD_CHANGE_NEW');
				$password_confirm = array_get_index($_POST, 'SQ_PASSWORD_CHANGE_CONFIRM');

				if (is_null($password_old)) return;
				if (is_null($password_new)) return;
				if (is_null($password_confirm)) return;

				if (!$this->user->comparePassword($password_old)) {
					$this->addMessage('Your old password is incorrect');
					return;
				}

				if ($password_old == $password_new) {
					$this->addMessage('The new password must be different from your old one');
					return;
				}

				if ($password_new != $password_confirm) {
					$this->addMessage('The new and confirmed passwords do not match');
					return;
				}

				include_once SQ_ATTRIBUTES_PATH.'/password/password.inc';
				if (!Asset_Attribute_Password::validateValue($password_new)) {
					$this->addMessage('Bad password values');
					return;
				}

				if ($this->user && !is_a($this->user, 'public_user')) {
					// logged in, and the password is correct. change the password
					if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS)) {
						$run_level_changed = TRUE;
						$GLOBALS['SQ_SYSTEM']->setRunLevel($GLOBALS['SQ_SYSTEM']->getRunLevel() - SQ_SECURITY_PERMISSIONS);
					} else {
						$run_level_changed = FALSE;
					}

					$result = $this->user->setAttrValue('password', $password_new);
					$result = $result && $this->user->saveAttributes();

					if ($run_level_changed) {
						$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
					}

					if ($result && $this->user->status != SQ_STATUS_LIVE) {
						$available_statuses = $this->user->getAvailableStatii();
						if (!isset($available_statuses[SQ_STATUS_LIVE]) || !$this->user->processStatusChange(SQ_STATUS_LIVE)) {
							trigger_localised_error('SYS0304', E_USER_ERROR);
							return;
						}
					}
				}

			break;

			case 'login' :
				$login_username = array_get_index($_POST, 'SQ_LOGIN_USERNAME');
				$login_password = array_get_index($_POST, 'SQ_LOGIN_PASSWORD');
				$login_key      = array_get_index($_POST, 'SQ_LOGIN_KEY');
				if (is_null($login_username) || is_null($login_password)) {
					return;
				}

				// if this person is already logged in, don't bother trying again because
				// we will only be giving them an incorrect login key error
				if (!is_null($this->user) && $login_username == $this->user->attr('username')) {
					return;
				}

				if (!is_null($login_key) && $this->loginKey() == $login_key) {

					// get a list of all the installed authentication systems
					$auth_folder =& $this->am->getSystemAsset('authentication_folder');
					if (is_null($auth_folder)) {
						trigger_localised_error('SYS0249', E_USER_ERROR);
						return;
					}
					$auth_systems = $auth_folder->getAuthSystems();
					// try each auth system in order to see if we can load a user asset
					$user = NULL;
					$user_status = NULL;
					foreach ($auth_systems as $systemid) {
						$system =& $this->am->getAsset($systemid);
						if (is_null($system)) continue;
						$user =& $system->authenticateUser($login_username, $login_password);
						if (!is_null($user)) $user_status = $user->status;
						// check that the user exists AND that it can login
						// (ie. it is not yet live, or has been locked)
						if (!is_null($user) && $user->canLogin()) {
							return $this->loginUser($user);
						}
					}

					$failure_reason = translate('user_not_found');
					if (!is_null($user_status)) {
						if ($user_status & SQ_SC_STATUS_NOT_LIVE) {
							$failure_reason = translate('user_status_not_live');
						} else {
							$failure_reason = translate('user_asset_in_trash');
						}
					}

					$this->addMessage(translate('user_not_found'));
					// log failed attempt
					$this->_logSecurityMsg('system.security.login.failed', Array('login_name' => $login_username, 'failure_reason' => $failure_reason), TRUE);

				} else {

					// incorrect login key
					$this->addMessage(translate('login_key_incorrect'));
				}

				// deliberately don't have a break here so if the
				// login fails we make sure that we're logged out
			case 'logout' :
				$for_real = ($this->user && !is_a($this->user, 'public_user'));
				$old_user =& $this->user;
				$this->_clearSession();
				$this->_loginPublicUser();
				$this->generateLoginKey(TRUE);
				if ($for_real) {
					$this->_logSecurityMsg('system.security.logout.manual');
					$this->broadcastTriggerEvent('trigger_event_user_logout', $old_user, NULL);
				}
			break;

			case 'send_security_key' :
				require_once SQ_FUDGE_PATH.'/general/general.inc';
				$vars =& $_SESSION['SQ_SYSTEM_SECURITY_KEY'];

				// are we regenerating the security key?
				if (isset($_GET['regen_key']) && $_GET['regen_key'] == 1) {
					$vars['key'] = generate_security_key(strlen($vars['key']));
				}

				security_key_image($vars['key'], $vars['width'], $vars['height'], $vars['bg_colour'], $vars['text_colour'], $vars['border_colour'], $vars['zoom']);
				exit(1);
			break;

			case 'rollback_view_start' :
				$_SESSION['sq_rollback_view'] = Array();

				if (isset($_GET['assetid']) && isset($_GET['version'])) {
					// work out when the version of the asset was created
					$assetid = $_GET['assetid'];
					$version = $_GET['version'];

					// note that we dont prep this query for rollback select
					// because we are selecting directly from the rollback table
					$sql = 'SELECT sQ_eff_from AS efrom
							FROM sq_rb_ast
							WHERE assetid = '.$this->db->quote($assetid).'
							  AND version = '.$this->db->quote($version);

					$then = $this->db->getOne($sql);
					assert_valid_db_result($then);
					$_SESSION['sq_rollback_view'] = Array('rollback_time' => $then);
				}

				unset($_GET['assetid']);
				unset($_GET['version']);
			break;

			case 'rollback_view_stop' :
				if (isset($_SESSION['sq_rollback_view'])) {
					unset($_SESSION['sq_rollback_view']);
				}
			break;

			case 'hipo' :
				$hh =& $this->getHipoHerder();
				if ($hh->processWeb()) $hh->paintWeb();
				exit(1);
			break;

			case 'set_design_name' :
				$_SESSION['SQ_DESIGN_NAME'] = (isset($_GET['SQ_DESIGN_NAME'])) ? $_GET['SQ_DESIGN_NAME'] : '';
				unset($_GET['SQ_DESIGN_NAME']);
			break;

			case 'clear_design_name' :
				if (isset($_SESSION['SQ_DESIGN_NAME'])) {
					unset($_SESSION['SQ_DESIGN_NAME']);
				}
			break;

			case 'asset_map_request' :
				require_once SQ_LIB_PATH.'/asset_map/asset_map.inc';
				$asset_map =& new Asset_Map();
				$asset_map->process($this->backend);
				exit(1);
			break;

		}//end switch

		// just so it isn't being used anywhere else
		$_REQUEST['SQ_ACTION'] = '';

	}//end _processGlobalActions()


	/**
	* Log in the public user
	*
	* @return void
	* @access public
	*/
	function _loginPublicUser()
	{
		if (empty($this->user) || !is_a($this->user, 'public_user')) {
			$this->user =& $this->getPublicUser();
			$_SESSION['user_is_root']      = $this->_user_is_root      = FALSE;
			$_SESSION['user_is_sys_admin'] = $this->_user_is_sys_admin = FALSE;
			$_SESSION['user_is_public']    = $this->_user_is_public    = TRUE;
		}

	}//end _loginPublicUser()


	/**
	* Try to login a user who has already been HTTP-authenticated by some external mechanism
	*
	* @return void
	* @access public
	*/
	function _loginHttpAuthenticatedUser()
	{
		if (array_get_index($_REQUEST, 'SQ_ACTION')) return;
		$http_username = array_get_index($_SERVER, SQ_CONF_HTTP_LOGIN_VAR);
		if (is_null($http_username) || ($http_username === '')) {
			return;
		}

		// get a list of all the installed authentication systems
		$auth_folder =& $this->am->getSystemAsset('authentication_folder');
		if (is_null($auth_folder)) {
			trigger_localised_error('SYS0249', E_USER_ERROR);
			return;
		}
		$auth_systems = $auth_folder->getAuthSystems();

		// try each auth system in order to see if we can load a user asset
		$user = NULL;
		foreach ($auth_systems as $systemid) {
			$system =& $this->am->getAsset($systemid);
			if (is_null($system)) continue;
			$user =& $system->authenticateHttpUser($http_username);
			if (!is_null($user)) return $this->loginUser($user);
		}

	}//end _loginHttpAuthenticatedUser()


	/**
	* Check the user authentication. If the user is authenticated then it assigns the user and returns TRUE else returns FALSE
	*
	* @return void
	* @access public
	*/
	function _authenticateHttpUser()
	{
		if (array_get_index($_REQUEST, 'SQ_ACTION')) return;

		if (!(array_get_index($_SERVER, 'PHP_AUTH_USER') && array_get_index($_SERVER, 'PHP_AUTH_PW'))) {
			return;
		}
		// get a list of all the installed authentication systems
		$auth_folder =& $this->am->getSystemAsset('authentication_folder');
		if (is_null($auth_folder)) {
			trigger_localised_error('SYS0249', E_USER_ERROR);
			return;
		}
		$auth_systems = $auth_folder->getAuthSystems();
		// try each auth system in order to see if we can load a user asset
		$user = NULL;
		foreach ($auth_systems as $systemid) {
			$system =& $this->am->getAsset($systemid);
			if (is_null($system)) continue;
			$user =& $system->authenticateUser($_SERVER['PHP_AUTH_USER'], $_SERVER['PHP_AUTH_PW']);
			if (!is_null($user)) return $this->loginUser($user);
		}
		return;

	}//end _authenticateHttpUser()


	/**
	* Log in the specified user, setting the session etc as appropriate
	*
	* SHOULD BE USED WITH CAUTION - the assumption is that this user has already been authenticated
	*
	* @param object	&$user	The user to log in
	*
	* @return void
	* @access public
	*/
	function loginUser(&$user)
	{
		if (is_null($user)) return FALSE;

		if (!$this->setCurrentUser($user)) return FALSE;


		$old_session_id = session_id();
		// regenerate the session id to protect against session fixation attacks
		session_regenerate_id();
		// when logging in we wipe the previous session file
		$sess_filename = session_save_path().'/sess_'.$old_session_id;
		@unlink($sess_filename);

		$_SESSION['userid'] = $user->id;
		$_SESSION['user_type_code'] = $user->type();
		$_SESSION['user'] = serialize($user);
		$_SESSION['user_serialized_ts'] = time();
		if (!$this->_user_is_public) {
			$_SESSION['remote_addr'] = $_SERVER['REMOTE_ADDR'];
		}

		// log a message so we know someone has logged in
		$this->_logSecurityMsg('system.security.login');

		// generate a new login key so that when they try and login next
		// they have to re-enter the details
		$this->generateLoginKey(TRUE);

		$_SESSION['user_is_root'] = $this->_user_is_root;
		$_SESSION['user_is_sys_admin'] = $this->_user_is_sys_admin;
		$_SESSION['user_is_public'] = $this->_user_is_public;
		$_SESSION['user_last_access'] = time();
		$_SESSION['user_last_authentication'] = time();

		// Do pass any event data since we can get it from $_SESSION
		$event_data = Array('password' => array_get_index($_POST, 'SQ_LOGIN_PASSWORD'));
		$this->broadcastTriggerEvent('trigger_event_user_login', $user, $event_data);

		// check warranty (if a warranty key exists)
		$decoded_wkey = base64_decode(SQ_LICENCE_WARRANTY_KEY);
		if (defined('SQ_LICENCE_WARRANTY_KEY') && (substr_replace($decoded_wkey, '', 6, 8) !== 'abcdef00000000') && (SQ_LICENCE_TYPE === 'SSV')) {
			$wkey_snippet = substr($decoded_wkey, 0, 6);
			$ikey_snippet = substr(SQ_LICENCE_INSTALL_KEY, 0, 6);

			$expiry_date = substr($decoded_wkey, 14);
			$current_date = date('Ymd');

			$warranty_expired = ($current_date > $expiry_date) || ($wkey_snippet !== $ikey_snippet);
		} else {
			// no warranty
			$warranty_expired = FALSE;
		}

		if ($this->userSystemAdmin($user) && $warranty_expired) {
			$return_url = current_protocol().'://'.current_url(FALSE);

			// We need to handle the 'return to HTTP' stuff here in any case
			if (!$_SESSION['user_is_public'] && array_get_index($_SESSION, 'SQ_RETURN_TO_HTTP')) {
				unset($_SESSION['SQ_RETURN_TO_HTTP']);
				$return_url = 'http://'.current_url(FALSE);
			}
			require_once dirname(__FILE__).'/key_reminder.inc';
			exit();
		}

		if (!$_SESSION['user_is_public'] && array_get_index($_SESSION, 'SQ_RETURN_TO_HTTP')) {
			unset($_SESSION['SQ_RETURN_TO_HTTP']);
			do_redirect('http://'.current_url(FALSE)); // exits
		}

		return TRUE;

	}//end loginUser()


	/**
	* Sets the current user of the system to be the specified user
	*
	* SHOULD BE USED WITH CAUTION - no authentication is done inside here
	* This is generally to be used by scripts etc - the session is not updated with the new user's details
	*
	* @param object	&$user	the user to make current
	*
	* @return boolean
	* @access public
	*/
	function setCurrentUser(&$user)
	{
		if (!is_a($user, 'user')) {
			trigger_localised_error('SYS0171', E_USER_WARNING);
			return FALSE;
		}

		// they can only become current user if they can login
		if (!$user->canSetAsCurrentUser()) return FALSE;

		if (!is_null($this->user)) {
			if (!isset($this->_tmp['current_user_cache'])) {
				$this->_tmp['current_user_cache'] = Array();
			}
			array_push($this->_tmp['current_user_cache'], Array($this->user->id, $this->user->type()));
		}

		$this->user = &$user;
		$this->_user_is_root = $this->userRoot($user);
		$this->_user_is_sys_admin = $this->userSystemAdmin($user);
		$this->_user_is_public = $this->userPublic($user);

		return TRUE;

	}//end setCurrentUser()


	/**
	* Reset the current user to the previously assigned user
	*
	* SHOULD BE USED WITH CAUTION
	*
	* @return boolean
	* @access public
	*/
	function restoreCurrentUser()
	{
		if (empty($this->_tmp['current_user_cache'])) {
			$user = NULL;
		} else {
			list($userid, $type_code) = array_pop($this->_tmp['current_user_cache']);
			$user =& $this->am->getAsset($userid, $type_code);
		}

		// They can only become current user if they can login
		if (is_null($user) || !$user->canSetAsCurrentUser()) {
			$this->user =& $this->getPublicUser();
			$this->_user_is_root      = FALSE;
			$this->_user_is_sys_admin = FALSE;
			$this->_user_is_public    = TRUE;

		} else {
			$this->user = &$user;
			$this->_user_is_root      = $this->userRoot($user);
			$this->_user_is_sys_admin = $this->userSystemAdmin($user);
			$this->_user_is_public    = $this->userPublic($user);
		}

		return TRUE;

	}//end restoreCurrentUser()


	/**
	* Returns a unique session key
	*
	* the key can be used to strengthen the sending of the
	* session id as a $_GET variable
	*
	* @return string
	* @access public
	*/
	function getUniqueSessionKey()
	{
		$key =  $_SESSION['userid'].
				$_SESSION['user_type_code'].
				$_SESSION['SQ_LOGIN_KEY'];

		if (!SQ_CONF_ALLOW_IP_CHANGE) {
			$key .= $_SESSION['remote_addr'];
		}

		return md5($key);

	}//end getUniqueSessionKey()


	/**
	* Generates a new login key and returns it
	*
	* @param boolean	$force_new_key	if TRUE the key will regenerated even if
	*									there is already a value save in the session
	*
	* @return string
	* @access public
	*/
	function generateLoginKey($force_new_key=FALSE)
	{
		require_once SQ_FUDGE_PATH.'/general/general.inc';

		if ($force_new_key || empty($_SESSION['SQ_LOGIN_KEY'])) {
			$_SESSION['SQ_LOGIN_KEY'] = random_password(20);
		}

		return $_SESSION['SQ_LOGIN_KEY'];

	}//end generateLoginKey()


	/**
	* Returns the current login key
	*
	* @return string
	* @access public
	*/
	function loginKey()
	{
		return (isset($_SESSION['SQ_LOGIN_KEY'])) ? $_SESSION['SQ_LOGIN_KEY'] : '';

	}//end loginKey()


	/**
	* Update the session cookie with the appropriate expiry time given the kind of user we are
	*
	* Note: this function must be called before any other cookies are set or it will clear their set-cookie headers
	*
	* @return void
	* @access public
	*/
	function _updateSessionCookie()
	{
		$session_id = session_id();
		if (empty($session_id)) return FALSE;

		$session_prefs = $this->getUserPrefs('user', 'SQ_USER_SESSION_PREFS', TRUE);
		$time_to_live = $session_prefs['persist']; // 1 or 0
		if (!empty($time_to_live)) {
			$time_to_live = $session_prefs['timeout'] + 3600; // cookie should last 1 extra day
			if (empty($time_to_live)) {
				// there is no max inactive time but we do want to persist across browser sessions...
				$time_to_live = 60 * 60 * 24 * 365; // give them a a year
			}
		}
		$session_params = session_get_cookie_params();
		$session_name = session_name();
		$header = "Set-Cookie: $session_name=$session_id; ";
		if (!empty($time_to_live)) {
			$header .= 'expires='.substr(date('r', time()+$time_to_live-((int)substr(date('O'), 0, 3)*60*60)), 0, -6).' GMT; ';
		}
		if (!empty($session_params['domain'])) {
			$domain = $session_params['domain'];
			if (defined('SQ_CONF_SYSTEM_PARENT_DOMAINS')) {
				$parent_domains = explode("\n", SQ_CONF_SYSTEM_PARENT_DOMAINS);
				if (is_array($parent_domains)) {
					foreach ($parent_domains as $parent_domain) {
						$parent_length = strlen($parent_domain);
						if ((strlen($domain) >= $parent_length) && (substr($domain, -$parent_length) == $parent_domain)) {
							// our current domain is a subdomain of this parent domain, so we will
							// actually set the cookie for the parent domain and its children
							$domain = '.'.$parent_domain;
							$session_params['path'] = '/';
							break;
						}
					}
				}
			}
			$header .= "domain=$domain; ";
		}
		if (!empty($session_params['path'])) {
			$header .= "path=$session_params[path]; ";
		}
		header($header);

	}//end _updateSessionCookie()


	/**
	* Clear the user's session of all data
	*
	* @return void
	* @access private
	*/
	function _clearSession()
	{
		foreach (array_keys($_SESSION) as $key) {
			if ($key == 'SQ_RETURN_TO_HTTP') continue;
			// do not reset the unsuccessful attempt, unset in &authenticateUser
			if ($key != 'user_login_attempts') {
				unset($_SESSION[$key]);
			}
		}

	}//end _clearSession()


	/**
	* Log a message to do with system security (logging in / out)
	*
	* @param string		$code				The message code for the message
	* @param array		$extra_replacements	Extra keyword replacements to use in the message (user_name is already there)
	* @param boolean	$failed_login		status of the log in process
	*
	* @return void
	* @access private
	*/
	function _logSecurityMsg($code, $extra_replacements=Array(), $failed_login=FALSE)
	{
		if ($this->userPublic() && !$failed_login) {
			return;
		}
		$ms =& $this->getMessagingService();
		$msg_reps = Array('user_name' => $this->user->name) + $extra_replacements;
		$log = $ms->newMessage(Array(), $code, $msg_reps);
		$log->parameters['remote_addr'] = $_SERVER['REMOTE_ADDR'];
		$log->parameters['sessionid'] = session_id();
		$log->send();

	}//end _logSecurityMsg()


//--        INFO ABOUT CURRENT USER        --//


	/**
	* Returns TRUE if the passed user is the logged in user
	*
	* @param object	$user	the user to check
	*
	* @return boolean
	* @access public
	*/
	function currentUser($user)
	{
		return ($this->user && $this->user->id == $user->id);

	}//end currentUser()


	/**
	* Returns the userid of the current user, or zero if none present
	*
	* @return mixed int|string
	* @access public
	*/
	function currentUserId()
	{
		if ($this->user) {
			return $this->user->id;
		} else if (isset($_SESSION['userid'])) {
			return $_SESSION['userid'];
		} else {
			return 0;
		}

	}//end currentUserId()


	/**
	* Returns the userid of the public access user, if it exists in the system
	*
	* @return string
	* @access public
	*/
	function &getPublicUser()
	{
		$public_user =& $this->am->getSystemAsset('public_user');
		return $public_user;

	}//end getPublicUser()


	/**
	* Is the passed/current user the root user?
	*
	* @param object	$user	the user object you want to test for root access
	*
	* @return boolean
	* @access public
	*/
	function userRoot($user=NULL)
	{
		if (is_null($user)) return $this->_user_is_root;
		if (!$user->id) return FALSE;
		if (get_class($user) == 'root_user') return TRUE;
		return FALSE;

	}//end userRoot()


	/**
	* Is the passed/current user a system administrator?
	*
	* NOTE: This function will return FALSE if the classname of $user is not exactly
	* equal to system_user - ie root user and any users that inherit from system_user
	* will return FALSE.
	*
	* @param object	$user	the user object you want to test for sysadmin access
	*
	* @return boolean
	* @access public
	*/
	function userSystemAdmin($user=NULL)
	{
		if (is_null($user)) return $this->_user_is_sys_admin;
		if (!$user->id) return FALSE;
		if (!is_a($user, 'backend_user')) return FALSE;

		// now make sure that the user is in the system administrators group
		$sys_admin_group =& $this->am->getSystemAsset('system_user_group');
		$sys_admins = $this->am->getLinks($sys_admin_group->id, SQ_LINK_TYPE_1);
		foreach ($sys_admins as $data) {
			if ($data['minorid'] == $user->id) return TRUE;
		}
		return FALSE;

	}//end userSystemAdmin()


	/**
	* Is the passed/current user the public user?
	*
	* @param object	$user	the user object you want to test for root access
	*
	* @return boolean
	* @access public
	*/
	function userPublic($user=NULL)
	{
		if (is_null($user)) return $this->_user_is_public;
		if (!$user->id) return FALSE;
		if (get_class($user) == 'public_user') return TRUE;
		return FALSE;

	}//end userPublic()


	/**
	* Get user preferences (user specific config settings) for the current user
	*
	* @param string		$asset_type	Asset type whose preferences we want (leave NULL to get all prefs)
	* @param string		$pref		Name of the preference var we want (leave NULL for all prefs for the asset type)
	* @param boolean	$value_only	If TRUE, returns the value of the pref only, rather than all its details
	*
	* @return mixed array|string|boolean
	* @access public
	*/
	function getUserPrefs($asset_type=NULL, $pref=NULL, $value_only=TRUE)
	{
		if (!is_file(SQ_DATA_PATH.'/private/conf/preferences.inc')) {
			return FALSE;
		}

		static $user_prefs = Array(); // per-script-execution cache
		if (empty($this->user->id)) {
			include SQ_DATA_PATH.'/private/conf/preferences.inc';
			if (is_null($pref)) {
				return $preferences;
			} else {
				if (!isset($preferences[$asset_type]) || !isset($preferences[$asset_type][$pref])) {
					return $preferences;
				}
				$pref_value = $preferences[$asset_type][$pref];
				if ($value_only) {
					return $pref_value['default'];
				} else {
					return $pref_value;
				}
			}
		}

		if (!isset($user_prefs[$this->user->id])) {
			$groups = $this->user->getGroups();
			include SQ_DATA_PATH.'/private/conf/preferences.inc';
			$global_prefs = $preferences;
			$final_prefs = Array();
			foreach ($groups as $groupid => $group_type_code) {
				$file_path = SQ_DATA_PATH.'/private/'.asset_data_path_suffix($group_type_code, $groupid).'/.preferences.inc';
				if (!is_file($file_path)) continue;
				include $file_path;
				foreach ($preferences as $type => $type_prefs) {
					if (!isset($final_prefs[$type])) {
						$final_prefs[$type] = $type_prefs;
					} else {
						$conflicting_prefs = array_intersect(array_keys($type_prefs), array_keys($final_prefs[$type]));
						if (empty($conflicting_prefs)) {
							$final_prefs[$type] += $type_prefs;
						} else {
							// a particular preference within this type has been defined twice, so we need the prefs
							// class to decide which value applies
							require_once SQ_SYSTEM_ROOT.'/'.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type, 'dir').'/'.$type.'_prefs.inc'; // can this be more efficient?
							eval('$final_prefs[$type] = '.$type.'_Prefs::mergePrefs($type_prefs, $final_prefs[$type]);');
						}
					}
					if ($value_only) {
						foreach ($final_prefs[$type] as $pref_name => $pref_details) {
							if (is_array($pref_details) && isset($pref_details['default'])) {
								$final_prefs[$type][$pref_name] = $pref_details['default'];
							} else {
								$final_prefs[$type][$pref_name] = $pref_details;
							}
						}
					}
				}
			}

			// now final_prefs contains all the group prefs; pad it out with remaining global prefs
			foreach ($global_prefs as $type => $type_prefs) {
				foreach ($type_prefs as $pref_name => $pref_details) {
					if (!isset($final_prefs[$type][$pref_name])) {
						$final_prefs[$type][$pref_name] = $value_only ? $pref_details['default'] : $pref_details;
					}
				}
			}

			$user_prefs[$this->user->id] = $final_prefs;
		}//end if
		if (is_null($asset_type)) {
			return $user_prefs[$this->user->id];
		} else {
			if (is_null($pref)) {
				return $user_prefs[$this->user->id][$asset_type];
			} else {
				return $user_prefs[$this->user->id][$asset_type][$pref];
			}
		}

	}//end getUserPrefs()


//--        GET MANAGERS        --//


	/**
	* Get the messaging service object so you can send/receive messages
	*
	* Always call this function to get the messaging service, dont use $this->ms directly
	*
	* @return object
	* @access public
	*/
	function &getMessagingService()
	{
		if (!isset($this->ms) || get_class($this->ms) != 'messaging_service') {
			require_once SQ_INCLUDE_PATH.'/messaging_service.inc';
			$this->ms =& new Messaging_Service();
		}
		return $this->ms;

	}//end getMessagingService()


	/**
	* Get the file versioning object so you can add/remove/commit/checkout file versions
	*
	* Always call this function to get the file versioning object, dont use $this->fv directly
	*
	* @return object
	* @access public
	*/
	function &getFileVersioning()
	{
		if (!isset($this->fv) || get_class($this->fv) != 'file_versioning') {
			require_once SQ_LIB_PATH.'/file_versioning/file_versioning.inc';
			$this->fv =& new File_Versioning(SQ_DATA_PATH.'/file_repository');
		}
		return $this->fv;

	}//end getFileVersioning()


	/**
	* Get the HIPO herder so you can queue and run HIPO jobs
	*
	* Always call this function to get the HIPO herder, dont use $this->hh directly
	*
	* @return object
	* @access public
	*/
	function &getHipoHerder()
	{
		if (!isset($this->hh) || get_class($this->hh) != 'hipo_herder') {
			require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_herder.inc';
			$this->hh =& new Hipo_Herder();
		}
		return $this->hh;

	}//end getHipoHerder()


	/**
	* Get the metadata manager so you can manage metadata in the system
	*
	* Always call this function to get the metadata manager, dont use $this->mm directly
	*
	* @return object
	* @access public
	*/
	function &getMetadataManager()
	{
		if (!isset($this->mm) || get_class($this->mm) != 'metadata_manager') {
			require_once SQ_INCLUDE_PATH.'/metadata_manager.inc';
			$this->mm =& new Metadata_Manager();
		}
		return $this->mm;

	}//end getMetadataManager()


	/**
	* Get the workflow manager so you can manage workflow in the system
	*
	* Always call this function to get the workflow manager, dont use $this->wm directly
	*
	* @return object
	* @access public
	*/
	function &getWorkflowManager()
	{
		if (!isset($this->wm) || get_class($this->wm) != 'workflow_manager') {
			require_once SQ_INCLUDE_PATH.'/workflow_manager.inc';
			$this->wm =& new Workflow_Manager();
		}
		return $this->wm;

	}//end getWorkflowManager()


	/**
	* Get the event manager so you can fire events
	*
	* Always call this function to get the event manager, dont use $this->em directly
	*
	* @return object
	* @access public
	*/
	function &getEventManager()
	{
		if (!isset($this->em) || get_class($this->em) != 'event_manager') {
			require_once SQ_INCLUDE_PATH.'/event_manager.inc';
			$this->em =& new Event_Manager();
		}
		return $this->em;

	}//end getEventManager()


	/**
	* Get the tag manager
	*
	* Always call this function to get the event manager, dont use $this->tm directly
	*
	* @return object
	* @access public
	*/
	function &getTagManager()
	{
		$am =& $this->am;
		if ((!isset($this->tag_manager) || (get_class($this->tag_manager) != 'tag_manager')) && $am->isSystemAssetType('tag_manager')) {
			$this->tag_manager = &$am->getSystemAsset('tag_manager');
		}
		return $this->tag_manager;

	}//end getTagManager()


	/**
	* Get the trigger manager so you can fire trigger events
	*
	* Always call this function to get the trigger manager, dont use $this->wm directly
	*
	* @return object
	* @access public
	*/
	function &getTriggerManager()
	{
		$am =& $this->am;
		if ((!isset($this->trigger_manager) || ($am->installed('trigger_manager') && !is_a($this->trigger_manager,'trigger_manager'))) && $am->isSystemAssetType('trigger_manager')) {
			$this->trigger_manager = &$am->getSystemAsset('trigger_manager');
		}

		return $this->trigger_manager;

	}//end getTriggerManager()


	/**
	* Get the Log Manager
	*
	* @return object
	* @access public
	*/
	function &getLogManager()
	{
		$am =& $this->am;
		if ((!isset($this->log_manager) || (get_class($this->log_manager) != 'log_manager')) && $am->isSystemAssetType('log_manager')) {
			$this->log_manager = &$am->getSystemAsset('log_manager');
		}

		// this is a bypass to allow logging while Log Manager asset has not been installed yet
		// necessary during installation, because we want ALL the log entries to be handled by the log manager,
		// not only those that appeared after it was installed.
		if (is_null($this->log_manager)) {
			return new Log_Manager();
		}

		return $this->log_manager;

	}//end getLogManager()


//--        RUN LEVELS        --//


	/**
	* Set the current run level
	*
	* @param int	$run_level	a combination of any of the SQ_SECURITY_* constants that determine
	*							which security sub-systems will be enabled (can also use the
	*							SQ_RUN_LEVEL_* constants as short-cuts)
	*
	* @return void
	* @access public
	*/
	function setRunLevel($run_level)
	{
		$this->_run_level_stack[] = $run_level;

	}//end setRunLevel()


	/**
	* Restore the run level to the previously set one
	*
	* @return void
	* @access public
	*/
	function restoreRunLevel()
	{
		if (empty($this->_run_level_stack)) {
			// there is no run level to restore
			trigger_localised_error('SYS0110', E_USER_ERROR);
		}

		array_pop($this->_run_level_stack);

	}//end restoreRunLevel()


	/**
	* Get the current run level
	*
	* Returns NULL if no run level has been set
	*
	* @return mixed int|NULL
	* @access public
	*/
	function getRunLevel()
	{
		if (empty($this->_run_level_stack)) return NULL;

		end($this->_run_level_stack);
		$current_run_level = current($this->_run_level_stack);
		reset($this->_run_level_stack);

		return $current_run_level;

	}//end getRunLevel()


	/**
	* Check if the current run level enables a particular security sub-system
	*
	* @param int	$security_system	one of the SQ_SECURITY_* constants thats represents
	*									the security sub-system we want to check is enabled
	*
	* @return void
	* @access public
	*/
	function runLevelEnables($security_system)
	{
		$current_run_level = $this->getRunLevel();
		if (is_null($current_run_level)) {
			// there is no run level to check
			trigger_localised_error('SYS0102', E_USER_ERROR);
		}

		return ($security_system & $current_run_level);

	}//end runLevelEnables()


//--        DATABASE        --//


	/**
	* Alter's which DB connection to use
	*
	* @param string	$conn	[db|db2] which DB connection to use
	*
	* @return void
	* @access public
	*/
	function changeDatabaseConnection($conn)
	{
		$dsn = NULL;
		switch ($conn) {
			case 'db':
				// If we are in the backend, we don't want any external concurrency
				// issues (eg. slow replication) to muck around with us. So we'll
				// grab db2 instead if we are in an editing mode
				if (SQ_IN_BACKEND || SQ_IN_LIMBO || SQ_PHP_CLI) {
					$dsn = SQ_CONF_DB2_DSN;
				} else {
					$dsn = SQ_CONF_DB_DSN;
				}
			break;

			case 'db2':
				$dsn = SQ_CONF_DB2_DSN;
			break;

			case 'db3':
				$dsn = SQ_CONF_DB3_DSN;
			break;

			case 'dbcache':
				// if empty, use DB2
				$dsn = (SQ_CONF_DBCACHE_DSN == '') ? SQ_CONF_DB2_DSN : SQ_CONF_DBCACHE_DSN;
			break;

			default:
				trigger_localised_error('SYS0129', E_USER_WARNING, $conn);
				return;
		}

		if (empty($this->_db_conns[$conn])) {

			$this->_db_conns[$conn] = DB::connect($dsn);

			if (DB::isError($this->_db_conns[$conn])) {
				if (!SQ_PHP_CLI) {
					header('HTTP/1.0 500 Internal Server Error');
				}
				trigger_error($this->_db_conns[$conn]->message, E_USER_ERROR);
			}

			// make sure all results are assoc arrays
			$this->_db_conns[$conn]->setFetchMode(DB_FETCHMODE_ASSOC);

			// if we are using oracle we want to set all tables and fields
			// to have lowercase names
			if ($this->_db_conns[$conn]->phptype == 'oci8') {
				$this->_db_conns[$conn]->setOption('portability', DB_PORTABILITY_ALL);
			}
			$this->_db_conns[$conn]->autoCommit(FALSE);
		}

		array_push($this->_db_stack, $conn);
		$this->db =& $this->_db_conns[$conn];

	}//end changeDatabaseConnection()


	/**
	* Restore the active DB connection to the previous one
	*
	* @return void
	* @access public
	*/
	function restoreDatabaseConnection()
	{
		$old_conn = array_pop($this->_db_stack);

		if (empty($this->_db_stack)) {
			unset($this->db);
			$this->db = NULL;
		} else {
			$conn = $this->_db_stack[count($this->_db_stack) - 1];
			$this->db =& $this->_db_conns[$conn];
		}

	}//end restoreDatabaseConnection()


	/**
	* Begin, Commit, or Rollback a transaction
	*
	* Call as many times as you like.
	* Ensures the transaction manager has been created.
	*
	* @param string	$type	the type of transaction to do (begin, commit, rollback)
	*
	* @return boolean
	* @access public
	*/
	function doTransaction($type)
	{
		if (!isset($this->tm) || get_class($this->tm) != 'transaction_manager') {
			require_once SQ_INCLUDE_PATH.'/transaction_manager.inc';
			$this->tm =& new Transaction_Manager();
		}
		// Use this when there is a transaction problem to create a log and then run this from the cmd line
		// and look for the fn with the odd number of calls
		// cat trans.log | awk '{ print $3 }' | sort | uniq -c # number of calls in a fn
		// cat trans.log | awk '{ print $5 }' | sort | uniq -c # total number of each type of call
		#$bt = debug_backtrace();
		#if (isset($bt[1])) {
		#	error_log ($this->_db_conns[$this->_db_stack[count($this->_db_stack) - 1]]->connection.' TRANS : '.$bt[1]['class'].'::'.$bt[1]['function'].' -- '.$type."\n", 3, SQ_SYSTEM_ROOT.'/data/private/logs/trans.log');
		#} else {
		#	error_log ($this->_db_conns[$this->_db_stack[count($this->_db_stack) - 1]]->connection.' TRANS : '.$bt[0]['file'].'::'.$bt[0]['line'].' -- '.$type."\n", 3, SQ_SYSTEM_ROOT.'/data/private/logs/trans.log');
		#}

		switch (strtolower($type)) {
			case 'begin':
				return $this->tm->begin($this->_db_conns[$this->_db_stack[count($this->_db_stack) - 1]]);
			case 'commit':
				return $this->tm->commit($this->_db_conns[$this->_db_stack[count($this->_db_stack) - 1]]);
			case 'rollback':
				return $this->tm->rollback($this->_db_conns[$this->_db_stack[count($this->_db_stack) - 1]]);
		}

	}//end doTransaction()


//--        SYSTEM ROLLBACK        --//


	/**
	* Prints a warning box to let the user know that they are viewing the system in rollback mode
	* Also includes the system time they are viewing and how long ago that was
	*
	* @return void
	* @access public
	*/
	function printRollbackWarning()
	{
		require_once SQ_FUDGE_PATH.'/general/datetime.inc';
		$then = strtotime($_SESSION['sq_rollback_view']['rollback_time']);
		$time_string = readable_datetime($then).' ('.easy_time_total(time() - $then, TRUE).' ago)';
		$warning_message = translate('rollback_mode_warning_message', $time_string);
		$exit_url = $_SERVER['PHP_SELF'].'?SQ_ACTION=rollback_view_stop';

		?>
		<div align="center">
		<style>
		.sq-rollback-warning {
			color: #FF0000;
			font-family:  Verdana, Arial, Helvetica, sans-serif;
			font-weight: bold;
			text-decoration: none;
			font-size: 11px;
		}
		</style>
		<br/>
		<table border="0" cellspacing="3" cellpadding="2" width="95%" bgcolor="#FF0000">
			<tr>
				<td bgcolor="#FFE5E8">
					<table border="0" cellspacing="2" cellpadding="2" width="100%">
						<tr>
							<td class="sq-rollback-warning" align="right"><?php echo translate('exit_rollback_view'); ?><?php sq_print_icon(sq_web_path('lib').'/web/images/icons/delete.png', '16', '16', translate('exit_rollback_view'), NULL, 'onclick="window.top.location=\''.$exit_url.'\'; return false;" style="cursor: pointer;"'); ?>
							</td>
						</tr>
						<tr>
							<td class="sq-rollback-warning" align="center"><u><?php echo translate('in_rollback_mode'); ?></u>
							</td>
						</tr>
						<tr>
							<td class="sq-rollback-warning"><?php echo $warning_message; ?>
							</td>
						</tr>
					</table>
				</td>
			</tr>
		</table>
		<br/>
		</div>
		<?php

	}//end printRollbackWarning()


	/**
	* Given a WHERE clause, this function will append extra where conditions to the clause
	* that allow selection of rollback entries based on the current rollback view mode settings
	*
	* @param string		$where			a current where clause without WHERE on the front
	*									NOTE: if the string begins with WHERE, it will be stripped
	* @param string		$table_alias	the table alias to add entries for
	*									eg. SELECT * FROM asset a --> $table_alias is 'a'
	*									so rollback queries for table 'a' get added, otherwise
	*									the entries will not be prefixed with an alias
	* @param string		$prefix			the prefix (eg 'WHERE' or 'AND') to put at the start of
	*									the condition
	* @param boolean	$is_table_alias	TRUE if the specified alias is a table alias, FALSE if it is
	*									a column prefix
	*
	* @return string
	* @access public
	*/
	function constructRollbackWhereClause($where='', $table_alias=NULL, $prefix='WHERE', $is_table_alias=TRUE)
	{
		// trim off a current prefix keyword if it exists
		$where = trim($where);
		if (strtoupper(substr($where, 0, strlen($prefix))) == strtoupper($prefix)) {
			$where = substr($where, strlen($prefix));
		}

		if (!SQ_ROLLBACK_VIEW) {
			// we are not in rollback view so we dont need anything special in the where clause
			if (empty($where)) {
				return '';
			} else {
				return ' '.$prefix.' '.$where;
			}
		}

		if (!is_null($table_alias)) {
			if ($is_table_alias) $table_alias .= '.';
		} else {
			$table_alias = '';
		}

		$then = $_SESSION['sq_rollback_view']['rollback_time'];

		if (!empty($where)) $where .= ' AND ';
		$where .= $table_alias.'sq_eff_from <= '.$this->db->quote($then).'
				  AND ('.$table_alias.'sq_eff_to IS NULL
					   OR '.$table_alias.'sq_eff_to > '.$this->db->quote($then).')';

		return ' '.$prefix.' '.$where;

	}//end constructRollbackWhereClause()


//--        LOCKING        --//


	/**
	* Attempts to acquire a lock
	*
	* returns TRUE on success or the error message on failure
	*
	* @param string	$lockid			the id of the lock
	* @param string	$source_lockid	the id of the lock that started this locking process
	* @param int	$expires		when the lock expires (timestamp or NULL)
	*								If expires is set to NULL, the lock will be acquired indefinitely (ie. it will never expire).
	*								If expires is set to zero, the lock will be acquired for the current time plus SQ_CONF_LOCK_LENGTH
	*
	* @return mixed boolean|string
	* @access public
	*/
	function acquireLock($lockid, $source_lockid='', $expires=0)
	{
		$current_lock = $this->getLockInfo($lockid);

		// is this asset already locked
		if (!empty($current_lock)) {
			// the user is asking to acquire a lock they already had
			// so just update the lock expiry date
			if ($current_lock['userid'] == $this->currentUserid()) {
				return $this->updateLock($lockid, $expires);
			} else {
				$user =& $this->am->getAsset($current_lock['userid']);
				return 'Cannot acquire lock, lock already held by "'.$user->name.'"';
			}
		}

		// if we have no source, we are the asset aquiring the locks
		if (empty($source_lockid)) $source_lockid = $lockid;

		if (!is_null($expires)) {
			$expires = (empty($expires)) ? (time() + SQ_CONF_LOCK_LENGTH) : (int) $expires;
			$expires = ts_iso8601($expires);
		}

		$expires = $this->db->quote($expires);
		if ($this->db->phptype == 'oci8') {
			$expires = 'TO_DATE('.$expires.', '.$this->db->quote('YYYY-MM-DD HH24:MI:SS').')';
		}

		$this->changeDatabaseConnection('db3');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$sql = 'INSERT INTO sq_lock
				(lockid, source_lockid, userid, expires)
				VALUES
				('.$this->db->quote($lockid).','.$this->db->quote($source_lockid).','.$this->db->quote($this->currentUserid()).','.$expires.')';
		$result = $this->db->query($sql);
		if (!assert_valid_db_result($result, '', TRUE, FALSE)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return 'Database Error';
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$this->restoreDatabaseConnection();
		return TRUE;

	}//end acquireLock()


	/**
	* Release the passed lock
	*
	* @param string	$lockid	the id of the lock to release
	*
	* @return mixed boolean|string
	* @access public
	*/
	function releaseLock($lockid)
	{
		$current_lock = $this->getLockInfo($lockid, FALSE, FALSE);
		if (empty($current_lock)) return TRUE;

		$this->changeDatabaseConnection('db3');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$sql = 'DELETE FROM sq_lock
				WHERE source_lockid = '.$this->db->quote($current_lock['source_lockid']);

		$result = $this->db->query($sql);
		if (!assert_valid_db_result($result, '', TRUE, FALSE)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return 'Database Error';
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		$this->restoreDatabaseConnection();
		return TRUE;

	}//end releaseLock()


	/**
	* Updates the expiry date of the lock
	*
	* If the current lock is being held indefinitely (ie. expires = NULL), the lock
	* expiry time will not be updated but the function will return TRUE. You must release
	* an indefinite lock before updating its expiry date.
	* returns TRUE on success, error message on failure
	*
	* @param string	$lockid		the id of the lock
	* @param int	$expires	when the lock expires (timestamp or NULL)
	*
	* @return mixed boolean|string
	* @access public
	*/
	function updateLock($lockid, $expires=0)
	{
		$current_lock = $this->getLockInfo($lockid);
		if (empty($current_lock) || empty($current_lock['expires'])) {
			return TRUE;
		}

		if (!is_null($expires)) {
			$expires = (!$expires) ? (time() + SQ_CONF_LOCK_LENGTH) : (int) $expires;
			$expires = ts_iso8601($expires);
		}

		$this->changeDatabaseConnection('db3');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$sql = 'UPDATE sq_lock
				   SET expires = '.$this->db->quote($expires).'
				WHERE source_lockid = '.$this->db->quote($current_lock['source_lockid']);

		$result = $this->db->query($sql);
		if (!assert_valid_db_result($result, '', TRUE, FALSE)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return 'Database Error';
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$this->restoreDatabaseConnection();
		return TRUE;

	}//end updateLock()


	/**
	* Returns an array of information about the passed lock
	*
	* @param string		$lockid			the id of the lock
	* @param boolean	$full_chain		return the info for every lock in the lock chain
	* @param boolean	$check_expires	check the expiry date and cleanup the lock if it has expired
	*
	* @return array
	* @access public
	*/
	function getLockInfo($lockid, $full_chain=FALSE, $check_expires=TRUE)
	{
		$this->changeDatabaseConnection('db3');
		$sql = 'SELECT lockid, source_lockid, userid, expires
				FROM sq_lock
				WHERE lockid = '.$this->db->quote($lockid);
		$result = $this->db->getRow($sql);
		assert_valid_db_result($result);

		if (!empty($result)) {
			$result['expires'] = empty($result['expires']) ? NULL : iso8601_ts($result['expires']);
			if ($check_expires && !empty($result['expires'])) {
				if ($result['expires'] < time()) {
					$this->releaseLock($lockid);
					$this->restoreDatabaseConnection();
					// this lock has expired
					return Array();
				}
			}

			if ($full_chain) {
				$result['chained_assets'] = Array();
				$sql = 'SELECT lockid, source_lockid, userid, expires
						FROM sq_lock
						WHERE source_lockid = '.$this->db->quote($result['source_lockid']).'
						  AND lockid <> '.$this->db->quote($lockid);
				$chain = $this->db->getAll($sql);
				assert_valid_db_result($chain);

				foreach ($chain as $lock_row) {
					$lock_row['expires'] = (empty($lock_row['expires'])) ? NULL : iso8601_ts($lock_row['expires']);
					$result['chained_assets'][$lock_row['lockid']] = $lock_row;
				}

			}//end if

		}//endif

		$this->restoreDatabaseConnection();
		return $result;

	}//end getLockInfo()


//--        SYSTEM MESSAGES        --//


	/**
	* Add a system message
	*
	* @param string	$msg	message
	*
	* @return void
	* @access public
	*/
	function addMessage($msg)
	{
		$this->_msgs[] = $msg;

	}//end addMessage()


	/**
	* Returns the system messages, array(string)
	*
	* @return array
	* @access public
	*/
	function messages()
	{
		return (empty($this->_msgs)) ? Array() : $this->_msgs;

	}//end messages()


//--        DATE/TIME        --//


	/**
	* Returns date time string for the passed timestamp
	*
	* @param int	$timestamp	timestamp to format, if null uses current timestamp
	*
	* @return string
	* @access public
	*/
	function datetime($timestamp=NULL)
	{
		return (is_null($timestamp)) ? date('d/m/Y H:i:s') : date('d/m/Y H:i:s', $timestamp);

	}//end datetime()


	/**
	* Returns date string for the passed timestamp
	*
	* @param int	$timestamp	timestamp to format, if null uses current timestamp
	*
	* @return string
	* @access public
	*/
	function date($timestamp=NULL)
	{
		return (is_null($timestamp)) ? date('d/m/Y') : date('d/m/Y', $timestamp);

	}//end date()


	/**
	* Returns time string for the passed timestamp
	*
	* @param int	$timestamp	timestamp to format, if null uses current timestamp
	*
	* @return string
	* @access public
	*/
	function time($timestamp=NULL)
	{
		return (is_null($timestamp)) ? date('H:i:s') : date('H:i:s', $timestamp);

	}//end time()


//--        PACKAGES        --//


	/**
	* Returns some information about the currently installed packages.
	*
	* Returns the code name, version, name and description for each of the currently installed packages
	* NOTE: do NOT ever cache the results of this function within the function because some parts of the install
	*       require the most up-to-date info about which packages are installed.
	*
	* @return boolean
	* @access public
	*/
	function getInstalledPackages()
	{
		$sql = 'SELECT code_name, version, name, description FROM sq_pkg';
		$packages = $this->db->getAll($sql);
		assert_valid_db_result($packages);
		return $packages;

	}//end getInstalledPackages()


//--        TRIGGERS        --//


	/**
	* Broadcast a trigger event
	*
	* @param string	$event_name		a valid trigger event name
	* @param object	&$broadcaster	an asset that broadcast the event
	* @param array	$parameters		an array of parameters that trigger conditions are expected to understand
	*
	* @return boolean
	* @access public
	*/
	function broadcastTriggerEvent($event_name, &$broadcaster, $parameters=NULL)
	{
		if (!$GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_TRIGGERS)) {
			return TRUE;
		}
		$tm =& $this->getTriggerManager();
		if (!is_null($tm)) {
			return $tm->broadcastEvent($event_name, $broadcaster, $parameters);
		} else {
			return TRUE;
		}

	}//end broadcastTriggerEvent()


	/**
	* Replacement function that allows request headers to be taken from CGI
	*
	* @return array
	* @access private
	*/
	function _getHeaders()
	{
		if (function_exists('apache_request_headers')) {
			// If the apache_request_headers function exists, we are
			// in an Apache module
			return apache_request_headers();
		} else if (isset($_ENV)) {
			// Otherwise, we are probably in CGI, where request headers are
			// dumped into $_ENV, so return that instead
			return $_ENV;
		}

	}//end _getHeaders()


	/**
	* Sees if an auth redirect is available, and then redirects if there is
	*
	* The redirect will be attempted on the same protocol (HTTP or HTTPS) that
	* the original request was made on. If the URL being redirected to does not
	* support the current protocol, it will use the protocol it can use instead.
	*
	* Returns boolean FALSE if no redirect exists, otherwise this function will
	* NOT return (as a result of exit() being called in do_redirect()).
	*
	* @return boolean
	* @access public
	*/
	function authRedirect()
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$current_url = current_url(FALSE, FALSE);
		$current_protocol = current_protocol();
		$root_url = $GLOBALS['SQ_SYSTEM']->am->getRootURL();

		$sql = 'SELECT
					auth_urlid
				FROM
					sq_ast_url
				WHERE
					urlid = '.$db->quote($root_url['urlid']);
		$auth_urlid = $db->getOne($sql);
		assert_valid_db_result($auth_urlid);

		// If there is no auth URL ID, return FALSE
		if (empty($auth_urlid)) return FALSE;

		$sql = 'SELECT
					url, http, https
				FROM
					sq_ast_url
				WHERE
					urlid = '.$db->quote($auth_urlid);
		$new_root_url = $db->getRow($sql);
		assert_valid_db_result($new_root_url);

		// If the protocol is valid, then use the same protocol, otherwise
		// use the one that is available
		if ($new_root_url[$current_protocol]) {
			$protocol = $current_protocol;
		} else {
			// If current protocol not valid, then only one must be selected...
			// and it's the wrong one
			$protocol = $new_root_url['https' ] ? 'https' : 'http';
		}

		// build the new URL, and redirect - this exits the script
		$url = $protocol.'://'.preg_replace('|^'.addslashes($root_url['url']).'|', $new_root_url['url'], $current_url);
		do_redirect($url);

		// Fall-through that should never be executed
		return FALSE;

	}//end authRedirect()


}//end class
?>
