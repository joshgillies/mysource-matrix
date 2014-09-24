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
* $Id: page_password_reset.inc,v 1.19 2013/08/09 01:59:56 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Page_Password_Reset
*
* Purpose
*	Allows a user to reset their password.
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.19 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Password_Reset extends Page
{

	/**
	* By default, we should display a bodycopy allowing the user to request
	* that their password be reset.
	*
	* @var string
	* @access public
	*/
	var $default_state = 'request';


	/**
	* Constructor.
	*
	* @param int	$assetid	the asset id to be loaded
	*
	* @return void
	* @access public
	*/
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
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
		// Set default value for email format attr
		$val = Array();
		$val['subject'] = translate('Password Reset Request');

		$val['text_format'] = sprintf(translate('A password reset request has been received for the account with username "%1$s".  To set a new password for this account please visit %2$s'), '%asset_attribute_username%', '%password_reset_url%');

		$val['html_format'] = '<p>'.sprintf(translate('A password reset request has been received for the account with username "%1$s".  To set a new password for this account please visit %2$s'), '%asset_attribute_username%', '<a href="%password_reset_url%">%password_reset_url%</a>').'</p>';

		$val['from'] = SQ_CONF_SYSTEM_NAME.' <'.SQ_CONF_DEFAULT_EMAIL.'>';
		$this->setAttrVAlue('request_email_format', $val);

		return parent::create($link);

	}//end create()


	/**
	* Perform any additional processing required during the creation of this asset.
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		// Add a group of users
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('user_group');
		$group_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1, 'value' => 'pending_accounts');

		$group = new User_Group();

		$group->setAttrValue('name', translate('Accounts Pending Reset'));

		if (!$group->create($group_link)) return FALSE;
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($group);

		// Add a bodycopy to this page when creating
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$copy_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1, 'value' => '');

		foreach ($this->_getBodycopies() as $value => $details) {
			$bodycopy = new Bodycopy();
			$copy_link['value'] = $value;

			$bodycopy->setAttrValue('name', $details['name']);
			unset($details['name']);
			if (!$bodycopy->create($copy_link, $details)) {
				 return FALSE;
			}
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bodycopy);
		}
		return TRUE;

	}//end _createAdditional()


	/**
	* Returns an array of all the permitted link types, the asset type and the cardinality.
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		$allowed_links = parent::_getAllowedLinks();

		$allowed_links[SQ_LINK_TYPE_2]['bodycopy'] = Array('card' => count($this->_getBodycopies()), 'exclusive' => TRUE);
		$allowed_links[SQ_LINK_TYPE_2]['user_group']	= Array('card' => '1', 'exclusive'	=> TRUE);

		return $allowed_links;

	}//end _getAllowedLinks()


	/**
	* Called by the design to print the body of this asset.
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		// Since we are going a lot of changes here, tie the paint and process
		// together using db2
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');

		$password_reset_action = $this->default_state;
		if (isset($_REQUEST['action'])) { // Handle form submission.
			$password_reset_action = $_REQUEST['action'];
		}
		echo $this->_printContents($password_reset_action);

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

	}//end printBody()


	/**
	* Add valid keywords for this asset to an array of keywords when asked.
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								we add keywords to the $vars['keywords'] array
	*
	* @return boolean
	* @access public
	*/
	function onRequestKeywords(&$broadcaster, $vars=Array())
	{
		if (!isset($vars['keywords'])) return;

		// which bodycopy?
		$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id, 'bodycopy', TRUE);
		$type_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy');
		foreach ($type_links as $link_info) {
			if (isset($parents[$link_info['minorid']])) {
				$bodycopy_type = $link_info['value'];
			}
		}

		// Make sure this is a bodycopy we can replace keywords for...?
		if (!isset($bodycopy_type) || empty($bodycopy_type)) {
			return;
		}
		if (!in_array($bodycopy_type, array_keys($this->_getBodycopies()))) {
			return;
		}

		// Generate the name of the function we should call for keywords
		$tmp_type = strtr($bodycopy_type, '_', ' ');
		$tmp_type = ucwords($tmp_type);
		$tmp_type = preg_replace('/\s+/', '', $tmp_type);
		$type_func = '_get'.$tmp_type.'Keywords';

		if (method_exists($this, $type_func)) {
			$vars['keywords'] = array_merge($vars['keywords'], $this->$type_func());
		}

	}//end onRequestKeywords()


	/**
	* Based on what 'action' is being performed, prints the frontend.
	*
	* @param string	$action	The name of the bodycopy to print
	*
	* @return string
	* @access private
	*/
	function _printContents($action)
	{
		if (empty($action)) $action = $this->default_state;

		// Generate the name of the function we should call for processing
		$tmp_process = strtr($action, '_', ' ');
		$tmp_process = ucwords($tmp_process);
		$tmp_process = preg_replace('/\s+/', '', $tmp_process);
		$process_func = '_print'.$tmp_process.'Action';

		if (method_exists($this, $process_func)) {
			return $this->$process_func();
		}
		return;

	}//end _printContents()


	/**
	* Prints the 'request' bodycopy.
	*
	* The 'request' bodycopy allows the user to enter their details so that
	* they may begin the process of resetting their password.
	*
	* @return string
	* @access private
	*/
	function _printRequestAction()
	{
		$replacements['password_reset_error'] = '';

		return $this->_buildContents('request', $replacements);

	}//end _printRequestAction()


	/**
	* Prints the 'request_receipt' bodycopy.
	*
	* This bodycopy is a receipt of whatever happend on the 'request' bodycopy. It is
	* used to inform the user of the results of the password reset request. Note that
	* the user won't always be taken here after the 'request' process.
	*
	* @return string
	* @access private
	*/
	function _printRequestReceiptAction()
	{
		$replacements = Array();
		if (!isset($_POST['username']) || empty($_POST['username'])) {
			$replacements['password_reset_error'] = translate('You must specify a username.');

			return $this->_buildContents('request', $replacements);
		} else {
			$username = $_POST['username'];
			$auth_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('authentication_folder');
			if (is_null($auth_folder)) {
				trigger_localised_error('SYS0249', translate('Unable to locate authentication system'), E_USER_ERROR);
				return Array();
			}
			$auth_systems = $auth_folder->getAuthSystems();

			$user = NULL;
			foreach ($auth_systems as $systemid) {
				$system = $GLOBALS['SQ_SYSTEM']->am->getAsset($systemid);
				if (is_null($system)) continue;

				if (method_exists($system, 'locateUser')) {
					$user = $system->locateUser($username);
					if (!is_null($user) && $user->canLogin()) {
						break;
					}
				}
			}

			$failure = FALSE;
			if (is_null($user)) {
				$failure = TRUE;
				// Shall we tell that the given user doesn't exits?
				if ($this->attr('invalid_user_error')) {
					$replacements['password_reset_error'] = translate('Specified username does not exist.');

					return $this->_buildContents('request', $replacements);
				}

			} else {
				foreach ($user->getAvailableKeywords() as $keyword => $description) {
					$replacements[$keyword] = $user->getKeywordReplacement($keyword);
				}
				$user_hash = $this->_createResetLink($user->id);
				if (!$user_hash) $failure = TRUE;
			}

			// make sure user opens reset link in the same session in which he requested reset
			$_SESSION['SQ_PASSWORD_RESET_HASH'] = $user_hash;

			// Do not attempt to send mail to anyone if we're not going to go ahead with this, especially
			// if we did't create any link!
			if (!$failure) {
				$email_replacements = $replacements;
				$email_replacements['password_reset_url'] = current_url().'?action=reset&prid='.$user_hash;

				$email_attr = $this->getAttribute('request_email_format');
				$email_value = unserialize($email_attr->value);
				$email_value['to'][] = $user->attr('email');
				$email_attr->value = serialize($email_value);

				@$email_attr->sendMail($email_replacements);
			}

		}//end else

		return $this->_buildContents('request_receipt', $replacements);

	}//end _printRequestReceiptAction()


	/**
	* Prints the 'reset' bodycopy.
	*
	* This bodycopy should only be accessable by the link sent in the 'reset
	* email'. It will allow the user who (hopefully) submitted the corresponding
	* request to specify a new password.
	*
	* @return string
	* @access private
	*/
	function _printResetAction()
	{
		$replacements = Array();
		$replacements['password_reset_error'] = '';

		$action = 'request';

		if (!isset($_REQUEST['prid']) || empty($_REQUEST['prid'])) {
				$replacements['password_reset_error'] = translate('No password reset request specified. Please submit another password reset request.');

		} else {
			$prid = $_REQUEST['prid'];
			$user_link = $this->_getResetLink($prid);
			if($this->attr('reset_in_same_session') && (!isset($_SESSION['SQ_PASSWORD_RESET_HASH']) || $_SESSION['SQ_PASSWORD_RESET_HASH'] !== $prid)) {
				$replacements['password_reset_error'] = translate('Please open the reset link in the same browser session in which the password reset was requested.');

			}
			else if (empty($user_link)) {
				$replacements['password_reset_error'] = translate('Password reset link is invalid. Please submit another password reset request.');

			} else {
				$userid = $user_link['minorid'];
				$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($userid);
				if (is_null($user)) {
					$replacements['password_reset_error'] = translate('The user associated with this password reset could not be found.');

				} else {
					foreach ($user->getAvailableKeywords() as $keyword => $description) {
						$replacements[$keyword] = $user->getKeywordReplacement($keyword);
					}
					$action = 'reset';
				}
			}
		}
		return $this->_buildContents($action, $replacements);

	}//end _printResetAction()


	/**
	* Prints the 'reset_receipt' bodycopy.
	*
	* The receipt of the 'reset' bodycopy. This is rendered to inform the user of
	* anything that happend during the 'reset' process. Note that the user will
	* not necessarily be taken here under all circumstances.
	*
	* @return string
	* @access private
	*/
	function _printResetReceiptAction()
	{
		$replacements = Array();
		$replacements['password_reset_error'] = '';

		$action = 'request';

		if (!isset($_POST['password']) || empty($_POST['password'])) {
			$replacements['password_reset_error'] = translate('You must specify a new password.');

		} else if (!isset($_REQUEST['prid']) || empty($_REQUEST['prid'])) {
			$replacements['password_reset_error'] = translate('No password reset request specified. Please submit another password reset request.');

		} else {
			$password_parts = array_unique($_POST['password']);

			$prid = $_REQUEST['prid'];
			$user_link = $this->_getResetLink($prid);
			if($this->attr('reset_in_same_session') && (!isset($_SESSION['SQ_PASSWORD_RESET_HASH']) || $_SESSION['SQ_PASSWORD_RESET_HASH'] !== $prid)) {
				$replacements['password_reset_error'] = translate('Please open the reset link in the same browser session in which the password reset was requested.');

			}
			else if (empty($user_link)) {
				$replacements['password_reset_error'] = translate('Password reset link is invalid. Please submit another password reset request.');

			} else {
				$userid = $user_link['minorid'];
				$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($userid);
				if (is_null($user)) {
					$replacements['password_reset_error'] = translate('The user associated with this password reset could not be found.');

				} else {
					foreach ($user->getAvailableKeywords() as $keyword => $description) {
						$replacements[$keyword] = $user->getKeywordReplacement($keyword);
					}
					$action = 'reset';
				}
			}
			if (count($password_parts) == 1) {
				if ($this->_resetPassword($user, reset($password_parts))) {
					// The password was reset sucessfully!
					$action = 'reset_receipt';
				} else {
					$replacements['password_reset_error'] = translate('You specified an invalid password. Please try again.');

				}
			} else {
				$replacements['password_reset_error'] = translate('The passwords entered do not match.');

			}
		}

		$field = '';
		if (isset($prid)) {
			ob_start();
				hidden_field('prid', $prid);
			$field = ob_get_clean();
		}
		return $this->_buildContents($action, $replacements, $field);

	}//end _printResetReceiptAction()


	/**
	* Prepares a bodycopy to be rendered.
	*
	* @param string	$action				The action to be performed, aka. the bodycopy to be displayed
	* @param array	$new_replacements	An array of keyword replacements to supplement
	* @param string	$extra				An extra contents to be printed
	*
	* @return string
	* @access private
	*/
	function _buildContents($action, $new_replacements=Array(), $extra='')
	{
		$bodycopy = $this->_getBodycopy($action);
		if (is_null($bodycopy)) {
			return '';
		}

		$replacements = Array();
		ob_start();
			$keywords = $bodycopy->getKeywords();

			foreach ($keywords as $whole_keyword) {

				$keyword = parse_keyword($whole_keyword, $modifiers);

				if(strpos($keyword, 'globals_') === 0) {
					$global_replacement = '%'.$keyword.'%';
					replace_global_keywords($global_replacement);
					$replacements[$whole_keyword] = $global_replacement;
				}
				else if (!in_array($keyword, array_keys($new_replacements))) {
					$replacements[$whole_keyword] = $this->getKeywordReplacement($keyword);
				} else {
					$replacements[$whole_keyword] = $new_replacements[$keyword];
				}

				apply_keyword_modifiers($replacements[$whole_keyword], $modifiers, Array('assetid' => $this->id));

			}//end foreach

			$bodycopy->setKeywordReplacements($replacements);
			$bodycopy->printBody();

			echo $extra;

			if (in_array($action, Array('request', 'reset'))) {
				return $this->_buildForm(ob_get_clean(), $action.'_receipt');
			}
		return ob_get_clean();

	}//end _buildContents()


	/**
	* Paints the bodycopy in a form.
	*
	* Required for bodycopies that use form keywords so that they will function
	* correctly.
	*
	* @param string	$body_contents	The contents to be rendered
	* @param string	$action			The 'action' that will be performed when the form is submitted
	*
	* @return string
	* @access private
	*/
	function _buildForm($body_contents, $action)
	{
		ob_start();
			?><form id="<?php echo $this->getPrefix() ?>_<?php echo $action; ?>_form" method="post" action="<?php echo htmlentities(replace_query_string_vars(Array())); ?>" enctype="multipart/form-data">
				<?php echo hidden_field('action', $action); ?>
				<?php echo $body_contents; ?>
			</form><?php
		return ob_get_clean();

	}//end _buildForm()


	/**
	* Resets the password for the given user to the new password supplied.
	*
	* @param object	&$user			The user to reset the password for
	* @param string	$new_password	The password to set for said user
	*
	* @return boolean
	* @access private
	*/
	function _resetPassword(&$user, $new_password)
	{
		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
			$result = @$user->setAttrValue('password', $new_password);

			if ($result) {
				$user->saveAttributes();

				$parentid = $this->_getPendingAccountsGroupId();
				if ($parentid == 0) return FALSE;

				$parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($parentid, 'user_group');
				if (is_null($parent)) return FALSE;

				// Remove this user completly from our pending reset group.
				$links = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($parentid, $user->id, SQ_LINK_TYPE_1, NULL, 'major', TRUE);
				foreach ($links as $link) {
					$parent->deleteLink($link['linkid']);
				}
			}
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

		// There should probably be an attribute that turns this functionality on/off.
		$GLOBALS['SQ_SYSTEM']->loginUser($user);

		return $result;

	}//end _resetPassword()


	/**
	* Creates a 'reset' link.
	*
	* We identify users that have requested to have their password reset by linking
	* them to the user group we should have linked under this asset.
	*
	* @param string	$assetid	The ID of the asset to link
	*
	* @return string
	* @access private
	*/
	function _createResetLink($assetid)
	{
		if (empty($assetid)) return FALSE;

		// The user asset.
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) return FALSE;

		$parentid = $this->_getPendingAccountsGroupId();
		if ($parentid == 0) return FALSE;

		// The user group asset.
		$parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($parentid, 'user_group');
		if (is_null($parent)) return FALSE;

		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');

			// That's right, delete any existing links.
			$links = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($parent->id, $assetid, SQ_LINK_TYPE_1, NULL, 'major', TRUE);
			foreach ($links as $link) {
				$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($link['linkid']);
			}

            include_once SQ_FUDGE_PATH.'/general/security.inc';
			$hash_value = md5(implode('::', Array($this->id, $asset->id, $asset->name, (time() * security_rand()))));

			$return_value = FALSE;
			if ($parent->createLink($asset, SQ_LINK_TYPE_1, $hash_value)) {
				$return_value = $hash_value;
			}

			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();


		return $return_value;

	}//end _createResetLink()


	/**
	* Returns a 'reset' link.
	*
	* Given the value of the link, returns the link information between our user group
	* and one of our 'password reset' pending users.
	*
	* @param string	$value	The value of the link between the two assets
	*
	* @return string
	* @access private
	*/
	function _getResetLink($value)
	{
		if (empty($value)) return Array();

		$parentid = $this->_getPendingAccountsGroupId();
		if ($parentid == 0) return FALSE;

		// This should return just the one link hopefully, otherwise we're in trouble ($value is a hash
		// and should be very unique).
		return $GLOBALS['SQ_SYSTEM']->am->getLink($parentid, SQ_LINK_TYPE_1, '', FALSE, $value);

	}//end _getResetLink()


	/**
	* A bunch of keywords that should be available on all of our bodycopies.
	*
	* @return array
	* @access protected
	*/
	function _getStandardKeywords()
	{
		$keywords = Array();

		$keywords['submit_button'] = 'The submit button';
		$keywords['password_reset_error'] = 'Error messages';

		return $keywords;

	}//end _getStandardKeywords()


	/**
	* The keywords used in the 'request' bodycopy.
	*
	* @return array
	* @access protected
	*/
	function _getRequestKeywords()
	{
		$keywords = $this->_getStandardKeywords();

		$keywords['username_input'] = 'The input field for username';

		return $keywords;

	}//end _getRequestKeywords()


	/**
	* The keywords used in the 'request_receipt' bodycopy.
	*
	* @return array
	* @access protected
	*/
	function _getRequestReceiptKeywords()
	{
		$keywords = $this->_getStandardKeywords();

		return $keywords;

	}//end _getRequestReceiptKeywords()


	/**
	* The keywords used in the 'reset' bodycopy.
	*
	* @return array
	* @access protected
	*/
	function _getResetKeywords()
	{
		$keywords = $this->_getStandardKeywords();

		$keywords['password_input'] = 'The input fields for password reset (new and confirmation password input fields)';
		$keywords['password_new'] = 'Input field for the new password';
		$keywords['password_confirmation'] = 'Input field for the Confirmation of the new password';

		return $keywords;

	}//end _getResetKeywords()


	/**
	* The keywords used in the 'reset_receipt'
	*
	* @return array
	* @access protected
	*/
	function _getResetReceiptKeywords()
	{
		$keywords = $this->_getStandardKeywords();

		return $keywords;

	}//end _getResetReceiptKeywords()


	/**
	* Prints form elements allowing the user to enter their username.
	*
	* @return string
	* @access public
	*/
	function getUsernameInputKeywordReplacement()
	{
		$value = '';
		if (isset($_POST['username'])) {
			$value = $_POST['username'];
		}

		ob_start();
			text_box('username', '');
		return ob_get_clean();

	}//end getUsernameInputKeywordReplacement()


	/**
	* Prints a form allowing the user to enter a new password.
	*
	* @return string
	* @access public
	*/
	function getPasswordInputKeywordReplacement()
	{
		$auto_complete = $this->attr('password_autocomplete') ? 'on' : 'off';
		ob_start();
			password_box('password[]', '', '', '', 'autocomplete="'.$auto_complete.'"');
			password_box('password[]', '', '', '', 'autocomplete="'.$auto_complete.'"');
		return ob_get_clean();

	}//end getPasswordInputKeywordReplacement()


	/**
	* Prints a form allowing the user to enter a new password.
	*
	* @return string
	* @access public
	*/
	function getPasswordNewKeywordReplacement()
	{
		$auto_complete = $this->attr('password_autocomplete') ? 'on' : 'off';
		ob_start();
			password_box('password[]', '', '', '', 'autocomplete="'.$auto_complete.'"');
		return ob_get_clean();

	}//end getPasswordNewKeywordReplacement()


	/**
	* Prints a form allowing the user to enter a new password.
	*
	* @return string
	* @access public
	*/
	function getPasswordConfirmationKeywordReplacement()
	{
		$auto_complete = $this->attr('password_autocomplete') ? 'on' : 'off';
		ob_start();
			password_box('password[]', '', '', '', 'autocomplete="'.$auto_complete.'"');
		return ob_get_clean();

	}//end getPasswordConfirmationKeywordReplacement()


	/**
	* Prints a form submit button.
	*
	* @return string
	* @access public
	*/
	function getSubmitButtonKeywordReplacement()
	{
		ob_start();
			submit_button('', translate('Submit'));

		return ob_get_clean();

	}//end getSubmitButtonKeywordReplacement()


	/**
	* Get the ID of the Pending Accounts user group.
	*
	* @return void
	* @access public
	*/
	function _getPendingAccountsGroupId()
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'user_group', FALSE, 'pending_accounts');
		if (empty($link)) return 0;

		return $link['minorid'];

	}//end _getPendingAccountsGroupId()


	/**
	* Returns the linked bodycopy with the specified link value.
	*
	* @param string	$link_value	The value of the link between this asset and the bodycopy
	*
	* @return object
	* @access public
	*/
	function &_getBodycopy($link_value)
	{
		$null = NULL;

		$bodycopy_data = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, $link_value, 'major', '1');
		if (!$bodycopy_data) return $null;

		$bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_data['minorid'], $bodycopy_data['minor_type_code']);
		if (!$bodycopy) return $null;

		return $bodycopy;

	}//end _getBodycopy()


	/**
	* Return an array of bodycopies that need to be created.
	*
	* @return array
	* @access private
	*/
	function _getBodycopies()
	{
		$res = Array();
		$res['request']['name'] = translate('Request');

		$res['request']['content'] = '<p>%password_reset_error%</p><p>'.translate('To reset your password, enter your username below. An email containing further instructions will be sent to the email address with which you registered.').'</p><p>%username_input% %submit_button% </p>';


		$res['request_receipt']['name'] = translate('Request Receipt');

		$res['request_receipt']['content'] = '<p>'.translate('Please check your email for instructions on how to complete the password reset procedure.').'</p>';


		$res['reset']['name'] = translate('Reset');

		$res['reset']['content'] = '<p>%password_reset_error%</p><p>'.translate('Please enter a new password for your account').'</p><p>%password_input%&nbsp;%submit_button%</p>';


		$res['reset_receipt']['name'] = translate('Reset Receipt');

		$res['reset_receipt']['content'] = '<p>'.translate('Your password has been reset and you are now logged in.').'</p>';


		return $res;

	}//end _getBodycopies()


}//end class
