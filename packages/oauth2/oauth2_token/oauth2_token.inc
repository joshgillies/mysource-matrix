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
* $Id: oauth2_token.inc,v 1.1.1.1.2.2 2013/08/06 02:53:57 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_PACKAGES_PATH.'/oauth2/lib/oauth2_client.inc';

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
class Oauth2_Token extends Page
{
	
	var $client = null;
	var $client_id = null;
	var $client_secret = null;
	var $authorization_endpoint = null;
	var $token_endpoint = null;
	var $scope = null;
	var $redirect_uri = null;
	
	var $auth_type = null;

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
			trigger_error(translate('oauth2_global_token_not_shared'), E_USER_ERROR);
		    
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
		trigger_error(translate('oauth2_permission_get_token'), E_USER_ERROR);
	    }
	    // no access for public user, if it's stored per user in DB. because public user is not identifiable
	    $token_storage = $this->attr('token_storage');
	    if($token_storage === 'user') {
		$current_user = $GLOBALS['SQ_SYSTEM']->user;
		if(is_null($current_user) ||  $current_user instanceof Public_User) {
		    $GLOBALS['SQ_SYSTEM']->paintLogin(translate('login'), translate('oauth2_public_user_save_token'));
		    return;
		}
	    }

	    if($force && !isset($_GET['code'])){
		 // global token storage would require write access
		 if($token_storage === 'global' && !$this->writeAccess('attributes')) {
		    trigger_error(translate('oauth2_permission_save_token'), E_USER_ERROR);
		 }
		// forget about existing token and start again
		$this->removeStoredToken();
		 // step 1 get auth code
		$this->_startAuthenticationRedirect();
	    }
		
	    // get stored token
	    $token = $this->loadTokenFromStorage();
	    
	    // no stored token?
	    if(empty($token)) {
		if(!isset($_GET['code'])) {
		    // step 1 get auth code
		    $this->_startAuthenticationRedirect();
		}
		else {
		     // step 2 exchange the auth code for access token
		    // verify this response is from original request by comparing state param
		    If(!isset($_SESSION['SQ_OAUTH2_CLIENT_STATE']) ||  !isset($_GET['state']))
			trigger_error(translate('oauth2_state_param_invalid'), E_USER_ERROR);
		    if($_SESSION['SQ_OAUTH2_CLIENT_STATE'] !== $_GET['state'])
			trigger_error(translate('oauth2_state_param_invalid'), E_USER_ERROR);

		    $params = array('code' => $_GET['code'], 'redirect_uri' => $this->redirect_uri);
		    $response = $this->client->getAccessToken($this->token_endpoint, 'authorization_code', $params);
		  
		    // global token storage would require write access
		    if($token_storage === 'global' && !$this->writeAccess('attributes')) {
			trigger_error(translate('oauth2_permission_save_token'), E_USER_ERROR);
		    }
		    
		    // parse and store access token in session
		    $token = $this->_parseStoreAccessToken($response);
		}
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
			$this->_startAuthenticationRedirect();
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
	 * Start authentication redirect flow
	 * 
	 * @return array
	 * @access protected
	 */
	protected function _startAuthenticationRedirect()
	{
	    // generate a random state to verify if the oauth callsback is original
	    $STATE = substr(str_shuffle("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"), 0, 10);
	    $_SESSION['SQ_OAUTH2_CLIENT_STATE'] = $STATE;
	
	    $auth_url = $this->client->getAuthenticationUrl($this->authorization_endpoint, $this->redirect_uri, Array('scope' => $this->scope, 'state' => $STATE));
	    header('Location: ' . $auth_url);
	    die('Redirect');
	}// end _startAuthenticationRedirect
	
	
	
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
		trigger_error(translate('oauth2_no_access_token_response'), E_USER_ERROR);
	    }	

	    if(is_array($response['result'])) {
		$info = $response['result'];
	    }
	    else {
		 parse_str($response['result'], $info);
	    }
	    if(!isset($info['access_token'])) {
		log_dump($response);
		trigger_error(translate('oauth2_no_access_token_response'), E_USER_ERROR);
	    }
	    
	    // store access token
	    $token = Array();
	    $token['access_token'] = $info['access_token'];
	    if(isset($info['expires']))
		$expire = $info['expires'];
	    if(isset($info['expires_in']))
		$expire = $info['expires_in'];
	    if(isset($expire))
		$token['expires_at'] = time() + $expire;
	     if(isset($info['refresh_token']))
		 $token['refresh_token'] = $info['refresh_token'];
	    $this->saveTokenToStorage($token);
	    
	    return ($token);
	    
	}// end _parseStoreAccessToken
	
	
	/**
	 * Load the stored access token
	 * 
	 * @return array
	 * @access public
	 */
	public function loadTokenFromStorage()
	{
	    $storage = $this->attr('token_storage');
	    $token = NULL;
	    if($storage === 'session') {
	    if(!isset($_SESSION[SQ_SESSION_SANDBOX_INDEX]['oauth2_'.$this->id.'_access_token'])) return Array();
		$token['access_token'] = $_SESSION[SQ_SESSION_SANDBOX_INDEX]['oauth2_'.$this->id.'_access_token'];
		$token['expires_at'] = isset($_SESSION[SQ_SESSION_SANDBOX_INDEX]['oauth2_'.$this->id.'_expires_at']) ? $_SESSION[SQ_SESSION_SANDBOX_INDEX]['oauth2_'.$this->id.'_expires_at'] : NULL;
		$token['refresh_token'] = isset($_SESSION[SQ_SESSION_SANDBOX_INDEX]['oauth2_'.$this->id.'_refresh_token']) ? $_SESSION[SQ_SESSION_SANDBOX_INDEX]['oauth2_'.$this->id.'_refresh_token'] : NULL;
	    }
	     else if ($storage === 'global') {
		$token = $this->attr('global_access_token');
	     }
	     else if ($storage === 'user') {
		 // DAL query
		$bind_vars = Array(
						'assetid'      => $this->id,
						'userid'      => $GLOBALS['SQ_SYSTEM']->currentUserId(),
					    );
		$result = MatrixDAL::executeAssoc('oauth2_package', 'getToken', $bind_vars);
		if(isset($result[0])) {
		    $token['access_token'] =$result[0]['access_token'];
		    $token['expires_at'] =strtotime($result[0]['expires_at']);
		    $token['refresh_token'] =$result[0]['refresh_token'];
		}
	     }
	     
	    return $token;
	}//end loadTokenFromStorage()
	
	/**
	 * Save the access token to storage
	 * @param array	$token	the access token array
	 * 
	 * @return bool
	 * @access public
	 */
	public function saveTokenToStorage($token)
	{
	    $storage = $this->attr('token_storage');
	    if($storage === 'session') {
		$_SESSION[SQ_SESSION_SANDBOX_INDEX]['oauth2_'.$this->id.'_access_token'] = $token['access_token'];
		if(isset($token['expires_at']))
		    $_SESSION[SQ_SESSION_SANDBOX_INDEX]['oauth2_'.$this->id.'_expires_at'] = $token['expires_at'];
		if(isset($token['refresh_token']))
		    $_SESSION[SQ_SESSION_SANDBOX_INDEX]['oauth2_'.$this->id.'_refresh_token'] = $token['refresh_token'];
	    }
	    else if ($storage === 'global') {
		//  write access to this asset should be checked earlier
		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
		$this->setAttrValue('global_access_token', $token);
		$this->saveAttributes();
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
	    }
	    else if ($storage === 'user') {
		
		// remove per user token first
		$bind_vars = Array(
						'assetid'	=>  $this->id,
						'userid'      => $GLOBALS['SQ_SYSTEM']->currentUserId(),
					    );
		$result = MatrixDAL::executeQuery('oauth2_package', 'deleteToken', $bind_vars);
		
		 // add new token
		$bind_vars = Array(
						'assetid'      => $this->id,
						'userid'      => $GLOBALS['SQ_SYSTEM']->currentUserId(),
						'access_token'	=> $token['access_token'],
						'expires_at'		=> isset($token['expires_at']) ? ts_iso8601($token['expires_at']) : NULL,
						'refresh_token'	=> isset($token['refresh_token']) ? $token['refresh_token'] : NULL,
						'created'		=> ts_iso8601(time()),
					    );
		$result = MatrixDAL::executeQuery('oauth2_package', 'addToken', $bind_vars);
	     }
	     return TRUE;
	}//end saveTokenToStorage
	
	
	/**
	 * Remove existing token for current user
	 * 
	 * @return bool
	 * @access public
	 */
	public function removeStoredToken()
	{
	    
	    $storage = $this->attr('token_storage');
	    if($storage === 'user') {
		// remove per user token first
		$bind_vars = Array(
						'assetid'	=>  $this->id,
						'userid'      => $GLOBALS['SQ_SYSTEM']->currentUserId(),
					    );
		$result = MatrixDAL::executeQuery('oauth2_package', 'deleteToken', $bind_vars);
		return $result;
	    }
	    else {
		$this->removeAllStoredToken();
	    }
	    return TRUE;	    
	}//end removeStoredToken()
	
	
	
	/**
	 * Remove all tokens stored for this asset
	 * 
	 * @return bool
	 * @access public
	 */
	public function removeAllStoredToken()
	{
	    // remove session token
	    unset($_SESSION[SQ_SESSION_SANDBOX_INDEX]['oauth2_'.$this->id.'_access_token']);
	    if(isset($_SESSION[SQ_SESSION_SANDBOX_INDEX]['oauth2_'.$this->id.'_expires_at']))
		    unset($_SESSION[SQ_SESSION_SANDBOX_INDEX]['oauth2_'.$this->id.'_expires_at']);
	    if(isset($_SESSION[SQ_SESSION_SANDBOX_INDEX]['oauth2_'.$this->id.'_refresh_token']))
		    unset($_SESSION[SQ_SESSION_SANDBOX_INDEX]['oauth2_'.$this->id.'_refresh_token']);
	    
	    // remove global token
	    //  write access to this asset should be checked earlier
	    $GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
	    $this->setAttrValue('global_access_token', Array());
	    $this->saveAttributes();
	    $GLOBALS['SQ_SYSTEM']->restoreRunLevel();
	    
	    
	    // remove all per user tokens
	    $bind_vars = Array(
					    'assetid'	=>  $this->id,		    
					);
	    $result = MatrixDAL::executeQuery('oauth2_package', 'deleteAllTokens', $bind_vars);
	    return $result;
	}//end removeAllStoredToken()
	
	
}//end class
?>
