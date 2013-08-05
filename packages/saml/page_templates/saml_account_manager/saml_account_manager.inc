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
* $Id: saml_account_manager.inc,v 1.8 2013/08/05 01:56:20 ewang Exp $
*
*/
require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_asset_builder/page_asset_builder.inc';
require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_DATA_PATH.'/private/conf/tools.inc';

/**
* SAML Account Manager
*
* Purpose
*
*
* @author  Edison Wang <ewang@squiz.com.au>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage file
*/
class SAML_Account_Manager extends Page_Asset_Builder
{

	
	var $saml_auth_manager;
	var $idp_entityid;
	
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
		if($this->isSimplesamlphpInstalled()) {
		    require_once SQ_TOOL_SIMPLESAMLPHP_PATH.'/lib/_autoload.php';
		    require_once SQ_PACKAGES_PATH.'/saml/lib/saml_auth_manager.inc';   
		}

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
		$this->saml_auth_manager = null;
		$this->idp_entityid = null;
		$this->__construct();
		
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
	* Get SAML Auth Manager class
	*
	* @return object
	* @access protected
	*/
	protected function _getAuthManager()
	{
		require_once SQ_PACKAGES_PATH.'/saml/lib/saml_auth_manager.inc';
		if($this->saml_auth_manager) return $this->saml_auth_manager;
		 $this->saml_auth_manager = new SAML_Auth_Manager($this->attr('auth_source'));
		 return $this->saml_auth_manager;
	}//end _getAuthManager()
	
	/**
	* Get entityid of the IDP
	*
	* @return string
	* @access protected
	*/
	protected function _getEntityid()
	{
		if($this->idp_entityid) return $this->idp_entityid;
		require SQ_TOOL_SIMPLESAMLPHP_PATH.'/config/authsources.php';
		$authConfig = array_get_index($config, $this->attr('auth_source'));
		 $this->idp_entityid = array_get_index($authConfig, 'idp');
		 return $this->idp_entityid;
	}//end _getEntityid()
	
	
	/**
	* Get unique saml id from authenticated saml user data
	*
	* @return string
	* @access protected
	*/
	protected  function _getAuthenticatedSamlUserid()
	{
	    $saml_auth_manager = $this->_getAuthManager();
	    $authData = $saml_auth_manager->getAuthDataArray();

	    $location = $this->attr('saml_userid_location');
	    if($location == 'nameid') {
		// use nameid as saml user id
		$nameid_array = array_get_index($authData, 'saml:sp:NameID');
		return (array_get_index($nameid_array, 'Value'));
	    }
	    else {	
		// use attribute value as saml user id
		$attribute = $this->attr('saml_userid_attribute');
		$attributes_array = array_get_index($authData, 'Attributes');
		return (array_get_index(array_get_index($attributes_array, $attribute), 0));
	    }
	   
	}// end _getAuthenticatedSamlUserid()
	
	
	
	/**
	* Is simplesamlphp librart installed?
	*
	* @return boolean
	* @access protected
	*/
	public function isSimplesamlphpInstalled()
	{
		return (SQ_TOOL_SIMPLESAMLPHP_PATH && file_exists(SQ_TOOL_SIMPLESAMLPHP_PATH.'/lib/_autoload.php'));
	}
	
	/**
	* Create this asset - Force the value of the "type" attribute to thesaurus term
	* @see asset::create()
	*
	* @return mixed int|boolean
	* @access public
	*/
	public function create(Array &$link)
	{
		return parent::create($link);

	}//end constructor
	
	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Asset builders create bodycopy assets when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		
		if (!parent::_createAdditional($link)) return FALSE;
		
		// add bodycopies to this page when creating
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('saml2_acs');
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('saml2_sls');

		$sub_assets= Array(
						'saml2_assertion_consumer_service'	=> 'saml2_acs',
						'saml2_single_logout_service'		=> 'saml2_sls',
					 );

		foreach ($sub_assets as $name => $type) {
			$asset = new $type();
			$copy_link = Array('asset' => &$this, 'value' => $name ,'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

			$asset->setAttrValue('name', ucwords(str_replace('_',' ', $name)));
			if (!$asset->create($copy_link)) return FALSE;
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			unset($asset);
		}
				
		return TRUE;

	}//end _createAdditional()
		
	
	
