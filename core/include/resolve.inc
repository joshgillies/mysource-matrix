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
	* @var object File_Versioning
	*/
	var $fv;

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
	function Resolve()
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
		$this->db = DB::connect(SQ_CONF_DB_DSN);
		if (DB::isError($this->db)) {
			trigger_error($this->db->getMessage(), E_USER_ERROR);
		}
		// make sure all results are assoc arrays
		$this->db->setFetchMode(DB_FETCHMODE_ASSOC);

		// initialise the asset manager as we are going to need it pretty much everywhere
		$this->am = new Asset_Manager();

		// running from the command line ? don't use cookies for session
		if (!SQ_PHP_CLI) {
			session_name('SQ_SYSTEM_SESSION');
			session_save_path(SQ_SYSTEM_ROOT.'/cache');
			session_start();

			// If there is a userid in the session then someone is logged in,
			// but make sure that the person is coming from the same machine as they logged in on
			// so get a reference to them
			if(!empty($_SESSION['userid']) && $_SESSION['remote_addr'] == $_SERVER['REMOTE_ADDR']) {
				$this->user = &$this->am->getAsset($_SESSION['userid']);
			} else {
				$_SESSION['userid']				= 0;
				$_SESSION['user_is_root']		= false;
				$_SESSION['user_is_sys_admin']	= false;
				$this->user = null;
			}//end if

			$this->user_is_root      = $_SESSION['user_is_root'];
			$this->user_is_sys_admin = $_SESSION['user_is_sys_admin'];

		}// end if

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
			$asset = &$this->am->getAssetFromURL(current_protocol());
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
					list($userid, $user_type) = User::find($_POST['SQ_LOGIN_USERNAME'], $_POST['SQ_LOGIN_PASSWORD']);
					if ($userid) {
						$user = &$this->am->getAsset($userid, $user_type);
						if ($this->setCurrentUser($user)) {
							$_SESSION['userid']      = $userid;
							$_SESSION['remote_addr'] = $_SERVER['REMOTE_ADDR'];
							// generate a new login key so that when they try and login next
							// they have to re-enter the details
							$this->generateLoginKey();

							$_SESSION['user_is_root']      = $this->user_is_root;
							$_SESSION['user_is_sys_admin'] = $this->user_is_sys_admin;
							break;
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
				$_SESSION['userid']				= 0;
				$_SESSION['user_is_root']		= false;
				$_SESSION['user_is_sys_admin']	= false;
				$this->user = null;
			break;

			case 'send_security_key' :
				require_once SQ_FUDGE_PATH.'/general/general.inc';
				$vars = $_SESSION['SQ_SYSTEM_SECURITY_KEY'];
				security_key_image($vars['key'], $vars['width'], $vars['height'], $vars['bg_colour'], $vars['text_colour'], $vars['border_colour'], $vars['zoom']);
				exit();
			break;

		}//end switch

		// just so it isn't being used anywhere else
		$_GET['SQ_ACTION'] = '';

	}//end _processGlobalActions()


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
		$this->user_is_root      = $this->userRoot($user);
		$this->user_is_sys_admin = $this->userSystemAdmin($user);

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
			$this->user_is_root      = false;
			$this->user_is_sys_admin = false;

		} else {
			$this->user = &$user;
			$this->user_is_root      = $this->userRoot($user);
			$this->user_is_sys_admin = $this->userSystemAdmin($user);
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
		if (is_null($user)) return $this->user_is_root;
		if (!$user->id) return false;
		if (get_class($user) == 'root_user' && (int) $user->id == 4) return true;
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
		if (is_null($user)) return $this->user_is_sys_admin;
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
		if(!isset($this->fv) && get_class($this->fv) != 'file_versioning') {
			require_once SQ_FUDGE_PATH.'/file_versioning/file_versioning.inc';
			$this->fv = new File_Versioning(SQ_DATA_PATH.'/file_repository', $this->db);
		}
		return $this->fv;

	}//end getFileVersioning()


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

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// first we delete all entries in the rollback table that have the same from time
		// and that we are going to be replacing because their value is no longer the most recent
		// in this script execution
		$result = $this->db->query($delete);
		if (DB::isError($result)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		// replace the NULL value effective_to entries because we are adding our own now
		$result = $this->db->query($update);
		if (DB::isError($result)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		// and finally insert the new rollback rows
		$result = db_extras_insert_select($this->db, $insert, $select);
		if (DB::isError($result)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
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
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// construct the update query
		$update_sql = 'UPDATE '.SQ_TABLE_PREFIX.$table_name.' SET ';
		foreach ($update_values as $column => $value) {
			$update_sql .= $column.' = '.$value.', ';
		}
		$update_sql = trim($update_sql, ', ');
		if (!empty($where_cond)) $update_sql .= ' WHERE '.$where_cond;

		$result = $this->db->query($update_sql);
		if (DB::isError($result)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		// add the rollback entries for the rows we updated
		if (!$this->addRollbackEntry($table_name, $where_cond)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
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

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$result = $this->db->query($insert_sql);
		if (DB::isError($result)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		// add the rollback entries for the inserted rows
		if (!$this->addRollbackEntry($table_name, $where_cond)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
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
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return 0;
		}

		// generate the column header string for the INSERT and SELECT tables
		$col_string = implode(', ', array_keys($insert_values));
		$select_col_string = implode(', ', array_values($insert_values));

		// construct the queries
		$insert_sql = 'INSERT INTO '.SQ_TABLE_PREFIX.$table_name.' ('.$col_string.')';
		$select_sql = 'SELECT '.$select_col_string. ' FROM '.$select_from.' WHERE '.$where_cond;

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$result = db_extras_insert_select($this->db, $insert_sql, $select_sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// add the rollback entries for the inserted rows
		if (!$this->addRollbackEntry($table_name, $where_clause)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}


		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
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
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// add the rollback entries for the rows we are going to delete
		if (!$this->addRollbackEntry($table_name, $where_cond)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// construct the delete query
		$delete_sql = 'DELETE FROM '.SQ_TABLE_PREFIX.$table_name.' WHERE '.$where_cond;

		$result = $this->db->query($delete_sql);
		if (DB::isError($result)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end rollbackDelete()


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
		$f = 'd/m/Y H:i:s';
		return (is_null($timestamp)) ? date($f) : date($f, $timestamp);

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
		$f = 'd/m/Y';
		return (is_null($timestamp)) ? date($f) : date($f, $timestamp);

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
		$f = 'H:i:s';
		return (is_null($timestamp)) ? date($f) : date($f, $timestamp);

	}//end time()

}//end class

?>