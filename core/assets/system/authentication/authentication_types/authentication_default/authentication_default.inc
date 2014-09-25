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
* $Id: authentication_default.inc,v 1.21 2012/08/30 01:09:10 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/authentication/authentication/authentication.inc';


/**
* Authentication_Default
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.21 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Authentication_Default extends Authentication
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Authentication_Default($assetid=0)
	{
		$this->Authentication($assetid);

	}//end constructor


	/**
	* Given a username, attempts to locate and return a user object.
	*
	* @param string	$username	The username of the user to locate
	*
	* @return object
	* @access public
	*/
	function &locateUser($username)
	{
		$db = DAL::getDb();
		try {
			$bind_vars = Array('user_name' => $username);
			$result = MatrixDAL::executeAll('authentication_default', 'locateUser', $bind_vars);
			if (empty($result)) {
				$result = Array();
			} else {
				$result = $result[0];
			}
		} catch (DALException $e) {
			throw new Exception('Unable to locate user with user name "'.$username.'" due to a database error: '.$e->getMessage());
		}

		if (isset($result['assetid']) && !empty($result['assetid'])) {
			$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($result['assetid']);

			if (is_null($user)) {
				// no user was found with that username
			} else {
				return $user;
			}
		}

		$null = NULL;
		return $null;

	}//end locateUser()


	/**
	* Attempts to login a user given a username and password
	*
	* If the wrong password is supllied, the user account will be locked after
	* a system defined number of incorrect login attempts
	* returns user or NULL
	*
	* @param string	$username	the user name to search for
	* @param string	$password	the password to match
	*
	* @return object
	* @access public
	*/
	function &authenticateUser($username, $password)
	{
		$user = NULL;
		// Try force lowercase username first, if not found or not in allowed type, try normal login
		if (SQ_CONF_FORCE_LOWERCASE_USERNAME) {
			$username_lowercase = strtolower($username);
			$user_lowercase = $this->locateUser($username_lowercase);
			if(!is_null($user_lowercase) && in_array(get_class($user_lowercase), Array('User', 'Simple_Edit_User', 'Backend_User', 'System_User')))
				$user = $user_lowercase;
		}

		if (is_null($user)) {
			$user = $this->locateUser($username);
        }

		if (is_null($user)) {
			// no user was found with that username
		} else if (!$user->comparePassword($password)) {
			// a user was found, but the wrong password was supplied
			$this->_registerInvalidLogin($user);
		} else {
			// user found with username and password supplied
			unset($_SESSION['user_login_attempts']);

            // Check for password upgrade.
            include_once SQ_FUDGE_PATH.'/general/security.inc';
            if (security_password_needs_rehash($user->attr('password')) === TRUE) {
                // Password needs updating, so force the level to change the users password.
                $GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_OPEN);
                $user->setAttrValue('password', $password);
                $user->saveAttributes();
                $GLOBALS['SQ_SYSTEM']->restoreRunLevel();
            }

			return $user;
		}

		// need this because of return by reference
		$null = NULL;
		return $null;

	}//end authenticateUser()


	/**
	* Registers that an invalid login attempt has been made and locks the user account if required
	*
	* @param object	&$user	the user account upon which an invalid login attempt was made
	*
	* @return void
	* @access private
	*/
	function _registerInvalidLogin(&$user)
	{
		// Zero (0) is unlimited attempts
		if (SQ_CONF_MAX_LOGIN_ATTEMPTS == 0) return;

		$username = $user->attr('username');
		if (!isset($_SESSION['user_login_attempts'])) {
			$_SESSION['user_login_attempts'] = Array();
		}
		if (!isset($_SESSION['user_login_attempts'][$username])) {
			$_SESSION['user_login_attempts'][$username] = 1;
		} else {
			$_SESSION['user_login_attempts'][$username]++;
		}

		if ($_SESSION['user_login_attempts'][$username] >= SQ_CONF_MAX_LOGIN_ATTEMPTS) {
			unset($_SESSION['user_login_attempts']);

			if (!is_null($user) && $user->canLogin()) {
				// pretend to be the root user so we can lock the account
				$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
				if ($GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
					$success          = FALSE;
					$changed_runlevel = FALSE;

					// turn off status integrity checking
					if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_STATUS_INTEGRITY)) {
						// FIXME: TESTME
						$changed_runlevel = TRUE;
						$GLOBALS['SQ_SYSTEM']->setRunLevel($GLOBALS['SQ_SYSTEM']->getRunLevel() - SQ_SECURITY_STATUS_INTEGRITY);
					}

					// lock the user
					if ($user->processStatusChange(SQ_STATUS_UNDER_CONSTRUCTION)) {
						$success = TRUE;
					}

					// if root user account is attemped, we need to disable it, change to UC status is not enough
					if($user instanceof Root_User) {
						$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
                		$vars = Array('SQ_CONF_ROOT_USER_DISABLED' => 1);
                		ob_start();
                        include_once SQ_INCLUDE_PATH.'/system_config.inc';
                        $system_config = new System_Config();
                        $result = $system_config->save($vars);
                		ob_end_clean();
                		$success = TRUE;
                		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
					}


					$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();



					// restore the runlevel if we have to
					if ($changed_runlevel) {
						$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
					}

					// log a message so we know someone has had their account locked
					// we need to do this here because before we restore the current user
					// we are actually logged in as ROOT
					if ($success) {
						$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
						$name = $user->name.' (Id: #'.$user->id.')';
						$msg_reps = Array(
										'user_name'		=> $name,
										'num_attempts'	=> (int)SQ_CONF_MAX_LOGIN_ATTEMPTS,
									);
						$log = $ms->newMessage(Array(), 'system.security.locked', $msg_reps);
						$log->subject = translate('User Account Locked');
						$log->body    = translate('User "%user_name%" has had their account locked after %num_attempts% unsuccessful login attempts');
						$log->parameters['remote_addr'] = $_SERVER['REMOTE_ADDR'];
						$log->parameters['sessionid'] = session_id();
						$log->send();
					}
				}//end if - set current user to root
			}//end if - user exists and can login
		}//end if - too many attempts

	}//end _registerInvalidLogin()


	/**
	* Given a username supplied by HTTP, attempts to locate and return a user object
	*
	* This function is used when processing a pre-authenticated username supplied
	* via HTTP headers or server vars.
	*
	* @param string	$username	The username to search for
	*
	* @return mixed object|NULL
	* @access public
	*/
	function &authenticateHttpUser($username)
	{
		try {
			$bind_vars = Array('user_name' => $username);
			$result = MatrixDAL::executeAll('authentication_default', 'locateUser', $bind_vars);
			if (empty($result)) {
				$result = Array();
			} else {
				$result = $result[0];
			}
		} catch (DALException $e) {
			throw new Exception('Unable to locate user with user name "'.$username.'" due to a database error: '.$e->getMessage());
		}

		$user = NULL;
		if (isset($result['assetid']) && !empty($result['assetid'])) {
			$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($result['assetid']);
		}
		return $user;

	}//end authenticateHttpUser()


}//end class

?>
