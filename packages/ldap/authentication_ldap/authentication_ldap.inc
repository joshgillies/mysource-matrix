<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: authentication_ldap.inc,v 1.6.2.1 2006/02/14 22:39:52 mmcintyre Exp $
*
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
* @version $Revision: 1.6.2.1 $
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
		return Array(
					SQ_LINK_TYPE_1	=> Array(
											'ldap_bridge'	=> Array(
																	'card'		=> 'M',
																	'exclusive'	=> false,
															   )
									   )
			   );

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

			if (is_null($ldap)) return $user;

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
				$msg_reps = Array(
								'asset_name'	=> $user->name,
								'user_name'		=> $username,
								'num_attempts'	=> (int)SQ_CONF_MAX_LOGIN_ATTEMPTS,
							);
				$log = $ms->newMessage(Array(), 'system.security.login.ldap', $msg_reps);
				$log->parameters['remote_addr'] = $_SERVER['REMOTE_ADDR'];
				$log->parameters['sessionid'] = session_id();
				$log->send();
			}
		}

	}//end _registerInvalidLogin()


	/**
	* If this authentication has a user matching the username in $_SERVER['REMOTE_USER'], return that user
	*
	* @access public
	* @return Object | null
	*/
	function &authenticateHttpUser()
	{
		$user = null;
		$bridges = $GLOBALS['SQ_SYSTEM']->am->getChildren($this->id, 'ldap_bridge', true);
		error_log(print_r($bridges, true));
		foreach ($bridges as $bridgeid => $type_code) {
			$bridge = &$GLOBALS['SQ_SYSTEM']->am->getAsset($bridgeid);
			if (is_null($bridge)) continue;
			error_log("Looking for ".$_SERVER['REMOTE_USER']." in  bridge ".$bridge->name);

			// Search for users matching the HTTP username
			$ldap = $bridge->connectToLdap();
			$attrs = $bridge->attr('attributes');
			$result = $ldap->search($bridge->attr('root_dn'),'('.$attrs['uid'].'='.$_SERVER['REMOTE_USER'].')');
			if (!$result) {
				error_log("No results");
				continue;
			}

			$entry = $ldap->getEntries($result);
			if (!$entry || ($entry['count'] == 0)) {
				error_log("No entries");
				continue;
			}
			if (isset($entry[0])) {
				$entry = $entry[0];
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('ldap_user');
				$user = new LDAP_User($bridge->id, $entry);
				break;
			}
		}
		return $user;

	}//end authenticateHttpUser()


}//end class
?>