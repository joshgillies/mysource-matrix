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
* $Id: user.inc,v 1.144 2013/09/25 05:50:11 ewang Exp $
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
* @version $Revision: 1.144 $
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

		if(SQ_CONF_FORCE_LOWERCASE_USERNAME && in_array(get_class($this), Array('User', 'Simple_Edit_User', 'Backend_User', 'System_User')))
			$this->setAttrValue('username', strtolower($this->attr('username')));

		$username = trim($this->attr('username'));
		$password = $this->attr('password');

		if (!$this->_catchBlankUserNamePass($username, $password)) {
			if (!array_get_index($this->_tmp, 'username_set') || !array_get_index($this->_tmp, 'password_set')) {
				// in this case they have tried to create without even trying to set the username or password, so
				// we error here.  Otherwise errors have already been thrown so we don't throw them now.
				trigger_localised_error('CORE0084', sprintf(translate('Unable to create a %s without a username and password'), $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name')), E_USER_WARNING);
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
			trigger_localised_error('CORE0019', sprintf(translate('Unable to set an initial web path for "%s"'), $this->name), E_USER_WARNING);
			return FALSE;
		}

		// Ensure that the current user can perform the following permissions change regardless of their own permissions on the parent
		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

		// give the user admin access to him/herself
		if (!$GLOBALS['SQ_SYSTEM']->am->setPermission($this->id, $this->id, SQ_PERMISSION_ADMIN, TRUE)) {
			trigger_localised_error('CORE0015', sprintf(translate('Unable to create user - permission could not be set on user asset "%s"'), $this->name), E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

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
	function setAttrValue($name, $value, $filter=NULL)
	{
	    include_once SQ_FUDGE_PATH.'/general/security.inc';

		// if this is the password, encrypt it
		if ($name == 'password') {
			$this->_tmp['password_set'] = TRUE; // so we know at create time whether to throw an extra error
			if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PASSWORD_VALIDATION)) {
				if (!$this->validatePassword($value)) return FALSE;
			}
			// Make sure password doesn't get re-hashed while reverting to system version
			//$value = $GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION'] ? $value : crypt($value); // let salt be generated
            $value = $GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION'] ? $value : security_password_hash($value); // let salt be generated
		} else if ($name == 'username') {
			$this->_tmp['username_set'] = TRUE; // so we know at create time whether to throw an extra error
			if (!$this->_catchBlankSetUserName(trim($value))) {
				trigger_localised_error('CORE0110', translate('Username cannot be blank'), E_USER_WARNING);
				return FALSE;
			}

			if(SQ_CONF_FORCE_LOWERCASE_USERNAME && in_array(get_class($this), Array('User', 'Simple_Edit_User', 'Backend_User', 'System_User')))
				$value = strtolower($value);
		}

		if (!parent::setAttrValue($name, $value, $filter)) return FALSE;

		if ($name === 'password' && defined('SQ_PASSWD_RULE_PASSWORD_HISTORY') && SQ_PASSWD_RULE_PASSWORD_HISTORY > 0) {
		    $history_passwords = $this->attr('history_passwords');
		    if(count($history_passwords) >= SQ_PASSWD_RULE_PASSWORD_HISTORY) {
			array_pop($history_passwords);
		    }
		    array_unshift($history_passwords, $value);
		    parent::setAttrValue('history_passwords', $history_passwords, $filter);
		}
		// tell the workspace (or anyone else listening) that the username has been updated
		if ($name == 'username') {
			$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
			$em->broadcastEvent($this, 'usernameUpdated', Array('username' => $value));
		}
		return TRUE;

	}//end setAttrValue()


	/**
	* Provides extra checking and disable invalid status change on errors
	*
	* @param int		$new_status		the new status to set for this asset
	* @param boolean	$update_parents	Specify if we want to update the parent as well
	* @param boolean	$run_updated	specify it we want to run _upated
	*
	* @return boolean
	* @access public
	*/
	function processStatusChange($new_status, $update_parents=TRUE, $run_updated=TRUE)
	{
		// Get the assetid of the root user sucka!
		$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('root_user');

		// Do not do this procedure if root user or a shadow asset (LDAP user etc.)
		if ($this->id != $root_user && strpos($this->id, ':') === FALSE) {
			// Ok to proceed
			switch ($new_status) {
				case SQ_STATUS_UNDER_CONSTRUCTION:
					// If the current status is archived, try putting in the unique value again
					if ($this->status == SQ_STATUS_ARCHIVED) {
						// Connect four!
						$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
						$db = MatrixDAL::getDb();
						$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

						// Get the attribute id
						try {
							$sql = 'SELECT oa.attrid as owning_attrid, a.attrid as attrid FROM sq_ast_attr a INNER JOIN sq_ast_attr oa ON (a.owning_type_code = oa.type_code AND a.name = oa.name) WHERE a.name = :name AND a.type_code = :type_code';
							$query = MatrixDAL::preparePdoQuery($sql);
							MatrixDAL::bindValueToPdo($query, 'name', 'username');
							MatrixDAL::bindValueToPdo($query, 'type_code', $this->type());
							$attributes = MatrixDAL::executePdoAssoc($query);
						} catch (Exception $e) {
							throw new Exception('Cannot check for attribute value "'.$this->name.'" of type code "'.$this->type().'" due to database error: '.$e->getMessage());
							return FALSE;
						}

						// Set the attributes
						$owning_attributeid = $attributes[0]['owning_attrid'];
						$attributeid = $attributes[0]['attrid'];

						// If username attribute already set in db, no need to do anything here #3994
						if ($this->_uniqueAttributeExists($owning_attributeid, $this->id)) {
							$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
							$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
							break;
						}

						// Check if another user has the same username
						$name = $this->getValidUsername($owning_attributeid, $this->attr('username'));

						// Name has changed better change it
						if ($name != $this->attr('username')) {
							$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
							$this->setAttrValue('username', $name);
							$this->saveAttributes();
							$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
						}//end if

						// Ok to contine, add the unique value back into the database
						try {
							$prepared = MatrixDAL::preparePdoQuery('INSERT INTO sq_ast_attr_uniq_val (custom_val, owning_attrid, assetid) VALUES (:custom_val, :owning_attrid, :assetid)');
							MatrixDAL::bindValueToPdo($prepared, 'custom_val', $name);
							MatrixDAL::bindValueToPdo($prepared, 'owning_attrid', $owning_attributeid);
							MatrixDAL::bindValueToPdo($prepared, 'assetid',	$this->id);
							MatrixDAL::execPdoQuery($prepared);
						} catch (Exception $e) {
							throw new Exception('Unable to add the unique attribute value for asset "'.$this->name.'" (#'.$this->id.') due to database error: '.$e->getMessage());
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
							return FALSE;
						}

						// Everything ok here save changes
						$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					}//end if
				break;
				case SQ_STATUS_ARCHIVED:
					$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
					$db = MatrixDAL::getDb();
					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
					try {
						$prepared = MatrixDAL::preparePdoQuery('DELETE FROM sq_ast_attr_uniq_val WHERE custom_val=:custom_val AND assetid=:assetid');
						MatrixDAL::bindValueToPdo($prepared, 'custom_val', $this->attr('username'));
						MatrixDAL::bindValueToPdo($prepared, 'assetid',	$this->id);
						MatrixDAL::execPdoQuery($prepared);
					} catch (Exception $e) {
						throw new Exception('Unable to delete unique attribute value for asset "'.$this->name.'" (#'.$this->id.') due to database error: '.$e->getMessage());
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						return FALSE;
					}
					$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				break;
			}//end switch
		}//end if

		return parent::processStatusChange($new_status, $update_parents, $run_updated);

	}//end processStatusChange()


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
        include_once SQ_FUDGE_PATH.'/general/security.inc';
		$password = $this->attr('password');

		if (!trim($password)) return FALSE;

		//return (crypt($entered_password,$password) == $password);
        return security_password_verify($entered_password,$password);

	}//end comparePassword()


	/**
	* Returns next available username
	*
	* @param string	$owning_attrid		The attribute id of the owning attribute
	* @param string	$current_username	The current username
	*
	* @return string|FALSE (on error)
	* @access public
	*/
	function getValidUsername($owning_attrid, $current_username)
	{
		$valid_username = $current_username;
		$found_one = FALSE;
		while (!$found_one) {
			$current_values = Array();
			try {
				$prepared = MatrixDAL::preparePdoQuery('SELECT custom_val FROM sq_ast_attr_uniq_val WHERE owning_attrid=:owning_attrid AND custom_val LIKE :custom_val');
				MatrixDAL::bindValueToPdo($prepared, 'custom_val', $valid_username);
				MatrixDAL::bindValueToPdo($prepared, 'owning_attrid', $owning_attrid);
				$current_values = MatrixDAL::executePdoAssoc($prepared);
			} catch (Exception $e) {
				throw new Exception('Unable to open database due to database error: '.$e->getMessage());
				return FALSE;
			}

			if (empty($current_values)) {
				$found_one = TRUE;
			} else {
				require_once SQ_FUDGE_PATH.'/general/file_system.inc';
				$valid_username = increment_filename($valid_username);
			}//end if
		}//end while

		return $valid_username;

	}//end getValidUsername()


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
				$GLOBALS['SQ_SYSTEM']->addMessage(translate('Password cannot be blank'));
			} else {
				trigger_localised_error('CORE0247', translate('Password cannot be blank'), E_USER_NOTICE);
			}
			$this->_tmp['password_validation_error'] = TRUE;
			return FALSE;
		}

		$errors = Array();

		// errors for when a character type does not occur in the password
		$not_occurring_error_map = Array(
									'SQ_PASSWD_RULE_NUM_CAPS'	=> Array('CORE0222', translate('Password contains only %1$s capital letters, must contain at least %2$s')),
									'SQ_PASSWD_RULE_NUM_LOWER'	=> Array('CORE0223', translate('Password contains only %1$s lower-case letters, must contain at least %2$s')),
									'SQ_PASSWD_RULE_NUM_INT'	=> Array('CORE0224', translate('Password contains only %1$s numeric characters, must contain at least %2$s')),
									'SQ_PASSWD_RULE_NUM_PUNC'	=> Array('CORE0225', translate('Password contains only %1$s punctuation characters, must contain at least %2$s')),
									'SQ_PASSWD_RULE_NUM_SPC'	=> Array('CORE0226', translate('Password contains only %1$s spaces, must contain at least %2$s')),
								   );

		// errors for when a character type incorrectly occurs in the password
		$occurring_error_map = Array(
								'SQ_PASSWD_RULE_NUM_CAPS'	=> Array('CORE0227', translate('Password must not contain capital letters')),
								'SQ_PASSWD_RULE_NUM_LOWER'	=> Array('CORE0228', translate('Password must not contain lower-case letters')),
								'SQ_PASSWD_RULE_NUM_INT'	=> Array('CORE0229', translate('Password must not contain numeric characters')),
								'SQ_PASSWD_RULE_NUM_PUNC'	=> Array('CORE0230', translate('Password must not contain punctuation characters')),
								'SQ_PASSWD_RULE_NUM_SPC'	=> Array('CORE0231', translate('Password must not contain spaces')),
							   );

		if (defined('SQ_PASSWD_RULE_LENGTH') && SQ_PASSWD_RULE_LENGTH > -1) {
			if (strlen($passwd) < SQ_PASSWD_RULE_LENGTH) {
				$errors[] = Array(
				    'CORE0220',
				    sprintf(
				        translate('Password is only %1$s characters long, it needs to be at least %2$s characters'),
				        strlen($passwd),
				        SQ_PASSWD_RULE_LENGTH
				    ),
				);
			}
		}

		if (defined('SQ_PASSWD_RULE_DISALLOW_USER_INFO') && SQ_PASSWD_RULE_DISALLOW_USER_INFO == 0) {
			$components = Array('username', 'first_name', 'last_name', 'email');
			foreach ($components as $component) {
				$attr = $this->attr($component);
				if (!empty($attr) && strpos(strtolower($passwd), strtolower($attr)) !== FALSE) {
					$errors[] = Array(
					    'CORE0221',
					    translate('Password must not contain any user information, eg. name, email'),
					);
					break;
				}
			}
		}

		if (defined('SQ_PASSWD_RULE_PASSWORD_HISTORY') && SQ_PASSWD_RULE_PASSWORD_HISTORY > 0) {
		    $history_passwords = $this->attr('history_passwords');
		    foreach ($history_passwords as $password) {
                if (crypt($passwd, $password) === $password) {
                    $errors[] = Array(
                        'CORE0321',
                        translate('You cannot choose a password that has previously been used on this account.'),
                    );
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
						$errors[] = $occurring_error_map[$rule];
					}
				} else if ($tmp_counter < $value) {
					if (isset($not_occurring_error_map[$rule])) {
					    $map_rule    = $not_occurring_error_map[$rule];
					    $map_rule[1] = sprintf($map_rule[1], $tmp_counter, $value);
						$errors[]    = $map_rule;
					}
				}
			}
		}

		// check if this password contains blacklisted word
		$blacklist = SQ_PASSWD_RULE_BLACK_LIST;
		if (!empty($blacklist)) {
			$blacklist = str_replace("\r\n","\n", $blacklist);
			$blacklist = str_replace("\r","\n", $blacklist);
			$blacklist = explode("\n", $blacklist);
			foreach ($blacklist as $word) {
				if ((!SQ_PASSWD_RULE_BLACK_LIST_EXACT && preg_match('/'.$word.'/', $passwd)) || (SQ_PASSWD_RULE_BLACK_LIST_EXACT && (strcmp($passwd, $word) == 0))) {
					$errors[] = Array(
					    'CORE0260',
					    translate('Password contains black listed word.'),
					);
				}
			}
		}

		if ($add_system_message) {
			$msgs = '';
			foreach ($errors as $error) {
				$msgs .= '<li>'.$error[1].'</li>';
			}
			if (!empty($errors)) {
				$GLOBALS['SQ_SYSTEM']->addMessage(sprintf(translate('The supplied password is not valid: %s'), '<ul>'.$msgs.'</ul>'));
			}
		} else {
			foreach ($errors as $error) {
				trigger_localised_error($error[0], $error[1], E_USER_WARNING);
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
			$res[] = sprintf(translate('Passwords must be at least %d characters long'), SQ_PASSWD_RULE_LENGTH);

		}
		if (SQ_PASSWD_RULE_DISALLOW_USER_INFO == 0) {
			// yes this was built the wrong way around
			$res[] = translate('Passwords must not contain any user information');

		}

		if (SQ_PASSWD_RULE_NUM_CAPS > 1) {
			$res[] = sprintf(translate('Passwords must contain at least %1$s %2$s'), SQ_PASSWD_RULE_NUM_CAPS, translate('capital letters'));


		}
		if (SQ_PASSWD_RULE_NUM_LOWER > 1) {
			$res[] = sprintf(translate('Passwords must contain at least %1$s %2$s'), SQ_PASSWD_RULE_NUM_LOWER, translate('lower case letters'));


		}
		if (SQ_PASSWD_RULE_NUM_INT > 1) {
			$res[] = sprintf(translate('Passwords must contain at least %1$s %2$s'), SQ_PASSWD_RULE_NUM_INT, translate('numeric digits'));


		}
		if (SQ_PASSWD_RULE_NUM_PUNC > 1) {
			$res[] = sprintf(translate('Passwords must contain at least %1$s %2$s'), SQ_PASSWD_RULE_NUM_PUNC, translate('punctuation characters'));


		}
		if (SQ_PASSWD_RULE_NUM_SPC > 1) {
			$res[] = sprintf(translate('Passwords must contain at least %1$s %2$s'), SQ_PASSWD_RULE_NUM_SPC, translate('spaces'));


		}

		if (SQ_PASSWD_RULE_NUM_CAPS == 1) {
			$res[] = sprintf(translate('Passwords must contain at least %1$s %2$s'), translate('one'), translate('capital letter'));



		}
		if (SQ_PASSWD_RULE_NUM_LOWER == 1) {
			$res[] = sprintf(translate('Passwords must contain at least %1$s %2$s'), translate('one'), translate('lower case letter'));



		}
		if (SQ_PASSWD_RULE_NUM_INT == 1) {
			$res[] = sprintf(translate('Passwords must contain at least %1$s %2$s'), translate('one'), translate('numeric digit'));



		}
		if (SQ_PASSWD_RULE_NUM_PUNC == 1) {
			$res[] = sprintf(translate('Passwords must contain at least %1$s %2$s'), translate('one'), translate('punctuation character'));



		}
		if (SQ_PASSWD_RULE_NUM_SPC == 1) {
			$res[] = sprintf(translate('Passwords must contain at least %1$s %2$s'), translate('one'), translate('space'));



		}

		$disallowed_chars = Array();
		if (SQ_PASSWD_RULE_NUM_CAPS == -1) {
			$disallowed_chars[] = translate('capital letters');

		}
		if (SQ_PASSWD_RULE_NUM_LOWER == -1) {
			$disallowed_chars[] = translate('lower case letters');

		}
		if (SQ_PASSWD_RULE_NUM_INT == -1) {
			$disallowed_chars[] = translate('numeric digits');

		}
		if (SQ_PASSWD_RULE_NUM_PUNC == -1) {
			$disallowed_chars[] = translate('punctuation characters');

		}
		if (SQ_PASSWD_RULE_NUM_SPC == -1) {
			$disallowed_chars[] = translate('spaces');

		}
		if (!empty($disallowed_chars)) {
			include_once SQ_FUDGE_PATH.'/general/text.inc';
			$res[] = ucfirst(sprintf(translate('%s are not allowed in passwords'), make_readable_list($disallowed_chars, strtolower(translate('And')))));


		}

		if (trim(SQ_PASSWD_RULE_BLACK_LIST) != '') {
			$black_words = Array();
			foreach (explode("\n", SQ_PASSWD_RULE_BLACK_LIST) as $word) {
				$black_words[] = '"'.trim($word).'"';
			}
			if (!empty($black_words)) {
				if (SQ_PASSWD_RULE_BLACK_LIST_EXACT) {
					$res[] = sprintf(translate('The following passwords are not allowed: %s'), implode(', ', $black_words));

				} else {
					$res[] = sprintf(translate('Passwords must not contain any of these words: %s'), implode(', ', $black_words));

				}
			}
		}

		if (!empty($res)) {
			return translate('The following rules apply to new passwords:').'<ul class="password-rules"><li>'.implode('</li><li>', $res).'</li></ul>';

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
	public function _getName($short_name=FALSE, $contextid=NULL)
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
			return translate('You can\'t delete the link between a user and their inbox');

		}

		$workspace_link = $this->getWorkspaceLink();
		if (!empty($workspace_link) && ($workspace_link['linkid'] == $linkid)) {
			return translate('You can\'t delete the link between a user and their workspace');

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
			$fetch_roles = ((SQ_CONF_ENABLE_ROLES_PERM_SYSTEM  == '1' || SQ_CONF_ENABLE_ROLES_WF_SYSTEM == '1') && (SQ_CONF_ENABLE_GLOBAL_ROLES == '1'));
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
			$fetch_roles = ((SQ_CONF_ENABLE_ROLES_PERM_SYSTEM  == '1' || SQ_CONF_ENABLE_ROLES_WF_SYSTEM == '1') && (SQ_CONF_ENABLE_GLOBAL_ROLES == '1'));
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
	* ...or FALSE if there is no support for it in this locking method (requires
	* ability to get active locks).
	*
	* @return array|boolean
	* @access public
	*/
	function getUserLocks()
	{
		$db = MatrixDAL::getDb();
		try {
			$class_name = 'locking_method_'.SQ_CONF_LOCKING_METHOD;
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($class_name);

			if (eval('return '.$class_name.'::supportsGetActiveLocks();') === TRUE) {
				$result = eval('return '.$class_name.'::getActiveLocks($this->id);');
			} else {
				// Unable to get the list of active locks due to lack of support
				$result = FALSE;
			}

		} catch (Exception $e) {
			throw new Exception('Unable to get all locks for current user (userid #'.$this->id.'), '.$e->getMessage());

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

		$keywords = parent::getAvailableKeywords();
		$keywords['asset_attribute_email_image'] = translate('User Email Image');

		// right now we have totaly of 56 keywords that are being returned by this function
		// if we have more keywords added later , assertion in will need to be changed for
		// unit_tests/by_file/core/assets/users/user/user.inc/test_get_user_locks.test

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
		$replacement = NULL;

		// Remove any modifiers from keyword
		$full_keyword = $keyword;
		$keyword = parse_keyword($keyword, $modifiers);
		$contextid = extract_context_modifier($modifiers);

		if ($contextid !== NULL) {
			// If we were able to extract a context ID to change to, and it's
			// different to our current one, then change and then reload a copy
			// of the asset in that context (as we would need to do anyway)

			if ((int)$contextid !== $GLOBALS['SQ_SYSTEM']->getContextId()) {
				$GLOBALS['SQ_SYSTEM']->changeContext($contextid);
				$contexted_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->id);

				// Get the keyword without any modifiers
				$replacement = $contexted_asset->getKeywordReplacement($keyword);

				// Then apply the ones we had remaining, then return it
				apply_keyword_modifiers($replacement, $modifiers, Array('assetid' => $contexted_asset->id));

				unset($contexted_asset);
				$GLOBALS['SQ_SYSTEM']->restoreContext();
				return $replacement;

			}//end if contextid is not the currently active context

		}//end if contextid is not NULL


		if ($keyword == 'asset_attribute_email_image') {
			$email = $this->attr('email');
			$email_image_link = '';
			if (!empty($email)) {
				$img_url = $_SERVER['PHP_SELF'].'?SQ_ACTION=attribute_image&id='.$this->id.'&attribute=email';
				$email_image_link = '<img id="email_'.$this->id.'" src="'.$img_url.'" />';
			}//end if
			$replacement = $email_image_link;
		}

		if ($replacement !== NULL) {
			if (count($modifiers) > 0) {
				apply_keyword_modifiers($replacement, $modifiers, Array('assetid' => $this->id));
			}
		} else {
			// Use full keyword, so that modifiers still get respected
			$replacement = parent::getKeywordReplacement($full_keyword);
		}

		return $replacement;

	}//end getKeywordReplacement()


	/**
	* Test if a unique attribute exists in db
	*
	* @param string	$owning_attrid		The attribute id of the owning attribute
	* @param string	$assetid			The asset id
	*
	* @return Boolean
	* @access private
	*/
	function _uniqueAttributeExists($owning_attrid, $assetid)
	{

		try {
			$prepared = MatrixDAL::preparePdoQuery('SELECT custom_val FROM sq_ast_attr_uniq_val WHERE owning_attrid=:owning_attrid AND assetid=:assetid');
			MatrixDAL::bindValueToPdo($prepared, 'assetid', $assetid);
			MatrixDAL::bindValueToPdo($prepared, 'owning_attrid', $owning_attrid);
			$current_values = MatrixDAL::executePdoAssoc($prepared);
		} catch (Exception $e) {
			throw new Exception('Unable to open database due to database error: '.$e->getMessage());
			return FALSE;
		}

		return (!empty($current_values));
	}//end _uniqueAttributeExists()



	/**
	* Get the user inbox messages
	*
	* NOTE: Used by %globals_user_message_*% keyword
	*
	* @return array
	* @access public
	*/
	public function getMessages($type, $messages_size=NULL)
	{
		// Try local cache first
		$cache_key = $type.'_'.$messages_size;
		if (isset($this->_tmp['user_messages']) && isset($this->_tmp['user_messages'][$cache_key])) {
			return $this->_tmp['user_messages'][$cache_key];
		}

		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
		switch ($type) {
			case 'inbox':
				$messages = $ms->getInternalMessages($this->id, Array(SQ_MSG_UNREAD, SQ_MSG_READ), $messages_size);
			break;
			case 'sent':
				$messages = $ms->getInternalMessages($this->id, Array(SQ_MSG_UNREAD, SQ_MSG_READ), $messages_size, 'inbox.sent');
			break;
			case 'trash':
				$messages = $ms->getInternalMessages($this->id, Array(SQ_MSG_DELETED), $messages_size);
			break;
			default:
				$messages = Array();
			break;
		}//end switch

		$this->_tmp['user_messages'][$cache_key] = $messages;

		return $messages;

	}//end getMessages()


	/**
	* Clear the internal message data in the local cache
	* See: $this->getMessages()
	*
	* @return void
	* @access public
	*/
	function clearUserMessageLocalCache()
	{
		unset($this->_tmp['user_messages']);
	}


}//end class

?>
