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
*
*/


require_once SQ_PACKAGES_PATH.'/oauth2/oauth2_token/oauth2_token.inc';
require_once SQ_PACKAGES_PATH.'/oauth2/lib/oauth2_client.inc';

/**
* Oauth2 Token asset
* Handles Oauth 2.0 configuration, authentication and storage/access of Access Token
* It supports automatic refresh of Access Token upon expiry if Refresh Token is given.
* Access token can be stored per user in session, globally in DB or per user in DB.
*
*
* @author  Edison Wang <ewang@squiz.com.au>
* @package MySource_Matrix_Packages
* @subpackage sugar
*/
class SugarCRM_Token extends Oauth2_Token
{

	var $client = null;
	var $client_id = null;
	var $client_secret = null;
	var $authorization_endpoint = null;
	var $token_endpoint = null;
	var $scope = null;
	var $redirect_uri = null;

	var $auth_type = null;

	var $api_version_url_suffix = Array('v10' => 'rest/v10/oauth2/token');

	/**
	* Wakeup.
	*
	* It's important to load components when restored from a serialised
	* state by Deja Vu.
	*
	* @access public
	*/
	function __wakeup()
	{
		// don't cache any config
		$this->__construct();
	}//end __wakeup()


	/**
	* Constructor
	*
	* @param string	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

		// init configure vars
		$this->client_id = $this->attr('client_id');
		$this->client_secret = $this->attr('client_secret');
		$this->authorization_endpoint = $this->attr('authorization_endpoint');
		$this->token_endpoint = $this->attr('token_endpoint');

		// append the token request url suffix
		$api_version = $this->attr('api_version');
		if(isset($this->api_version_url_suffix[$api_version])) {
			$token_endpoint_suffix = $this->api_version_url_suffix[$api_version];
			if(substr($this->token_endpoint, -1) == '/') {
				$this->token_endpoint .= $token_endpoint_suffix;
			}
			else {
				$this->token_endpoint .= '/'.$token_endpoint_suffix;
			}
		}

		$this->scope = $this->attr('scope');
		$redirect_uri = $this->attr('redirect_uri');
		$this->redirect_uri = empty($redirect_uri) ?  current_url(TRUE, TRUE) : $redirect_uri;
		$auth_type = $this->attr('authorization_type');
		$this->auth_type = constant('Oauth2_Client::AUTH_TYPE_'. strtoupper($auth_type));

		$this->client = new Oauth2_Client($this->client_id, $this->client_secret, $this->auth_type, $this->attr('verify_ssl_peer'));

	}//end constructor


	 /**
	* Outputs the requested resource(s).
	*
	* @return void
	* @access public
	*/
	public function printFrontend()
	{
		// don't let browser/proxy cache this asset
		header('Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0');
		header('Pragma: no-cache');
		header('Expires: '.gmdate('D, d M Y H:i:s', time()-3600).' GMT');

		// start authentication (if required)
		$token = $this->getAccessToken(isset($_GET['force']));


		// output the access token in json format (if required)
		if(isset($_GET['output'])) {
		    // but not if it's global token, too risky to share token
		    $token_storage = $this->attr('token_storage');
		    if($token_storage === 'global')
			trigger_error(translate('Globally stored Access Token can not be displayed'), E_USER_ERROR);


		    header('Content-Type: application/json');
		    echo (json_encode_array($token));
		}


		return parent::printFrontend();
	}//end printFrontend()




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

	    // some basic error checks
	    $username = $this->attr('username');
	    $platform = $this->attr('platform');
	    $token_endpoint = $this->attr('token_endpoint');
	    $client_id = $this->attr('client_id');

