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
* $Id: authentication_default.inc,v 1.7 2005/04/27 00:14:43 lwright Exp $
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
* @version $Revision: 1.7 $
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
				  FROM '.SQ_TABLE_RUNNING_PREFIX.'ast a, sq_ast_attr at, '.SQ_TABLE_RUNNING_PREFIX.'ast_attr_uniq_val auv';
		$where = 'a.type_code IN (
								SELECT type_code
								FROM sq_ast_typ_inhd
								WHERE inhd_type_code = '.$db->quote('user').'
								)
				  AND at.name           = '.$db->quote('username').'
				  AND auv.assetid       = a.assetid
				  AND at.attrid         = auv.owning_attrid
				  AND auv.custom_val    = '.$db->quote($username);
	
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
				$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
				if ($GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
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
						$msg_reps = Array(
								'user_name'		=> $username,
								'num_attempts'	=> (int)SQ_CONF_MAX_LOGIN_ATTEMPTS,
							);
						$log = $ms->newMessage(Array(), 'system.security.login', $msg_reps);
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