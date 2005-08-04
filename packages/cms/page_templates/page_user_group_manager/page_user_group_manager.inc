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
* $Id: page_user_group_manager.inc,v 1.1 2005/08/04 06:55:25 dmckee Exp $
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
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_User_Group_Manager extends Page
{


	var $bodycopies = Array(
						'linkman_bc'	=> 'Link Manager Layout',
					  );

	var $keywords = Array(
						'linkman_bc'	=> Array(
											'update_message',
											'failure_message',
											'location_picker',
											'user_name',
											'user_short_name',
											'user_id',
											'commit_button',
										   ),
					);


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Page_User_Group_Manager($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Page($assetid);

	}//end constructor


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Search pages create a few bodycopy assets when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @access private
	* @return boolean
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return false;

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

			$bodycopy =& new Bodycopy();
			$bodycopy->setAttrValue('name', $name);
			if (!$bodycopy->create($copy_link)) return false;
		}
		return true;

	}//end _createAdditional()


	/**
	* printBody
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		//Process any operations. Deleting, New links etc.
		$this->processForm();
		?>
			<form name="<?php echo $this->getPrefix(); ?>" method="post">
			<input type="hidden" name="submitted" value="1" />
		<?php
		//Print the form and details again
		echo $this->printBodycopyContents();
		?></form><?php
	}//end printBody()


	/**
	* Process submissions made by the user.
	*
	*
	* @return bool
	* @access public
	*/
	function processForm()
	{
		$user_groups = Array();
		$link_type = $this->attr('link_type');

		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($GLOBALS['SQ_SYSTEM']->currentUserId(), $link_type, '', false, 'minor');
		$valid_nodes = array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($this->attr('root_node'), 'user_group', false));
		$current_groups = Array();
		foreach ($current_links as $id => $link) {
			if (in_array($link['majorid'], $valid_nodes)) {
				$current_groups[] = $link['majorid'];
			}
		}

		$this->_tmp['selected'] = $current_groups;

		if (isset($_REQUEST['submitted'])) {
			if (isset($_REQUEST[$this->getPrefix().'_location_selection'])) {
				$user_groups = $_REQUEST[$this->getPrefix().'_location_selection'];
			}
			if ($this->attr('allow_multiple') == false) {
				//Change the single entruy into an array
				$user_groups = Array($user_groups);
			}
			$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
			$this->processMinorLinks($GLOBALS['SQ_SYSTEM']->currentUserId(), $current_groups, $user_groups, $link_type);
			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
		}
		return true;
	}//end processForm()


	/**
	* Handle linking this asset under multiple parents
	*
	* @param int	$minor_id			asset we are going to link
	* @param Array	$existing_ids	The current assets we are linked to
	* @param Array	$new_ids		The new selections we wish to be linked to
	* @param int	$link_type		type of link to use
	*
	* @return void
	* @access public
	*/
	function processMinorLinks($minor_id, $old_links=Array(), $new_links=Array(), $link_type)
	{
		$cannot_create = Array();
		$cannot_delete = Array();
		$to_delete = array_diff($old_links, $new_links);
		$to_create = array_diff($new_links, $old_links);
		$minor = &$GLOBALS['SQ_SYSTEM']->am->getAsset($minor_id);

		// loop through and link our minor under all majors
		foreach ($to_create as $majorid) {
			$major = &$GLOBALS['SQ_SYSTEM']->am->getAsset($majorid);
			if (!$GLOBALS['SQ_SYSTEM']->am->createAssetLink($major, $minor, $link_type)) {
				$cannot_create[] = $majorid;
			}

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($major);
		}

		// handle links marked to be removed
		foreach ($to_delete as $majorid) {
			$curr_link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($majorid, $minor_id, $link_type);
			$link_id = $curr_link['linkid'];

			if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($link_id)) {
				$cannot_remove[] = $majorid;
			}
		}

		// cleanup
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($minor);

		//create some messages for links we cant create/delte
		if (!empty($cannot_create) || empty($cannot_delete)) {
			$create_msg = 'The following assets could not be linked to: ';
			$delete_msg = 'The following assets could not remove links from: ';
			foreach ($cannot_create as $id) {
				$create_msg .= $id.',';
			}

			foreach ($cannot_delete as $id) {
				$delete_msg .= $id.',';
			}

			$this->_tmp['success'] = $create_msg;
			$this->_tmp['failure'] = $delete_msg;
		}

		// Set selected var to newly created links
		$this->_tmp['selected'] = $new_links;
	}//end processMinorLinks()


	/**
	* Displays a list of available locations to link the current asset to
	*
	* @return void
	* @access public
	*/
	function printAvailableLocations()
	{

		//populate our list
		$type_codes = Array('user_group' => '1');
		$full_types = Array();
		foreach ($type_codes as $type => $inherit) {
			$full_types[] = $type;
			if ($inherit == '1') {
				$full_types += $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($type);
			}
		}
		ob_start();
			structured_drop_down($this->getPrefix().'_location_selection', $this->attr('root_node'), $full_types, $this->_tmp['selected'], $this->attr('structured_width'), $this->attr('structured_height'), $this->attr('max_depth'), $this->attr('allow_multiple'), $this->attr('seperator'), $this->attr('include_root_node'), '', $this->attr('structured_style'));
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

		return Array(SQ_LINK_TYPE_2 => Array('bodycopy' => Array('card' => 'M', 'exclusive' => false)));

	}//end _getAllowedLinks()


	/**
	* Retrieves a bodycopy's contents by name
	*
	* @param string		$name	the name of the bodycopy to retrieve
	*
	* @return [string|null] the bodycopy's contents, or empty string if the name does not match any bodycopies
	* @access private
	* @see Search_Page::bodycopies
	* @see Search_Page::getBodycopy
	*/
	function &getBodycopyContents($name, $replacements=Array())
	{
		$bodycopy =& $this->getBodycopy($name);
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
	*
	* @param string		$name	the name of the bodycopy to retrieve
	*
	* @return [object|null] The bodycopy, or null if the name does not match any bodycopies.
	* @access private
	* @see Search_Page::bodycopies
	*/
	function &getBodycopy($name)
	{
		if (array_key_exists($name, $this->bodycopies)) {
			$am = &$GLOBALS['SQ_SYSTEM']->am;
			$link = $am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, $name, 'major', '1');
			$bodycopy =& $am->getAsset($link['minorid'], $link['minor_type_code']);
			return $bodycopy;
		} else {
			return null;
		}

	}//end getBodycopy()


	/**
	* Add valid keywords for this asset to an array of keywords when asked
	*
	* @param &object Asset	$asset	the asset that triggered the event
	* @param Array			$vars	the vars that get submitted by the broadcaster
	*								we add keywords to the $vars['keywords'] array
	*
	* @return boolean
	* @access private
	*/
	function onRequestKeywords(&$broadcaster, $vars=Array())
	{
		$vars['keywords'] = isset($vars['keywords']) ? $vars['keywords'] : Array();

		//Get any bodycopy parents of the broadcaster
		$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id, 'bodycopy', true);
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
	* @param string $name	Name of bodycopy to print
	*
	* @return string
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function printBodycopyContents()
	{
		$bodycopy =& $this->getBodycopy('linkman_bc');
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
	* Returns replacement for the update_message keyword.
	*
	* @return string
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function getUpdateMessageKeywordReplacement()
	{
		if (isset($this->_tmp['update_message'])) {
			return $this->_tmp['update_message'];
		}
		return '';
	}//end getUpdateMessageKeywordReplacement()


	/**
	* Returns replacement for the update_message keyword.
	*
	* @return string
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function getFailureMessageKeywordReplacement()
	{
		if (isset($this->_tmp['failure_message'])) {
			return $this->_tmp['failure_message'];
		}
		return '';
	}//end getFailureMessageKeywordReplacement()


	/**
	* Returns replacement for the location_picker keyword.
	*
	* @return string
	* @access private
	* @see Asset::_getAllowLinks()
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
	* @see Asset::_getAllowLinks()
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
	* @see Asset::_getAllowLinks()
	*/
	function getUserShortNameKeywordReplacement()
	{
		$user = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($GLOBALS['SQ_SYSTEM']->currentUserId()));
		return $user[$GLOBALS['SQ_SYSTEM']->currentUserId()]['short_name'];
	}//end getUserShortNameKeywordReplacement()


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


}//end class

?>
