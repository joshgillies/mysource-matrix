<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: mysource.inc,v 1.18 2003/11/10 02:57:17 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/


require_once 'DB.php';
require_once SQ_INCLUDE_PATH.'/mysource_object.inc';
require_once SQ_INCLUDE_PATH.'/asset_manager.inc';

/**
* MySource
*
* Purpose
*    This is the object from which everything else is run
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class MySource
{

	/**
	* this is the current DB object
	*
	* @var object DB
	*/
	var $db;

	/**
	* this holds all open DB objects
	*
	* @var Array()
	* @see changeDatabaseConnection(), restoreDatabaseConnection()
	*/
	var $_db_conns = Array();

	/**
	* this is the stack for db
	*
	* @var Array()
	* @see changeDatabaseConnection(), restoreDatabaseConnection()
	*/
	var $_db_stack = Array();

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
	* @var object Indexing_Manager
	*/
	var $im;

	/**
	* @var object File_Versioning
	*/
	var $fv;

	/**
	* @var object Event_Manager
	*/
	var $em;

	/**
	* Object of logged in user
	*
	* @var object User
	*/
	var $user = null;

	/**
	* Whether the logged in user is the root user or not
	*
	* @var boolean
	*/
	var $_user_is_root = false;

	/**
	* Whether the logged in user is a system admin or not
	*
	* @var boolean
	*/
	var $_user_is_sys_admin = false;

	/**
	* Array of system messages
	*
	* @var Array(string)
	*/
	var $_msgs;


	/**
	* Constructor
	*
	*/
	function MySource()
	{
	}// end constructor


	/**
	* This initialises all the objects that we need to use, the reason that we can't do
	* this in the constructor is that the Asset_Manager uses the DB in it's constructor
	* in order to load the asset types but if initialise the Asset_Manager in our constructor
	* we don't exist and therefore cannot be accessed to get the DB for the Asset_Manager to use
	*
	* @access public
	*/
	function init()
	{
		$this->changeDatabaseConnection('db');

		// initialise the asset manager as we are going to need it pretty much everywhere
		$this->am = new Asset_Manager();

		// running from the command line ? don't use cookies for session
		if (!SQ_PHP_CLI) {
			session_name('SQ_SYSTEM_SESSION');
			session_save_path(SQ_SYSTEM_ROOT.'/cache');
			session_start();

			// process rollback view mode global actions here so setting the rollback vars below
			if (!empty($_GET['SQ_ACTION']) && strpos($_GET['SQ_ACTION'], 'rollback_view') === 0) {
					$this->_processGlobalActions();
			}

			// check if we are in rollback view mode
			if (!empty($_SESSION['sq_rollback_view'])) {
				// we are viewing the system at some time in the past
				define('SQ_ROLLBACK_VIEW',        '1');
				define('SQ_TABLE_RUNNING_PREFIX', SQ_TABLE_ROLLBACK_PREFIX);
			} else {
				define('SQ_ROLLBACK_VIEW',        '0');
				define('SQ_TABLE_RUNNING_PREFIX', SQ_TABLE_PREFIX);
			}

			// If there is a userid in the session then someone is logged in,
			// but make sure that the person is coming from the same machine as they logged in on
			// so get a reference to them
			$this->user = null;
			if(!empty($_SESSION['user_last_access']) 
				&& time() < ((int) $_SESSION['user_last_access'] + SQ_CONF_MAX_IDLE_TIME)
				&& !empty($_SESSION['userid']) 
				&& $_SESSION['remote_addr'] == $_SERVER['REMOTE_ADDR']) {
				// we mute errors trying to get the user because we will handle this gracefully later
				$this->user = &$this->am->getAsset($_SESSION['userid'], $_SESSION['user_type_code'], true);
			}

			if (is_null($this->user)) {
				$_SESSION['userid']            = 0;
				$_SESSION['user_type_code']    = '';
				$_SESSION['user_is_root']      = false;
				$_SESSION['user_is_sys_admin'] = false;
			}

			$_SESSION['user_last_access'] = time();
			$this->_user_is_root      = $_SESSION['user_is_root'];
			$this->_user_is_sys_admin = $_SESSION['user_is_sys_admin'];

		} else {
			define('SQ_ROLLBACK_VIEW',        '0');
			define('SQ_TABLE_RUNNING_PREFIX', SQ_TABLE_PREFIX);
		}// end if

		// process any global actions we have
		if (!empty($_GET['SQ_ACTION'])) $this->_processGlobalActions();

	}// end init()


	/**
	* From here is where everything else happens, it starts the ball rolling
	*
	* @access public
	*/
	function start()
	{
		// initiailise event listeners
		$GLOBALS['SQ_EVENT_LISTENER_OBJECTS'] = Array();
		$GLOBALS['SQ_EVENT_LISTENERS']        = Array();

		if (SQ_IN_BACKEND) {
			if ($this->user) {
				if (is_a($this->user, 'backend_user')) {
					require_once SQ_INCLUDE_PATH.'/backend.inc';
					$backend = new Backend();
					$backend->paint();
				} else {
					$this->paintLogin('Login', 'You need to be a backend user in order to access the backend');
				}
			} else {
				$this->paintLogin('Login', 'You need to login before you can access the backend');
			}

		// we are on the frontend
		} else {
			// check for ./?a=xx type urls or normal
			if (isset($_GET['a'])) $asset = &$this->am->getAsset((int) $_GET['a']);
			else $asset = &$this->am->getAssetFromURL(current_protocol());
			if (is_null($asset)) exit(1);
			$asset->printFrontend();
		}

		$this->db->disconnect();

	}//end start()


	/**
	* Paints the login form, based upon which url we are currently at
	*
	* @param string	$heading	a heading to print on the form
	* @param string	$msg		a msg to print with the form
	*
	* @access public
	*/
	function paintLogin($heading, $msg)
	{
		$db = &$this->db;

		header("HTTP/1.0 403 Forbidden");
		$GLOBALS['SQ_LOGIN_FORM'] = Array('paint' => 'login', 'heading' => $heading, 'msg' => $msg);

		if (empty($_GET['FORCE_BACKUP_LOGIN'])) {

			$current_asset = &$this->am->getAssetFromURL(null, null, true, true);
			// if we can't find a current asset, use the root folder
			if (is_null($current_asset)) {
				$current_asset = &$this->am->getSystemAsset('root_folder');
				if (is_null($current_asset)) {
					trigger_error('Unable to get Root Folder Asset', E_USER_ERROR);
				}
			}

			$sql = 'SELECT ld.designid, a.type_code
					FROM sq_asset_lookup_design ld INNER JOIN sq_asset a ON ld.designid = a.assetid
					WHERE ld.url  = '.$db->quote(strip_url(current_url(false, true))).'
					  AND ld.name = '.$db->quote('system_design::login');
			$result = $db->getRow($sql);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			} else {

				// we have found the design to use
				if ($result) {
					$design = &$this->am->getAsset($result['designid'], $result['type_code']);
					if (!is_null($design)) {
						$design->paint($current_asset);
						return;
					}
				}

				// OK, use the system login form
				$design = &$this->am->getSystemAsset('login_design');
				if (!is_null($design)) {
					$design->paint($current_asset);
					return;
				}

			}// end if

		}// end if

		// shit we can't find a design, something has well and truly F@#ked up,
		// resort to our backup
		if ($this->_msgs) echo implode('<br />', $this->_msgs);
		require_once SQ_INCLUDE_PATH.'/login.inc';

	}// end printLogin()


	/**
	* Processes any global actions that need to be taken care of
	*
	* @access private
	*/
	function _processGlobalActions()
	{
		switch($_GET['SQ_ACTION']) {
			case 'login' :
				if (isset($_POST['SQ_LOGIN_KEY']) && isset($_POST['SQ_LOGIN_USERNAME']) && 
					isset($_POST['SQ_LOGIN_PASSWORD']) && $this->loginKey() == $_POST['SQ_LOGIN_KEY']) {
					$this->am->includeAsset('user');
					$user = &User::find($_POST['SQ_LOGIN_USERNAME'], $_POST['SQ_LOGIN_PASSWORD']);
					if (!is_null($user)) {
						if ($this->setCurrentUser($user)) {
							$_SESSION['userid']         = $user->id;
							$_SESSION['user_type_code'] = $user->type();
							$_SESSION['remote_addr']    = $_SERVER['REMOTE_ADDR'];

							// log a message so we know someone has logged in
							$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
							$log_body = 'User "'.$user->name.'" logged in';
							$log = $ms->newMessage(Array(), 'User Logged In', $log_body, 'system.security.login');
							$log->parameters['remote_addr'] = $_SERVER['REMOTE_ADDR'];
							$log->parameters['sessionid'] = session_id();
							$log->send();

							// generate a new login key so that when they try and login next
							// they have to re-enter the details
							$this->generateLoginKey();

							$_SESSION['user_is_root']      = $this->_user_is_root;
							$_SESSION['user_is_sys_admin'] = $this->_user_is_sys_admin;
							$_SESSION['user_last_access']  = time();
							unset($_SESSION['user_login_attempts']);
							return;
						}
					} else {

						$username = $_POST['SQ_LOGIN_USERNAME'];
						if (!isset($_SESSION['user_login_attempts'])) $_SESSION['user_login_attempts'] = Array();
						if (empty($_SESSION['user_login_attempts'][$username])) $_SESSION['user_login_attempts'][$username] = 1;
						else $_SESSION['user_login_attempts'][$username]++;

						if ($_SESSION['user_login_attempts'][$username] >= SQ_CONF_MAX_LOGIN_ATTEMPTS) {
							$user = &User::find($username);
							if (!is_null($user) && $user->canLogin()) {
								if ($this->setCurrentUser($this->am->getSystemAsset('root_user'))) {
									$success = false;
									if ($user->processStatusChange(SQ_STATUS_UNDER_CONSTRUCTION)) {
										$this->addMessage('User "'.$username.'" locked');
										$success = true;
									}
									$this->restoreCurrentUser();

									// log a message so we know someone has had their account locked
									// we need to do this here because before we restore the current user
									// we are actually logged in as ROOT
									if ($success) {
										$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
										$log_body = 'User "'.$username.'" has had their account locked after '.(int)SQ_CONF_MAX_LOGIN_ATTEMPTS.' unsuccessful login attempts';
										$log = $ms->newMessage(Array(), 'User Account Locked', $log_body, 'system.security.login');
										$log->parameters['remote_addr'] = $_SERVER['REMOTE_ADDR'];
										$log->parameters['sessionid'] = session_id();
										$log->send();
									}
								}
							}
						}
					}

					$this->addMessage('User not found');

				} else {
					// incorrect login key
					$this->addMessage('Login Key incorrect');
				}

				// deliberalty don't have a break here so it the
				// login fails we make sure that we're logged out
			case 'logout' :
				if ($this->user) {
					// log a message so we know someone has logged out
					$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
					$log_body = 'User "'.$this->user->name.'" logged out';
					$log = $ms->newMessage(Array(), 'User Logged Out', $log_body, 'system.security.login');
					$log->parameters['remote_addr'] = $_SERVER['REMOTE_ADDR'];
					$log->parameters['sessionid'] = session_id();
					$log->send();
				}

				$_SESSION['userid']				= 0;
				$_SESSION['user_type_code']		= '';
				$_SESSION['user_is_root']		= false;
				$_SESSION['user_is_sys_admin']	= false;
				unset($_SESSION['user_last_access']);
				$this->user = null;
			break;

			case 'send_security_key' :
				require_once SQ_FUDGE_PATH.'/general/general.inc';
				$vars =& $_SESSION['SQ_SYSTEM_SECURITY_KEY'];

				// are we regenerating the secturity key?
				if (isset($_GET['regen_key']) && $_GET['regen_key'] == 1) {
					$vars['key'] = generate_security_key(strlen($vars['key']));
				}

				security_key_image($vars['key'], $vars['width'], $vars['height'], $vars['bg_colour'], $vars['text_colour'], $vars['border_colour'], $vars['zoom']);
				exit();
			break;

			case 'rollback_view_start' :
				if (isset($_GET['assetid']) && isset($_GET['version'])) {
					// work out when the version of the asset was created
					$assetid = $_GET['assetid'];
					$version = $_GET['version'];

					// note that we dont prep this query for rollback select
					// because we are selecting directly from the rollback table
					$sql = 'SELECT '.SQ_TABLE_PREFIX.'effective_from AS efrom
							FROM '.SQ_TABLE_ROLLBACK_PREFIX.'asset
							WHERE assetid = '.$this->db->quote($assetid).'
							  AND version = '.$this->db->quote($version);

					$then = $this->db->getOne($sql);
					if (DB::isError($then)) {
						trigger_error($then->getMessage().'<br/>'.$then->getUserInfo(), E_USER_WARNING);
						return Array();
					}

					$_SESSION['sq_rollback_view'] = Array('rollback_time' => $then);
				} else {
					$_SESSION['sq_rollback_view'] = Array();
				}

				unset($_GET['assetid']);
				unset($_GET['version']);
			break;

			case 'rollback_view_stop' :
				if (isset($_SESSION['sq_rollback_view'])) unset($_SESSION['sq_rollback_view']);
			break;

			case 'hipo' :
				$hh = &$this->getHipoHerder();
				if (!$hh->processWeb()) trigger_error('HIPO failed to process', E_USER_ERROR);
				$hh->paintWeb();
				exit();
			break;

			case 'set_design_name' :
				$_SESSION['SQ_DESIGN_NAME'] = (isset($_GET['SQ_DESIGN_NAME'])) ? $_GET['SQ_DESIGN_NAME'] : '';
				unset($_GET['SQ_DESIGN_NAME']);
			break;

		}//end switch

		// just so it isn't being used anywhere else
		$_GET['SQ_ACTION'] = '';

	}//end _processGlobalActions()


	/**
	* Generates a new login key and returns it
	*
	* @access public
	*/
	function generateLoginKey()
	{
		require_once SQ_FUDGE_PATH.'/general/general.inc';
		return $_SESSION['SQ_LOGIN_KEY'] = random_password(20);

	}//end generateLoginKey()


	/**
	* Returns the current login key
	*
	* @access public
	*/
	function loginKey()
	{
		return $_SESSION['SQ_LOGIN_KEY'];

	}//end loginKey()


