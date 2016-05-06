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
* $Id: oauth2_token.inc,v 1.3 2013/08/06 02:52:57 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_PACKAGES_PATH.'/oauth2/lib/oauth2_client.inc';
require_once SQ_PACKAGES_PATH.'/oauth2/oauth2_token/oauth2_token.inc';

/**
* Oauth2 Token asset
* Handles Oauth 2.0 configuration, authentication and storage/access of Access Token
* It supports automatic refresh of Access Token upon expiry if Refresh Token is given.
* Access token can be stored per user in session, globally in DB or per user in DB.
*
*
* @author  Edison Wang <ewang@squiz.com.au>
* @version $ $
* @package MySource_Matrix_Packages
* @subpackage oauth2
*/
class Oauth2_Two_Legged_Token extends Oauth2_Token
{

	


	/**
	 * Get the access token.
	 * If not present, start authentication. It's present, return it.
	 * @param bool	$force	force authentication to get a new acces token
	 *
	 * @return array
	 * @access public
	 */
	public function getAccessToken ($force = FALSE)
	{

	    // check read access
	    if (!$this->readAccess()) {
		trigger_error(translate('You do not have read permission to get the Access Token'), E_USER_ERROR);

	    }
	    // no access for public user, if it's stored per user in DB. because public user is not identifiable
	    $token_storage = $this->attr('token_storage');
	    if($token_storage === 'user') {
		$current_user = $GLOBALS['SQ_SYSTEM']->user;
		if(is_null($current_user) ||  $current_user instanceof Public_User) {
		    $GLOBALS['SQ_SYSTEM']->paintLogin(translate('Login'), translate('Access Token can not be stored for a Public User'));


		    return;
		}
	    }

	    // get stored token
	    $token = $this->loadTokenFromStorage();

	    // no stored token?
	    if($force || empty($token)) {
		    $response = $this->client->getAccessToken($this->token_endpoint, 'client_credentials', Array());

		    // global token storage would require write access
		    if($token_storage === 'global' && !$this->writeAccess('attributes')) {
			trigger_error(translate('You do not have write permission to save the Access Token'), E_USER_ERROR);

		    }

		    // parse and store access token in session
		    $token = $this->_parseStoreAccessToken($response);
	    }


	    // refresh token / get a new token if stored token is expired
	    if(isset($token['expires_at'])) {
		$expire = $token['expires_at'];
		// if it will expire within next 10 seconds, try refresh it
		if($expire < time() +10) {
		    // if it expired, do we have a refresh token?
		    if(isset($token['refresh_token']) && !empty($token['refresh_token'])) {
			$params = array('refresh_token' => $token['refresh_token']);
			// refresh the access token
			$response = $this->client->getAccessToken($this->token_endpoint, 'refresh_token', $params);

			// keep the previous refresh token if no new refresh token issued
			if(!isset($response['result']['refresh_token']) || empty($response['result']['refresh_token']))
			    $response['result']['refresh_token'] = $token['refresh_token'];

			// parse and store the new access token
			// since we are only refreshing the existing token, no need to worry about permission
			$token = $this->_parseStoreAccessToken($response);
		    }
		    else {
				// no refresh token
				// just start the auth again to get new access token
				if($token_storage !== 'global') {
					$this->getAccessToken(TRUE);
				}{
					trigger_error('Access Token has expired and there is no Refresh Token available.');
				}
		    }
		}
	    }

	    return $token;
	}// end getAccessToken


}//end class
?>
