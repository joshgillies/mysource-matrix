<?php

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* User
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class User extends Asset
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function User($assetid=0)
	{
		$this->Asset($assetid);
	}


	/**
	* Create this asset
	*
	* @param array()	&$link	information used to create the initial link
	*
	* @see Asset::create()
	* @return mixed int or false
	* @access public
	*/
	function create(&$link)
	{
		$username = trim($this->attr('username'));
		$password = $this->attr('password');

		if ($username == '' || $password = '') {
			trigger_error('Unable to create a '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name').' without a username and password', E_USER_WARNING);
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$linkid = parent::create($link);

		if (!$linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		$user_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'dependant' => 1, 'exclusive' => 1);

		// create an inbox
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('inbox');
		$inbox = new Inbox();
		if (!$inbox->create($user_link)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		// create a workspace
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('workspace');
		$workspace = new Workspace();
		$workspace->setAttrValue('name', 'Workspace');
		if (!$workspace->create($user_link)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		if (!$this->setPermission($this->id, SQ_PERMISSION_ADMIN, '1')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}


		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end create()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(SQ_LINK_TYPE_1 => Array(),
					 SQ_LINK_TYPE_2 => Array('inbox'     => Array('card' => 1, 'exclusive' => true),
											 'workspace' => Array('card' => 1, 'exclusive' => true)),
					 SQ_LINK_TYPE_3 => Array(),
					 SQ_LINK_NOTICE => Array()
					);
	}//end _getAllowedLinks()


	/**
	* Set the value for the passed variable to the passed value
	* Overwritten here to check inputs (username and password) and to
	* encrypt password before setting value
	*
	* @param string		$name				the name of the attribute
	* @param string		$value				the new value of the attribute
	* @param boolean	$dont_run_updated	if true this->_updated() won't be run 
	*										(THIS IS ONLY EVER CALLED FROM Asset::create())
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value, $dont_run_updated=false)
	{
		if ($this->id) {
			// if this is the password, encrypt it
			// BUT not if we have been created,
			// otherwise when we try and save to DB in create() it encrypts the encrypted version
			if ($name == 'password') {
				$value = crypt($value); // let salt be generated
			}

			if ($this->vars[$name]['value'] == $value) return true;
			
			if ($name == 'username') {
				$db = &$GLOBALS['SQ_SYSTEM']->db;
				$sql = 'SELECT COUNT(*)
						FROM sq_asset_attribute_value v
						  INNER JOIN sq_asset_attribute a ON v.attributeid = a.attributeid
						WHERE a.name = '.$db->quote('username').'
						  AND custom_value='.$db->quote($value);
				$result = $db->getOne($sql);
				if (DB::isError($result)) {
					trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
				}

				if ($result > 0) {
					trigger_error('Failed creating user, username "'.$value.'" already exists', E_USER_WARNING);
					return false;
				}
			}
		}

		return parent::setAttrValue($name, $value, $dont_run_updated);

	}//end setAttrValue()


	/**
	* Returns whether the passed password matches this users' password
	*
	* @access public
	*/
	function comparePassword($entered_password)
	{

		$password = $this->attr('password');

		if (!trim($password)) {
			return false;
		}

		return (crypt($entered_password,$password) == $password);

	}//end comparePassword()


	/**
	* Returns name of the user
	*
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @see Asset::_getName()
	* @access public
	*/
	function _getName($short_name=false)
	{
		return ($this->attr('first_name') || $this->attr('last_name')) ? $this->attr('first_name').' '.$this->attr('last_name') : $this->attr('username');
	}//end _getName()


	/**
	* Attempts to find a user that matches the passed username and login, returning it's assetid
	*
	* @param string	$username
	* @param string	$password
	*
	* @return Array(int,string)
	* @access public
	*/
	function find($username, $password, $type_code='user')
	{
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT a.assetid, a.type_code
				FROM sq_asset a, sq_asset_attribute at, sq_asset_attribute_value av
				WHERE a.type_code IN (~SQ0~) 
				  AND at.name         = '.$db->quote('username').'
				  AND at.attributeid  = av.attributeid
				  AND av.assetid      = a.assetid
				  and av.custom_value = '.$db->quote($username);

		$subs = Array( 'SELECT type_code 
						FROM sq_asset_type_inherited 
						WHERE inherited_type_code = '.$db->quote($type_code)
						);

		$sql = db_extras_subquery($db, $sql, $subs);

		$result = $db->getRow($sql);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		if (!$result['assetid']) return array(0,'');

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($result['assetid']);
		if (is_null($asset) || !$asset->comparePassword($password)) {
			return array(0,'');
		}

		return array($result['assetid'], $result['type_code']);

	}//end find()


	/**
	* Returns true if at least one of the passed assetids has the passed permission
	* Does not check current status of lock
	*
	* @param int	$perm		the permission to check access for
	* @param array	$assetids	An array of user_group and/or user ids to check for permission
	*							If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	function _checkPermissionAccess($perm, $assetids=Array())
	{
		if (empty($assetids)) {
			// root can do anything
			if ($GLOBALS['SQ_SYSTEM']->userRoot()) return true;

			if ($GLOBALS['SQ_SYSTEM']->userRoot($this)) {
				// we have to be root to view root's details
				return false;
			} else if ($GLOBALS['SQ_SYSTEM']->userSystemAdmin($this)) {
				// this is a sysadmin so they need to
				// be root or the sysadmin themselves
				if (!$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) return false;
				if ($this->id == $GLOBALS['SQ_SYSTEM']->curretUserId()) return true;
				return false;
			} else {
				// any other user's details
				if ($GLOBALS['SQ_SYSTEM']->userSystemAdmin()) return true;
				if ($this->id == $GLOBALS['SQ_SYSTEM']->currentUserId()) return true;
			}

			return false;
		} else {
			return parent::_checkPermissionAccess($perm, $assetids);
		}
	}//end checkAccess()


	/**
	* Returns true if this user can log in
	* Note that this is not for backend logins only, it applys to all logins
	*
	* @return boolean
	* @access public
	*/
	function canLogin() {
		// check that this user is live
		if ($this->status & SQ_SC_STATUS_NOT_LIVE) return false;

		// check that this user has not expired
		$expires = $this->attr('expiry_date');
		if ($expires != '---------- --:--:--') {
			$now = date('Y-m-d h:i:s');
			if ($expires <= $now) return false;
		}

		// check that this user is not only in the trash
		if ($GLOBALS['SQ_SYSTEM']->am->assetInTrash($this->id, true)) return false;

		return true;
	}


	/**
	* Called by the design to print the body of this asset
	*
	* @access public
	*/
	function printBody()
	{
		// print navigation
		$url = $_SERVER['PHP_SELF'].'?user_screen=';
		?>
		<a href="<?php echo $url?>view">View your details</a></br>
		<a href="<?php echo $url?>edit">Edit your details</a></br>
		<a href="<?php echo $url?>workspace">View your workspace</a></br>
		<a href="<?php echo $url?>messages">Check your messages</a></br>
		<?php

		// print the contents of the screen
		$user_screen = (isset($_REQUEST['user_screen'])) ? $_REQUEST['user_screen'] : '';
		switch ($_REQUEST['user_screen']) {
			case 'edit'      :
				bam('EDIT DETAILS');
				break;
			case 'workspace' :
				// get the workspace attached to us
				$workspace_data = $this->getLink(SQ_LINK_TYPE_2, 'workspace', true, null, 'major', '1');
				$workspace = &$GLOBALS['SQ_SYSTEM']->am->getAsset($workspace_data['minorid'], $workspace_data['type_code']);
				$workspace->printBody();
				break;
			case 'messages'  :
				// get the inbox attached to us
				$inbox_data = $this->getLink(SQ_LINK_TYPE_2, 'inbox', true, null, 'major', '1');
				$inbox = &$GLOBALS['SQ_SYSTEM']->am->getAsset($inbox_data['minorid'], $inbox_data['type_code']);
				$inbox->printBody();
				break;
			default :
				bam('VIEW DETAILS');
				break;
		}

		
	}// end printBody()

}//end class
?>