//--        CURRENT USER        --//


	/**
	* Set's the current user of the system to this user,
	* SHOULD BE USED WITH CAUTION
	*
	* @param object User	$user	the user to check
	*
	* @access public
	* @return boolean true on success
	*/
	function setCurrentUser(&$user)
	{
		// They can only become current user if they can login
		if (!$user->canLogin()) return false;

		if (!is_null($this->user)) {
			if (!isset($this->_tmp['current_user_cache'])) $this->_tmp['current_user_cache'] = Array();
			array_push($this->_tmp['current_user_cache'], Array($this->user->id, $this->user->type()));
		}

		$this->user = &$user;
		$this->_user_is_root      = $this->userRoot($user);
		$this->_user_is_sys_admin = $this->userSystemAdmin($user);

		return true;

	}// end setCurrentUser()


	/**
	* Reset's the current user to the previously assigned user
	* SHOULD BE USED WITH CAUTION
	*
	* @access public
	* @return boolean true on success
	*/
	function restoreCurrentUser()
	{
		if (empty($this->_tmp['current_user_cache'])) {
			$user = null;
		} else {
			list($userid, $type_code) = array_pop($this->_tmp['current_user_cache']);
			$user = &$this->am->getAsset($userid, $type_code);
		}

		// They can only become current user if they can login
		if (is_null($user) || !$user->canLogin()) {
			unset($this->user); // unset ref first so we don't blow away other references to the user
			$this->user = null;
			$this->_user_is_root      = false;
			$this->_user_is_sys_admin = false;

		} else {
			$this->user = &$user;
			$this->_user_is_root      = $this->userRoot($user);
			$this->_user_is_sys_admin = $this->userSystemAdmin($user);
		}

		return true;

	}// end setCurrentUser()


	/**
	* Returns a boolean if the passed user is the logged in user
	*
	* @param object User	$user	the user to check
	*
	* @access public
	*/
	function currentUser($user)
	{
		return ($this->user && $this->user->id == $user->id);

	}//end currentUser()


	/**
	* Returns the userid of the current user, or zero if none present (ie frontend)
	*
	* @access public
	*/
	function currentUserId()
	{
		return ($this->user) ? (int) $this->user->id : 0;

	}//end currentUserId()


	/**
	* Is the passed/current user the root user?
	*
	* @param object	$user	the user object you want to test for root access
	*
	* @return boolean
	* @access public
	*/
	function userRoot($user=null)
	{
		if (is_null($user)) return $this->_user_is_root;
		if (!$user->id) return false;
		if (get_class($user) == 'root_user') return true;
		return false;

	}//end userRoot()


	/**
	* Is the passed/current user a system administrator?
	*
	* @param object	$user	the user object you want to test for sysadmin access
	*
	* @return boolean
	* @access public
	*/
	function userSystemAdmin($user=null)
	{
		if (is_null($user)) return $this->_user_is_sys_admin;
		if (!$user->id) return false;
		if (get_class($user) != 'system_user') return false;

		// now make sure that the user is in the system administrators group
		$sys_admin_group = &$this->am->getSystemAsset('system_user_group');
		$sys_admins = $this->am->getLinks($sys_admin_group->id, SQ_LINK_TYPE_1, 'system_user');
		foreach ($sys_admins as $data) if ($data['minorid'] == $user->id) return true;

		return false;

	}//end userSystemAdmin()


