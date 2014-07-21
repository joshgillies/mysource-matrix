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
* $Id: page_account_manager.inc,v 1.81 2013/10/11 01:23:04 cupreti Exp $
*
*/


require_once dirname(__FILE__).'/../../page_templates/page_asset_builder/page_asset_builder.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Page_Account_Manager
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Revision: 1.81 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Account_Manager extends Page_Asset_Builder
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Create this asset
	*
	* The return value will be:<br/>
	* <ul>
	*   <li>FALSE if the asset was not created</li>
	*   <li>the ID of the newly created link if the asset and intital link were created</li>
	* </ul>
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return mixed int|boolean
	* @access public
	*/
	function create(&$link)
	{
		// Set default value for validation email format attr
		$val = Array();
		$val['subject'] = translate('cms_account_manager_validation_email_default_subject');
		$val['text_format'] = translate('cms_account_manager_validation_email_default_content', '%validation_url%');
		$val['html_format'] = '<p>'.translate('cms_account_manager_validation_email_default_content', '<a href="%validation_url%">%validation_url%</a>').'</p>';
		$val['from'] = SQ_CONF_SYSTEM_NAME.' <'.SQ_CONF_DEFAULT_EMAIL.'>';
		$this->setAttrVAlue('validation_email_format', $val);

		return parent::create($link);

	}//end create()


	/**
	* Save attributes set with setAttrValue to the database
	*
	* @param boolean	$dont_run_updated	if true this->_updated() won't be run
	*										(THIS IS ONLY EVER CALLED FROM Asset::create())
	*
	* @return boolean
	* @access public
	* @see setAttrValue()
	*/
	function saveAttributes($dont_run_updated=FALSE)
	{
		if (isset($this->_tmp['vars_set']['use_email_validation'])) {
			if ($this->attr('use_email_validation')) {
				$new_names = Array(
								'create_no_login'	=> translate('cms_account_manager_awaiting_validation'),
								'create_login'		=> translate ('cms_account_manager_validated'),
							 );
			} else {
				$new_names = Array(
								'create_no_login'	=> translate('cms_account_manager_created_not_live'),
								'create_login'		=> translate('cms_account_manager_created_live'),
							 );
			}
			foreach (Array('create_login', 'create_no_login') as $value) {
				$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, NULL, 'bodycopy', TRUE, $value);
				if (!empty($link)) {
					$bc = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], 'bodycopy');
					$bc->setAttrValue('name', $new_names[$value]);
					$bc->saveAttributes();
				}
			}
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// if we are using email validation - ensure that the username and email attributes are required fields
		if ($this->attr('use_email_validation')) {

			$required_fields = $this->attr('required_fields');
			list($create_type) = array_keys($this->attr('create_type'));
			$save = FALSE;
			if (!isset($required_fields[$create_type])) {
				$required_fields[$create_type] = Array();
			}
			if (!isset($required_fields[$create_type]['username'])) {
				$required_fields[$create_type]['username'] = translate('cms_account_manager_username_required');
				$save = TRUE;
			}
			if (!isset($required_fields[$create_type]['email'])) {
				$required_fields[$create_type]['email'] = translate('cms_account_manager_email_required');
				$save = TRUE;
			}

			if ($save) {
				if (!$this->setAttrValue('required_fields', $required_fields)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}// end if
			}// en dif

		}// end if

		// Create / remove layouts for the logged in customisation
		if ($this->id && !$GLOBALS['SQ_SYSTEM']->am->assetInTrash($this->id, TRUE)) {
			$customise_logged_in  = $this->attr('customise_logged_in');
			$type_code_to_create  = array_keys($this->attr('create_type'));
			$type_code_to_create  = $type_code_to_create[0];
			$current_layout_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'layout', TRUE, 'major');
			$current_layouts = Array();

			if (!empty($current_layout_links)) {
				foreach ($current_layout_links as $link) {
					if (strpos($link['value'], '_logged_in_customisation') === FALSE) continue;
					if ($customise_logged_in && $link['value'] === $type_code_to_create.'_logged_in_customisation') {
						$current_layouts[] = $link['value'];
					} else {
						$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($link['linkid']);
					}
				}
			}

			if ($customise_logged_in && empty($current_layouts)) {
				$type_names = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo(Array($type_code_to_create), 'name');
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('layout');
				$layout = new Layout();
				$layout->setAttrValue('name', translate('cms_account_manager_logged_in_layout_for_type', $type_names[$type_code_to_create]));
				$link = Array(
							'asset'			=> &$this,
							'link_type'		=> SQ_LINK_TYPE_2,
							'is_dependant'	=> TRUE,
							'is_exclusive'	=> TRUE,
							'value'			=> $type_code_to_create.'_logged_in_customisation',
						);
				$layout->create($link);
			}
		}

		if (!parent::saveAttributes($dont_run_updated)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end saveAttributes()


	/**
	* Prints out the Frontend for this asset
	*
	* @return void
	* @access public
	*/
	function printFrontend()
	{
		$current_user = $GLOBALS['SQ_SYSTEM']->user;

		if ((!is_null($current_user) && !($current_user instanceof Public_User)) && !SQ_IN_LIMBO && !$this->attr('customise_logged_in')) {

			if ($this->attr('enter_edit_mode')) {
				// this person is logged in - so redirect them to limbo where they can edit their details
				$redirect_url = strip_url($this->getURL()).'/'.SQ_CONF_LIMBO_SUFFIX.'/?ignore_frames=1';
			}

			// BUT if they have just logged in via this page we might want to redirect them somewhere else...
			if (isset($_REQUEST['SQ_LOGIN_ACCOUNT_MANAGER_STATE']) && $_REQUEST['SQ_LOGIN_ACCOUNT_MANAGER_STATE'] == 'create_login') {
				$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', FALSE, 'login_redirect');
				if (!empty($link)) {
					// the manager is configured with a custom redirect location
					$redirect_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid']);
					$redirect_url = $redirect_asset->getURL();
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($redirect_asset);
				}//end if
			}//end if
		} //end if

		if (isset($redirect_url)) {
			header('Location: '.$redirect_url);
			exit();
		} else {
			parent::printFrontend();
		}

	}//end printFrontend()


	/**
	* Loads keyword replacements for the create asset interface, into the Asset Builder
	*
	* @return void
	* @access private
	*/
	function _loadCreateReplacements()
	{
		$this->_replacements['resend_validation_form'] = $this->getkeywordreplacement('re_send_validation_form');
		parent::_loadCreateReplacements();

	}//end _loadCreateReplacements()


	/**
	* Initialise the limbo (simple-edit) interface by painting to the limbo outputter
	*
	* This function should be overridden if assets want to expose another asset's interface
	* instead of their own.  By default, the normal backend is printed for the default screen.
	*
	* @return void
	* @access public
	* @see printLimbo
	*/
	function initLimbo()
	{
		// intentonally blank

	}//end initLimbo()


	/**
	* Called by the design to print the body of this asset when in limbo
	*
	* @return void
	* @access public
	*/
	function printLimbo()
	{
		// if there is no backend outputter object it is likely we are using a login design
		// the backend outputter will be present however when the user logs in
		if (isset($GLOBALS['SQ_SYSTEM']->backend->out)) {
			$GLOBALS['SQ_SYSTEM']->backend->out->addFormActionGetVar('ignore_frames', '1', TRUE);
		}

		$current_user = $GLOBALS['SQ_SYSTEM']->user;
		$this->_current_state = 'logged_in';

		if (!is_null($current_user) && !($current_user instanceof Public_User)) {
			// try and acquire a lock on the current user so they can edit their details
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array(
						'assetids'			=> Array($current_user->id,),
						'lock_type'			=> 'attributes',
						'forceably_acquire'	=> FALSE,
					);
			$lock_errors = $hh->freestyleHipo('hipo_job_acquire_locks', $vars);


			  /////////////////////////
			 //  EDIT DETAILS FORM  //
			/////////////////////////
			ob_start();

				if (!empty($lock_errors)) {
					$this->_replacements['edit_details_lock_error'] = $this->attr('edit_details_lock_error');
					$current_user->printBody();
				} else {


					// ensure that the user has access to edit their details
					// ie. they have permission to access the default screen for a user
					require_once SQ_INCLUDE_PATH.'/asset_edit_interface.inc';
					$aei = new Asset_Edit_Interface($current_user->type());

					$allowed_screens = $aei->getAllowedScreens($current_user);
					$default_screen = $aei->_default_screen;
					if (empty($default_screen)) {
						$default_screen = 'details';
					}

					$check_screen_restrictions = (!($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin()));
					if ($check_screen_restrictions) {
						if (empty($allowed_screens)) {
							$check_screen_restrictions = FALSE;
						}
					}

					if ($check_screen_restrictions && !isset($allowed_screens[$default_screen])) {
						$this->_replacements['edit_details_invite'] = '<p>'.translate('cms_account_manager_cannot_edit_details').'</p>';
						$current_user->printBody();
					} else {
						$this->_replacements['edit_details_invite'] = $this->attr('edit_details_invite');
						$current_user->initLimbo();
						$current_user->printLimbo();
					}
					unset($aei);
				}
				$this->_replacements['edit_details_form'] = ob_get_contents();
			ob_end_clean();


			  ////////////////////
			 //  LOG OUT FORM  //
			////////////////////
			$this->_replacements['logout_form'] = $this->getKeywordReplacement('logout_form');
		}//end if
		$this->_printContents();

	}//end printLimbo()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return boolean
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		$allowed = parent::_getAllowedLinks();
		$allowed[SQ_LINK_NOTICE]['asset']   = Array('card' => 'M', 'exclusive' => FALSE);

		return $allowed;

	}//end _getAllowedLinks()


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
		list($create_type) = array_keys($this->attr('create_type'));

		// resend validation email form was submitted?

		if (isset($_POST['AB_'.$this->id.'_ASSET_BUILDER_ACTION']) && $_POST['AB_'.$this->id.'_ASSET_BUILDER_ACTION'] == 'resend_validation') {
			return $this->resendValidation();
		}

		if (strtolower(array_get_index($_GET, 'action', '')) == 'validate') {
			// grab the user and create locations
			$user = $this->_getCurrentPendingUser();
			if (!is_null($user)) {

				if ($this->validateUser($user)) {
					$this->_current_state = 'create_login';
					$_POST['SQ_LOGIN_USERNAME'] = $user->attr('username');
					$this->_replacements['login_invite'] = $this->attr('login_invite');
					$this->_replacements['login_form'] = $this->getKeywordReplacement('login_form');
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($user);
					return TRUE;
				}
				return FALSE;

			}//end if
		}//end if action is validate

		// additional checks if we are using email validation
		if ($this->attr('use_email_validation') && isset($_POST['AB_'.$this->id.'_ASSET_BUILDER_ACTION']) && strtolower($_POST['AB_'.$this->id.'_ASSET_BUILDER_ACTION']) == 'create') {

			// create the unique ID we will use to identify this new user if it gets created
			$this->_tmp['am_created_asset_key'] = '';

			$GLOBALS['SQ_SYSTEM']->am->includeAsset($create_type);
			$new_asset = new $create_type();

			$real_username = $new_asset->getAttribute('username', TRUE);
			if (is_null($real_username)) continue;
			// take a copy of the username so we don't muck anything up for the processBackend() (if we get that far;)
			$username = $real_username;

			$username->process($new_asset->getPrefix().'_'.$username->id);
			if ($username->processed) {
				if (!empty($username->value)) {
					$this->_tmp['am_created_asset_key'] = md5(microtime().$username->value);
				}
			}

			unset($real_username);
			unset($username);

		}//end if using email validation

		// get asset_builder to create the asset, but since we have overridden
		// getCreateLocations it will only go into our pending accounts group
		// if email verification is enabled.

		$success = parent::_processGlobalActions();
		if (!$success) return FALSE;

		if (isset($_POST['AB_'.$this->id.'_ASSET_BUILDER_ACTION'])) {
			switch (strtolower($_POST['AB_'.$this->id.'_ASSET_BUILDER_ACTION'])) {
				case 'create' :
					if (empty($this->_errors) && !empty($this->_tmp['created_asset'])) {
						if ($this->attr('use_email_validation')) {
							// notice link the newly-created asset underneath the selected create locations,
							// so when we validate we know which create locations were selected
							$new_asset =& $this->_tmp['created_asset'];

							// Remove all permission from the created user, apart from the admin permission the
							// user is allowed to hold on itself. This means the user will not inherit permissions
							// from the Pending Accounts folder.
							$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

							$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
							$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

							foreach (get_constant_values('SQ_PERMISSION_') as $permission_name => $permission) {
								$permissions = $GLOBALS['SQ_SYSTEM']->am->getPermission($new_asset->id, $permission, NULL, FALSE);
								foreach ($permissions as $userid) {
									if (!(($permission == SQ_PERMISSION_ADMIN) && ($userid == $new_asset->id))) {
										if (!$GLOBALS['SQ_SYSTEM']->am->deletePermission($new_asset->id, $userid, $permission)) {
											$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
											$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
											$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
											return FALSE;
										}
									}
								}
								$permissions = $GLOBALS['SQ_SYSTEM']->am->getPermission($new_asset->id, $permission, NULL, FALSE);
							}

							$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
							$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
							$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

							// Notice link to the additional create locations where user will be linked after validation
							$additional_create_locations = $this->getSelectedCreateLocations() + $this->getDynamicCreateLocations() + $this->getAdditionalCreateLocations();
							if (!empty($additional_create_locations)) {
								$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

									$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
									$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

										foreach ($additional_create_locations as $locationid => $link_type) {
											$location = $GLOBALS['SQ_SYSTEM']->am->getAsset($locationid, '', TRUE);
											if (is_null($location)) continue;
											if (!$GLOBALS['SQ_SYSTEM']->am->createAssetLink($location, $new_asset, SQ_LINK_NOTICE, $this->getPrefix().'_selected_create_location-'.$link_type)) {
												// bail out
												$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
												$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
												$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
												return FALSE;
											}
										}

									$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
									$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

								$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
							}

							// we are validating this user using email, so send them one
							$replacements = Array();
							$attrs = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($create_type);
							foreach ($attrs as $attr => $type) {
								if (($attr == 'password') || ($type['type'] == 'serialise')) {
									continue;
								}
								$replacements[$attr] = $new_asset->attr($attr);
							}

							$replacements['validation_url'] = current_url().'?action=validate&amid='.$this->_tmp['am_created_asset_key'];

							// add an email to the TO list for the user who just signed up
							$email_attr = $this->getAttribute('validation_email_format');
							$new_value = unserialize($email_attr->value);
							$new_value['to'][] = $replacements['email'];
							$email_attr->value = serialize($new_value);

							// let the email format attribute send the email for us
							$email_attr->sendMail($replacements);
						}//end if

						// the new user was created and can log in, so make sure the page is in the right state
						if ($this->attr('create_status') >= SQ_STATUS_LIVE && !$this->attr('use_email_validation')) {
							$this->_current_state = 'create_login';
							$_POST['SQ_LOGIN_USERNAME'] = $this->_tmp['created_asset']->attr('username');
							$this->_replacements['login_invite'] = $this->attr('login_invite');
							$this->_replacements['login_form'] = $this->getKeywordReplacement('login_form');
						} else {
							$this->_current_state = 'create_no_login';
						}
					}//end if
				break;
			}//end switch
		}//end if

		return TRUE;

	}//end _processGlobalActions()


	/**
	* Work out the create locations for a new asset
	*
	* Account manager replaces the create location list with its pending-accounts
	* group when email validation is enabled, but it still checks the parent
	* create locations to make sure there will be somewhere to move the user to
	* after it's been validated.
	*
	* @param string	$create_type	the type of asset we are getting create locations for
	*
	* @return array
	* @access private
	*/
	function getCreateLocations($create_type)
	{
		$locs = parent::getCreateLocations($create_type);
		if ($this->attr('use_email_validation') && !empty($locs)) {
			// Validation is enabled, and we have some 'real' create locations,
			// so go ahead and create the asset in the pending-users group,
			// as long as the group exists
			$locs = Array();
			$pending_groupid = $this->_getPendingAccountsGroupId();
			if ($pending_groupid) {
				$locs = Array($pending_groupid => SQ_LINK_TYPE_1);
			}
		}
		return $locs;

	}//end getCreateLocations()


	/**
	* Takes the default link that was created and allows for adjustments if necessary
	*
	* @param array	&$link	the current link array
	*
	* @return void
	* @access private
	*/
	function _modifyCreateLink(&$link)
	{
		parent::_modifyCreateLink($link);
		if ($this->attr('use_email_validation')) {
			$link['value'] = $this->_tmp['am_created_asset_key'];
		}

	}//end _modifyCreateLink()


	/**
	* Sets the status of the newly created asset
	*
	* If we are using email validation we dont change the status of the new asset
	*
	* @param object	&$new_asset	the newly created asset
	*
	* @return boolean
	* @access private
	*/
	function _setCreateStatus(&$new_asset)
	{
		if ($this->attr('use_email_validation')) {
			return TRUE;
		} else {
			return parent::_setCreateStatus($new_asset);
		}

	}//end _setCreateStatus()


	/**
	* Processes a backend submission from this asset, returns true if all OK
	*
	* @param object	&$o		Backendoutputer
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access public
	*/
	function processBackend(&$o, &$link)
	{
		if (SQ_IN_LIMBO) {
			$current_user = $GLOBALS['SQ_SYSTEM']->user;

			if (!is_null($current_user) && !($current_user instanceof Public_User)) {
				set_error_handler(Array(&$this, '_errorHandler'));

					if ($current_user->processBackend($o, $link)) {
						$this->_replacements['edit_details_success'] = $this->attr('edit_details_success');
					}
				restore_error_handler();
				
				$errors = '';
				if (!empty($this->_errors)) {
					foreach($this->_errors as $text) {
						$errors .= '<li>'.$text.'</li>';
					}//end foreach
				}//end if
				$this->_replacements['form_errors'] = $errors;
			}

			return TRUE;
		}

		return parent::processBackend($o, $link);

	}//end processBackend()


	/**
	* Determine if the current user is allowed into this asset's backend interface
	*
	* @return boolean
	* @access public
	*/
	function backendAccess()
	{
		if (SQ_IN_LIMBO) {
			return ($GLOBALS['SQ_SYSTEM']->user instanceof User);
		} else {
			return parent::backendAccess();
		}

	}//end backendAccess()


	/**
	* Return an array of bodycopies that need to be created
	*
	* @return array
	* @access private
	*/
	function _getBodycopies()
	{
		$res = Array();

		$res['not_logged_in']['name'] = translate('not_logged_in');
		$res['not_logged_in']['content'] = '<p>%create_error%%login_error%</p>'."\n".
											'%create_invite%%create_form%%login_invite%%login_form%';

		$res['create_no_login']['name'] = translate('cms_account_manager_created_not_live');
		$res['create_no_login']['content'] = translate('cms_account_manager_created_need_activation');

		$res['create_login']['name'] = translate('cms_account_manager_created_live');
		$res['create_login']['content'] = translate('cms_account_manager_created_please_log_in')."\n%login_form%";

		$res['logged_in']['name'] = translate('logged_in');
		$res['logged_in']['content'] = '<p>%edit_details_lock_error%%edit_details_success%</p>'."\n".
										'%edit_details_invite%%edit_details_form%';

		return $res;

	}//end _getBodycopies()


