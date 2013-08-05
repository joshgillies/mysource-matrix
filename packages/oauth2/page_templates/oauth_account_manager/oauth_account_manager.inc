<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: oauth_account_manager.inc,v 1.1.1.1.2.1 2013/08/05 02:00:52 ewang Exp $
*
*/
require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_asset_builder/page_asset_builder.inc';
require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_DATA_PATH.'/private/conf/tools.inc';

/**
* Oauth Account Manager
*
* Purpose
*
*
* @author  Edison Wang <ewang@squiz.com.au>
* @version $Revision: 1.1.1.1.2.1 $
* @package MySource_Matrix_Packages
* @subpackage oauth2
*/
class Oauth_Account_Manager extends Page_Asset_Builder
{

	
	var $oauth_token;
	var $oauth_user_id;
	var $oauth_user_profile;
	
	
	/**
	* Constructor
	*
	* @param string	$assetid    the asset id to be loaded
	* @param Array	$data		the shadow asset data that will construct this variety
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);
		$this->oauth_token = $this->_getSpecialAsset('oauth_token_source');
	}//end constructor

	
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
		$this->oauth_user_id = null;
		$this->oauth_user_profile = null;
		$this->oauth_token = $this->_getSpecialAsset('oauth_token_source');
		
	}//end __wakeup()
	
	
	/**
	* Determine whether this asset should send cacheable header
	*
	*
	* @return boolean
	* @access public
	*/
	public function canSendCacheableHeader()
	{
		return FALSE;
	}//end canSendCacheableHeader()
	
	
	
