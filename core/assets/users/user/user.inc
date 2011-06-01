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
* $Id: user.inc,v 1.128.4.1 2009/04/08 23:38:50 bpearson Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_INCLUDE_PATH.'/password_rules_config.inc';

/**
* User
*
* Purpose
*     Processing interfaces for the (standard) user asset - one that can log in
*     but has no backend editing privileges. This also defines the baseline
*     interface for all user assets in the system (whether they be standard,
*     backend, system admin, etc).
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.128.4.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class User extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	**/
	function User($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		$this->Asset($assetid);

	}//end constructor


	/**
	* Perform any pre-validation required during the creation of this asset
	*
	* Users ensure a username and password has been set before they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _preCreateCheck(Array &$link)
	{
		if (!parent::_preCreateCheck($link)) return FALSE;

		$username = trim($this->attr('username'));
		$password = $this->attr('password');

		if (!$this->_catchBlankUserNamePass($username, $password)) {
			if (!array_get_index($this->_tmp, 'username_set') || !array_get_index($this->_tmp, 'password_set')) {
				// in this case they have tried to create without even trying to set the username or password, so
				// we error here.  Otherwise errors have already been thrown so we don't throw them now.
				trigger_localised_error('CORE0084', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
			}
			return FALSE;
		}

		return TRUE;

	}//end _preCreateCheck()


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Standard pages create a bodycopy asset when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _createAdditional(Array &$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		if (!$this->makeAndSaveInitialWebPath(strtolower($this->attr('username')), $link)) {
			trigger_localised_error('CORE0019', E_USER_WARNING, $this->name);
			return FALSE;
		}

		// Ensure that the current user can perform the following permissions change regardless of their own permissions on the parent
		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

		// give the user admin access to him/herself
		if (!$GLOBALS['SQ_SYSTEM']->am->setPermission($this->id, $this->id, SQ_PERMISSION_ADMIN, TRUE)) {
			trigger_localised_error('CORE0015', E_USER_WARNING, $this->name);
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

		$user_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

		// create an inbox
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('inbox');
		$inbox = new Inbox();
		if (!$inbox->create($user_link)) return FALSE;

		// create a workspace
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('workspace');
		$workspace = new Workspace();
		$workspace->setAttrValue('name', 'Workspace');
		if (!$workspace->create($user_link)) return FALSE;

		return TRUE;

	}//end _createAdditional()


	/**
	* Reload this asset from the DB, clearing all its local caches
	*
	* @return void
	* @access public
	*/
	function reload()
	{
		parent::reload();
		unset($_SESSION['sq_user_groups'][$this->id]);

	}//end reload()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
	{
		$allowed = parent::_getAllowedLinks();
		// Allow any type of asset to be linked underneath
		$allowed[SQ_LINK_TYPE_1]['asset']     = Array('card' => 'M', 'exclusive' => FALSE);
		$allowed[SQ_LINK_TYPE_2]['asset']     = Array('card' => 'M', 'exclusive' => FALSE);
		$allowed[SQ_LINK_TYPE_3]['asset']     = Array('card' => 'M', 'exclusive' => FALSE);
		$allowed[SQ_LINK_NOTICE]['asset']     = Array('card' => 'M', 'exclusive' => FALSE);

		// Restrict the inbox and workspace to exclusive links
		$allowed[SQ_LINK_TYPE_2]['inbox']     = Array('card' => 1, 'exclusive' => TRUE);
		$allowed[SQ_LINK_TYPE_2]['workspace'] = Array('card' => 1, 'exclusive' => TRUE);
		return $allowed;

	}//end _getAllowedLinks()


	/**
	* Set the value for the passed variable to the passed value
	*
	* Overwritten here to check inputs (username and password) and to
	* encrypt password before setting value
	*
	* @param string	$name	the name of the attribute
	* @param string	$value	the new value of the attribute
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value)
	{
		// if this is the password, encrypt it
		if ($name == 'password') {
			$this->_tmp['password_set'] = TRUE; // so we know at create time whether to throw an extra error
			if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PASSWORD_VALIDATION)) {
				if (!$this->validatePassword($value)) return FALSE;
			}
			$value = crypt($value); // let salt be generated
		} else if ($name == 'username') {
			$this->_tmp['username_set'] = TRUE; // so we know at create time whether to throw an extra error
			if (!$this->_catchBlankSetUserName(trim($value))) {
				trigger_localised_error('CORE0110', E_USER_WARNING);
				return FALSE;
			}
		}

		if (!parent::setAttrValue($name, $value)) return FALSE;

		// tell the workspace (or anyone else listening) that the username has been updated
		if ($name == 'username') {
			$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
			$em->broadcastEvent($this, 'usernameUpdated', Array('username' => $value));
		}
		return TRUE;

	}//end setAttrValue()


	/**
	* Returns whether the passed password matches this users' password
	*
	* @param string	$entered_password	the password sent to us
	*
	* @return boolean
	* @access public
	*/
	function comparePassword($entered_password)
	{
		$password = $this->attr('password');

		if (!trim($password)) return FALSE;

		return (crypt($entered_password,$password) == $password);

	}//end comparePassword()


	/**
	* Validates the passed password against a list of rules
	*
	* @param string		$passwd				the password to validate
	* @param boolean	$add_system_message	If true, adds a system message instead of triggering errors
	*
	* @return boolean
	* @access public
	*/
	function validatePassword($passwd, $add_system_message=FALSE)
	{
		$config = new Password_Rules_Config();
		require_once $config->config_file;

		// one rule that is always in effect:
		if ($passwd === '') {
			if ($add_system_message) {
				$GLOBALS['SQ_SYSTEM']->addMessage($$GLOBALS['SQ_SYSTEM']->getErrorMessage('CORE0247'));
			} else {
				trigger_localised_error('CORE0247', E_USER_NOTICE);
			}
			$this->_tmp['password_validation_error'] = TRUE;
			return FALSE;
		}

		$errors = Array();

		// errors for when a character type does not occur in the password
		$not_occurring_error_map = Array(
									'SQ_PASSWD_RULE_NUM_CAPS'	=> 'CORE0222',
									'SQ_PASSWD_RULE_NUM_LOWER'	=> 'CORE0223',
									'SQ_PASSWD_RULE_NUM_INT'	=> 'CORE0224',
									'SQ_PASSWD_RULE_NUM_PUNC'	=> 'CORE0225',
									'SQ_PASSWD_RULE_NUM_SPC'	=> 'CORE0226',
								   );

		// errors for when a character type incorrectly occurs in the password
		$occurring_error_map = Array(
								'SQ_PASSWD_RULE_NUM_CAPS'	=> 'CORE0227',
								'SQ_PASSWD_RULE_NUM_LOWER'	=> 'CORE0228',
								'SQ_PASSWD_RULE_NUM_INT'	=> 'CORE0229',
								'SQ_PASSWD_RULE_NUM_PUNC'	=> 'CORE0230',
								'SQ_PASSWD_RULE_NUM_SPC'	=> 'CORE0231',
							   );

		if (defined('SQ_PASSWD_RULE_LENGTH') && SQ_PASSWD_RULE_LENGTH > -1) {
			if (strlen($passwd) < SQ_PASSWD_RULE_LENGTH) {
				$errors[] = Array('CORE0220', strlen($passwd), SQ_PASSWD_RULE_LENGTH);
			}
		}

		if (defined('SQ_PASSWD_RULE_DISALLOW_USER_INFO') && SQ_PASSWD_RULE_DISALLOW_USER_INFO == 0) {
			$components = Array('username', 'first_name', 'last_name', 'email');
			foreach ($components as $component) {
				$attr = $this->attr($component);
				if (!empty($attr) && preg_match('/.*'.$this->attr($component).'.*/i', $passwd)) {
					$errors[] = Array('CORE0221');
					break;
				}
			}
		}

		$charlist = count_chars($passwd, 1);
		$num_rules = get_constant_values('SQ_PASSWD_RULE_NUM_');

		foreach ($num_rules as $rule => $value) {
			eval('$'.$rule.'_counter = 0;');
			if ($value != SQ_PASSWD_RULE_DISABLED) {
				foreach ($charlist as $char => $occurances) {
					switch ($rule) {
						case 'SQ_PASSWD_RULE_NUM_CAPS':
							if (ctype_upper($char)) {
								eval('$'.$rule.'_counter += $occurances;');
							}
						break;

						case 'SQ_PASSWD_RULE_NUM_LOWER':
							if (ctype_lower($char)) {
								eval('$'.$rule.'_counter += $occurances;');
							}
						break;

						case 'SQ_PASSWD_RULE_NUM_INT':
							if (ctype_digit($char)) {
								eval('$'.$rule.'_counter += $occurances;');
							}
						break;

						case 'SQ_PASSWD_RULE_NUM_PUNC':
							if (ctype_punct($char)) {
								eval('$'.$rule.'_counter += $occurances;');
						}
						break;

						case 'SQ_PASSWD_RULE_NUM_SPC':
							if (ctype_space($char)) {
								eval('$'.$rule.'_counter += $occurances;');
							}
						break;
					}//end switch on rule
				}//end foreach character
			}//end if password rule enabled
		}//end foreach rule

		foreach ($num_rules as $rule => $value) {
			if ($value != SQ_PASSWD_RULE_DISABLED) {
				eval('$tmp_counter = $'.$rule.'_counter;');
				if ($value == SQ_PASSWD_RULE_DISALLOWED && $tmp_counter > 0) {
					if (isset($occurring_error_map[$rule])) {
						$errors[] = Array($occurring_error_map[$rule]);
					}
				} else if ($tmp_counter < $value) {
					if (isset($not_occurring_error_map[$rule])) {
						$errors[] = Array($not_occurring_error_map[$rule], $tmp_counter, $value);
					}
				}
			}
		}

		// check if this password contains blacklisted word
		$blacklist = SQ_PASSWD_RULE_BLACK_LIST;
		if (!empty($blacklist)) {
			$blacklist = str_replace("\r\n","\n", $blacklist);
			$blacklist = str_replace("\r","\n", $blacklist);
			$blacklist = split("\n", $blacklist);
			foreach ($blacklist as $word) {
				if ((!SQ_PASSWD_RULE_BLACK_LIST_EXACT && preg_match('/'.$word.'/', $passwd)) || (SQ_PASSWD_RULE_BLACK_LIST_EXACT && (strcmp($passwd, $word) == 0))) {
					$errors[] = Array('CORE0260');
				}
			}
		}

		if ($add_system_message) {
			$msgs = '';
			foreach ($errors as $error) {
				$code = array_shift($error);
				$bit = $GLOBALS['SQ_SYSTEM']->lm->getErrorMessage($code, $error);
				$bit = preg_replace('/\[([^\]]*)\]$/', '', $bit); // remove error code
				$msgs .= '<li>'.$bit.'</li>';
			}
			if (!empty($errors)) {
				$GLOBALS['SQ_SYSTEM']->addMessage(translate('password_validation_error', '<ul>'.$msgs.'</ul>'));
			}
		} else {
			foreach ($errors as $error) {
				$code = array_shift($error);
				$GLOBALS['SQ_SYSTEM']->lm->raiseError($code, E_USER_WARNING, $error);
			}
		}

		return empty($errors);

	}//end validatePassword()


	/**
	* Get a description of the password rules currently in effect
	*
	* @return string
	* @access public
	*/
	function getPasswordRulesDescription()
	{
		include_once SQ_DATA_PATH.'/private/conf/password_rules.inc';
		$res = Array();
		if (SQ_PASSWD_RULE_LENGTH > 0) {
			$res[] = translate('password_rules_min_length', SQ_PASSWD_RULE_LENGTH);
		}
		if (SQ_PASSWD_RULE_DISALLOW_USER_INFO == 0) {
			// yes this was built the wrong way around
			$res[] = translate('password_rules_user_info_prohibited');
		}

		if (SQ_PASSWD_RULE_NUM_CAPS > 1) {
			$res[] = translate('password_rules_at_least', SQ_PASSWD_RULE_NUM_CAPS, translate('capital_letters'));
		}
		if (SQ_PASSWD_RULE_NUM_LOWER > 1) {
			$res[] = translate('password_rules_at_least', SQ_PASSWD_RULE_NUM_LOWER, translate('lowercase_letters'));
		}
		if (SQ_PASSWD_RULE_NUM_INT > 1) {
			$res[] = translate('password_rules_at_least', SQ_PASSWD_RULE_NUM_INT, translate('numeric_digits'));
		}
		if (SQ_PASSWD_RULE_NUM_PUNC > 1) {
			$res[] = translate('password_rules_at_least', SQ_PASSWD_RULE_NUM_PUNC, translate('punctuation_characters'));
		}
		if (SQ_PASSWD_RULE_NUM_SPC > 1) {
			$res[] = translate('password_rules_at_least', SQ_PASSWD_RULE_NUM_SPC, translate('spaces'));
		}

		if (SQ_PASSWD_RULE_NUM_CAPS == 1) {
			$res[] = translate('password_rules_at_least', translate('one'), translate('capital_letter'));
		}
		if (SQ_PASSWD_RULE_NUM_LOWER == 1) {
			$res[] = translate('password_rules_at_least', translate('one'), translate('lowercase_letter'));
		}
		if (SQ_PASSWD_RULE_NUM_INT == 1) {
			$res[] = translate('password_rules_at_least', translate('one'), translate('numeric_digit'));
		}
		if (SQ_PASSWD_RULE_NUM_PUNC == 1) {
			$res[] = translate('password_rules_at_least', translate('one'), translate('punctuation_character'));
		}
		if (SQ_PASSWD_RULE_NUM_SPC == 1) {
			$res[] = translate('password_rules_at_least', translate('one'), translate('space'));
		}

		$disallowed_chars = Array();
		if (SQ_PASSWD_RULE_NUM_CAPS == -1) {
			$disallowed_chars[] = translate('capital_letters');
		}
		if (SQ_PASSWD_RULE_NUM_LOWER == -1) {
			$disallowed_chars[] = translate('lowercase_letters');
		}
		if (SQ_PASSWD_RULE_NUM_INT == -1) {
			$disallowed_chars[] = translate('numeric_digits');
		}
		if (SQ_PASSWD_RULE_NUM_PUNC == -1) {
			$disallowed_chars[] = translate('punctuation_characters');
		}
		if (SQ_PASSWD_RULE_NUM_SPC == -1) {
			$disallowed_chars[] = translate('spaces');
		}
		if (!empty($disallowed_chars)) {
			include_once SQ_FUDGE_PATH.'/general/text.inc';
			$res[] = ucfirst(translate('password_rules_not_allowed', make_readable_list($disallowed_chars, strtolower(translate('and')))));
		}

		if (trim(SQ_PASSWD_RULE_BLACK_LIST) != '') {
			$black_words = Array();
			foreach (explode("\n", SQ_PASSWD_RULE_BLACK_LIST) as $word) {
				$black_words[] = '"'.trim($word).'"';
			}
			if (!empty($black_words)) {
				if (SQ_PASSWD_RULE_BLACK_LIST_EXACT) {
					$res[] = translate('password_rules_blacklist_strict', implode(', ', $black_words));
				} else {
					$res[] = translate('password_rules_blacklist_loose', implode(', ', $black_words));
				}
			}
		}

		if (!empty($res)) {
			return translate('password_rules_intro').'<ul class="password-rules"><li>'.implode('</li><li>', $res).'</li></ul>';
		}
		return '';

	}//end getPasswordRulesDescription()


	/**
	* Returns name of the user
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	public function _getName($short_name=FALSE)
	{
		return ($this->attr('first_name') || $this->attr('last_name')) ? $this->attr('first_name').' '.$this->attr('last_name') : $this->attr('username');

	}//end _getName()


	/**
	* Returns TRUE if at least one of the passed assetids has the passed permission
	* Does not check current status of lock
	*
	* @param int		$perm			the permission to check access for
	* @param array		$assetids		An array of user_group and/or user ids to check for permission
	*									If the array is empty, the current user will be checked
	* @param boolean	$only_workflow	check only workflow
	*
	* @return boolean
	* @access public
	*/
	public function _checkPermissionAccess($perm, $assetids=Array(), $only_workflow=TRUE)
	{
		if (empty($assetids)) {
			// root can do anything
			if ($GLOBALS['SQ_SYSTEM']->userRoot()) return TRUE;

			if ($GLOBALS['SQ_SYSTEM']->userRoot($this)) {
				// we have to be root to view root's details
				return FALSE;
			} else if ($GLOBALS['SQ_SYSTEM']->userSystemAdmin($this)) {
				// this is a sysadmin so they need to
				// be root or the sysadmin themselves
				if (!$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
					return FALSE;
				}
				if ($this->id == $GLOBALS['SQ_SYSTEM']->currentUserId()) {
					return TRUE;
				}
				return FALSE;
			} else {
				// any other user's details
				if ($GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
					return TRUE;
				}
				if ($this->id == $GLOBALS['SQ_SYSTEM']->currentUserId()) {
					return TRUE;
				}
			}

		} else {
			// a user always has full access to themselves
			if (in_array($this->id, $assetids)) return TRUE;

		}

		return parent::_checkPermissionAccess($perm, $assetids);

	}//end _checkPermissionAccess()


	/**
	* Returns TRUE if we can delete the passed link, or a string with the error msg
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return mixed boolean|string
	* @access public
	*/
	function isDeletableLink($linkid)
	{
		if ($GLOBALS['SQ_PURGING_TRASH']) return TRUE;

		// we dont want our inbox or workspace deleted
		$inbox_link = $this->getInboxLink();
		if (!empty($inbox_link) && ($inbox_link['linkid'] == $linkid)) {
			return translate('cannot_delete_inbox-user_link');
		}

		$workspace_link = $this->getWorkspaceLink();
		if (!empty($workspace_link) && ($workspace_link['linkid'] == $linkid)) {
			return translate('cannot_delete_workspace-user_link');
		}

		return parent::isDeletableLink($linkid);

	}//end isDeletableLink()


	/**
	* Check if this user can log in
	* Note that this is not for backend logins only, it applys to all logins
	*
	* @return boolean
	* @access public
	*/
	function canLogin()
	{
		// check that this user is live
		if ($this->status & SQ_SC_STATUS_NOT_LIVE) {
			return FALSE;
		}

		// check that this user is not only in the trash
		if ($GLOBALS['SQ_SYSTEM']->am->assetInTrash($this->id, TRUE)) {
			return FALSE;
		}

		return TRUE;

	}//end canLogin()


	/**
	* Check if this user can access the backend interface
	*
	* This will be disabled by default for a standard user, and enabled for
	* users who are allowed backend access (eg. backend user).
	*
	* @return boolean
	* @access public
	*/
	function canAccessBackend()
	{
		return FALSE;

	}//end canAccessBackend()


	/**
	* Can we set this as the current user?
	*
	* @return boolean
	* @access public
	*/
	function canSetAsCurrentUser()
	{
		return $this->canLogin();

	}//end canSetAsCurrentUser()


	/**
	* Get all the groups that this user is in, regardless of group security restrictions
	*
	* Returns a list of assetids of the matching groups
	* The result is stored in the session for faster access
	*
	* @return array
	* @access public
	*/
	function getGroups()
	{
		if (!isset($_SESSION['sq_user_groups'][$this->id])) {
			// we store userid as key in this array to make sure we have the right user,
			// but to avoid invalidity we clear any other userids out
			$fetch_roles = ((SQ_CONF_ENABLE_ROLES_SYSTEM == '1') && (SQ_CONF_ENABLE_GLOBAL_ROLES == '1'));
			$group_types = Array('user', 'user_group', 'system_user_group');
			if ($fetch_roles) $group_types[] = 'role';
			$_SESSION['sq_user_groups'] = Array($this->id => $GLOBALS['SQ_SYSTEM']->am->getParents($this->id, $group_types, TRUE));
		}
		return $_SESSION['sq_user_groups'][$this->id];

	}//end getGroups()


	/**
	* Gets all the groups that this user is in and for which this user satisfies the security restrictions
	*
	* Returns a list of assetids of the matching groups.
	*
	* @return array
	* @access public
	*/
	function getUserGroups()
	{
		// cache the result so that we dont do this more than once in an execution
		if (!isset($_SESSION['sq_effective_user_groups'][$this->id])) {
			$_SESSION['sq_effective_user_groups'][$this->id] = Array();
			$fetch_roles = ((SQ_CONF_ENABLE_ROLES_SYSTEM == '1') && (SQ_CONF_ENABLE_GLOBAL_ROLES == '1'));
			$groupids = $GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'user_group', FALSE);
			foreach ($groupids as $groupid => $type_code) {
				// if we pass the conditions, let us belong to this group
				// EXCEPT if this is a role, and roles are disabled
				if (!(($type_code == 'role') && !$fetch_roles)) {
					if ($this->_belongsToGroup($groupid)) {
						$_SESSION['sq_effective_user_groups'][$this->id][] = $groupid;
					}
				}
			}
		}
		return $_SESSION['sq_effective_user_groups'][$this->id];

	}//end getUserGroups()


	/**
	* Verify if a user belongs to a given group, return TRUE if we are
	*
	* Here is where we call the user conditions and certify the result.
	* When a new user condition is added, please add the specifics for calling it here.
	*
	* @param string	$group_assetid	assetid of the group in question
	*
	* @return boolean
	* @access private
	*/
	function _belongsToGroup($group_assetid)
	{
		$restrictions = $this->attr('restrictions');

		// if this group isnt in the restrictions array
		// then we just belong to it and so return
		if (!in_array($group_assetid, array_keys($restrictions))) {
			return TRUE;
		}

		// for each restriction associated to this group
		foreach ($restrictions[$group_assetid] as $restriction) {

			// grab the condition name, make an object
			$condition_type_code = $restriction['name'];
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($condition_type_code);
			$condition = new $condition_type_code();

			$match = $condition->evaluate($this, $restriction['condition_data']);

			// they don't match up, so not in this group
			if ($match != $restriction['match']) return FALSE;

		}

		// if we got this far, we've passed all restrictions
		return TRUE;

	}//end _belongsToGroup()


	/**
	* Returns the link to the inbox
	*
	* @return array
	* @access public
	*/
	function getInboxLink()
	{
		return $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'inbox');

	}//end getInboxLink()


	/**
	* Returns the link to the workspace
	*
	* @return array
	* @access public
	*/
	function getWorkspaceLink()
	{
		return $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'workspace');

	}//end getWorkspaceLink()


	/**
	* Returns this user's Workspace
	*
	* @return object
	* @access public
	*/
	function &getWorkspace()
	{
		$null = NULL;
		$link = $this->getWorkspaceLink();

		if (empty($link)) {
			trigger_localised_error('CORE0109', E_USER_WARNING,$this->name);
			return $null;
		}

		$workspace = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid']);
		if (is_null($workspace)) {
			trigger_localised_error('CORE0109', E_USER_WARNING,$this->name);
			return $null;
		}
		return $workspace;

	}//end getWorkspace()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		?>
		<table border="0" cellspacing="2" cellpadding="2">
			<tr>
				<td><b>Username</b></td>
				<td><?php echo $this->attr('username'); ?></td>
			</tr>
			<tr>
				<td><b>First Name</b></td>
				<td><?php echo $this->attr('first_name'); ?></td>
			</tr>
			<tr>
				<td><b>Last Name</b></td>
				<td><?php echo $this->attr('last_name'); ?></td>
			</tr>
			<tr>
				<td><b>Email</b></td>
				<td><?php echo $this->attr('email'); ?></td>
			</tr>
		</table>
		<?php

	}//end printBody()


	/**
	* Catches blank username/password combination made during create()
	*
	* Override this and return TRUE if a specific subclass wants to skip this check
	*
	* @param string	$username	the passed user name
	* @param string	$password	the passed pass
	*
	* @return boolean
	* @access public
	*/
	protected function _catchBlankUserNamePass($username, $password)
	{
		return (($username != '') && ($password != ''));

	}//end _catchBlankUserNamePass()


	/**
	* Catches blank username being set during setAttrValue()
	*
	* Override this and return TRUE if a specific subclass wants to skip this check
	*
	* @param string	$username	the passed user name
	*
	* @return boolean
	* @access public
	*/
	protected function _catchBlankSetUserName($username)
	{
		if ($username === '') {
			$this->_tmp['blank_username_caught'] = TRUE;
			return FALSE;
		}
		return TRUE;

	}//end _catchBlankSetUserName()


	/**
	* Returns an array of the current locks that this user holds
	*
	* @return array
	* @access public
	*/
	function getUserLocks()
	{
		$db = MatrixDAL::getDb();
		try {
			$bind_vars	= Array (
							'userid'	=> $this->id,
						  );

			$result = MatrixDAL::executeAll('user', 'getUserCurrentLocks', $bind_vars);
		} catch (Exception $o) {
			throw new Exception('Unable to get all locks for current user (userid #'.$this->id.') due to database error: '.$e->getMessage());
		}//end try catch

		return $result;

	}//end getUserLocks()


	/**
	* Releases all locks that this user currently holds in the system
	*
	* @return void
	* @access public
	*/
	function releaseUserLocks()
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$current_locks = $this->getUserLocks();
		foreach ($current_locks as $lock_data) {
			$GLOBALS['SQ_SYSTEM']->releaseLock($lock_data['lockid']);
		}

	}//end releaseUserLocks()


	/**
	* Return the keywords for this asset type.
	*
	* @return array
	* @access public
	*/
	public function getAvailableKeywords()
	{

		$keywords = Array();
		$keywords['asset_attribute_email_image'] = translate('user_email_image');

		return $keywords;

	}//end getAvailableKeywords()


	/**
	* Get bodycopy keyword replacement
	*
	* @param string	$keyword	Keyword to find a replacement for
	*
	* @return string
	* @access public
	* @see _getKeywordReplacement
	*/
	public function getKeywordReplacement($keyword)
	{
		$replacement = '';
		if ($keyword == 'asset_attribute_email_image') {
			$email = $this->attr('email');
			$email_image_link = '';
			if (!empty($email)) {
				$img_url = $_SERVER['PHP_SELF'].'?SQ_ACTION=attribute_image&id='.$this->id.'&attribute=email';
				$email_image_link = '<img id="email_'.$this->id.'" src="'.$img_url.'" />';
			}//end if
			$replacement = $email_image_link;
		} else {
			$replacement = parent::getKeywordReplacement($keyword);
		}//end if

		return $replacement;

	}//end getKeywordReplacement()


}//end class

?>
