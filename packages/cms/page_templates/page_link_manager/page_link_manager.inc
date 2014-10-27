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
* $Id: page_link_manager.inc,v 1.24 2012/08/30 00:58:08 ewang Exp $
*
*/

require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_asset_listing/page_asset_listing.inc';
require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

define('SQ_PAGE_LINK_MGR_PRESET_LINKED', 'Linked');

/**
* Page_Link_Manager
*
* Purpose
*
*
* @author  Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.24 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Link_Manager extends Page_Asset_Listing
{

	var $bodycopies = Array(
						'logged_in'		=> 'Logged In Layout',
						'not_logged_in'	=> 'Not Logged In Layout',
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
	* Creates bodycopies for this asset
	*
	* Overridden, as the Page_Link_Manager needs an extra 'Not Logged In' bodycopy
	*
	* @return boolean
	* @access protected
	*/
	function _createBodycopies()
	{
		if (!parent::_createBodycopies()) return FALSE;

		// add a new 'not_logged_in' bodycopy
		$asset = new Bodycopy();
		$copy_link = Array(
						'asset'			=> &$this,
						'value'			=> 'not_logged_in',
						'link_type'		=> SQ_LINK_TYPE_2,
						'is_dependant'	=> 1,
						'is_exclusive'	=> 1,
					 );
		$args = Array('content' => $this->_getDefaultBodycopyContent('not_logged_in'));
		$asset->setAttrValue('name', 'Page Contents (Not Logged In)');
		return $asset->create($copy_link, $args);

	}//end _createBodycopies()


	/**
	* Get the default content for a bodycopy
	*
	* @param string	$bodycopy_code	The code name for the bodycopy
	*
	* @return string
	* @access private
	*/
	function _getDefaultBodycopyContent($bodycopy_code)
	{
		switch ($bodycopy_code) {
			case 'not_logged_in':
				return '<p>'.translate('This page is only available to logged-in users').'</p>';

			case 'page_contents':
				return '<h2>'.sprintf(translate('Managing links for %s'), '%managed_asset_asset_name%').'</h2>%asset_listing%<p>%submit_button%</p>';

			case 'default_format':
				return '%Linked_asset_selection% %asset_name%';
		}
		return parent::_getDefaultBodycopyContent($bodycopy_code);

	}//end _getDefaultBodycopyContent()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowedLinks()
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_TYPE_2]['bodycopy'] = Array('card' => 3,   'exclusive' => FALSE);
		return $page_links;

	}//end _getAllowedLinks()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	* @param int		$contextid	what context to return the name from
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE, $contextid=NULL)
	{
		// No context specified, using the current context
		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}//end if

		// Obtain the attribute value for Name from the specified Context
		$attr = ($short_name) ? 'short_name' : 'name';
		$values = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName($attr, $this->type(), Array($this->id), $contextid);
		if (empty($values) === TRUE) {
			return parent::_getName($short_name, $contextid);
		} else {
			return $values[$this->id];
		}

	}//end _getName()


//--        Linking Functions        --//


	/**
	* Return a list of assets the specified asset is linked, restricted to links
	* where the asset is the minor, and by the link type and root node selected
	* in the backend, in the format: Array(assetid => type)
	*
	* @param object		&$asset		the asset to retrieve links from
	* @param boolean	$no_cache	make sure the result isn't cached, and clear the cache for good measure
	*
	* @return array
	* @access private
	*/
	function &_getAssetLinks(&$asset, $no_cache=FALSE)
	{
		if (is_null($asset)) {
			$this->_tmp['linked_selections'] = Array();
		}

		if (!isset($this->_tmp['linked_selections'])) {

			// get the asset links
			$asset_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, $this->attr('link_type'), '', FALSE, 'minor', NULL, NULL, NULL, NULL, SQ_PERMISSION_WRITE);

			// gather the root nodes
			$root_asset_ids = $this->getRootNodes();
			$valid_nodes = Array();
			foreach ($root_asset_ids as $root_asset_id) {
				$new_valid_nodes = array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($root_asset_id, array_keys($this->attr('types')), FALSE));
				$valid_nodes = array_merge($valid_nodes, $new_valid_nodes);
			}

			$current_links = Array();
			foreach ($asset_links as $id => $link) {
				if (in_array($link['majorid'], $valid_nodes)) {
					$current_links[$link['majorid']] = $link['majorid'];
				}
			}

			// if no_cache is turned on, clear the cache
			if ($no_cache) {
				unset($this->_tmp['linked_selections']);
				return $current_links;
			}

			$this->_tmp['linked_selections'] = $current_links;
		}

		return $this->_tmp['linked_selections'];

	}//end _getAssetLinks()


	/**
	* Returns the asset to be managed - defauls to the current user if no asset is specified
	*
	* @return object
	* @access public
	*/
	function &_getManagedAsset()
	{
		$return = NULL;

		// check that we don't have a root
		$parameter_map       = $this->getAttribute('parameter_map');
		$tmp_managed_assetid =  $parameter_map->getParameterValue('managed_asset');
		if (!empty($tmp_managed_assetid)) {
			$return = $GLOBALS['SQ_SYSTEM']->am->getAsset($tmp_managed_assetid);
		}

		// if nothing, try the attribute
		if (is_null($return) && $this->attr('managed_asset')) {
			$return = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->attr('managed_asset'));
		}

		if (is_null($return)) {
			$return = $GLOBALS['SQ_SYSTEM']->user;
		}

		return $return;

	}//end _getManagedAsset()


	/**
	* Return the friendly name of the asset type of the managed asset
	*
	* @return string
	* @access private
	*/
	function _getManagedAssetTypeName()
	{
		$asset = $this->_getManagedAsset();
		return $asset->name;

	}//end _getManagedAssetTypeName()


