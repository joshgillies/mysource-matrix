<?php

require_once 'DB.php';
require_once SQ_INCLUDE_PATH.'/resolve_object.inc';
require_once SQ_INCLUDE_PATH.'/asset_manager.inc';

/**
* Resolve
*
* Purpose
*    This is the object from which everything else is run
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
*/
class Resolve
{

	/**
	* this is the DB object
	*
	* @var object DB
	*/
	var $db;

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
	* Object of logged in user
	*
	* @var object User
	*/
	var $user;

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
	function Resolve()
	{
	}// end constructor

	/**
	* This initialises all the objects that we need to use, the reason that we can't do this in the 
	* constructor is that the Asset_Manager uses the DB in it's constructor in order to load the asset types
	* but if initialise the Asset_Manager in our constructor we don't exist and therefore cannot be accessed
	* to get the DB for the Asset_Manager to use
	*
	* @access public
	*/
	function init()
	{
		session_name('SQ_SYSTEM_SESSION');
		session_save_path(SQ_SYSTEM_ROOT.'/cache');
		session_start();

		$this->db = DB::connect(SQ_CONF_DB_DSN);
		if (DB::isError($this->db)) {
			trigger_error($this->db->getMessage(), E_USER_ERROR);
		}
		// make sure all results are assoc arrays
		$this->db->setFetchMode(DB_FETCHMODE_ASSOC);

		// initialise the asset manager as we are going to need it pretty much everywhere
		$this->am = new Asset_Manager();

		// If there is a userid in the session then someone is logged in,
		// but make sure that the person is coming from the same machine as they logged in on
		// so get a reference to them
		if(!empty($_SESSION['userid']) && $_SESSION['remote_addr'] == $_SERVER['REMOTE_ADDR']) {
			$this->user = &$this->am->getAsset($_SESSION['userid']);
		} else {
			$_SESSION['userid'] = 0;
			$this->user = null;
		}//end if

		if (!empty($_GET['SQ_ACTION'])) {
			$this->_processGlobalActions();
		}//end if


	}// end init()


	/**
	* From here is where everything else happens, it starts the ball rolling
	*
	* @access public
	*/
	function start()
	{
		// initiailise event listeners
		$GLOBALS['SQ_EVENT_LISTENER_OBJECTS'] = array();
		$GLOBALS['SQ_EVENT_LISTENERS'] = array();

		if (SQ_IN_BACKEND) {
			if ($this->user) {
				require_once SQ_INCLUDE_PATH.'/backend.inc';
				$backend = new Backend();
				$backend->paint();
			} else {
				$this->paintLogin('Login', 'You need to login before you can access the backend');
			}

		// we are on the frontend
		} else {

			$asset = &$this->am->getAssetFromURL(current_protocol());
			if (is_null($asset)) exit(1);
			$asset->printFrontend();

		}//end if

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
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		header("HTTP/1.0 403 Forbidden");
		$GLOBALS['SQ_LOGIN_FORM'] = Array('paint' => 'login', 'heading' => $heading, 'msg' => $msg);

		$current_asset = &$this->am->getAssetFromURL(null, null, true, true);
		// if we can't find a current asset, use the root folder
		if (is_null($current_asset)) {
			$current_asset = &$this->am->getSystemAsset('root_folder');
			if (is_null($current_asset)) {
				trigger_error('Unable to get Root Folder Asset', E_USER_ERROR);
			}
		}

		$sql = 'SELECT l.login_designid, a.type_code
				FROM sq_asset_lookup l, sq_asset a
				WHERE l.login_designid = a.assetid
				  AND l.url = '.$db->quote(strip_url(current_url(false, true)));
		$result = $db->getRow($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
		}

		// we have found the design to use
		if ($result) {
			$design = &$GLOBALS['SQ_SYSTEM']->am->getAsset($result['login_designid'], $result['type_code']);
			if (!is_null($design)) {
				$design->paint($current_asset);
				return;
			}
		} 

		// OK, use the system login form
		$design = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('login_design');
		if (!is_null($design)) {
			$design->paint($current_asset);
			return;
		}

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
				if ($this->loginKey() == $_POST['SQ_LOGIN_KEY']) {
					$this->am->includeAsset('user');
					list($userid, $user_type) = User::find($_POST['SQ_LOGIN_USERNAME'], $_POST['SQ_LOGIN_PASSWORD'], 'backend_user');
					if ($userid) {
						$user = &$this->am->getAsset($userid, $user_type);
						// okay, we have a user, but we need to check that this user
						// can actually login
						if (!$user->canLogin()) {
							$this->addMessage('User not found');
						} else {
							$_SESSION['userid']      = $userid;
							$_SESSION['remote_addr'] = $_SERVER['REMOTE_ADDR'];
							// generate a new login key so that when they try and login next
							// they have to re-enter the details
							$this->generateLoginKey();

							$this->user = &$user;
						}
						break;

					} else {
						$this->addMessage('User not found');
					}
				} else {
					// incorrect login key
					$this->addMessage('Login Key incorrect');
				}

				// deliberalty don't have a break here so it the
				// login fails we make sure that we're logged out
			case 'logout' :
				$_SESSION['userid'] = 0;
				$this->user = null;
				break;
		}//end switch