	/**
	* Prints out the Frontend for this asset
	*
	* @return void
	* @access public
	*/
	function printFrontend()
	{

		// set global redirect URL
		if(!isset($_SESSION['sq_oauth_account_manager_current_state_redirect_url'])) {
		    $redirect_url = $this->_getRedirectURL();
		    if(!empty($redirect_url)) 
			$_SESSION['sq_oauth_account_manager_current_state_redirect_url'] = $redirect_url;
		}
	
		// check MUST have configs
		if(empty($this->oauth_token)) {
		    trigger_error(translate('oauth_account_manager_no_token'));
		    return;
		}
		$storage_type = $this->oauth_token->attr('token_storage');
		if($storage_type !== 'session') {
		    trigger_error(translate('oauth_account_manager_token_storage_incorrect'));
		    return;
		}
		$api_url = $this->attr('userid_api_url');
		if(empty($api_url)) {
		    trigger_error(translate('oauth_account_manager_no_profile_api'));
		    return;
		}
		    
		
		
		/*
		 *  special actions
		 */
			
		
		// link current matrix user to the authenticated oauth acount
		if(isset($_GET['linkAccounts'])) {
		    $allow_create_link = $this->attr('allow_link');
		    // logged in oauth user
		    if($allow_create_link) {
			// login oauth if not already did
			$this->_loginOauthUser();
			if($this->oauth_user_id) {
			    $current_user = $GLOBALS['SQ_SYSTEM']->user;
			    
			    //  prefill attributes
			    $this->_prefillUserAttributes($current_user);
		    
			    //  if current user is logged in matrix
			    if($current_user && !($current_user instanceof Public_User)) {
				// link those 2 accounts
				$this->_linkMatrixUser($current_user->id, $this->oauth_user_id);
				
				// redirect after linking
				if($this->attr('redirect_after_link')) {
				    if(isset($_SESSION['sq_oauth_account_manager_current_state_redirect_url']) && $_SESSION['sq_oauth_account_manager_current_state_redirect_url'])  {
					$redirect_url = $_SESSION['sq_oauth_account_manager_current_state_redirect_url'];
					unset($_SESSION['sq_oauth_account_manager_current_state_redirect_url']);
					do_redirect($redirect_url);
				    }
				}
				// stop further redirects
				unset($_SESSION['sq_oauth_account_manager_current_state_redirect_url']);
			    }
			}  
		    }
		}
		
	
		
		//  logout  user
		if(isset($_GET['logout'])) {
		    // log out of matrix first
		    $this->_logoutUser();
		    
		    // forget oauth token
		    $this->oauth_token->removeStoredToken();
		    
		    // redirect after logout
		    $redirect_url = $this->_getRedirectURL();
		    do_redirect($redirect_url);

		}
		
	
		//  move on....
		parent::printFrontend();

	}//end printFrontend()
	
	
	
	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{	
	    

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');

		$this->_current_state = '';
		$current_user = $GLOBALS['SQ_SYSTEM']->user;
		set_error_handler(Array(&$this, '_errorHandler'));
			$success = $this->_processGlobalActions();
		restore_error_handler();

		// create a friendly error message on error
		if (isset($_POST['AB_'.$this->id.'_ASSET_BUILDER_ACTION']) && !$success) {
			$this->_replacements[strtolower($_POST['AB_'.$this->id.'_ASSET_BUILDER_ACTION']).'_error'] = $this->attr(strtolower($_POST['AB_'.$this->id.'_ASSET_BUILDER_ACTION']).'_error');

			if (!empty($this->_errors)) {
				$errors = '';
				foreach ($this->_errors as $text) {
					$errors .= '<li>'.$text.'</li>';
				}
				$this->_replacements[strtolower($_POST['AB_'.$this->id.'_ASSET_BUILDER_ACTION']).'_error'] .= '<ul>'.$errors.'</ul>';
			}
		}

		// start performance mode timer
		$GLOBALS['SQ_SYSTEM']->pm->startTimer($this, 'printBody');
		
		// if a new asset is created, it will be stored in the tmp vars for use by other asset types
		// that extend this one, but we wont need it any more - so chuck it out, but first, load up
		// some replacements for the newly created asset
		if (isset($this->_tmp['created_asset'])) {
			$this->_loadAssetReplacements();
			unset($this->_tmp['created_asset']);
		}

		// If we don't allow linking oauth user to existing Matrix user
		// and if user already logged in, just do nothing
		$allow_create_link = $this->attr('allow_link');
		if(empty($this->_current_state) && !$allow_create_link) {
		    if(!is_null($current_user) && !($current_user instanceof Public_User))
			$this->_current_state = 'logged_in';
		}
		
		if (empty($this->_current_state)) {
			/* 
			 * Oauth authentication starts here
			 */
			 if(!$this->oauth_user_id)
			    $this->_loginOauthUser(TRUE);
			
			 
			// Oauth user should have been already authenticated at this point
			 // check if we have enough info about the user
			 if(empty($this->oauth_user_id) || empty($this->oauth_user_profile)) {
			    $this->_replacements['login_error'] = translate('oauth_account_manager_no_userid');
			    $this->_current_state = 'login_fail';
			 }
			 else {	
			    // try find linked Matrix user
			    $matrixUserid = $this->_searchLinkedMatrixUser($this->oauth_user_id);

			    // if the linked user doesn't exist, just remove it from DB
			    if($matrixUserid && !$GLOBALS['SQ_SYSTEM']->am->assetExists($matrixUserid)) {
			       $this-> _unlinkMatrixUser($matrixUserid, $this->oauth_user_id);
				$matrixUserid = NULL;
			    }

			     // Log in as the linked Matrix user
			    if ((is_null($current_user) || ($current_user instanceof Public_User)) && $matrixUserid) {
				if(!$this->_loginUser($matrixUserid)){
				    // login failed
				    $this->_replacements['login_error'] = translate('oauth_account_manager_login_failed');
				    $this->_current_state = 'login_fail';
				}
				else {
				    $current_user = $GLOBALS['SQ_SYSTEM']->user;
				}
			    }
			 }

			if(empty($this->_current_state)) {
			    // can't find a linked matrix user for the authenticated user, create a new matrix user
			    if (is_null($current_user) || ($current_user instanceof Public_User)) {
				    $auto_create = $this->attr('auto_create');
				    // automatically create the user asset, skipping the create user form, randomly generates username and password
				    if($auto_create) {
					    $create_type = $this->attr('create_type');
					    $create_type = array_pop(array_keys($create_type));
					    $this->_createUser($create_type);
				    }
				    else {
					$this->_current_state = 'create_user';
					$this->_replacements['login_invite'] = $this->getKeywordReplacement('login_invite');
					$this->_replacements['login_form'] = $this->getKeywordReplacement('login_form');
				    }

			    } else {
				    // if logged in as both Matrix user and oauth user
				    if($GLOBALS['SQ_SYSTEM']->currentUserId() == $matrixUserid) {
					// the oauth user is correctly linked to current matrix user
					$this->_current_state = 'logged_in';
				    }
				    else {
					// the oauth user is not linked to current logged in matrix user, create a link?
					 $this->_current_state = 'create_link';
				    }	
			    }
			}
		}
		
		// instead of displaying logged in body, do redirect
		if( $this->_current_state == 'logged_in' && $this->attr('redirect_after_login') && isset($_SESSION['sq_oauth_account_manager_current_state_redirect_url']) && $_SESSION['sq_oauth_account_manager_current_state_redirect_url'])  {
		    $redirect_url = $_SESSION['sq_oauth_account_manager_current_state_redirect_url'];
		    unset($_SESSION['sq_oauth_account_manager_current_state_redirect_url']);
		    do_redirect($redirect_url);
		}
				    

		$fn = '_load'.$this->_default_state.'Replacements';
		$this->_loadcreateReplacements();
		$this->$fn();

		$this->_replacements['user_id'] = $this->oauth_user_id;
		$this->_printBody();

		// stop performance mode timer
		$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this, 'printBody');
		
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

	}//end printBody()

	
	
	
	
	/**
	* Process any global Account Manager actions
	*
	* This function builds on the Asset Builder one by auto filling the username field of
	* the login form that may be printed once the user account has been created.
	*
	* @return boolean
	* @access private
	* @see Page_Asset_Builder::_processGlobalActions()
	*/
	function _processGlobalActions()
	{
	    
		// set to live status once created
		$this->vars['create_status']['value'] = SQ_STATUS_LIVE;
		
		$success = parent::_processGlobalActions();
		if (!$success) return FALSE;

		/*
		 * Log in the just created local user
		 */
		if($this->_current_state === 'created'  && !empty($this->_tmp['created_asset'])) {
		    // prefill attributes
		    // make sure we do have user profile data available, in case auto creation is not enabled, we have to call api again
		    $prefilled_fields = $this->attr('prefilled_fields');
		    if(empty($this->oauth_user_profile) && !empty($prefilled_fields[$this->_tmp['created_asset']->type()])) {
			$this->_loginOauthUser();
		    }
		    $this->_prefillUserAttributes($this->_tmp['created_asset']);

		    // link the created user with this oauth account
		    if(!empty($this->oauth_user_id))
			$this->_linkMatrixUser($this->_tmp['created_asset']->id, $this->oauth_user_id);
		    
		    // login user
		    $this->_loginUser($this->_tmp['created_asset']->id);
		    $this->_current_state = 'logged_in';
		    
		}
		
		return TRUE;

	}//end _processGlobalActions()
	
	
	
	/**
	* Prefill user attributes from user profile data
	*
	* @param object	$user	the user asset to prefill
	* 
	* @return bool
	* @access protected
	*/
	protected function _prefillUserAttributes ($user)
	{
		  $prefilled_fields = $this->attr('prefilled_fields');
		    if (!empty($prefilled_fields[$user->type()]) && !empty($this->oauth_user_profile) && !empty($user)) {
			    foreach ($prefilled_fields[$user->type()] as $attr => $location) {
				    // get the value to set from user profile
				    $prefill_value = $this->_getValueFromLocation($location, $this->oauth_user_profile);
				    if(is_null($prefill_value)) continue;
				    
				    $pos = strpos($attr, $this->getPrefix().'_metadata_');
				    if ($pos !== FALSE) {
					    // Process metadata
					    $mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
					    $metadata_field = substr($attr, strlen($this->getPrefix().'_metadata_'));
					    $GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
					    $mm->setMetadata($user->id, Array($metadata_field => Array('0' => Array('value' => $prefill_value))));
					    $mm->generateContentFile($user->id, TRUE);
					    $GLOBALS['SQ_SYSTEM']->restoreRunLevel();
				    } else {
					    // Process attributes
					    $attribute = $user->getAttribute($attr, TRUE);
					    if (is_null($attribute)) continue;
					    $GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
					    $user->setAttrValue($attr, $prefill_value);
					    $user->saveAttributes();
					    $GLOBALS['SQ_SYSTEM']->restoreRunLevel();
					    unset($attribute);

				    }//end else
			    }//end foreach

		    }//end if
		    return TRUE;
	}//end _prefillUserAttributes()
	
	
	/**
	* Search linked Matrix user by Oauth user id
	*
	* @param varchar	$oauth_userid	Oauth userid string
	* 
	* @return int | null
	* @access protected
	*/
	protected function _searchLinkedMatrixUser ($oauth_userid)
	{
		if (empty($oauth_userid))
		    return NULL;
		
		// DAL query
		$bind_vars = Array(
						'oauth_userid'      => $oauth_userid,
						'assetid'      => $this->id,
					    );
		$result = MatrixDAL::executeAll('oauth2_package', 'getUserid', $bind_vars);
		if(empty($result)) return NULL;
		
		return ($result[0]['matrix_userid']);
	}//end _searchLinkedMatrixUser()
	
	
	/**
	* Link Matrix user with specified Oauth account
	*
	* @param varchar	$matrix_userid	Matrix userid  string
	* @param varchar	$oauth_userid	Oauth userid string
	* 
	* @return int | null
	* @access protected
	*/
	protected function _linkMatrixUser ($matrix_userid, $oauth_userid)
	{
		if (empty($matrix_userid) || empty($oauth_userid) )
		    return NULL;
		
		// already linked?
		 $existingMatrixUserid = $this->_searchLinkedMatrixUser($oauth_userid);
		 if($existingMatrixUserid) {
		     $this->_unlinkMatrixUser($existingMatrixUserid, $oauth_userid);
		 }
		
		// insert new entry
		$bind_vars = Array(
						'matrix_userid'	=>  $matrix_userid,		    
						'oauth_userid'	=>  $oauth_userid,
						'assetid'	=> $this->id,
						'created'	=>  ts_iso8601(time()),
					    );
		$result = MatrixDAL::executeQuery('oauth2_package', 'addLink', $bind_vars);
		 return $result;
	}//end _linkMatrixUser()
	
	
	
	/**
	* Delete a link
	*
	* @param varchar	$matrix_userid	Matrix userid  string
	* @param varchar	$oauth_userid	Oauth userid string
	* 
	* @return int | null
	* @access protected
	*/
	protected function _unlinkMatrixUser ($matrix_userid, $oauth_userid)
	{
		if (empty($matrix_userid) || empty($oauth_userid) )
		    return NULL;
		 $bind_vars = Array(
					    'matrix_userid'	=>  $matrix_userid,
					    'oauth_userid'	=>  $oauth_userid,
					    'assetid'	=> $this->id,
					);
		$result = MatrixDAL::executeQuery('oauth2_package', 'deleteLink', $bind_vars);
		 return $result;
	}//end _unlinkMatrixUser()
	
	

	
	/**
	* Return the currently logged in Oauth user id.
	*
	* @param bool	$force	If we should force getting a new access token
	* 
	* @return int | null
	* @access protected
	*/
	protected function _loginOauthUser ($force = FALSE)
	{
		// call get user id API
		$response = $this->oauth_token->fetch($this->attr('userid_api_url'), $force);

		// set the user id and profile
		$this->oauth_user_profile = $response['result'];
		$userid_loc = $this->attr('userid_location');
		$this->oauth_user_id = $this->_getValueFromLocation( empty($userid_loc) ? 'id' : $userid_loc, $this->oauth_user_profile);
		
		// store profile data in session sandbox
		if(!empty($this->oauth_user_profile)) {
		    $_SESSION[SQ_SESSION_SANDBOX_INDEX]['oauth_profile'] = json_encode_array($this->oauth_user_profile);
		    $this->_store_results($this->oauth_user_profile);
		}
		return $this->oauth_user_id;
	}//end _loginOauthUser()
	
	
	
	/**
	* This function recursively checks an object properties and store them in session
	* The stored data can be printed via keywords %globals_session_oauth_profile_xx%
	*
	* @return void
	* @access private
	*/
	protected  function _store_results($object, $prefix = 'oauth_profile_')
	{
		if (empty($object)) return;

		$vars = is_object($object) ? get_object_vars($object) : $object;
		if (empty($vars)) return;

		foreach ($vars as $index => $value) {
			if (is_object($value) || is_array($value)) {
				$this->_store_results ($value, $prefix.$index);
			} else {
				$_SESSION[SQ_SESSION_SANDBOX_INDEX][$prefix.'_'.$index] = $value;
			}
		}

	}//end _store_results()
	
	
	/**
	* Return the value for the specified location in user profile array
	*
	* @param string	$location	the location string of element value, e.g id or hometown.id
	* @param array	$data	user profile data array
	* 
	* @return null
	* @access private
	*/
	private function _getValueFromLocation($location, $data)
	{
	    // if it's just the element in first level
	    if(isset($data[$location]) && is_scalar($data[$location])) return $data[$location];
	    if(strpos($location, '.') !== NULL) {
		$location_parts = explode('.', $location);
		foreach ($location_parts as $part) {
		    if(!isset($data[$part])) return NULL;
		    $data = $data[$part];
		    if(is_scalar($data)) {
			return $data;
		    }
		}
	    }
		
	    return NULL;
	}//end _getValueFromLocation()
	
	
	/**
	* Returns the asset (if any) which is noticed linked. e.g oauth_token_source, redirect_asset
	* @param varchar	$name	the link value of the special asset
	*
	* @return mixed object|NULL
	* @access protected
	*/
	protected function _getSpecialAsset($name)
	{
		$res = NULL;
		if ($this->useSystemVersion()) {
			$notice_links = unserialize(file_to_string($this->data_path.'/.sq_notice_links'));
			foreach ($notice_links as $link) {
				if ($link['value'] === $name) {
					break;
				}
			}
		} else {
			if (!empty($this->id)) {
				$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', TRUE, $name);
				
			}
		}
		if (!empty($link)) {
			$res = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		}

		return $res;

	}//end _getSpecialAsset()


	
	
	/**
	* get the URL that we will be redirecting to
	*
	* @return string
	* @access private
	*/
	function _getRedirectURL()
	{
		// See if we are given a query string value to look at
		$qrystr_param = $this->attr('use_querystring_parameter');
		$url = trim($this->attr('redirect_url'));
		if ($qrystr_param && isset($_GET[$qrystr_param]) && trim($_GET[$qrystr_param]) != '') {
			$url = trim($_GET[$qrystr_param]);

		// See if there is a URL entered manually in the redirect_url attribute field
		} else if ($url != '') {
			// Replace global keywords
			replace_global_keywords($url);

		// if there is no url, see if we have an asset selected
		} else {
			$redirect_asset = $this->_getSpecialAsset('redirect_asset');
			if (!is_null($redirect_asset)) {
				if (($url = $redirect_asset->getURL()) == '') {
					$url = current_url().'?a='.$redirect_asset->id;
				}
			}
		}//end if

		if ($url == '') {
			return '';
		}


		// Strip newlines (and other dodgy characters) from the $url that may have
		// been inadvertently added by keywords or modifiers. They're not allowed in
		// the 'Location' header and will cause an error.
		$stripchars = array();
		for ($i = 0; $i < 32; $stripchars[] = chr($i), $i++);
		$url = str_replace($stripchars, '', $url);

		return $url;

	}//end _getRedirectURL()
	
	
	
	
	/**
	* Add valid keywords for this asset to an array of keywords when asked
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								we add keywords to the $vars['keywords'] array
	*
	* @return boolean
	* @access private
	*/
	function onRequestKeywords(&$broadcaster, $vars=Array())
	{

		if (!isset($vars['keywords'])) return;

		// get type-code
		$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id, 'bodycopy', TRUE);
		$type_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy');
		$bodycopy_type = '';
		foreach ($type_links as $link_info) {
			if (isset($parents[$link_info['minorid']])) {
				$bodycopy_type = $link_info['value'];
			}
		}

		switch ($bodycopy_type) {
			case 'create_user' :
				$vars['keywords'] = array_merge($vars['keywords'], $this->_getNotLoggedInKeywords());
			break;
		    	case 'login_fail' :
				$vars['keywords'] = array_merge($vars['keywords'], Array('login_error' => translate('oauth_account_manager_login_error')));
			break;
		}
		
		$vars['keywords'] = array_merge($vars['keywords'], Array('user_id' => translate('oauth_account_manager_user_id')));

	}//end onRequestKeywords()
	
	
		
	

	
	/**
	* Return an array of bodycopies that need to be created
	*
	* @return array
	* @access private
	*/
	function _getBodycopies()
	{
		$res = Array();
		$res['create_user']= Array(
							'name'	=> translate('oauth_account_manager_create_user'),
							'content'	=> '%create_error% %login_error% %login_invite% %login_form% %create_form%',
						    );
		$res['create_link']= Array(
							'name'	=> translate('oauth_account_manager_create_link'),
							'content'	=> 'This account is not linked to current Matrix user. <a href="?linkAccounts">Link them now</a>.',
						 );
		$res['logged_in']= Array(
							'name'	=> translate('oauth_account_manager_logged_in'),
							'content'	=> 'logged in as %globals_user_name%',
						   );
		$res['login_fail']= Array(
							'name'	=> translate('oauth_account_manager_login_fail'),
							'content'	=> '%login_error%',
						   );
		return $res;

	}//end _getBodycopies()
	
	/**
	* Log in the Matrix user (and do redirects if required)
	* 
	* @param string	$userid	matrix user id
	* 
	* @return bool
	* @access protected
	*/
	protected function _loginUser($userid)
	{
	    $user =  $GLOBALS['SQ_SYSTEM']->am->getAsset($userid);
	    if($user && $user->canLogin()) {
		unset($_SESSION['SQ_RETURN_TO_HTTP']);
		$GLOBALS['SQ_SYSTEM']->loginUser($user);
		// redirect after login
		if($this->attr('redirect_after_login')) {
		    if(isset($_SESSION['sq_oauth_account_manager_current_state_redirect_url']) && $_SESSION['sq_oauth_account_manager_current_state_redirect_url'])  {
			$redirect_url = $_SESSION['sq_oauth_account_manager_current_state_redirect_url'];
			unset($_SESSION['sq_oauth_account_manager_current_state_redirect_url']);
			do_redirect($redirect_url);
		    }
		}
		// stop further redirects
		unset($_SESSION['sq_oauth_account_manager_current_state_redirect_url']);
		return TRUE;
	    }
	    return FALSE;
	}// end _loginUser
	
	
	/**
	* Log out of the Matrix user
	*
	* @return null
	* @access protected
	*/
	protected function _logoutUser()
	{			    
	    $for_real = ( $GLOBALS['SQ_SYSTEM']->user && !($GLOBALS['SQ_SYSTEM']->user instanceof Public_User));
	    $old_user = &$GLOBALS['SQ_SYSTEM']->user;
	    if ($for_real) {
		    $GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_before_user_logout', $old_user, NULL);
	    }
	    foreach (array_keys($_SESSION) as $key) {
		if ($key == 'SQ_RETURN_TO_HTTP') continue;
		// do not reset the unsuccessful attempt, 
		if ($key == 'user_login_attempts') continue;

		unset($_SESSION[$key]);
	    }
	    if ($for_real) {
		    $GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_user_logout', $old_user, NULL);
	    }
	    $GLOBALS['SQ_SYSTEM']->loginPublicUser();
	    $GLOBALS['SQ_SYSTEM']->generateLoginKey(TRUE);
	    
	}//end _logoutUser	    
	
	
	/**
	* Automatically create the local Matrix user
	* 
	* @param string	$create_type	matrix user type
	* 
	* @return null
	* @access protected
	*/
	protected function _createUser($create_type)
	{
	    $_POST['AB_'.$this->id.'_ASSET_BUILDER_ACTION'] = 'create';
	    $_POST['AB_'.$this->id.'_ASSET_BUILDER_CREATE_TYPE'] = $create_type;
	    
	    $GLOBALS['SQ_SYSTEM']->am->includeAsset($create_type);
	    $new_user = new $create_type();
	    $prefix = $new_user->getPrefix();
	    $username_attr = $new_user->getAttribute('username', TRUE);
	    $password_attr = $new_user->getAttribute('password', TRUE);
	    $GLOBALS['SQ_SYSTEM']->am->includeAsset('user');
	    $user = new User();
	    // username unique value is stored as owning type user
	    $user_username_attr = $user->getAttribute('username', TRUE);
	    $_POST['asset_action'] = 'create';
	    $_REQUEST['asset_action'] = 'create';
	    
	    // generate username from oauth userid, and make up a random password
	    $oauth_userid = $this->oauth_user_id;
	    $valid_username = $new_user->getValidUsername($user_username_attr->id, 'oauth_'.$this->id.'_'.$oauth_userid);
	    $valid_password = random_user_password(Array($valid_username));
	    
	    $_REQUEST[$prefix.'_'.$username_attr->id] = $valid_username;
	    $_REQUEST[$prefix.'_'.$password_attr->id.'_one'] = $valid_password;
	    $_REQUEST[$prefix.'_'.$password_attr->id.'_two'] = $valid_password;
	    
	    $this->_processGlobalActions();
	    
	} // end _createUser	
	


}//end class
?>
