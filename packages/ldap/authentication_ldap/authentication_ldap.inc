<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: authentication_ldap.inc,v 1.33 2012/10/10 04:32:42 akarelia Exp $
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
* @version $Revision: 1.33 $
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
	* Attempts to find a user that matches the passed username and password
	*
	* If the wrong password is supplied, the user account will be locked after
	* a system defined number of incorrect login attempts
	*
	* @param string	$username	the user name to search for
	* @param string	$password	the password to match
	*
	* @return mixed object|NULL
	* @access public
	*/
	function &authenticateUser($username, $password)
	{
		$user = NULL;
		$bridge_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_SC_LINK_BACKEND_NAV, 'ldap_bridge', TRUE);

		$bridges = Array();
		if (!empty($bridge_links)) {
			foreach($bridge_links as $bridge_info) {
				$bridges[$bridge_info['sort_order']] = $bridge_info['minorid'];
			}
		}

		foreach ($bridges as $sort_order => $bridgeid) {
			$bridge = $GLOBALS['SQ_SYSTEM']->am->getAsset($bridgeid);
			if (is_null($bridge)) continue;

			// if status is not live then skip this bridge
			if ($bridge->status != SQ_STATUS_LIVE) continue;

			$ldap = $bridge->connectToLdap();

			if (is_null($ldap)) continue;

			$entry = $this->_getLdapEntries($username, $bridge, $ldap);
			if (!$entry || ($entry['count'] == 0)) {
				continue;
			}
			if (isset($entry[0])) {
				$entry = $entry[0];

				// create our user
				$user_type = $bridge->attr('user_type');
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($user_type);
				$user = new $user_type($bridge->id, $entry);

				// verify password
				$success = @$ldap->connectAsUser($user->attr('dn'), $password);

				if ($success) {
					// ldap user found with username and password supplied
					if ($this->attr('log_in_as') == 'local') {
						// If we are logging in as a local user, we will ask
						// Default Auth to merely locate a user for us
						$auth_assetids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('authentication_default');
						if (count($auth_assetids) > 0) {
							$auth_default = $GLOBALS['SQ_SYSTEM']->am->getAsset($auth_assetids[0]);
						}

						$user = $auth_default->locateUser($username);
					}

					if (!is_null($user)) {
						unset($_SESSION['user_login_attempts']);
					}
					return $user;
				}
			}
		}//end foreach

		if (!is_null($user)) {
			// a user was found, but the wrong password was supplied
			$this->_registerInvalidLogin($user);
		}

		$return_value = NULL;
		return $return_value;

	}//end authenticateUser()


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
		$user = NULL;
		$bridge_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_SC_LINK_BACKEND_NAV, 'ldap_bridge', TRUE);

		$bridges = Array();
		if (!empty($bridge_links)) {
			foreach($bridge_links as $bridge_info) {
				$bridges[$bridge_info['sort_order']] = $bridge_info['minorid'];
			}
		}

		foreach ($bridges as $sort_order => $bridgeid) {
			$bridge = $GLOBALS['SQ_SYSTEM']->am->getAsset($bridgeid);
			if (is_null($bridge)) continue;

			// if status is not live then skip this bridge
			if ($bridge->status != SQ_STATUS_LIVE) continue;

			$entry = $this->_getLdapEntries($username, $bridge);
			if (!$entry || ($entry['count'] == 0)) {
				continue;
			}
			if (isset($entry[0])) {
				$entry = $entry[0];
				$user_type = $bridge->attr('user_type');
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($user_type);
				$user = new $user_type($bridge->id, $entry);
				break;
			}
		}

		return $user;

	}//end authenticateHttpUser()


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
		$user = NULL;
		$bridge_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_SC_LINK_BACKEND_NAV, 'ldap_bridge', TRUE);

		$bridges = Array();
		if (!empty($bridge_links)) {
			foreach($bridge_links as $bridge_info) {
				$bridges[$bridge_info['sort_order']] = $bridge_info['minorid'];
			}
		}

		foreach ($bridges as $sort_order => $bridgeid) {
			$bridge = $GLOBALS['SQ_SYSTEM']->am->getAsset($bridgeid);
			if (is_null($bridge)) continue;

			// if status is not live then skip this bridge
			if ($bridge->status != SQ_STATUS_LIVE) continue;

			$entry = $this->_getLdapEntries($username, $bridge);
			if (!$entry || ($entry['count'] == 0)) {
				continue;
			}
			if (isset($entry[0])) {
				$entry = $entry[0];
				$user_type = $bridge->attr('user_type');
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($user_type);
				$user = new $user_type($bridge->id, $entry);
				break;
			}
		}

		return $user;

	}//end locateUser()

	/**
	* Registers that an invalid login attempt has been made
	*
	* @param object	&$user	the user account upon which an invalid login attempt was made
	*
	* @return void
	* @access private
	*/
	function _registerInvalidLogin(&$user)
	{
		$username = $user->attr('dn');
		if (!isset($_SESSION['user_login_attempts'])) {
			$_SESSION['user_login_attempts'] = Array();
		}
		if (empty($_SESSION['user_login_attempts'][$username])) {
			$_SESSION['user_login_attempts'][$username] = 1;
		} else {
			$_SESSION['user_login_attempts'][$username]++;
		}

		if ($_SESSION['user_login_attempts'][$username] >= SQ_CONF_MAX_LOGIN_ATTEMPTS) {
			if (!is_null($user) && $user->canLogin()) {
				// log a message so we know someone has tried to log in too many times
				$ms =& $GLOBALS['SQ_SYSTEM']->getMessagingService();
				$msg_reps = Array(
								'asset_name'	=> $user->name,
								'user_name'		=> $username,
								'num_attempts'	=> (int)SQ_CONF_MAX_LOGIN_ATTEMPTS,
							);
				$log = $ms->newMessage(Array(), 'system.security.login.ldap', $msg_reps);
				$log->subject = translate('User Account Access Warning');
				$log->body    = translate('LDAP User "%asset_name%" (username: %user_name%) has had their account accessed unsuccessfully %num_attempts% times in a session');

				$log->parameters['remote_addr'] = $_SERVER['REMOTE_ADDR'];
				$log->parameters['sessionid'] = session_id();
				$log->send();
			}
		}

	}//end _registerInvalidLogin()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		return Array(
				SQ_LINK_TYPE_1	=> Array(
									'ldap_bridge'	=> Array(
														'card'		=> 'M',
														'exclusive'	=> FALSE,
													   ),
								   ),
			   );

	}//end _getAllowedLinks()


	/**
	* Returns the results of a search for a given username on the supplied bridge
	*
	* @param string $username    The username to search for
	* @param object &$bridge     The bridge to search for the user in
	* @param object &$ldap_conn  LDAP connection to use, if not supplied, create a new one from the bridge.
	*                            Prevents opening multiple connections when we need to continue working with one
	*
	* @return array
	* @access private
	*/
	function &_getLdapEntries($username, &$bridge, &$ldap_conn=NULL)
	{
		$null = NULL;
		$ldap = $ldap_conn ? $ldap_conn : $bridge->connectToLdap();
		// the attribute mappings to the LDAP schemas
		$attrs = $bridge->attr('attributes');

		if (empty($attrs['uid']) || is_null($ldap)) return $null;

		// we do the search instead of binding directly as the user because ldap->connect_as_user requires
		// the full dn as the first arg and doing it this way doesn't limit you to having all the users in the
		// same organisational unit in the ldap directory
		if (trim($bridge->attr('auth_dn')) !== '') {
			$auth_base_dn = $bridge->attr('auth_dn');
		} else {
			$auth_base_dn = $bridge->attr('root_dn');
		}

		// See #4860 Can't authenticate LDAP username with brackets
		$username = $ldap->escape_filter_value($username);
		// Check for extra LDAP filtering
		$auth_filter = trim($bridge->attr('auth_filter'));
		if (!empty($auth_filter)) {
			// Add surrounding parentheses only if the filter does not have them.
			// If the filter already have surrounding parentheses, adding them will
			// cause invalid filter, e.g. ((o=Squiz)) is an invalid filter
			if ($auth_filter[0] != '(') {
				$auth_filter = '('.$auth_filter.')';
			}
			$filter = '(&'.$auth_filter.'('.$attrs['uid'].'='.$username.'))';
		} else {
			$filter = '('.$attrs['uid'].'='.$username.')';
		}//end if

		$required_attrs = array_merge(
			array_keys(array_get_index($bridge->attr('user_attributes'), 'show', Array())),
			array_values($attrs),
			Array('objectclass')
		);
		$result = $ldap->searchEntries($auth_base_dn, $filter, NULL, TRUE, TRUE, $required_attrs);

		return $result;

	}//end _getLdapEntries()


}//end class

?>
