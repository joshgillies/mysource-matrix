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

		# initialise the asset manager as we are going to need it pretty much everywhere
		$this->am = new Asset_Manager();

		# If there is a userid in the session then someone is logged in,
		# but make sure that the person is coming from the same machine as they logged in on
		# so get a reference to them
		if(!empty($_SESSION['userid']) && $_SESSION['remote_addr'] == $_SERVER['REMOTE_ADDR']) {
			$this->user = &$this->getObject($_SESSION['userid']);
		} else {
			$_SESSION['userid'] = 0;
			$this->user = null;
		}#end if

		if (!empty($_REQUEST['SQ_ACTION'])) {
			$this->_processGlobalActions();
		}#end if

		if (SQ_IN_BACKEND) {
			if (true || $this->user) {
				include_once(SQ_INCLUDE_PATH.'/backend.inc');
				$backend = new Backend();
				$backend->paint();
			} else {
				$this->print_login();
			}

		# we are on the frontend
		} else {

		#	require_once SQ_INCLUDE_PATH.'/frontend.inc';
		#	$GLOBALS['SQ_FRONTEND'] = new Backend();
		#	$GLOBALS['SQ_FRONTEND']->paint();
			echo "FRONT END";

		}#end if

		if (is_object($this->_db)) {
			$this->_db->disconnect();
		}

	}#end start()


	/*
	* Processes any global actions that need to be taken care of
	*
	* @access private
	*/
	function _processGlobalActions() {

		switch($_REQUEST['SQ_ACTION']) {
			case 'login' :
				if ($this->login_key() == $_POST['login_key']) {
					$user = new User();
					if ($user->login($_POST['login_username'], $_POST['login_password'])) {
						$_SESSION['userid'] = $user->get_userid();
						$_SESSION['remote_addr'] = $_SERVER['REMOTE_ADDR'];
						# generate a new login key so that when they try and login next
						# they have to re-enter the details
						$this->gen_login_key();

						$this->user = &$user;
						$this->register_object($this->user);
						break;

					}#end if
				# incorrect login key
				} else {
					$this->outputter->add_message(SQ_MSG_ERROR, 'Login Key incorrect');
				}#end if

				# deliberalty don't have a break here so it the
				# login fails we make sure that we're logged out
			case 'logout' :
				$_SESSION['userid'] = 0;
				$this->user = null;
			break;
		}#end switch

		# just so it isn't being used anywhere else
		$_REQUEST['SQ_ACTION'] = '';

	}#end _processGlobalActions()


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
			# make sure all results are assoc arrays
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
	}#end currentUser()

	/*
	* returns the userid of the current user, or zero if none present (ie frontend)
	*
	* @access public
	*/
	function currentUserId() {
		return ($this->user) ? $this->user->id : 0;
	}#end currentUserId()


	 ############################################################
	# generates a new login key
	function gen_login_key() {
		$_SESSION['login_key'] = random_password(20);
	}#end gen_login_key()

	 ############################################################
	# generates a new login key
	function login_key() {
		return $_SESSION['login_key'];
	}#end login_key()

}#end class
?>