		// just so it isn't being used anywhere else
		$_GET['SQ_ACTION'] = '';

	}//end _processGlobalActions()


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
		if (empty($user)) $user =& $this->user;
		if (!$user->id) return false;
		if (get_class($user) == 'root_user' && $user->id == 4) return true;
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
		if (empty($user)) $user =& $this->user;
		if (!$user->id) return false;
		if (get_class($user) != 'system_user') return false;

		// now make sure that the user is in the system administrators group
		$sys_admin_group = &$this->am->getSystemAsset('system_user_group');
		$sys_admins = $sys_admin_group->getLinks(SQ_LINK_TYPE_1, 'system_user');
		foreach ($sys_admins as $data) if ($data['minorid'] == $user->id) return true;

		return false;
	}//end userSystemAdmin()


	/**
	* Allows an object to listen to an event in the system and be notifies of it
	* Internally, classes boradcast events which the objects listening get
	* notified of
	*
	* @param object	&$object	the object that is listening (use $this for yourself)
	* @param array	$events		an array of events that the object wants to listen to
	*
	* @return boolean
	* @access public
	*/
	function addEventListener(&$object, $events=Array())
	{
		if (empty($events)) return;
		$GLOBALS['SQ_EVENT_LISTENER_OBJECTS'][] = &$object;
		$obj_index = count($GLOBALS['SQ_EVENT_LISTENER_OBJECTS']) -1;
		foreach ($events as $event) $GLOBALS['SQ_EVENT_LISTENERS'][trim($event)][$obj_index] = true;
		return true;
	}//end addEventListener()


	/**
	* Returns date time string for the passed timestamp
	* Object listening have their onEVENT functions called
	*
	* @param string	$event		the event to boradcast for
	* @param array	$event_data	some data that event functions can use
	*
	* @return boolean
	* @access public
	*/
	function broadcastEvent($event, $event_data=Array())
	{
		$function_name = 'on'.$event;
		if (empty($GLOBALS['SQ_EVENT_LISTENERS'][trim($event)])) return;
		foreach ($GLOBALS['SQ_EVENT_LISTENERS'][trim($event)] as $obj_index => $true) {
			$object = &$GLOBALS['SQ_EVENT_LISTENER_OBJECTS'][$obj_index];
			if (!method_exists($object,$function_name)) continue;
			$object->$function_name($event_data);
		}
		return true;
	}//end broadcastEvent()


	/**
	* Get the messaging service object so you can send/receive messages
	* Always call this function to get the messaging service, dont use $this->ms directly
	*
	* @return object Messaging_Service
	* @access public
	*/
	function &getMessagingService()
	{
		if(!isset($this->ms) && get_class($this->ms) != 'messaging_service') {
			include_once SQ_INCLUDE_PATH.'/messaging_service.inc';
			$this->ms = new Messaging_Service();
		}
		return $this->ms;
	}//end getMessagingService()


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
	function doTransaction($type) {
		if (get_class($this->tm) != 'transaction_manager') {
			require_once SQ_INCLUDE_PATH.'/transaction_manager.inc';
			$this->tm = new Transaction_Manager();
		}
		switch (strtolower($type)) {
			case 'begin'    :
				$this->tm->begin();
				break;
			case 'commit'   :
				$this->tm->commit();
				break;
			case 'rollback' :
				$this->tm->rollback();
				break;
		}
	}//end doTransaction()


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
	* returns the userid of the current user, or zero if none present (ie frontend)
	*
	* @access public
	*/
	function currentUserId()
	{
		return ($this->user) ? $this->user->id : 0;
	}//end currentUserId()


	/**
	* generates a new login key and returns it
	*
	* @access public
	*/
	function generateLoginKey()
	{
		require_once SQ_FUDGE_PATH.'/general/general.inc';
		return $_SESSION['SQ_LOGIN_KEY'] = random_password(20);
	}//end generateLoginKey()


	/**
	* returns the current login key
	*
	* @access public
	*/
	function loginKey()
	{
		return $_SESSION['SQ_LOGIN_KEY'];
	}//end loginKey()


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
	* returns the system messages
	*
	* @return Array(string)
	* @access public
	*/
	function messages()
	{
		return (empty($this->_msgs)) ? Array() : $this->_msgs;
	}//end messages()


	/**
	* returns date time string for the passed timestamp
	*
	* @param int	$timestamp	timestamp to format, if null uses current timestamp
	*
	* @return string
	* @access public
	*/
	function datetime($timestamp=null)
	{
		$f = 'd/m/Y H:i:s';
		return (is_null($timestamp)) ? date($f) : date($f, $timestamp);
	}//end datetime()


	/**
	* returns date string for the passed timestamp
	*
	* @param int	$timestamp	timestamp to format, if null uses current timestamp
	*
	* @return string
	* @access public
	*/
	function date($timestamp=null)
	{
		$f = 'd/m/Y';
		return (is_null($timestamp)) ? date($f) : date($f, $timestamp);
	}//end date()


	/**
	* returns time string for the passed timestamp
	*
	* @param int	$timestamp	timestamp to format, if null uses current timestamp
	*
	* @return string
	* @access public
	*/
	function time($timestamp=null)
	{
		$f = 'H:i:s';
		return (is_null($timestamp)) ? date($f) : date($f, $timestamp);
	}//end time()

}//end class
?>
