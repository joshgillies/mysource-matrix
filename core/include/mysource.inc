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
* $Id: mysource.inc,v 1.421 2013/10/24 01:06:38 cupreti Exp $
*
*/

//require_once 'DB.php';
require_once SQ_LIB_PATH.'/MatrixDAL/MatrixDAL.inc';

require_once SQ_INCLUDE_PATH.'/mysource_object.inc';
require_once SQ_INCLUDE_PATH.'/asset_manager.inc';
require_once SQ_INCLUDE_PATH.'/locale_manager.inc';
require_once SQ_INCLUDE_PATH.'/performance_manager.inc';

// include the system version setttings
require_once SQ_INCLUDE_PATH.'/version.inc';

/**
* MySource
*
* Purpose
*    This is the object from which everything else is run
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.421 $
* @package MySource_Matrix
*/
class MySource extends MySource_Object
{
	/**
	* The current DB object
	*
	* @var object DB
	*/
	public $db;

	/**
	* Holds all open DB objects
	*
	* @var array
	* @see changeDatabaseConnection(), restoreDatabaseConnection()
	*/
	protected $_db_conns = Array();

	/**
	* The stack for DB connections
	*
	* @var array
	* @see changeDatabaseConnection(), restoreDatabaseConnection()
	*/
	protected $_db_stack = Array();


	/**
	* The stack for run levels
	*
	* @var array
	* @see setRunLevel(), restoreRunLevel()
	*/
	protected $_run_level_stack = Array();


	/**
	* The stack for contexts
	*
	* @var array
	* @see changeContext(), restoreContext()
	*/
	protected $_context_stack = Array();

	/**
	* The current Backend object
	*
	* @var object Backend
	*/
	public $backend;

	/**
	* @var object Asset_Manager
	*/
	public $am;

	/**
	* @var object Transaction_Manager
	*/
	public $tm;

	/**
	* @var object Performance_Manager
	*/
	public $pm;

	/**
	* @var object Messaging_Service
	*/
	protected $ms;

	/**
	* @var object Metadata_Manager
	*/
	protected $mm;

	/**
	* @var object HIPO_Herder
	*/
	protected $hh;

	/**
	* @var object Workflow_Manager
	*/
	protected $wm;

	/**
	* @var object File_Versioning
	*/
	protected $fv;

	/**
	* @var object Event_Manager
	*/
	protected $em;

	/**
	* @var object Tag_Manager
	*/
	protected $tag_manager;

	/**
	* @var object Locale_Manager
	*/
	public $lm;

	/**
	* @var object Trigger_Manager
	*/
	protected $trigger_manager;

	/**
	* @var object Log_Manager
	*/
	public $log_manager;

	/**
	* The logged in user
	*
	* @var object User
	*/
	public $user = NULL;

	/**
	* The object of the (main) asset being printed on the frontend
	*
	* @var object Asset
	*/
	public $frontend_asset;

	/**
	* Whether the logged in user is the root user or not
	*
	* @var boolean
	*/
	protected $_user_is_root = FALSE;

	/**
	* Whether the logged in user is a system admin or not
	*
	* @var boolean
	*/
	protected $_user_is_sys_admin = FALSE;

	/**
	* Whether the person using the system is a logged in user or not
	*
	* @var boolean
	*/
	protected $_user_is_public = TRUE;


	/**
	* Array of system messages
	*
	* @var array(string)
	*/
	protected $_msgs;


	/**
	* Array of globally defined variables
	*
	* @var array(string)
	*/
	protected $_global_defines;


	/**
	 * Deja Vu
	 *
	 * @var object Deja_Vu
	 */
	protected $_deja_vu;


	/**
	* Content that will be replaced at the highest level
	*
	* @var array(string)
	*/
	protected $_raw_contents = Array();


	/**
	* Constructor
	*
	*/
	function __construct()
	{

	}//end constructor



	/**
	 * Destructor
	 *
	 * Attempts to save locale manager in Deja Vu to improve performance.
	 *
	 * @return void
	 */
	function __destruct()
	{
		if (!empty($this->lm) && array_get_index($this->lm->_tmp, 'strings_modified', FALSE)) {
			$this->lm->locale_stack = Array();
			// Attempt to store me so that I don't have to go through all that again.
			$deja_vu = $this->getDejaVu();
			if ($deja_vu) {
				$deja_vu->remember(SQ_DEJA_VU_LOCALE, NULL, $this->lm);
			}
		}

		// force saving the session to memcache before respond to browser, otherwise, it would take forever, and concurrent session access issue happens.
		if(SQ_CONF_SESSION_HANDLER === 'memcache')
			session_write_close();

	}//end destructor


	/**
	* This initialises all the objects that we need to use, the reason that we can't do
	* this in the constructor is that the Asset_Manager uses the DB in it's constructor
	* in order to load the asset types but if initialise the Asset_Manager in our constructor
	* we don't exist and therefore cannot be accessed to get the DB for the Asset_Manager to use
	*
	* @return void
	* @access public
	*/
	public function init()
	{
		$this->_processUserIP();

		$this->changeDatabaseConnection('db');
		$this->changeContext(0);

		// initialise the asset manager as we are going to need it pretty much everywhere
		$this->am = new Asset_Manager();

		// initialise the performance manager
		$this->pm = new Performance_Manager();


		// Same with the locale manager; translations will be all around us.
		// Loading them is slow though, so first try deja_vu.
		$deja_vu = $GLOBALS['SQ_SYSTEM']->getDejaVu();
		if ($deja_vu) {
			$this->lm = $deja_vu->recall(SQ_DEJA_VU_LOCALE);
		}

		// If nothing, start from scratch.
		if (empty($this->lm)) {
			$this->lm = new Locale_Manager();
		}

		$this->lm->setCurrentLocale(SQ_CONF_DEFAULT_BACKEND_LOCALE);

		if (!SQ_PHP_CLI && $GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS)) {
			// we are accessing from the web, and the security subsystem is enabled in this run level,
			// so we use cookies for security

			$proxy_conf_file = SQ_DATA_PATH.'/private/conf/proxy_authentication.inc';
			if (file_exists($proxy_conf_file)) {
				// proxy configuration exists, let's see if we need to do anything
				require_once($proxy_conf_file);

				if (SQ_PROXY_URL_PORT_STRIP && isset($_SERVER['HTTP_HOST'])) {
					$url_parts = parse_url($_SERVER['HTTP_HOST']);

					// remove port information from url and reconstruct it
					if (isset($url_parts['port'])) {
						unset($url_parts['port']);
					}
					$_SERVER['HTTP_HOST'] = implode('', $url_parts);
				}
			}

			// Set up the session handler
			$session_handler = $this->getSessionHandlerClassName();
			call_user_func(Array($session_handler, 'init'));

			if(SQ_CONF_SESSION_HANDLER === 'memcache') {
				// memcache can only hold 30 days of cache, otherwise will fail silently
				ini_set('session.gc_maxlifetime', SQ_CONF_SESSION_GC_MAXLIFETIME > 2592000 ? 2592000 : SQ_CONF_SESSION_GC_MAXLIFETIME);
			}
			else {
				ini_set('session.gc_maxlifetime', SQ_CONF_SESSION_GC_MAXLIFETIME);
			}

			$session_set = FALSE;
			if (isset($_GET['SESSION_ID']) && (isset($_GET['SESSION_KEY']) || isset($_GET['SOAP_SESSION_KEY']))) {
				// asset map or someone has sent session key through the GET
				session_id($_GET['SESSION_ID']);
				$session_set = TRUE;
			}

			if (in_array(SQ_CONF_SESSION_HANDLER, array('memcache', 'redis'))) {
				$old_level = error_reporting(E_ERROR);
				session_start();
				error_reporting($old_level);
			} else {
				session_start();
			}

			$restore_session = FALSE;
			if (isset($_SESSION['activated']) === TRUE && $_SESSION['activated'] === 1) {
				// This session is already activated.
				// If we found the session is already activated.
                include_once SQ_FUDGE_PATH.'/general/security.inc';
				security_regenerate_session_id(FALSE);
				$new_session_id = session_id();
				if (isset($_COOKIE['SQ_SYSTEM_SESSION'])) {
					$new_session_id = $_COOKIE['SQ_SYSTEM_SESSION'];
				}//end if
				session_destroy();
				$this->_clearSession();
				session_id($new_session_id);

				if (in_array(SQ_CONF_SESSION_HANDLER, array('memcache', 'redis'))) {
					$old_level = error_reporting(E_ERROR);
					session_start();
					error_reporting($old_level);
				} else {
					session_start();
				}
				$restore_session = TRUE;
			}//end if

			$invalid_key = TRUE;
			if ($restore_session === FALSE) {
				if (isset($_GET['SOAP_SESSION_KEY']) === TRUE) {
					$invalid_key = ($this->getUniqueSOAPSessionKey() != $_GET['SOAP_SESSION_KEY']);
					if ($invalid_key === FALSE) {
						$_SESSION['activated'] = 1;
						define('SQ_SOAP_AUTH', '1');
					}//end if
				} else if (isset($_GET['SESSION_KEY']) === TRUE) {
					$invalid_key = ($this->getUniqueSessionKey() != $_GET['SESSION_KEY']);
				}//end if
			}//end if

			if (!defined('SQ_SOAP_AUTH')) {
				define('SQ_SOAP_AUTH', '0');
			}//end if

			// check to see if the session key is the same as that used to login
			if ($session_set && $invalid_key && !$restore_session) {
				define('SQ_ROLLBACK_VIEW', '0');
				define('SQ_TABLE_RUNNING_PREFIX', 'sq_');
				$ms = $this->getMessagingService();
				$log = $ms->newMessage(Array(), 'system.security.alert', Array());
				$log->parameters['remote_addr'] = $_SERVER['REMOTE_ADDR'];
				$log->parameters['sessionid'] = session_id();
				$log->parameters['userid'] = $_SESSION['userid'];
				$log->send();
				exit();
			}//end if

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

				$db   = MatrixDAL::getDb();

				if (MatrixDAL::getDbType() == 'pgsql') {
					$sql  = 'SELECT sq_set_rollback_timestamp(:then)';
				} else if (MatrixDAL::getDbType() == 'oci') {
					$sql = 'DECLARE BEGIN sq_common_pkg.sq_set_rollback_timestamp(:then); END;';
				}

				try {
					$query = MatrixDAL::preparePdoQuery($sql);
					MatrixDAL::bindValueToPdo($query, 'then', $then);
					MatrixDAL::execPdoQuery($query);
				} catch (DALException $e) {
					throw new Exception('Unable to set rollback timestamp due to database error: '.$e->getMessage());
				}

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

		// In performance timing mode, set current user if required
		 if(defined('SQ_IN_PERFORMANCE_TIMING') && SQ_IN_PERFORMANCE_TIMING) {
			 if(isset($_REQUEST['performance_setting_user'])) {
			 // set user to either Public user or default user based on GET and SESSION settings
			 // the default user in SESSION must be in performance manager only with caution, we don't want to expose security hole
			 if($_REQUEST['performance_setting_user'] === 'Default' && ($GLOBALS['SQ_SYSTEM']->user instanceof Public_User) && isset($_SESSION['SQ_PERFORMANCE_SETTING']['default_user'])) {
				 $user = $GLOBALS['SQ_SYSTEM']->am->getAsset($_SESSION['SQ_PERFORMANCE_SETTING']['default_user']);
				 $this->setCurrentUser($user);
			 }
			 else if ($_REQUEST['performance_setting_user'] === 'Public' && !($GLOBALS['SQ_SYSTEM']->user instanceof Public_User)) {
				 $user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('public_user');
				 $this->setCurrentUser($user);
			 }
			 }
		}
		// In result mode, print performance result
		else  if(defined('SQ_IN_PERFORMANCE_RESULT') && SQ_IN_PERFORMANCE_RESULT) {
				// restore to default user
			 if(isset($_REQUEST['performance_setting_user']) ) {
				 if($GLOBALS['SQ_SYSTEM']->user instanceof Public_User && isset($_SESSION['SQ_PERFORMANCE_SETTING']['default_user'])) {
				 $user = $GLOBALS['SQ_SYSTEM']->am->getAsset($_SESSION['SQ_PERFORMANCE_SETTING']['default_user']);
				 $this->setCurrentUser($user);
				 }
			}

			if(empty($GLOBALS['SQ_SYSTEM']->user) || !$GLOBALS['SQ_SYSTEM']->user->canAccessBackend()) exit(1);
			$this->pm->printResults();
			exit(1);
		}


		// process any global actions we have
		if (!empty($_REQUEST['SQ_ACTION'])) {
			$this->_processGlobalActions();
		}

		// If this session is new, select an appropriate context
		// and store it in the session so it sticks between pages
		if (isset($_SESSION) === TRUE) {
			$sandbox = array_get_index($_SESSION, SQ_SESSION_SANDBOX_INDEX, Array());

			// Get the backend override first, then if that's not set, then
			// go to the user preference
			$alternate_context_id = NULL;

			if ((SQ_IN_BACKEND === TRUE) || (SQ_IN_LIMBO === TRUE)) {
				if ($this->userPublic() === FALSE) {
					$alternate_context_id = array_get_index($sandbox, 'SQ_BACKEND_ALTERNATE_CONTEXT_ID');
				}
			}

			if ($alternate_context_id === NULL) {
				$alternate_context_id = array_get_index($sandbox, 'SQ_ALTERNATE_CONTEXT_ID');
			}

			if ($alternate_context_id !== NULL) {
				// If the context no longer exists (eg. the current
				// one was deleted through the backend interface),
				// get Matrix to select a new one.
				try {
					$context_exists = MatrixDAL::executeAll('core', 'getContexts', Array('contextids' => Array($alternate_context_id)));
					if (empty($context_exists) === TRUE) {
						$alternate_context_id = NULL;
					}
				} catch (DALException $e) {
					// Eep. This query hasn't even been baked yet.
					// Set it to the default context since there's
					// not much else that can be done here.
					$alternate_context_id = 0;
				}
			}
		} else if (SQ_PHP_CLI) {
			$alternate_context_id = 0;
		}//end if

		if ((isset($alternate_context_id) === FALSE) || ($alternate_context_id === NULL)) {
			$alternate_context_id = $this->getAlternateContext();

			if ((SQ_IN_BACKEND === TRUE) || (SQ_IN_LIMBO === TRUE)) {
				if (array_key_exists('SQ_BACKEND_ALTERNATE_CONTEXT_ID', array_get_index($_SESSION, SQ_SESSION_SANDBOX_INDEX, Array())) === FALSE) {
					$_SESSION[SQ_SESSION_SANDBOX_INDEX]['SQ_BACKEND_ALTERNATE_CONTEXT_ID'] = $alternate_context_id;
				}
			}
		}

		$this->changeContext($alternate_context_id);

	}//end init()