//--        GET MANAGERS        --//


	/**
	* Get the messaging service object so you can send/receive messages
	* Always call this function to get the messaging service, dont use $this->ms directly
	*
	* @return object Messaging_Service
	* @access public
	*/
	function &getMessagingService()
	{
		if(!isset($this->ms) || get_class($this->ms) != 'messaging_service') {
			require_once SQ_INCLUDE_PATH.'/messaging_service.inc';
			$this->ms = new Messaging_Service();
		}
		return $this->ms;

	}//end getMessagingService()


	/**
	* Get the file versioning object so you can add/remove/commit/checkout file versions
	* Always call this function to get the file versioning object, dont use $this->fv directly
	*
	* @return object File_Versioning
	* @access public
	*/
	function &getFileVersioning()
	{
		if(!isset($this->fv) || get_class($this->fv) != 'file_versioning') {
			require_once SQ_FUDGE_PATH.'/file_versioning/file_versioning.inc';
			$this->changeDatabaseConnection('db'); // just to make sure
			$this->fv = new File_Versioning(SQ_DATA_PATH.'/file_repository', $this->db);
			$this->restoreDatabaseConnection();
		}
		return $this->fv;

	}//end getFileVersioning()


	/**
	* Get the file versioning object so you can add/remove/commit/checkout file versions
	* Always call this function to get the file versioning object, dont use $this->fv directly
	*
	* @return object Hipo_Herder
	* @access public
	*/
	function &getHipoHerder()
	{
		if(!isset($this->hh) || get_class($this->hh) != 'hipo_herder') {
			require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_herder.inc';
			$this->hh = new Hipo_Herder();
		}
		return $this->hh;

	}//end getHipoHerder()


	/**
	* Get the metadata manager so you can manage metadata in the system
	* Always call this function to get the metadata manager, dont use $this->mm directly
	*
	* @return object Metadata_Manager
	* @access public
	*/
	function &getMetadataManager()
	{
		if(!isset($this->mm) || get_class($this->mm) != 'metadata_manager') {
			require_once SQ_INCLUDE_PATH.'/metadata_manager.inc';
			$this->mm = new Metadata_Manager();
		}
		return $this->mm;

	}//end getMetadataManager()


	/**
	* Get the workflow manager so you can manage workflow in the system
	* Always call this function to get the workflow manager, dont use $this->wm directly
	*
	* @return object Workflow_Manager
	* @access public
	*/
	function &getWorkflowManager()
	{
		if(!isset($this->wm) || get_class($this->wm) != 'workflow_manager') {
			require_once SQ_INCLUDE_PATH.'/workflow_manager.inc';
			$this->wm = new Workflow_Manager();
		}
		return $this->wm;

	}//end getWorkflowManager()


	/**
	* Get the indexing manager 
	*
	* @return object Indexing_Manager
	* @access public
	*/
	function &getIndexingManager()
	{
		if(!isset($this->im) || get_class($this->im) != 'indexing_manager') {
			require_once SQ_INCLUDE_PATH.'/indexing_manager.inc';
			$this->im = new Indexing_Manager();
		}
		return $this->im;

	}//end getIndexingManager()


	/**
	* get the Event Manager
	*
	* @access public
	* @return object Event_Manager
	*/
	function &getEventManager()
	{
		if(!isset($this->em) || get_class($this->em) != 'event_manager') {
			require_once SQ_INCLUDE_PATH.'/event_manager.inc';
			$this->em = new Event_Manager();
		}
		return $this->em;

	} // end getEvemtManager()


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
		switch($conn){
			case 'db'  :
				if (empty($this->_db_conns[$conn])) {
					$this->_db_conns[$conn] = DB::connect(SQ_CONF_DB_DSN);
					if (DB::isError($this->_db_conns[$conn])) {
						trigger_error($this->_db_conns[$conn]->getMessage(), E_USER_ERROR);
					}
					// make sure all results are assoc arrays
					$this->_db_conns[$conn]->setFetchMode(DB_FETCHMODE_ASSOC);
				}// endif

				array_push($this->_db_stack, $conn);
				$this->db = &$this->_db_conns[$conn];

				break;

			case 'db2' :
				// the second database connection is only valid if we are running from the command-line
				// because if we aren't then the script isn't going to take long enough to warrant opening
				// a second connection
				if (SQ_PHP_CLI) {
					if (empty($this->_db_conns[$conn])) {
						$this->_db_conns[$conn] = DB::connect(SQ_CONF_DB2_DSN);
						if (DB::isError($this->_db_conns[$conn])) {
							trigger_error($this->_db_conns[$conn]->getMessage(), E_USER_ERROR);
						}
						// make sure all results are assoc arrays
						$this->_db_conns[$conn]->setFetchMode(DB_FETCHMODE_ASSOC);
					}// endif
					array_push($this->_db_stack, $conn);
					$this->db = &$this->_db_conns[$conn];

				} else {
					array_push($this->_db_stack, 'db');

				}// end if

				break;
			default :
				trigger_error('Database Connection Type "'.$conn.'" unknown', E_USER_WARNING);
		}// end switch

	}// end changeDatabaseConnection()


	/**
	* Restores the active DB connection to the previous one
	*
	* @return void
	* @access public
	*/
	function restoreDatabaseConnection()
	{
		array_pop($this->_db_stack);

		if (empty($this->_db_stack)) {
			unset($this->db);
			$this->db = null;
		} else {
			$conn = $this->_db_stack[count($this->_db_stack) - 1];
			$this->db = &$this->_db_conns[$conn];
		}

	}// end restoreDatabaseConnection()


	/**
	* Begins, Commits, or Rolls back a transaction
	* Call as many times as you like
	* Ensures the transaction manager has been created
	*
	* @param string	$type	the type of transaction to do (begin, commit, roolback)
	*
	* @return void
	* @access public
	*/
	function doTransaction($type)
	{
		if (!isset($this->tm) || get_class($this->tm) != 'transaction_manager') {
			require_once SQ_INCLUDE_PATH.'/transaction_manager.inc';
			$this->tm = new Transaction_Manager();
		}
		// Use this when there is a transaction problem to create a log and then run this from the cmd line 
		// and look for the fn with the odd number of calls
		// cat trans.log | awk '{ print $3 }' | sort | uniq -c # number of calls in a fn
		// cat trans.log | awk '{ print $5 }' | sort | uniq -c # total number of each type of call
		#$bt = debug_backtrace();
		#error_log('TRANS : '.$bt[1]['class'].'::'.$bt[1]['function'].' -- '.$type."\n", 3, SQ_SYSTEM_ROOT.'/trans.log');
		switch (strtolower($type)) {
			case 'begin'    :
				$this->tm->begin($this->_db_stack[count($this->_db_stack) - 1]);
				break;
			case 'commit'   :
				$this->tm->commit($this->_db_stack[count($this->_db_stack) - 1]);
				break;
			case 'rollback' :
				$this->tm->rollback($this->_db_stack[count($this->_db_stack) - 1]);
				break;
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
		$time_string = readable_datetime($then).' ('.easy_time_total(time() - $then, true).' ago)';
		$warning_message = 'The system is as it appeared on '.$time_string.'.<br/>While viewing in rollback mode you cannot make any changes to the system.';
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
							<td class="sq-rollback-warning" align="right">Exit Rollback View <a href="#" onClick="Javascript: window.top.location='<?php echo $exit_url; ?>'; return false" target="top" class="sq-rollback-warning" style="cursor: pointer;"><script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo sq_web_path('lib').'/web/images/icons/delete.png'; ?>", "16", "16", "Exit Rollback View");</script></a></u>
							</td>
						</tr>
						<tr>
							<td class="sq-rollback-warning" align="center"><u>You are currently viewing the system in rollback mode</u>
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
	* @param string	$where			a current where clause without WHERE on the front
	*								NOTE: if the string begins with WHERE, it will be stripped
	* @param string	$table_alias	the table alias to add entries for
	*								eg. SELECT * FROM asset a --> $table_alias is 'a'
	*								so rollback queries for table 'a' get added, otherwise
	*								the entries will not be prefixed with an alias
	* @param string	$prefix			the prefix (eg 'WHERE' or 'AND') to put at the start of
	*								the condition
	*
	* @return string
	* @access public
	*/
	function constructRollbackWhereClause($where='', $table_alias=null, $prefix='WHERE')
	{
		// trim off a current prefix keyword if it exists
		$where = trim($where);
		if (strtoupper(substr($where, 0, strlen($prefix))) == strtoupper($prefix)) $where = substr($where, 5);


		if (!SQ_ROLLBACK_VIEW) {
			// we are not in rollback view so we dont need anything special in the where clause
			if (empty($where)) return '';
			else return ' '.$prefix.' '.$where;
		}

		if (!is_null($table_alias)) $table_alias .= '.';
		else $table_alias = '';

		$then = $_SESSION['sq_rollback_view']['rollback_time'];

		if (!empty($where)) $where .= ' AND ';
		$where .= $table_alias.SQ_TABLE_PREFIX.'effective_from <= '.$this->db->quote($then).'
				  AND ('.$table_alias.SQ_TABLE_PREFIX.'effective_to IS NULL
					   OR '.$table_alias.SQ_TABLE_PREFIX.'effective_to > '.$this->db->quote($then).')';

		return ' '.$prefix.' '.$where;

	}//end constructRollbackWhereClause()


	/**
	* Given a table name and a where clause, this function will copy the rows from the table
	* to the rollback table. It will handle the time of the rollback across script execution
	* as well as any conflicts in the times
	*
	* @param string	$table_name	the base name of the table (without prefixes)
	*							eg. asset, workflow etc
	* @param string	$where_cond	a string to tack onto the WHERE clause of the queries
	*							to limit the rows have rollback entries created for
	*
	* @return boolean
	* @access public
	*/
	function addRollbackEntry($table_name, $where_cond)
	{
		// if we are in rollback view, pretend e did our job but really just slack off
		if (SQ_ROLLBACK_VIEW) return true;

		if (!SQ_CONF_ROLLBACK_ENABLED) {
			// rollback is not enabled on the system, so return true
			// here to let the system know everything is going to plan
			return true;
		}

		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		// work out the columns in this table and cache them
		if (empty($this->_tmp['sq_tables'])) {
			require_once SQ_DATA_PATH.'/private/db/table_columns.inc';
			$this->_tmp['sq_tables'] = $tables;
		}
		$table_cols = $this->_tmp['sq_tables'][$table_name]['columns'];

		$col_string = implode(', ', $table_cols);
		$where_cond = trim($where_cond);
		static $effective_from;

		// $effective_from is declared as a static variable so that every time
		// this function gets called through a script execution, the effective_from
		// time is always the same, and so rollback entries are in sync
		if (empty($effective_from)) $effective_from = time();
		$from_time = $effective_from;

		$delete = ' DELETE FROM '.SQ_TABLE_ROLLBACK_PREFIX.$table_name.'
					WHERE '.SQ_TABLE_PREFIX.'effective_from = '.$this->db->quote(ts_iso8601($from_time)).'
					  AND '.$where_cond;

		// set the effective_to time of the latest rollback entry to the
		// effective_from of the new entry
		$update = ' UPDATE '.SQ_TABLE_ROLLBACK_PREFIX.$table_name.'
					SET '.SQ_TABLE_PREFIX.'effective_to = '.$this->db->quote(ts_iso8601($from_time)).'
					WHERE '.SQ_TABLE_PREFIX.'effective_to IS NULL
					  AND '.$where_cond;

		// need an INSERT SELECT to add a rollback entry because we are basically just copying rows
		// from the "live data" table to the rollback table
		$insert = ' INSERT INTO '.SQ_TABLE_ROLLBACK_PREFIX.$table_name.'
					('.SQ_TABLE_PREFIX.'effective_from, '.SQ_TABLE_PREFIX.'effective_to, '.$col_string.') ';
		$select = ' SELECT '.$this->db->quote(ts_iso8601($from_time)).', NULL, '.$col_string.'
					FROM sq_'.$table_name.'
					WHERE '.$where_cond;

		$this->doTransaction('BEGIN');

		// first we delete all entries in the rollback table that have the same from time
		// and that we are going to be replacing because their value is no longer the most recent
		// in this script execution
		$result = $this->db->query($delete);
		if (DB::isError($result)) {
			$this->doTransaction('ROLLBACK');
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		// replace the NULL value effective_to entries because we are adding our own now
		$result = $this->db->query($update);
		if (DB::isError($result)) {
			$this->doTransaction('ROLLBACK');
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		// and finally insert the new rollback rows
		$result = db_extras_insert_select($this->db, $insert, $select);
		if (DB::isError($result)) {
			$this->doTransaction('ROLLBACK');
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		$this->doTransaction('COMMIT');
		return true;

	}//end addRollbackEntry()


	/**
	* Constructs and executes an update query before adding rollback entries for the rows
	* that were updated by the query
	*
	* @param string	$table_name		the base name of the table (without prefixes)
	*								eg. asset, workflow etc
	* @param array	$update_values	an array in form column => new_value
	*								NOTE: new values must be quoted outside (ie. $db->quote())
	* @param string	$where_cond		a string to tack onto the WHERE clause of the query
	*								to limit the rows that get updated
	*
	* @return boolean
	* @access public
	*/
	function rollbackUpdate($table_name, $update_values, $where_cond)
	{
		// if we are in rollback view, pretend e did our job but really just slack off
		if (SQ_ROLLBACK_VIEW) return true;

		$this->doTransaction('BEGIN');

		// construct the update query
		$update_sql = 'UPDATE '.SQ_TABLE_PREFIX.$table_name.' SET ';
		foreach ($update_values as $column => $value) {
			$update_sql .= $column.' = '.$value.', ';
		}
		$update_sql = trim($update_sql, ', ');
		if (!empty($where_cond)) $update_sql .= ' WHERE '.$where_cond;

		$result = $this->db->query($update_sql);
		if (DB::isError($result)) {
			$this->doTransaction('ROLLBACK');
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		// add the rollback entries for the rows we updated
		if (!$this->addRollbackEntry($table_name, $where_cond)) {
			$this->doTransaction('ROLLBACK');
			return false;
		}

		$this->doTransaction('COMMIT');
		return true;

	}//end rollbackUpdate()


	/**
	* Constructs and executes an insert query and then adds rollback entries for the rows
	* that were just inserted
	*
	* @param string	$table_name		the base name of the table (without prefixes)
	*								eg. asset, workflow etc
	* @param array	$insert_values	an array in form column => value
	*								NOTE: new values should NOT be quoted outside (ie. $db->quote())
	*
	* @return boolean
	* @access public
	*/
	function rollbackInsert($table_name, $insert_values)
	{
		// if we are in rollback view, pretend e did our job but really just slack off
		if (SQ_ROLLBACK_VIEW) return true;

		// we work out the primary key fields for the table we are inserting into
		// so we can generate a WHERE clause based on the primary key fields and send
		// that to the function that handles rollback so that it know which rows need
		// rollback entries created for them
		if (empty($this->_tmp['sq_tables'])) {
			require_once SQ_DATA_PATH.'/private/db/table_columns.inc';
			$this->_tmp['sq_tables'] = $tables;
		}
		$primary_cols = $this->_tmp['sq_tables'][$table_name]['primary_key'];

		$col_string = '';
		$val_string = '';
		$where_cond = '';

		// go through now and quote all the values we are inserting, as well as
		// generate the WHERE clause based on primary key fields
		foreach ($insert_values as $column => $value) {
			$col_string .= $column.', ';
			$val_string .= $this->db->quote($value).', ';
			if (in_array($column, $primary_cols)) {
				if (!empty($where_cond)) $where_cond .= ' AND ';
				$where_cond .= $column.' = '.$this->db->quote($value);
			}
		}
		$col_string = trim($col_string, ', ');
		$val_string = trim($val_string, ', ');

		// construct the insert query
		$insert_sql = 'INSERT INTO '.SQ_TABLE_PREFIX.$table_name.' ('.$col_string.') VALUES ('.$val_string.')';
		$this->doTransaction('BEGIN');

		$result = $this->db->query($insert_sql);
		if (DB::isError($result)) {
			$this->doTransaction('ROLLBACK');
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		// add the rollback entries for the inserted rows
		if (!$this->addRollbackEntry($table_name, $where_cond)) {
			$this->doTransaction('ROLLBACK');
			return false;
		}

		$this->doTransaction('COMMIT');
		return true;

	}//end rollbackInsert()


	/**
	* Constructs and executes an insert select query and then adds rollback entries for the rows
	* that were just inserted
	*
	* @param string	$table_name		the base name of the table (without prefixes)
	*								eg. asset, workflow etc
	* @param string	$select_from	the FROM clause (eg. asset a INNER JOIN asset_link l ON...)
	* @param array	$insert_values	an array in form insert_column => select_column
									where inset_column is the column in the INSERT table and
									select_column is the column in the SELECT clause
	* @param string	$where_cond		a string to tack onto the WHERE clause of the query
	*								to limit the rows that get selected for inserting
	*
	* @return boolean
	* @access public
	*/
	function rollbackInsertSelect($table_name, $select_from, $insert_values, $where_cond)
	{
		// if we are in rollback view, pretend e did our job but really just slack off
		if (SQ_ROLLBACK_VIEW) return true;

		// we work out the primary key fields for the table we are inserting into
		// so we can generate a WHERE clause based on the primary key fields and send
		// that to the function that handles rollback so that it know which rows need
		// rollback entries created for them
		if (empty($this->_tmp['sq_tables'])) {
			require_once SQ_DATA_PATH.'/private/db/table_columns.inc';
			$this->_tmp['sq_tables'] = $tables;
		}
		$primary_cols = $this->_tmp['sq_tables'][$table_name]['primary_key'];

		// we construct a CONCAT clause here, based on the different database types
		// and their quirks, that we are going to use in
		$concat_string = '';
		$concat_col = '';
		$concat_values = Array();
		foreach ($insert_values as $col => $value) {
			if (in_array($col, $primary_cols)) $concat_values[$col] = $value;
		}

		// INSERT SELECTS are funny because to work out the rows that just got inserted, we
		// actually contrust a sub-query to pass to the rollback function by concatenating
		// the primary key fields with a '~' and matching on that
		// EXAMPLE: assetid~version IN ('12~0.0.2') rather than asset=12 AND version='0.0.2'
		foreach ($insert_values as $col => $value) {
			if ($this->db->phptype == 'mysql') {
				$concat_string = 'CONCAT('.implode(', '.$this->db->quote('~').', ', array_values($concat_values)).')';
				$concat_col = 'CONCAT('.implode(', '.$this->db->quote('~').', ', array_keys($concat_values)).')';
			} else {
				$concat_string = implode(' || '.$this->db->quote('~').' || ', array_values($concat_values));
				$concat_col = implode(' || '.$this->db->quote('~').' || ', array_keys($concat_values));
			}
		}

		// and here is the sub-query, you were promised, to work out the WHERE clause
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
		$where_clause = $concat_col.' IN (~SQ0~)';
		$subs = Array('SELECT '.$concat_string.' FROM '.$select_from.' WHERE '.$where_cond);
		$where_clause = db_extras_subquery($this->db, $where_clause, $subs);

		if (DB::isError($where_clause)) {
			trigger_error($where_clause->getMessage().'<br/>'.$where_clause->getUserInfo(), E_USER_WARNING);
			$this->doTransaction('ROLLBACK');
			return 0;
		}

		// generate the column header string for the INSERT and SELECT tables
		$col_string = implode(', ', array_keys($insert_values));
		$select_col_string = implode(', ', array_values($insert_values));

		// construct the queries
		$insert_sql = 'INSERT INTO '.SQ_TABLE_PREFIX.$table_name.' ('.$col_string.')';
		$select_sql = 'SELECT '.$select_col_string. ' FROM '.$select_from.' WHERE '.$where_cond;

		$this->doTransaction('BEGIN');

		$result = db_extras_insert_select($this->db, $insert_sql, $select_sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			$this->doTransaction('ROLLBACK');
			return false;
		}

		// add the rollback entries for the inserted rows
		if (!$this->addRollbackEntry($table_name, $where_clause)) {
			$this->doTransaction('ROLLBACK');
			return false;
		}

		$this->doTransaction('COMMIT');
		return true;

	}//end rollbackInsert()


	/**
	* Constructs and executes a delete query after adding rollback entries for the rows
	* that are going to be deleted by the query
	*
	* @param string	$table_name		the base name of the table to delete from (without prefixes)
	*								eg. asset, workflow etc
	* @param string	$where_cond		a string to tack onto the WHERE clause of the query
	*								to limit the rows that get deleted
	*
	* @return boolean
	* @access public
	*/
	function rollbackDelete($table_name, $where_cond)
	{
		// if we are in rollback view, pretend e did our job but really just slack off
		if (SQ_ROLLBACK_VIEW) return true;

		$this->doTransaction('BEGIN');

		// add the rollback entries for the rows we are going to delete
		if (!$this->addRollbackEntry($table_name, $where_cond)) {
			$this->doTransaction('ROLLBACK');
			return false;
		}

		// construct the delete query
		$delete_sql = 'DELETE FROM '.SQ_TABLE_PREFIX.$table_name.' WHERE '.$where_cond;

		$result = $this->db->query($delete_sql);
		if (DB::isError($result)) {
			$this->doTransaction('ROLLBACK');
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		$this->doTransaction('COMMIT');
		return true;

	}//end rollbackDelete()


//--        LOCKING        --//


	/**
	* Attempts to acquire a lock
	*
	* @param string		$lockid			the id of the lock
	* @param string		$source_lockid	the id of the lock that started this locking process
	* @param int|NULL	$expires		when the lock expires (timestamp or NULL)
	*									If expires is set to NULL, the lock will be acquired indefinitely (ie. it will never expire).
	*									If expires is set to zero, the lock will be acquired for the current time plus SQ_CONF_LOCK_LENGTH
	*
	* @return TRUE|string	returns true on success or the error message on failure
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
				$user = &$this->getAsset($current_lock['userid']);
				return 'Cannot acquire lock, lock already held by "'.$user->name.'"';
			}// end if
		}// end if

		// if we have no source, we are the asset aquiring the locks
		if (empty($source_lockid)) $source_lockid = $lockid;

		if (!is_null($expires)) {
			$expires = (empty($expires)) ? (time() + SQ_CONF_LOCK_LENGTH) : (int) $expires;
			$expires = ts_iso8601($expires);
		}

		$this->changeDatabaseConnection('db2');
		$sql = 'INSERT INTO '.SQ_TABLE_PREFIX.'lock 
				(lockid, source_lockid, userid, expires) 
				VALUES 
				('.$this->db->quote($lockid).','.$this->db->quote($source_lockid).','.$this->db->quote($this->currentUserid()).','.$this->db->quote($expires).')';
		$result = $this->db->query($sql);
		$this->restoreDatabaseConnection();
		if (DB::isError($result)) { 
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return 'Database Error';
		}
		return true;

	}//end acquireLock()


	/**
	* Release the passed lock
	*
	* @param string	$lockid		the id of the lock to release
	*
	* @return true|string	true on success, error message on failure
	* @access public
	*/
	function releaseLock($lockid)
	{
		$current_lock = $this->getLockInfo($lockid, false, false);
		if (empty($current_lock)) return true;

		$this->changeDatabaseConnection('db2');
		$sql = 'DELETE FROM '.SQ_TABLE_PREFIX.'lock 
				WHERE source_lockid = '.$this->db->quote($current_lock['source_lockid']);

		$result = $this->db->query($sql);
		$this->restoreDatabaseConnection();
		if (DB::isError($result)) { 
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return 'Database Error';
		}
		return true;

	}//end releaseLock()


	/**
	* Updates the expiry date of the lock
	*
	* If the current lock is being held indefinitely (ie. expires = NULL), the lock
	* expiry time will not be updated but the function will return true. You must release
	* an indefinite lock before updating its expiry date.
	*
	* @param string		$lockid		the id of the lock
	* @param int|NULL	$expires	when the lock expires (timestamp or NULL)
	*
	* @return true|string	true on success, error message on failure
	* @access public
	*/
	function updateLock($lockid, $expires=0)
	{
		$current_lock = $this->getLockInfo($lockid);
		if (empty($current_lock) || empty($current_lock['expires'])) return true;

		if (!is_null($expires)) {
			$expires = (!$expires) ? (time() + SQ_CONF_LOCK_LENGTH) : (int) $expires;
			$expires = ts_iso8601($expires);
		}

		$this->changeDatabaseConnection('db2');
		$sql = 'UPDATE '.SQ_TABLE_PREFIX.'lock 
				   SET expires = '.$this->db->quote($expires).'
				WHERE source_lockid = '.$this->db->quote($current_lock['source_lockid']);

		$result = $this->db->query($sql);
		$this->restoreDatabaseConnection();
		if (DB::isError($result)) { 
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return 'Database Error';
		}
		return true;

	}//end updateLock()


	/**
	* Returns an array of information about the passed lock
	*
	* @param string		$lockid			the id of the lock
	* @param boolean	$full_chain		return the info for every lock in the lock chain
	* @param boolean	$check_expires	check the expiry date and cleanup the lock if it has expired
	*
	* @return Array
	* @access public
	*/
	function getLockInfo($lockid, $full_chain=false, $check_expires=true)
	{
		$this->changeDatabaseConnection('db2');
	
		$sql = 'SELECT lockid, source_lockid, userid, expires
				FROM '.SQ_TABLE_PREFIX.'lock 
				WHERE lockid = '.$this->db->quote($lockid);

		$result = $this->db->getRow($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			$this->restoreDatabaseConnection();
			return Array();
		}

		if (!empty($result)) {
			$result['expires'] = empty($result['expires']) ? NULL : iso8601_ts($result['expires']);
			if ($check_expires && !empty($result['expires'])) {
				if ($result['expires'] < time()) {
					$this->releaseLock($lockid);
					$this->restoreDatabaseConnection();
					// this lock has expired
					return Array();
				}// end if
			}// end if
			
			if ($full_chain) {
				$result['chained_assets'] = Array();
				$sql = 'SELECT lockid, source_lockid, userid, expires
						FROM '.SQ_TABLE_PREFIX.'lock 
						WHERE source_lockid = '.$this->db->quote($result['source_lockid']).'
						  AND lockid <> '.$this->db->quote($lockid);
				$chain = $this->db->getAll($sql);
				if (DB::isError($chain)) {
					trigger_error($chain->getMessage().'<br/>'.$chain->getUserInfo(), E_USER_WARNING);
					$this->restoreDatabaseConnection();
					return Array();
				}

				foreach ($chain as $lock_row) {
					$lock_row['expires'] = (empty($lock_row['expires'])) ? NULL : iso8601_ts($lock_row['expires']);
					$result['chained_assets'][$lock_row['lockid']] = $lock_row;
				}

			}// end if

		}// endif 

		$this->restoreDatabaseConnection();
		return $result;

	}//end getLockInfo()


//--        SYSTEM MESSAGES        --//


	/**
	* Add a system message
	*
	* @access public
	*/
	function addMessage($msg)
	{
		$this->_msgs[] = $msg;

	}//addMessage()


	/**
	* Returns the system messages
	*
	* @return Array(string)
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
	function datetime($timestamp=null)
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
	function date($timestamp=null)
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
	function time($timestamp=null)
	{
		return (is_null($timestamp)) ? date('H:i:s') : date('H:i:s', $timestamp);

	}//end time()

}//end class

?>
