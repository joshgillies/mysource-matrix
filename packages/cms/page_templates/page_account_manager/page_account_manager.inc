<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: page_account_manager.inc,v 1.13 2004/05/12 07:09:33 gsherwood Exp $
* $Name: not supported by cvs2svn $
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
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Account_Manager extends Page_Asset_Builder
{

	/**
	* An array of keywords that this asset makes available for use along with a default replacement
	* @var Array(keyword => Array('name' => short description, 'default' => default replacement))
	*/
	var $_available_keywords = Array(
								'login_invite',
								'login_form',
								'login_error',
								'create_invite',
								'create_form',
								'create_error',
								'edit_details_invite',
								'edit_details_form',
								'edit_details_success',
								'edit_details_lock_error',
								'logout_form',
								);

	/**
	* An array of bodycopies that need to be created
	* @var Array()
	*/
	var $_bodycopies = Array( 'Not Logged In'    => 'not_logged_in',
							  'Created Not Live' => 'create_no_login',
							  'Created Live'     => 'create_login',
							  'Logged In'        => 'logged_in',
							 );


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
	function saveAttributes($dont_run_updated=false)
	{
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!parent::saveAttributes($dont_run_updated)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// if we are using emailm validation - ensure that the validation assets are created
		if ($this->attr('use_email_validation')) {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'user_group', true, 'pending_accounts');
			if (empty($link)) {
				$group_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'sort_order' => 0, 'dependant' => 1, 'exclusive' => 1, 'value' => 'pending_accounts');
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('user_group');
				
				$group = new User_Group();
				$group->setAttrValue('name', 'Pending Accounts');
				if (!$group->create($group_link)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
			}
		}

		return true;

	}//end saveAttributes()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		parent::printBody();

		$current_user = &$GLOBALS['SQ_SYSTEM']->user;
		if (!is_null($current_user) && !SQ_IN_BACKEND) {
			// this person is logged in - so redirect them to limbo where they can edit their details
			$redirect_url = strip_url(current_url()).'/'.SQ_CONF_LIMBO_SUFFIX.'/?ignore_frames=1';
			?>
			<script language="Javascript">
				document.location = "<?php echo $redirect_url; ?>";
			</script>
			Click <a href="<?php echo $redirect_url; ?>">here</a> to edit your details.
			<?php
		}

	}//end printBody()


	/**
	* Called by the design to print the body of this asset when in limbo
	*
	* Standard page puts its bodycopy up for editing instead of itself
	*
	* @return void
	* @access public
	*/
	function printLimbo()
	{	
		$GLOBALS['SQ_SYSTEM']->backend->out->addFormActionGetVar('ignore_frames', '1', true);

		$current_user = &$GLOBALS['SQ_SYSTEM']->user;
		$this->_current_state = 'logged_in';

		if (!is_null($current_user)) {
			// try and acquire a lock on the current user so they can edit their details
			$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array('assetid' => $current_user->id, 'lock_type' => 'attributes', 'forceably_acquire' => false);
			$lock_errors = $hh->freestyleHipo('hipo_job_acquire_lock', $vars);


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
					if (empty($default_screen)) $default_screen = 'details';

					$check_screen_restrictions = (!($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin()));
					if ($check_screen_restrictions) if (isset($allowed_screens['0']['0'])) $check_screen_restrictions = false;
				
					if ($check_screen_restrictions && !isset($allowed_screens[$default_screen])) {
						$this->_replacements['edit_details_invite'] = '<p>You do not have access to edit your details.</p>';
						$current_user->printBody();
					} else {
						$this->_replacements['edit_details_invite'] = $this->attr('edit_details_invite');
						$current_user->printLimbo();
					}
					unset($aei);
				}
				$this->_replacements['edit_details_form'] = ob_get_contents();
			ob_end_clean();


			  ////////////////////
			 //  LOG OUT FORM  //
			////////////////////

			ob_start();
				?>
				<form name="account_manager_logout" method="POST" action="<?php echo current_url(true, true); ?>?SQ_ACTION=logout">
					<?php echo submit_button('logout', 'Logout'); ?>
				</form>
				<?php
				$this->_replacements['logout_form'] = ob_get_contents();
			ob_end_clean();
		}

		$this->_printContents();

	}//end printLimbo()


	/**
	* Process any global Account Manager actions
	*
	* This function builds on the Asset Builder one by auto filling the username field of
	* the login form that may be printed once the user account has been created.
	*
	* @return void
	* @access private
	* @see Page_Asset_Builder::_processGlobalActions()
	*/
	function _processGlobalActions()
	{
		// validate a pending user account
		if (isset($_GET['action'])) {
			switch (strtolower($_GET['action'])) {
				case 'validate' :
					$id = (isset($_REQUEST['amid'])) ? trim($_REQUEST['amid']) : '';
					$locs = (isset($_REQUEST['amlocs'])) ? trim($_REQUEST['amlocs']) : '';
					if (!empty($id)) {

						$pending_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'user_group', true, 'pending_accounts');
						if (!empty($pending_link)) {
							$link = $GLOBALS['SQ_SYSTEM']->am->getLink($pending_link['minorid'], SQ_LINK_TYPE_1, 'user', false, $id);
							if (!empty($link)) {
								$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);

								// populate an array of attribute values for the user so we can work
								// out the create locations for the user account
								$user_attributes = Array();
								$attrs = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($this->attr('create_type'));
								foreach ($attrs as $attr => $type) $user_attributes[$attr] = $user->attr($attr);
								
								$create_locations = parent::_getCreateLocations($user_attributes);
								$optional_locations = explode('-', $locs);
								$optional_locations = array_intersect($optional_locations, $this->attr('create_location_optional'));
								$create_locations = array_unique(array_merge($create_locations, $optional_locations));

								// set the install flag because we are creating an asset without being
								// logged in - so any write access checks will fail
								$GLOBALS['SQ_INSTALL'] = true;

									$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

									// link the user asset into the locations it is supposed to go
									foreach (array_values($create_locations) as $link_parentid) {
										$link_parentid = (int) $link_parentid;
	
										$link_parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link_parentid, '', true);
										if (is_null($link_parent)) continue;
										if (!$GLOBALS['SQ_SYSTEM']->am->createAssetLink($link_parent, $user, $this->attr('create_link_type'))) {
											$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
											return false;
										}
									}

									// remove the link from the pending accounts group
									if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($link['linkid'])) {
										$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
										return false;
									}
	
									// set the status of the asset
									if (!parent::_setCreateStatus($user)) {
										$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
										return false;
									}
	
									// all done
									$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

								$GLOBALS['SQ_INSTALL'] = false;
								
								$this->_current_state = 'create_login';
								$_POST['SQ_LOGIN_USERNAME'] = $user->attr('username');
								$this->_loadLoginReplacements();
								$GLOBALS['SQ_SYSTEM']->am->forgetAsset($user);
								return true;
							}
						}
					}
				break;
			}
		}


		// additional checks if we are using email validation
		if ($this->attr('use_email_validation')) {

			if (isset($_POST['ASSET_BUILDER_ACTION']) && strtolower($_POST['ASSET_BUILDER_ACTION']) == 'create') {

				// create the unique ID we will use to identify this new user if it gets created
				$this->_tmp['am_created_asset_key'] = '';

				$create_type = $this->attr('create_type');
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($create_type);
				$new_asset = new $create_type();
				$prefix = $new_asset->getPrefix();

				if (isset($_POST[$prefix.'_username'])) {
					// a username has been supplied
					$username = trim($_POST[$prefix.'_username']);
					if (!empty($username)) $this->_tmp['am_created_asset_key'] = md5(microtime().$username);
					else trigger_error('Please enter a username', E_USER_NOTICE);
				} else {
					trigger_error('Please enter a username', E_USER_NOTICE);
				}

				// we need an email to send to for email validation
				// so make sure there is one that we can use
				if (isset($_POST[$prefix.'_email'])) {
					$email = $_POST[$prefix.'_email'];
					$attribute = &$new_asset->getAttribute('email', true);
					if (empty($email)) {
						trigger_error('Please enter an email address', E_USER_NOTICE);
					} else if (!$attribute->validateValue($email)) {
						trigger_error('The email address supplied is not valid', E_USER_NOTICE);
					}
				} else {
					trigger_error('Please enter an email address', E_USER_NOTICE);
				}

				// dont bother doing any more processing if we have errors because we are
				// not going to be able to validate this user using email
				if (!empty($this->_errors)) return false;
			}

		}//end if using email validation


		parent::_processGlobalActions();


		if (isset($_POST['ASSET_BUILDER_ACTION'])) {
			switch (strtolower($_POST['ASSET_BUILDER_ACTION'])) {
				case 'create' :
					if (empty($this->_errors) && !empty($this->_tmp['created_asset'])) {
						if ($this->attr('use_email_validation')) {
							// we are validating this user using email, so send them one
							$replacements = Array();
							$attrs = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($this->attr('create_type'));
							foreach ($attrs as $attr => $type) {
								if ($attr == 'password') continue;
								$replacements[$attr] = $this->_tmp['created_asset']->attr($attr);
							}
							
							$validation_url = current_url().'?action=validate&amid='.$this->_tmp['am_created_asset_key'].'&amlocs=';
							
							$create_type = $this->attr('create_type');
							$GLOBALS['SQ_SYSTEM']->am->includeAsset($create_type);
							$new_asset = new $create_type();
							$prefix = $new_asset->getPrefix();
							unset($new_asset);

							// work out what optional create locations they have selected (if any)
							// NOTE: we dont add mandatory or rule based locations to the URL as these can be worked
							// out later once they activate their account
							// we need strict checking of the groups we do put on the URL so they cant add themselves to
							// and group they feel like
							$create_locations = array_intersect(parent::_getCreateLocations(), $this->attr('create_location_optional'));
							
							if (!empty($create_locations)) {
								foreach ($create_locations as $location) $validation_url .= $location.'-';
								$validation_url = trim($body, '-');
							}
							$replacements['validation_url'] = $validation_url;

							// add an email to the TO list for the user who just signed up
							$email_attr = $this->getAttribute('validation_email_format');
							$new_value = unserialize($email_attr->value);
							$new_value['to'][] = $replacements['email'];
							$email_attr->value = serialize($new_value);
							
							// let the email format attribute send the email for us
							$email_attr->sendMail($replacements);
						}

						// the new user was created and can log in, so make sure the page is in the right state
						if ($this->attr('create_status') >= SQ_STATUS_LIVE && !$this->attr('use_email_validation')) {
							$this->_current_state = 'create_login';
							$_POST['SQ_LOGIN_USERNAME'] = $this->_tmp['created_asset']->attr('username');
							$this->_loadLoginReplacements();
						} else {
							$this->_current_state = 'create_no_login';
						}
					}
				break;
			}
		}

	}//end _processGlobalActions()


	/**
	* Work out the create locations for a new asset
	*
	* @param array	$attributes	an array of attributes to use for rule comparison
	*							leave empty to use the submitted form vars
	*
	* @return array
	* @access private
	*/
	function _getCreateLocations($attributes=Array())
	{
		if ($this->attr('use_email_validation')) {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'user_group', true, 'pending_accounts');
			if (empty($link)) return Array();
			else return Array($link['minorid']);
		} else {
			return parent::_getCreateLocations($attributes);
		}

	}//end _getCreateLocations()


	/**
	* Construct the link array for creating the asset
	*
	* The returned link array does not contain the 'asset' index where the parent
	* asset normally goes because this is generated on the fly
	*
	* @return array
	* @access private
	*/
	function _getCreateLink()
	{
		$link = parent::_getCreateLink();
		$link['link_type'] = SQ_LINK_TYPE_1;
		if ($this->attr('use_email_validation')) $link['value'] = $this->_tmp['am_created_asset_key'];

		return $link;

	}//end _getCreateLink()


	/**
	* Sets the status of the newly created asset
	*
	* If we are using email validation we dont change the status of the new asset
	*
	* @param object Asset	&$new_asset		the newly created asset
	*
	* @return boolean
	* @access private
	*/
	function _setCreateStatus(&$new_asset)
	{
		if ($this->attr('use_email_validation')) return true;
		else return parent::_setCreateStatus($new_asset);

	}//end _setCreateStatus()


	/**
	* Processes a backend submission from this asset, returns true if all OK
	*
	* @param object Backend_Outputter	&$o
	* @param Array(string)				&$link	information used to create the initial link
	*
	* @return boolean
	* @access public
	*/
	function processBackend(&$o, &$link)
	{
		if (SQ_IN_LIMBO) {
			$current_user = &$GLOBALS['SQ_SYSTEM']->user;

			if (!is_null($current_user)) {
				if ($current_user->processBackend($o, $link)) {
					$this->_replacements['edit_details_success'] = $this->attr('edit_details_success');
				}
			}

			return true;
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
		if (SQ_IN_LIMBO) return is_a($GLOBALS['SQ_SYSTEM']->user, 'user');
		else return parent::backendAccess();

	}//end backendAccess()


	/**
	* Add valid keywords for this asset to an array of keywords when asked
	*
	* @param &object Asset	&$broadcaster	the asset that triggered the event
	* @param Array			$vars			the vars that get submitted by the broadcaster
	*										we add keywords to the $vars['keywords'] array
	*
	* @return boolean
	* @access private
	*/
	function onRequestKeywords(&$broadcaster, $vars=Array())
	{
		if (!isset($vars['keywords'])) return;

		$keywords = Array();
		foreach ($this->_available_keywords as $keyword) {
			$name = ucwords(str_replace('_', ' ', $keyword));
			$type = 'inline';
			if ($keyword == 'logout_form') {
				$default = '<input style="border: 1px solid #5A5A5A; background-color: #F4F4EB;" type="button" value="Logout">';
				$name = 'Logout Button';
				$type = 'special';
			} else if (substr($keyword, -5) == '_form') {
				$default = $name;
				$type = 'block';
			} else {
				$default = $this->attr($keyword);
			}
			$keywords[$keyword] = Array('name' => $name, 'default' => $default, 'type' => $type);
		}
		$vars['keywords'] = array_merge($vars['keywords'], $keywords);
		
	}//end onRequestKeywords()


}//end class

?>
