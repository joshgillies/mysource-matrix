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
	* @param int    $assetid      the asset id to be loaded
	*
	*/
	function User($assetid=0)
	{
		$this->Asset($assetid);
	}


	/**
	* Create this asset
	*
	* @param array(string)	&$link		information used to create the initial link
	* @param string			$username	the unique username
	* @param string			$password
	* @param string			$first_name
	* @param string			$last_name
	* @param string			$email
	*
	* @return boolean
	* @access public
	*/
	function create(&$link, $username, $password, $first_name='', $last_name='', $email='')
	{

		if (parent::create($link)) {

			$this->setAttrValue('username',   $username);
			$this->setAttrValue('password',   $password);
			$this->setAttrValue('first_name', $first_name);
			$this->setAttrValue('last_name',  $last_name);
			$this->setAttrValue('email',      $email);

			$user_link = Array('asset' => &$this, 'link_type' => SQ_LINK_EXCLUSIVE);

			// create an inbox
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('inbox');
			$inbox = new Inbox();
			if (!$inbox->create($user_link)) return false;

			// create a workspace
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('workspace');
			$workspace = new Workspace();
			if (!$workspace->create($user_link, 'Workspace')) return false;

			return true;

		}//end if

		return false;

	}//end create()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(SQ_LINK_EXCLUSIVE => Array('inbox' => '1', 'workspace' => '1'),
					 SQ_LINK_UNITE     => Array(),
					 SQ_LINK_USES      => Array(),
					 SQ_LINK_NOTICE    => Array()
					);
	}//end _getAllowedLinks()


	/**
	* Returns actual link info for all links that this asset (but not any of it's kids) deems as dependant
	*
	* @return Array()
	* @access private
	*/
	function _getDependantLinks()
	{
		// we are dependant on our inbox and our workspace
		return array_merge(parent::_getDependantLinks(), $this->getLinks(SQ_LINK_EXCLUSIVE, Array('inbox', 'workspace')));
	}// end _getDependantLinks()


	/**
	* Set the value for the passed variable to the passed value
	* Overwritten here to check inputs (username and password) and to
	* encrypt password before setting value
	*
	* @access public
	*/
	function setAttrValue($name, $value)
	{

		trigger_error('CHECK USER INPUTS FOR USERNAME, PASSWORD ETC', E_USER_NOTICE);

		if ($name == 'password') {
			$value = crypt($value); // let salt be generated
		}

		return parent::setAttrValue($name, $value);

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

		$subs = Array( 'SELECT type_code FROM sq_asset_type_inherited 
						WHERE inherited_type_code = '.$db->quote($type_code).'
						   OR type_code = '.$db->quote($type_code));

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
	*
	* @param array $assetids An array of user_group and/or user ids to check for permission
	*						 If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	function checkAccess($perm, $assetids=Array())
	{
		// root can do anything
		if ($GLOBALS['SQ_SYSTEM']->userRoot()) return true;

		if ($GLOBALS['SQ_SYSTEM']->userRoot($this)) {
			// we have to be root to view root's details
			return false;
		} else if ($GLOBALS['SQ_SYSTEM']->userSystemAdmin($this)) {
			// this is a sysadmin so they need to
			// be root or the sysadmin themselves
			if (!$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) return false;
			if ($this->id == $GLOBALS['SQ_SYSTEM']->user->id) return true;
			return false;
		} else {
			// any other user's details
			if ($GLOBALS['SQ_SYSTEM']->userSystemAdmin()) return true;
			if ($this->id == $GLOBALS['SQ_SYSTEM']->user->id) return true;
		}

		return false;
	}//end checkAccess()


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
				$workspace_data = $this->getLink(SQ_LINK_EXCLUSIVE, 'workspace');
				$workspace = &$GLOBALS['SQ_SYSTEM']->am->getAsset($workspace_data['minorid'], $workspace_data['type_code']);
				$workspace->printBody();
				break;
			case 'messages'  :
				// get the inbox attached to us
				$inbox_data = $this->getLink(SQ_LINK_EXCLUSIVE, 'inbox');
				$inbox = &$GLOBALS['SQ_SYSTEM']->am->getAsset($inbox_data['minorid'], $inbox_data['type_code']);
				$inbox->printBody();
				break;
			default :
				bam('VIEW DETAILS');
				break;
		}

		
	}// end printBody()


	/**
	* Processes a backend submission from this asset, returns true if all OK
	*
	* @param array(string)	&$link	information used to create the initial link
	*
	* @return boolean
	* @access public
	*/
	function processBackend($link)
	{
		switch($_POST['asset_action']) {
			case 'create' :
				$ei = &$this->getEI();
				$ei->process($this, true);

				$username = $this->getAttribute('username');
				$password = $this->getAttribute('password');
				if ($username->processed && $password->processed) {
					if (!$this->create($link, $username->value, $password->value)) return false;
				
					// users get admin permission of themselves so they
					// can edit their details, manage their workspace, check their
					// internal messages etc etc
					return $this->grantPermission($this->id, SQ_PERMISSION_ADMIN);
				} else {
					return false;
				}

				break;

			default :
				return parent::processBackend($link);

		}// end switch

	}// end processBackend()


}//end class
?>
