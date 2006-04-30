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
* $Id: page_account_manager.inc,v 1.44 2006/04/30 23:28:43 tbarrett Exp $
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
* @version $Revision: 1.44 $
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
	function Page_Account_Manager($assetid=0)
	{
		$this->Page_Asset_Builder($assetid);

	}//end constructor


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

		if (!parent::saveAttributes($dont_run_updated)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// if we are using email validation - ensure that the validation assets are created
		if ($this->attr('use_email_validation')) {
			$pending_accounts_groupid = $this->_getPendingAccountsGroupId();
			if (empty($pending_accounts_groupid)) {
				$this->_createPendingAccountsGroup();
			}
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
		$current_user =& $GLOBALS['SQ_SYSTEM']->user;
		if ((!is_null($current_user) && !is_a($current_user, 'public_user')) && !SQ_IN_LIMBO) {
			// this person is logged in - so redirect them to limbo where they can edit their details
			$redirect_url = strip_url($this->getURL()).'/'.SQ_CONF_LIMBO_SUFFIX.'/?ignore_frames=1';

			// BUT if they have just logged in via this page we might want to redirect them somewhere else...
			if (isset($_REQUEST['SQ_LOGIN_ACCOUNT_MANAGER_STATE']) && $_REQUEST['SQ_LOGIN_ACCOUNT_MANAGER_STATE'] == 'create_login') {
				$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', FALSE, 'login_redirect');
				if (!empty($link)) {
					// the manager is configured with a custom redirect location
					$redirect_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid']);
					$redirect_url = $redirect_asset->getURL();
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($redirect_asset);
				}
			}
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
		array_merge($this->_replacements, parent::_loadCreateReplacements());

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
		$GLOBALS['SQ_SYSTEM']->backend->out->addFormActionGetVar('ignore_frames', '1', TRUE);

		$current_user =& $GLOBALS['SQ_SYSTEM']->user;
		$this->_current_state = 'logged_in';

		if (!is_null($current_user) && !is_a($current_user, 'public_user')) {
			// try and acquire a lock on the current user so they can edit their details
			$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
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
					$this->_replacements['edit_details_lock_error'] = $this->getKeywordReplacement('edit_details_lock_error');
					$current_user->printBody();
				} else {


					// ensure that the user has access to edit their details
					// ie. they have permission to access the default screen for a user
					require_once SQ_INCLUDE_PATH.'/asset_edit_interface.inc';
					$aei =& new Asset_Edit_Interface($current_user->type());

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

		if (isset($_POST['ASSET_BUILDER_ACTION']) && $_POST['ASSET_BUILDER_ACTION'] == 'resend_validation') {
			return $this->resendValidation();
		}

		if (strtolower(array_get_index($_GET, 'action', '')) == 'validate') {
			// grab the user and create locations
			$user =& $this->_getCurrentPendingUser();
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
		if ($this->attr('use_email_validation') && isset($_POST['ASSET_BUILDER_ACTION']) && strtolower($_POST['ASSET_BUILDER_ACTION']) == 'create') {

			// create the unique ID we will use to identify this new user if it gets created
			$this->_tmp['am_created_asset_key'] = '';

			$GLOBALS['SQ_SYSTEM']->am->includeAsset($create_type);
			$new_asset =& new $create_type();

			$real_username =& $new_asset->getAttribute('username', TRUE);
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

		if (isset($_POST['ASSET_BUILDER_ACTION'])) {
			switch (strtolower($_POST['ASSET_BUILDER_ACTION'])) {
				case 'create' :
					if (empty($this->_errors) && !empty($this->_tmp['created_asset'])) {
						if ($this->attr('use_email_validation')) {
							// notice link the newly-created asset underneath the selected create locations,
							// so when we validate we know which create locations were selected
							$new_asset =& $this->_tmp['created_asset'];

							// manually parse the create locations, format into an array in the format Array(locationid => Array('name'=>code_name, 'link_type'=>link_type), ...)
							$selected_create_locations = Array();
							foreach ($this->attr('create_location_optional') as $set_name => $details) {
								$tmp_locations = Array();
								$code_name = $this->_sanitiseLocationSetName($set_name);
								if (isset($_POST[$code_name])) {
									if ($details['allow_multiple'] && is_array($_POST[$code_name])) {
										foreach ($_POST[$code_name] as $location_id) {
											// add it to the list
											$tmp_locations[] = $location_id;
										}
										// check the list options are valid
										$tmp_locations = array_intersect($tmp_locations, array_keys($details['locations']));
										// add them to the collection
										foreach ($tmp_locations as $location_id) {
											$selected_create_locations[$location_id] = Array(
																						'set_name'	=> $set_name,
																						'link_type'	=> (int)$details['link_type'],
																					   );
										}
									} else {
										// if it's valid, add it to the list
										if (in_array($_POST[$code_name], array_keys($details['locations']))) {
											$selected_create_locations[(int)$_POST[$code_name]] = (int)$details['link_type'];
										}
									}
								} else if (!$details['allow_empty']) {
									trigger_localised_error('CMS0036', E_USER_WARNING, $set_name);
									return FALSE;
								}
							}


							// use the formatted array, create the notice links
							if (!empty($selected_create_locations)) {
								$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

									$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
									$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

										foreach ($selected_create_locations as $locationid => $link_type) {
											$location =& $GLOBALS['SQ_SYSTEM']->am->getAsset($locationid, '', TRUE);
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
								if (($attr == 'password') || ($type == 'serialise')) {
									continue;
								}
								$replacements[$attr] = $new_asset->attr($attr);
							}

							$replacements['validation_url'] = current_url().'?action=validate&amid='.$this->_tmp['am_created_asset_key'];

							// add an email to the TO list for the user who just signed up
							$email_attr =& $this->getAttribute('validation_email_format');
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
	* @param string	$create_type	the type of asset we are getting create locations for
	*
	* @return array
	* @access private
	*/
	function getCreateLocations($create_type)
	{
		if ($this->attr('use_email_validation')) {
			$pending_groupid = $this->_getPendingAccountsGroupId();
			if ($pending_groupid) {
				return Array($pending_groupid => SQ_LINK_TYPE_1);
			} else {
				return Array();
			}
		} else {
			return parent::getCreateLocations($create_type);
		}

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
			$current_user =& $GLOBALS['SQ_SYSTEM']->user;

			if (!is_null($current_user) && !is_a($current_user, 'public_user')) {
				if ($current_user->processBackend($o, $link)) {
					$this->_replacements['edit_details_success'] = $this->getKeywordReplacement('edit_details_success');
				}
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
			return is_a($GLOBALS['SQ_SYSTEM']->user, 'user');
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
		return Array(
				'create_login'		=> translate('cms_account_manager_created_live'),
				'create_no_login'	=> translate('cms_account_manager_created_not_live'),
				'logged_in'			=> translate('logged_in'),
				'not_logged_in'		=> translate('not_logged_in'),
			   );

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
			// the link value should be in the format: 'PREFIX_selected_create_location-LINK_VALUE'
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

		// set the install run level because we are creating an asset without being
		// logged in - so any write access checks will fail
		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			// link the user asset into the locations it is supposed to go
			foreach ($create_locations as $link_parentid => $link_type) {
				$link_parent =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link_parentid, '', TRUE);
				if (is_null($link_parent)) continue;
				if (!$GLOBALS['SQ_SYSTEM']->am->createAssetLink($link_parent, $user, $link_type)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
					return FALSE;
				}
			}

			// remove the selectable create location notice links, if they exist
			foreach ($selected_create_locations as $link_parentid => $link_type) {
				$link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($link_parentid, $user->id, SQ_LINK_NOTICE);
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
		$user =& $GLOBALS['SQ_SYSTEM']->am->getAsset($item);
		if (is_null($user)) return FALSE;
		$validated = $this->validateUser($user);
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($user);
		return $validated;

	}//end hipoValidateUser()


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

				$auth_folder =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('authentication_folder');
				$auth_systems = $auth_folder->getAuthSystems();
				$user = NULL;
				foreach ($auth_systems as $systemid) {
					$system =& $GLOBALS['SQ_SYSTEM']->am->getAsset($systemid);
					if (is_null($system)) continue;
					$user =& $system->locateUser($username);
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
							if (($attr == 'password') || ($type == 'serialise')) {
								continue;
							}
							$replacements[$attr] = $user->attr($attr);
						}

						$replacements['validation_url'] = current_url().'?action=validate&amid='.$validation_key;

						// add an email to the TO list for the user who just signed up
						$email_attr =& $this->getAttribute('validation_email_format');
						$new_value = unserialize($email_attr->value);
						$new_value['to'][] = $replacements['email'];
						$email_attr->value = serialize($new_value);

						// let the email format attribute send the email for us
						$email_attr->sendMail($replacements);
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
	* @return void
	* @access protected
	*/
	function _createPendingAccountsGroup()
	{
		$group_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'sort_order' => 0, 'is_dependant' => 1, 'is_exclusive' => 1, 'value' => 'pending_accounts');
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('user_group');

		$group =& new User_Group();
		$group->setAttrValue('name', 'Pending Accounts');
		if (!$group->create($group_link)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

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
					$user =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
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
			submit_button('', $this->attr('resend_validation_button'), 'this.form.action = \''.$_SERVER['PHP_SELF'].'\'; document.getElementById(\'ASSET_BUILDER_ACTION\').value = \'resend_validation\';');
		return ob_get_clean();

	}//end getReSendValidationFormKeywordReplacement()


	/**
	* Get edit_details_lock_error keyword replacement
	*
	* @return string
	* @access public
	*/
	function getEditDetailsLockErrorKeywordReplacement()
	{
		return $this->attr('edit_details_lock_error');

	}//end getEditDetailsLockErrorKeywordReplacement()


	/**
	* Get edit_details_success keyword replacement
	*
	* @return string
	* @access public
	*/
	function getEditDetailsSuccessKeywordReplacement()
	{
		return $this->attr('edit_details_success');

	}//end getEditDetailsSuccessKeywordReplacement()


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


}//end class

?>