//--        Selections        --//


	/**
	* Provide the default values for each of the selection fields
	* Link_Manager grabs its values from whatever folders the asset is linked to
	*
	* @param string	$selection_name	the id of the asset selection
	* @param string	$assetid		the assetid of the item to grab out of the cart
	*
	* @return int
	* @access public
	*/
	function getAssetSelectionValue($selection_name, $assetid)
	{
		// if the selection name matches the hard-set selection
		if ($selection_name == SQ_PAGE_LINK_MGR_PRESET_LINKED) {

			$asset_links = $this->_getAssetLinks($this->_getManagedAsset());

			// check whether this assetid is linked
			if (!empty($asset_links[$assetid])) {
				// if radio button, return the linked assetid
				// otherwise, return 1 for every time a linked asset is found
				$selections = $this->attr('asset_selections');
				if (isset($selections[SQ_PAGE_LINK_MGR_PRESET_LINKED]['type'])
						&& $selections[SQ_PAGE_LINK_MGR_PRESET_LINKED]['type'] == 'radio'
						&& empty($selection[SQ_PAGE_LINK_MGR_PRESET_LINKED]['options']['allow_multiple'])) {
					return $assetid;
				} else {
					return 1;
				}
			}
		}

		return 0;

	}//end getAssetSelectionValue()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		// Tie the process and following paint together using db2 in order to
		// avoid possible replication slowdown on db1
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$this->processAssetSelections();
		parent::printBody();

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

	}//end printBody()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printContents()
	{
		// print the 'not logged in' bodycopy if the user hasn't logged in
		$public_user = $GLOBALS['SQ_SYSTEM']->getPublicUser();
		if ($GLOBALS['SQ_SYSTEM']->currentUser($public_user)) {

			$bodycopy = $this->getBodycopy('not_logged_in');
			if (is_null($bodycopy)) return FALSE;

			require_once SQ_FUDGE_PATH.'/general/text.inc';
			$keywords     = $bodycopy->getKeywords();
			$replacements = Array();
			foreach ($keywords as $keyword) {
				$replacements[$keyword] = $this->getKeywordReplacement($keyword);
			}
			$bodycopy->setKeywordReplacements($replacements);
			$bodycopy->printBody();
			return;

		} else {
			return parent::printContents();
		}

	}//end printContents()


	/**
	* Preprocess the Asset Selection values before displaying the page or redirecting
	* Overwritten to handle the asset linking
	*
	* @return boolean
	* @access public
	*/
	function processAssetSelections()
	{
		$managed_asset = $this->_getManagedAsset();

		// get the assets the managed asset is linked to, and make sure the list isn't cached
		$asset_links = $this->_getAssetLinks($managed_asset, TRUE);

		$asset_selection_linked = array_get_index($this->attr('asset_selections'), SQ_PAGE_LINK_MGR_PRESET_LINKED, Array());

		// is the asset selection a checkbox?
		$is_checkbox = array_get_index($asset_selection_linked, 'type', 'radio')
					   && array_get_index(array_get_index($asset_selection_linked, 'options', Array()), 'allow_multiple', 1);

		// take effect if the selection is present
		$request_index = $this->_getSelectionFieldNamePrefix(SQ_PAGE_LINK_MGR_PRESET_LINKED);
		if ((!$is_checkbox && isset($_REQUEST[$request_index])) || ($is_checkbox && isset($_REQUEST['selection_flag_'.$request_index]))) {
			$new_links = array_get_index($_REQUEST, $request_index, Array());

			// sanitise the new links for later
			if (!is_array($new_links)) {
				$new_links = Array(
								$new_links	=> $new_links,
							 );
			} else {
				foreach ($new_links as $index => $ticked) {
					$new_links[$index] = $index;
				}
			}

			// if the listing is paginated, make sure we're only modifying the displayed assets
			// this way is terribly inefficient, but I'm not sure how to do this without massive cut-pastes
			$num_per_page = $this->attr('num_per_page');
			if ($num_per_page > 0) {
				$children = $this->getAssetList();
				$this->filterAssetList($children);

				// the list is sorted and positions set, now we can split to groups
				if ($this->attr('group_by') == 'grouped') {
					$this->groupAssetsRecursively($this->attr('asset_grouping'), $children, $children);
					$this->sortGroups($this->attr('asset_grouping'), $children, $children);
				}

				$asset_result_page_var   = 'result_'.$this->id.'_result_page';
				$generic_result_page_var = 'result_page';

				// have [assetid]_result_page take precedence over result_page
				$result_page = 1;
				if (isset($_REQUEST[$generic_result_page_var])) {
					$result_page = (int)$_REQUEST[$generic_result_page_var];
				}
				if (isset($_REQUEST[$asset_result_page_var])) {
					$result_page = (int)$_REQUEST[$asset_result_page_var];
				}

				$num_per_page = $this->attr('num_per_page');
				if ($result_page <= 0) $result_page = 1;

				$replacements = Array();

				$chunk           =& $this->getChunk($children, $replacements, Array(), $result_page, $num_per_page);
				$flattened_chunk = dal_array_flatten($chunk, 'type_code');

				// remove any assets from the asset_links array that isn't visible on the page
				foreach ($asset_links as $assetid => $value) {
					if (!isset($flattened_chunk[$assetid])) {
						unset($asset_links[$assetid]);
					}
				}
			}//end if

			// keep forced, but make sure we have link integrity turned on
			$run_level = SQ_RUN_LEVEL_FORCED | SQ_SECURITY_LINK_INTEGRITY;
			$GLOBALS['SQ_SYSTEM']->setRunLevel($run_level);
				$done = $this->processMinorLinks($managed_asset->id, $asset_links, $new_links, $this->attr('link_type'));
			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

			// Clear the user groups cache so that any new user groups are picked
			// up (assuming we are working on a user here - otherwise this will
			// have no effect, but no harm done either)
			unset($_SESSION['sq_user_groups']);

			$this->generateMessages($done);
		}//end if

		// update the lookups of the managed asset
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array('assetids' => Array($managed_asset->id));
		$hh->freestyleHipo('hipo_job_update_lookups', $vars);

		return TRUE;

	}//end processAssetSelections()


	/**
	* Handle linking this asset under multiple parents
	*
	* Return an array of success/failure information
	*
	* @param int	$minor_id	asset we are going to link
	* @param array	$old_links	The current assets we are linked to
	* @param array	$new_links	The new selections we wish to be linked to
	* @param int	$link_type	type of link to use
	*
	* @return array
	* @access public
	*/
	function processMinorLinks($minor_id, $old_links=Array(), $new_links=Array(), $link_type)
	{
		$cannot_create =  Array();
		$cannot_delete =  Array();
		$to_delete     =  array_diff($old_links, $new_links);
		$to_create     =  array_diff($new_links, $old_links);
		$minor         = $GLOBALS['SQ_SYSTEM']->am->getAsset($minor_id);

		$done          = Array();

		// loop through and link our minor under all majors
		foreach ($to_create as $majorid) {
			$major = $GLOBALS['SQ_SYSTEM']->am->getAsset($majorid);
			if (!$GLOBALS['SQ_SYSTEM']->am->createAssetLink($major, $minor, $link_type)) {
				$done['add_error'][] = $major->id;
			} else {
				$done['add_success'][] = $major->id;
			}

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($major);
		}
		// handle links marked to be removed
		foreach ($to_delete as $majorid) {
			$curr_link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($majorid, $minor_id, $link_type);
			$link_id = $curr_link['linkid'];

			if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($link_id)) {
				$done['delete_error'][] = $majorid;
			} else {
				$done['delete_success'][] = $majorid;
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
	* @param array	$fail_success	Takes an array of success/failure messages (generated by processMinorLinks)
	*
	* @return void
	* @access public
	*/
	function generateMessages($fail_success)
	{
		$messages = Array(
			'add_success'    => translate('Successfully added "%s" to the following: %s'),
			'delete_success' => translate('Successfully removed "%s" from the following: %s'),
			'add_error'      => translate('Unable to add "%s" to the following: %s'),
			'delete_error'   => translate('Unable to remove "%s" from the following: %s'),
		);
		cms_link_manager_message_
		foreach ($fail_success as $message_type => $assetids) {
			switch ($message_type) {
				case 'add_success':
				case 'delete_success':
					$this->messages[] = sprintf($messages[$message_type], $this->_getManagedAssetTypeName(), $this->_getCommaSeparatedNamesByAssetid($assetids));
				break;
				case 'add_error':
				case 'delete_error':
					$this->errors[] = sprintf($messages[$message_type], $this->_getManagedAssetTypeName(), $this->_getCommaSeparatedNamesByAssetid($assetids));
				break;
			}
		}

	}//end generateMessages()


	/**
	* Retrieves a bodycopy's contents by name: returns the bodycopy's contents,
	* or empty string if the name does not match any bodycopies
	*
	* @param string	$name			the name of the bodycopy to retrieve
	* @param string	$replacements	the keyword replacements to apply to the contents
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
	* Retrieves a bodycopy by name: returns the bodycopy object, or
	* NULL if the name does not match any bodycopies.
	*
	* @param string	$name	the name of the bodycopy to retrieve
	*
	* @return mixed object|NULL
	* @access private
	* @see Search_Page::bodycopies
	*/
	function &getBodycopy($name)
	{
		if (array_key_exists($name, $this->bodycopies)) {
			$link     =  $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, $name, 'major', '1');
			$bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			return $bodycopy;
		} else {
			return NULL;
		}

	}//end getBodycopy()


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
	* Takes an array of assetids and returns the asset names as a comma-separated string
	*
	* @param array	$assetids	the list of assetids to prettify
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
		if (!empty($names)) {
			// split out the names into something that looks like: "Foo", "Bar"
			$string .= '"'.implode('", "', $names).'"';
		}

		return $string;

	}//end _getCommaSeparatedNamesByAssetid()


