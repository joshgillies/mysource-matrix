<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: authentication_default.inc,v 1.2 2004/09/29 01:51:41 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/authentication/authentication/authentication.inc';


/**
* Authentication_Default
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
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
	* Attempts to find a user that matches the passed username and password
	*
	* If the wrong password is supllied, the user account will be locked after
	* a system defined number of incorrect login attempts
	*
	* @param string	$username	the user name to search for
	* @param string	$password	the password to match
	*
	* @return object User | NULL
	* @access public
	*/
	function &authenticateUser($username, $password)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql   = 'SELECT a.assetid, a.type_code
				  FROM '.SQ_TABLE_RUNNING_PREFIX.'asset a, '.SQ_TABLE_PREFIX.'asset_attribute at, '.SQ_TABLE_RUNNING_PREFIX.'asset_attribute_value av ';
		$where = 'a.type_code IN (
								SELECT type_code
								FROM '.SQ_TABLE_PREFIX.'asset_type_inherited
								WHERE inherited_type_code = '.$db->quote('user').'
								)
				  AND at.name         = '.$db->quote('username').'
				  AND at.attributeid  = av.attributeid
				  AND av.assetid      = a.assetid
				  AND av.custom_value = '.$db->quote($username);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'av');

		$result = $db->getRow($sql.$where);
		assert_valid_db_result($result);

		if ($result['assetid']) {
			$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($result['assetid']);
			
			if (is_null($user)) {
				// no user was found with that username
			} else if (!$user->comparePassword($password)) {
				// a user was found, but the wrong password was supplied
				$this->_registerInvalidLogin($user);
			} else {
				// user found with username and password supplied
				unset($_SESSION['user_login_attempts']);
				return $user;
			}
		}

		// need this because of return by reference
		$null = null;
		return $null;

	}//end authenticateUser()


	/**
	* Registers that an invalid login attempt has been made and locks the user account if required
	*
	* @param object User	&$user	the user account upon which an invalid login attempt was made
	*
	* @return void
	* @access private
	*/
	function _registerInvalidLogin(&$user)
	{
		$username = $user->attr('username');
		if (!isset($_SESSION['user_login_attempts'])) $_SESSION['user_login_attempts'] = Array();
		if (empty($_SESSION['user_login_attempts'][$username])) $_SESSION['user_login_attempts'][$username] = 1;
		else $_SESSION['user_login_attempts'][$username]++;

		if ($_SESSION['user_login_attempts'][$username] >= SQ_CONF_MAX_LOGIN_ATTEMPTS) {
			if (!is_null($user) && $user->canLogin()) {
				// pretend to be the root user so we can lock the account
				if ($GLOBALS['SQ_SYSTEM']->setCurrentUser($GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user'))) {
					$success = false;
					if ($user->processStatusChange(SQ_STATUS_UNDER_CONSTRUCTION)) {
						$GLOBALS['SQ_SYSTEM']->addMessage('User "'.$username.'" locked');
						$success = true;
					}
					$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();

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

	}//end _registerInvalidLogin()


}//end class

?>