	/**
	* From here is where everything else happens, it starts the ball rolling
	*
	* @return void
	* @access public
	*/
	public function start()
	{
		// init perfromance timer
		$this->pm->begin();

		if(SQ_CONF_ENFORCE_FRAME_SAMEORIGIN)
			header('X-Frame-Options: SAMEORIGIN');

		// if the user is Up For Review, prompt the user to change password
		$require_password_change = ($this->user && !($this->user instanceof Public_User) && $this->user->status == SQ_STATUS_LIVE_APPROVAL);

		if (SQ_IN_BACKEND || SQ_IN_LIMBO) {

			if ($require_password_change) {
				$this->paintPasswordChange(translate('change_password'), translate('must_change_password_to_access_backend'));
				exit();
			}

			if ($this->user && !($this->user instanceof Public_User)) {
				require_once SQ_INCLUDE_PATH.'/backend.inc';
				$backend = new Backend();
				$this->backend = &$backend;

				$current_url     = current_url(FALSE, TRUE);
				$designid        = $GLOBALS['SQ_SYSTEM']->am->getDesignFromUrl($current_url);
				$design_no_frame = NULL;
				if (!empty($designid)) {
					$design = $GLOBALS['SQ_SYSTEM']->am->getAsset($designid['designid']);
					if ($design->type() === 'design_customisation') {
						// Get the design from the design customisation.
						$parent_designs = $GLOBALS['SQ_SYSTEM']->am->getParents($designid['designid'], 'design', TRUE);
						$effective_designid = NULL;
						if (count($parent_designs) > 0) {
							$effective_designid = array_keys($parent_designs);
							$effective_designid = $effective_designid[0];
						}//end if
					} else {
						$effective_designid = $designid['designid'];
					}//end else
					$design_no_frame = $GLOBALS['SQ_SYSTEM']->am->getDesignHideFrameValues($effective_designid);
				}//end if

				if ($design_no_frame) {
					define('SQ_DESIGN_NO_FRAME', 1);
				} else {
					define('SQ_DESIGN_NO_FRAME', 0);
				}//end else

				ob_start();
					$backend->paint();
				ob_end_flush();
			} else {
				$this->paintLogin(translate('login'), translate('must_login_to_access_backend'));
			}

		} else if (SQ_IN_LOGIN) {
			$this->paintLogin(translate('login'), translate('must_login_to_access_backend'));

		} else {
			// we are on the frontend
			if (!(SQ_IN_CRON || SQ_PHP_CLI) && defined('SQ_CONF_REDIRECT_URL_WITH_TRAILING_SLASH') && SQ_CONF_REDIRECT_URL_WITH_TRAILING_SLASH){
				$host = (isset($_SERVER['HTTP_HOST'])) ? $_SERVER['HTTP_HOST'] : FALSE;
				$uri = (isset($_SERVER['REQUEST_URI'])) ? $_SERVER['REQUEST_URI'] : FALSE;
				if ($host && $uri && substr($uri, -1) == '/') {
					$uri = rtrim($uri, '/');
					//do not redirect root URLs because apache may append a trailing slash
					if (!empty($uri)){
						$protocol = current_protocol().'://';
						header('HTTP/1.1 301 Moved Permanently');
						header('Location: '.$protocol.$host.$uri);
						exit;
					}
				}
			}

			// check for ./?a=xx type urls or normal
			if (isset($_REQUEST['a'])) {
				// requested asset is exclusively in the trash - don't display
				if ($this->am->assetInTrash($_REQUEST['a'], TRUE)) {
					$asset = NULL;
				} else {
					$asset = $this->am->getAsset($_REQUEST['a'], '', TRUE);
				}

				//Feature #4759: Asset ID to URL remaps.
				if (is_null($asset)){
					$rm = $this->am->getSystemAsset('remap_manager');
					$protocol = current_protocol();
					if ($rm->loadRemapFromURL($protocol, '?a='.$_REQUEST['a'])) exit();
					//Additionally Enhancement #4991
					$substr = substr($_REQUEST['a'], 0, strpos($_REQUEST['a'], '?'));
					if ($substr && $rm->loadRemapFromURL($protocol, '?a='.$substr)) exit();
				}
			} else {
				$asset = $this->am->getAssetFromURL(NULL, NULL, TRUE, TRUE);
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

				$rm = $this->am->getSystemAsset('remap_manager');
				if(!($rm->loadRemapFromCurrentURL())) {
					$url      = strip_url(current_url(FALSE, TRUE));
					$protocol = current_protocol();

					// okay, the remap manager could not remap this url, so
					// check to see if we can find a 404 page from a site matching this url
					$root_url = $this->am->getRootURL($url);

					// if we can get a site from the url,
					// try to delegate the 404 to its not found page
					if (!empty($root_url)) {
						$site = $this->am->getAsset($root_url['assetid']);
						if (!is_null($site)) {
							$page = $site->getSpecialPage('not_found');
							if (!is_null($page)) {
								$this->_paintNotFoundAsset($page, $site);
								exit(1);
							}
						}
					}

					// if we can't get a site from the url, or the
					// site does not have a 404 page, then try asking
					// the remap manager to remap to its 404 page
					if (!$rm->paintPageNotFoundRemap()) {
						header('HTTP/1.0 404 Not Found');

						$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
						// Send Cacheable Header based on cache manager default setting
						if (SQ_CONF_SEND_CACHEABLE_HEADER && SQ_CONF_SEND_404_CACHEABLE_HEADER && $cm->cacheableHeadersEnabledForCurrentProtocol()) {
							header('Pragma: cache');

							$browser_cache_expiry = $cm->attr('browser_cache_expiry');
							$acc_cache_expiry = $cm->attr('accelerator_cache_expiry');
							$acc_cache_string = empty($acc_cache_expiry) ? '' : "s-maxage=$acc_cache_expiry, ";

							if (empty($browser_cache_expiry)) {
								$browser_cache_expiry = $cm->attr('expiry');
							}
							$level = $cm->cacheControlLevel();
							header('Cache-Control: max-age='.$browser_cache_expiry.', '.$acc_cache_string.$level);
							header('Expires: '.gmdate('D, d M Y H:i:s', time() + $browser_cache_expiry).' GMT');
						}

						// last resort, display little green error
						trigger_localised_error('SYS0218', E_USER_NOTICE, $protocol.'://'.$url);
					}
				}//end else

				exit(1);

			}//end if is_null(asset)

			// we need to check if the asset is being requested by Funnelback for Indexing Purpose
			// if the condition matches then we need to send few extra headers along with regular contents
			// Check the user agent first so we don't load funnelback manager unless we really need to.
			if (isset($_SERVER['HTTP_USER_AGENT']) && strpos($_SERVER['HTTP_USER_AGENT'], 'Funnelback') !== FALSE) {
				$fbm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('funnelback_manager', TRUE);
				if (!is_null($fbm)) {
					$fb_user = $fbm->attr('user');
					if ($GLOBALS['SQ_SYSTEM']->currentUserId() == $fb_user) {
						$security_keys = $fbm->getAccessLocks($asset);
						header('X-Funnelback-Locks: '.$security_keys);

						header('X-Funnelback-SQUIZASSETYPE: '.$asset->type());
						header('X-Funnelback-last_modified: '.date('Y-m-d', $asset->updated).'T'.date('H:i:s', $asset->updated));
						if ($asset instanceof File) {
							header('X-Funnelback-file-title: '.$asset->attr('title'));
							header('X-Funnelback-file-name: '.$asset->name);
						}
						if ($asset->created_userid != 0) {
							$created_by = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->created_userid);
							header('X-Funnelback-author-name: '.$created_by->name);
							header('X-Funnelback-author-id: '.$created_by->id);
						}
						header('X-Funnelback-created: '.date('Y-m-d', $asset->created).'T'.date('H:i:s', $asset->created));
						header('X-Funnelback-ancestors: '.implode(' ', array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($asset->id))));

						// Add in the metadata
						$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
						require_once SQ_FUDGE_PATH.'/general/text.inc';

						$metadata = $mm->getMetadata($asset->id);
						foreach ($metadata as $info) {
							$name = $info[0]['name'];

							$value = $info[0]['value'];
							$keywords = retrieve_keywords_replacements($value);
							$replacements = Array();
							foreach ($keywords as $keyword) {
								$replacements[$keyword] = $asset->getKeywordReplacement($keyword);
							}//end foreach
							replace_keywords($value, $replacements);
							$name = $fbm->encodeText(htmlspecialchars($name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET));
							$value = $fbm->encodeText(htmlspecialchars($value, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET));
							header('X-Funnelback-metadata-'.$name.': '.$value);
						}//end foreach
					}//end if
				}//end if
			}//end if

			// In _performance view mode
			if (SQ_IN_PERFORMANCE) {
				if(empty($GLOBALS['SQ_SYSTEM']->user) || !$GLOBALS['SQ_SYSTEM']->user->canAccessBackend() || !$asset->writeAccess()) {
					$this->paintLogin(translate('login'), translate('must_login_to_access_backend'));
				} else {
					$this->pm->printFrames($asset);
				}
				exit(1);
			}

			// If we've loaded up an asset previously from the current url,
			// check if it's a site asset.
			// If it is, we don't need to work out the site again.
			$site = NULL;
			if (!is_null($asset)) {
				$asset_type = $asset->type();
				if ($asset_type == 'site' || $GLOBALS['SQ_SYSTEM']->am->isTypeDecendant($asset_type, 'site')) {
					$site = $asset;
				}
			}

			// It's not a site asset or we don't know what the asset is?
			if ($site === NULL) {
				$asset_url = current_url(FALSE, TRUE);
				// Muting errors by passing in the third parameter
				$asset_lineage = $GLOBALS['SQ_SYSTEM']->am->getLineageFromURL(NULL, $asset_url, TRUE);
				foreach ($asset_lineage as $asset_link) {
					$asset_type = $asset_link['type_code'];
					if ($asset_type == 'site' || $GLOBALS['SQ_SYSTEM']->am->isTypeDecendant($asset_type, 'site')) {
						$site_assetid = $asset_link['assetid'];
						if (!empty($site_assetid)) {
							$site = $GLOBALS['SQ_SYSTEM']->am->getAsset($site_assetid);
							break;
						}//end if
					}
				}
			}

			if (!is_null($site)) {
				$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_SITE', $site);
			}//end if

			// Are we an archived asset and we don't have the correct permission?
			// (We need read permission, which in Archived mode actually means
			// WRITE permission is required)
			if (($asset->status == SQ_STATUS_ARCHIVED) && !$asset->readAccess()) {
				// Don't get in the way of backend operations
				if (!(SQ_IN_BACKEND || SQ_IN_LIMBO)) {
					if ($GLOBALS['SQ_SYSTEM']->isGlobalDefineSet('CURRENT_SITE')) {
						$site = $GLOBALS['SQ_SYSTEM']->getGlobalDefine('CURRENT_SITE');
					} else {
						$url      = strip_url(current_url(FALSE, TRUE));
						$root_url = $this->am->getRootURL($url);
						if (!empty($root_url)) {
							$site = $this->am->getAsset($root_url['assetid']);
						}//end if
					}//end if

					if (!is_null($site)) {
						$page = $site->getSpecialPage('archive');
						if (!is_null($page)) {
							$this->_paintNotFoundAsset($page, $site);
							exit(1);
						}
					}
				}//end if
			}//end if

			$this->frontend_asset = &$asset;
			$this->broadcastTriggerEvent('trigger_event_asset_accessed', $asset);

			// See if caching is enabled for the asset
			$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');


			// buffer the content and do keyword replacement
			ob_start();
				if (basename($_SERVER['PHP_SELF']) == SQ_CONF_RECACHE_SUFFIX) {

					// Gather assetids from cache table
					$url = strip_url(current_url(TRUE, TRUE));
					$cm->clearCachedAssetsByUrl($url);
					$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_asset_recached', $asset);

					// Finally recache
					if (SQ_CONF_SEND_CACHEABLE_HEADER && $asset->canSendCacheableHeader() && $cm->cacheableHeadersEnabledForCurrentProtocol() && $cm->sendCacheableHeaders($asset->type(), $asset->id)) {
						$this->_sendHTTPHeaders($asset);
					} else {
						$this->_printAssetFrontend($asset);
					}
				}//end if
				// caching is enabled and asset is readable by public user, send HTTP headers
				// however not send if user have just logged out (see bug #3766)
				else if (basename($_SERVER['PHP_SELF']) != SQ_CONF_NOCACHE_SUFFIX &&
						 SQ_CONF_SEND_CACHEABLE_HEADER &&
						 $asset->canSendCacheableHeader() &&
						 empty($_POST) && $asset->status >= SQ_STATUS_LIVE &&
						 $asset->readAccess() &&
						 !(isset($_REQUEST['just_logged_out']) && $_REQUEST['just_logged_out']) &&
						 $cm->cacheableHeadersEnabledForCurrentProtocol() &&
						 $cm->sendCacheableHeaders($asset->type(), $asset->id)
						) {
					$this->_sendHTTPHeaders($asset);

				} else {
					$this->_printAssetFrontend($asset);
				}
				$content = ob_get_contents();
			// now clear off our buffer but only if had anything in it
			if (ob_get_length() !== FALSE) ob_end_clean();

			$this->replaceKeyword($content);

			// parse out self links if configured so
			if (SQ_CONF_NO_ASSET_SELF_LINKS) {
				$content = parse_self_links($content);
			}


			// Carryout highest level ./?a=xx URL translation
			$this->_translateMatrixURL($content);

			// translate all urls to performance mode style url _performance and target=_top
			$this->_translatePerformanceModeURL($content);

			echo $content;