	/**
	* Returns an array of all the permitted link types, the asset type and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_TYPE_2]['saml2_acs'] = Array('card' => 'M', 'exclusive' => TRUE);
		$page_links[SQ_LINK_TYPE_2]['saml2_sls'] = Array('card' => 'M', 'exclusive' => TRUE);
		return $page_links;

	}//end _getAllowedLinks()
	
	
	/**
	* Prints out the Frontend for this asset
	*
	* @return void
	* @access public
	*/
	function printFrontend()
	{

	    	 // require library installed
		if(!$this->isSimplesamlphpInstalled()) {
		    trigger_error('Simplesamlphp library is not installed');
		    return FALSE;
		}
		require_once SQ_TOOL_SIMPLESAMLPHP_PATH.'/lib/_autoload.php';
		require_once SQ_PACKAGES_PATH.'/saml/lib/saml_auth_manager.inc';
		
		$saml_auth_manager = $this->_getAuthManager();

		// set global redirect URL
		if(!isset($_SESSION['sq_saml_account_manager_current_state_redirect_url'])) {
		    $redirect_url = $this->_getRedirectURL();
		    if(!empty($redirect_url)) 
			$_SESSION['sq_saml_account_manager_current_state_redirect_url'] = $redirect_url;
		}
	
	
			    
		/*
		 * SAML special actions
		 */
			
		// print SP metadata file
		if(isset($_GET['showMetadata'])) {
		   // get ACS URL
		   $acs_asset = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'saml2_acs', TRUE, 'saml2_assertion_consumer_service', 'major', '1');
		   $acs_url = array_get_index( $GLOBALS['SQ_SYSTEM']->am->getAssetURL(Array($acs_asset['minorid'])), $acs_asset['minorid']);

		    // get SLS URL
		    $sls_asset = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'saml2_sls', TRUE, 'saml2_single_logout_service', 'major', '1');
		    $sls_url = array_get_index( $GLOBALS['SQ_SYSTEM']->am->getAssetURL(Array($sls_asset['minorid'])), $sls_asset['minorid']);
  