//--        PENDING ACCOUNTS        --//


	/**
	* Returns a list of selected create locations for a specified pending user
	*
	* @param string	$userid	the userid of the pending user
	*
	* @return array
	* @access public
	*/
	function getSelectedCreateLocationsForPendingUser($userid)
	{
		$create_locations = Array();
		$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($userid, SQ_LINK_NOTICE, Array('user_group'), FALSE, 'minor');
		foreach ($links as $link) {
			// check that the link is actually a selectable create location link
			// the link value should be in the format: 'PREFIX_selected_create_location-LINK_TYPE'
			preg_match('/^'.$this->getPrefix().'_selected_create_location-([\d]*)$/', $link['value'], $matches);
			if (!empty($matches)) {
				$create_locations[$link['majorid']] = $matches[1];
			}
		}
		return $create_locations;

	}//end getSelectedCreateLocationsForPendingUser()


	/**
	* Validates the passed user
	*
	* @param object	&$user	the user to validate
	*
	* @return boolean
	* @access public
	*/
	function validateUser(&$user)
	{
		// Figure out create locations.
		//    - Mandatory ones are easily looked up
		//    - Rule-based ones are calculated from an
		//      array of asset attribute values
		//    - User-selected ones are grabbed from the
		//      user's notice links
		list($create_type) = array_keys($this->attr('create_type'));

		$user_attributes = Array();
		$attrs = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($create_type);
		foreach ($attrs as $attr => $type) {
			$user_attributes[$attr] = $user->attr($attr);
		}

		$selected_create_locations = $this->getSelectedCreateLocationsForPendingUser($user->id);
		$create_locations = parent::getFixedCreateLocations()
							+ parent::getRuleMatchCreateLocations($create_type, $user_attributes)
							+ $selected_create_locations;

		if (empty($create_locations)) {
			trigger_localised_error('CMS0071', E_USER_WARNING);
			return FALSE;
		}

		// set the install run level because we are creating an asset without being
		// logged in - so any write access checks will fail
		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			// link the user asset into the locations it is supposed to go
			foreach ($create_locations as $link_parentid => $link_type) {
				$link_parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($link_parentid, '', TRUE);
				if (is_null($link_parent)) continue;
				if (!$GLOBALS['SQ_SYSTEM']->am->createAssetLink($link_parent, $user, $link_type)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
					return FALSE;
				}

				// Bug #5509: Account Manager User asset webpath conflict.
				$current_paths = $user->getWebPaths();
				require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
				$valid_paths = make_valid_web_paths($current_paths, $user->id);
				$valid_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($link_parent, $valid_paths, $user->id, TRUE);
				if (!$user->saveWebPaths($valid_paths, FALSE)){
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
					return FALSE;
				}
			}

			// remove the selectable create location notice links, if they exist
			foreach ($selected_create_locations as $link_parentid => $link_type) {
				$link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($link_parentid, $user->id, SQ_LINK_NOTICE, $this->getPrefix().'_selected_create_location-'.$link_type);
				if (!empty($link)) {
					if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($link['linkid'])) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
						return FALSE;
					}
				}
			}

			// remove the link from the pending accounts group
			if (!$this->_removePendingUserFromPendingGroup($user->id)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
				return FALSE;
			}

			// set the status of the asset
			if (!parent::_setCreateStatus($user)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
				return FALSE;
			}

			// if we move an asset, we need to update the urls (lookups) as well.
			$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
 			$vars = Array('assetids' => Array($user->id), 'auto_add_remaps' => FALSE);
			$hipo_empty_array = $hh->freestyleHipo('hipo_job_update_lookups', $vars);

			// Here after moving the asset away from pending accounts folder we check for the
			// permission/metadata/workflow of the new parent asset and cascade same permission
			// to the new user also if the hipo was not successfull - fullstop and rollback.
			foreach ($create_locations as $location_id => $link_type) {
				foreach (Array(SQ_PERMISSION_READ, SQ_PERMISSION_WRITE, SQ_PERMISSION_ADMIN) as $perm) {
					$set_perms = $GLOBALS['SQ_SYSTEM']->am->getAssetPermissionByCascade($location_id, $perm, NULL, TRUE);
					foreach ($set_perms as $perm_info) {
						if (!empty($hipo_empty_array) || !$GLOBALS['SQ_SYSTEM']->am->setPermission($user->id, $perm_info['userid'], $perm, $perm_info['granted'], TRUE, TRUE)) {
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
							$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
							return FALSE;
						}
					}
				}//end foreach

				$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
				$mm_schema =  $mm->getSchemas($location_id, NULL, TRUE);
				foreach ($mm_schema as $schema_id => $granted ) {
					if (!$mm->setSchema($user->id, $schema_id, $granted)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
						return FALSE;
					}//end if
				}//end foreach

				$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
				$wf_schema = $wfm->getSchemas($location_id, NULL, FALSE, TRUE);
				foreach ($wf_schema as $schema_id => $granted ) {
					if (!$wfm->setSchema($user->id, $schema_id, $granted)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
						return FALSE;
					}//end if
				}//end foreach
			}

			// all done
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
		return TRUE;

	}//end validateUser()


	/**
	* A Quick HIPO callback function that validates a user passed to it
	*
	* @param mixed	$index		The index of the item currently being processed
	* @param mixed	$item		The complete list of items to process
	* @param array	$settings	An array containing whatever settings this Quick HIPO requires
	*
	* @return boolean
	* @access public
	*/
	function hipoValidateUser($index, $item, $settings)
	{
		// load the user asset
		$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($item);
		if (is_null($user)) return FALSE;
		$validated = $this->validateUser($user);
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($user);
		return $validated;

	}//end hipoValidateUser()


	/**
	* A Quick HIPO callback function that deletes a user that is passetd to it
	*
	* @param mixed	$index		The index of the item currently being processed
	* @param mixed	$item		The complete list of items to process
	* @param mixed	$settings	An array containing whatever settings this Quick HIPO requires
	*
	* @return boolean
	* @access public
	*/
	function hipoDeleteUser($index, $item, $settings)
	{
		$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($item);
		$user->saveWebPaths(Array());
		$GLOBALS['SQ_SYSTEM']->am->trashAsset($item);

		// trick the user asset into thinking we're purging the
		// assets (which we are, just not from the trash), so it
		// will release its hold on its inbox and workspace
		$GLOBALS['SQ_PURGING_TRASH'] = TRUE;

			// Delete the inbox and remove its URLs
			$inbox_link = $user->getInboxLink();
			$inbox = $GLOBALS['SQ_SYSTEM']->am->getAsset($inbox_link['minorid']);
			$GLOBALS['SQ_SYSTEM']->am->trashAsset($inbox->id);
			$inbox->updateLookups();

			// Delete the workspace and remove its URLs
			$workspace = $user->getWorkspace();
			$workspace->saveWebPaths(Array());
			$GLOBALS['SQ_SYSTEM']->am->trashAsset($workspace->id);
			$workspace->updateLookups();

			$inbox->delete(FALSE);
			$workspace->delete(FALSE);

		$GLOBALS['SQ_PURGING_TRASH'] = FALSE;

		// Delete the account
		$user->delete(FALSE);
		unset($user);

		return TRUE;

	}//end hipoDeleteUser()


	/**
	* Re-sends validation email to specified user
	*
	* @param string	$username	The username of the user to resend the email for (optional)
	*
	* @return boolean
	* @access private
	*/
	function resendValidation($username=NULL)
	{
		if (is_null($username)) {
			if (!empty($_REQUEST[$this->getPrefix().'_resend_validation_username'])) {
				$username = $_REQUEST[$this->getPrefix().'_resend_validation_username'];
			}
		}

		if (!is_null($username) && $this->attr('use_email_validation')) {
				// add an email to the TO list for the user who just signed up

				$auth_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('authentication_folder');
				$auth_systems = $auth_folder->getAuthSystems();
				$user = NULL;
				foreach ($auth_systems as $systemid) {
					$system = $GLOBALS['SQ_SYSTEM']->am->getAsset($systemid);
					if (is_null($system)) continue;
					$user = $system->locateUser($username);
					if (!is_null($user) && $user->id) {
						break;
					}
				}

				if (is_null($user)) {
					$this->_errors[] = translate('cms_account_manager_keyword_resend_validation_user_not_found');
					return FALSE;

				} else {
					if ($validation_key = $this->getValidationKeyForUser($user)) {
						$replacements = Array();
						list($create_type) = array_keys($this->attr('create_type'));
						$attrs = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($create_type);
						foreach ($attrs as $attr => $type) {
							if (($attr == 'password') || ($type['type'] == 'serialise')) {
								continue;
							}
							$replacements[$attr] = $user->attr($attr);
						}
						$url = SQ_IN_CRON ? $this->getURL() : current_url();
						$replacements['validation_url'] = $url.'?action=validate&amid='.$validation_key;

						// add an email to the TO list for the user who just signed up
						$email_attr = $this->getAttribute('validation_email_format');
						$new_value = unserialize($email_attr->value);
						$new_value['to'][] = $replacements['email'];
						$new_email_attr = new Asset_Attribute_Email_Format(0, serialize($new_value));
						if (is_null($new_email_attr)) {
							$this->_errors[] = translate('cms_account_manager_keyword_resend_validation_cannot_send_user');
							return FALSE;
						}

						// let the email format attribute send the email for us
						$new_email_attr->sendMail($replacements);
						$this->_replacements['resend_validation_success'] = $this->getKeywordReplacement('re_send_validation_success');
						return TRUE;

					} else {
						$this->_errors[] = translate('cms_account_manager_keyword_resend_validation_cannot_send_user');
						return FALSE;
					}

				}//end else

		} else {
			$this->_errors[] = translate('cms_account_manager_keyword_resend_validation_cannot_send_user');
			return FALSE;
		}

		return FALSE;

	}//end resendValidation()


	/**
	* Returns the validation key for the supplied user.
	*
	* @param object	&$user	The user we want the validation key for
	*
	* @return string
	* @access public
	*/
	function getValidationKeyForUser(&$user)
	{
		$pending_usergroup_id = $this->_getPendingAccountsGroupId();
		$links = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($pending_usergroup_id, $user->id);

		// we should only have one link
		if (!empty($links)) return $links['value'];
		return FALSE;

	}//end getValidationKeyForUser()


	/**
	* Create the pending accounts user group
	*
	* @return boolean
	* @access protected
	*/
	function _createPendingAccountsGroup()
	{
		$group_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'sort_order' => 0, 'is_dependant' => 1, 'is_exclusive' => 1, 'value' => 'pending_accounts');
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('user_group');

		$group = new User_Group();
		$group->setAttrValue('name', 'Pending Accounts');
		if (!$group->create($group_link)) {
			return FALSE;
		}

		return TRUE;

	}//end _createPendingAccountsGroup()


	/**
	* Get the ID of the Pending Accounts group
	*
	* @return string
	* @access public
	*/
	function _getPendingAccountsGroupId()
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'user_group', TRUE, 'pending_accounts');
		if (empty($link)) {
			return 0;
		} else {
			return $link['minorid'];
		}

	}//end _getPendingAccountsGroupId()


	/**
	* Get the current pending user (for a request that came from a validation email)
	*
	* @return object
	* @access protected
	*/
	function &_getCurrentPendingUser()
	{
		$user = NULL;
		$id = (isset($_REQUEST['amid'])) ? trim($_REQUEST['amid']) : '';
		if (!empty($id)) {
			$pending_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'user_group', TRUE, 'pending_accounts');
			if (!empty($pending_link)) {
				$link = $GLOBALS['SQ_SYSTEM']->am->getLink($pending_link['minorid'], SQ_LINK_TYPE_1, 'user', FALSE, $id);
				if (!empty($link)) {
					$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
				}
			}
		}
		return $user;

	}//end _getCurrentPendingUser()


	/**
	* Remove a user from the pending-users group
	*
	* @param string	$userid	the assetid of the pending user to remove
	*
	* @return boolean
	* @access protected
	*/
	function _removePendingUserFromPendingGroup($userid)
	{
		$pending_accounts_groupid = $this->_getPendingAccountsGroupId();
		if (!empty($pending_accounts_groupid)) {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($pending_accounts_groupid, $userid, SQ_LINK_TYPE_1);
			if (!empty($link)) {
				$resent_link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($pending_accounts_groupid, $userid, SQ_LINK_NOTICE, 'validation_email_resent');
				if (!empty($resent_link)) $GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($resent_link['linkid']);
				return $GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($link['linkid']);
			}
		}
		return FALSE;

	}//end _removePendingUserFromPendingGroup()


	/**
	* Return an array of the currently pending users
	*
	* @return object
	* @access protected
	*/
	function _getPendingUserIds()
	{
		$users = Array();

		$group_folder_id = $this->_getPendingAccountsGroupId();
		if (!empty($group_folder_id)) {
			$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($group_folder_id, SQ_LINK_TYPE_1, 'user', FALSE);
			foreach ($links as $link) {
				$users[] = $link['minorid'];
			}
		}

		return $users;

	}//end _getPendingUserIds()


