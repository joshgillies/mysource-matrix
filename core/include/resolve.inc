<?

require_once 'DB.php';
require_once SQ_INCLUDE_PATH.'/resolve_object.inc';
require_once SQ_INCLUDE_PATH.'/asset_manager.inc';

/**
* Resolve
*
* Purpose
*
*    This is the object from which everything else is run
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
*/
class Resolve {

	/**
	* @var object Asset_Manager
	*/
	var $am;

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
	* this is the DB object
	*
	* @var object DB
	*/
	var $_db;

	/**
	* Constructor
	*
	*/
	function Resolve() {
		session_name('SQ_RESOLVE_SESSION');
		session_save_path(SQ_SYSTEM_ROOT.'/cache');
		session_start();
	}

	/*
	* From here is where everything else happens, it starts the ball rolling
	*
	* @access public
	*/
	function start() {

		// initialise the asset manager as we are going to need it pretty much everywhere
		$this->am = new Asset_Manager();

#		error_log("PATH : ".$_SERVER['REQUEST_URI'].", SESSION : ".array_contents($_SESSION).", COOKIE : ".array_contents($_COOKIE));

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

		if (SQ_IN_BACKEND) {
			if (true || $this->user) {
				include_once(SQ_INCLUDE_PATH.'/backend.inc');
				$backend = new Backend();
				$backend->paint();
			} else {
				include SQ_INCLUDE_PATH.'/login.inc';
				print_login('Login', 'You need to login before you can access the backend');
			}

		// we are on the frontend
		} else {

		//	require_once SQ_INCLUDE_PATH.'/frontend.inc';
		//	$GLOBALS['SQ_FRONTEND'] = new Backend();
		//	$GLOBALS['SQ_FRONTEND']->paint();
			echo "FRONT END";

		}//end if

		if (is_object($this->_db)) {
			$this->_db->disconnect();
		}

	}//end start()


	/*
	* Processes any global actions that need to be taken care of
	*
	* @access private
	*/
	function _processGlobalActions() {

		switch($_GET['SQ_ACTION']) {
			case 'login' :
				if ($this->loginKey() == $_POST['SQ_LOGIN_KEY']) {
					$this->am->includeAsset('user');
					if ($userid = User::find($_POST['SQ_LOGIN_USERNAME'], $_POST['SQ_LOGIN_PASSWORD'])) {
						$user = &$this->am->getAsset($userid);
						$_SESSION['userid']      = $userid;
						$_SESSION['remote_addr'] = $_SERVER['REMOTE_ADDR'];
						// generate a new login key so that when they try and login next
						// they have to re-enter the details
						$this->generateLoginKey();

						$this->user = &$user;
						break;

					}//end if
				// incorrect login key
				} else {
					$this->addMessage('Login Key incorrect');
				}//end if

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


	/*
	* return a reference to the DB object
	*
	* @access public
	*/
	function &getDb() {
		if (!is_object($this->_db)) {
			$this->_db = DB::connect(SQ_CONF_DB_DSN);
			if (DB::isError($this->_db)) {
				trigger_error($this->_db->getMessage(), E_USER_ERROR);
			}
			// make sure all results are assoc arrays
			$this->_db->setFetchMode(DB_FETCHMODE_ASSOC);
		}
		return $this->_db;
	}

	/*
	* returns a boolean if the passed user is the logged in user
	*
	* @access public
	*/
	function currentUser($user) {
		return ($this->user && $this->user->id == $user->id);
	}//end currentUser()

	/*
	* returns the userid of the current user, or zero if none present (ie frontend)
	*
	* @access public
	*/
	function currentUserId() {
		return ($this->user) ? $this->user->id : 0;
	}//end currentUserId()

	/*
	* generates a new login key and returns it
	*
	* @access public
	*/
	function generateLoginKey() {
		include_once SQ_LIB_PATH.'/general/general.inc';
		return $_SESSION['SQ_LOGIN_KEY'] = random_password(20);
	}

	/*
	* returns the current login key
	*
	* @access public
	*/
	function loginKey() {
		return $_SESSION['SQ_LOGIN_KEY'];
	}

	/*
	* Add a system message
	*
	* @access public
	*/
	function addMessage($msg) {
		$this->_msgs[] = $msg;
	}

	/*
	* returns the system messages
	*
	* @return Array(string)
	* @access public
	*/
	function messages() {
		return $this->_msgs;
	}

}//end class
?>