			// Register the asset as "visited" in the frontend
			$this->_registerAssetAsVisited($asset);

		}//end else backend/limbo

		if (SQ_CONF_DEBUG & 4) {
			// report all the assets with a reference count > 0
			log_write($this->am->getForgottenItemsReport(), 'asset_cache', E_USER_NOTICE, FALSE);
		}

		if (!defined('SQ_DESIGN_NO_FRAME')) {
			define('SQ_DESIGN_NO_FRAME', 0);
		}//end if

		$this->_setApacheNotes();
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($cm);

		// finalize perfromance timer
		$this->pm->end();
	}//end start()


	/**
	* Sets the Apache Request Notes with the set information filled on the the Apache Configuration Screen
	*
	* @return void
	* @access private
	*/
	protected function _setApacheNotes()
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
	protected function _sendHTTPHeaders(&$asset)
	{
		$recache = basename($_SERVER['PHP_SELF']) == SQ_CONF_RECACHE_SUFFIX;
		// Don't send cacheable headers when recacheing (see bug #3947)
		if (!$recache) {
			header('Pragma: cache');
		}

		$contextid = $this->getContextId();
		$cache_key = 'cache_info';
		if ($contextid !== 0) {
			$cache_key .= '.'.$contextid;
		}

		// save/load the cache information of this asset
		$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');

		$cache_info = $cm->loadFromCache($asset->id, $asset->type(), $cache_key);

		$cache_expiry = $cm->getExpiry($asset->type(), $asset->id);
		$browser_cache_expiry = $cm->getBrowserCacheExpiry($asset->type(), $asset->id);
		$acc_cache_expiry = $cm->getAcceleratorCacheExpiry($asset->type(), $asset->id);
		$acc_cache_string = empty($acc_cache_expiry) ? '' : "s-maxage=$acc_cache_expiry, ";

		if (empty($browser_cache_expiry)) {
			$browser_cache_expiry = $cache_expiry;
		}

		if (!$recache) {
			$level = $cm->cacheControlLevel();
			header('Cache-Control: max-age='.$browser_cache_expiry.', '.$acc_cache_string.$level);
		}

		// not cached or has expired
		if ($cache_info === FALSE) {
			ob_start();
				$loaded_assetids = $GLOBALS['SQ_SYSTEM']->am->_asset_cache->getAssetKeys();
				$this->_printAssetFrontend($asset);
				$all_assetids = $GLOBALS['SQ_SYSTEM']->am->_asset_cache->getAssetKeys();
				$content = ob_get_contents();
			ob_end_clean();

			// work out the correct last updated time of this asset
			$assetids = array_diff($all_assetids, $loaded_assetids);

			// Remove any context info from asset cache key entries
			foreach ($assetids as &$assetid) {
				$assetid_bits = explode('\\', $assetid, 2);
				if ($assetid_bits > 1) {
					$assetid = $assetid_bits[1];
				}
			}

			$last_updated = $asset->getEffectiveLastUpdatedTime($assetids);

			// The Matrix Cache Expiry (a.k.a "Default Expiry") is used when saving to cache
			$info = Array(
						'last_modified'	=> $last_updated,
						'expires'		=> (time() + $cache_expiry),
					);
			$cm->saveToCache($asset->id, $asset->type(), $cache_key, serialize($info));
			// if cache_status is FALSE, then caching is probably disabled, expiry time = now

			// Calculate the "Browser Cache Expiry" value for use in HTTP headers
			$expiry_time =  time() + $browser_cache_expiry;

			if (!$recache) {
				header('Expires: '.gmdate('D, d M Y H:i:s', $expiry_time).' GMT');
				if (SQ_CONF_SEND_LAST_MODIFIED_HEADER ) {
					header('Last-Modified: '.gmdate('D, d M Y H:i:s', $info['last_modified']).' GMT');
				}
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

			// If the Browser Cache Expiry is set, caclculate the expiry time from now.
			// Otherwise, use the expiry time as supplied by the Matrix cache.
			$browser_cache_expiry = $cm->getBrowserCacheExpiry($asset->type(), $asset->id);

			if (empty($browser_cache_expiry)) {
				header('Expires: '.gmdate('D, d M Y H:i:s', $info['expires']).' GMT');
			} else {
				header('Expires: '.gmdate('D, d M Y H:i:s', time() + $browser_cache_expiry).' GMT');
			}

			if (SQ_CONF_SEND_LAST_MODIFIED_HEADER) {
				header('Last-Modified: '.$last_modified);
			}
			if (SQ_CONF_SEND_NOT_MODIFIED_HEADER && $not_modified) {
				header('HTTP/1.1 304 Not Modified');
				exit(1);
			} else {
				// not sending 304 header, print the content
				$this->_printAssetFrontend($asset);
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
	* @access public
	*/
	public function replaceKeyword(&$content, $additional_replacements=Array())
	{

		if ($this->getGlobalDefine('SQ_REPLACE_MYSOURCE_LEVEL_KEYWORDS', TRUE)) {
	    	// Replace any global keywords first
			replace_global_keywords($content);
		
			// Perform any additional replacements
			$keywords = retrieve_keywords_replacements($content, '', TRUE);
			foreach ($keywords as $word) {
				if (!isset($additional_replacements[$word])) {
					if(!isset($this->frontend_asset)) {
					    $additional_replacements[$word] = '';
					    continue;
					}

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

		// After the highest level keyword replacement, place these contents back
		foreach($this->_raw_contents as $hash => $replacement) {
			$content = str_replace($hash, $replacement, $content);
		}

	}//end replaceKeyword()


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
	public function setGlobalDefine($index, $value)
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
	public function unsetGlobalDefine($index)
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
	public function getGlobalDefine($index, $default=NULL)
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
	public function isGlobalDefineSet($index)
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
	public function paintLogin($heading, $msg)
	{
		// Attempt to perform an auth redirect. If there is one, then the script
		// will terminate inside the do_redirect() function and redirect. If
		// not, this function will return FALSE and we will fall through to the
		// login box.
		$this->authRedirect();

		$db = $this->db;
		header('HTTP/1.0 403 Forbidden');

		if (array_get_index($_REQUEST, 'SQ_BACKEND_PAGE') == 'header') {
			// we don't want to print a login box in the top frame, so reload the parent frame
			$redirect_location = replace_query_string_vars(Array('SQ_BACKEND_PAGE' => NULL));
			$redirect_location = strip_tags(addslashes($redirect_location));
			?>
			<script type="text/javascript">
				window.parent.document.location = '<?php echo $redirect_location; ?>';
			</script>
			<?php
			return;
		}
		if ((current_protocol() == 'http') && (SQ_CONF_FORCE_SECURE_LOGIN_URLS != '')) {
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
				$host = array_get_index($_SERVER, 'HTTP_HOST', '');
				do_redirect('https://'.$host.$_SERVER['REQUEST_URI']); // exits
			}
		}//end if http
		$GLOBALS['SQ_LOGIN_FORM'] = Array('paint' => 'login', 'heading' => $heading, 'msg' => $msg);

		// let's try do this via HTTP authentication
		if (SQ_CONF_ENABLE_HTTP_LOGIN && !empty($_GET['USE_HTTP_LOGIN']) && !isset($_SERVER['PHP_AUTH_USER'])) {
			// Blank out the session parameters regarding public user before sending the headers (see bug #4209)
			if ($this->userPublic($this->user)) {
				$_SESSION['user'] = NULL;
				$_SESSION['userid'] = NULL;
				$_SESSION['user_type_code'] = NULL;
			}

			header('WWW-Authenticate: Basic realm="MySource Matrix"');
			header('HTTP/1.0 401 Unauthorized');
		}

		if (empty($_GET['FORCE_BACKUP_LOGIN'])) {

			$current_asset = $this->am->getAssetFromURL(NULL, NULL, TRUE, TRUE);
			// if we can't find a current asset, use the root folder
			if (is_null($current_asset)) {
				$current_asset = $this->am->getSystemAsset('root_folder');
				if (is_null($current_asset)) {
					trigger_localised_error('SYS0248', E_USER_ERROR);
				}
			}

			$design_info = $this->am->getDesignFromURL(strip_url(current_url(FALSE, TRUE)), 'design::system::login');

			// we have found the design to use
			if ($design_info) {
				$design = $this->am->getAsset($design_info['designid'], $design_info['type_code']);
			} else {
				// OK, use the system login form
				$design = $this->am->getSystemAsset('login_design');
			}

			if (!is_null($design)) {
				ob_start();
				$design->paint($current_asset);
				$content = ob_get_clean();
				$this->replaceKeyword($content);
				echo $content;
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
	public function paintPasswordChange($heading, $msg)
	{
		$db = $this->db;

		$GLOBALS['SQ_PASSWORD_CHANGE_FORM'] = Array('paint' => 'password_change', 'heading' => $heading, 'msg' => $msg);

		if ((current_protocol() == 'http') && (SQ_CONF_FORCE_SECURE_LOGIN_URLS != '')) {
			// If we require secure login for this URL, we will also require it
			// for password changes.
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

		if (empty($_GET['FORCE_BACKUP_PASSWORD_CHANGE'])) {

			$current_asset = $this->am->getAssetFromURL(NULL, NULL, TRUE, TRUE);
			// if we can't find a current asset, use the root folder
			if (is_null($current_asset)) {
				$current_asset = $this->am->getSystemAsset('root_folder');
				if (is_null($current_asset)) {
					trigger_localised_error('SYS0248', E_USER_ERROR);
				}
			}

			$design_info = $this->am->getDesignFromURL(strip_url(current_url(FALSE, TRUE)), 'design::system::password_change');

			// we have found the design to use
			if ($design_info) {
				$design = $this->am->getAsset($design_info['designid'], $design_info['type_code']);
			} else {
				// OK, use the system password change form
				$design = $this->am->getSystemAsset('password_change_design');
			}

			if (!is_null($design)) {
				ob_start();
				$design->paint($current_asset);
				$content = ob_get_clean();
				$this->replaceKeyword($content);
				echo $content;
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
	public function setupUser()
	{
		if (!SQ_CONF_ALLOW_IP_CHANGE && isset($_SESSION['remote_addr']) && ($_SESSION['remote_addr'] != $_SERVER['REMOTE_ADDR'])) {
			// If not Using SOAP AUTH.
			if (!SQ_SOAP_AUTH) {
				if (isset($_SESSION['activated']) === FALSE || $_SESSION['activated'] != 1) {
					// IP Address has changed, and we are not authenticating via SOAP.
					$this->_logSecurityMsg('system.security.logout.ip_change', Array('old_ip' => $_SESSION['remote_addr'], 'new_ip' => $_SERVER['REMOTE_ADDR']));
					$this->_clearSession();
					$this->loginPublicUser();
					$this->generateLoginKey(TRUE);
					return;
				}//end if
			}//end if
		}//end if

		$this->user = NULL;

		if (!empty($_SESSION['userid']) && !empty($_SESSION['user']) && !empty($_SESSION['user_serialised_ts'])) {
			$userid = $_SESSION['userid'];
			// There is a serialised user object in the session; if it's not stale, unserialise and use it
			$user_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($userid), $_SESSION['user_type_code'], TRUE, 'updated');
			$user_updated_date = array_get_index($user_info, $userid, FALSE);
			if ($user_updated_date && $_SESSION['user_serialised_ts'] > iso8601_ts($user_updated_date) || (!$user_updated_date && SQ_ROLLBACK_VIEW)) {
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($_SESSION['user_type_code']);
				$this->user = unserialize($_SESSION['user']);
				$GLOBALS['SQ_SYSTEM']->am->rememberAsset($this->user);
			}
		}

		if (is_null($this->user) && !empty($_SESSION['userid'])) {
			// We failed to unserialise a user from the session, but there is nevertheless a userid
			// in the session, so the serialised version must have been stale.  So, let's load the asset
			// from the DB using the userid and re-save to session.
			$this->user = $GLOBALS['SQ_SYSTEM']->am->getAsset($_SESSION['userid'], $_SESSION['user_type_code']);
			if (!is_null($this->user)) {
				$_SESSION['user'] = serialize($this->user);
				$_SESSION['user_serialised_ts'] = time();
			}
		}

		if (is_null($this->user) || $this->user instanceof Public_User) {
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
			$this->loginPublicUser();
		}

		// Right, now we have a user of some sort; let's check their session preferences
		$security_actions = Array('login', 'logout', 'password_change', 'hipo', 'asset_map_request');
		$has_security_action = in_array(array_get_index($_REQUEST, 'SQ_ACTION'), $security_actions);
		if (array_get_index($_SESSION, 'user_last_access') && !$has_security_action) {

			$session_prefs = $this->getUserPrefs('user', 'SQ_USER_SESSION_PREFS', TRUE);
			$user_prefs = $this->_findValidSessionExpiry($session_prefs);

			// See if it's been too long since their last MatrixActivity(R)
			if (!empty($user_prefs['timeout'])) {
				if (time() - $_SESSION['user_last_access'] > $user_prefs['timeout']) {
					$this->_logSecurityMsg('system.security.logout.idle', Array('timeout' => $user_prefs['timeout']));
					$this->_clearSession();
					$this->loginPublicUser();
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
				if (!empty($user_prefs['max_length'])) {
					$session_expiry_ts = $_SESSION['user_last_authentication'] + $user_prefs['max_length'];
					$today_start_ts = strtotime('00:00:01 today');
					$tomorrow_start_ts = strtotime('00:00:01 tomorrow');
					if ($session_expiry_ts < $today_start_ts) {
						// it expired yesterday so clear the session / force a login right now
						$this->_logSecurityMsg('system.security.logout.staleauth', Array('timeout' => $user_prefs['max_length']));
						$this->_clearSession();
						$this->loginPublicUser();
						$this->generateLoginKey(TRUE);
					} else if ($session_expiry_ts < $tomorrow_start_ts) {
						// it's meant to expire today...
						if (time() - $_SESSION['user_last_access'] > SQ_CONF_EDITING_TIME) {
							// because they haven't done anything for a while we assume it's safe to log them out
							// and ask for re-authentication
							$this->_logSecurityMsg('system.security.logout.staleauth', Array('timeout' => $user_prefs['max_length']));
							$this->_clearSession();
							$this->loginPublicUser();
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
	public function _processGlobalActions()
	{
		$db = MatrixDAL::getDb();

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
				$pass_attr = new Asset_Attribute_Password();
				if (!$pass_attr->validateValue($password_new)) {
					$this->addMessage('Bad password values');
					return;
				}

				if ($this->user && !($this->user instanceof Public_User)) {
					// logged in, and the password is correct. change the password
					if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS)) {
						$run_level_changed = TRUE;
						$GLOBALS['SQ_SYSTEM']->setRunLevel($GLOBALS['SQ_SYSTEM']->getRunLevel() - SQ_SECURITY_PERMISSIONS);
					} else {
						$run_level_changed = FALSE;
					}

					$result = FALSE;
					if ($this->user->validatePassword($password_new, TRUE)) {
						$result = $this->user->setAttrValue('password', $password_new);
						$result = $result && $this->user->saveAttributes();
					}

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
					if (SQ_IN_LOGIN) {
						$current_url = current_url(TRUE,TRUE);
						do_redirect($current_url);
					}
					return;
				}
				if (!is_null($login_key) && $this->loginKey() == $login_key) {

					// get a list of all the installed authentication systems
					$auth_folder = $this->am->getSystemAsset('authentication_folder');
					if (is_null($auth_folder)) {
						trigger_localised_error('SYS0249', E_USER_ERROR);
						return;
					}
					$auth_systems = $auth_folder->getAuthSystems();
					// try each auth system in order to see if we can load a user asset
					$user = NULL;
					$user_status = NULL;
					foreach ($auth_systems as $systemid) {
						$system = $this->am->getAsset($systemid);
						if (is_null($system)) continue;

						// if we are loging in as root user dont worry about other authentication system
						// roadmap issue #811 Logging in as root should not interrogate any ldap bridges
						if ($login_username === 'root' && $system->name != 'Default Authentication') continue;

						$user = $system->authenticateUser($login_username, $login_password);
						if (!is_null($user)) $user_status = $user->status;
						// check that the user exists AND that it can login
						// (ie. it is not yet live, or has been locked)
						if (!is_null($user) && $user->canLogin()) {
							return $this->loginUser($user, '', TRUE);
						}
					}

					$display_message = $failure_reason = translate('user_not_found');
					if (!is_null($user_status)) {
						if ($login_username === 'root') {
							$display_message = $failure_reason = translate('root_user_disabled');
						} else if ($user_status & SQ_SC_STATUS_NOT_LIVE) {
							$failure_reason = translate('user_status_not_live');
						} else {
							$failure_reason = translate('user_asset_in_trash');
						}
					}

					$this->addMessage($display_message);
					// log failed attempt
					$this->_logSecurityMsg('system.security.login.failed', Array('login_name' => $login_username, 'failure_reason' => $failure_reason), TRUE);

				} else {

					// incorrect login key
					$this->addMessage(translate('login_key_incorrect'));
				}

				// deliberately don't have a break here so if the
				// login fails we make sure that we're logged out
			case 'logout' :
				$for_real = ($this->user && !($this->user instanceof Public_User));
				$old_user = &$this->user;

				// Log a slightly different system message if we logging out from secondary user session (after using "Login As...")
				if (isset($_SESSION['login_as_invoker_user_full_name'])) {
					$this->_logSecurityMsg('system.security.logout.secondary.manual', Array('secondary_user_name' => $_SESSION['login_as_invoker_user_full_name']));
				} else {
					$this->_logSecurityMsg('system.security.logout.manual');
				}

				// code below is added with respect to #5731 Login as and
				// stalled hippo. If we are switching back to the primary
				// user don't worry about chec king the stalled hippo jobs
				if (!isset($_SESSION['login_as_invoker_id']) && !isset($_SESSION['login_as_invoker_username'])) {
					$stalled_hipo_array = Array();
					$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
					$current_userid = $GLOBALS['SQ_SYSTEM']->currentUserId();
					// Only show hipo stalled job warning when it's really logging out from backend
					if ($for_real){
						$stalled_hipo_array = $hh->getJobsForUser($current_userid);
					}
					if (SQ_IN_BACKEND && !empty($stalled_hipo_array)) {
						$return_url = current_protocol().'://'.current_url(FALSE);

						// We need to handle the 'return to HTTP' stuff here in any case
						if (!$_SESSION['user_is_public'] && array_get_index($_SESSION, 'SQ_RETURN_TO_HTTP')) {
							unset($_SESSION['SQ_RETURN_TO_HTTP']);
							$return_url = 'http://'.current_url(FALSE);
						}
						$return_url = replace_query_string_vars(Array('SQ_ACTION' => NULL), $return_url);
						require_once dirname(__FILE__).'/hipo_reminder.inc';
						exit();
					}
				}

				if ($for_real) {
					$this->broadcastTriggerEvent('trigger_event_before_user_logout', $old_user, NULL);
				}

				// clear PHP authentication
				if (isset($_SERVER['PHP_AUTH_USER'])) { unset($_SERVER['PHP_AUTH_USER']); }
				if (isset($_SERVER['PHP_AUTH_PW'])) { unset ($_SERVER['PHP_AUTH_PW']); }

				// Restore a previous System Administrator account after user switching
				$invoker_user_id = NULL;
				if (isset($_SESSION['login_as_invoker_id']) && isset($_SESSION['login_as_invoker_username'])) {
					$invoker_user_id = $_SESSION['login_as_invoker_id'];
					$invoker_username = $_SESSION['login_as_invoker_username'];
					$this->_loginAsUser($invoker_username, $invoker_user_id, TRUE);
				} else {
					$this->_clearSession();
					if ($for_real) {
						$this->broadcastTriggerEvent('trigger_event_user_logout', $old_user, NULL);
					}

					$this->loginPublicUser();
					$this->generateLoginKey(TRUE);

					// Flag to know that logout request is made
					// $_REQUEST['SQ_ACTION'] cannot be used as its value is unset after once its used
					$_REQUEST['just_logged_out'] = 1;

					// Strip any SQ_ACTION from the URL and redirect.
					$new_get = $_GET;
					unset($new_get['SQ_ACTION']);
					$current_url = current_url(TRUE, FALSE);
					if (count($new_get) > 0) {
						$current_url .= '?'.make_raw_post_data($new_get);
					}
					do_redirect($current_url);
				}
			break;
			case 'real_logout' :
				$for_real = ($this->user && !($this->user instanceof Public_User));
				$old_user = &$this->user;
				if ($for_real) {
					$this->broadcastTriggerEvent('trigger_event_before_user_logout', $old_user, NULL);
				}
				$this->_logSecurityMsg('system.security.logout.manual');
				$this->_clearSession();
				if ($for_real) {
					$this->broadcastTriggerEvent('trigger_event_user_logout', $old_user, NULL);
				}
				$this->loginPublicUser();
				$this->generateLoginKey(TRUE);

				// Strip any SQ_ACTION from the URL and redirect.
				$new_get = $_GET;
				unset($new_get['SQ_ACTION']);
				$current_url = current_url(TRUE, FALSE);
				if (count($new_get) > 0) {
					$current_url .= '?'.make_raw_post_data($new_get);
				}
				do_redirect($current_url);

			break;
			case 'login_as' :
				$username = '';
				if (isset($_GET['user'])) {
					$username = trim($_GET['user']);

					/*
					* Don't allow the user to login again as themselves - that's a bit silly
					* Also don't allow the user to switch to "root" as no-one is worthy. Only an initial login
					* as "root" will provide access to this account
					*/

					if (($username != 'root') && (($username != $this->user->attr('username')) && (strlen($username) > 0))) {
						$this->_loginAsUser($username);
					}
				}
			break;
			case 'send_security_key' :
				require_once SQ_FUDGE_PATH.'/general/general.inc';
				$vars = &$_SESSION['SQ_SYSTEM_SECURITY_KEY'];

				// always regenerating the security key to stop painting same text for same session
				$vars['key'] = generate_security_key(strlen($vars['key']), $vars['include_uppercase'], $vars['include_numbers']);

				security_key_image(
									$vars['key_len'],
									$vars['key'],
									$vars['width'],
									$vars['height'],
									$vars['bg_colour'],
									$vars['text_colour'],
									$vars['border_colour'],
									$vars['zoom'],
									$vars['use_colours'],
									$vars['use_font'],
									$vars['font'],
									$vars['font_size'],
									$vars['min_angle'],
									$vars['max_angle'],
									$vars['x_start'],
									$vars['min_dist'],
									$vars['max_dist'],
									$vars['ttf_width'],
									$vars['ttf_height'],
									$vars['use_arc'],
									$vars['arc_colour'],
									$vars['include_uppercase'],
									$vars['include_numbers']
								  );
				exit(1);
			break;
			case 'attribute_image' :
				// grab the details we need to make this work
				$allowed_attributes = Array('email');
				if (isset($_GET['attribute']) && !empty($_GET['attribute'])) {
					$attribute = $_GET['attribute'];
					if (!in_array($attribute, $allowed_attributes)) {
						$attribute = '';
					}//end if
				}//end if
				if (isset($_GET['id']) && !empty($_GET['id'])) {
					$assetid = $_GET['id'];
					$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, '', TRUE);
					if (is_null($asset) || !empty($attribute)) {
						$text = $asset->attr($attribute);
						if (empty($text)) {
							$text = '';
						}//end if
					} else {
						$text = $asset->attr($attribute);
					}//end if
				}

				$this->_generateAttributeImage($text);
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
					$then = NULL;
					try {
						$bind_vars = Array(
										'assetid'	=> $assetid,
										'version'	=> $version,
									 );
						$then = MatrixDAL::executeOne('core', 'rollbackGetTimeStampFrom', $bind_vars);
					} catch (Exception $e) {
						throw new Exception('Could not find version '.$version.' of asset ID #'.$assetid.' in rollback system due to database error: '.$e->getMessage());
					}
					$_SESSION['sq_rollback_view'] = Array('rollback_time' => $then);
				}

				unset($_GET['version']);
			break;

			case 'rollback_view_stop' :
				if (isset($_SESSION['sq_rollback_view'])) {
					unset($_SESSION['sq_rollback_view']);
				}
				if (!empty($_REQUEST['assetid'])) {
					// Check if the backend asset in the rollback mode still exists in the present
					$sql = 'SELECT assetid FROM sq_ast WHERE assetid = :assetid';
					$query = MatrixDAL::preparePdoQuery($sql);
					MatrixDAL::bindValueToPdo($query, 'assetid', $_REQUEST['assetid']);
					if (!MatrixDAL::executePdoOne($query)) {
						unset($_REQUEST['assetid']);
					}
				}
			break;

			case 'hipo' :
				$hh = $this->getHipoHerder();
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

			case 'set_context' :
				// if this is a valid context name, then use it, otherwise ignore
				// eg. SQ_CONTEXT_NAME=Default%20Context
				$context_name = (isset($_GET['SQ_CONTEXT_NAME'])) ? $_GET['SQ_CONTEXT_NAME'] : '';
				unset($_GET['SQ_CONTEXT_NAME']);
				$context_data = MatrixDAL::executeAll('core', 'getContextByName', Array('name' => Array($context_name)));
				if (empty($context_data) === FALSE) {
					$contextid = $context_data[0]['contextid'];
					$_SESSION[SQ_SESSION_SANDBOX_INDEX]['SQ_ALTERNATE_CONTEXT_ID'] = $contextid;
				}
			break;

			case 'set_backend_context' :
				// Only make this take effect while we are in the backend
				// and we are logged in as some sort of user
				if ((SQ_IN_BACKEND === TRUE) || (SQ_IN_LIMBO === TRUE)) {
					if ($this->userPublic() === FALSE) {
						$context_name = (isset($_GET['SQ_CONTEXT_NAME'])) ? $_GET['SQ_CONTEXT_NAME'] : '';
						unset($_GET['SQ_CONTEXT_NAME']);
						$context_data = MatrixDAL::executeAll('core', 'getContextByName', Array('name' => Array($context_name)));
						if (empty($context_data) === FALSE) {
							$contextid = $context_data[0]['contextid'];
							$_SESSION[SQ_SESSION_SANDBOX_INDEX]['SQ_BACKEND_ALTERNATE_CONTEXT_ID'] = $contextid;
						}
					}
				}
			break;

			case 'clear_context' :
				if ((isset($_SESSION[SQ_SESSION_SANDBOX_INDEX]) === TRUE) && (isset($_SESSION[SQ_SESSION_SANDBOX_INDEX]['SQ_ALTERNATE_CONTEXT_ID']) === TRUE)) {
					unset($_SESSION[SQ_SESSION_SANDBOX_INDEX]['SQ_ALTERNATE_CONTEXT_ID']);
				}
			break;

			case 'asset_map_request' :
				require_once SQ_LIB_PATH.'/asset_map/asset_map.inc';
				$asset_map = new Asset_Map();
				$asset_map->process($this->backend);
				exit(1);
			break;

			case 'get_soap_downloaded_file' :
				$filename = basename(array_get_index($_GET, 'soap_filename', ''));
				$sessionid = session_id();
				$full_file_path = SQ_DATA_PATH.'/private/soap_action_file_responses/'.$sessionid.'/'.$filename;

				if (!file_exists($full_file_path)) {
					trigger_localised_error('FVER0007', E_USER_WARNING, $filename);
					exit();
				}

				if (version_compare(PHP_VERSION, '5.3.0') >= 0) {
					$finfo = finfo_open(FILEINFO_MIME_TYPE);
					$mime_type = finfo_file($finfo, $full_file_path);
				} else {
					$mime_type = mime_content_type($full_file_path);
				}
				header('Content-type: '.$mime_type);
				readfile($full_file_path);

				exit(1);
			break;

			case 'getToken' :
				echo (get_unique_token());
				exit(1);
			break;

			case 'verify_email' :
				if(isset($_GET['email_verification_token']) && isset($_SESSION['SQ_VERIFY_EMAIL_TOKEN'])) {
				    if($_GET['email_verification_token'] === $_SESSION['SQ_VERIFY_EMAIL_TOKEN']) {
					$_SESSION['SQ_VERIFY_EMAIL_VERIFIED'] = $_SESSION['SQ_VERIFY_EMAIL_RAW'];
				    }
				}
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
	public function loginPublicUser()
	{
		if (empty($this->user) || !($this->user instanceof Public_User)) {
			$this->user = $this->getPublicUser();
			$_SESSION['user_is_root']      = $this->_user_is_root      = FALSE;
			$_SESSION['user_is_sys_admin'] = $this->_user_is_sys_admin = FALSE;
			$_SESSION['user_is_public']    = $this->_user_is_public    = TRUE;
			$_SESSION['userid'] = $this->user->id;
			$_SESSION['user_type_code'] = $this->user->type();
			$_SESSION['remote_addr'] = (isset($_SERVER['REMOTE_ADDR'])) ? $_SERVER['REMOTE_ADDR'] : '';
			$_SESSION['user'] = serialize($this->user);
			$_SESSION['user_serialised_ts'] = time();
			$this->broadcastTriggerEvent('trigger_event_public_user_session_created', $this->user, NULL);
		}

	}//end loginPublicUser()


	/**
	* Log in as a predetermined user
	* The existing user must be a System Admin or Root User and be the administrator of that User for this to succeed
	* The asset ID of the previous user must be supplied if resuming a session after a "Login As" operation
	*
	* @param string		$username				The username with which to log in
	* @param string		$resuming_user_id_match	The asset ID of the resuming user
	* @param boolean	$resuming_session		Whether we are going back to the invoking account
	*
	* @return boolean
	* @access private
	*/
	private function _loginAsUser($username, $resuming_user_id_match='', $resuming_session=FALSE)
	{
		$logged_in = ($this->user && !($this->user instanceof Public_User));

		if ($logged_in && ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin() || $resuming_session)) {
			$old_user = $this->user;
			$invoking_user_id = $this->user->id;
			$invoking_username = $this->user->attr('username');
			$invoking_user_full_name = $this->user->name;

			// Be nice and store the user's navigation history so we make it easier when they resume
			$invoking_user_nav_history = NULL;
			if ($resuming_session) {
				if (isset($_SESSION['login_as_invoker_sq_nav_history'])) {
					$invoking_user_nav_history = $_SESSION['login_as_invoker_sq_nav_history'];
				}
			} else {
				if (isset($_SESSION['sq_nav_history'])) {
					$invoking_user_nav_history = $_SESSION['sq_nav_history'];
				}
			}

			// Log out the invoking user (System Admin)
			$this->broadcastTriggerEvent('trigger_event_before_user_logout', $this->user, NULL);
			$this->_logSecurityMsg('system.security.logout.manual');

			// Prepare a new session
			$this->_clearSession();
			$this->loginPublicUser();
			$this->generateLoginKey(TRUE);

			$this->broadcastTriggerEvent('trigger_event_user_logout', $old_user, NULL);

			// Switch to the secondary user without authenticating
			// Don't worry about the login key as we know we are performing the request ourselves

			// Get a list of all the installed authentication systems
			$auth_folder = $this->am->getSystemAsset('authentication_folder');
			if (is_null($auth_folder)) {
				trigger_localised_error('SYS0249', E_USER_ERROR);
				return;
			}
			$auth_systems = $auth_folder->getAuthSystems();

			// Try each auth system in order to see if we can load a user asset
			$secondary_user = NULL;
			$user_status = NULL;
			$failure_reason = translate('user_not_found');

			foreach ($auth_systems as $systemid) {
				$system = $this->am->getAsset($systemid);
				if (is_null($system)) continue;

				// Don't authenticate this time, but just locate the user and log in
				$secondary_user = $system->locateUser($username);
				if (!is_null($secondary_user)) {
					// Ensure that if we are resuming, that the username and ID match so we
					// are certain that we are logging back in to the correct account type
					if ($resuming_session && ($secondary_user->id != $resuming_user_id_match)) {
						// No go
						$this->_logSecurityMsg('system.security.login.secondary.failed', Array('login_name' => $old_user->attr('username'), 'secondary_login_name' => $username, 'failure_reason' => translate('user_switching_access_denied')), TRUE);
						return FALSE;
					}

					// user found with username and password supplied
					unset($_SESSION['user_login_attempts']);

					$user_status = $secondary_user->status;

					// check that the user exists AND that it can login
					// (ie. it is not yet live, or has been locked)
					if ($secondary_user->canLogin()) {
						// Store the details of the invoking user so we can switch back quickly
						if ($resuming_session) {
							// Alrighty, let's resume the user's previous account and their navigation history
							if (isset($invoking_user_nav_history)) {
								$_SESSION['sq_nav_history'] = $invoking_user_nav_history;
							}
							return $this->loginUser($secondary_user, '', TRUE);
						} else {
							/*
							* Ok to be perfectly clear, we're NOT allowed to log in as the Root User, and
							* System Administrators are not allowed to log in as other System Administrators.
							*
							* When referring to System Administrators here we are interested in real admins, not the Root User
							* which just happens to be a System Administrator too.
							*/
							$old_user_system_admin = (($old_user instanceof System_User) && !($old_user instanceof Root_User));
							$secondary_user_system_admin = (($secondary_user instanceof System_User) && !($secondary_user instanceof Root_User));

							$allowed_login = !(($secondary_user instanceof Root_User) || ($old_user_system_admin && $secondary_user_system_admin));

							if ($allowed_login) {
								$_SESSION['login_as_invoker_username'] = $invoking_username;
								$_SESSION['login_as_invoker_user_full_name'] = $invoking_user_full_name;
								$_SESSION['login_as_invoker_id']  = $invoking_user_id;
								if (isset($invoking_user_nav_history)) {
									$_SESSION['login_as_invoker_sq_nav_history'] = $invoking_user_nav_history;
								}

								// Ok all is good - login as the user we are after
								return $this->loginUser($secondary_user, $old_user->name, TRUE);
							} else {
								$failure_reason = translate('user_switching_access_denied');
								$user_status = NULL;
							}

						}//end else
					}//end if (secondary user can login)

				}//end if (secondary user not null)

			}//end foreach (authentication systems)

			if (!is_null($user_status)) {
				if ($user_status & SQ_SC_STATUS_NOT_LIVE) {
					$failure_reason = translate('user_status_not_live');
				} else {
					$failure_reason = translate('user_asset_in_trash');
				}
			}

			$this->addMessage(translate('user_not_found'));

			// Log failed attempt
			$this->_logSecurityMsg('system.security.login.secondary.failed', Array('login_name' => $old_user->attr('username'), 'secondary_login_name' => $username, 'failure_reason' => $failure_reason), TRUE);
		}

		return FALSE;

	}//end _loginAsUser()


	/**
	* Try to login a user who has already been HTTP-authenticated by some external mechanism
	*
	* @return void
	* @access public
	*/
	protected function _loginHttpAuthenticatedUser()
	{
		if (array_get_index($_REQUEST, 'SQ_ACTION')) return;
		$http_username = array_get_index($_SERVER, SQ_CONF_HTTP_LOGIN_VAR);
		if (is_null($http_username) || ($http_username === '')) {
			return;
		}

		// get a list of all the installed authentication systems
		$auth_folder = $this->am->getSystemAsset('authentication_folder');
		if (is_null($auth_folder)) {
			trigger_localised_error('SYS0249', E_USER_ERROR);
			return;
		}
		$auth_systems = $auth_folder->getAuthSystems();

		// try each auth system in order to see if we can load a user asset
		$user = NULL;
		foreach ($auth_systems as $systemid) {
			$system = $this->am->getAsset($systemid);
			if (is_null($system)) continue;
			$user = $system->authenticateHttpUser($http_username);
			if (!is_null($user)) return $this->loginUser($user);
		}

	}//end _loginHttpAuthenticatedUser()


	/**
	* Check the user authentication. If the user is authenticated then it assigns the user and returns TRUE else returns FALSE
	*
	* @return void
	* @access public
	*/
	protected function _authenticateHttpUser()
	{
		if (array_get_index($_REQUEST, 'SQ_ACTION')) return;

		if (!(array_get_index($_SERVER, 'PHP_AUTH_USER') && array_get_index($_SERVER, 'PHP_AUTH_PW'))) {
			return;
		}
		// get a list of all the installed authentication systems
		$auth_folder = $this->am->getSystemAsset('authentication_folder');
		if (is_null($auth_folder)) {
			trigger_localised_error('SYS0249', E_USER_ERROR);
			return;
		}
		$auth_systems = $auth_folder->getAuthSystems();
		// try each auth system in order to see if we can load a user asset
		$user = NULL;
		foreach ($auth_systems as $systemid) {
			$system = $this->am->getAsset($systemid);
			if (is_null($system)) continue;
			$user = $system->authenticateUser($_SERVER['PHP_AUTH_USER'], $_SERVER['PHP_AUTH_PW']);
			// we can not do redirect for http authed user, or it would be infinate loop
			if (!is_null($user)) return $this->loginUser($user);
		}

		// clear the PHP_AUTH_USER / PASS if authentication failed
		unset($_SERVER['PHP_AUTH_USER']);
		unset($_SERVER['PHP_AUTH_PW']);

		return;

	}//end _authenticateHttpUser()


	/**
	* Log in the specified user, setting the session etc as appropriate
	*
	* SHOULD BE USED WITH CAUTION - the assumption is that this user has already been authenticated
	*
	* @param User	$user						The user object to log in
	* @param string	$invoking_user_full_name	The name of the user who has invoked the login (only used from the "Login As..." function)
	* @param bool	$redirect					If we should do a redirect after login to remove the SQ_ACTION URL param
	*
	* @return void
	* @access public
	*/
	public function loginUser(User $user, $invoking_user_full_name = '', $redirect=FALSE)
	{
		if (is_null($user)) return FALSE;

		if (!$this->setCurrentUser($user)) return FALSE;

		$old_session_id = session_id();

		// regenerate the session id to protect against session fixation attacks
        include_once SQ_FUDGE_PATH.'/general/security.inc';
        security_regenerate_session_id();
		// when logging in we wipe the previous session file
		$sess_filename = session_save_path().'/sess_'.$old_session_id;
		@unlink($sess_filename);
		$this->_updateSessionCookie(TRUE);

		$_SESSION['userid'] = $user->id;
		$_SESSION['user_type_code'] = $user->type();
		$_SESSION['user'] = serialize($user);
		$_SESSION['user_serialised_ts'] = time();
		if (!$this->_user_is_public) {
			$_SESSION['remote_addr'] = $_SERVER['REMOTE_ADDR'];
		}

		// log a message so we know someone has logged in
		if (empty($invoking_user_full_name)) {
			$this->_logSecurityMsg('system.security.login');
		} else {
			$this->_logSecurityMsg('system.security.login.secondary', Array('secondary_user_name' => $invoking_user_full_name));
		}

		// Get login context if any. If the value returned is numeric, it's
		// a context ID. (Otherwise it's something like 'default', ie. don't
		// change it.) Save it to the session...
		$login_contextid = $this->getUserPrefs('user', 'SQ_USER_SET_CONTEXT_ON_LOGIN', TRUE);

		if (is_numeric($login_contextid) === TRUE) {
			$_SESSION[SQ_SESSION_SANDBOX_INDEX]['SQ_ALTERNATE_CONTEXT_ID'] = $login_contextid;
			$this->changeContext($login_contextid);
		}

		// generate a new login key so that when they try and login next
		// they have to re-enter the details
		$this->generateLoginKey(TRUE);

		$_SESSION['user_is_root'] = $this->_user_is_root;
		$_SESSION['user_is_sys_admin'] = $this->_user_is_sys_admin;
		$_SESSION['user_is_public'] = $this->_user_is_public;
		$_SESSION['user_last_access'] = time();
		$_SESSION['user_last_authentication'] = time();


		// set user last login date for real user asset
		if(strpos($user->id, ':') === FALSE ) {
			$this->setRunLevel(SQ_RUN_LEVEL_FORCED);
			$user->setAttrValue('login_date', date("Y-m-d H:i:s"));
			$user->setAttrValue('login_ip', filter_var($_SERVER['REMOTE_ADDR'], FILTER_VALIDATE_IP));
			$user->saveAttributes();
			$this->restoreRunLevel();
		}

		// Do pass any event data since we can get it from $_SESSION
		$event_data = Array('password' => array_get_index($_POST, 'SQ_LOGIN_PASSWORD'));
		$this->broadcastTriggerEvent('trigger_event_user_login', $user, $event_data);

		if (!$_SESSION['user_is_public'] && array_get_index($_SESSION, 'SQ_RETURN_TO_HTTP')) {
			unset($_SESSION['SQ_RETURN_TO_HTTP']);

			// Strip login suffix if we are "in login" as we have already been authenticated
			do_redirect('http://'.current_url(FALSE, SQ_IN_LOGIN)); // exits
		}

		if (SQ_IN_LOGIN) {
			$current_url = current_url(TRUE, TRUE);
			do_redirect($current_url);
		}

		// Strip any SQ_ACTION from the URL and redirect.
		if($redirect) {
			$new_get = $_GET;
			unset($new_get['SQ_ACTION']);
			$current_url = current_url(TRUE, FALSE);
			if (count($new_get) > 0) {
				$current_url .= '?'.make_raw_post_data($new_get);
			}
			do_redirect($current_url);
		}
	
		return TRUE;

	}//end loginUser()


	/**
	* Sets the current user of the system to be the specified user
	*
	* SHOULD BE USED WITH CAUTION - no authentication is done inside here
	* This is generally to be used by scripts etc - the session is not updated with the new user's details
	*
	* @param User	$user	the user to make current
	*
	* @return boolean
	* @access public
	*/
	public function setCurrentUser(User $user)
	{
		if (!($user instanceof User)) {
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
	public function restoreCurrentUser()
	{
		if (empty($this->_tmp['current_user_cache'])) {
			$user = NULL;
		} else {
			list($userid, $type_code) = array_pop($this->_tmp['current_user_cache']);
			$user = $this->am->getAsset($userid, $type_code);
		}

		// They can only become current user if they can login
		if (is_null($user) || !$user->canSetAsCurrentUser()) {
			$this->user = $this->getPublicUser();
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
	public function getUniqueSessionKey()
	{
		$key =  $_SESSION['userid'].
				$_SESSION['user_type_code'];

		if (isset($_SESSION['SQ_LOGIN_KEY'])) {
				$key .= $_SESSION['SQ_LOGIN_KEY'];
		}

		if (!SQ_CONF_ALLOW_IP_CHANGE) {
			$key .= $_SESSION['remote_addr'];
		}

		return md5($key);

	}//end getUniqueSessionKey()


	/**
	* Returns a unique session key
	*
	* the key can be used to strengthen the sending of the
	* session id as a $_GET variable
	*
	* @return string
	* @access public
	*/
	public function getUniqueSOAPSessionKey()
	{
		$key =  $_SESSION['userid'].
				$_SESSION['user_type_code'];

		if (isset($_SESSION['SQ_LOGIN_KEY'])) {
			$key .= $_SESSION['SQ_LOGIN_KEY'];
		}//end if

		return md5($key);

	}//end getUniqueSOAPSessionKey()


	/**
	* Generates a new login key and returns it
	*
	* @param boolean	$force_new_key	if TRUE the key will regenerated even if
	*									there is already a value save in the session
	*
	* @return string
	* @access public
	*/
	public function generateLoginKey($force_new_key=FALSE)
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
	public function loginKey()
	{
		return (isset($_SESSION['SQ_LOGIN_KEY'])) ? $_SESSION['SQ_LOGIN_KEY'] : '';

	}//end loginKey()


	public function getSessionHandlerClassName()
	{
		if (SQ_CONF_SESSION_HANDLER === '') {
			$session_handler = 'Session_Handler_Default';
		} else {
			$session_handler = 'Session_Handler_'.ucwords(SQ_CONF_SESSION_HANDLER);
		}

		try {
			$this->am->includeAsset(strtolower($session_handler));
		} catch (Exception $e) {
			// Throwing a fatal may be a bit heavy-handed, but the alternative
			// is to throw a notice, and this breaks when other headers need
			// to be sent. Best to make them fix it first
			trigger_error('Cannot start MySource user session; session handler \''.SQ_CONF_SESSION_HANDLER.'\' does not exist', E_USER_ERROR);
		}

		return $session_handler;

	}//end getSessionHandlerClassName()


	/**
	* Update the session cookie with the appropriate expiry time given the kind of user we are
	*
	* Note: this function must be called before any other cookies are set or it will clear their set-cookie headers
	* 
	* @param boolean	$force_send	force send cookie header to clear previous set-cookie headers
	*
	* @return void
	* @access public
	*/
	protected function _updateSessionCookie($force_send=FALSE)
	{
		$session_id = session_id();
		if (empty($session_id)) return FALSE;

		$session_prefs = $this->getUserPrefs('user', 'SQ_USER_SESSION_PREFS', TRUE);
		$user_prefs = $this->_findValidSessionExpiry($session_prefs);

		$time_to_live = $user_prefs['persist']; // 1 or 0
		if (!empty($time_to_live)) {
			$time_to_live = !empty($user_prefs['timeout']) ? ($user_prefs['timeout'] + 3600) : 0; // cookie should last 1 extra hour
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
		if (defined('SQ_CONF_SYSTEM_PARENT_DOMAINS') && SQ_CONF_SYSTEM_PARENT_DOMAINS !== '') {
			$domain = strip_url(sq_web_path('root_url'), TRUE);
			$pos =  strpos($domain, '/');
			if($pos !== FALSE) $domain = substr($domain, 0, $pos);
			// Remove the port number from the host part, if any.
			$domain = preg_replace('|:\d+$|', '', $domain);
			$new_domain = '';
			$parent_domains = explode("\n", SQ_CONF_SYSTEM_PARENT_DOMAINS);
			if (is_array($parent_domains)) {
				foreach ($parent_domains as $parent_domain) {
					$parent_length = strlen($parent_domain);
					if ((strlen($domain) >= $parent_length) && (substr($domain, -$parent_length) === $parent_domain)) {
						// our current domain is a subdomain of this parent domain, so we will
						// actually set the cookie for the parent domain and its children
						$new_domain = '.'.$parent_domain;
						$session_params['path'] = '/';
						break;
					}
				}
			}
			if(!empty($new_domain)) $header .= "domain=$new_domain; ";
		}
		if (!empty($session_params['path'])) {
			$header .= "path=$session_params[path]; ";
		}

		if (isset($session_params['httponly']) && $session_params['httponly']) {
			$header .= "HttpOnly; ";
		}

		if (isset($session_params['secure']) && $session_params['secure']) {
			$header .= "secure; ";
		}

		// we only need to send set-cookie header here if it is needed
		// otherwise session handler should have already sent set-cookie header, no need to duplicate
		if($force_send || (isset($new_domain) && !empty($new_domain)) || !empty($time_to_live))
		    header($header, FALSE);

	}//end _updateSessionCookie()


	/**
	* Find the largest valid session expiry for the current user
	*
	* @param array	$session_prefs	An array of all the current preferences
	*
	* @return array
	* @access public
	*/
	public function _findValidSessionExpiry($session_prefs)
	{
		$prefs = Array(
					'persist'		=> '0',
					'timeout'		=> '0',
					'max_length'	=> '0',
				 );
		$current_ip = (isset($_SERVER['REMOTE_ADDR'])) ? $_SERVER['REMOTE_ADDR'] : '';
		$remote_ip_is_ipv6 = isIpV6($current_ip);

		$patt = '/\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\b/';
		if (is_array($session_prefs)) {
			foreach ($session_prefs as $id => $sess_pref) {
				$temp = Array(
						'persist'		=> '0',
						'timeout'		=> '0',
						'max_length'	=> '0',
						);

				// Check the user type restriction
				if (isset($sess_pref['user_types']) && !empty($sess_pref['user_types'])) {
					$user_types = explode(',', $sess_pref['user_types']);
					$current_user_type = '';
					if (empty($this->user)) {
						$current_userid = $this->currentUserId();
						if ($current_userid) {
							$current_user = $GLOBALS['SQ_SYSTEM']->am->getAsset($current_userid);
							if (!is_null($current_user)) {
								$current_user_type = $current_user->type();
							}
							$GLOBALS['SQ_SYSTEM']->am->forgetAsset($current_user);
						}
					} else {
						$current_user_type = $this->user->type();
					}

					if (!empty($current_user_type) && in_array($current_user_type, $user_types)) {
						$prefs['persist'] = $sess_pref['persist'];
						$prefs['timeout'] = $sess_pref['timeout'];
						$prefs['max_length'] = $sess_pref['max_length'];
						break;
					}
				}//end if

				// Check that the current IP falls between the 'to' and 'from' field (also check an IP was passed)
				if (isset($sess_pref['ip_from']) && !empty($sess_pref['ip_from']) && preg_match($patt, $sess_pref['ip_from']) && !$remote_ip_is_ipv6) {
					$current_ip_value = ip2long($current_ip);
					$ip_from_value = ip2long($sess_pref['ip_from']);
					if (($current_ip_value > -1) && ($current_ip_value != FALSE) && ($ip_from_value > -1) && ($ip_from_value != FALSE)) {
						if (!empty($current_ip) && ($current_ip == $sess_pref['ip_from'])) {
							// Match
							$prefs['persist'] = $sess_pref['persist'];
							$prefs['timeout'] = $sess_pref['timeout'];
							$prefs['max_length'] = $sess_pref['max_length'];
							break;
						} else if (!empty($current_ip) && ($current_ip_value > $ip_from_value)) {
							if (isset($sess_pref['ip_to']) && !empty($sess_pref['ip_to']) && preg_match($patt, $sess_pref['ip_to'])) {
								if ((ip2long($sess_pref['ip_to']) > -1) && (ip2long($sess_pref['ip_to']) != FALSE)) {
									if ($current_ip_value < ip2long($sess_pref['ip_to'])) {
										// Match
										$prefs['persist'] = $sess_pref['persist'];
										$prefs['timeout'] = $sess_pref['timeout'];
										$prefs['max_length'] = $sess_pref['max_length'];
										break;
									} else {
										// No Match
										continue;
									}//end if
								}//end if
							} else {
								// No Match
								continue;
							}//end if
						} else {
							// No Match
							continue;
						}//end if
					}//end if
				} else if (!empty($sess_pref['ip_from']) && isIpV6($sess_pref['ip_from']) && $remote_ip_is_ipv6) {
					if (!isIpV6($sess_pref['ip_from'])) {
						continue;
					}
					$remote_ip_bits = ipv6ToBits($current_ip);
					$ip_from_bits = ipv6ToBits($sess_pref['ip_from']);
					if ($remote_ip_bits === FALSE || $ip_from_bits === FALSE) {
						continue;
					}

					// Check if the IP is the first one in the range
					if (strcmp($remote_ip_bits, $ip_from_bits) === 0) {
						// Its a match!
						$prefs['persist'] = $sess_pref['persist'];
						$prefs['timeout'] = $sess_pref['timeout'];
						$prefs['max_length'] = $sess_pref['max_length'];
						break;
					}

					// If we don't have "to" specifed for the IP range, we don't have the match
					if (empty($sess_pref['ip_to']) || !isIpV6($sess_pref['ip_to'])) {
						continue;
					}

					$ip_to_bits = ipv6ToBits($sess_pref['ip_to']);
					if ($ip_to_bits === FALSE) {
						continue;
					}

					// If the IP lies within the range
					if (strcmp($remote_ip_bits, $ip_to_bits) === 0 || (strcmp($ip_from_bits, $remote_ip_bits) < 0 && strcmp($remote_ip_bits, $ip_to_bits) < 0)) {
						$prefs['persist'] = $sess_pref['persist'];
						$prefs['timeout'] = $sess_pref['timeout'];
						$prefs['max_length'] = $sess_pref['max_length'];
						break;
					} else {
						continue;
					}

				}//end else if ipv6

				// No IP Restriction found, so continue
				$temp['persist'] = $sess_pref['persist'];
				$temp['timeout'] = $sess_pref['timeout'];
				$temp['max_length'] = $sess_pref['max_length'];

				// Look for the largest general result
				foreach ($temp as $key => $value) {
					if ($value > $prefs[$key]) $prefs[$key] = $value;
				}//end foreach
			}//end foreach
		}//end if

		// Return the results
		return $prefs;

	}//end _findValidSessionExpiry()


	/**
	* Clear the user's session of all data
	*
	* @return void
	* @access private
	*/
	protected function _clearSession()
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
	protected function _logSecurityMsg($code, $extra_replacements=Array(), $failed_login=FALSE)
	{
		if ($this->userPublic($this->user) && !$failed_login) {
			return;
		}
		$ms = $this->getMessagingService();
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
	public function currentUser($user)
	{
		return ($this->user && $this->user->id == $user->id);

	}//end currentUser()


	/**
	* Returns the userid of the current user, or zero if none present
	*
	* @return mixed int|string
	* @access public
	*/
	public function currentUserId()
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
	public function &getPublicUser()
	{
		$public_user = $this->am->getSystemAsset('public_user');
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
	public function userRoot($user=NULL)
	{
		if (is_null($user)) return $this->_user_is_root;
		if (!$user->id) return FALSE;

		if (SQ_CONF_ROOT_USER_DISABLED) {
			if ($this->userSystemAdmin($user)) {
				return TRUE;
			}
		} else if (get_class($user) == 'Root_User') {
			return TRUE;
		}

		return FALSE;

	}//end userRoot()


	/**
	* Does the passed/current user have effective system administrator access?
	*
	* To have system administrator access you must be a user with backend access
	* (in default authentication, this translates to a Backend User or above),
	* and be a member of the system administrators group
	*
	* @param object	$user	the user object you want to test for sysadmin access
	*
	* @return boolean
	* @access public
	*/
	public function userSystemAdmin($user=NULL)
	{
		if (is_null($user)) return $this->_user_is_sys_admin;
		if (!$user->id) return FALSE;
		if (!$user->canAccessBackend()) return FALSE;

		// now make sure that the user is in the system administrators group
		$sys_admin_group = $this->am->getSystemAsset('system_user_group');
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
	public function userPublic($user=NULL)
	{
		if (is_null($user)) return $this->_user_is_public;
		if (!$user->id) return FALSE;
		if (get_class($user) == 'Public_User') return TRUE;
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
	public function getUserPrefs($asset_type=NULL, $pref=NULL, $value_only=TRUE)
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
							$final_prefs[$type] = call_user_func(Array($type.'_Prefs', 'mergePrefs'), $type_prefs, $final_prefs[$type]);
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
			if (is_null($pref) && isset($user_prefs[$this->user->id][$asset_type])) {
				return $user_prefs[$this->user->id][$asset_type];
			} else if (isset($user_prefs[$this->user->id][$asset_type][$pref])) {
				return $user_prefs[$this->user->id][$asset_type][$pref];
			} else {
				return FALSE;
			}
		}

	}//end getUserPrefs()


//--        CONTEXT SET/GET        --//


	/**
	* Get the current context ID in force
	*
	* @todo
	* @return int
	*/
	public function getContextId()
	{
		$current_contextid = NULL;
		$stack_height = count($this->_context_stack);
		if ($stack_height > 0) {
			$current_contextid = $this->_context_stack[$stack_height - 1];
		}
		if (is_null($current_contextid) === FALSE) {
			$current_contextid = (int)$current_contextid;
		}
		return $current_contextid;

	}//end getContextId()


	/**
	* Change the current context, and place it on the stack
	*
	* @param int	$contextid	The ID of the context to change to
	*
	* @return void
	*/
	public function changeContext($contextid)
	{
		array_push($this->_context_stack, $contextid);

	}//end changeContextId()


	/**
	* Restore the previous context from the stack
	*
	* @return void
	*/
	public function restoreContext()
	{
		if (count($this->_context_stack) <= 0) {
			// Cannot restore the context as the context stack is empty
			trigger_localised_error('SYS0332', E_USER_ERROR);
		} else {
			array_pop($this->_context_stack);
		}

	}//end restoreContext()


	/**
	* Gets and caches information about all contexts
	*
	* This could be just called from the database each time.
	* This is being placed in a function to allow us to
	* cache the info, as it's unlikely to change within a
	* request.
	*
	* If it's vital to get the latest context info, call
	* with a parameter of TRUE to force reload.
	*
	* @param boolean	$force_reload	If TRUE, always reload data from
	*									the database. If FALSE, use cache
	*									if loaded.
	*
	* @return void
	*/
	public function getAllContexts($force_reload=FALSE)
	{
		if (((boolean)$force_reload === TRUE) || (is_array($this->_tmp) === FALSE) || (array_key_exists('context_cache', $this->_tmp) === FALSE)) {
			$context_data = MatrixDAL::executeGroupedAssoc('core', 'getContexts', Array('contextids' => NULL));
			foreach ($context_data as $contextid => &$context_item) {
				$context_item = $context_item[0];
				if (empty($context_item["conditions"]) === FALSE) {
					$context_item['conditions'] = unserialize($context_item['conditions']);
				}
			}
			unset($context_item);
			$this->_tmp['context_cache'] = $context_data;
		}

		return $this->_tmp['context_cache'];

	}//end getContextInfo()


	/**
	* Copies the attribute data for a new contextid from the default
	*
	* Returns TRUE on success, FALSE on failure
	*
	* @param int	$contextid	The ID of the new context
	*
	* @return boolean
	*/
	public function insertContextData($contextid)
	{
		try {
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			MatrixDAL::executeQuery('core', 'copyContextAttributeData', Array('contextid' => $contextid));
			MatrixDAL::executeQuery('core', 'copyContextUniqueAttributeData', Array('contextid' => $contextid));
			MatrixDAL::executeQuery('core', 'copyContextMetadataDefaults', Array('contextid' => $contextid));
			MatrixDAL::executeQuery('core', 'copyContextAssetMetadata', Array('contextid' => $contextid));
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		} catch (DALException $e) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			throw new Exception('Cannot insert default attribute data for context #'.$contextid.'; DB returned error "'.$e->getMessage().'"');
		}

		// Broadcast a 'context created' event so that assets can do something
		// with it if they want (such as create a new content file to go with it)
		// The broadcaster makes no difference since this will be called from a
		// config screen - so the currently logged in user will suffice
		$em = $this->getEventManager();
		$em->broadcastEvent($this->user, 'contextCreate', Array('contextid' => $contextid));

		return TRUE;

	}//end insertContextData()


	/**
	* Deletes the information associated with a deleted context
	*
	* Returns TRUE on success, FALSE on failure
	*
	* @param int	$contextid	The ID of the condemned context
	*
	* @return boolean
	*/
	public function deleteContextData($contextid)
	{
		try {
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			MatrixDAL::executeQuery('core', 'deleteContextAttributeData', Array('contextids' => Array($contextid)));
			MatrixDAL::executeQuery('core', 'deleteContextUniqueAttributeData', Array('contextids' => Array($contextid)));
			MatrixDAL::executeQuery('core', 'deleteContextMetadataDefaults', Array('contextids' => Array($contextid)));
			MatrixDAL::executeQuery('core', 'deleteContextMetadataValue', Array('contextids' => Array($contextid)));
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		} catch (DALException $e) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			throw new Exception('Cannot delete attribute data for context #'.$contextid.'; DB returned error "'.$e->getMessage().'"');
		}

		// Broadcast a 'context deleted' event so that assets can do something
		// with it if they want (such as cleaning up content files)
		// The broadcaster makes no difference since this will be called from a
		// config screen - so the currently logged in user will suffice
		$em = $this->getEventManager();
		$em->broadcastEvent($this->user, 'contextDelete', Array('contextid' => $contextid));
		return TRUE;

	}//end deleteContextData()


	/**
	* Returns the context we should be using based on the
	* settings on the Context Configuration screen
	*
	* Returns the ID of the context that we should be using,
	* or '0' (representing the default context) if none are
	* appropriate.
	*
	* @return int
	*/
	public function getAlternateContext()
	{
		// Check first to see whether we have an overriding context name set,
		// that hasn't already been swallowed by either set_context or
		// set_backend_context global action
		$override_context_name = array_get_index($_GET, 'SQ_CONTEXT_NAME', NULL);
		if ($override_context_name !== NULL) {
			$context_name = (isset($_GET['SQ_CONTEXT_NAME'])) ? $_GET['SQ_CONTEXT_NAME'] : '';
			unset($_GET['SQ_CONTEXT_NAME']);
			$context_data = MatrixDAL::executeAll('core', 'getContextByName', Array('name' => Array($context_name)));
			if (empty($context_data) === FALSE) {
				$contextid = $context_data[0]['contextid'];
				return $contextid;
			}
		}

		// Get all contexts, except the default
		$contexts = $this->getAllContexts();
		unset($contexts[0]);

		// If the system doesn't have any contexts, just return.
		if (empty($contexts)) {
			$contextid = 0;
			return $contextid;
		}

		uasort($contexts, create_function('$a,$b', 'return $a["sort_order"] - $b["sort_order"];'));

		// Get the base context ID relating to the current URL
		// However, if it's stale and the context has since been deleted,
		// use the default context.
		$root_url_info = $GLOBALS['SQ_SYSTEM']->am->getRootURL();
		if (array_key_exists($root_url_info['base_contextid'], $contexts) === TRUE) {
			$use_contextid = $root_url_info['base_contextid'];
		} else {
			$use_contextid = 0;
		}

		foreach ($contexts as $contextid => $context) {
			// If set to "all", change to the number of conditions we have
			// Then calculate the maximum number of fails we are allowed
			if ($context['at_least'] === NULL) {
				$context['at_least'] = max(1, count($context['conditions']));
			}
			$maximum_failures = count($context['conditions']) - $context['at_least'];

			if ($maximum_failures < 0) {
				continue;
			}

			$total_failures = 0;

			foreach ($context['conditions'] as $condition) {
				$condition_type    = $condition['type'];
				$condition_options = $condition['options'];

				$pass = FALSE;

				switch ($condition_type) {
					case 'accept-language':
						$language = $condition_options['language'];
						$language = str_replace('_', '-', strtolower($language));
						$min_q    = $condition_options['min_q'];
						$header   = array_get_index($_SERVER, 'HTTP_ACCEPT_LANGUAGE');

						if ($header !== NULL) {
							$header_items = explode(',', $header);
							foreach ($header_items as $header_item) {
								$item_bits = explode(';', trim($header_item));
								if (count($item_bits) === 1) {
									$item_bits[1] = 'q=1.0';
								}
								list($header_language, $header_q_value) = $item_bits;
								$header_language = strtolower(trim($header_language));
								$header_q_value = (float)str_replace('q=', '', trim($header_q_value));

								// ??? List of Matrix's languages only handle language and not locale
								// (eg. en vs. en-au). Currently handled by taking start of locale
								if (strpos($header_language, $language) === 0) {
									if ($header_q_value >= $min_q) {
										$pass = TRUE;
										break;
									}
								}
							}
						}
					break;

					case 'get':
					case 'post':
						if ($condition_type == 'get') {
							$var_value = array_get_index($_GET, $condition_options['var_name']);
						} else {
							$var_value = array_get_index($_POST, $condition_options['var_name']);
						}

						$matched = FALSE;
						$match_type  = $condition_options['match_type'];
						$match_value = $condition_options['match_value'];
						$case_sensitive = array_get_index($condition_options, 'case_sensitive', 1);

						if ($match_type === 'exists') {
							$matched = ($var_value === NULL) ? FALSE : TRUE;
						} else {
							if ($var_value !== NULL) {
								if ($case_sensitive === 0) {
									$var_value = strtolower($var_value);
									$match_value = strtolower($match_value);
								}

								switch ($match_type) {
									case 'exact':
										if ($var_value === $match_value) {
											$matched = TRUE;
										}
									break;

									case 'begins':
										if (strpos($var_value, $match_value) === 0) {
											$matched = TRUE;
										}
									break;

									case 'ends':
										if (strpos($var_value, $match_value) === strlen($var_value) - strlen($match_value)) {
											$matched = TRUE;
										}
									break;

									case 'contains':
										if (strpos($var_value, $match_value) !== FALSE) {
											$matched = TRUE;
										}
									break;
								}//end switch on match type

							}//end if header is not null

						}//end if match type is "variable exists"

						if ($condition_options['match_logic'] === 0) {
							$pass = !$matched;
						} else {
							$pass = $matched;
						}

					break;

					case 'header':
						$server_index = 'HTTP_'.str_replace('-', '_', strtoupper($condition_options['header_name']));
						$header_value = array_get_index($_SERVER, $server_index);

						$matched = FALSE;
						$match_type  = $condition_options['match_type'];
						$match_value = $condition_options['match_value'];
						$case_sensitive = array_get_index($condition_options, 'case_sensitive', 1);

						if ($match_type === 'exists') {
							$matched = ($header_value === NULL) ? FALSE : TRUE;
						} else {
							if ($header_value !== NULL) {
								if ($case_sensitive === 0) {
									$header_value = strtolower($header_value);
									$match_value = strtolower($match_value);
								}

								switch ($match_type) {
									case 'exact':
										if ($header_value === $match_value) {
											$matched = TRUE;
										}
									break;

									case 'begins':
										if (strpos($header_value, $match_value) === 0) {
											$matched = TRUE;
										}
									break;

									case 'ends':
										if (strpos($header_value, $match_value) === strlen($header_value) - strlen($match_value)) {
											$matched = TRUE;
										}
									break;

									case 'contains':
										if (strpos($header_value, $match_value) !== FALSE) {
											$matched = TRUE;
										}
									break;
								}//end switch on match type

							}//end if header is not null

						}//end if match type is "header exists"

						if ($condition_options['match_logic'] === 0) {
							$pass = !$matched;
						} else {
							$pass = $matched;
						}

					break;

					case 'url':
						$current_url = current_url(FALSE, TRUE);

						$matched = FALSE;
						$match_type  = $condition_options['match_type'];
						$match_value = $condition_options['match_value'];

						switch ($match_type) {
							case 'exact':
								if ($current_url === $match_value) {
									$matched = TRUE;
								}
							break;

							case 'begins':
								if (strpos($current_url, $match_value) === 0) {
									$matched = TRUE;
								}
							break;

							case 'ends':
								if (strpos($current_url, $match_value) === strlen($current_url) - strlen($match_value)) {
									$matched = TRUE;
								}
							break;

							case 'contains':
								if (strpos($current_url, $match_value) !== FALSE) {
									$matched = TRUE;
								}
							break;
						}//end switch on match type

						if ($condition_options['match_logic'] === 0) {
							$pass = !$matched;
						} else {
							$pass = $matched;
						}

					break;

					case 'port':
						// If we do not have a server port, assume port 80
						$current_port = (int)array_get_index($_SERVER, 'SERVER_PORT', 80);
						$match_type = $condition_options['match_type'];
						$match_value = (int)$condition_options['match_value'];

						if ($match_type === 'is') {
							// Secure
							$pass = ($current_port === $match_value) ? TRUE : FALSE;
						} else {
							// Not secure
							$pass = ($current_port !== $match_value) ? TRUE : FALSE;
						}

					break;

					case 'secure':
						$current_protocol = current_protocol();
						$match_value = (int)$condition_options['match_value'];

						if ($match_value === 1) {
							// Secure
							$pass = ($current_protocol === 'https') ? TRUE : FALSE;
						} else {
							// Not secure
							$pass = ($current_protocol === 'http') ? TRUE : FALSE;
						}

					break;

				}//end switch on condition type

				if ($pass === FALSE) {
					$total_failures++;

					// Exceeded maximum number of failures,
					// so stop processing this context
					if ($total_failures > $maximum_failures) {
						break;
					}
				}

			}//end foreach condition

			// We survived! This is the alternate context
			// we want.
			if ($total_failures <= $maximum_failures) {
				$use_contextid = $contextid;
				break;
			}

		}//end foreach context

		return $use_contextid;

	}//end getAlternateContext()


	/**
	 * Returns TRUE if the frontend filter is enabled for this user.
	 *
	 * The front-end filter does not apply to backend/limbo/login screen/cron/CLI
	 * modes, also in certain other situations.
	 *
	 * @return boolean
	 */
	public function frontendFilterEnabled()
	{
		$filter_enabled = FALSE;
		if (!SQ_IN_BACKEND && !SQ_IN_LIMBO && !SQ_IN_LOGIN && !SQ_IN_CRON && !SQ_PHP_CLI) {
			$filter_enabled = $GLOBALS['SQ_SYSTEM']->getUserPrefs('user', 'SQ_USER_FILTER_FRONT_END_INPUT');
			if (isset($GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION'] ) && $GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION']) {
				$filter_enabled = FALSE;
			} else if (isset($GLOBALS['SQ_CLONE_COMPONENTS'] ) && $GLOBALS['SQ_CLONE_COMPONENTS']) {
				$filter_enabled = FALSE;
			} else {
				// If user can access backend, it's safe
				$user = $GLOBALS['SQ_SYSTEM']->user;
				if ($user->canAccessBackend() === TRUE) {
					$filter_enabled = FALSE;
				}
			}//end if
		}//end if frontend

		return $filter_enabled;
	}


//--        GET MANAGERS        --//


	/**
	* Get the messaging service object so you can send/receive messages
	*
	* Always call this function to get the messaging service, dont use $this->ms directly
	*
	* @return object
	* @access public
	*/
	public function &getMessagingService()
	{
		if (!isset($this->ms) || (get_class($this->ms) != 'Messaging_Service')) {
			require_once SQ_INCLUDE_PATH.'/messaging_service.inc';
			$this->ms = new Messaging_Service();
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
	public function &getFileVersioning()
	{
		if (!isset($this->fv) || (get_class($this->fv) != 'File_Versioning')) {
			require_once SQ_LIB_PATH.'/file_versioning/file_versioning.inc';
			$this->fv = new File_Versioning(SQ_DATA_PATH.'/file_repository');
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
	public function &getHipoHerder()
	{
		if (!isset($this->hh) || (get_class($this->hh) != 'HIPO_Herder')) {
			require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_herder.inc';
			$this->hh = new HIPO_Herder();
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
	public function &getMetadataManager()
	{
		if (!isset($this->mm) || (get_class($this->mm) != 'Metadata_Manager')) {
			require_once SQ_INCLUDE_PATH.'/metadata_manager.inc';
			$this->mm = new Metadata_Manager();
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
		if (!isset($this->wm) || (get_class($this->wm) != 'Workflow_Manager')) {
			require_once SQ_INCLUDE_PATH.'/workflow_manager.inc';
			$this->wm = new Workflow_Manager();
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
		if (!isset($this->em) || (get_class($this->em) != 'Event_Manager')) {
			require_once SQ_INCLUDE_PATH.'/event_manager.inc';
			$this->em = new Event_Manager();
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
		$am = $this->am;
		if ((!isset($this->tag_manager) || (get_class($this->tag_manager) != 'Tag_Manager')) && $am->isSystemAssetType('tag_manager')) {
			$this->tag_manager = $am->getSystemAsset('tag_manager');
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
		$am = $this->am;
		if ((!isset($this->trigger_manager) || ($am->installed('trigger_manager') && !($this->trigger_manager instanceof Trigger_Manager))) && $am->isSystemAssetType('trigger_manager')) {
			$this->trigger_manager = $am->getSystemAsset('trigger_manager');
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
		$am = $this->am;
		if ((!isset($this->log_manager) || (get_class($this->log_manager) != 'Log_Manager')) && $am->isSystemAssetType('log_manager')) {
			$this->log_manager = $am->getSystemAsset('log_manager');
		}

		// this is a bypass to allow logging while Log Manager asset has not been installed yet
		// necessary during installation, because we want ALL the log entries to be handled by the log manager,
		// not only those that appeared after it was installed.
		if (is_null($this->log_manager)) {
			return new Log_Manager();
		}

		return $this->log_manager;

	}//end getLogManager()


	/**
	* Get Deja Vu for that eerie "have me not met before?" feeling.
	*
	* Always call this function to get Deja Vu, dont use $this->_deja_vu directly
	*
	* @return object
	* @access public
	*/
	function getDejaVu()
	{
		if (!isset($this->_deja_vu) || (get_class($this->_deja_vu) != 'Deja_Vu')) {
			require_once SQ_INCLUDE_PATH.'/deja_vu.inc';
			$this->_deja_vu = new Deja_Vu();
		}

		if ($this->_deja_vu->enabled()) {
			return $this->_deja_vu;
		} else {
			return NULL;
		}

	}//end getDejaVu()


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
	* @param string	$oci_force_new	 force a new connection for oci
	*
	* @return void
	* @access public
	*/
	function changeDatabaseConnection($conn, $oci_force_new = FALSE)
	{
		$dsn = NULL;
		$db_conf = require(SQ_DATA_PATH.'/private/conf/db.inc');

		// If attempting to connect to empty dbcache entry
		if ($conn === 'dbcache') {
			if (empty($db_conf[$conn]) && empty($db_conf['db2'])) {
				throw new Exception ('Unable to connect to connection "'.$conn.'". No connection with that name is defined in the database config, and unable to fall back to "db2" connection.');
			} else if (empty($db_conf[$conn])) {
				// no dbcache, but db2 exists, use that instead
				$dsn_array = $db_conf['db2'];
			} else {
				// dbcache exists
				$dsn_array = $db_conf[$conn];
			}
		} else if ($conn === 'dbsearch') {
			if (empty($db_conf[$conn]) && empty($db_conf['db'])) {
				throw new Exception ('Unable to connect to connection "'.$conn.'". No connection with that name is defined in the database config, and unable to fall back to "db" connection.');
			} else if (empty($db_conf[$conn])) {
				// no dbsearch, but db exists, use that instead
				$dsn_array = $db_conf['db'];
			} else {
				// dbsearch exists
				$dsn_array = $db_conf[$conn];
			}
		} else {
			// other connection (db, db2, db3)
			if (empty($db_conf[$conn])) {
				throw new Exception ('Unable to connect to connection "'.$conn.'". No connection with that name is defined in the database config.');
			} else {
				// When in backend or CLI, replace connections to DB with DB2
				// to avoid concurrency issues.
				if ($conn === 'db' && (SQ_IN_BACKEND || SQ_IN_LIMBO || SQ_PHP_CLI)) {
					$dsn_array = $db_conf['db2'];
				} else {
					$dsn_array = $db_conf[$conn];
				}
			}
		}

		if (empty($this->_db_conns[$conn])) {
			try {
				// TODO: parse DSN
				$this->_db_conns[$conn] = MatrixDAL::dbConnect($dsn_array, $conn, $oci_force_new);
				$db = MatrixDAL::getDb($conn);

				if ($dsn_array['type'] === 'oci') {
					// We cannot specify any functions upon connect for OCI8
					// functions - we must specify them in functions like
					// oci_execute()...
					;
				} else {
					// Turn off auto-commit, and ensure column names are
					// returned in lowercase (important for Oracle)
					if ($dsn_array['type'] === 'oci') {
						$db->setAttribute(PDO::ATTR_AUTOCOMMIT, FALSE);
					}

					// Ensure that no conversion is done to empty strings and NULLs
					$db->setAttribute(PDO::ATTR_ORACLE_NULLS, PDO::NULL_NATURAL);

					// Return field names in lowercase
					$db->setAttribute(PDO::ATTR_CASE, PDO::CASE_LOWER);

					// String-ify all fetches
					$db->setAttribute(PDO::ATTR_STRINGIFY_FETCHES, TRUE);
				}

			} catch (Exception $e) {
				header('HTTP/1.0 500 Internal Server Error');
				throw new Exception('Could not create database connection: '.$e->getMessage());
			}
		}//end if

		MatrixDAL::changeDb($conn);

	}//end changeDatabaseConnection()


	/**
	* Restore the active DB connection to the previous one
	*
	* @param boolean	$force_reconnect	set to true for pcntl fork reconnect
	*
	* @return void
	* @access public
	*/
	function restoreDatabaseConnection($force_reconnect=FALSE)
	{
		MatrixDAL::restoreDb();
		if ($force_reconnect) {
			// need to clear this otherwise will not get new db connection
			// from DAL
			array_pop($this->_db_conns);
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
		if (!isset($this->tm) || get_class($this->tm) != 'Transaction_Manager') {
			require_once SQ_INCLUDE_PATH.'/transaction_manager.inc';
			$this->tm = new Transaction_Manager();
		}
		// Use this when there is a transaction problem to create a log and then run this from the cmd line
		// and look for the fn with the odd number of calls
		// cat trans.log | awk '{ print $4 }' | sort | uniq -c # number of calls in a fn
		// cat trans.log | awk '{ print $6 }' | sort | uniq -c # total number of each type of call
		// $bt = debug_backtrace();
		// if (isset($bt[1])) {
		// error_log (MatrixDAL::getCurrentDbId().' TRANS : '.$bt[1]['class'].'::'.$bt[1]['function'].':'.$bt[0]['line'].' -- '.$type."\n", 3, SQ_SYSTEM_ROOT.'/data/private/logs/trans.log');
		// }  else {
		// error_log (MatrixDAL::getCurrentDbId().' TRANS : '.$bt[0]['file'].'::'.$bt[0]['line'].' -- '.$type."\n", 3, SQ_SYSTEM_ROOT.'/data/private/logs/trans.log');
		// }

		$current_db = MatrixDAL::getCurrentDbId();

		switch (strtolower($type)) {
			case 'begin':
				return $this->tm->begin($current_db);
			case 'commit':
				return $this->tm->commit($current_db);
			case 'rollback':
				return $this->tm->rollback($current_db);
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
		if (!empty($_GET['assetid'])) {
			$exit_url .= '&assetid='.$_GET['assetid'].'&am_section=edit_asset&asset_ei_screen='.array_get_index($_GET, 'asset_ei_screen', 'details');
		}

		?>
		<div class="sq-rollback-warning-wrapper">
			<style>
			.sq-rollback-warning-table {
				border: 1px solid #e0d3a2 !important;
			}
			.sq-rollback-warning-table th{
				border-bottom: 1px solid #e0d3a2 !important;
				background:	#fff1bb;
				color: #000;
				text-shadow: none;
			}
			.sq-rollback-warning-table .sq-rollback-header{
				background:	#fff1bb url(<?php echo sq_web_path('lib').'/web/images/warning.png'; ?>) no-repeat 5px 6px !important;
				padding-left: 35px;
				border-right: none;
			}
			.sq-rollback-warning-table td{
				background:#fff1bb !important;
			}
			.sq-rollback-warning-table img{
				vertical-align: top;
			}
			.sq-rollback-warning {
				color: #000;
				font-size: 11px;
			}
			</style>
			<table class="sq-backend-table sq-rollback-warning-table">
				<tr>
					<th class="sq-rollback-header">
						<?php echo translate('in_rollback_mode'); ?>
					</th>
					<th style="text-align:right;">
						<a href="#" onclick="window.top.location='<?php echo $exit_url; ?>'; return false;" >
							<?php 
								echo translate('exit_rollback_mode').' ';
								sq_print_icon(sq_web_path('lib').'/web/images/icons/cancel.png', '16', '16', translate('exit_rollback_view'), NULL); 
							?>
						</a>
					</th>
				<tr>
					<td colspan="2" class="sq-rollback-warning"><?php echo $warning_message; ?>
					</td>
				</tr>
			</table>
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
		$where .= $table_alias.'sq_eff_from <= \''.$then.'\'
				  AND ('.$table_alias.'sq_eff_to IS NULL
					   OR '.$table_alias.'sq_eff_to > \''.$then.'\')';

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
		$class_name = 'locking_method_'.SQ_CONF_LOCKING_METHOD;
		$this->am->includeAsset($class_name);

		try {
			$ok = call_user_func(array($class_name, 'acquireLock'), $lockid, $source_lockid, $expires);
		} catch (Exception $e) {
			return $e->getMessage();
		}

		return $ok;

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
		$class_name = 'locking_method_'.SQ_CONF_LOCKING_METHOD;
		$this->am->includeAsset($class_name);

		try {
			$ok = call_user_func(array($class_name, 'releaseLock'), $lockid);
		} catch (Exception $e) {
			return $e->getMessage();
		}

		return $ok;

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
		$class_name = 'locking_method_'.SQ_CONF_LOCKING_METHOD;
		$this->am->includeAsset($class_name);

		try {
			$ok = call_user_func(array($class_name, 'updateLock'), $lockid, $expires);
		} catch (Exception $e) {
			return $e->getMessage();
		}

		return $ok;

	}//end updateLock()


	/**
	* Returns an array of information about the passed lock
	*
	* @param string|array	$lockid			the id of the lock
	* @param boolean		$full_chain		return the info for every lock in the lock chain
	* @param boolean		$check_expires	check the expiry date and cleanup the lock if it has expired
	* @param boolean		$allow_only_one	Force only one lock returned
	*
	* @return array
	* @access public
	*/
	function getLockInfo($lockid, $full_chain=FALSE, $check_expires=TRUE, $allow_only_one=TRUE)
	{
		$class_name = 'locking_method_'.SQ_CONF_LOCKING_METHOD;
		$this->am->includeAsset($class_name);

		try {
			$lock_info = call_user_func(array($class_name, 'getLockInfo'), $lockid, $full_chain, $check_expires, $allow_only_one);
		} catch (Exception $e) {
			return $e->getMessage();
		}

		return $lock_info;

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
		try {
			$sql = 'SELECT code_name, version, name, description FROM sq_pkg';
			$packages = MatrixDAL::executeSqlAssoc($sql);
		} catch (DALException $e) {
			throw new Exception ('Could not get installed packages list due to database error: '.$e->getMessage());
		}

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
		$tm = $this->getTriggerManager();
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
	* Paint a Not Found page
	*
	* This function will print the HTTP 404 header, any necessary headers for
	* the sending of Cacheable Headers, and then the "Not Found" asset itself.
	* This will typically be the "Not Found" asset of the site, but may have
	* been set to something else (eg. the "Archived Asset" 404 link).
	*
	* @param Asset	$asset	The asset that represents the Not Found page
	* @param Site	$site	The site that has set the Not Found page
	*
	* @return void
	*/
	protected function _paintNotFoundAsset(Asset $asset, Site $site)
	{
		header('HTTP/1.0 404 Not Found');

		// Send Cacheable Header based on Not Found page asset type
		if (SQ_CONF_SEND_CACHEABLE_HEADER && SQ_CONF_SEND_404_CACHEABLE_HEADER) {
			$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
			header('Pragma: cache');

			$browser_cache_expiry = $cm->getBrowserCacheExpiry($asset->type(), $asset->id);
			$acc_cache_expiry = $cm->getAcceleratorCacheExpiry($asset->type(), $asset->id);
			$acc_cache_string = empty($acc_cache_expiry) ? '' : "s-maxage=$acc_cache_expiry, ";

			if (empty($browser_cache_expiry)) {
				$browser_cache_expiry = $cm->getExpiry($asset->type(), $asset->id);
			}
			$level = $cm->cacheControlLevel();
			header('Cache-Control: max-age='.$browser_cache_expiry.', '.$acc_cache_string.$level);
			header('Expires: '.gmdate('D, d M Y H:i:s', time() + $browser_cache_expiry).' GMT');
		}

		$old_current_asset = $GLOBALS['SQ_SYSTEM']->getGlobalDefine('CURRENT_ASSET', NULL);
		$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $asset);

		// If "Not found" page has overriding design and/or paint layout
		$override_design = $site->getSpecialPage('not_found_design');
		$override_layout = $site->getSpecialPage('not_found_layout');
		ob_start();
			$site->paintAsset($asset, $override_design, $override_layout);
		$content = ob_get_clean();

		$this->replaceKeyword($content);

		if (is_null($old_current_asset)) {
			$GLOBALS['SQ_SYSTEM']->unsetGlobalDefine('CURRENT_ASSET');
		} else {
			$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $old_current_asset);
		}//end if

		echo $content;

	}//end _paintNotFoundAsset()


	/**
	* Sees if an auth redirect is available, and then redirects if there is
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
		$db = $GLOBALS['SQ_SYSTEM']->db;

		$current_url = current_url(FALSE, FALSE);
		$current_protocol = current_protocol();
		$root_url = $GLOBALS['SQ_SYSTEM']->am->getRootURL();

		$bind_vars = Array(
						'urlid' => $root_url['urlid'],
					 );
		$result = MatrixDAL::executeAll('core', 'getAuthRedirect', $bind_vars);

		if (empty($result)) return FALSE;
		$new_root_url = $result[0];
		unset($result);

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

		// Before redirecting, blanking out the public user, see ya on the flip side!
		if ($this->userPublic($this->user)) {
			$_SESSION['user'] = NULL;
			$_SESSION['userid'] = NULL;
			$_SESSION['user_type_code'] = NULL;
		}

		do_redirect($url);

		// Fall-through that should never be executed
		return FALSE;

	}//end authRedirect()


	/**
	* Generates a attribute image
	*
	* @param string	$text	The text of the image
	*
	* @return void
	* @access public
	*/
	public function _generateAttributeImage($text)
	{
		// Default settings
		putenv('GDFONTPATH=' . realpath(SQ_LIB_PATH.'/fonts'));
		$default_fonts = Array( 1,2,3,4,5 );
		$backup_font = 3;

		// Load the user prefs
		$image_prefs = $this->getUserPrefs('user', 'SQ_USER_ATTRIBUTE_IMAGE', TRUE);
		$font = $image_prefs['face'];
		$size = $image_prefs['size'];
		$hpad = $image_prefs['hpad'];
		$vpad = $image_prefs['vpad'];
		$colour1 = ltrim($image_prefs['bgcol'], '#');
		$colour2 = ltrim($image_prefs['fgcol'], '#');

		// Calculate the sizes etc.
		$width = ((in_array($font, $default_fonts)) ? (imagefontwidth($font) * strlen($text)) : ($size * strlen($text))) + ($hpad * 2);
		$height = ((in_array($font, $default_fonts)) ? imagefontheight($font) : ($size + $size/2)) + ($vpad * 2);
		$hh = $height / 2;
		$x = 0 + $hpad;
		$y = (in_array($font, $default_fonts)) ? (0 + $vpad) : (round($height - ($size / 2), 0) - $vpad);

		// Create the image
		if (!$image = imagecreatetruecolor($width, $height)) {
			trigger_error('Cannot Initialize new GD image stream', E_USER_WARNING);
			exit();
		}

		// Work out the colours
		list($r1,$g1,$b1) = str_split($colour1, 2);
		list($r2,$g2,$b2) = str_split($colour2, 2);
		$bg_colour = imagecolorallocate($image, hexdec($r1), hexdec($g1), hexdec($b1));
		$fg_colour = imagecolorallocate($image, hexdec($r2), hexdec($g2), hexdec($b2));

		// Colour the background
		imagefill($image, 0,0, $bg_colour);

		// Write the text on the image
		if (in_array($font, $default_fonts)) {
			$text_created = imagestring($image, $font, $x, $y, $text, $fg_colour);
		} else {
			$text_created = @imagettftext($image, $size, 0, $x, $y, $fg_colour, $font, $text);
		}//end if

		// Getting image sizes correct the first time, is kind of tricky
		// So creating the image putting the text on it, then resizing the image, seems to work
		if ($text_created === FALSE) {
			// An error occured with drawing the text, try using the default font
			$width = (imagefontwidth * strlen($text)) + ($hpad * 2);
			$height = imagefontheight + ($vpad * 2);
			if (!$image = imagecreatetruecolor($width, $height)) {
				trigger_error('Cannot Initialize new GD image stream', E_USER_WARNING);
				exit();
			}
			imagefill($image, 0,0, $bg_colour);
			$text_created = imagestring($image, $backup_font, $x, $y, $text, $fg_colour);
		} else if (isset($text_created[2]) && !empty($text_created[2]) && $text_created[2] != $width) {
			$width = $text_created[2]+1;
			$height = $text_created[3];
			if (!$image = imagecreatetruecolor($width, $height)) {
				trigger_error('Cannot Initialize new GD image stream', E_USER_WARNING);
				exit();
			}
			$bg_colour = imagecolorallocate($image, hexdec($r1), hexdec($g1), hexdec($b1));
			$fg_colour = imagecolorallocate($image, hexdec($r2), hexdec($g2), hexdec($b2));
			imagefill($image, 0,0, $bg_colour);
			$text_created = @imagettftext($image, $size, 0, $x, $y, $fg_colour, $font, $text);
		}//end if
		header('Content-type: image/png');
		imagepng($image);

	}//end _generateAttributeImage()


	/**
	* Replace "./?a=xx" URLs by  their respective full URLs
	*
	* @param string	&$content	The content to replace ./a=xx urls in
	*
	* @return void
	* @access private
	*/
	function _translateMatrixURL(&$content)
	{
		// Get all the matrix ./a=xx links in the content
		preg_match_all('!\./\?a=([0-9]+(?:\:[0-9a-z]+\$?)?)!msi', $content, $matches);
		if (empty($matches[1])) return;

		$org_assetids = Array();
		foreach($matches[1] as $key => $assetid) {
			$matches[1][$key] = rtrim($assetid, '$');
			$org_assetids[$key] = $assetid;
		}
		// bug fix for squizmap issue #5266 /?a= link interpretation error
		// replace the longer assetids first so that when we get to the maller assetids
		// there is no chance of partial match and the world makes sense once again
		arsort($matches[1]);

		$assets_url = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($matches[1]);

		// Replace each ./a=xx link by its respective full URL
		foreach($matches[1] as $index => $assetid) {
			if (isset($assets_url[$assetid]) && !empty($assets_url[$assetid])) {
				$content = str_ireplace('./?a='.$org_assetids[$index], $assets_url[$assetid], $content);
			}
		}
	}//end _translateMatrixURL()


	/**
	* Replace all URLs with performance suffix and target = _top
	*
	* @param string	&$content	The entire matrix output content
	*
	* @return void
	* @access private
	*/
	function _translatePerformanceModeURL(&$content)
	{
		if(!defined('SQ_IN_PERFORMANCE_TIMING')) return;

		// Get all links in the content
		preg_match_all('/<a\s[^>]*href=(\"??)([^\" >]*?)\\1[^>]*>(.*)<\/a>/siU', $content, $matches);

		// Replace hyper links with _performance style url
		foreach($matches[0] as $index => $url) {
				$src = $matches[2][$index];
				if(empty($src)) continue;

				// remove trailing slash
				if(substr($src, -1) == '/') {
					$src = rtrim($src, '/');
				}
				// add _performance suffix
				if(strpos($src, '?') !== FALSE) {
					$src = preg_replace('/\?/', '/'.SQ_CONF_PERFORMANCE_SUFFIX.'/?', $src);
				}
				else {
					$src = $src.'/'.SQ_CONF_PERFORMANCE_SUFFIX;
				}

				// only replace the first link found
				$pos = strpos($url, $matches[2][$index]);
				if($pos !== FALSE)
					$new_url = substr_replace($url, $src, $pos, strlen($matches[2][$index]));

				// remove target attribute
				$new_url = preg_replace('/target[ ]*=[ ]*[^a-zA-Z_"\']*/i', '', $new_url);

				// add new target attribute
				$new_url = preg_replace('/^<a/i', '<a target="_top"', $new_url);

				$content = str_replace($url, $new_url, $content);
		}


		// Get all forms in the content
		preg_match_all('/<form\s[^>]*action=(\"??)([^\" >]*?)\\1[^>]*>/siU', $content, $matches);

		// Add onsubmit event for all forms. We have to refresh the performance result frame in this way(using js addEvent could cause too much issues)
		foreach($matches[0] as $index => $url) {
				$new_url = $url;
				if(preg_match('/onsubmit/i', $new_url)) {
					// existing submit attribute
					$new_url = preg_replace('/onsubmit[ ]*=[ ]*["\']+([^"\']*)["\']+/i', 'onsubmit="parent.result_frame.document.location.reload(); \\1"', $new_url);
				}
				else {
					// add new submit event
					$new_url = preg_replace('/^<form/i', '<form onsubmit="parent.result_frame.document.location.reload();"', $new_url);
				}


				$content = str_replace($url, $new_url, $content);
		}

	}//end _translatePerformanceModeURL()


	/**
	* If SQ_CONF_USE_HTTP_X_FORWARDED_FOR enabled, attempts to obtain User IP using HTTP_X_FORWARDED_FOR
	* and overwrites the REMOTE_ADDR $_SERVER variable.
	* This is to avoid recieving and using a Proxy IP in REMOTE_ADDR when behind a proxy.
	* A trusted/known Proxy IP SQ_CONF_FORWARDED_IP_PROXY_LIST list is required.
	*
	* @return void
	* @access private
	*/
	private function _processUserIP()
	{
		if(!defined('SQ_CONF_USE_HTTP_X_FORWARDED_FOR') || !defined('SQ_CONF_FORWARDED_IP_PROXY_LIST')) return;
		if (SQ_CONF_USE_HTTP_X_FORWARDED_FOR && SQ_CONF_FORWARDED_IP_PROXY_LIST != ''){
			$safe_list = explode("\n", SQ_CONF_FORWARDED_IP_PROXY_LIST);
			if (array_key_exists('REMOTE_ADDR', $_SERVER) && in_array($_SERVER['REMOTE_ADDR'], $safe_list)) {
				// if we have an X-Forwarded-For set
				if (!empty($_SERVER['HTTP_X_FORWARDED_FOR'])) {
					$forwards = explode(', ', $_SERVER['HTTP_X_FORWARDED_FOR']);
					// right most is more recent $forwards[$n]
					// starting from right, find the first IP which is not in SQ_CONF_FORWARDED_IP_PROXY_LIST
					for ($n=count($forwards)-1; $n > 0; $n--) {
						if (!in_array($forwards[$n], $safe_list)) {
							break;
						}
					}
					$last_forward = trim($forwards[$n]);
					//change the remote addr
					$_SERVER['REMOTE_ADDR'] = $last_forward;
				}
			}
		}

	}//end _processUserIP()


	/**
	* Print the asset frontend
	*
	* @param object $asset	Asset of which we are printing the frontend
	*
	* @return void
	* @access private
	*/
	private function _printAssetFrontend($asset)
	{
		// Register the very first 'entry page' assetid
		if (!isset($_SESSION['entry_asset']) && !isset($_REQUEST['just_logged_out'])) {
			$_SESSION['entry_asset'] = $asset->id;
			$_SESSION['entry_url'] = rtrim(current_url(), '/');
		}

		// Register the session's referrer (site referrer)
		if (!isset($_SESSION['site_referrer']) && !isset($_REQUEST['just_logged_out'])) {
			$_SESSION['site_referrer'] = isset($_SERVER['HTTP_REFERER']) ? $_SERVER['HTTP_REFERER'] : '';
		}

		// Print the asset frontend
		$asset->printFrontend();

	}//end _printAssetFrontend()


	/**
	* Add the asset and current url to the 'visited' stack in the session
	*
	* @param object $asset	Asset being registered as 'visited'
	*
	* @return void
	* @access private
	*/
	private function _registerAssetAsVisited($asset)
	{
		// Register the url and assetid as "visited"
		if (SQ_CONF_VISITED_PAGES_MAX_ENTRIES > 0) {
			if (!isset($_SESSION['visited_urls'])) {
				$_SESSION['visited_urls'] = Array();
			}
			if (!isset($_SESSION['visited_assets'])) {
				$_SESSION['visited_assets'] = Array();
			}

			$current_url = rtrim(current_url(), '/');
			$assetid = $asset->id;
			// We add a prefix to the array keys because numerical array keys (in case of assteids)
			// gets reset and lost when doing array_shift()
			$prefix = 'v_';

			// Get rid of existing duplicate entry
			if (isset($_SESSION['visited_urls'][$prefix.$current_url])) {
				unset($_SESSION['visited_urls'][$prefix.$current_url]);
			}
			if (isset($_SESSION['visited_assets'][$prefix.$assetid])) {
				unset($_SESSION['visited_assets'][$prefix.$assetid]);
			}
			$_SESSION['visited_urls'][$prefix.$current_url] = $current_url;
			$_SESSION['visited_assets'][$prefix.$assetid] = $assetid;

			// Stay within the defined "max entires" size
			if (count($_SESSION['visited_urls']) > SQ_CONF_VISITED_PAGES_MAX_ENTRIES) {
				array_shift($_SESSION['visited_urls']);
			}
			if (count($_SESSION['visited_assets']) > SQ_CONF_VISITED_PAGES_MAX_ENTRIES) {
				array_shift($_SESSION['visited_assets']);
			}
		}

	}//end _registerAssetAsVisited()


	/**
	* Add the content in the output that will be replaced after
	* the highest level keyword replacement
	*
	* @param string $content
	*
	* @return void
	* @accesss public
	*/
	public function addRawContent($content)
	{
		//Hash string to uniquely identify the content
		$key = '<!--'.md5($content).strlen($content).'-->';
		if (!isset($this->_raw_contents[$key])) {
			$this->_raw_contents[$key] = $content;
		}

		// Output the hash key
		echo $key;

	}//end addRawContent()


}//end class
?>
