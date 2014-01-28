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
* $Id: page_user_group_manager.inc,v 1.23 2012/08/30 00:58:09 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Page_Link_Manager
*
* Purpose
*
*
* @author  Darren McKee <dmckee@squiz.net>
* @author  David Heppell <dheppell@squiz.net>
* @version $Revision: 1.23 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_User_Group_Manager extends Page
{

	var $bodycopies = Array(
						'logged_in'		=> 'Logged In Layout',
						'not_logged_in'	=> 'Not Logged In Layout',
					  );

	var $keywords = Array(
						'logged_in'		=> Array(
											'error_messages_list',
											'success_messages_list',
											'location_picker',
											'user_name',
											'user_short_name',
											'user_email',
											'commit_button',
										   ),
						'not_logged_in'	=> Array(
											'login_form',
										   ),
					);

	var $errors;

	var $messages;


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

		// add a bodycopy for the Page contents, and the format of locations
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');

		foreach ($this->bodycopies as $value => $name) {
			$copy_link = Array(
							'asset'			=> &$this,
							'link_type'		=> SQ_LINK_TYPE_2,
							'is_dependant'	=> 1,
							'is_exclusive'	=> 1,
							'value'			=> $value,
						 );

			$bodycopy = new Bodycopy();
			$bodycopy->setAttrValue('name', $name);
			$args = Array('content' => $this->_getDefaultBodycopyContents($value));
			if (!$bodycopy->create($copy_link, $args)) {
				return FALSE;
			}
		}
		return TRUE;

	}//end _createAdditional()


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
				return '<h2>'.translate('cms_user_group_manager_title', '%user_name%').'</h2>'."\n".
						'%error_messages_list%%success_messages_list%'."\n".
						'<p>%location_picker%%commit_button%</p>';
			case 'not_logged_in':
				return translate('cms_user_group_manager_must_be_logged_in');
		}
		return '';

	}//end _getDefaultBodycopyContents()


	/**
	* printBody
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		// if the page is being submitted, a slow DB replication could mean that
		// the printed bodycopy contents may not immediately match the newly
		// committed data if done on db1... so tying the process and print
		// together, IF the form is being submitted
		if (isset($_REQUEST['submitted'])) {
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		}

		// Process any operations. Deleting, New links etc.
		$this->processForm();
		?>
			<form id="<?php echo $this->getPrefix(); ?>" action="<?php echo current_url(); ?>" method="post">
			<input type="hidden" name="submitted" value="1" />
		<?php
		// Print the form and details again
		echo $this->printBodycopyContents();
		?></form><?php

		if (isset($_REQUEST['submitted'])) {
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		}

	}//end printBody()


	/**
	* Process submissions made by the user.
	*
	* @return boolean
	* @access public
	*/
	function processForm()
	{
		$user_groups = Array();
		$link_type = $this->attr('link_type');

		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($GLOBALS['SQ_SYSTEM']->currentUserId(), $link_type, '', FALSE, 'minor');
		// only allow links we have write access to
		$valid_nodes = array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($this->attr('root_node'), 'user_group', FALSE, NULL, NULL, SQ_PERMISSION_WRITE));

		$current_groups = Array();
		foreach ($current_links as $id => $link) {
			if (in_array($link['majorid'], $valid_nodes)) {
				$current_groups[] = $link['majorid'];
			}
		}

		if (isset($_REQUEST['submitted'])) {
			if (isset($_REQUEST[$this->getPrefix().'_location_selection'])) {
				$user_groups = $_REQUEST[$this->getPrefix().'_location_selection'];

				if ($this->attr('allow_multiple') == FALSE) {
					// Change the single entry into an array
					$user_groups = !empty($user_groups) ? Array($user_groups) : Array();
				}

				$this->_tmp['selected'] = $user_groups;
				$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
				$done = $this->processMinorLinks($GLOBALS['SQ_SYSTEM']->currentUserId(), $current_groups, $user_groups, $link_type);
				$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

				$this->generateMessages($done);
			}
		}

		return TRUE;

	}//end processForm()


	/**
	* Handle linking this asset under multiple parents
	*
	* @param int	$minor_id	asset we are going to link
	* @param array	$old_links	The current assets we are linked to
	* @param array	$new_links	The new selections we wish to be linked to
	* @param int	$link_type	type of link to use
	* @param string $link_value value associated with the link
	*
	* @return void
	* @access public
	*/
	function processMinorLinks($minor_id, $old_links=Array(), $new_links=Array(), $link_type, $link_value=NULL)
	{
		$cannot_create = Array();
		$cannot_delete = Array();
		$to_delete = array_diff($old_links, $new_links);
		$to_create = array_diff($new_links, $old_links);
		$minor = $GLOBALS['SQ_SYSTEM']->am->getAsset($minor_id);

		$done = Array();

		// loop through and link our minor under all majors
		foreach ($to_create as $majorid) {
			$major = $GLOBALS['SQ_SYSTEM']->am->getAsset($majorid);
			if (!$GLOBALS['SQ_SYSTEM']->am->createAssetLink($major, $minor, $link_type)) {
				$done['add_fail'][] = $major->id;
			} else {
				$done['add_success'][] = $major->id;
			}

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($major);
		}

		// handle links marked to be removed
		foreach ($to_delete as $majorid) {
			$curr_link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($majorid, $minor_id, $link_type, $link_value);
			if (!empty($curr_link)) {
				$link_id = $curr_link['linkid'];

				if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($link_id)) {
					$done['delete_fail'][] = $majorid;
				} else {
					$done['delete_success'][] = $majorid;
				}
			}
		}

		// cleanup
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($minor);

		return $done;

	}//end processMinorLinks()


	/**
	* Takes an associative array of assetids and writes relevant error/success
	* messages to $this->[errors|messages] for later formatting
	*
	* @param array	$fail_success	associative array of assetids
	*
	* @return void
	* @access public
	*/
	function generateMessages($fail_success)
	{
		foreach ($fail_success as $message_type => $assetids) {
			switch ($message_type) {
				case 'add_fail':
					$this->errors[] = 'Unable to add this user to the following groups: '.$this->_getCommaSeparatedNamesByAssetid($assetids);
				break;
				case 'add_success':
					$this->messages[] = 'Successfully added this user to the following groups: '.$this->_getCommaSeparatedNamesByAssetid($assetids);
				break;
				case 'delete_fail':
					$this->errors[] = 'Unable to remove this user from the following groups: '.$this->_getCommaSeparatedNamesByAssetid($assetids);
				break;
				case 'delete_success':
					$this->messages[] = 'Successfully removed this user from the following groups: '.$this->_getCommaSeparatedNamesByAssetid($assetids);
				break;
			}
		}

	}//end generateMessages()


	/**
	* Displays a list of available locations to link the current asset to
	*
	* @param string	$suffix	The suffix
	*
	* @return void
	* @access public
	*/
	function printAvailableLocations($suffix='_location_selection')
	{
		// populate our list
		$type_codes = Array('user_group' => '1');
		$full_types = Array();
		foreach ($type_codes as $type => $inherit) {
			$full_types[] = $type;
			if ($inherit == '1') {
				$full_types += $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($type);
			}
		}
		$prefix = $this->getPrefix();
		ob_start();
			$root_node = $this->attr('root_node');
			$default = array_get_index($this->_tmp, 'selected', Array());
			// if it is a public user, this function should return locations that public user can read
			// else this function should return locations that current user has write permissions
			$current_user = $GLOBALS['SQ_SYSTEM']->user;
			if ($current_user instanceof Public_User) {
				$_permission = SQ_PERMISSION_READ;
			} else {
				$_permission = SQ_PERMISSION_WRITE;
			}
			structured_drop_down($prefix.$suffix, $root_node, $full_types, $default, $this->attr('structured_width'), $this->attr('structured_height'), $this->attr('max_depth'), $this->attr('allow_multiple'), $this->attr('seperator'), $this->attr('include_root_node'), '', $this->attr('structured_style'), $this->attr('statuses'), TRUE, Array(), FALSE, $_permission);
			$selector = ob_get_contents();
		ob_end_clean();
		return $selector;

	}//end printAvailableLocations()


	/**
	* Returns an array of all the permitted link types, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		return Array(SQ_LINK_TYPE_2 => Array('bodycopy' => Array('card' => 'M', 'exclusive' => FALSE)));

	}//end _getAllowedLinks()


	/**
	* Retrieves a bodycopy's contents by name
	* Returns the bodycopy's contents, or empty string if the name does not match any bodycopies
	*
	* @param string	$name			the name of the bodycopy to retrieve
	* @param array	$replacements	Replacement texts
	*
	* @return mixed string|NULL
	* @access private
	* @see Search_Page::bodycopies
	* @see Search_Page::getBodycopy
	*/
	function &getBodycopyContents($name, $replacements=Array())
	{
		$bodycopy = $this->getBodycopy($name);
		if (is_null($bodycopy)) return '';

		$bodycopy->setKeywordReplacements($replacements);

		ob_start();
			$bodycopy->printBody();
			$html = ob_get_contents();
		ob_end_clean();

		return $html;

	}//end getBodycopyContents()


	/**
	* Retrieves a bodycopy by name
	* Returns the bodycopy, or null if the name does not match any bodycopies.
	*
	* @param string	$name	the name of the bodycopy to retrieve
	*
	* @return mixed object|NULL
	* @access private
	* @see Search_Page::bodycopies
	*/
	function &getBodycopy($name)
	{
		$null = NULL;
		if (array_key_exists($name, $this->bodycopies)) {
			$am = $GLOBALS['SQ_SYSTEM']->am;
			$link = $am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, $name, 'major', '1');
			if (!$link) {
				return $null;
			}
			$bodycopy = $am->getAsset($link['minorid'], $link['minor_type_code']);

			return $bodycopy ? $bodycopy : $null;
		}

		return $null;

	}//end getBodycopy()


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
		$vars['keywords'] = isset($vars['keywords']) ? $vars['keywords'] : Array();

		// Get any bodycopy parents of the broadcaster
		$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id, 'bodycopy', TRUE);
		$bodycopy_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy');
		$chosen_bodycopy = '';
		foreach ($bodycopy_links as $id => $link) {
			if (in_array($link['minorid'], array_keys($parents))) {
				$chosen_bodycopy = $link['value'];
				break;
			}
		}
		$keys = Array();
		if (!empty($chosen_bodycopy)) {
			foreach ($this->keywords[$chosen_bodycopy] as $key_word) {
				$keys[$key_word] = ucwords(str_replace('_', ' ', $key_word));
			}
		}
		$vars['keywords'] = array_merge($vars['keywords'], $keys);

	}//end onRequestKeywords()


	/**
	* Returns the contents of the bodycopy with all replacements made
	*
	* @return string
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function printBodycopyContents()
	{
		$public_user = $GLOBALS['SQ_SYSTEM']->getPublicUser();
		if ($GLOBALS['SQ_SYSTEM']->currentUser($public_user)) {
			$bodycopy = $this->getBodycopy('not_logged_in');
		} else {
			$bodycopy = $this->getBodycopy('logged_in');
		}
		if (!$bodycopy) {
			return '';
		}

		$keywords = $bodycopy->getKeywords();
		$replacements = Array();
		foreach ($keywords as $name) {
			$replacements[$name] = $this->getKeywordReplacement($name);
		}
		$bodycopy->setKeywordReplacements($replacements);

		ob_start();
			$bodycopy->printBody();
			$html = ob_get_contents();
		ob_end_clean();

		return $html;

	}//end printBodycopyContents()


	/**
	* Takes an array of assetids and returns the asset names as a comma
	* separated string
	*
	* @param array	$assetids	Array of assetids
	*
	* @return string
	* @access private
	*/
	function _getCommaSeparatedNamesByAssetid($assetids)
	{
		$string = '';
		$names = Array();
		foreach ($assetids as $assetid) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$names[] = $asset->name;
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
		}
		$string .= implode(', ', $names);

		return $string;

	}//end _getCommaSeparatedNamesByAssetid()


	/**
	* Returns replacement for the location_picker keyword.
	*
	* @return string
	* @access private
	*/
	function getLocationPickerKeywordReplacement()
	{
		return $this->printAvailableLocations();

	}//end getLocationPickerKeywordReplacement()


	/**
	* Returns replacement for the user_name keyword.
	*
	* @return string
	* @access private
	*/
	function getUserNameKeywordReplacement()
	{
		$user = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($GLOBALS['SQ_SYSTEM']->currentUserId()));
		return $user[$GLOBALS['SQ_SYSTEM']->currentUserId()]['name'];

	}//end getUserNameKeywordReplacement()


	/**
	* Returns replacement for the user_short_name keyword.
	*
	* @return string
	* @access private
	*/
	function getUserShortNameKeywordReplacement()
	{
		$user = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($GLOBALS['SQ_SYSTEM']->currentUserId()));
		return $user[$GLOBALS['SQ_SYSTEM']->currentUserId()]['short_name'];

	}//end getUserShortNameKeywordReplacement()


	/**
	* Returns replacement for the user_email keyword.
	*
	* @return string
	* @access private
	*/
	function getUserEmailKeywordReplacement()
	{
		$current_user = $GLOBALS['SQ_SYSTEM']->user;
		$current_user_email = $current_user->attr('email');
		return $current_user_email;

	}//end getUserEmailKeywordReplacement()


	/**
	* Returns replacement for the user_id keyword.
	*
	* @return string
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function getUserIdKeywordReplacement()
	{
		return $GLOBALS['SQ_SYSTEM']->currentUserId();

	}//end getUserIdKeywordReplacement()


	/**
	* Returns replacement for the commit_button keyword.
	*
	* @return string
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function getCommitButtonKeywordReplacement()
	{
		ob_start();
			submit_button($this->getPrefix().'_submit_button', $this->attr('submit-button'));
			$button = ob_get_contents();
		ob_end_clean();
		return $button;

	}//end getCommitButtonKeywordReplacement()


	/**
	* Returns replacement for the error_messages_list keyword.
	*
	* @return string
	* @access private
	*/
	function getErrorMessagesListKeywordReplacement()
	{
		return $this->_generateMessageList($this->errors, '_errors');

	}//end getErrorMessagesListKeywordReplacement()


	/**
	* Returns replacement for the error_messages_list keyword.
	*
	* @return string
	* @access private
	*/
	function getSuccessMessagesListKeywordReplacement()
	{
		return $this->_generateMessageList($this->messages, '_success');

	}//end getSuccessMessagesListKeywordReplacement()


	/**
	* Print out an unordered list
	*
	* @param array	$messages	Array of messages
	* @param string	$suffix		The suffix
	*
	* @return string
	* @access private
	*/
	function _generateMessageList($messages, $suffix)
	{
		$string = '';
		if (!empty($messages)) {
			$string .= '<ul>';
			foreach ($messages as $message) {
				$string .= '<li class="'.$this->getPrefix().'_'.$suffix.'">'.$message.'</li>';
			}
			$string .= '</ul>';
		}
		return $string;

	}//end _generateMessageList()


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
			<input type="hidden" name="SQ_LOGIN_KEY"      value="<?php echo $GLOBALS['SQ_SYSTEM']->generateLoginKey(); ?>" />
			<input type="hidden" name="SQ_LOGIN_REFERER"  value="" />

			<table border="0">
				<tr>
					<td valign="top"><?php echo translate('username'); ?></td>
					<td valign="top"><?php echo text_box('SQ_LOGIN_USERNAME', (isset($_POST['SQ_LOGIN_USERNAME'])) ? $_POST['SQ_LOGIN_USERNAME'] : 'Enter your username here', 25, 0, (isset($_POST['SQ_LOGIN_USERNAME'])) ? FALSE : TRUE, 'title="Username"'); ?></td>
				</tr>
				<tr>
					<td valign="top"><?php echo translate('password'); ?></td>
					<td valign="top"><?php echo password_box('SQ_LOGIN_PASSWORD', '', 25, 0, 'title="Password"'); ?></td>
				</tr>
				<tr>
					<td valign="top">&nbsp;</td>
					<td valign="top">
						<input type="hidden" name="SQ_ACTION" value="" id="SQ_ACTION" />
						<?php echo submit_button('SQ_LOGIN_SUBMIT', 'Login', 'document.getElementById(\'SQ_ACTION\').value = \'login\'; document.getElementById(\''.$this->id.'ASSET_BUILDER_ACTION\').value = \'login\';'); ?>
					</td>
				</tr>
			</table>

			<?php
			$login_form = ob_get_contents();
		ob_end_clean();
		return $login_form;

	}//end getLoginFormKeywordReplacement()


}//end class

?>
