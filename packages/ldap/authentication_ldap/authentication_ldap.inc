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
* $Id: authentication_ldap.inc,v 1.1 2004/02/06 03:11:19 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/authentication/authentication/authentication.inc';
require_once SQ_FUDGE_PATH.'/ldap/ldap.inc';


/**
* Authentication_LDAP
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage ldap
*/
class Authentication_LDAP extends Authentication
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Authentication_LDAP($assetid=0)
	{
		$this->Authentication($assetid);

	}//end constructor


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		return Array(SQ_LINK_TYPE_1 => Array('ldap_bridge' => Array('card' => 'M', 'exclusive' => false)));

	}//end _getAllowedLinks()


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
		$user = null;

		$bridges = $GLOBALS['SQ_SYSTEM']->am->getChildren($this->id, 'ldap_bridge', true);
		foreach ($bridges as $bridgeid => $type_code) {

			$bridge = &$GLOBALS['SQ_SYSTEM']->am->getAsset($bridgeid);
			if (is_null($bridge)) continue;

			$ldap = $bridge->connectToLdap();

			// the attribute mappings to the LDAP schemas
			$attrs = $bridge->attr('attributes');

			// we do the search instead of binding directly as the user because ldap->connect_as_user requires
			// the full dn as the first arg and doing it this way doesn't limit you to having all the users in the
			// same organisational unit in the ldap directory
			$result = $ldap->search($bridge->attr('root_dn'),'('.$attrs['uid'].'='.$username.')');
			if (!$result) continue;

			$entry = $ldap->getEntries($result);
			if (!$entry || ($entry['count'] == 0)) continue;
			if (isset($entry[0])) {
				$entry = $entry[0];

				// create our user
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('ldap_user');
				$user = new LDAP_User($bridge->id, $entry);

				// verify password
				$success = @$ldap->connectAsUser($user->attr('dn'), $password);

				if ($success) {
					// user found with username and password supplied
					unset($_SESSION['user_login_attempts']);
					return $user;
				}
			}

		}//end foreach

		if (!is_null($user)) {
			// a user was found, but the wrong password was supplied
			$this->_registerInvalidLogin($user);
		}

		return $null;

	}//end authenticateUser()


	/**
	* Registers that an invalid login attempt has been made
	*
	* @param object User	&$user	the user account upon which an invalid login attempt was made
	*
	* @return void
	* @access private
	*/
	function _registerInvalidLogin(&$user)
	{
		$username = $user->attr('dn');
		if (!isset($_SESSION['user_login_attempts'])) $_SESSION['user_login_attempts'] = Array();
		if (empty($_SESSION['user_login_attempts'][$username])) $_SESSION['user_login_attempts'][$username] = 1;
		else $_SESSION['user_login_attempts'][$username]++;

		if ($_SESSION['user_login_attempts'][$username] >= SQ_CONF_MAX_LOGIN_ATTEMPTS) {
			if (!is_null($user) && $user->canLogin()) {
				// log a message so we know someone has tried to log in too many times
				$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
				$log_body = 'LDAP User "'.$user->name.'" (username: '.$username.') has had their account accessed unsuccessfully '.(int)SQ_CONF_MAX_LOGIN_ATTEMPTS.' times in a session';
				$log = $ms->newMessage(Array(), 'User Account Access Warning', $log_body, 'system.security.login');
				$log->parameters['remote_addr'] = $_SERVER['REMOTE_ADDR'];
				$log->parameters['sessionid'] = session_id();
				$log->send();
			}
		}

	}//end _registerInvalidLogin()


}//end class

?>