//--        KEYWORD DESCRIPTION        --//


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
			case 'not_logged_in' :
				$vars['keywords'] = array_merge($vars['keywords'], $this->_getNotLoggedInKeywords());
			break;
			case 'logged_in' :
				$vars['keywords'] = array_merge($vars['keywords'], $this->_getLoggedInKeywords());
			break;
			case 'create_login' :
			case 'create_no_login' :
				$vars['keywords'] = array_merge($vars['keywords'], $this->_getCreateLoginKeywords());
			break;

		}

		$vars['keywords'] = array_merge($vars['keywords'], $this->_getSelectableLocationKeywords());
		$vars['keywords'] = array_merge($vars['keywords'], $this->_getDynamicLocationKeywords());
	}//end onRequestKeywords()


	/**
	* Get the keyword replacements used in not_logged_in bodycopy
	*
	* @return array
	* @access protected
	*/
	function _getLoggedInKeywords()
	{
		$keywords = parent::_getLoggedInKeywords();

		// add new keywords
		$keywords['edit_details_invite']		= translate('cms_account_manager_keyword_edit_details_invite');
		$keywords['edit_details_form']			= translate('cms_account_manager_keyword_edit_details_form');
		$keywords['edit_details_success']		= translate('cms_account_manager_keyword_edit_details_success');
		$keywords['edit_details_lock_error']	= translate('cms_account_manager_keyword_edit_details_lock_error');

		$keywords['logout_form']				= translate('cms_asset_builder_keyword_logout_form');

		// Exclude CAPTCHA keywords and other SPAM prevention functionality keywords from the "Logged In" form. 
		// Presume if the user has logged in, that they are not an artificial intelligence
		if ($this->attr('require_captcha')) {
			unset($keywords['form_captcha']);
			unset($keywords['form_captcha_image']);
			unset($keywords['form_captcha_field']);
			unset($keywords['form_captcha_link']);
		}

		if ($this->attr('use_mollom')) {
			unset($keywords['asset_builder_mollom_captcha']);
			unset($keywords['asset_builder_mollom_captcha_image']);
			unset($keywords['asset_builder_mollom_captcha_audio']);
			unset($keywords['asset_builder_mollom_captcha_field']);
			unset($keywords['asset_builder_mollom_captcha_regen_link']);
		}
		if ($this->attr('use_recaptcha')) {
			unset($keywords['form_recaptcha']);
		}
		unset($keywords['form_honeypot_field']);
		
		return $keywords;

	}//end _getLoggedInKeywords()


	/**
	* Get the keyword replacements used in not_logged_in bodycopy
	*
	* @return array
	* @access protected
	*/
	function _getCreateLoginKeywords()
	{
		$keywords = parent::_getCreatedKeywords();

		// override parent's keywords
		$keywords['create_invite'] 			= translate('cms_account_manager_keyword_create_invite');
		$keywords['create_form'] 			= translate('cms_account_manager_keyword_create_form');
		$keywords['create_error'] 			= translate('cms_account_manager_keyword_create_error');
		$keywords['resend_validation_form']	= translate('cms_account_manager_keyword_resend_validation_form');

		return $keywords;

	}//end _getCreateLoginKeywords()


	/**
	* Get the keyword replacements used in not_logged_in bodycopy
	*
	* @return array
	* @access protected
	*/
	function _getNotLoggedInKeywords()
	{
		$keywords = parent::_getNotLoggedInKeywords();
		$keywords['resend_validation_error']	= translate('cms_account_manager_keyword_resend_validation_error');
		$keywords['resend_validation_form'] 	= translate('cms_account_manager_keyword_resend_validation_form');
		$keywords['resend_validation_success'] 	= translate('cms_account_manager_keyword_resend_validation_success');

		return $keywords;

	}//end _getNotLoggedInKeywords()


	//--        KEYWORD REPLACEMENT        --//


	/**
	* Get edit_details_lock_error keyword replacement
	*
	* @return string
	* @access public
	*/
	function getReSendValidationFormKeywordReplacement()
	{
		ob_start();
			text_box($this->getPrefix().'_resend_validation_username', '',20);
			submit_button('', $this->attr('resend_validation_button'), 'document.getElementById(\'AB_'.$this->id.'_ASSET_BUILDER_ACTION\').value = \'resend_validation\';');
		return ob_get_clean();

	}//end getReSendValidationFormKeywordReplacement()


	/**
	* Get edit_details_success keyword replacement
	*
	* @return string
	* @access public
	*/
	function getReSendValidationSuccessKeywordReplacement()
	{
		return $this->attr('resend_validation_success');

	}//end getReSendValidationSuccessKeywordReplacement()


	/**
	* Get login_form keyword replacement
	*
	* @return string
	* @access public
	*/
	function getLoginFormKeywordReplacement()
	{
		ob_start();
			?>
			<input type="hidden" name="SQ_LOGIN_ACCOUNT_MANAGER_STATE" value="<?php echo $this->_current_state; ?>" />
			<?php
		$output = ob_get_clean();

		return $output.parent::getLoginFormKeywordReplacement();

	}//end getLoginFormKeywordReplacement()


	/**
	* Get logout_form keyword replacement
	*
	* @return string
	* @access public
	*/
	function getLogoutFormKeywordReplacement()
	{
		ob_start();
			?>
			<form id="<?php echo $this->getPrefix() ?>_logout" method="POST" action="<?php echo current_url(TRUE, TRUE); ?>?SQ_ACTION=logout">
				<?php echo submit_button('logout', translate('logout')); ?>
			</form>
			<?php
			$replacement = ob_get_contents();

		ob_end_clean();
		return $replacement;

	}//end getLogoutFormKeywordReplacement()


	/**
	* Get logout_form keyword replacement
	*
	* @return string
	* @access public
	*/
	function printBody()
	{
		$asset = $GLOBALS['SQ_SYSTEM']->user;
		$this->_current_state = 'logged_in';
		$content = '';

		if (!is_null($asset) && !($asset instanceof Public_User) && $this->attr('customise_logged_in')) {

			// try and acquire a lock on the current user so they can edit their details
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array(
						'assetids'			=> Array($asset->id,),
						'lock_type'			=> 'all',
						'forceably_acquire'	=> FALSE,
					);
			$lock_errors = $hh->freestyleHipo('hipo_job_acquire_locks', $vars);
			$type_code_to_create  = array_keys($this->attr('create_type'));
			$layout = $this->_getLoggedInFormLayout($type_code_to_create[0]);
			$ei = $asset->getEI();
	        require_once SQ_INCLUDE_PATH.'/limbo_outputter.inc';
	        $o = new Limbo_Outputter();

			$errors = '';
			if (array_get_index($_POST, 'form_submitted', FALSE)) {
				$screens = Array();
				$keywords = $layout->getLayoutKeywords();

				if (isset($keywords['screens']) && !empty($keywords['screens'])) {
					$screens = array_unique(array_merge(array_keys($keywords['screens']), $screens));
				}

				if (isset($keywords['entire_screens']) && !empty($keywords['entire_screens'])) {
					$screens = array_unique(array_merge(array_keys($keywords['entire_screens']), $screens));
				}
				set_error_handler(Array(&$this, '_errorHandler'));
					foreach ($screens as $screen) {
						$ei->processInline($asset,$o, FALSE, $screen);
					}
				restore_error_handler();

				if (!empty($this->_errors)) {
					foreach($this->_errors as $text) {
						$errors .= '<li>'.$text.'</li>';
					}//end foreach
				}//end if
			}

			ob_start();

				if (!empty($lock_errors)) $this->_replacements['edit_details_lock_error'] = $this->attr('edit_details_lock_error');

				$o->_paintInlineHeader();

				$GLOBALS['SQ_SYSTEM']->setRunLevel($GLOBALS['SQ_SYSTEM']->getRunLevel() - SQ_SECURITY_PERMISSIONS);

				$o->addHiddenField('form_submitted', '1');
				$ei->paintLayout($asset, $o, $layout);
				$o->paint();

				// ie. they have permission to access the default screen for a user
				require_once SQ_INCLUDE_PATH.'/asset_edit_interface.inc';
				$aei = new Asset_Edit_Interface($asset->type());
				$allowed_screens = $aei->getAllowedScreens($asset);
				$default_screen = $aei->_default_screen;
				if (empty($default_screen)) {
					$default_screen = 'details';
				}

				$check_screen_restrictions = (!($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin()));
				if ($check_screen_restrictions) {
					if (empty($allowed_screens)) {
						$check_screen_restrictions = FALSE;
					}
				}

				if ($check_screen_restrictions && !isset($allowed_screens[$default_screen])) {
					$this->_replacements['edit_details_invite'] = '<p>'.translate('cms_account_manager_cannot_edit_details').'</p>';
				} else {
					$this->_replacements['edit_details_invite'] = $this->attr('edit_details_invite');
				}
				unset($aei);

				$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
				$content = ob_get_contents();
			ob_end_clean();

			if (!empty($errors)) {
				$this->_replacements['form_errors'] = $errors;
			} else if (array_get_index($_POST, 'process_form', FALSE) && empty($lock_errors)){
				$this->_replacements['edit_details_success'] = $this->attr('edit_details_success');
			}

			replace_keywords($content, $this->_replacements);

			echo $content;
		} else {
			parent::printBody();
		}

		return TRUE;

	}//end printBody()


	/**
	* Get the create form layout, if available, for a certain create type
	*
	* @param string $type_code  The type code to get the layout for
	*
	* @return object
	* @access private
	*/
	function &_getLoggedInFormLayout($type_code)
	{
		$res = NULL;
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'layout', TRUE, $type_code.'_logged_in_customisation');
		if (!empty($link)) {
			$res = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], 'layout');
		}

		return $res;

	}//end _getLoggedInFormLayout()


}//end class

?>