//--        Keywords        --//


	/**
	* Return the available keywords in the Page Contents Bodycopy for this asset
	*
	* The return value is in the form:
	* <pre>
	* Array(
	*     'keyword' => 'name',
	* )
	* </pre>
	*
	* @return array
	* @access public
	*/
	function _getContentsKeywords()
	{
		$keywords = parent::_getContentsKeywords();

		// messages
		$keywords += Array(
						'error_messages_list'	=> translate('The list of error messages'),

						'success_messages_list'	=> translate('The list of success messages'),

					 );

		// add replacements for the managed assets
		$managed_asset    = $this->_getManagedAsset();
		$managed_keywords =  $managed_asset->getAvailableKeywords();
		foreach ($managed_keywords as $keyword => $description) {
			// upper and lowercase
			$keywords['managed_asset_'.$keyword] = str_replace('Asset', 'Managed Asset', str_replace('asset', 'managed asset', $description));
		}
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($managed_asset);

		return $keywords;

	}//end _getContentsKeywords()


	/**
	* Get bodycopy keyword replacement
	*
	* Provides a mechanism for providing keyword replacement functions by
	* looking up the function named 'getKeywordNameKeywordReplacement'
	* so an asset wishing to add support for 'best_ever' custom keyword
	* should implement a function getBestEverKeywordReplacement()
	* which returns a replacement string
	*
	* @param string	$keyword	Keyword to find a replacement for
	*
	* @return string
	* @access public
	*/
	function getKeywordReplacement($keyword)
	{
		// Handle keywords for the managed asset
		if (strpos($keyword, 'managed_asset_') === 0) {
			$managed_asset = $this->_getManagedAsset();
			if (isset($managed_asset)) {
				return $managed_asset->getKeywordReplacement(substr($keyword, strlen('managed_asset_')));
			}
		}

		return parent::getKeywordReplacement($keyword);

	}//end getKeywordReplacement()


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
	* @param array	$messages	an array of messages to print
	* @param string	$suffix		the suffix to bolt onto the asset prefix, to use as the list element classes
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
	* Get the prefix to use for input elements relevant to the specified selection
	*
	* @param string	$selection_name	The name of the selection
	*
	* @return string
	* @access protected
	*/
	function _getSelectionFieldNamePrefix($selection_name)
	{
		return $this->getPrefix().'_'.parent::_getSelectionFieldNamePrefix($selection_name);

	}//end _getSelectionFieldNamePrefix()


}//end class

?>