		   $xml = $saml_auth_manager->printSPMetadataXML($acs_url, $sls_url);
		    header('Content-Type: application/samlmetadata+xml');
		    echo($xml);
		    exit();
		}
		
		// link current matrix user to the authenticated SAML acount
		if(isset($_GET['linkAccounts'])) {
		    $allow_create_link = $this->attr('allow_link');
		    // logged in saml user
		    if($allow_create_link && $saml_auth_manager->isAuthenticated()) {
			$samlUserid = $this->_getAuthenticatedSamlUserid();
			if($samlUserid) {
			    $current_user = $GLOBALS['SQ_SYSTEM']->user;
			    //  logged in matrix user
			    if($current_user && !($current_user instanceof Public_User)) {
				// link those 2 accounts
				$this->_linkMatrixUserWithSAML($current_user->id, $samlUserid);
				
				// redirect after linking
				if($this->attr('redirect_after_link')) {
				    if(isset($_SESSION['sq_saml_account_manager_current_state_redirect_url']) && $_SESSION['sq_saml_account_manager_current_state_redirect_url'])  {
					$redirect_url = $_SESSION['sq_saml_account_manager_current_state_redirect_url'];
					unset($_SESSION['sq_saml_account_manager_current_state_redirect_url']);
					do_redirect($redirect_url);
				    }
				}
				// stop further redirects
				unset($_SESSION['sq_saml_account_manager_current_state_redirect_url']);
			    }
			}  
		    }
		}
		
		/*
		 * probably we don't really need this function because allow user to unlink itself to be able to create new local users could be pointless and annoying
		 * it's rare to run into this situation and probaly best to inform sysadmin to unlink in backend.
		 * 
		// unlink current matrix user to the authenticated SAML account
		if(isset($_GET['unlinkAccounts'])) {
		    // logged in saml user
		    if($saml_auth_manager->isAuthenticated()) {
			$saml_userid = $this->_getAuthenticatedSamlUserid();
			if($saml_userid) {
			    $linked_matrix_userid = $this->_searchLinkedMatrixUser($saml_userid);
			    $current_user = $GLOBALS['SQ_SYSTEM']->user;
			    //  logged in matrix user is linked to the samlid
			    if($current_user && !($current_user instanceof Public_User) && ($current_user->id == $linked_matrix_userid)) {
				// unlink those 2 accounts
				$this->_unlinkMatrixUserWithSAML($linked_matrix_userid, $saml_userid);
				// logout of Matrix and saml, and start a new session
				$this->_logoutUser();
				do_redirect($this->getURL());
				return;
			    }
			}  
		    }
		}
		 */
		
		// single logout  user
		if(isset($_GET['logout'])) {
		    // log out of matrix first
		    $this->_logoutUser(FALSE);
		    
		    // start single logout of saml
		    if($saml_auth_manager->isAuthenticated()) {
			$options = Array();
			$return_url = $this->_getRedirectURL();
			if($return_url) {
			    $options['ReturnTo'] = $return_url;
			}
			$saml_auth_manager->logout($options);
		    }

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
	    
		$saml_auth_manager = $this->_getAuthManager();

		if (isset($_POST['AB_'.$this->id.'_ASSET_BUILDER_ACTION'])) {
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		}

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
		if(isset($_SESSION['sq_saml_account_manager_auth_error'])) {
		    $this->_replacements['auth_error'] = $_SESSION['sq_saml_account_manager_auth_error'];
		    $this->_current_state = 'auth_fail';
		    unset($_SESSION['sq_saml_account_manager_auth_error']);
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

		// If we don't allow linking saml user to existing Matrix user
		// and if user already logged in, just do nothing
		$allow_create_link = $this->attr('allow_link');
		if(empty($this->_current_state) && !$allow_create_link) {
		    if(!is_null($current_user) && !($current_user instanceof Public_User))
			$this->_current_state = 'logged_in';
		}
		
		if (empty($this->_current_state)) {
			/* 
			 * SAML authentication starts here
			 */
			
			if(!$saml_auth_manager->isAuthenticated()) {
			
			    $options = Array();
			    // add saml extensions to request message
			    if($this->attr('use_extensions')) {
				$extensions = $this->attr('extensions');
				$ext = Array();
				if(!empty($extensions)) {
				    $dom = new DOMDocument();
				    $dom->loadXML($extensions);
				    $extension_node = $dom->getElementsByTagName('Extensions')->item(0);
				    $child_nodes = $extension_node->childNodes;
				    foreach ($child_nodes as $node) {
					$ext[] = new SAML2_XML_Chunk($node);
				    }
				}
				if(!empty($ext)) {
				    $options['saml:Extensions'] = $ext;
				}
			    }

			     // add ACS URL
			    $acs_asset = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'saml2_acs', TRUE, 'saml2_assertion_consumer_service', 'major', '1');
			    $acs_url = array_get_index( $GLOBALS['SQ_SYSTEM']->am->getAssetURL(Array($acs_asset['minorid'])), $acs_asset['minorid']);
			    $options['acs_url'] = $acs_url;
			    
			    // add other options
			    $protocol_binding = $this->attr('protocol_binding');
			    $nameid_policy = $this->attr('nameid_policy');
			    $authn_context_class_ref = $this->attr('authn_context_class_ref');
			    
			    if(!empty($protocol_binding)) {
				$options['ProtocolBinding'] = $protocol_binding;
			    }
			   if(!empty($nameid_policy)) {
				$options['saml:NameIDPolicy'] = $nameid_policy;
			    }
			   if(!empty($authn_context_class_ref)) {
				$options['saml:AuthnContextClassRef'] = $authn_context_class_ref;
			    }
			   
			    // remove all buffered content, so it won't interrupt the SAML redirect
			    while (ob_get_level() > SQ_INIT_OB_LEVEL) {
				ob_end_clean();
			    }
    
			    // login saml, start the dance
			   $saml_auth_manager->login($options);
			   exit();
			}
			
			
			// SAML user should have been already authenticated at this point
			 
	
			// try find linked Matrix user
			$samlUserid = $this->_getAuthenticatedSamlUserid();
			$matrixUserid = $this->_searchLinkedMatrixUser($samlUserid);
			
			
			// Log in as the linked Matrix user
			if ((is_null($current_user) || ($current_user instanceof Public_User)) && $matrixUserid) {
			    $this->_loginUser($matrixUserid);
			    $current_user = $GLOBALS['SQ_SYSTEM']->user;
			}
			
			
			
			// can't find a linked matrix user for the authenticated saml user, create a new matrix user
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
			// if logged in as both Matrix user and SAML user
			    	if($GLOBALS['SQ_SYSTEM']->currentUserId() == $matrixUserid) {
				    // the saml user is correctly linked to current matrix user
				    $this->_current_state = 'logged_in';
				}
				else {
				    // the saml user is not linked to current logged in matrix user, create a link?
				     $this->_current_state = 'create_link';
				}
			
				
			}
		}
		
		// instead of displaying logged in body, do redirect
		if( $this->_current_state == 'logged_in' && $this->attr('redirect_after_login') && isset($_SESSION['sq_saml_account_manager_current_state_redirect_url']) && $_SESSION['sq_saml_account_manager_current_state_redirect_url'])  {
		    $redirect_url = $_SESSION['sq_saml_account_manager_current_state_redirect_url'];
		    unset($_SESSION['sq_saml_account_manager_current_state_redirect_url']);
		    do_redirect($redirect_url);
		}
				    

		$fn = '_load'.$this->_default_state.'Replacements';
		$this->_loadcreateReplacements();
		$this->$fn();
		
		
		
		if($saml_auth_manager->isAuthenticated()) {
		    $auth_data = $saml_auth_manager->getAuthDataArray();
		    $attributes_array = array_get_index($auth_data, 'Attributes');
		    
		    // replace user attribute keyword
		    if(!empty($attributes_array)) {
			// get the bodycopy attached to us
			$bodycopy_data = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, $this->_current_state, 'major', '1');
			if(!empty($bodycopy_data)) {
			    $bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_data['minorid'], $bodycopy_data['minor_type_code']);
			    $keywords = $bodycopy->getKeywords();
			    foreach($keywords as $word) {
				    $keyword = parse_keyword($word, $modifiers);
				    if($keyword === 'user_attributes_xml') {
					// replace it with xml version of user's attributes, also enable modifier like xpath.
					$xml = new SimpleXMLElement('<root/>');
					arrayToXml($attributes_array, $xml);
					$replacement = $xml->asXML();
					apply_keyword_modifiers($replacement, $modifiers);
					$this->_replacements[$word] = $replacement;

				    }
			    }
			}
		    }
		    
		    // replace saml id keyword
		    $this->_replacements['user_saml_id'] =  $this->_getAuthenticatedSamlUserid();
		}
		 
		
		$this->_printBody();

		// stop performance mode timer
		$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this, 'printBody');
		
		if (isset($_POST['AB_'.$this->id.'_ASSET_BUILDER_ACTION'])) {
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		}

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
		    // link the created user with this saml account
		    $samlUserid = $this->_getAuthenticatedSamlUserid();
		    if(!empty($samlUserid))
			$this->_linkMatrixUserWithSAML($this->_tmp['created_asset']->id, $samlUserid);
		 
		    $this->_loginUser($this->_tmp['created_asset']->id);
		    
		    $this->_current_state = 'logged_in';
		}
		

		return TRUE;

	}//end _processGlobalActions()
	
	
	
	
	/**
	* Search linked Matrix user by SAML user id
	*
	* @param varchar	$samlUserid	SAML userid string
	* 
	* @return int | null
	* @access protected
	*/
	protected function _searchLinkedMatrixUser ($samlUserid)
	{
		if (empty($samlUserid))
		    return NULL;
		
		// DAL query
		$bind_vars = Array(
						'samlid'      => $samlUserid,
						'entityid'      => $this->_getEntityid(),
					    );
		$result = MatrixDAL::executeAll('saml_package', 'getUserid', $bind_vars);
		if(empty($result)) return NULL;
		
		return ($result[0]['assetid']);
	}//end _searchLinkedMatrixUser()
	
	
	/**
	* Link Matrix user with specified SAML account
	*
	* @param varchar	$matrixUserid	Matrix userid  string
	* @param varchar	$samlUserid	SAML userid string
	* 
	* @return int | null
	* @access protected
	*/
	protected function _linkMatrixUserWithSAML ($matrixUserid, $samlUserid)
	{
		if (empty($matrixUserid) || empty($samlUserid) )
		    return NULL;
		
		// already linked?
		 $existingMatrixUserid = $this->_searchLinkedMatrixUser($samlUserid);
		 if($existingMatrixUserid) {
		     $bind_vars = Array(
						'assetid'	=>  $matrixUserid,
						'samlid'	=>  $samlUserid,
						'entityid'	=> $this->_getEntityid(),
			 			'updated'	=>  ts_iso8601(time()),
					    );
		    $result = MatrixDAL::executeQuery('saml_package', 'updateUserid', $bind_vars);
		     return $result;
		 }
		
		// insert new entry
		$linkid = MatrixDAL::executeOne('core', 'seqNextVal', Array('seqName' => 'sq_saml_lnk_seq'));
		$bind_vars = Array(
						'linkid'	=>  $linkid,
						'assetid'	=>  $matrixUserid,		    
						'samlid'	=>  $samlUserid,
						'entityid'	=> $this->_getEntityid(),
						'created'	=>  ts_iso8601(time()),
						'updated'	=>  ts_iso8601(time()),
					    );
		$result = MatrixDAL::executeQuery('saml_package', 'addLink', $bind_vars);
		 return $result;
	}//end _linkMatrixUserWithSAML()
	
	
	/**
	* Unlink Matrix user with specified SAML account
	*
	* @param varchar	$matrixUserid	Matrix userid  string
	* @param varchar	$samlUserid	SAML userid string
	* 
	* @return int | null
	* @access protected
	*/
	protected function _unlinkMatrixUserWithSAML ($matrixUserid, $samlUserid)
	{
		if (empty($matrixUserid) || empty($samlUserid) )
		    return NULL;
		
		// delete entry
		$bind_vars = Array(
						'assetid'	=>  $matrixUserid,		    
						'samlid'	=>  $samlUserid,
						'entityid'	=> $this->_getEntityid(),
					    );
		$result = MatrixDAL::executeQuery('saml_package', 'deleteLink', $bind_vars);
		 return $result;
	}//end _linkMatrixUserWithSAML()
		

	
	
	/**
	* Returns the asset (if any) that we are redirecting to
	*
	* @return mixed object|NULL
	* @access public
	*/
	function &getRedirectAsset()
	{
		$res = NULL;
		if ($this->useSystemVersion()) {
			$notice_links = unserialize(file_to_string($this->data_path.'/.sq_notice_links'));
			foreach ($notice_links as $link) {
				if ($link['value'] == 'redirect_asset') {
					$redirect_link = $link;
					break;
				}
			}
		} else {
			if (!empty($this->id)) {
				$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', TRUE, 'redirect_asset');
			}
		}
		if (!empty($link)) {
			$res = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		}

		return $res;

	}//end getRedirectAsset()


	/**
	* Set's the asset (if any) that we are going to be redirecting to
	*
	* @param object	&$asset	the asset that we want to redirect to (NULL clears the redirect asset)
	*
	* @return boolean
	* @access public
	*/
	function setRedirectAsset(&$asset)
	{
		$old_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', TRUE, 'redirect_asset');
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		if (empty($old_link)) {
			// if there is not existing asset, and we aren't setting on, we are done here
			if (is_null($asset)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return TRUE;
			}

		// else if there is a link, and we are about to set an asset, but it is the same as we already have
		} else if (!is_null($asset) && $old_link['minorid'] == $asset->id) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return TRUE;

		// else something different, delete the old link
		} else {
			if (!$this->deleteLink($old_link['linkid'])) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		}

		if (!is_null($asset)) {
			if (!$this->createLink($asset, SQ_LINK_NOTICE, 'redirect_asset')) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end setRedirectAsset()
	
	
	
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
			$redirect_asset = $this->getRedirectAsset();
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
			case 'auth_fail' :
				$vars['keywords'] = array_merge($vars['keywords'], Array('auth_error' => translate('saml_account_manager_auth_error')));
			break;

		}
		
		$vars['keywords'] = array_merge($vars['keywords'], Array('user_attributes_xml' => translate('saml_account_manager_user_attributes_xml')));
		$vars['keywords'] = array_merge($vars['keywords'], Array('user_saml_id' => translate('saml_account_manager_user_saml_id')));

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
							'name'	=> translate('saml_account_manager_create_user'),
							'content'	=> '%create_error% %login_error% %login_invite% %login_form% %create_form%',
						    );
		$res['create_link']= Array(
							'name'	=> translate('saml_account_manager_create_link'),
							'content'	=> 'SAML account is not linked to current Matrix user. <a href="?linkAccounts">Link them now</a>.',
						 );
		$res['logged_in']= Array(
							'name'	=> translate('logged_in'),
							'content'	=> 'logged in as %globals_user_name%',
						   );
		$res['auth_fail']= Array(
							'name'	=> translate('saml_account_manager_auth_fail'),
							'content'	=> '%auth_error%',
						   );
		return $res;

	}//end _getBodycopies()
	
	/**
	* Log in the Matrix user (and do redirects if required)
	* 
	* @param string	$userid	matrix user id
	* 
	* @return null
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
		    if(isset($_SESSION['sq_saml_account_manager_current_state_redirect_url']) && $_SESSION['sq_saml_account_manager_current_state_redirect_url'])  {
			$redirect_url = $_SESSION['sq_saml_account_manager_current_state_redirect_url'];
			unset($_SESSION['sq_saml_account_manager_current_state_redirect_url']);
			do_redirect($redirect_url);
		    }
		}
		// stop further redirects
		unset($_SESSION['sq_saml_account_manager_current_state_redirect_url']);
	    }
	}// end _loginUser
	
	
	/**
	* Log out of the Matrix user
	*
	* @param bool	$clear_saml_session	clears saml session
	* @return null
	* @access protected
	*/
	protected function _logoutUser($clear_saml_session = TRUE)
	{			    
	    $for_real = ( $GLOBALS['SQ_SYSTEM']->user && !($GLOBALS['SQ_SYSTEM']->user instanceof Public_User));
	    $old_user = &$GLOBALS['SQ_SYSTEM']->user;
	    if ($for_real) {
		    $GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_before_user_logout', $old_user, NULL);
	    }
	    foreach (array_keys($_SESSION) as $key) {
		if ($key == 'SQ_RETURN_TO_HTTP') continue;
		// do not reset the unsuccessful attempt, 
		// do not reset current saml session
		if ($key == 'user_login_attempts') continue;
		  
		if( !$clear_saml_session && $key == 'SimpleSAMLphp_SESSION') continue;
		
		unset($_SESSION[$key]);
	    }
	    if ($for_real) {
		    $GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_user_logout', $old_user, NULL);
	    }
	    $GLOBALS['SQ_SYSTEM']->loginPublicUser();
	    $GLOBALS['SQ_SYSTEM']->generateLoginKey(TRUE);
	    
	    if($clear_saml_session) {
		$this->saml_auth_manager = null;
	    }
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
	    
	    // generate username from samlid, and make up a random password
	    $samlid = $this->_getAuthenticatedSamlUserid();
	    $valid_username = $new_user->getValidUsername($user_username_attr->id, $samlid);
	    $valid_password = random_user_password(Array($valid_username));
	    
	    $_REQUEST[$prefix.'_'.$username_attr->id] = $valid_username;
	    $_REQUEST[$prefix.'_'.$password_attr->id.'_one'] = $valid_password;
	    $_REQUEST[$prefix.'_'.$password_attr->id.'_two'] = $valid_password;
	    
	    $this->_processGlobalActions();
	    
	} // end _createUser	
	


}//end class
?>
