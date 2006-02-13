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
* $Id: listing_engine.inc,v 1.38.2.32 2006/02/13 23:10:56 sdanis Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';


/**
* Listing_Engine
*
* Purpose
*     Provides common listing services to a range of CMS and search
*     related assets, such as Asset Listing and Search Page. This is
*     a 'base asset' that should not be created directly - rather you
*     should create its children
*
*
* @author  Luke Wright   <lwright@squiz.net>
* @author  David Heppell <dheppell@squiz.net>
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @author  Greg Sherwood <greg@squiz.net>
* @author  Andrei Railean <arailean@squiz.net>
*
* @version $Revision: 1.38.2.32 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Listing_Engine extends Page
{


	/**
	* Determines whether the output of this asset should be inside the form
	*
	* @var boolean
	*/
	var $requires_form = false;

	/**
	* Provides a list of keywords and their names
	*
	* @var array
	*/
	var $contents_keywords = Array();


	function Listing_Engine($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Page($assetid);

		$this->contents_keywords['sort_by']		= translate('cms_listing_keyword_sort_by');
		$this->contents_keywords['sort_order']		= translate('cms_listing_keyword_sort_order');
		$this->contents_keywords['submit_button']	= translate('cms_listing_keyword_submit_button');
		$this->contents_keywords['root_nodes']		= translate('cms_listing_keyword_root_nodes');

	}//end constructor


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Asset listings create bopycopy and folder asset when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return false;

		// add a bodycopy to this page when creating
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');

		$sub_assets= Array(
						'type_formats'		=> 'folder',
						'position_formats'	=> 'folder',
						'group_formats'		=> 'folder',
					 );

		$type_formats = null;
		foreach ($sub_assets as $name => $type) {
			$asset =& new $type();
			$copy_link = Array('asset' => &$this, 'value' => $name ,'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

			$asset->setAttrValue('name', ucwords(str_replace('_',' ', $name)));
			if (!$asset->create($copy_link)) return false;

			if ($name == 'type_formats') $type_formats = &$asset;

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			unset($asset);
		}
		$asset =& new Bodycopy();
		$copy_link = Array('asset' => &$type_formats, 'value' => 'default_format' ,'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);
		$asset->setAttrValue('name', translate('default_format'));

		return $asset->create($copy_link);

	}//end _createAdditional()


	/**
	* Returns a list of lock types available for this asset type
	*
	* Only maps Content lock type to the Menu lock type (Attributes|Links) for
	* backward compatibilty purposes with Asset Listing. {@internal If this
	* mapping is not required then this function can be removed}
	*
	* @return array
	* @access public
	*/
	function lockTypes()
	{
		$lock_types = parent::lockTypes();
		$lock_types['content'] = $lock_types['menu'];
		return $lock_types;

	}//end lockTypes()


	/**
	* Prepares for linking by checking that the link values are valid for this link
	*
	* This function will be called if this asset is the major or minor party in the link, so the side_of_link flag
	* should be checked to work out what side of the link this asset is on. The return value should indicate
	* if any of the link fields were changed.
	*
	* @param object	&$asset			the major or minor asset that we are linking to
	* @param string	$side_of_link	the side of the link we are on (major or minor)
	* @param string	&$link_type		the type of link we are creating
	* @param string	&$value			the value that is to be associated with the link
	* @param string	&$sort_order	the position in the links list that this link should take,
	* @param string	&$dependant		'0' / '1' on whether the this asset is dependant on the
	*								asset that will be linked by the new link
	* @param string	&$exclusive		'0' / '1' on whether the this asset is linked exclusivly
	*								to the asset that will be linked by the new link
	*
	* @return boolean
	* @access public
	*/
	function prepareLink(&$asset, $side_of_link, &$link_type, &$value, &$sort_order, &$dependant, &$exclusive)
	{
		// if a bodycopy is linking to us then we need to make it a dependant link
		if ($side_of_link == 'major' && is_a($asset, 'bodycopy') && $dependant != '1') {
			$dependant = '1';
			return true;
		}

		return false;

	}//end prepareLink()


	/**
	* Return a human readable description of the passed link
	*
	* @param int	$linkid	the link ID of the link to describe
	*
	* @return string
	* @access public
	*/
	function describeLink($linkid)
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid);
		switch (strtolower($link['value'])) {
			case 'root' :
				return translate('cms_listing_root_node_link_desc');
			break;
			default :
				return parent::describeLink($linkid);
			break;
		}

	}//end describeLink()


	/**
	* Returns TRUE if we can delete the passed link, or a string with the error msg
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return mixed boolean|string
	* @access public
	*/
	function isDeletableLink($linkid)
	{
		$folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', true, 'type_formats');
		if ($folder_link['linkid'] == $linkid && !$GLOBALS['SQ_PURGING_TRASH']) {
			return translate('cms_listing_cannot_delete_type_formats_link', str_replace('_',' ', $this->type()));
		}

		$asset_folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', true, 'position_formats');
		if ($asset_folder_link['linkid'] == $linkid && !$GLOBALS['SQ_PURGING_TRASH']) {
			return translate('cms_listing_cannot_delete_position_formats_link', str_replace('_',' ', $this->type()));
		}

		return parent::isDeletableLink($linkid);

	}//end isDeletableLink()


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
		$page_links[SQ_LINK_TYPE_2]['folder']   = Array('card' => 'M', 'exclusive' => false);
		$page_links[SQ_LINK_NOTICE]['asset']    = Array('card' => 'M', 'exclusive' => false);
		return $page_links;

	}//end _getAllowedLinks()


	/**
	* Return a reference to a folder linked to this asset determined by the passed in value on the link
	*
	* @param string	$type	the value on the link for this folder
	*
	* @return object
	* @access public
	*/
	function &getFolder($type='type_formats')
	{
		$null = null;
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', true, $type);
		if (empty($link)) return $null;

		$folder = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		if (is_null($folder)) return $null;

		return $folder;

	}//end getFolder()


	/**
	* Get a list of all formats of a certain type
	*
	* The results of this function are cached for the length of the script execution
	*
	* @param string	$type	the value on the link for the formats folder
	*
	* @return array
	* @access public
	*/
	function getFormats($type='type_formats')
	{
		if (!isset($this->_tmp['formats'][$type])) {
			$folder = &$this->getFolder($type);
			$format_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($folder->id, SQ_LINK_TYPE_2, 'bodycopy', true);
			if (empty($format_links)) return Array();

			$formats = Array();
			foreach ($format_links as $link_data) {
				$formats[$link_data['minorid']] = $link_data['value'];
			}
			$this->_tmp['formats'][$type] = $formats;
		}
		return $this->_tmp['formats'][$type];

	}//end getFormats()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		// get the contents of this page
		// we do this because when contents are prepared the query vars that are used by the form elements
		// need to be unset for us to use them in the form action string
		ob_start();
			$this->printContents();
		$contents = ob_get_clean();

		$used_fields = $this->getUsedFormFields();
		$keys_to_remove = Array();

		// we're removing all the contents
		foreach ($used_fields as $key) {
			$keys_to_remove[$key] = null;
		}

		if (!empty($used_fields) || $this->requires_form) {
			?>
				<form name="listing_form" method="get" action="<?php echo htmlspecialchars(replace_query_string_vars($keys_to_remove)); ?>">
				<?php echo $contents; ?>
				</form>
			<?php
		} else {
			echo $contents;
		}

	}//end printBody()


	/**
	* Dummy function that is used by printBody to generate the page contents
	*
	* All extending assets should override this
	*
	* @return string
	* @access public
	*/
	function printContents()
	{
		return '';

	}//end printContents()


	/**
	* Work out the unique cache key to represent the current page
	*
	* @return string
	* @access private
	*/
	function _getCacheKey()
	{
		$group_by = $this->attr('group_by');

		// we work out what screen we are on here so we can correctly cache the screen with a unique ID
		// note that we never cache the results of a random asset listing for obvious reasons
		$cache_key = '';
		switch ($group_by) {
			case 'grouped' :
			case 'number' :
				$cache_key = (isset($_REQUEST['result_page'])) ? (int)$_REQUEST['result_page'] : 1;
				if ($cache_key <= 0) $cache_key = 1;
			break;

			case 'letter' :
				$cache_key = (isset($_REQUEST['result_page'])) ? $_REQUEST['result_page'] : 'A';
			break;
		}

		// cache key takes into account different sorting scenarios
		$cache_key .= serialize($this->getSortInfo());
		$cache_key .= (int)$this->isDescending();

		return $cache_key;

	}//end _getCacheKey()


	/**
	* Returns keyword replacements to be replaced in the body
	* of the format bodycopy
	*
	* @param string	$keyword	keyword to replace
	*
	* @return array
	* @access public
	*/
	function getKeywordReplacement($keyword)
	{
		if (empty($keyword)) return '';

		$replacement = "%$keyword%";
		$tmp_keyword = strtr($keyword, '_', ' ');
		$tmp_keyword = ucwords($tmp_keyword);
		$tmp_keyword = preg_replace('/\s+/', '', $tmp_keyword);
		$func_name = 'get'.$tmp_keyword.'KeywordReplacement';
		if (method_exists($this, $func_name)) {
			$replacement = $this->$func_name();
		}

		return $replacement;

	}//end getKeywordReplacement()


	/**
	* Get the list of asset that should be printed
	*
	* This is defined as an abstract method of sorts; it's up to the kids to
	* override this
	* The return array is in the form Array(assetid => type_code) unless
	* we are grouping by letter, in which case the return array is
	* <pre>
	* Array(assetid => Array(
	*						'type_code' => type_code,
	*						'first_letter' => first_letter,
	*						);
	*		);
	* </pre>
	*
	* @return array
	* @access protected
	*/
	function getAssetList()
	{
		return Array();

	}//end getAssetList()


	/**
	* Sort the list of assets
	*
	* @param array	$asset_list	an array of assets to be sorted
	* @param string	$sort_info	sorting information as provided by getSortInfo()
	*
	* @return array
	* @access public
	* @see Listing_Engine::getSortInfo()
	*/
	function sortAssetList($asset_list, $sort_info)
	{

		if (count($asset_list) == 1 || empty($sort_info)){
			return $asset_list;
		}

		if ((!is_array($asset_list)) || (empty($asset_list))) {
			return Array();
		}

		$skipped_assets = Array();
		$sort_assets = Array();

		// default to a numeric sort until we are proven otherwise
		$sort_numeric = true;

		// check and prepare sorting data
		$is_sort_by_attribute = ($sort_info['type'] == 'asset_attrib');
		$is_sort_by_field     = ($sort_info['type'] == 'field');

		$sort_params =& $sort_info['params'];

		$assetids = array_keys($asset_list);

		if ($is_sort_by_attribute) {
			$sort_by_attr = $sort_params['attribute'];
			$sort_by_type_code = $sort_params['type_code'];

			foreach ($this->attr('types') as $type_code => $inherit) {
				// initially we plan not to process this type code
				$process = false;

				if ($type_code == $sort_by_type_code){
					$process = true;

				} else {
					if ($inherit) {
						// we have to find our closest ancestor with a sort field
						$type_anc = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($type_code);
						array_unshift($type_anc, $type_code);

						foreach ($type_anc as $this_type_code) {
							if ($this_type_code == $sort_by_type_code){
								$process = true;
								break;
							}
						}
					}
				}

				// get the assetids that may be affected
				$rem_assetids = array_keys($GLOBALS['SQ_SYSTEM']->am->getAssetInfo($assetids, $type_code, !$inherit));

				// do we have an attribute to sort on for this type code?
				// if we don't, we'll have to come back to it as a skipped asset
				if ($process) {
					if (!in_array($sort_params['attr_type'], Array('int', 'float'))) {
						$sort_numeric = false;
					}

					$attr_info = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName($sort_by_attr, $sort_by_type_code, $rem_assetids);
					foreach ($rem_assetids as $assetid) {
						if (isset($attr_info[$assetid])) {
							$sort_assets[$assetid] = strtolower($attr_info[$assetid]);

							if (($pos = array_search($assetid, $skipped_assets)) !== false) {
								unset($skipped_assets[$pos]);
							}
						}
					}

					$skipped_assets = array_merge($skipped_assets, array_diff($rem_assetids,array_keys($attr_info)));
				} else {
					// add to the skipped assets array if still in the list
					// of used type codes
					$skipped_assets = array_merge($skipped_assets, $rem_assetids);
				}
			}

		} else {
			$sort_field = $sort_params['field'];

			if ($sort_field == 'no_sorting') return $asset_list;

			if (in_array($sort_field, $this->getSortableAssetInfo())) {
				$sort_assets = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($assetids, null, true, $sort_field);
				for (reset($sort_assets); null !== ($assetid = key($sort_assets)); next($sort_assets)) {
					$sort_assets[$assetid] = strtolower($sort_assets[$assetid]);
				}

				if ($sort_field != 'assetid') $sort_numeric = false;
			} else {
				// sort field is not defined, return the assetlist
				return $asset_list;
			}
		}

		$reverse_sort = $this->isDescending();

		if ($sort_numeric) {
			uasort($sort_assets, create_function('$a,$b','return '.($reverse_sort? '$b - $a' : '$a - $b').';'));
		} else {
			if ($reverse_sort) {
				arsort($sort_assets);
			} else {
				asort($sort_assets);
			}
		}

		$assets_to_list = Array();
		$sorted_assets = array_keys($sort_assets);
		foreach ($sorted_assets as $asset) {
			$assets_to_list[$asset] = $asset_list[$asset];
		}

		// add skipped assets to the bottom of the list
		foreach ($skipped_assets as $asset) {
			$assets_to_list[$asset] = $asset_list[$asset];
		}

		return $assets_to_list;

	}//end sortAssetList()


	/**
	* Remove unwanted assets from the todo list
	*
	* @param array	&$todo	an array of assets to list in the same format as the return
	*						value of getAssetList()
	*
	* @return void
	* @access protected
	* @see getAssetList()
	*/
	function filterAssetList(&$todo)
	{
		$group_by = $this->attr('group_by');

		// remove non-live assets that the user doesnt have write access to
		if (!empty($todo)) $this->filterAssetStatuses($todo);
		if (empty($todo)) return;

		// remove unwanted asset types
		switch ($group_by) {
			case 'number' :
			case 'random' :
			case 'grouped' :
				$this->filterAssetTypes($todo);
				$this->adjustAssetPositions($todo);
			break;

			case 'letter' :
				$filtered_todo = Array();
				for (reset($todo); null !== ($todo_id = key($todo)); next($todo)) {
					$filtered_todo[$todo_id] =& $todo[$todo_id]['type_code'];
				}
				$this->filterAssetTypes($filtered_todo);
				foreach ($filtered_todo as $todo_id => $type_code) {
					$todo[$todo_id]['type_code'] = $type_code;
				}
				unset($filtered_todo);
			break;
		}

	}//end filterAssetList()


	/**
	* Remove non-live assets from the list of assets to print if the user doesnt have write access
	*
	* The assets we filter out here could have read access granted for the current user, but
	* non-live assets require write access to be viewed, and we need to load the asset to ask for that.
	*
	* @param array	&$todo	an array of assets to list in the same format as the return
	*						value of getAssetList()
	*
	* @return void
	* @access protected
	* @see getAssetList()
	*/
	function filterAssetStatuses(&$todo)
	{
		// first check to see if this is a special user without restrictions
		$user_restrictions = (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin());
		if (!$user_restrictions) return;

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$assetids = array_keys($todo);
		if (empty($assetids)) return;

		for (reset($assetids); null !== ($k = key($assetids)); next($assetids)) {
			$assetids[$k] = $db->quote($assetids[$k]);
		}

		// get a list of assets that are not live
		if (!empty($assetids)) {
			$sql = 'SELECT assetid from sq_ast
					WHERE assetid IN ('.implode(',', $assetids).')
					  AND status < '.$db->quote(SQ_STATUS_LIVE);
			$non_live_assets = $db->getCol($sql);
			assert_valid_db_result($non_live_assets);

			foreach ($non_live_assets as $assetid) {
				//we get the type code from a different place for letters
				$type_code = ($this->attr('group_by') == 'letter') ? $todo[$assetid]['type_code'] : $todo[$assetid];

				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type_code);
				if (!$asset->writeAccess()) unset($todo[$assetid]);
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				unset($asset);
			}
		}

		return;

	}//end filterAssetStatuses()


	/**
	* Remove and/or update the todo list based on the types of the assets in it
	*
	* If an entry in the list is a child of a wanted asset type, its type code is changed
	* to the wanted parent type.  If an entry is not a wanted type or a child thereof,
	* it is removed.  Otherwise it's left alone.
	*
	* @param array	&$todo	an array of assets to list in the same format as the return
	*						value of getAssetList()
	*
	* @return void
	* @access protected
	* @see getAssetList()
	*/
	function filterAssetTypes(&$todo)
	{
		$allowed_types = $this->getAllowedAssetTypeMap();
		if (empty($allowed_types)) {
			$todo = Array();
			return;
		}

		foreach ($todo as $assetid => $type_code) {
			if (!isset($allowed_types[$type_code])) {
				unset($todo[$assetid]);
			} else {
				$todo[$assetid] = $allowed_types[$type_code];
			}
		}

	}//end filterAssetTypes()


	/**
	* Get the mapping from assets type to the parent type that is handling presentation
	*
	* Returns the map in array format, with allowed asset type as an index
	* with the parent asset being one of the elements of $this->attr('types');
	* Because presentation (type format) is defined only for each of $this->attr('types'),
	* this helps determine the handler of the presentation for a particular type
	* Also, if a particular asset type is defined as the index, that type is allowed to be displayed
	* Handler Asset Type can be the same as Asset Type. Resulting array is in following format:
	* Array['Asset Type'] = 'Handler Asset Type'
	* where Handler Asset Types are unique
	*
	* @return array
	* @access public
	*/
	function getAllowedAssetTypeMap()
	{
		if (isset($this->_tmp['allowed_asset_map'])) {
			return $this->_tmp['allowed_asset_map'];
		}

		$wanted_types = $this->attr('types');
		if (empty($wanted_types)) return Array();

		$allowed_asset_map = Array();
		foreach ($wanted_types as $allowed_parent_type => $inherit) {
			if (trim($allowed_parent_type) == '') {
				continue;
			} else if (!$inherit) {
				$allowed_asset_map[$allowed_parent_type] = $allowed_parent_type;
			} else {
				$descendants = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($allowed_parent_type, true);
				foreach ($descendants as $allowed_type) {
					if (!isset($allowed_asset_map[$allowed_type])){
						$allowed_asset_map[$allowed_type] = $allowed_parent_type;
					}
				}
			}
		}

		$this->_tmp['allowed_asset_map'] = $allowed_asset_map;
		return $allowed_asset_map;

	}//end getAllowedAssetTypeMap()


	/**
	* Place assets in hard set list positions
	*
	* @param array	&$todo	an array of assets to list in the same format as the return
	*						value of getAssetList()
	*
	* @return void
	* @access protected
	* @see getAssetList()
	*/
	function adjustAssetPositions(&$todo)
	{
		$asset_positions = $this->attr('asset_positions');
		if (empty($asset_positions)) return;

		// replace the original todo list because we are going to build another one
		$all_assets = $todo;
		$adjusted_positions = Array();
		$allowed_asset_map = $this->getAllowedAssetTypeMap();

		// re-adjustment depends on the fact the asset_positions is oredered by position
		// note: list length can only be extended by placing items inside it, not at the end
		// this means that if the list has N elements, you cannot customize position N+1,
		// unless you first insert something into the middle of the list, which will make it N+1 long
		$maximum_possible_position = count($all_assets);
		foreach ($asset_positions as $position => $asset_data) {
			$id = $asset_data['id'];
			if ($position > $maximum_possible_position) {
				// if this position does not fit, neither will anything else
				break;
			}

			if (!isset($all_assets[$id])) {
				// inserting a new asset
				$maximum_possible_position++;
				$type = $asset_data['type'];
				$all_assets[$id] = array_get_index($allowed_asset_map, $type, $type);
			} else {
				// rearranging existing list
				unset($todo[$id]);
			}

			$adjusted_positions[$position] = $id;
		}

		// check if adjustment is still required
		if (empty($adjusted_positions)) return;

		$originally_ordered_ids = array_keys($todo);
		$todo = Array();

		$current_position = 1;
		do {
			if (isset($adjusted_positions[$current_position])) {
				$current_id = $adjusted_positions[$current_position];
				unset($adjusted_positions[$current_position]);
			} else {
				$current_id = array_shift($originally_ordered_ids);
			}

			$todo[$current_id] = array_get_index($all_assets, $current_id);

			$current_position++;
		} while (!empty($originally_ordered_ids) || isset($adjusted_positions[$current_position]));

	}//end adjustAssetPositions()


	/**
	* Gets the specified chunk based on group-by and pagination options
	*
	* @param array	&$children		the list of assets that qualify to be
	*								printed
	* @param array	&$replacements	the current replacements array, which will
	*								be added to by this function
	* @param array	$result_page	result page
	* @param array	$num_per_page	number of assets per page
	* @param array	$group_by		what to group by
	*
	* @return array
	* @access public
	*/
	function &getChunk(&$children, &$replacements, $result_page, $num_per_page, $group_by=null)
	{
		// grouping by number shows a certain number of results per page (eg. 10 assets per page)
		// grouping by letter makes this template an A-Z listing of assets
		// grouping by 'random' means that we grab X random assets
		//grouping by custom groups means we will follow the user defined group structure
		if (is_null($group_by)) {
			$group_by = $this->attr('group_by');
		}

		switch ($group_by) {
			case 'number' :
				// do it CHUNK Noris style
				if ($num_per_page > 0) {
					$chunks = array_chunk($children, $num_per_page, true);
				} else {
					$chunks = Array('0' => $children);
				}

				if ($result_page > count($chunks)) {
					$result_page = count($chunks);
				}
				$todo = $chunks[($result_page-1)];

				// previous page link
				if ($result_page <= 1) {
					$replacements['previous_page'] = $this->attr('prev_page_text');
				} else {
					$replacements['previous_page'] = '<a href="'.replace_query_string_vars(Array('result_page' => $result_page-1)).'">'.$this->attr('prev_page_text').'</a>';
				}

				// next page link
				if ($result_page >= count($chunks)) {
					$replacements['next_page'] = $this->attr('next_page_text');
				} else {
					$replacements['next_page'] = '<a href="'.replace_query_string_vars(Array('result_page' => $result_page+1)).'">'.$this->attr('next_page_text').'</a>';
				}

				$replacements['current_page'] = (int)$result_page;
				$replacements['total_pages'] = count($chunks);

				// list of pages and links to them
				$page_list = '';
				for ($i = 1; $i <= count($chunks); $i++) {
					if ($i == $result_page) {
						$page_list .= " $i ";
					} else {
						$page_list .= ' <a href="'.replace_query_string_vars(Array('result_page' => $i)).'">'.$i.'</a> ';
					}
				}
				$replacements['page_list'] = $page_list;

				unset($chunks);
			break;


			case 'letter' :
				foreach ($children as $childid => $data) {
					$todo[strtolower($data['first_letter'])][$childid] = $data['type_code'];
				}

				$result_page = (isset($_REQUEST['result_page'])) ? $_REQUEST['result_page'] : '';
				if (empty($result_page)) {
					for ($i = 65; $i <= 90; $i++) {
						if (isset($todo[strtolower(chr($i))])) {
							$result_page = chr($i);
							break;
						}
					}
				}

				// previous page link
				$result_letter = ord(strtoupper($result_page));
				$replacements['previous_page'] = $this->attr('prev_page_text');
				if ($result_letter > 65) {
					for ($i = ($result_letter-1); $i >= 65; $i--) {
						if (isset($todo[strtolower(chr($i))])) {
							$replacements['previous_page'] = '<a href="'.replace_query_string_vars(Array('result_page' => chr($i))).'">'.$this->attr('prev_page_text').'</a>';
							break;
						}
					}
				}

				// next page link
				$replacements['next_page'] = $this->attr('next_page_text');
				if ($result_letter < 90) {
					for ($i = ($result_letter+1); $i <= 90; $i++) {
						if (isset($todo[strtolower(chr($i))])) {
							$replacements['next_page'] = '<a href="'.replace_query_string_vars(Array('result_page' => chr($i))).'">'.$this->attr('next_page_text').'</a>';
							break;
						}
					}
				}

				$replacements['current_page'] = strtoupper($result_page);
				$replacements['total_pages'] = 26;

				// list of pages and links to them
				$page_list = '';
				for ($i = 65; $i <= 90; $i++) {
					if (isset($todo[strtolower(chr($i))])) {
						$page_list .= ' <a href="'.replace_query_string_vars(Array('result_page' => chr($i))).'">'.chr($i).'</a> ';
					} else {
						$page_list .= ' '.chr($i).' ';
					}
				}
				$replacements['page_list'] = $page_list;

				if (isset($todo[strtolower($result_page)])) {
					$todo = $todo[strtolower($result_page)];
				} else {
					$todo = Array();
				}

			break;

			case 'random' :
				// random does things differently, as there is no pagination. But this is a good place
				// to grab the random asset
				$count = count($children);
				$num_per_page = $this->attr('num_per_page');
				$todo = Array();

				if ($count == 1) {
					$todo = $children;
				} else if ($num_per_page <= 1) {
					// grab a single asset to display
					$random_asset = array_rand($children);
					$todo[$random_asset] = $children[$random_asset];
				} else {
					// this will return a subset of the children, as defined by the num_per_page
					// attribute.
					$random_assets = array_rand($children, min($num_per_page, $count));
					foreach ($random_assets as $value) {
						$todo[$value] = $children[$value];
					}
				}

			break;

			case 'grouped' :
				//we need to do an element count so we can work out the page
				//stuff
				$total_assets = $this->_arrayCountRecursive($children, count($this->attr('asset_grouping')));
				$count = $num_per_page * $result_page;

				//if were showing all, make our count equal to all
				if ($count == 0) {
					$num_per_page = $total_assets;
					$count = $total_assets;
				}

				//get a chunk of the size were after, $todo will have the contents
				//after the function call
				$todo = Array();

				// chunk it hard, baby
				$this->_groupChunk($children, $todo, $count, $num_per_page, count($this->attr('asset_grouping')));

				if ($num_per_page == 0) {
					$num_per_page = count($todo);
					$num_pages = 1;
				} else {
					$chunks = array_chunk($todo, $num_per_page, true);
					$num_pages = ceil($total_assets / $num_per_page);
					//we want the last chunk
					$todo = end($chunks);
				}

				// if the result page is too high, set to last page
				$result_page = min($result_page, ceil($total_assets / $num_per_page));

				// previous page link
				if ($result_page <= 1) {
					$replacements['previous_page'] = $this->attr('prev_page_text');
				} else {
					$replacements['previous_page'] = '<a href="'.replace_query_string_vars(Array('result_page' => $result_page-1)).'">'.$this->attr('prev_page_text').'</a>';
				}

				// next page link
				if ($result_page >= $num_pages) {
					$replacements['next_page'] = $this->attr('next_page_text');
				} else {
					$replacements['next_page'] = '<a href="'.replace_query_string_vars(Array('result_page' => $result_page+1)).'">'.$this->attr('next_page_text').'</a>';
				}

				$replacements['current_page'] = (int)$result_page;
				$replacements['total_pages'] = $num_pages;

				// list of pages and links to them
				$page_list = '';
				for ($i = 1; $i <= $num_pages; $i++) {
					if ($i == $result_page) {
						$page_list .= " $i ";
					} else {
						$page_list .= ' <a href="'.replace_query_string_vars(Array('result_page' => $i)).'">'.$i.'</a> ';
					}
				}
				$replacements['page_list'] = $page_list;

			break;

		}//end switch

		return $todo;

	}//end getChunk()


	/**
	* Returns a specified chunk of elements
	*
	* will traverse a multidimensional array and return a chunk equal to the
	* size the user specifies, works for single and multi dimensional arrays
	*
	* @param array	&$todo				pointer to the array we want to traverse
	* @param array	&$chunk				pointer to array we want to fill
	* @param int	&$remaining_count	how many assets in we want our chunk including the chunk we want
	* @param mixed	$num_per_page		the number of assets we want returned
	* @param int	$group_limit		group limit
	*
	* @return void
	* @access private
	*/
	function _groupChunk(&$todo, &$chunk, &$remaining_count, $num_per_page, $group_limit=null)
	{

		//keep calling this for each group until we get to the actual assets
		if (($group_limit > 0) || is_null($group_limit)) {
			foreach ($todo as $index => $child) {
				if ($remaining_count >= 0) {
					$this->_groupChunk($todo[$index], $chunk[$index], $remaining_count, $num_per_page, ($group_limit == null ? null : $group_limit - 1));
				}

				if (empty($chunk[$index])) unset($chunk[$index]);
			}
			return;
		}

		$todo = $this->sortAssetList($todo, $this->getSortInfo());
		foreach ($todo as $index => $value) {
			if (($remaining_count > 0) && ($remaining_count <= $num_per_page)) {
				$chunk[$index] = $value;
			}

			$remaining_count--;
		}

	}//end _groupChunk()


	/**
	* Print the list of assets that we are listing
	*
	* @param array	$todo	an array of assets to list in the same format as the return value of getAssetList()
	*
	* @return void
	* @access public
	* @see getAssetList()
	* @see _printAssetList()
	*/
	function printAssetList($todo)
	{
		$this->_tmp['formats']        = Array();
		$this->_tmp['default_format'] = '';
		$this->_tmp['is_default']     = Array();
		$this->_tmp['needs_asset']    = Array();
		$this->_tmp['assets_info']    = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($todo));

		$type_folder = &$this->getFolder('type_formats');
		$this->_tmp['type_folder_id'] = $type_folder->id;

		$this->_tmp['position_formats'] = Array();
		$position_folder = &$this->getFolder('position_formats');
		$format_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($position_folder->id, SQ_LINK_TYPE_2, 'bodycopy', true);
		foreach ($format_links as $link_data) {
			$position = substr($link_data['value'], 9);
			$this->_tmp['position_formats'][$position] = $link_data['minorid'];
		}

		if ($this->attr('group_by') == 'grouped') {
			$this->sortGroups($this->attr('asset_grouping'), $todo, $todo);
			$this->_printGroups($todo, $this->attr('asset_grouping'));
		} else {
			$this->_printAssetList($todo);
		}
	}//end printAssetList()


	/**
	* Prints groups and their asset listings
	*
	* Treats each group as an asset listing to display, if we're not working
	* with groups then it will just skip and call _printAssetList as if
	* nothing happened
	*
	* @param array	&$todo		Our list of groups/assets to print
	* @param array	$group_info	Current group information we are working with
	*
	* @return void
	* @access private
	*/
	function _printGroups(&$todo, $group_info)
	{
		$pointer = reset($todo);
		if (!empty($group_info)) {
			//get the group bodycopy relative to this level
			$remaining_groups = array_shift($group_info);
			$format_bodycopy =& $GLOBALS['SQ_SYSTEM']->am->getAsset($remaining_groups['format_assetid']);

			foreach ($todo as $group_level => $value) {
					//lets buffer whatever contents we need
					ob_start();
						$this->_printGroups($todo[$group_level], $group_info);
						$group_contents = ob_get_contents();
					ob_end_clean();

					switch ($remaining_groups['group_type']) {

						case 'parent_asset':
							// we need to get info from the parent asset
							$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($group_level);
							$group_name = $asset->attr('name');
						break;

						case 'metadata':
							// the metadata field value will have been the index
							$group_name = $group_level;
						break;

						case 'attribute':
							// oh noes, implementation reserved!
						break;

						default:
							// there is no spoon .....
						break;
					}

					// populate replacements
					$replacements['group_listing'] = $group_contents;
					$replacements['group_name']    = $group_name;

					$format_bodycopy->setKeywordReplacements($replacements);
					$format_bodycopy->printBody();
			}//end foreach $todo

		} else {
			// otherwise we're up to the asset list, we can do the normal listing stuff here
			// make sure we add not replace the assets info array, just in case somethings needed
			$group_assets = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($todo));
			for (reset($group_assets); null !== ($index = key($group_assets)); next($group_assets)) {
				$this->_tmp['assets_info'][$index] = $group_assets[$index];
			}
			$this->_printAssetList($todo);
		}
	}//end _printGroups()


	/**
	* Called by printAssetList to take care of the layout of the asset list
	*
	* This method ensures the list is printed in columns if required, and keeps
	* track of what position in the list we are printing
	*
	* @param array	$todo	an array of assets to list in the same format as the return value of getAssetList()
	*
	* @return void
	* @access private
	* @see printAssetList()
	* @see _printAsset()
	*/
	function _printAssetList($todo)
	{

		$allowed_types = $this->getAllowedAssetTypeMap();
		$column_layout = $this->attr('column_layout');
		$groups = $this->attr('asset_grouping');
		if (empty($groups)) {
			$using_groups = false;
		} else {
			$using_groups = true;
		}
		$using_columns = (count($column_layout) > 1 || $column_layout['0'] > 1);

		$list_position = 1;

		if ($using_columns) {
			$num_rows = max(array_keys($column_layout)) + 1;
			$num_columns = max(array_values($column_layout));

			$current_row = 1;

			echo '<table width="100%">';

			for (reset($todo); null !== ($assetid = key($todo));) {

				echo '<tr>';
				$cols_in_row = array_get_index($column_layout, $current_row, $column_layout['0']);
				$each_colspan = (int)($num_columns / $cols_in_row);
				$first_colspan = $each_colspan + $num_columns - ($cols_in_row * $each_colspan);

				for ($c = 1; $c <= $cols_in_row; $c++) {
					$colspan = ($c == 1) ? $first_colspan : $each_colspan;
					?>
					<td<?php echo ($colspan > 1) ? ' colspan="'.$colspan.'"' : ''; ?>>
						<?php
						if ($assetid == null) {
							echo '&nbsp;';
						} else {
							$this->_printAsset($assetid, $todo[$assetid], $list_position);
							next($todo);
							$assetid = key($todo);
							$list_position++;
						}
						?>
					</td>
					<?php
				}
				echo '</tr>';
				$current_row++;
			}//end for

			echo '</table>';
		} else {
			foreach ($todo as $assetid => $type_code) {
				if ($using_groups) {
					$this->_printAsset($assetid, $type_code, array_get_index($type_code, 'position_number', 0));
				} else {
					$this->_printAsset($assetid, $type_code, $list_position);
				}
				$list_position++;
			}
		}

	}//end _printAssetList()


	/**
	* Print a single asset from the list
	*
	* @param string	$assetid		the ID of the asset we are printing
	* @param string	$type_code		the type code of the asset we are printing
	* @param int	$list_position	the position of this asset in the list
	*
	* @return void
	* @access private
	* @see printAssetList()
	*/
	function _printAsset($assetid, $type_code, $list_position)
	{
		$mm         = null;
		$contents   = '';
		$keywords   = Array();
		$customised = $this->getFormats('type_formats');

		$asset_info_fields = $this->getSortableAssetInfo();

		// if the type code is empty, we probably stuffed around with the todo list
		// but we can get the type code from the asset info array
		if ($type_code == '') {
			$info = array_get_index($this->_tmp['assets_info'], $assetid, Array('type_code' => ''));
			$type_code = $info['type_code'];
		}

		// check to see if there is a specific position format for us
		if (isset($this->_tmp['position_formats'][$list_position])) {
			$bodycopy = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_tmp['position_formats'][$list_position], 'bodycopy');
			if (!is_null($bodycopy)) {
				$contents = $bodycopy->getRawBodycopyContent();
				// see whether the keywords in it require asset loading or not
				$bc_keywords = retrieve_keywords_replacements($contents);
				$this_needs_asset = false;
				foreach ($bc_keywords as $keyword) {
					if (!(strpos($keyword, 'asset_') === 0) || ((strpos($keyword, 'asset_metadata_') !== 0) && (!in_array(substr($keyword, 6), $asset_info_fields)))) {
						$this_needs_asset = true;
						break;
					}
				}
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bodycopy);
			}
		}//end if has position format

		// if we can't find a position format, then we can search for type
		// and default formats
		if (empty($bodycopy)) {

			// our format will be based on type code
			if (in_array($type_code, $customised)) {
				$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->_tmp['type_folder_id'], SQ_LINK_TYPE_2, 'bodycopy', true, $type_code);

				if (!$link) {
					// use the default format
					$this->_tmp['is_default'][] = $type_code;
				} else {
					$bodycopy = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], 'bodycopy');
					if (!is_null($bodycopy)) {
						// get the format for this type
						$this->_tmp['formats'][$type_code] = $bodycopy->getRawBodycopyContent();
						// see whether the keywords in it require asset loading or not
						$bc_keywords = retrieve_keywords_replacements($this->_tmp['formats'][$type_code]);
						$this->_tmp['needs_asset'][$type_code] = false;
						foreach ($bc_keywords as $keyword) {
							if (!(strpos($keyword, 'asset_') === 0) || ((strpos($keyword, 'asset_metadata_') !== 0) && (!in_array(substr($keyword, 6), $asset_info_fields)))) {
								$this->_tmp['needs_asset'][$type_code] = true;
								break;
							}
						}
					}
				}
			} else {
				$this->_tmp['is_default'][] = $type_code;
			}

			if (in_array($type_code, $this->_tmp['is_default'])) {
				$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->_tmp['type_folder_id'], SQ_LINK_TYPE_2, 'bodycopy', true, 'default_format');
				if (empty($link)) {
					// TODO: localise
					trigger_error('Cannot Find the Default Bodycopy');
					return;
				} else {
					$bodycopy = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], 'bodycopy');
				}
				if (!is_null($bodycopy)) {
					// get the bodycopy content
					$this->_tmp['default_format'] = $bodycopy->getRawBodycopyContent();
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bodycopy);
					// see whether the keywords in it require asset loading or not
					$bc_keywords = retrieve_keywords_replacements($this->_tmp['default_format']);
					$this->_tmp['needs_asset']['default'] = false;
					foreach ($bc_keywords as $keyword) {
						if (!(strpos($keyword, 'asset_') === 0) || ((strpos($keyword, 'asset_metadata_') !== 0) && (!in_array(substr($keyword, 6), $asset_info_fields)))) {
							$this->_tmp['needs_asset']['default'] = true;
							break;
						}
					}
				}
				$this_needs_asset = $this->_tmp['needs_asset']['default'];
			} else {
				$this_needs_asset = $this->_tmp['needs_asset'][$type_code];
			}//end if using default format

		}//end if position bodycopy not found

		if ($this_needs_asset) {
			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			if (is_null($asset)) return;
			$asset_keywords = $asset->getAssetKeywords();
			foreach ($asset_keywords as $keyword_name => $keyword) {
				$keywords[$keyword_name] = $keyword['value'];
			}
			if ($assetid != $this->id) {
				if (preg_match('|%asset_contents%|is', $bodycopy->getRawBodycopyContent())) {
					ob_start();
						$asset->printBody();
						$keywords['asset_contents'] = ob_get_contents();
					ob_end_clean();
				}
			}

			// stuff carried over results body replacements

			$keywords['asset_name_linked'] = '<a href="'.$asset->getUrl().'">'.$asset->name.'</a>';
			$keywords['asset_short_name_linked'] = '<a href="'.$asset->getUrl().'">'.$asset->short_name.'</a>';

			if (in_array('asset_lineage', $bc_keywords) || in_array('asset_lineage_linked', $bc_keywords)) {
				// There is a lineage keyword in the bodycopy
				$lineage = '';
				$linked_lineage = '';
				$first = true;
				$found_root = false;

				//Prepare the url for getLineageFromUrl
				$nurl = $asset->getUrl();

				//No URL, have to use get Parents
				if (empty($nurl)) {

					$parent_asset_ids = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($asset->id));

					// searching for the root asset
					$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', false, 'root');
					$root_assetid = empty($root_link) ? 0 : $root_link['minorid'];
					foreach ($parent_asset_ids as $parent_asset_id) {
						if ($parent_asset_id == $root_assetid) {
							$found_root = true;
						}
						if (!$found_root) continue;
						if (!$first) {
							$lineage .= $this->attr('lineage_seperator');
							$linked_lineage .= $this->attr('lineage_seperator');
						}
						$first = false;
						$ancestor_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($parent_asset_id);
						$linked_lineage .= '<a href="'.$ancestor_asset->getUrl().'">'.$ancestor_asset->name.'</a>';
						$lineage .= $ancestor_asset->short_name;
						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($ancestor_asset);
					}
				} else {
					$nurl = strip_url($nurl);
					$nurl = stristr($nurl,'/');
					$nurl = substr($nurl,2);

					$parent_assets = $GLOBALS['SQ_SYSTEM']->am->getLineageFromUrl(null, $nurl);

					// searching for the root asset
					$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', false, 'root');
					$root_assetid = empty($root_link) ? 0 : $root_link['minorid'];
					$show_root = false;
					$started = false;
					foreach ($parent_assets as $data) {
						if ($data['assetid'] == $root_assetid) {
							$found_root = true;
						}
						if (!$found_root) continue;
						if ($first && !$show_root) {
							$first = false;
							continue;
						}

						// If this is the asset itself, don't put it in the lineage.
						if ($data['assetid'] == $assetid) continue;
						if (!$first && $lineage != '') {
							$lineage .= $this->attr('lineage_seperator');
							$linked_lineage .= $this->attr('lineage_seperator');
						}
						$linked_lineage .= '<a href="'.$data['protocol'].'://'.$data['url'].'">'.$data['short_name'].'</a>';
						$lineage .= $data['short_name'];
						$first = false;
					}

					// We didn't find the search_root in the lineage, it's probably a file
					// It is valid though, so lets print it, as it's better than the
					// warning messages it used to spit out
					if (!$found_root) {
						$first = true;
						foreach ($parent_assets as $data) {
							if (!$first && $lineage != '') {
								$lineage .= $this->attr('lineage_seperator');
								$linked_lineage .= $this->attr('lineage_seperator');
							}
							$lineage .= $data['short_name'];
							$linked_lineage .= '<a href="'.$data['protocol'].'://'.$data['url'].'">'.$data['short_name'].'</a>';
							$first = false;
						}
					}
				}
				$keywords['asset_lineage'] = $lineage;
				$keywords['asset_lineage_linked'] = $linked_lineage;
			} else {
				$keywords['asset_lineage'] = '';
				$keywords['asset_lineage_linked'] = '';
			}
			// end results body stuff

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			unset($asset);
		} else {
			$keywords = Array();
			foreach ($this->_tmp['assets_info'][$assetid] as $field => $value) {
				$keywords['asset_'.$field] = $value;
			}
		}//end if needs asset

		$metadata_keywords = Array();
		foreach ($bc_keywords as $keyword) {
			if (substr($keyword, 0, 15) == 'asset_metadata_') {
				$metadata_keywords[] = substr($keyword, 15);
			}
		}
		if (!empty($metadata_keywords)) {
			if (is_null($mm)) {
				$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
			}
			// get all the metadata keywords for this asset
			$metadata_values = $mm->getMetadataFieldValues($assetid, $metadata_keywords);

			foreach ($metadata_values as $field => $value) {
				$keywords['asset_metadata_'.$field] = $value;
			}
		}

		$keywords['page_href']      = $this->getHref();
		$keywords['asset_position'] = (array_get_index($_REQUEST, 'result_page', 1) - 1) * $this->attr('num_per_page') + $list_position;

		// print asset ID always
		$keywords['asset_assetid'] = $assetid;

		// kept for backwards compatibility
		$keywords['result_no']      = &$keywords['asset_position'];
		$keywords['root_nodes'] = implode(',', $this->getRootNodes());

		$keywords['result_number'] = '%result_number%';
		$keywords['score'] = '%score%';

		$bodycopy->setKeywordReplacements($keywords);
		$bodycopy->printBody();

	}//end _printAsset()


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
		$vars['keywords'] = isset($vars['keywords'])?$vars['keywords']:Array();

		$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id, 'bodycopy', true);
		$parent_folders = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id, 'folder', true);

		// if we're being handled here (instead of one of our extensions),
		// it must be from one of our format bodycopies, or something else altogether,
		// so we'll try to find out which of the format types it is.
		$group_folder = &$this->getFolder('group_formats');
		$position_folder = &$this->getFolder('position_formats');
		$type_folder = &$this->getFolder('type_formats');

		$keywords = Array();
		if (in_array($group_folder->id, array_keys($parent_folders))) {
			// group format
			$keywords = Array(
							'group_listing'	=> 'Group Asset Listing',
							'group_name'	=> 'Group Name',
						);

		} else if (in_array($position_folder->id, array_keys($parent_folders))) {
			// position format
			$keywords['asset_contents'] = translate('cms_listing_asset_contents');
			$keywords['asset_name_linked'] = translate('cms_listing_asset_name_linked');
			$keywords['asset_short_name_linked'] = translate('cms_listing_asset_short_name_linked');
			$keywords['asset_position'] = translate('cms_listing_asset_position');
			$keywords['root_nodes'] = translate('cms_listing_keyword_root_nodes');

			foreach (Asset::getAssetKeywords(false) as $keyword => $info) {
				$name = ucwords(str_replace('_', ' ', $keyword));
				$keywords[$keyword] = $name;
			}

		} else if (in_array($type_folder->id, array_keys($parent_folders))) {
			// specific or default type format
			$keywords['asset_contents'] = translate('cms_listing_asset_contents');
			$keywords['asset_name_linked'] = translate('cms_listing_asset_name_linked');
			$keywords['asset_short_name_linked'] = translate('cms_listing_asset_short_name_linked');
			$keywords['asset_position'] = translate('cms_listing_asset_position');
			$keywords['root_nodes'] = translate('cms_listing_keyword_root_nodes');

			$type_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($type_folder->id, SQ_LINK_TYPE_2, 'bodycopy');
			$type_codes = Array();
			foreach ($type_links as $link_info) {
				if (isset($parents[$link_info['minorid']])) {
					$type_codes[] = $link_info['value'];
				}
			}

			if ((count($type_codes) == 1) && ($type_codes[0] != 'default_format')) {
				$type_code = $type_codes[0];
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
				$dummy_asset =& new $type_code();
				$type_keywords = $dummy_asset->getAssetKeywords(false);
				unset($dummy_asset);
			} else {
				$type_keywords = Asset::getAssetKeywords(false);
			}

			foreach ($type_keywords as $keyword => $info) {
				$name = ucwords(str_replace('_', ' ', $keyword));
				$keywords[$keyword] = $name;
			}

		} else {
			// page contents format
			$keywords = $this->getContentsKeywords();
		}

		$vars['keywords'] = array_merge($vars['keywords'], $keywords);

	}//end onRequestKeywords()


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
	function getContentsKeywords()
	{
		return $this->contents_keywords;

	}//end getContentsKeywords()


	/**
	* Gets the list of available asset info fields that can be used
	*
	* @return array
	* @access public
	*/
	function getSortableAssetInfo()
	{
		return Array(
				'assetid','type_code','version','name','short_name',
				'status','languages','charset','force_secure','created',
				'created_userid','updated','updated_userid','published',
				'published_userid',
			   );

	}//end getSortableAssetInfo()


	/**
	* Gets the list of current root nodes
	*
	* @return array
	* @access public
	*/
	function getRootNodes()
	{
		if (!isset($this->_tmp['root_asset_ids'])) {
			$root_asset_ids = Array();
			$root_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE, '', false, 'major', 'root');
			foreach ($root_links as $root_link) {
				$root_asset_ids[] = (int)$root_link['minorid'];
			}

			$parameter_map = &$this->getAttribute('parameter_map');
			$raw_dynamic_root_nodes = $parameter_map->getParameterValue('root_node');

			// check if we have a dynamic parameter -- if so, let's do some security checks
			if (!empty($raw_dynamic_root_nodes)) {
				$dynamic_root_nodes = Array();
				// note that dynamic roots can be supplied as an array of asset ids or
				// as a comma delimited string of asset ids
				if (!is_array($raw_dynamic_root_nodes)) {
					$raw_dynamic_root_nodes = explode(',', $raw_dynamic_root_nodes);
				}

				foreach ($raw_dynamic_root_nodes as $dynamic_root) {
					if (!strlen($dynamic_root)) continue;

					$dynamic_parents = $GLOBALS['SQ_SYSTEM']->am->getParents($dynamic_root);

					// first, check to see if the dynamic parameter specified is already one of the root nodes
					// if so, just use that single root node
					if (in_array($dynamic_root, $root_asset_ids)) {
						$dynamic_root_nodes[] = $dynamic_root;
					} else {
						// otherwise, check to see if the dynamic parameter specified is a child of one of the root
						// nodes. If so, use the dynamic parameter as the root node; otherwise return an error.
						$matching_root_ids = array_intersect(array_keys($dynamic_parents), $root_asset_ids);
						if (empty($matching_root_ids)) {
							trigger_localised_error('CMS0020', E_USER_WARNING, $dynamic_root);
						} else {
							$dynamic_root_nodes[] = $dynamic_root;
						}
					}
				}

				if (!empty($dynamic_root_nodes)) {
					$root_asset_ids = $dynamic_root_nodes;
				}
			}
			$this->_tmp['root_asset_ids'] = $root_asset_ids;
		}

		return $this->_tmp['root_asset_ids'];

	}//end getRootNodes()


	// Keyword Replacement Handlers


	/**
	* Get the replacement for "sort_by" keyword
	*
	* Produces a dropdown list of all the sorting options registered for this asset
	*
	* @return string
	* @access public
	*/
	function getSortByKeywordReplacement()
	{
		$prefix = $this->getPrefix();
		$field_name = $prefix.'_sort_by';
		$this->registerFormField($field_name);

		$default_sort_by = $this->attr('default_sort_by');
		$sort_by = array_get_index($_REQUEST, $field_name, $default_sort_by);

		$sort_by_list = $this->attr('sort_by');
		if (!isset($sort_by_list[$sort_by])) {
			$sort_by = $default_sort_by;
		}

		$options = Array();
		foreach ($sort_by_list as $key => $value) {
			$options[$key] = $value['name'];
		}

		ob_start();
			combo_box($field_name, $options, false, $sort_by);
		return ob_get_clean();

	}//end getSortByKeywordReplacement()


	/**
	* Get the replacement for "sort_order" keyword
	*
	* @return string
	* @access public
	*/
	function getSortOrderKeywordReplacement()
	{
		$prefix = $this->getPrefix();
		$field_name = $prefix.'_sort_direction';
		$this->registerFormField($field_name);

		$options = Array(
					'0'	=> $this->attr('sort_direction_asc_text'),
					'1'	=> $this->attr('sort_direction_desc_text'),
				   );

		ob_start();
			combo_box($field_name, $options, false, (int)$this->isDescending());
		return ob_get_clean();

	}//end getSortOrderKeywordReplacement()


	/**
	* Get the replacement for "submit_button" keyword
	*
	* @return string
	* @access public
	*/
	function getSubmitButtonKeywordReplacement()
	{
		$prefix = $this->getPrefix();
		$field_name = $prefix.'_submit_button';
		$this->registerFormField($field_name);

		$button_text = $this->attr('submit_button_text');

		ob_start();
			submit_button($field_name, $button_text);
		return ob_get_clean();

	}//end getSubmitButtonKeywordReplacement()


	/**
	* Get the replacement for "root_nodes" keyword
	*
	* @return string
	* @access public
	*/
	function getRootNodesKeywordReplacement()
	{
		$root_nodes = $this->getRootNodes();
		return implode(',', $root_nodes);

	}//end getRootNodesKeywordReplacement()


	/**
	* Determine if descending sort option was selected
	*
	* Looks at request data before consulting saved attributes
	*
	* @return boolean
	* @access public
	*/
	function isDescending()
	{
		$prefix = $this->getPrefix();
		$field_name = $prefix.'_sort_direction';

		$sort_order = array_get_index($_REQUEST, $field_name);
		switch ($sort_order) {
			case '1':
				$result = true;
			break;

			case '0':
				$result = false;
			break;
			default:
				$result = $this->attr('reverse_sort');
		}

		return $result;

	}//end isDescending()


	/**
	* Register a form field name
	*
	* This is useful if we want to determine whether a field has been used in the form
	* This functionality is required for
	*
	* @param string	$field_name	Name of the form field
	*
	* @return void
	* @access public
	*/
	function registerFormField($field_name)
	{
		if (!empty($field_name)) {
			$this->_tmp['used_form_fields'][] = $field_name;
		}

	}//end registerFormField()


	/**
	* Return an array of form fields registered for this asset
	*
	* Registered fields are those that have already been used
	* i.e. as a result of a Keyword replacement request
	*
	* @return array
	* @access public
	*/
	function getUsedFormFields()
	{
		return array_get_index($this->_tmp, 'used_form_fields', Array());

	}//end getUsedFormFields()


	/**
	* Gets current sort option
	*
	* Looks at the state to determine which sort option is currently selected
	* Returns one entry from  the 'sort_by' attribute or an empty Array.
	* -
	* -
	* Array(
	*	name => String
	*	type => String
	*	params => Array()
	*  )
	* -
	* params array changes depending on the type value
	* valid types are: field, asset_attrib
	* -
	* if type = "field", params is
	* Array(
	*	field => 'assetid'
	* )
	* where field contains has a name of db column in the asset table to sort by
	* -
	* if type is "asset_attrib" params is
	* Array(
	*	type_code => 'page'
	*	attribute => 'short_name'
	* )
	* where type_code is the type code of the asset whose attribute is being used
	* and 'attribute' is the name of that attribute (as defined by the management class)
	*
	* @return array
	* @access public
	*/
	function getSortInfo()
	{
		$prefix = $this->getPrefix();
		$field_name = $prefix.'_sort_by';

		$default_sort_by = $this->attr('default_sort_by');
		$sort_by = array_get_index($_REQUEST, $field_name, $default_sort_by);

		$sort_by_list = $this->attr('sort_by');
		if (!isset($sort_by_list[$sort_by])) {
			$sort_by = $default_sort_by;
		}

		return array_get_index($sort_by_list, $sort_by, Array());

	}//end getSortInfo()


	/**
	* Recursive function to build multi level grouping
	*
	* (actually this is now non-recursive, function name will have to be fixed!)
	* Will construct an array in the following form:
	* if it is a  group its index will be group name (assetid or string) and its
	* contents an array if its an asset in a group its index will be an assetid
	* and its contents will be a typecode
	* The last paramater &$results will hold the contents at the end of the
	* recursive calling instead of working off a return basis
	*
	* @param array	$group_levels	remaining group levels to iterate
	* @param array	$assetids		asset ids that we are working with in this iteration
	* @param array	&$results		reference to the array/section of array we are working on
	*
	* @return void
	* @access public
	*/
	function groupAssetsRecursively($group_levels, $assetids, &$results)
	{
		//we need assets and groups, so if none given, let's bail
		if (empty($assetids) || empty($group_levels)) {
			return;
		}

		// we need to reverse this because we want to do the inner level first
		$group_levels = array_reverse($group_levels);

		$asset_groups = Array();
		foreach ($assetids as $assetid => $group_data) {
			$asset_groups[] = Array('assetid' => $assetid);
		}

		// keep track of the last time we encountered a parent asset group
		$last_parent_asset = false;
		foreach ($group_levels as $group_key => $group_level) {
			$new_asset_groups = Array();

			foreach ($asset_groups as $key => $group_data) {
				$assetid = $group_data['assetid'];

				if ($last_parent_asset === false) {
					$current_parent = $assetid;
				} else {
					$current_parent = $group_data['group'][$last_parent_asset];
				}

				switch ($group_level['group_type']) {
					case 'parent_asset':
						$value = $this->getGroupableParentAssetids($current_parent, $group_level['restrict_types']);
					break;

					case 'metadata':
						$metadata_field_id = $group_level['metadata_field'];
						$metadata_field =& $GLOBALS['SQ_SYSTEM']->am->getAsset($metadata_field_id);
						$value = '';
						if (!is_null($metadata_field)) {
							// get the metadata manager if we haven't already
							if (!isset($mm)) {
								$mm =& $GLOBALS['SQ_SYSTEM']->getMetadataManager();
							}

							$parent_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($current_parent);
							if (!is_null($parent_asset)) {
								$value = $mm->getMetadataValueByAssetid($current_parent, $metadata_field_id, true);
								$value = $metadata_field->getMetadataValue($parent_asset, $value);
								$GLOBALS['SQ_SYSTEM']->am->forgetAsset($current_parent->id);
							}
						}
					break;

					case 'attribute':
						// implementation reserved
					break;
				}

				if (!is_array($value)) {
					$group_data['group'][$group_key] = $value;
					$new_asset_groups[] = $group_data;
				} else {
					foreach ($value as $sub_value) {
						$group_data['group'][$group_key] = $sub_value;
						$new_asset_groups[] = $group_data;
					}
				}
			}//end foreach on asset groups
			$asset_groups = $new_asset_groups;

			if ($group_level['group_type'] == 'parent_asset') {
				$last_parent_asset = $group_key;
			}

		}//end foreach on group level

		$results = Array();
		$group_keys = array_keys($group_levels);

		// we need to flip the keys back to what they were to construct the array
		foreach ($asset_groups as $group_data) {
			$base =& $results;
			foreach (array_reverse($group_keys) as $group_key) {
				$key = $group_data['group'][$group_key];
				if (is_null($key)) $key = '';
				$base =& $base[$key];
			}
			$base[$group_data['assetid']] = $assetids[$group_data['assetid']];
		}

	}//end groupAssetsRecursively()


	/**
	* Recursive function to sort the group keys
	*
	* Returns void but returns a modified array (in sort order only, contents
	* themselves are unaffected) in &$results
	*
	* @param array	$group_levels	remaining group levels to iterate
	* @param array	$assetids		asset ids that we are working with in this
	*								iteration
	* @param array	&$results		reference to the array/section of array we are
	*								working on
	*
	* @return void
	* @access public
	*/
	function sortGroups($group_levels, $assetids, &$results)
	{
		//we need assets, so if none given, let's bail
		if (empty($assetids)) return;

		//point us at the first element
		$group_type = reset($group_levels);

		// sort the groups
		switch ($group_type['group_type']) {
			case 'parent_asset':
				// find the info on the assets that are being used, sort by them,
				// then re-order the results based upon them
				$group_order = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($results), 'asset', false, array_get_index($group_type, 'sort_by', 'assetid'));

				if (array_get_index($group_type, 'sort_order', 'asc') == 'asc') {
					asort($group_order);
				} else {
					arsort($group_order);
				}

				$group_results = Array();
				foreach (array_keys($group_order) as $assetid) {
					$group_results[$assetid] =& $results[$assetid];
				}
				$results = $group_results;

			break;

			case 'metadata':
				// metadata groups are sorted just on the key's value, because
				// the key IS the value
				if (array_get_index($group_type, 'sort_order', 'asc') == 'asc') {
					ksort($results);
				} else {
					krsort($results);
				}
			break;

			case 'attribute':
				// implementation reserved
			break;
		}

		//we're done with this group level, so lets discard it
		array_shift($group_levels);

		//if their are group levels remaining, we want to call
		//this function again for each of the groups we have made
		if (count($group_levels) > 0) {
			foreach ($results as $index => $group) {
				$this->sortGroups($group_levels, $group, $results[$index]);
			}
		}
	}//end sortGroups()


	/**
	* Get all the parents of this asset which can be used for grouping. This
	* function needs to be overridden in assets like Search_Page where the use
	* of proxy search types require alternate logic to just getting ALL parents.
	*
	* @param array	$child_assetid	the asset to find the parents of
	*
	* @return array
	* @access public
	*/
	function getGroupableParentAssetids($child_assetid, $restrict_types=Array())
	{
		$root_nodes = $this->getRootNodes();

		$parent_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($child_assetid, SQ_SC_LINK_BACKEND_NAV, '', true, 'minor');

		$parent_link_assetids = Array();
		foreach ($parent_links as $parent_link) {
			if (empty($root_nodes)) {
				// if no root nodes set, allow all parents
				$parent_link_assetids[] = $parent_link['majorid'];
			} else {
				$parent_assetid = $parent_link['majorid'];

				if (in_array($parent_assetid, $root_nodes)) {
					// check if parent is a root node itself
					$parent_link_assetids[] = $parent_assetid;
				} else {
					// check if the parent is within the root node
					$parents_of_parent = $GLOBALS['SQ_SYSTEM']->am->getParents($parent_assetid);

					if (!empty($restrict_types['type_code'])) {
						// find the first parent that is one of the restricted types
						$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($child_assetid);
						foreach ($parents as $parentid => $type_code) {
							$inherit_values = $restrict_types['inherit'];
							foreach ($restrict_types['type_code'] as $restricted_type_code) {
								$inherit = array_shift($inherit_values);
								if ($type_code == $restricted_type_code || ($inherit && $GLOBALS['SQ_SYSTEM']->am->isTypeDecendant($type_code, $restricted_type_code))) {
									$parent_assetid = $parentid;
								}
							}
						}
					}

					if (count(array_intersect(array_keys($parents_of_parent), $root_nodes)) > 0) {
						$parent_link_assetids[] = $parent_assetid;
					}
				}
			}//end if empty root nodes
		}//end foreach

		return $parent_link_assetids;

	}//end getGroupableParentAssetids()


	/**
	* Reindexes the group format bodycopies so any gaps are closed and bodycopy
	* names are referring to their correct group numbers
	*
	* @return boolean
	* @access public
	*/
	function reindexGroupFormats()
	{
		// if we have no group formats, it's not worth our timee
		$group_folder =& $this->getFolder('group_formats');
		$groups = $this->attr('asset_grouping');
		if (empty($groups)) return true;


		// set forced run level because we want to change the names of these
		// formats whether we have permission or not
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

		$i = 1;
		foreach ($groups as $group) {
			// update the group format
			$format_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($group['format_assetid']);
			if ($format_asset) {
				$format_asset->setAttrValue('name', 'Group Level '.$i.' Format');
				$format_asset->saveAttributes();
			}

			$i++;
		}

		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end reindexGroupFormats()


	/**
	* Recursively count array elements
	*
	* @param array	&$array	Array to count
	* @param string	$limit	Number of levels to traverse
	*
	* @return int
	* @access private
	*/
	function _arrayCountRecursive(&$array, $limit=null)
	{
		$count = 0;

		for (reset($array); null !== ($k = key($array)); next($array)) {
			$el =& $array[$k];
			if (is_array($el) && (is_null($limit) || ($limit > 0))) {
				$count += $this->_arrayCountRecursive($el, (is_null($limit) ? null : $limit - 1));
			} else {
				$count++;
			}
		}

		return $count;

	}//end _arrayCountRecursive()


	/**
	* Return a list of position formats
	*
	* @return array
	* @access private
	*/
	function _getPositionFormats()
	{
		if (!isset($this->_tmp['position_formats'])) {
			$this->_tmp['position_formats'] = Array();

			$position_folder = &$this->getFolder('position_formats');
			$format_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($position_folder->id, SQ_LINK_TYPE_2, 'bodycopy', true);
			foreach ($format_links as $link_data) {
				$position = substr($link_data['value'], 9);
				$this->_tmp['position_formats'][$position] = $link_data['minorid'];
			}
		}

		return $this->_tmp['position_formats'];

	}//end _getPositionFormats()


}//end class
?>
