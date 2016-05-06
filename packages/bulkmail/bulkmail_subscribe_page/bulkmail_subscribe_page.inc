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
* $Id: bulkmail_subscribe_page.inc,v 1.35 2013/03/04 22:44:09 akarelia Exp $
*
*/

require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_user_group_manager/page_user_group_manager.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once 'Mail.php';
require_once 'Mail/mime.php';
require_once SQ_FUDGE_PATH.'/recaptcha/recaptcha.inc';

/**
* Bulkmail_Subscribe_Page
*
* Purpose
*
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Rayne Ong <rong@squiz.net>
* @version $Revision: 1.35 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class  Bulkmail_Subscribe_Page extends Page_User_Group_Manager
{

	var $keywords = Array(
						'logged_in'		=> Array(
											'error_messages_list',
											'success_messages_list',
											'current_subscriptions',
											'subscriptions_picker',
											'unsubscriptions_picker',
											'user_name',
											'user_short_name',
											'user_email',
											'subscribe_user_name',
											'subscribe_email_address',
											'commit_button',
										   ),
						'not_logged_in'	=> Array(
											'error_messages_list',
											'success_messages_list',
											'subscriptions_picker',
											'unsubscriptions_picker',
											'subscribe_user_name',
											'subscribe_email_address',
											'commit_button',
										   ),
					);


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

	}//end constructor


	/**
	* Returns an array of all the permitted link types, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		return Array(
				SQ_LINK_TYPE_2	=> Array(
									'bodycopy'	=> Array(
													'card'		=> 'M',
													'exclusive'	=> FALSE,
												   ),
									'folder'	=> Array(
													'card'		=> 'M',
													'exclusive'	=> FALSE,
												   ),
								   ),
				SQ_LINK_NOTICE	=> Array(
									'design'	=> Array(
													'card'		=> 'M',
													'exclusive'	=> FALSE,
												   ),
									'paint_layout_page'	=> Array(
													'card'		=> 'M',
													'exclusive'	=> FALSE,
												   ),
								   ),
			   );

	}//end _getAllowedLinks()


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Search pages create a few bodycopy assets when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');

		$copy_link = Array(
						'asset'			=> &$this,
						'link_type'		=> SQ_LINK_TYPE_2,
						'is_dependant'	=> 1,
						'is_exclusive'	=> 1,
						'value'			=> 'unverified_users',
					 );

		$unverified_users = new Folder();
		$unverified_users->setAttrValue('name', 'Unverified Users');
		if (!$unverified_users->create($copy_link)) {
			return FALSE;
		}

		return TRUE;

	}//end _createAdditional()


	/**
	* Retrieves the unverified users folder
	*
	* @return object
	* @access private
	*/
	function &getUnverifiedUsersFolder()
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, 'unverified_users', 'major', '1');
		$folder = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		return $folder;

	}//end getUnverifiedUsersFolder()


	/**
	* Gets the assetid of the unique Bulkmail_User asset with an email of $value
	*
	* @param string	$value	the email address to look for (unique attribute value)
	*
	* @return int
	* @access private
	*/
	function getExistingBulkmailUserid($value)
	{
		try {
			$bind_vars['email'] = $value;
			$result = MatrixDAL::executeOne('bulkmail_package', 'getExistingBmailUser', $bind_vars);
		} catch (Exception $e) {
			throw new Exception("Unable to get existing bulkmail user due to database error: ".$e->getMessage());
		}

		if (!$result) return 0;

		return $result;

	}//end getExistingBulkmailUserid()


	/**
	* Gets a list of the valid mailing lists, so that we can check if people are
	* rewriting the GET vars to link themselves elsewhere
	*
	* @return array
	* @access private
	*/
	function getAvailableLocations()
	{
		$locations = $GLOBALS['SQ_SYSTEM']->am->getChildren($this->attr('root_node'), 'user_group');

		// check if we have statuses we want to filter out
		$statuses = $this->attr('statuses');
		if (!empty($statuses)) {
			$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($locations));
			foreach ($asset_info as $assetid => $info) {
				if (!in_array($info['status'], $statuses)) {
					unset($locations[$assetid]);
				}
			}
		}

		// if the root node is a user group and has the correct status, include it
		$available_locations = array_keys($locations);
		$root_node = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->attr('root_node'));
		if (($root_node instanceof User_Group) && (!empty($statuses) && in_array($root_node->status, $statuses))) {
			$available_locations[] = $root_node->id;
		}
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($root_node);

		$current_user = $GLOBALS['SQ_SYSTEM']->user;
		$is_public_user = $current_user instanceof Public_User;

		foreach ($available_locations as $index => $user_group_id) {
			$user_group = $GLOBALS['SQ_SYSTEM']->am->getAsset($user_group_id);
			if(($is_public_user && !$user_group->readAccess()) || (!$is_public_user && !$user_group->writeAccess())) {
				unset($available_locations[$index]);
			}

		}
		return $available_locations;

	}//end getAvailableLocations()


	/**
	* This is called before the bodycopy contents are printed. It decides which
	* functions to call based on whether the user is logged in or not (or has
	* has clicked on a link in a verification email)
	*
	* @return NULL
	* @access private
	*/
	function processForm()
	{
		$public_user = $GLOBALS['SQ_SYSTEM']->getPublicUser();
		$email_address = array_get_index($_REQUEST, $this->getPrefix().'_email_address', '');

		// Any user attempting verification
		if (!empty($_GET['uniqid'])) {
			$this->processVerification();
		}

		$subscriptions = array_get_index($_REQUEST, $this->getPrefix().'_subscriptions', Array());
		$unsubscriptions = array_get_index($_REQUEST, $this->getPrefix().'_unsubscriptions', Array());
		if((!empty($subscriptions) || !empty($unsubscriptions)) && $GLOBALS['SQ_SYSTEM']->currentUser($public_user)) {
			// verify captcha
			if ($this->attr('require_captcha')) {
				if (!validate_security_key()) {
					$error_text = $this->attr('captcha_invalid_error_message');
					$this->errors[] = $error_text;
					return;
				}
			}

			// reCAPTCHA check
			if ($this->attr('use_recaptcha')) {
				$recaptcha_response = Recaptcha::verifyRecaptcha();
				if (!$recaptcha_response->is_valid) {
					$error_msg = $this->attr('recaptcha_error_message');
					if (empty($error_msg)) {
						$error_msg = array_shift($recaptcha_response->error);
					}
					$this->errors[] = $error_msg;
					return;
				}
			}//end if

			// Honeypot question validation
			$name = $this->attr('honeypot_name');
			if (empty($name)) {
				$name = 'q'.$this->id.':comment';
			}
			if (!empty($_POST[$name])) {
				$this->errors[] = $this->attr('honeypot_error_message');
				return;
			}//end if
		}

		// Public user
		if ($GLOBALS['SQ_SYSTEM']->currentUser($public_user)) {
			$this->processPublicUserForm();

		// Matrix User
		} else {
			$current_user = $GLOBALS['SQ_SYSTEM']->user;
			$current_user_email = $current_user->attr('email');

			// The user is logged in, but wants to subscribe/unsubscribe a different email address
			if (!empty($email_address) && ($email_address != $current_user_email)) {
				$this->processPublicUserForm();

			// Subscribe the logged in user to the list
			} else {
				$this->processMatrixUserForm();
			}
		}

	}//end processForm()


	/**
	* Process a verification request. This will get called whenever the GET vars
	* indicate that the user has done a click-through from a verification email.
	*
	* @return NULL
	* @access private
	*/
	function processVerification()
	{
		$uniqid = array_get_index($_GET, 'uniqid', '');
		if (empty($uniqid)) return;

		$subscriptions = array_get_index($_GET, 'subscribe', Array());
		$unsubscriptions = array_get_index($_GET, 'unsubscribe', Array());
		$subscriptions = empty($subscriptions) ? Array() : explode(',', $subscriptions);
		$unsubscriptions = empty($unsubscriptions) ? Array() : explode(',', $unsubscriptions);

		if (empty($subscriptions) && empty($unsubscriptions)) {
			$this->errors[] = translate('No subscription/unsubscription selected');

			return;
		}

		// OK, let's check if there is a user linked under the unverified users
		// folder with $uniqid as the link value

		$unverified_folder = $this->getUnverifiedUsersFolder();
		$type_codes = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('user', TRUE);
		$type_codes[] = 'bulkmail_user';
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($unverified_folder->id, NULL, $type_codes, TRUE, $uniqid);

		if (empty($link)) {
			// Validating and sub/unsubscribing straight after will probably be
			// confusing if we don't just return silently
			// $this->errors[] = 'We have no record of your email address. You may need to subscribe again.';
			return;
		}

		$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid']);

		// This is to trip up all the sneaky bastards who want to
		// try and re-write the URL to link themselves under other groups
		$all_mailing_lists = $this->getAvailableLocations();
		$parents = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($user->id, 'user_group', FALSE));
		$current_subscriptions = array_intersect($all_mailing_lists, $parents);

		// if the 'Show Root Node' is set to yes and if it was selected then we wont find the individual assetids
		// for the subscription and unsubscriptions. see bug #5705 Problem with "all" on Bulkmail subscribe page
		if ($this->attr('include_root_node')) {
			if (in_array($this->attr('root_node'), $subscriptions)) $subscriptions = $all_mailing_lists;
			if (in_array($this->attr('root_node'), $unsubscriptions)) $unsubscriptions = $all_mailing_lists;
		}

		$bad_subscriptions = array_diff($subscriptions, $all_mailing_lists);
		$bad_unsubscriptions = array_diff($unsubscriptions, $all_mailing_lists);

		$good_subscriptions = array_diff($subscriptions, $bad_subscriptions);
		$good_unsubscriptions = array_diff($unsubscriptions, $bad_unsubscriptions);

		$curr_plus_new = array_merge($current_subscriptions, $good_subscriptions);
		$new_minus_old = array_diff($curr_plus_new, $good_unsubscriptions);
		$new_subscriptions = array_unique($new_minus_old);

		if (!empty($bad_subscriptions)) {
			$this->errors[] = translate('You have attempted to subscribe to lists that are unavailable to you - these lists will be skipped.');

		}

		if (!empty($bad_unsubscriptions)) {
			$this->errors[] = translate('You have attempted to unsubscribe from lists that are unavailable to you - these lists will be skipped.');

		}

		// Check if the subscriptions are empty because otherwise
		// the Bulkmail_User will just go straight to the trash
		if (!empty($good_subscriptions) || !empty($good_unsubscriptions)) {
			$current_subscriptions[] = $unverified_folder->id;
			$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED + SQ_SECURITY_LINK_INTEGRITY);
				if (!empty($good_subscriptions))
					$done = $this->processMinorLinks($link['minorid'], $current_subscriptions, $new_subscriptions, $this->attr('link_type'), $uniqid);
				else
					$done = $this->processMinorLinks($link['minorid'], $current_subscriptions, $new_subscriptions, $this->attr('link_type'));
			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

			// OK, let's get rid of the $unverifed_folder->id from
			// $done so that it doesn't get shown to the frontend user
			foreach ($done as $done_type => $done_ids) {
				foreach ($done_ids as $index => $done_id) {
					if ($done_id == $unverified_folder->id) {
						unset($done[$done_type][$index]);
					}
					if (empty($done[$done_type])) {
						unset($done[$done_type]);
					}
				}
			}

			$this->addLinkingMessages($done);

		} else {
			$this->errors[] = translate('None of the subscriptions/unsubscriptions specified are valid.');

		}

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($unverified_folder);

	}//end processVerification()


	/**
	* Process a sub/unsubscribe request from a public user (ie, someone that just
	* wants to key in their email address and hit subscribe / unsubscribe), or a
	* logged in user who has specified a different email address
	*
	* @return NULL
	* @access private
	*/
	function processPublicUserForm()
	{
		$email_address = array_get_index($_REQUEST, $this->getPrefix().'_email_address', '');
		$user_name = htmlspecialchars(array_get_index($_REQUEST, $this->getPrefix().'_user_name', ''));

		if (empty($email_address)) {
			if (isset($_REQUEST[$this->getPrefix().'_email_address'])) {
				$this->errors[] = translate('Email address was not provided');

			}
			return;
		}

		$available_user_groups = $this->getAvailableLocations();

		$subscriptions = array_get_index($_REQUEST, $this->getPrefix().'_subscriptions', Array());
		$unsubscriptions = array_get_index($_REQUEST, $this->getPrefix().'_unsubscriptions', Array());

		// Flag to check if the subscription request is made
		$subscription_request = FALSE;

		if (!empty($subscriptions))
				$subscription_request = TRUE;
		// if the values are not passed as an array, force it into an array
		if (!is_array($subscriptions)) {
			if (empty($subscriptions)) {
				$subscriptions = Array();
			} else {
				$subscriptions = Array($subscriptions);
			}
		}
		if (!is_array($unsubscriptions)) {
			if (empty($unsubscriptions)) {
				$unsubscriptions = Array();
			} else {
				$unsubscriptions = Array($unsubscriptions);
			}
		}
		if (empty($subscriptions) && empty($unsubscriptions)) {
			$this->errors[] = translate('No subscription/unsubscription selected');

			return;
		}


		// check user read access to those subscriptions
		$bad_subscriptions = array_diff($subscriptions, $available_user_groups);
		$bad_unsubscriptions = array_diff($unsubscriptions, $available_user_groups);
		if(!empty($bad_subscriptions) || !empty($bad_unsubscriptions)) {
			return;
		}


		switch ($this->attr('verify_not_logged_in')) {
			case 'always':
				$sub_verify = !empty($subscriptions);
				$unsub_verify = !empty($unsubscriptions);
			break;
			case 'subscribe':
				$sub_verify = !empty($subscriptions);
				$unsub_verify = FALSE;
			break;
			case 'unsubscribe':
				$sub_verify = FALSE;
				$unsub_verify = !empty($unsubscriptions);
			break;
			case 'never':
			default:
				$sub_verify = $unsub_verify = FALSE;
			break;
		}

		$unverified_folder = $this->getUnverifiedUsersFolder();
		$uniqid = md5(uniqid(''));

		// Get the bulkmail user from email address
		$bulk_userid = $this->getExistingBulkmailUserid($email_address);

		// If exists in system, check if user is trying to subscribe to a already subscribed group.
		if ($bulk_userid) {

			
			$user_parents = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($bulk_userid, 'user_group', FALSE));
			$currently_subscribed_groups = array_intersect($available_user_groups, $user_parents);

			// While subscribing, if user has subscribed to this group previously, don't process further
			if (array_intersect($subscriptions, $currently_subscribed_groups)) {
				// However display a message of a bogus conformation email being sent (for security reasons)
				if ($sub_verify === TRUE) {
					$from_addr = $this->attr('verify_from_address');
					$from_addr = empty($from_addr) ? SQ_CONF_DEFAULT_EMAIL : $from_addr;
					if (empty($from_addr)) {
						trigger_localised_error('SYS0315', translate('System Default email address has not been set.'), E_USER_WARNING);
						trigger_localised_error('BML0016', translate('Cannot send the verification email'), E_USER_WARNING);
					}
				}
				$this->messages[] = sprintf(translate('A verification email has been sent to %s. When you receive the email, please follow the instructions carefully.'), $email_address);

				return;
			}
		}

		if ($sub_verify || $unsub_verify) {
			$from_addr = $this->attr('verify_from_address');
			$from_addr = empty($from_addr) ? SQ_CONF_DEFAULT_EMAIL : $from_addr;
			if (empty($from_addr)) {
				trigger_localised_error('SYS0315', translate('System Default email address has not been set.'), E_USER_WARNING);
				trigger_localised_error('BML0016', translate('Cannot send the verification email'), E_USER_WARNING);
			} else {
				require_once SQ_FUDGE_PATH.'/general/www.inc';
				$_email_address_trim = trim($email_address);
				// if the email is valid send a confirmation
				if (valid_email($_email_address_trim) && !empty($_email_address_trim)) {
					$this->sendConfirmationEmail($email_address, $from_addr, $subscriptions, $unsubscriptions, $uniqid);
					$this->messages[] = sprintf(translate('A verification email has been sent to %s. When you receive the email, please follow the instructions carefully.'), $email_address);

				}
			}
		}

		// We need to verify both subscribing and unsubscribing
		if ($sub_verify && $unsub_verify) {
			$subscriptions = Array($unverified_folder->id);
			$unsubscriptions = Array();

		// Only verify subscriptions
		} else if ($sub_verify) {
			$subscriptions = Array($unverified_folder->id);

		// Only verify unsubscriptions
		} else if ($unsub_verify) {
			$subscriptions[] = $unverified_folder->id;
			$unsubscriptions = Array();
		}

		// The user doesn't exist at all, so we are going to create it
		if ((empty($bulk_userid)) && !empty($subscriptions) && $subscription_request) {
			// We should clear the unsubscriptions considering you can't unsubscribe
			// if you don't exist
			$unsubscriptions = Array();

			$GLOBALS['SQ_SYSTEM']->am->includeAsset('bulkmail_user');
			$bulkmail_user = new Bulkmail_User();

			$parent_assetid = array_shift($subscriptions);
			$parent_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($parent_assetid);
			$link_value = $sub_verify ? $uniqid : '';
			$link = Array(
						'asset'		=> $parent_asset,
						'link_type'	=> $this->attr('link_type'),
						'value'		=> $link_value,
					);

			$creation_succeed = FALSE;
			$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED + SQ_SECURITY_LINK_INTEGRITY);

				// if we could not set the email add a error
				if (!$bulkmail_user->setAttrValue('email', $email_address)) {
					$this->errors[] = translate('Email address provided is invalid');

				}

				// if we could not set the name add a error
				if (!$bulkmail_user->setAttrValue('name', $user_name)) {
					$this->errors[] = translate('Username provided is invalid');

				}
				$bulkmail_user->create($link);
				// if create succeed processMinor Links
				if (!empty($bulkmail_user->id)) {
					$done = $this->processMinorLinks($bulkmail_user->id, Array(), $subscriptions, $this->attr('link_type'));
					$creation_succeed = TRUE;
				}
			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

			if ($creation_succeed) {
				if ($parent_asset->id != $unverified_folder->id) {
					$done['add_success'][] = $parent_assetid;
				}
				$this->addLinkingMessages($done);
			}

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($parent_asset);
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bulkmail_user);
		// The user already exists, so we need to shift them in and out of whatever groups they have selected
		// and thats only if user is not already subscribed to the group
		} else if ($bulk_userid) {
			$bulkmail_user = $GLOBALS['SQ_SYSTEM']->am->getAsset($bulk_userid, 'bulkmail_user');

			foreach ($subscriptions as $index => $assetid) {
				if ($assetid == $unverified_folder->id) {
					// We need to manually create the link to the unverified users folder here
					// because it needs a link value
					$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
						$GLOBALS['SQ_SYSTEM']->am->createAssetLink($unverified_folder, $bulkmail_user, $this->attr('link_type'), $uniqid);
					$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
					unset ($subscriptions[$index]);
				}
			}

			$all_mailing_lists = $this->getAvailableLocations();
			$parents = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($bulkmail_user->id, 'user_group', FALSE));
			$current_subscriptions = array_intersect($all_mailing_lists, $parents);

			$curr_plus_new = array_merge($current_subscriptions, $subscriptions);
			$new_minus_old = array_diff($curr_plus_new, $unsubscriptions);
			$new_subscriptions = array_unique($new_minus_old);

			$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED + SQ_SECURITY_LINK_INTEGRITY);
				$done = $this->processMinorLinks($bulkmail_user->id, $current_subscriptions, $new_subscriptions, $this->attr('link_type'));
			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

			$this->addLinkingMessages($done);
		}

	}//end processPublicUserForm()


	/**
	* Process a sub/unsubscribe request from logged in user
	*
	* @return NULL
	* @access private
	*/
	function processMatrixUserForm()
	{
		$subscriptions = array_get_index($_REQUEST, $this->getPrefix().'_subscriptions', Array());
		$unsubscriptions = array_get_index($_REQUEST, $this->getPrefix().'_unsubscriptions', Array());
		$available_user_groups = $this->getAvailableLocations();

		// if the values are not passed as an array, force it into an array
		if (!is_array($subscriptions)) {
			if (empty($subscriptions)) {
				$subscriptions = Array();
			} else {
				$subscriptions = Array($subscriptions);
			}
		}
		if (!is_array($unsubscriptions)) {
			if (empty($unsubscriptions)) {
				$unsubscriptions = Array();
			} else {
				$unsubscriptions = Array($unsubscriptions);
			}
		}
		if (empty($subscriptions) && empty($unsubscriptions)) {
			return;
		}


		// check user read access to those subscriptions
		$bad_subscriptions = array_diff($subscriptions, $available_user_groups);
		$bad_unsubscriptions = array_diff($unsubscriptions, $available_user_groups);
		if(!empty($bad_subscriptions) || !empty($bad_unsubscriptions)) {
			return;
		}

		switch ($this->attr('verify_logged_in')) {
			case 'always':
				$sub_verify = !empty($subscriptions);
				$unsub_verify = !empty($unsubscriptions);
			break;
			case 'subscribe':
				$sub_verify = !empty($subscriptions);
				$unsub_verify = FALSE;
			break;
			case 'unsubscribe':
				$sub_verify = FALSE;
				$unsub_verify = !empty($unsubscriptions);
			break;
			case 'never':
			default:
				$sub_verify = $unsub_verify = FALSE;
			break;
		}

		$unverified_folder = $this->getUnverifiedUsersFolder();
		$uniqid = md5(uniqid(''));

		$current_user = $GLOBALS['SQ_SYSTEM']->user;
		$current_user_email = $current_user->attr('email');
		if (empty($current_user_email)) {
			$this->errors[] = translate('You do not currently have an email address configured for your user.');

			return;
		}

		// If subscription already exists, then reject the subscription
		$user_parents = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($current_user->id, 'user_group', FALSE));
		$currently_subscribed_groups = array_intersect($available_user_groups, $user_parents);

		// While subscribing, if user has subscribed to this group previously, don't process further
		if (array_intersect($subscriptions, $currently_subscribed_groups)) {
			// However display a message of a bogus conformation email being sent (for security reasons)
			// Behaviour for a logged in user is to show nothing if subs are not verified, though.
			if ($sub_verify === TRUE) {
				$from_addr = $this->attr('verify_from_address');
				$from_addr = empty($from_addr) ? SQ_CONF_DEFAULT_EMAIL : $from_addr;
				if (empty($from_addr)) {
					trigger_localised_error('SYS0315', translate('System Default email address has not been set.'), E_USER_WARNING);
					trigger_localised_error('BML0016', translate('Cannot send the verification email'), E_USER_WARNING);
				} else {
					$this->messages[] = sprintf(translate('A verification email has been sent to %s. When you receive the email, please follow the instructions carefully.'), $current_user_email);

				}
			}
			return;
		}

		if ($sub_verify || $unsub_verify) {
			$from_addr = $this->attr('verify_from_address');
			$from_addr = empty($from_addr) ? SQ_CONF_DEFAULT_EMAIL : $from_addr;
			if (empty($from_addr)) {
				trigger_localised_error('SYS0315', translate('System Default email address has not been set.'), E_USER_WARNING);
				trigger_localised_error('BML0016', translate('Cannot send the verification email'), E_USER_WARNING);
			} else {
				$this->sendConfirmationEmail($current_user_email, $from_addr, $subscriptions, $unsubscriptions, $uniqid);
				$this->messages[] = sprintf(translate('A verification email has been sent to %s. When you receive the email, please follow the instructions carefully.'), $current_user_email);

			}
		}

		// We need to verify both subscribing and unsubscribing
		if ($sub_verify && $unsub_verify) {
			$subscriptions = Array($unverified_folder->id);
			$unsubscriptions = Array();

		// Only verify subscriptions
		} else if ($sub_verify) {
			$subscriptions = Array($unverified_folder->id);

		// Only verify unsubscriptions
		} else if ($unsub_verify) {
			$subscriptions[] = $unverified_folder->id;
			$unsubscriptions = Array();
		}

		foreach ($subscriptions as $index => $assetid) {
			if ($assetid == $unverified_folder->id) {
				// We need to manually create the link to the unverified users folder here
				// because it needs a link value
				$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
					$link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($unverified_folder->id, $current_user->id);
					if (!empty($link)) {
						$GLOBALS['SQ_SYSTEM']->am->updateLink($link['linkid'], $link['link_type'], $uniqid);
					} else {
						$GLOBALS['SQ_SYSTEM']->am->createAssetLink($unverified_folder, $current_user, $this->attr('link_type'), $uniqid);
					}
				$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
				unset ($subscriptions[$index]);
			}
		}

		$all_mailing_lists = $this->getAvailableLocations();
		$parents = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($current_user->id, 'user_group', FALSE));
		$current_subscriptions = array_intersect($all_mailing_lists, $parents);

		$curr_plus_new = array_merge($current_subscriptions, $subscriptions);
		$new_minus_old = array_diff($curr_plus_new, $unsubscriptions);
		$new_subscriptions = array_unique($new_minus_old);

		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED + SQ_SECURITY_LINK_INTEGRITY);
			$done = $this->processMinorLinks($current_user->id, $current_subscriptions, $new_subscriptions, $this->attr('link_type'));
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

		$this->addLinkingMessages($done);

	}//end processMatrixUserForm()


	/**
	* Takes an associative array of assetids and writes relevant error/success
	* messages to $this->[errors|messages] for later formatting
	*
	* @param array	$fail_success	associative array with failed and successful assetids
	*
	* @return void
	* @access public
	*/
	function addLinkingMessages($fail_success)
	{
		foreach ($fail_success as $message_type => $assetids) {
			switch ($message_type) {
				case 'add_fail':
					$this->errors[] = translate('Unable to subscribe you to the following mailing lists:').$this->_getCommaSeparatedNamesByAssetid($assetids);

				break;
				case 'add_success':
					$this->messages[] = translate('You have been subscribed to the following mailing lists:').$this->_getCommaSeparatedNamesByAssetid($assetids);

				break;
				case 'delete_fail':
					$this->errors[] = translate('Unable to unsubscribe you from the following mailing lists:').$this->_getCommaSeparatedNamesByAssetid($assetids);

				break;
				case 'delete_success':
					$this->messages[] = translate('You have been unsubscribed from the following mailing lists:').$this->_getCommaSeparatedNamesByAssetid($assetids);

				break;
			}
		}

	}//end addLinkingMessages()


	/**
	* Send an email to the user asking them to verify their subscription /
	* unsubscription request
	*
	* @param string	$to					Recipent email address (either 'user@example.com' or 'User <user@example.com>')
	* @param string	$from				Sending email address (formatted as above)
	* @param array	$subscriptions		Assetids the user wanted to link under
	* @param array	$unsubscriptions	Assetids the user wanted to remove himself from
	* @param array	$uniqid				Unique identifier (value of the link between user & unverified user folder)
	*
	* @return void
	* @access public
	*/
	function sendConfirmationEmail($to, $from, $subscriptions, $unsubscriptions, $uniqid)
	{
		if (empty($subscriptions) && empty($unsubscriptions)) {
			return;
		}

		$custom_subject = $this->attr('verify_subject_line');
		$headers = Array(
					'From'		=> $from,
					'Subject'	=> empty($custom_subject) ? translate('Please confirm your subscription / unsubscription request') : $this->attr('verify_subject_line'),

				   );

		$body = '';

		$get_vars = Array(
						'email'			=> $to,
						'subscribe'		=> implode(',', $subscriptions),
						'unsubscribe'	=> implode(',', $unsubscriptions),
						'uniqid'		=> $uniqid,
					);

		$url = current_url().'?';
		foreach ($get_vars as $key => $value) {
			$url .= '&'.$key.'='.$value;
		}

		if (!empty($subscriptions) && !empty($unsubscriptions)) {
			$body .= sprintf(translate('You have requested to subscribe to %1$s and unsubscribe from %2$s.'), $this->_getCommaSeparatedNamesByAssetid($subscriptions), $this->_getCommaSeparatedNamesByAssetid($unsubscriptions));

			$body .= sprintf(translate('To confirm your request, click the link below:

%s'), $url);

		} else if (!empty($subscriptions)) {
			$body .= sprintf(translate('Thank you for subscribing to %s.'), $this->_getCommaSeparatedNamesByAssetid($subscriptions));

			$body .= sprintf(translate('To activate your subscription, click the link below:

%s'), $url);

		} else if (!empty($unsubscriptions)) {
			$body .= sprintf(translate('You have requested to unsubscribe from %s.'), $this->_getCommaSeparatedNamesByAssetid($unsubscriptions));

			$body .= sprintf(translate('To confirm your request, click the link below:

%s'), $url);

		}
			$body .= sprintf(translate('

If you have received this email in error, please take no further action or contact %s.'), $from);



		$mime = new Mail_mime("\n");
		$mime->setTXTBody($body);
		$param = Array(
						'head_charset'	=> SQ_CONF_DEFAULT_CHARACTER_SET,
						'text_charset'	=> SQ_CONF_DEFAULT_CHARACTER_SET,
						'html_charset'	=> SQ_CONF_DEFAULT_CHARACTER_SET,
					  );
		$body = @$mime->get($param);
		$headers = @$mime->headers($headers);

		$mf = new Mail();
		$mail = $mf->factory('mail', "-f$from");
		$status = @$mail->send($to, $headers, $body);
		if (PEAR::isError($status)) {
			// unable to send verification email
			trigger_localised_error('BML0018', translate('Unable to send verification email. Please contact your administrator.'), E_USER_WARNING);
		}

		return;

	}//end sendConfirmationEmail()


	/**
	* Send email stating that attempt was made to subscribe them to lists they
	* are already a part of
	*
	* @param string	$to					Recipient email address (either 'user@example.com' or 'User <user@example.com>')
	* @param string	$from				Sending email address (formatted as above)
	* @param array	$subscriptions		Assetids the user wanted to link under
	*
	* @return void
	* @access public
	*/
	function sendAlreadySubscribedEmail($to, $from, $subscriptions)
	{
		if (empty($subscriptions)) {
			return;
		}

		$custom_subject = $this->attr('verify_subject_line');
		$headers = Array(
					'From'		=> $from,
					'Subject'	=> empty($custom_subject) ? translate('Please confirm your subscription / unsubscription request') : $this->attr('verify_subject_line'),

				   );

		$url = current_url();
		$body = sprintf(translate('You have requested to subscribe to %1$s.
This request was made from the following URL: %2$s

Your email address is already subscribed, therefore no action has been taken.'), $this->_getCommaSeparatedNamesByAssetid($subscriptions), $url);


		$mime = new Mail_mime("\n");
		$mime->setTXTBody($body);
		$param = Array(
						'head_charset'	=> SQ_CONF_DEFAULT_CHARACTER_SET,
						'text_charset'	=> SQ_CONF_DEFAULT_CHARACTER_SET,
						'html_charset'	=> SQ_CONF_DEFAULT_CHARACTER_SET,
					  );
		$body = @$mime->get($param);
		$headers = @$mime->headers($headers);

		$mf = new Mail();
		$mail = $mf->factory('mail', "-f$from");
		$status = $mail->send($to, $headers, $body);
		if (PEAR::isError($status)) {
			// unable to send verification email
			trigger_localised_error('BML0018', translate('Unable to send verification email. Please contact your administrator.'), E_USER_WARNING);
		}

		return;

	}//end sendAlreadySubscribedEmail()


	/**
	* Returns replacement for the subscribe_email_address keyword.
	*
	* @return string
	* @access private
	*/
	function getSubscribeEmailAddressKeywordReplacement()
	{
		ob_start();
			text_box($this->getPrefix().'_email_address', '', 30);
			$email_address = ob_get_contents();
		ob_end_clean();
		return $email_address;

	}//end getSubscribeEmailAddressKeywordReplacement()


	/**
	* Returns replacement for the subscribe_email_address keyword.
	*
	* @return string
	* @access private
	*/
	function getSubscribeUserNameKeywordReplacement()
	{
		ob_start();
			text_box($this->getPrefix().'_user_name', '', 30);
			$user_name = ob_get_contents();
		ob_end_clean();
		return $user_name;

	}//end getSubscribeUserNameKeywordReplacement()


	/**
	* Returns replacement for the subscription_picker keyword.
	*
	* @return string
	* @access private
	*/
	function getSubscriptionsPickerKeywordReplacement()
	{
		return $this->printAvailableLocations('_subscriptions');

	}//end getSubscriptionsPickerKeywordReplacement()


	/**
	* Returns replacement for the subscription_picker keyword.
	*
	* @return string
	* @access private
	*/
	function getUnsubscriptionsPickerKeywordReplacement()
	{
		return $this->printAvailableLocations('_unsubscriptions');

	}//end getUnsubscriptionsPickerKeywordReplacement()


	/**
	* Returns replacement for the current_subscriptions keyword.
	*
	* @return string
	* @access private
	*/
	function getCurrentSubscriptionsKeywordReplacement()
	{
		$current_user = $GLOBALS['SQ_SYSTEM']->user;
		if ($current_user instanceof Public_User) {
			// if current user is public user, %current_subscriptions keyword will be blanked.
			return '';
		}
		$all_mailing_lists = $this->getAvailableLocations();
		// only get direct parents
		$parents = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($current_user->id, 'user_group', FALSE, NULL, NULL, NULL, 1));
		$current_subscriptions = array_intersect($all_mailing_lists, $parents);
		$string = '';
		if (!empty($current_subscriptions)) {
			$string .= '<ul>';
			$info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_values($current_subscriptions), Array(), TRUE, 'name');
			foreach ($current_subscriptions as $current_subscription) {
				$string .= '<li class="'.$this->getPrefix().'_current_subscriptions">'.$info[$current_subscription].'</li>';
			}
			$string .= '</ul>';
		}
		return $string;

	}//end getCurrentSubscriptionsKeywordReplacement()


	/**
	* Get the default contents for a bodycopy
	*
	* @param string	$code	The code name for the bodycopy
	*
	* @return void
	* @access public
	*/
	function _getDefaultBodycopyContents($code)
	{
		switch ($code) {
			case 'logged_in':
				return '<h2>'.sprintf(translate('Editing list subscriptions for %s'), '%user_name%').'</h2>

					%success_messages_list% %error_messages_list%
					<h3>'.translate('Current Subscriptions').'</h3>

					%current_subscriptions%
					<h3>'.translate('Subscribe to new list').'</h3>

					%subscriptions_picker%
					<h3>'.translate('Unsubscribe from list').'</h3>

					%unsubscriptions_picker%
					<br />%commit_button%
				';
			case 'not_logged_in':
				return '<h2>'.translate('Edit List Subscriptions').'</h2>

					%success_messages_list% %error_messages_list%
					'.translate('Username').': %subscribe_user_name% <br />

					'.translate('Email').': %subscribe_email_address% <br />

					<h3>'.translate('Subscribe to new list').'</h3>

					%subscriptions_picker%
					<h3>'.translate('Unsubscribe from list').'</h3>

					%unsubscriptions_picker%
					<br />%commit_button%
				';
		}
		return '';

	}//end _getDefaultBodycopyContents()

	/**
	* Form Captcha keyword replacement
	*
	* @return string
	* @access public
	*/
	function getFormCaptchaKeywordReplacement()
	{
		if (!$this->attr('captcha_ttf') == '') {
			$font = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->attr('captcha_ttf'));
			$file_path = $font->data_path.'/'.$font->attr('name');
		} else {
			$file_path = '';
		}

		ob_start();
			security_key(
				$this->attr('captcha_length'),
				$this->attr('captcha_length'),
				$this->attr('captcha_zoom'),
				FALSE,
				trim($this->attr('captcha_bgcolour'), '#'),
				trim($this->attr('captcha_textcolour'), '#'),
				trim($this->attr('captcha_bordercolour'), '#'),
				$this->attr('captcha_use_colours'),
				$this->attr('captcha_usefont'),
				$file_path,
				$this->attr('captcha_font_size'),
				$this->attr('captcha_min_angle'),
				$this->attr('captcha_max_angle'),
				$this->attr('captcha_x_start'),
				$this->attr('captcha_min_distance'),
				$this->attr('captcha_max_distance'),
				$this->attr('captcha_width'),
				$this->attr('captcha_height'),
				$this->attr('captcha_arc'),
				trim($this->attr('captcha_arc_colour'), '#'),
				$this->attr('captcha_include_uppercase'),
				$this->attr('captcha_include_numbers'),
				$this->attr('provide_accessible_captcha'),
				FALSE,
				$this->attr('captcha_label_text')
			);
		return ob_get_clean();

	}//end getFormCaptchaKeywordReplacement()


	/**
	* Form Captcha Image keyword replacement
	*
	* @return string
	* @access public
	*/
	function getFormCaptchaImageKeywordReplacement()
	{
		if (!$this->attr('captcha_ttf') == '') {
			$font = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->attr('captcha_ttf'));
			$file_path = $font->data_path.'/'.$font->attr('name');
		} else {
			$file_path = '';
		}

		ob_start();
			security_key_image_box(
				$this->attr('captcha_length'),
				$this->attr('captcha_zoom'),
				trim($this->attr('captcha_bgcolour'), '#'),
				trim($this->attr('captcha_textcolour'), '#'),
				trim($this->attr('captcha_bordercolour'), '#'),
				$this->attr('captcha_use_colours'),
				$this->attr('captcha_usefont'),
				$file_path,
				$this->attr('captcha_font_size'),
				$this->attr('captcha_min_angle'),
				$this->attr('captcha_max_angle'),
				$this->attr('captcha_x_start'),
				$this->attr('captcha_min_distance'),
				$this->attr('captcha_max_distance'),
				$this->attr('captcha_width'),
				$this->attr('captcha_height'),
				$this->attr('captcha_arc'),
				trim($this->attr('captcha_arc_colour'), '#'),
				$this->attr('captcha_include_uppercase'),
				$this->attr('captcha_include_numbers'),
				$this->attr('provide_accessible_captcha')
			);
		return ob_get_clean();

	}//end getFormCaptchaImageKeywordReplacement()


	/**
	* Form Captcha Field keyword replacement
	*
	* @return string
	* @access public
	*/
	function getFormCaptchaFieldKeywordReplacement()
	{
		ob_start();
			security_key_field($this->attr('captcha_length'));
		return ob_get_clean();

	}//end getFormCaptchaFieldKeywordReplacement()


	/**
	* Form Captcha Link keyword replacement
	*
	* @return string
	* @access public
	*/
	function getFormCaptchaLinkKeywordReplacement()
	{
		ob_start();
			security_key_image_link($this->attr('captcha_link_text'));
		return ob_get_clean();

	}//end getFormCaptchaLinkKeywordReplacement()


	/**
	* Form Captcha Field keyword replacement
	*
	* @return string
	* @access public
	*/
	function getFormCaptchaLabelKeywordReplacement()
	{
		return $this->attr('captcha_label_text');

	}//end getFormCaptchaLabelTextKeywordReplacement()

	/**
	* Get reCAPTCHA replcement
	*
	* @return string
	* @access public
	*/
	function getFormRecaptchaKeywordReplacement()
	{
		$replacement = '';
		if ($this->attr('use_recaptcha')) {
			$replacement =  Recaptcha::getRecaptcha(Array(), $this->attr('recaptcha_theme'));
		}

		return $replacement;

	}//end getFormRecaptchaKeywordReplacement()


	/**
	* Get the Honeypot field
	* This field must not be visible to the user
	*
	* @return boolean
	* @access private
	*/
	function getFormHoneypotFieldKeywordReplacement()
	{
		$name = $this->attr('honeypot_name');
		$id = $this->attr('honeypot_id');
		$class = $this->attr('honeypot_class');

		if (empty($name)) {
			$name = 'q'.$this->id.':comment';
		}
		if (empty($id)) {
			$id = 'q'.$this->id.'_comment';
		}

		$extras = 'class="'.$class.'"';
		$extras = $extras.' id="'.$id.'"';
		ob_start();
			text_box($name, '', 10, 10, FALSE, $extras);

		return ob_get_clean();

	}//end getFormHoneypotFieldKeywordReplacement()



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
		parent::onRequestKeywords($broadcaster, $vars);
		// only provide captcha keywords for not logged in user
		if(!isset($vars['keywords']['user_name'])) {
			$keywords = Array();
			if ($this->attr('require_captcha')) {
				$keywords['form_captcha'] = translate('Display Captcha Form');

				$keywords['form_captcha_image'] = translate('Display Captcha Image');

				$keywords['form_captcha_field'] = translate('Display Captcha Field');

				$keywords['form_captcha_link'] = translate('Display Captcha Link');

			}

			if ($this->attr('use_recaptcha')) {
				$keywords['form_recaptcha'] = translate('Display reCAPTCHA Form');
	
			}
			$keywords['form_honeypot_field'] = translate('Display Honeypot question');
			$vars['keywords'] = array_merge($vars['keywords'], $keywords);
		}
	}//end onRequestKeywords()

}//end class

?>
