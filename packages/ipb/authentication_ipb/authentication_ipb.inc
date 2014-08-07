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
* $Id: authentication_ipb.inc,v 1.12 2012/08/30 00:58:55 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/authentication/authentication/authentication.inc';
require_once SQ_PACKAGES_PATH.'/ipb/ipb.inc';
require_once SQ_FUDGE_PATH.'/general/www.inc';

/**
* Authentication_IPB
*
* Purpose
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage ipb
*/
class Authentication_IPB extends Authentication
{

	/**
	* Weather the asset is deprecated or not.
	* this will be used in asset_map.inc:_cloneAsset() to see if the asset type
	* can be created (cloned) on the system
	*/
	public $deprecated = TRUE;


	/**
	* Marking the asset deprecated since so that we can
	* use it when warning the users on backend
	*/
	public $deprecated_since = '5.1';


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Authentication_IPB($assetid=0)
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
	* @return mixed object|NULL
	* @access public
	*/
	function &authenticateUser($username, $password)
	{
		$user = NULL;

		$bridges = $GLOBALS['SQ_SYSTEM']->am->getChildren($this->id, 'ipb_bridge', TRUE);
		foreach ($bridges as $bridgeid => $type_code) {

			$bridge =& $GLOBALS['SQ_SYSTEM']->am->getAsset($bridgeid);
			if (is_null($bridge)) continue;

			// if status is not live then skip this bridge
			if ($bridge->status != SQ_STATUS_LIVE) continue;

			$ipb =& $bridge->getIpbConn();

			$email_login = FALSE;
			if (valid_email($username)) $email_login = TRUE;

			if (!$this->attr('allow_banned')) {
				// check banned group option
				$result = $ipb->canViewBoard($username);
				if (empty($result)) return $user;
				$can_view = intval($result['g_view_board']);
				if (!$can_view) return $user;
			}
			$result = $ipb->authenticateUser($bridge, $username, $password, $email_login);
			if ($result) {
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('ipb_user');
				$data = $ipb->getMemberInfoByName($username);
				$data['id'] = 'u_'.$data['id'];
				$user = new Ipb_User($bridge->id, $data);
				return $user;
			}
		}

		// if we are here, we could not find the user
		$null = NULL;
		return $null;

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
		$bridges = $GLOBALS['SQ_SYSTEM']->am->getChildren($this->id, 'ipb_bridge', TRUE);
		foreach ($bridges as $bridgeid => $type_code) {
			$bridge =& $GLOBALS['SQ_SYSTEM']->am->getAsset($bridgeid);
			if (is_null($bridge)) continue;

			// if status is not live then skip this bridge
			if ($bridge->status != SQ_STATUS_LIVE) continue;

			$ipb =& $bridge->getIpbConn();
			$data = $ipb->getMemberByName($username);
			if (empty($data)) continue;

			$GLOBALS['SQ_SYSTEM']->am->includeAsset('ipb_user');
			$user = new Ipb_User($bridge->id, $data);
			break;
		}
		return $user;

	}//end authenticateHttpUser()


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
		$username = $user->attr('name');
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
				$log = $ms->newMessage(Array(), 'system.security.login.ipb', $msg_reps);
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
									'ipb_bridge'	=> Array(
														'card'		=> 'M',
														'exclusive'	=> FALSE,
													   ),
								   ),
			   );

	}//end _getAllowedLinks()


}//end class

?>