	    if(empty($username) || empty($platform) ||  empty($token_endpoint) || empty($client_id)) {
	    	trigger_error(translate('You must specify Username, Platform, Client ID and SugarCRM Host URL.'));
	    	return FALSE;
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

	    if($force){
			// forget about existing token and start again
			$this->removeStoredToken();
	    }

	    // get stored token
	    $token = $this->loadTokenFromStorage();

	    // no stored token?
	    if(empty($token)) {
		    $params = array('username' => $this->attr('username'), 'password' => $this->attr('password'), 'platform' => $this->attr('platform'));
		    try {
		    	$response = $this->client->getAccessToken($this->token_endpoint, 'password', $params);
			}
			catch (Exception $e) {
				trigger_error(translate('Failed to get Access Token: ').$e->getMessage(), E_USER_ERROR);
			}

		    // parse and store access token in session
		    $token = $this->_parseStoreAccessToken($response);
	    }


	    // refresh token / get a new token if stored token is expired
	    if(isset($token['expires_at'])) {
		$expire = $token['expires_at'];
		// if it will expire within next 30 seconds, try refresh it
		if($expire < time() + 10) {
		    // if it expired, do we have a refresh token?
		    if(isset($token['refresh_token']) && !empty($token['refresh_token']) && isset($token['refresh_token_expires_at']) && ($token['refresh_token_expires_at'] >= time() + 10)) {
			$params = array('refresh_token' => $token['refresh_token']);
			// refresh the access token
			try {
				$response = $this->client->getAccessToken($this->token_endpoint, 'refresh_token', $params);
			}
			catch (Exception $e) {
				trigger_error(translate('Failed to use Refresh Token to get new Access Token: ').$e->getMessage(), E_USER_ERROR);
			}
			// keep the previous refresh token if no new refresh token issued
			if(!isset($response['result']['refresh_token']) || empty($response['result']['refresh_token']))
			    $response['result']['refresh_token'] = $token['refresh_token'];

			// parse and store the new access token
			// since we are only refreshing the existing token, no need to worry about permission
			$token = $this->_parseStoreAccessToken($response);
		    }
		    else {
			// no refresh token (or expired refresh token)
			// just start the password auth again to get new access token
			$this->getAccessToken(TRUE);
		    }
		}
	    }

	    return $token;
	}// end getAccessToken



	/**
	 * Call the REST API with the stored token
	 *
	 * @param string	$url	the REST API to call
	 *
	 * @return array
	 * @access public
	 */
	public function fetch ($url, $force=TRUE)
	{
	    $token = $this->getAccessToken($force);
	    $this->client->setAccessToken($token['access_token']);
	    $this->client->setAccessTokenParamName($this->attr('access_token_param_name'));

	    $type = $this->attr('access_token_type');
	    $this->client->setAccessTokenType(constant('Oauth2_Client::ACCESS_TOKEN_'.strtoupper($type)));

	    $response = $this->client->fetch($url);
	    return $response;
	}





	/**
	 * Parse the response data of get access token request to extract the access token
	 * @param array	$response	the response of get access token request
	 *
	 * @return array
	 * @access public
	 */
	protected function _parseStoreAccessToken($response = Array())
	{
	    if(!isset($response['result'])) {
		log_dump($response);
		trigger_error(translate('There is no Access Token in response'), E_USER_ERROR);

	    }

	    if(is_array($response['result'])) {
		$info = $response['result'];
	    }
	    else {
		 parse_str($response['result'], $info);
	    }
	    if(!isset($info['access_token'])) {
		log_dump($response);
		trigger_error(translate('There is no Access Token in response'), E_USER_ERROR);

	    }

	    // store access token
	    $token = Array();
	    $token['access_token'] = $info['access_token'];
	    if(isset($info['expires']))
			$expire = $info['expires'];
	    if(isset($info['expires_in']))
			$expire = $info['expires_in'];

		// for social media network Yammer, the response is slightly different
		if(is_array($info['access_token'])) {
			if(isset($info['access_token']['token'])) {
				$token['access_token'] = $info['access_token']['token'];
			}
			if(isset($info['access_token']['expires_at'])) {
				$expire = $info['access_token']['expires_at'];
			}
		}


	    if(isset($expire))
			$token['expires_at'] = time() + $expire;
	    if(isset($info['refresh_token']))
			$token['refresh_token'] = $info['refresh_token'];


	    if(isset($info['refresh_expires_in'])) {
	    	$token['refresh_token_expires_at'] = time() + $info['refresh_expires_in'];
	    }


	    if(isset($info['download_token']))
			$token['download_token'] = $info['download_token'];


	    $this->saveTokenToStorage($token);

	    return ($token);

	}// end _parseStoreAccessToken




}//end class
?>
