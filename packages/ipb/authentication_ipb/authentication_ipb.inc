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
* $Id: authentication_ipb.inc,v 1.2.2.1 2006/02/16 03:33:58 skim Exp $
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
* @version $Revision: 1.2.2.1 $
* @package MySource_Matrix_Packages
* @subpackage ipb
*/
class Authentication_IPB extends Authentication
{


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
	* @return mixed object|null
	* @access public
	*/
	function &authenticateUser($username, $password)
	{
		$user = NULL;

		$bridges = $GLOBALS['SQ_SYSTEM']->am->getChildren($this->id, 'ipb_bridge', TRUE);
		foreach ($bridges as $bridgeid => $type_code) {

			$bridge =& $GLOBALS['SQ_SYSTEM']->am->getAsset($bridgeid);
			if (is_null($bridge)) continue;

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
				$user =& new Ipb_User($bridge->id, $data);
				return $user;
			}
		}

		// if we are here, we could not find the user
		$null = NULL;
		return $null;

	}//end authenticateUser()


	/**
	* If this authentication has a user matching the username in $_SERVER['REMOTE_USER'], return that user
	*
	* @return mixed object|null
	* @access public
	*/
	function &authenticateHttpUser()
	{
		$user = NULL;
		$bridges = $GLOBALS['SQ_SYSTEM']->am->getChildren($this->id, 'ipb_bridge', TRUE);
		foreach ($bridges as $bridgeid => $type_code) {
			$bridge =& $GLOBALS['SQ_SYSTEM']->am->getAsset($bridgeid);
			if (is_null($bridge)) continue;

			$ipb =& $bridge->getIpbConn();
			$data = $ipb->getMemberByName($_SERVER['REMOTE_USER']);
			if (empty($data)) continue;

			$GLOBALS['SQ_SYSTEM']->am->includeAsset('ipb_user');
			$user =& new Ipb_User($bridge->id, $data);
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
