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
* $Id: listing_engine.inc,v 1.129 2005/12/19 04:06:17 lwright Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once SQ_FUDGE_PATH.'/general/www.inc';

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
* @author Luke Wright   <lwright@squiz.net>
* @author David Heppell <dheppell@squiz.net>
* @author Marc McIntyre <mmcintyre@squiz.net>
* @author Greg Sherwood <greg@squiz.net>
* @author Andrei Railean <arailean@squiz.net>
* @author Robert Howard <rhoward@squiz.net>
*
* @version $Revision: 1.129 $
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


	function Listing_Engine($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Page($assetid);

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

			if ($name == 'type_formats') $type_formats = $asset;

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			unset($asset);
		}
		$asset =& new Bodycopy();
		$copy_link = Array('asset' => &$type_formats, 'value' => 'default_format' ,'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);
		$asset->setAttrValue('name', translate('default_format'));
		if (!$asset->create($copy_link)) return false;

		return $this->_createBodycopies();

	}//end _createAdditional()


	/**
	* Creates bodycopies for this asset
	*
	* Override if different bodycopies are needed (eg. in search page).
	*
	* @return boolean
	* @access protected
	*/
	function _createBodycopies()
	{
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$asset =& new Bodycopy();
		$copy_link = Array('asset' => &$this, 'value' => 'page_contents' ,'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);
		$asset->setAttrValue('name', 'Page Contents');
		if (!$asset->create($copy_link)) return false;

		if (!$this->createNoResultsBodycopy(true)) {
			return false;
		}

		return true;

	}//end _createBodycopies()


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

		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'page_contents');
		if ($bodycopy_link['linkid'] == $linkid && !$GLOBALS['SQ_PURGING_TRASH']) {
			return translate('cms_cannot_delete_link', $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
		}

		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'no_results');
		if ($bodycopy_link['linkid'] == $linkid && !$GLOBALS['SQ_PURGING_TRASH']) {
			return translate('cms_cannot_delete_link', $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
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
		$page_links[SQ_LINK_TYPE_2]['bodycopy'] = Array('card' => 2,   'exclusive' => false);
		$page_links[SQ_LINK_TYPE_3]['bodycopy'] = Array('card' => 1,   'exclusive' => false);
		//$page_links[SQ_LINK_TYPE_2]['bodycopy'] = Array('card' => 2,   'exclusive' => true);
		//$page_links[SQ_LINK_TYPE_3]['bodycopy'] = Array('card' => 1,   'exclusive' => true);
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

		$selections = $this->attr('asset_selections');

		if (!empty($used_fields) || $this->requires_form) {

			// include the 'select all' js if required
			$using_select_all = false;
			foreach ($selections as $selection_name => $selection_settings) {
				if (array_get_index($selection_settings, 'type', null) == 'radio' && !empty($selection_settings['options']['allow_multiple'])) {
					?>
					<script type="text/javascript" src="<?php echo sq_web_path('data') ?>/asset_types/listing_engine/js/select_all.js"></script>
					<?php
					$using_select_all = true;
					break;
				}
			}

			// we're not using stuff that requires the select all code
			if (!$using_select_all) {
				$contents = str_replace($this->getSelectAllJSCodeKeywordReplacement(), '', $contents);
			}

			// if we are posting, we really shouldn't be having anything in the GET
			if ($this->_getFormSubmitMethod() == 'post') {
				$form_url = htmlspecialchars($this->getURL());
			} else {
				$form_url = htmlspecialchars(replace_query_string_vars($keys_to_remove, $this->getURL()));
			}

			?>
			<form id="<?php echo $this->getPrefix() ?>" method="<?php echo $this->_getFormSubmitMethod() ?>" action="<?php echo $form_url; ?>">
			<?php echo $contents; ?>
			</form>
			<?php
		} else {
			// no form at all - remove the JS code relations stuff
			$contents = str_replace($this->getSelectAllJSCodeKeywordReplacement(), '', $contents);
			echo $contents;
		}

	}//end printBody()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printContents()
	{
		// we need a unique cache key to represent the current result page
		$cache_key = $this->_getCacheKey();

		$cached_contents = '';
		if (!empty($cache_key)) {
			$cm = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
			$cached_contents = $cm->loadFromCache($this->id, $this->type(), $cache_key);
		}

		// if the cache is empty, go ahead and regenerate a new version
		if (empty($cached_contents)) {

			$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'page_contents');
			if (empty($bodycopy_link)) return false;

			$format_bodycopy = &$GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link['minorid'], $bodycopy_link['minor_type_code']);
			if (is_null($format_bodycopy)) return false;

			require_once SQ_FUDGE_PATH.'/general/text.inc';
			$keywords = $format_bodycopy->getKeywords();
			$replacements = Array();

			$children = $this->getAssetList();
			$this->filterAssetList($children);
			$num_kids = count($children);

			// if there are no children available, just print the page bodycopy -
			// or the NO RESULTS bodycopy if it exists
			if (empty($children)) {
				$no_results_bodycopy = $this->getNoResultsBodycopy();

				$replacements['asset_count']			= 0;
				$replacements['asset_listing']			= '';
				$replacements['page_list']				= '';
				$replacements['page_number']			= 0;
				$replacements['page_asset_count']		= 0;
				$replacements['total_pages']			= 0;
				$replacements['first_asset_position']	= 0;
				$replacements['last_asset_position']	= 0;
				$replacements['previous_page']			= '';
				$replacements['next_page']				= '';

				if (!is_null($no_results_bodycopy)) {
					$no_results_bodycopy->setKeywordReplacements($replacements);
					$no_results_bodycopy->printBody();
				} else {
					$selections = $this->attr('asset_selections');
					foreach ($selections as $selection_name => $settings) {
						$replacements[$selection_name.'_check_all'] = '';
						$replacements[$selection_name.'_check_all_in_group'] = '';
					}

					$format_bodycopy->setKeywordReplacements($replacements);
					$format_bodycopy->printBody();
				}
				return;
			}

			if (!in_array('asset_listing', $keywords)) {
				// there is no asset listing keyword in the contents, so we dont
				// need to do all the extra processing for the listing
				$format_bodycopy->printBody();
				return;
			}

			// the list is sorted and positions set, now we can split to groups
			if ($this->attr('group_by') == 'grouped') {
				$this->groupAssetsRecursively($this->attr('asset_grouping'), $children, $children);
				$this->sortGroups($this->attr('asset_grouping'), $children, $children);
			}

			foreach ($keywords as $word) {
				$replacements[$word] = $this->getKeywordReplacement($word);
			}

			$num_per_page = $this->attr('num_per_page');
			$result_page = (isset($_REQUEST['result_page'])) ? (int)$_REQUEST['result_page'] : 1;
			if ($result_page <= 0) $result_page = 1;

			// get our page chunk here
			$todo =& $this->getChunk($children, $replacements, $result_page, $num_per_page);

			// get any extra keyword replacements from this asset or asset_listing
			$replacements = $this->getContentsKeywordReplacements($keywords) + $replacements;
			$num_assets_showing = count($todo);
			ob_start();
				$this->printAssetList($todo);
				$global_contents = ob_get_contents();
			ob_end_clean();

			// the global replacement to print the asset listing
			$replacements['asset_listing'] = $global_contents;
			if ($this->attr('group_by') != 'grouped') {
				$replacements['asset_count'] = $num_kids;
			} else {
				$total_assets = $this->_arrayCountRecursive($children, count($this->attr('asset_grouping')));
				$replacements['asset_count'] = $total_assets;
				$replacements['unique_asset_count'] = $num_kids;
			}

			$result_page = $replacements['page_number'];
			$replacements['page_asset_count'] = $num_assets_showing;
			// compatibility

			if ($replacements['asset_count'] == 0) {
				$replacements['first_asset_position'] = 0;
			} else if ($num_per_page == 0) {
				$replacements['first_asset_position'] = 1;
			} else {
				$replacements['first_asset_position'] = min(1, $num_per_page * ($result_page - 1) + 1);
			}

			if ($num_per_page == 0) {
				$replacements['last_asset_position'] = $replacements['asset_count'];
			} else {
				$replacements['last_asset_position'] = min($replacements['asset_count'], $num_per_page * $result_page);
			}

			$replacements['select_all_js_code'] = $this->getSelectAllJSCodeKeywordReplacement();

			// print the contents of page - replacing the global keywords
			$format_bodycopy->setKeywordReplacements($replacements);
			ob_start();
				if (!in_array('select_all_js_code', $keywords)) {
					// there is no asset listing keyword in the contents, so we dont
					// need to do all the extra processing for the listing
					echo $this->getSelectAllJSCodeKeywordReplacement();
				}
				$format_bodycopy->printBody();
				if (!empty($cache_key)) {
					$cm->saveToCache($this->id, $this->type(), $cache_key, ob_get_contents());
				}
			ob_end_flush();

			// clean up after ourselves
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($format_bodycopy);

		} else {

			// the contents have already been cached, so just output them
			echo $cached_contents;

		}//end if no cache

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

		// if we don't have a cache key from the listing it's because we shouldn't be caching
		if ($cache_key) {
			// because the root nodes can be dynamic we need to cache the results based upon root node
			$root_nodes = $this->getRootNodes();
			$cache_key .= '-'.implode('-', $root_nodes);
		}

		return $cache_key;

	}//end _getCacheKey()


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
		if (count($asset_list) == 1 || empty($sort_info)) {
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
		$is_sort_by_metadata     = ($sort_info['type'] == 'metadata');

		$sort_params =& $sort_info['params'];

		$assetids = array_keys($asset_list);

		if ($is_sort_by_attribute) {
			$sort_by_attr = $sort_params['attribute'];
			$sort_by_type_code = $sort_params['type_code'];

			foreach ($this->attr('types') as $type_code => $inherit) {
				// initially we plan not to process this type code
				$process = false;

				if ($type_code == $sort_by_type_code) {
					$process = true;

				} else {
					if ($inherit) {
						// we have to find our closest ancestor with a sort field
						$type_anc = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($type_code);
						array_unshift($type_anc, $type_code);

						foreach ($type_anc as $this_type_code) {
							if ($this_type_code == $sort_by_type_code) {
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
		} else if ($is_sort_by_metadata) {
			// Sort by Metadata field
			$sort_numeric = false;
			$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
			$fieldid = $sort_info['field'];

			if (isset($sort_info['field'])) {
				$fieldid = $sort_info['field'];
			}

			if ($fieldid) {
				$field_default_val = $mm->getMetadataFieldDefaultValue($fieldid);
			} else {
				$field_default_val = '';
			}

			foreach ($asset_list as $assetid => $type) {
				$meta_field_val = $mm->getMetadataValueByAssetid($assetid,$fieldid,true);
				$field_value = $field_default_val;

				if (isset($meta_field_val)) {
					$field_value = $meta_field_val;

					// get keyword replacements
					if (preg_match('/^%(.*)%$/',$field_value,$matches)) {
						$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
						if ($asset->id) {
							$field_value = $asset->getKeywordReplacement($matches[1]);
						}
						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
					}
				}
				$sort_assets[$assetid] = strtolower($field_value);
			}

		} else {
			$sort_field = $sort_params['field'];
			//no sorting is required. So return the assetlist
			if ($sort_field == 'no_sorting') return $asset_list;

			if (in_array($sort_field, $this->getSortableAssetInfo())) {
				$sort_assets = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($assetids, null, true, $sort_field);
				for (reset($sort_assets); null !== ($assetid = key($sort_assets)); next($sort_assets)) {
					$sort_assets[$assetid] = strtolower($sort_assets[$assetid]);
				}

				if ($sort_field != 'assetid') $sort_numeric = false;
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
	* Remove assets with a status that we do not want to show
	*
	* For assets with a status less than LIVE, the asset is loaded and read access is checked.
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
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$todo_keys = array_keys($todo);
		for (reset($todo_keys); null !== ($key = key($todo_keys)); next($todo_keys)) {
			$todo_keys[$key] = $db->quoteSmart((string) $todo_keys[$key]);
		}

		$wanted_statuses = $this->attr('statuses');

		// push all the shoadow assets into the respective array
		// change the todo_keys array so that instead of shadow assets the query checks on the parent asset's status
		$shadow_assets = Array();
		$dummy_to_do = array_keys($todo);
		foreach ($todo as $key => $value) {
			if (strpos($key, ':') !== false) {
				$shadow_assets[$key] = $value;
				foreach ($dummy_to_do as $index => $key_new) {
					if ($key_new == $key) {
						$key_new = $db->quoteSmart(substr($key, 0, strpos($key, ':')));
						$todo_keys[$index] = $key_new;
					}
				}
			}
		}

		if (!empty($wanted_statuses) && !empty($todo_keys)) {
			$sql = 'SELECT
						assetid, type_code
					FROM
						sq_ast
					WHERE
							assetid IN ('.implode(', ', $todo_keys).')
						AND status IN ('.implode(', ', $wanted_statuses).')';

			$wanted_assets = $db->getAssoc($sql);
			assert_valid_db_result($wanted_assets);

			// now checking if the parent is existing in the wanted assets array
			// if yes then the shadow assets are added to the wanted assets array as well
			foreach ($wanted_assets as $wanted => $type_code) {
				foreach ($shadow_assets as $key => $value) {
					$shadow_array = split(':', $key);
					if ($shadow_array[0] == $wanted) {
						$wanted_assets[$key] = $type_code;
					}
				}
			}

			foreach ($todo as $assetid => $v) {
				if (!isset($wanted_assets[$assetid])) {
					unset($todo[$assetid]);
				}
			}
		}

		// check to see if this is a special user without restrictions
		$user_restrictions = (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin());
		if (!$user_restrictions) return;

		if (empty($wanted_statuses)) {
			// get a list of assets that are not live
			$sql = 'SELECT
						assetid, type_code
					FROM
						sq_ast
					WHERE
							assetid IN ('.implode(',', $todo_keys).')
						AND status < '.$db->quote(SQ_STATUS_LIVE);

			$wanted_assets = $db->getAssoc($sql);
			assert_valid_db_result($wanted_assets);

			foreach ($wanted_assets as $wanted) {
				foreach ($shadow_assets as $key => $value) {
					if (strpos($key, $wanted) !== false) {
						$wanted_assets[] = $key;
					}
				}
			}
		}

		foreach ($todo as $assetid => $v) {
			if (isset($wanted_assets[$assetid])) {
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $wanted_assets[$assetid]);
				if (!$asset->readAccess()) unset($todo[$assetid]);
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				unset($asset);
			}
		}



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
					if (!isset($allowed_asset_map[$allowed_type])) {
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
		// grouping by custom groups means we will follow the user defined group structure
		if (is_null($group_by)) {
			$group_by = $this->attr('group_by');
		}

		$this->_tmp['result_page'] = 1;

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

				$this->_tmp['result_page'] = (int)$result_page;
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
				// other letter here refers to the situation where there are assets to list that do not start with A-Z
				$other_letter_value = $this->attr('other_page_text');

				// '@' symbol is chosen here out of the blue, this is used only as a value of the request var
				$other_letter = '@';
				foreach (range('A','Z') as $one_letter) {
					$all_letters[$one_letter] = $one_letter;
				}

				foreach ($children as $childid => $data) {
					$letter = strtoupper($data['first_letter']);
					if (!isset($all_letters[$letter])) {
						$letter = $other_letter;
					}
					$todo[$letter][$childid] = $data['type_code'];
				}

				// 'other' letter is only displayed if there are assets that start with non A-Z chars
				if (isset($todo[$other_letter])) {
					$all_letters[$other_letter] = $other_letter_value;
				}

				// here we generate a list of only active letters
				$letters = Array();
				foreach ($all_letters as $one_letter => $text) {
					if (isset($todo[$one_letter])) {
						$letters[] = $one_letter;
					}
				}

				$first_available_letter = $letters[0];

				$result_page = array_get_index($_REQUEST, 'result_page', $first_available_letter);

				// by default, result letter is the first letter
				if (!isset($todo[$result_page])) {
					$result_page = $first_available_letter;
				}

				$this_letter_key = array_search($result_page, $letters);

				// previous page link
				$prev_page_text = $this->attr('prev_page_text');
				if ($this_letter_key > 0) {
					$prev_page_text = '<a href="'.replace_query_string_vars(Array('result_page' => $letters[$this_letter_key - 1])).'">'.$prev_page_text.'</a>';
				}
				$replacements['previous_page'] = $prev_page_text;

				// next page link
				$next_page_text = $this->attr('next_page_text');
				if ($this_letter_key < (sizeof($letters) - 1)) {
					$next_page_text = '<a href="'.replace_query_string_vars(Array('result_page' => $letters[$this_letter_key + 1])).'">'.$next_page_text.'</a>';
				}
				$replacements['next_page'] = $next_page_text;

				$replacements['total_pages'] = count($letters);
				$this->_tmp['result_page'] = $this_letter_key + 1;

				// list of pages and links to them
				$page_list = '';
				foreach ($all_letters as $one_letter => $text_value) {
					if (isset($todo[$one_letter])) {
						$page_list .= ' <a href="'.replace_query_string_vars(Array('result_page' => $one_letter)).'">'.$text_value.'</a> ';
					} else {
						$page_list .= ' '.$text_value.' ';
					}
				}

				$todo = $todo[$result_page];
				$replacements['page_list'] = $page_list;

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
				//we need to do an element count so we can work out the page stuff
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
				$this->sortGroups($this->attr('asset_grouping'), $children, $children);
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
				if ($num_per_page) {
					$result_page = min($result_page, ceil($total_assets / $num_per_page));
				}

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

				$this->_tmp['result_page'] = (int)$result_page;
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

		$replacements['page_number'] = $this->_tmp['result_page'];

		// do this for compatibility - deprecated. not sure if any clients
		// are using this, since 'current_page' is not one of the advertised keywords
		$replacements['current_page'] = $replacements['page_number'];

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
		if (empty($todo)) return;

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

		// in case it wasn't done beforehand in printContents() or something
		$relations = $this->_analyseCheckAllRelations($todo);

		ob_start();
			?>
			<script type="text/javascript">
			<!--
				<?php echo $this->_buildRelationsJS(); ?>
			//-->
			</script>
			<?php
		$this->_tmp['js_relation_code'] = ob_get_clean();

		if ($this->attr('group_by') == 'grouped') {
			$this->_printGroups($todo, $this->attr('asset_grouping'));
		} else {
			$todo = $this->sortAssetList($todo, $this->getSortInfo());
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
			// get the group bodycopy relative to this level
			$remaining_groups = array_shift($group_info);
			$format_bodycopy =& $GLOBALS['SQ_SYSTEM']->am->getAsset($remaining_groups['format_assetid']);
			$format_keywords = $format_bodycopy ->getKeywords();
			foreach ($todo as $group_level => $value) {
					// lets buffer whatever contents we need
					$replacements = Array();

					switch ($remaining_groups['group_type']) {

						case 'parent_asset':
							// we need to get info from the parent asset
							$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($group_level);
							$group_name = $asset->attr('name');
							foreach ($format_keywords as $format_keyword) {
								$pos = strpos($format_keyword, 'group');
								if ($pos === false) {
									$keyword = substr_replace($format_keyword, 'asset', 0, 6);
									$replacements[$format_keyword] = $asset->getKeywordReplacement($keyword);
								}
							}
							$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
						break;

						case 'metadata':
						case 'standard_asset':
							// the field value will have been the index
							$group_name = $group_level;
						break;

						case 'attribute':
							// oh noes, implementation reserved!
						break;

						default:
							// there is no spoon .....
						break;
					}

					if (isset($this->_tmp['group_info'])) {
						array_push($this->_tmp['group_info'], $group_level);
					} else {
						$this->_tmp['group_info'] = Array($group_level);
					}

					// the selection stuff needs the group_info array popped for
					// each group level selection, so we need a copy for _printAsset()
					// too
					if (isset($this->_tmp['group_levels'])) {
						array_push($this->_tmp['group_levels'], $group_level);
					} else {
						$this->_tmp['group_levels'] = Array($group_level);
					}

					ob_start();
						$this->_printGroups($todo[$group_level], $group_info);
						$group_contents = ob_get_contents();
					ob_end_clean();

					// populate replacements
					$replacements['group_listing'] = $group_contents;
					$replacements['group_name']    = $group_name;

					// only print the 'check all' checkbox if we're only dealing with checkboxes
					$selections = $this->attr('asset_selections');
					$selection_defaults = $this->getAssetSelectionDefaults();

					foreach ($selections as $selection_name => $settings) {
						if (!in_array($selection_name.'_check_all_in_group', $format_keywords)) {
							continue;
						}
						$this->registerFormField($selection_name.'_check_all_in_group');

						// predefine some commonly used settings, for brevity's sake
						$selection_options = array_get_index($settings, 'options', $selection_defaults['options']);
						$selection_allow_multiple = array_get_index($selection_options, 'allow_multiple', $selection_defaults['options']['allow_multiple']);

						if ((array_get_index($settings, 'type', $selection_defaults['type']) == 'radio') && $selection_allow_multiple) {
							ob_start();
							$onclick = Array();
							$group_cb_id  = 'g_'.implode('_', $this->_tmp['group_info']);
							$control_name = make_valid_html_id(str_replace(' ', '_', $selection_name.$group_cb_id));
							check_box($control_name, '1', false, 'sq_listing_check_state(this, \''.$selection_name.'\')');
							$replacements[$selection_name.'_check_all_in_group'] = ob_get_clean();
							$replacements[$selection_name.'_selection_id'] = $control_name;
						}
					}

					array_pop($this->_tmp['group_info']);
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
		$using_groups = $this->attr('group_by') == 'grouped';
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
							$this->_printAsset($assetid, $list_position);
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
				$this->_printAsset($assetid, $list_position);
				$list_position++;
			}
		}

	}//end _printAssetList()


	/**
	* Print a shadow asset from the list
	*
	* @param string	$assetid		the ID of the asset we are printing
	* @param array	$bc_keywords	the tyope format body copy keywords that are to be replaced
	*
	* @return array
	* @access private
	* @see printAssetList()
	*/
	function _getShadowAssetKeywordReplacements($assetid, $bc_keywords)
	{
		$keywords = Array();
		$shadow_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		foreach ($bc_keywords as $key => $keyword) {
			$keywords[$keyword] = $shadow_asset->getKeywordReplacement($keyword);
		}
		return $keywords;

	}//end _getShadowAssetKeywordReplacements()


	/**
	* Print a single asset from the list
	*
	* @param string	$assetid		the ID of the asset we are printing
	* @param int	$list_position	the position of this asset in the list
	*
	* @return void
	* @access private
	* @see printAssetList()
	*/
	function _printAsset($assetid, $list_position)
	{
		$mm         = null;
		$contents   = '';
		$keywords   = Array();
		$customised = $this->getFormats('type_formats');

		$asset_info_fields = $this->getSortableAssetInfo();
		$bodycopy = null;

		//if a ':' exists in the assetid, then it is a shadow asset
		$is_shadow = strpos($assetid, ':');

		// we can get the type code from the asset info array
		//if the asset info array is empty then there is a possibility that we are trying to print a shadow asset
		//so we load the asset and get its typecode
		$type_code = null;
		$info = array_get_index($this->_tmp['assets_info'], $assetid, Array('type_code' => ''));
		if (!empty($info['type_code'])) {
			$type_code = $info['type_code'];
		} else {
			$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$type_code = $asset->type();
		}
		// check to see if there is a specific position format for us
		if (isset($this->_tmp['position_formats'][$list_position])) {
			$bodycopy = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_tmp['position_formats'][$list_position], 'bodycopy');
		}//end if has position format

		// if we can't find a position format
		// search for type format, if we don't already know that default format should be used
		if (is_null($bodycopy) && !in_array($type_code, $this->_tmp['is_default'])) {
			// is there previous information about type format?
			if (isset($this->_tmp['type_formats']) && array_key_exists($type_code, $this->_tmp['type_formats'])) {
				$type_key = array_search($this->_tmp['type_formats'][$type_code], $customised);
				$type_code = $this->_tmp['type_formats'][$type_code];
			} else {

				$parent = null;
				$parents = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($type_code);

				if (($type_key = array_search($type_code, $customised)) === false) {
					foreach ($parents as $parent) {
						// any of parent in the custom format?
						if (array_search($parent, $customised) !== false) {
							$this->_tmp['type_formats'][$type_code] = $parent;
							$type_key = array_search($this->_tmp['type_formats'][$type_code], $customised);
							$type_code = $parent;
						}
					}
				}
			}
			if (isset($type_key) && $type_key !== false) {
				$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->_tmp['type_folder_id'], SQ_LINK_TYPE_2, 'bodycopy', true, $type_code);
				if ($link) {
					$bodycopy = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], 'bodycopy');
				} else {
					$this->_tmp['is_default'][] = $type_code;
				}
			}

		}

		// if no bodycopy found, try the default format
		if (is_null($bodycopy)) {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->_tmp['type_folder_id'], SQ_LINK_TYPE_2, 'bodycopy', true, 'default_format');
			if ($link) {
				$bodycopy = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], 'bodycopy');
			} else {
				// TODO: localise
				trigger_error('Cannot Find the Default Bodycopy');
				return;
			}//end if using default format
		}


		// by the time we reach this point we already have a bodycopy for this asset
		$this_needs_asset = false;
		$bc_keywords = $bodycopy->getKeywords();
		$keywords = Array();
		//if the asset is a shadow asset  then get the keywords separately
		if ($is_shadow) {
			$keywords = $this->_getShadowAssetKeywordReplacements($assetid, $bc_keywords);
		} else {
			foreach ($bc_keywords as $keyword) {
				if (!(strpos($keyword, 'asset_') === 0) || ((strpos($keyword, 'asset_metadata_') !== 0) && (!in_array(substr($keyword, 6), $asset_info_fields)))) {
					$this_needs_asset = true;
					break;
				}
			}

			if ($this_needs_asset) {
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				if (is_null($asset)) return;
				foreach ($bc_keywords as $keyword) {
					$keywords[$keyword] = $asset->getKeywordReplacement($keyword);
				}
				if (preg_match('|%asset_contents%|is', $bodycopy->getRawBodycopyContent())) {
					ob_start();
						if ($asset->id != $this->id) $asset->printBody();
						$keywords['asset_contents'] = ob_get_contents();
					ob_end_clean();

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
					$nurl = strip_url($nurl);
					$nurl = stristr($nurl,'/');
					$nurl = substr($nurl,2);

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
						$parent_assets = $GLOBALS['SQ_SYSTEM']->am->getLineageFromUrl(null, $nurl);

						// searching for the root asset
						$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', false, 'root');
						$root_assetid = empty($root_link) ? 0 : $root_link['minorid'];
						$show_root = $this->attr('root_in_lineage');
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
						// It is a valid lineage valid though, so lets print it, as it's better than the
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
				}// end if lineage keyword present

				// end results body stuff

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				unset($asset);
			} else {
				$keywords = Array();
				foreach ($this->_tmp['assets_info'][$assetid] as $field => $value) {
					$keywords['asset_'.$field] = $value;
				}

			}//end if needs asset
		}

		$group_keywords = preg_grep('/group[0-9]+.*/', $bc_keywords);
		foreach ($group_keywords as $group_keyword) {
			preg_match('/group([0-9]+)_(.*)/', $group_keyword, $matches);
			if (isset($this->_tmp['group_levels'][$matches[1] - 1])) {
				$parent_id = $this->_tmp['group_levels'][$matches[1] - 1];
				if (assert_valid_assetid($parent_id, '', true, false)) {
					// we have an assetid, so do what we want with the parent
					switch ($matches[2]) {
						case 'link_value':
							$link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($parent_id, $assetid);
							$keywords[$group_keyword] = array_get_index($link, 'value', '');
						break;
						case 'link_type':
						case 'dependant':
						case 'exclusive':
						default:
							// other possible uses
						break;
					}
				}
			}
		}

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

		$keywords['page_href'] = $this->getHref();

		if ($this->attr('group_by') == 'letter') {
			$keywords['asset_position'] = $list_position;
		} else {
			$keywords['asset_position']  = (array_get_index($this->_tmp, 'result_page', 1) - 1) * $this->attr('num_per_page') + $list_position;
		}

		// asset selections
		$selections         = $this->attr('asset_selections');
		$selection_defaults = $this->getAssetSelectionDefaults();

		if (is_array($selections) && !empty($selections)) {

			foreach ($selections as $selection_name => $settings) {

				$buffering = in_array($selection_name.'_asset_selection', $bc_keywords);

				if ($buffering) {
					ob_start();
					$this->registerFormField($selection_name.'_asset_selection');
				}

				// predefine some commonly used settings, for brevity's sake
				$selection_options = array_get_index($settings, 'options', $selection_defaults['options']);
				$selection_restrict = array_get_index($selection_options, 'restrict_range', $selection_defaults['options']['restrict_range']);
				$selection_restrict_enable = array_get_index($selection_restrict, 'enable', $selection_defaults['options']['restrict_range']['enable']);
				$selection_restrict_min = array_get_index($selection_restrict, 'min', $selection_defaults['options']['restrict_range']['min']);
				$selection_restrict_max = array_get_index($selection_restrict, 'max', $selection_defaults['options']['restrict_range']['max']);
				$selection_allow_negative = array_get_index($selection_options, 'allow_negative', $selection_defaults['options']['allow_negative']);
				$selection_allow_multiple = array_get_index($selection_options, 'allow_multiple', $selection_defaults['options']['allow_multiple']);

				// grab the values for the field to display
				$selection_value = $this->getAssetSelectionValue($selection_name, $assetid);

				if ($selection_restrict_enable) {
					$selection_value = ($selection_restrict_min > $selection_value ? $selection_restrict_min : $selection_value);
				}

				switch (array_get_index($settings, 'type', $selection_defaults['type'])) {
					case 'textbox':
						$selection_name_field = str_replace(' ', '_', $selection_name.'['.$assetid.']');
						$selection_id = make_valid_html_id(str_replace(' ', '_', $selection_name.'_'.$assetid), '_');

						if ($buffering) {
							if ($selection_restrict_enable) {
								int_text_box($selection_name_field, $selection_value, $selection_allow_negative, 3, $selection_restrict_min, $selection_restrict_max, '', false, 'id="'.$selection_id.'"');
							} else {
								int_text_box($selection_name_field, $selection_value, $selection_allow_negative, 3, null, null, '', false, 'id="'.$selection_id.'"');
							}
						}
					break;

					case 'selection':
						$selection_name_field = str_replace(' ', '_', $selection_name.'['.$assetid.']');
						$selection_id = make_valid_html_id(str_replace(' ', '_', $selection_name.'_'.$assetid), '_');

						if ($buffering) {
							// list contents
							$selection_list = Array();
							if (!empty($settings['options']['restrict_range']['enable'])) {
								for ($ii = $selection_restrict_min; $ii <= $selection_restrict_max; $ii++) {
									$selection_list[$ii] = $ii;
								}
							} else {
								$selection_list = Array('0');
							}

							// check that the value is on the list
							if (!isset($selection_list[$selection_value])) {
								$selection_value = reset($selection_list);
							}

							// list display
							combo_box($selection_name_field, $selection_list, $selection_allow_multiple, $selection_value, 0, 'id="'.$selection_id.'"');
						}
					break;

					case 'radio':
					default:
						if ($selection_allow_multiple) {
							$selection_name_field = str_replace(' ', '_', $selection_name.'['.$assetid.']');
							$selection_id = make_valid_html_id(str_replace(' ', '_', $selection_name.'_'.$assetid), '_');

							if ($buffering) {
								check_box($selection_name_field, '1', ($selection_value > 0 ? true : false), "sq_listing_check_state(this, '".$selection_name."')", 'id="'.$selection_id.'"');

								// the flag is used to determine whether a checkbox is
								// present and not ticked, or not present at all
								//hidden_field($selection_name.'_flag', '1');
							}
						} else {
							$selection_name_field = str_replace(' ', '_', $selection_name);
							$selection_id = make_valid_html_id(str_replace(' ', '_', $selection_name.'_'.$assetid), '_');

							if ($buffering) {
								radio_button($selection_name_field, $assetid, ($selection_value == $assetid ? true : false), '', 'id="'.$selection_id.'"');
							}
						}
					break;
				}

				// set selection and its ids keywords
				if ($buffering) {
					$keywords[$selection_name.'_asset_selection'] = ob_get_clean();
				}
				$keywords[$selection_name.'_selection_name'] = $selection_name_field;
				$keywords[$selection_name.'_selection_id'] = $selection_id;
			}
		}

		// print asset ID always
		$keywords['asset_assetid'] = $assetid;

		// kept for backwards compatibility
		$keywords['result_no']  = &$keywords['asset_position'];
		$keywords['root_nodes'] = implode(',', $this->getRootNodes());

		$keywords += $this->getExtendedAssetKeywordReplacements();

		$bodycopy->setKeywordReplacements($keywords);
		$bodycopy->printBody();

	}//end _printAsset()


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
				if (strstr($root_link['minorid'], ':') !== false) {
					$root_asset_ids[] = $root_link['minorid'];
				} else {
					$root_asset_ids[] = (int)$root_link['minorid'];
				}
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

			if (empty($root_asset_ids)) {
				$root_asset_ids = Array($this->id);
			}

			$this->_tmp['root_asset_ids'] = $root_asset_ids;

		}

		return $this->_tmp['root_asset_ids'];

	}//end getRootNodes()


	/**
	* Returns a list of extra keywords that should be replaced for a particular
	* listing asset
	*
	* This allows search page, for instance, to define its result number and
	* score fields so they don't blank out when keywords are replaced in listing
	* engine. Return data format: Array(string => string)	keyword name => replacement
	*
	* @return array
	* @access public
	*/
	function getExtendedAssetKeywordReplacements()
	{
		return Array();

	}//end getExtendedAssetKeywordReplacements()


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
	* This functionality is required for being able to determine if a field was used
	* during output creation in the printContents. This is useful if to avoid field name
	* conflicts or to perform additional logic like using it in javascript, etc
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
		$min_depth = $this->attr('min_depth');
		if ($min_depth == '') $min_depth = null;
		$max_depth = $this->attr('max_depth');
		if ($max_depth == '') $max_depth = null;

		$root_nodes = $this->getRootNodes();

		// we need assets and groups, so if none given, let's bail
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
		$limit_array = Array();

		foreach ($group_levels as $group_key => $group_level) {
			array_unshift($limit_array, array_get_index($group_level, 'max_children', null));
			$new_asset_groups = Array();

			// go one node UP the tree, no matter whether we're going up or down
			if (!is_null($min_depth)) {
				$min_depth -= 1 * ($this->attr('direction') == 'up' ? -1 : 1);
			}
			if (!is_null($max_depth)) {
				$max_depth -= 1 * ($this->attr('direction') == 'up' ? -1 : 1);
			}

			foreach ($asset_groups as $key => $group_data) {
				$assetid = $group_data['assetid'];

				if ($last_parent_asset === false) {
					$current_parent = $assetid;
				} else {
					$current_parent = $group_data['group'][$last_parent_asset];
				}

				switch ($group_level['group_type']) {
					case 'parent_asset':
						$value = $this->getGroupableParentAssetids($current_parent);

						// skip this whole section if both depth fields are null
						// otherwise, we are limiting by parent's depth, relative
						// to what is set for leaf nodes
						if (!is_null($min_depth) || !is_null($max_depth)) {

							foreach($value as $key => $parent_assetid) {
								if ($this->attr('direction') == 'down') {
									$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($parent_assetid, '', false, null, null, $min_depth, $max_depth);
								} else {
									$parents = $GLOBALS['SQ_SYSTEM']->am->getChildren($parent_assetid, '', false, null, null, $min_depth, $max_depth);
								}

								$intersection = array_intersect(array_keys($parents), $root_nodes);
								if (empty($intersection)) {
									unset($value[$key]);
								}
							}
						}

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

					case 'standard_asset':
						$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($current_parent));
						if (!empty($asset_info)) {
							$asset_info = $asset_info[$current_parent];

							$standard_asset_field = $group_level['field'];

							switch ($standard_asset_field) {
								case 'assetid':
									$value = $current_parent;
								break;

								case 'status':
									$value = get_status_description($asset_info['status']);
								break;

								default:
									$value = $asset_info[$standard_asset_field];
								break;
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

		$this->limitAssetsRecursively($results, $limit_array);

	}//end groupAssetsRecursively()


	/**
	* Recursive function to limit the number of assets in each group
	*
	* The second parameter, $limit_array, contains a list of integers that determine
	* how many assets/child groups will be kept at each level. eg. in a listing
	* engine-derived asset with two groups, a $limit_array of Array(2,3) means that
	* the top-level group will contain at most 2 child groups, and each child group
	* will contain at most 3 child assets. Any value can be null or zero for all
	* assets to be kept. The $limit_array passed to each recursive level is reduced
	* by one (the top level) each time.
	*
	* @param array	&$results		The grouped and sorted results list (array of mixed)
	* @param array	$limit_array	Contains group limits for each level (array of int|null)
	*
	* @return void
	* @access public
	*/
	function limitAssetsRecursively(&$results, $limit_array)
	{
		$num = array_shift($limit_array);

		foreach ($results as $key => $val) {
			if (!is_null($num) && ($num >= 1)) {
				while (count($results[$key]) > $num) {
					array_pop($results[$key]);
				}
			}

			if (!empty($limit_array)) {
				$this->limitAssetsRecursively($results[$key], $limit_array);
			}
		}

	}//end limitAssetsRecursively()


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
	function getGroupableParentAssetids($child_assetid)
	{
		$root_nodes = $this->getRootNodes();

		$parent_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($child_assetid, $this->attr('link_types'), '', true, 'minor');

		$parent_link_assetids = Array();
		foreach ($parent_links as $parent_link) {
			if (empty($root_nodes) || ($this->attr('direction') == 'up')) {
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

					if (count(array_intersect(array_keys($parents_of_parent), $root_nodes)) > 0) {
						$parent_link_assetids[] = $parent_assetid;
					}
				}
			}//end if empty root nodes
		}//end foreach

		return $parent_link_assetids;

	}//end getGroupableParentAssetids()


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

		//point us at the first element, discarding it as we go
		$group_type = array_shift($group_levels);

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

			case 'standard_asset':
				// standard asset groups are sorted just on the value
				if (array_get_index($group_type, 'sort_order', 'asc') == 'asc') {
					asort($results);
				} else {
					arsort($results);
				}
			break;

			case 'attribute':
				// implementation reserved
			break;
		}

		//if their are group levels remaining, we want to call
		//this function again for each of the groups we have made
		if (count($group_levels) > 0) {
			foreach ($results as $index => $group) {
				$this->sortGroups($group_levels, $group, $results[$index]);
			}
		}

	}//end sortGroups()


	/**
	* Reindexes the group format bodycopies so any gaps are closed and bodycopy
	* names are referring to their correct group numbers
	*
	* @return boolean
	* @access public
	*/
	function reindexGroupFormats()
	{
		// if we have no group formats, it's not worth our time
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
	* Preprocess the Asset Selection values before displaying the page or redirecting
	* Overwritten by the children
	*
	* @return boolean
	* @access public
	*/
	function processAssetSelections()
	{
		return true;

	}//end processAssetSelections()


	/**
	* Provide the default values for each of the selection fields
	* Overwritten by the children
	*
	* @param string	$selection_name	name
	* @param string	$assetid		asset id
	*
	* @return int
	* @access public
	*/
	function getAssetSelectionValue($selection_name, $assetid)
	{
		return 0;

	}//end getAssetSelectionValue()


	/**
	* List the default settings format for Asset Selections
	*
	* @return array
	* @access public
	*/
	function getAssetSelectionDefaults()
	{
		return Array(
				'type'		=> 'radio',
				'options'	=> Array(
								'allow_multiple'	=> 0,
								'allow_negative'	=> 0,
								'restrict_range'	=> Array(
														'enable'	=> 0,
														'min'		=> 0,
														'max'		=> 10,
													   ),
							   ),
				'permanent'	=> 0,
			   );

	}//end getAssetSelectionDefaults()


	/**
	* List the default settings format for Asset Targets
	*
	* @return array
	* @access private
	*/
	function getAssetTargetDefaults()
	{
		return Array(
				'label'			=> '',
				'target_asset'	=> Array(
									'assetid'	=> 0,
									'url'		=> '',
								   ),
				'target_url'	=> '',
				'permanent'		=> 0,
			   );

	}//end getAssetTargetDefaults()


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
			$keywords = $this->_getGroupFolderKeywords();

		} else if (in_array($position_folder->id, array_keys($parent_folders))) {
			// position format
			$keywords = $this->_getGenericSingleAssetFormatKeywords();

		} else if (in_array($type_folder->id, array_keys($parent_folders))) {
			// specific or default type format
			$type_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($type_folder->id, SQ_LINK_TYPE_2, 'bodycopy');
			$type_codes = Array();
			foreach ($type_links as $link_info) {
				if (isset($parents[$link_info['minorid']])) {
					$type_codes[] = $link_info['value'];
				}
			}

			$type_name = null;
			if ((count($type_codes) == 1) && ($type_codes[0] != 'default_format')) {
				$type_name = $type_codes[0];
			}
			$keywords = $this->_getGenericSingleAssetFormatKeywords($type_name);

		} else {
			// page contents format
			$keywords = $this->_getContentsKeywords();
		}

		$vars['keywords'] = array_merge($vars['keywords'], $keywords);
	}//end onRequestKeywords()


	/**
	* Get keywords from another asset
	*
	* Gets a list of keywords either from a base asset or a specified asset type
	*
	* @param string	$type	Asset type to get keywords from
	*
	* @return array
	* @access private
	*/
	function _getGenericSingleAssetFormatKeywords($type=null)
	{
		$keywords = $this->_getAdditionalSingleAssetFormatKeywords();

		if (!empty($type)) {
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($type);
			$dummy_asset =& new $type();
			$keywords += $dummy_asset->getAvailableKeywords();
			unset($dummy_asset);
		} else {
			$keywords += Asset::getAvailableKeywords();
		}

		return $keywords;

	}//end _getGenericSingleAssetFormatKeywords()


	/**
	* Get additional keywords to be used in the single-asset format
	*
	* Provides keywords to be used alongside those provided by assets themselves.
	* These keywords are understood only by this asset
	*
	* @return void
	* @access private
	*/
	function _getAdditionalSingleAssetFormatKeywords()
	{
		$keywords['asset_lineage'] = translate('asset_lineage');
		$keywords['asset_lineage_linked'] = translate('asset_lineage_linked');
		$keywords['asset_contents'] = translate('cms_listing_asset_contents');
		$keywords['asset_name_linked'] = translate('cms_listing_asset_name_linked');
		$keywords['asset_short_name_linked'] = translate('cms_listing_asset_short_name_linked');
		$keywords['asset_position'] = translate('cms_listing_asset_position');
		$keywords['root_nodes'] = translate('cms_listing_keyword_root_nodes');

		// asset selections
		$selections = $this->attr('asset_selections');
		if (is_array($selections) && !empty($selections)) {
			foreach ($selections as $name => $settings) {
				$keywords[$name.'_asset_selection'] = translate('cms_listing_keyword_asset_selection', $name);
			}
		}

		return $keywords;

	}//end _getAdditionalSingleAssetFormatKeywords()


	/**
	* Return the available keywords from the parents
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
	function getAvailableKeywords()
	{
		$keywords = parent::getAvailableKeywords();
		return $keywords;

	}//end getAvailableKeywords()


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
		$GLOBALS['SQ_SYSTEM']->lm->includeAssetStrings('page_asset_listing');

		$keywords['sort_by']				= translate('cms_listing_keyword_sort_by');
		$keywords['sort_order']				= translate('cms_listing_keyword_sort_order');
		$keywords['reset_button']			= translate('cms_listing_keyword_reset_button');
		$keywords['submit_button']			= translate('cms_listing_keyword_submit_button');
		$keywords['root_nodes']				= translate('cms_listing_keyword_root_nodes');

		$keywords['asset_listing']			= translate('cms_listing_asset_listing');
		$keywords['previous_page']			= translate('cms_listing_previous_page_link');
		$keywords['next_page']				= translate('cms_listing_next_page_link');
		$keywords['page_list']				= translate('cms_listing_page_list');
		$keywords['page_number']			= translate('cms_listing_page_number');
		$keywords['asset_count']			= translate('cms_listing_asset_count');
		$keywords['page_asset_count']		= translate('cms_listing_page_asset_count');
		$keywords['total_pages']			= translate('cms_listing_total_pages');
		$keywords['first_asset_position']	= translate('cms_listing_first_asset_position');
		$keywords['last_asset_position']	= translate('cms_listing_last_asset_position');
		$keywords['unique_asset_count']     = translate('cms_listing_unique_asset_count');

		$keywords['select_all_js_code']		= 'Select All Script Code';

		$selections = $this->attr('asset_selections');
		if (is_array($selections) && !empty($selections)) {
			foreach ($selections as $name => $settings) {
				if (array_get_index($settings, 'type', '') == 'radio' && !empty($settings['options']['allow_multiple'])) {
					$keywords[$name.'_check_all'] = translate('cms_listing_asset_selection_check_all', $name);
				}
			}
		}

		$targets = $this->attr('asset_targets');
		if (is_array($targets) && !empty($targets)) {
			foreach ($targets as $name => $settings) {
				$keywords[$name.'_asset_target'] = translate('cms_listing_asset_target_button', $name);
			}
		}

		return $keywords;

	}//end _getContentsKeywords()


	/**
	* Get the keywords which are specific in Type Formats
	*
	* Marked deprecated on 15/09/2005
	*
	* @deprecated
	* @return array
	* @access protected
	*/
	function _getTypeFormatsKeywords()
	{
		$keywords['asset_lineage'] = translate('asset_lineage');
		$keywords['asset_lineage_linked'] = translate('asset_lineage_linked');
		$keywords['asset_contents'] = translate('cms_listing_asset_contents');
		$keywords['asset_name_linked'] = translate('cms_listing_asset_name_linked');
		$keywords['asset_short_name_linked'] = translate('cms_listing_asset_short_name_linked');
		$keywords['asset_position'] = translate('cms_listing_asset_position');
		$keywords['root_nodes'] = translate('cms_listing_keyword_root_nodes');

		// asset selections
		$selections = $this->attr('asset_selections');
		if (is_array($selections) && !empty($selections)) {
			foreach ($selections as $name => $settings) {
				$keywords[$name.'_asset_selection'] = translate('cms_listing_keyword_asset_selection', $name);
			}
		}

		return $keywords;

	}//end _getTypeFormatsKeywords()


	/**
	* Get the keywords which are specific in Group Folder
	*
	* @return array
	* @access protected
	*/
	function _getGroupFolderKeywords()
	{
		$fake_asset =& new Asset();
		$keywords = $fake_asset->getAvailableKeywords();
		foreach ($keywords as $key =>$keyword) {
			$key1 = substr_replace($key, 'parent', 0, 5);
			$keywords[$key1] = $keywords[$key];
			unset($keywords[$key]);
		}
		$keywords['group_listing'] = translate('cms_listing_group_listing');
		$keywords['group_name']    = translate('cms_listing_group_name');
		$selections = $this->attr('asset_selections');
		if (is_array($selections) && !empty($selections)) {
			foreach ($selections as $name => $settings) {
				if (array_get_index($settings, 'type', '') == 'radio' && !empty($settings['options']['allow_multiple'])) {
					$keywords[$name.'_check_all_in_group'] = translate('cms_listing_asset_selection_check_all_in_group', $name);
				}
			}
		}
		return $keywords;

	}//end _getGroupFolderKeywords()


//--        KEYWORD REPLACEMENT        --//


	/**
	* Get bodycopy keyword replacement
	*
	* Provides a mechanism for providing keyword replacement functions by
	* looking up the function named 'getKeywordNameKeywordReplacement'
	* so an asset wishing to add support for 'best_ever' custom keyword
	* should implement a function getBestEverKeywordReplacement()
	* which returns a replacement string
	* -
	* Overridden in listing_engine to handle dynamic keywords in the page
	* contents, such as Asset Targets
	*
	* @param string	$keyword	Keyword to find a replacement for
	*
	* @return string
	* @access public
	*/
	function getKeywordReplacement($keyword)
	{
		if (empty($keyword)) return '';

		// check the keyword against the dynamic keywords

		// asset targets
		if (preg_match('/^(.*)(_asset_target)$/', $keyword, $matches)) {
			if (isset($matches[1]) && isset($matches[2])) {
				$args         = $matches[1];
				$keyword_stem = $matches[2];
			}
		}


		// setup the replacement function call
		if (!empty($keyword_stem) && !empty($args)) {
			$func_name = 'get'.ucwords_no_space($keyword_stem).'KeywordReplacement';
			if (method_exists($this, $func_name)) {
				return $this->$func_name($args);
			}
		}

		// otherwise, fall back to the parent
		$replacement = parent::getKeywordReplacement($keyword);
		return $replacement;

	}//end getKeywordReplacement()


	/**
	* Return a subset of the replacements for the keywords available to the Page Contents Bodycopy for this asset
	*
	* The return value is in the form:
	* <pre>
	* Array(
	*     'keyword' => 'name',
	* )
	* </pre>
	*
	* @param array	$keywords	the keywords from the content
	*
	* @return array
	* @access public
	*/
	function getContentsKeywordReplacements($keywords=Array())
	{
		$replacements = Array();

		$selections = $this->attr('asset_selections');
		$selection_defaults = $this->getAssetSelectionDefaults();

		foreach ($selections as $selection_name => $settings) {

			if (!in_array($selection_name.'_check_all', $keywords)) {
				continue;
			}
			$this->registerFormField($selection_name.'_check_all');

			// predefine some commonly used settings, for brevity's sake
			$selection_options = array_get_index($settings, 'options', $selection_defaults['options']);
			$selection_allow_multiple = array_get_index($selection_options, 'allow_multiple', $selection_defaults['options']['allow_multiple']);

			// only print the 'check all' checkbox if we're only dealing with checkboxes
			if ((array_get_index($settings, 'type', $selection_defaults['type']) == 'radio') && $selection_allow_multiple) {
				ob_start();
				$onclick = Array();
				$group_cb_id  = 'all';
				$control_name = str_replace(' ', '_', $selection_name.$group_cb_id);
				check_box($control_name, '1', false, "sq_listing_check_state(this, '".$selection_name."')");
				$replacements[$selection_name.'_check_all'] = ob_get_clean();
				$replacements[$selection_name.'_selection_id'] = $control_name;
			} else {
				$replacements[$selection_name.'_check_all'] = '';
			}
		}

		return $replacements;

	}//end getContentsKeywordReplacements()


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
	* Get the replacement for "reset_button" keyword
	*
	* @return string
	* @access public
	*/
	function getResetButtonKeywordReplacement()
	{
		$prefix = $this->getPrefix();
		$field_name = $prefix.'_reset_button';
		$this->registerFormField($field_name);

		$button_text = $this->attr('reset_button_text');

		ob_start();
			reset_button($field_name, $button_text);
		return ob_get_clean();

	}//end getResetButtonKeywordReplacement()


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
	* Get the replacements for the dynamic "_asset_target" keyword
	*
	* @param string	$name	name
	*
	* @return string
	* @access public
	*/
	function getAssetTargetKeywordReplacement($name)
	{
		$targets    = $this->attr('asset_targets');
		$prefix     = $this->getPrefix();
		$field_name = $prefix.'_'.$name.'_asset_target';
		$this->registerFormField($field_name);

		if (isset($targets[$name])) {
			$defaults         = $this->getAssetTargetDefaults();
			$button_text      = array_get_index($targets[$name], 'label',        $defaults['label']);
			$target = null;

			// determine whether we're using the url or the asset (url takes precedence)
			$target_url       = array_get_index($targets[$name], 'target_url',   $defaults['target_url']);
			if ($target_url) {
				$target = $target_url;
			} else {
				$target_asset     = array_get_index($targets[$name], 'target_asset', $defaults['target_asset']);
				$target_assetid   = array_get_index($target_asset,   'assetid',      $defaults['target_asset']['assetid']);
				if ($GLOBALS['SQ_SYSTEM']->am->assetExists($target_assetid)) {
					$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($target_assetid);
					$target = $asset->getURL();
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($target_assetid);
				}
			}

			if (!is_null($target)) {
				ob_start();
					submit_button($field_name, $button_text, "this.form.action = '".$target."';");
				return ob_get_clean();
			}
		}

		return '';

	}//end getAssetTargetKeywordReplacement()


	/**
	* Get the replacement for "structured_search_location" keyword
	*
	* @return string
	* @access public
	*/
	function getStructuredRootNodeKeywordReplacement()
	{
		$options = $this->attr('structured_dropdown_options');

		//Initial type_codes, before we check for inheritance
		$pass_codes = array_keys($options['asset_types']);
		foreach ($options['asset_types'] as $code => $inherit) {
			if ($inherit == 1) {
				$pass_codes += $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($code);
			}
		}
		$selected = (isset($_REQUEST[$options['name']])) ? Array($_REQUEST[$options['name']]) : Array();

		ob_start();
			structured_drop_down($options['name'], $options['root_node'], $pass_codes, $selected, $options['width'], $options['height'], $options['max_depth'], false, $options['seperator'], true, $options['all_text'], $this->attr('check_boxes'));
			$replacement = ob_get_contents();
		ob_end_clean();
		return $replacement;

	}//end getStructuredRootNodeKeywordReplacement()


	/**
	* Get the replacement for "select_all_js_code" keyword
	*
	* @return string
	* @access public
	*/
	function getSelectAllJSCodeKeywordReplacement()
	{
		return array_get_index($this->_tmp, 'js_relation_code', '');

	}//end getSelectAllJSCodeKeywordReplacement()


//--        ASSET SELECTION HELPERS        --//


	/**
	* Analyse "check-all" relations recursively
	*
	* @param array	&$todo		assets to list
	* @param array	&$relations	relations
	* @param string	$prefix		prefix
	* @param int	$limit		limit
	*
	* @return void
	* @access private
	*/
	function _recurseCheckAllRelations(&$todo, &$relations, $prefix, $limit=null)
	{
		for (reset($todo); null !== ($k = key($todo)); next($todo)) {
			$el =& $todo[$k];
			if (is_array($el) && (is_null($limit) || ($limit > 0))) {

				$new_rel = Array('parent' => $prefix.$k, 'children' => Array());
				foreach (array_keys($el) as $el_key) {
					if ($limit > 1) {
						$new_rel['children'][] = $prefix.$k.'_'.$el_key;
					} else {
						$new_rel['children'][] = '['.$el_key.']';
					}
				}
				$relations[] = $new_rel;

				$this->_recurseCheckAllRelations($el, $relations, $prefix.$k.'_', (is_null($limit) ? null : $limit - 1));
			}
		}

	}//end _recurseCheckAllRelations()


	/**
	* Kick off analysing "check-all" relations
	*
	* The results of this function are cached in the _tmp member variable as
	* $this->_tmp['check_all_relations'].
	*
	* @param array	$todo	assets to check
	*
	* @return void
	* @access private
	*/
	function _analyseCheckAllRelations($todo)
	{
		if (!isset($this->_tmp['check_all_relations'])) {
			// if custom grouping is enabled and set up, we need to modify the
			// first level of groups to use the 'g' prefix
			if (($this->attr('group_by') == 'grouped') && (count($this->attr('asset_grouping')) > 0)) {
				$new_rel = Array('parent' => '_all', 'children' => Array());
				foreach (array_keys($todo) as $group_id) {
					$new_rel['children'][] = 'g_'.$group_id;
				}
			} else {
				$new_rel = Array('parent' => '_all', 'children' => Array());
				foreach (array_keys($todo) as $child) {
					$new_rel['children'][] = '['.$child.']';

				}
			}
			$relations = Array($new_rel);

			$this->_recurseCheckAllRelations($todo, $relations, 'g_', count($this->attr('asset_grouping')));
			$this->_tmp['check_all_relations'] =& $relations;
		}

		return $this->_tmp['check_all_relations'];

	}//end _analyseCheckAllRelations()


	/**
	* Build 'on click' code to ensure 'select all' functionality works
	*
	* @return void
	* @access private
	*/
	function _buildRelationsJS()
	{
		// order is important, and we actually need the 'ALL' one LAST so we need
		// to reverse this
		$rels = array_reverse($this->_tmp['check_all_relations']);
		$code = 'init_select_list_array(); select_list[\''.$this->getPrefix().'\'] = [';

		$select_lines = Array();
		foreach ($rels as $rel) {
			$parent_code = str_replace(' ', '_', $rel['parent']);
			for (reset($rel['children']); null !== ($k = key($rel['children'])); next($rel['children'])) {
				if (preg_match('|\[(.*)\]|', $rel['children'][$k], $matches)) {
					$rel['children'][$k] = $matches[1];
					$bracketed = true;
				} else {
					$bracketed = false;
				}

				if (is_numeric($rel['children'][$k])) {
					$rel['children'][$k] = str_replace(' ', '_', $rel['children'][$k]);
				} else {
					$rel['children'][$k] = make_valid_html_id(str_replace(' ', '_', $rel['children'][$k]));
				}

				if ($bracketed) {
					$rel['children'][$k] = '['.$rel['children'][$k].']';
				}
			}
			$select_lines[] = "[['".make_valid_html_id($parent_code)."'], ['".implode("', '", $rel['children'])."']]";
		}
		$code .= implode(', ', $select_lines);

		$code .= ']';

		return $code;

	}//end _buildRelationsJS()


	/**
	* Get the current form submit method
	*
	* @return string
	* @access protected
	*/
	function _getFormSubmitMethod()
	{
		return $this->attr('form_submit_method');

	}//end _getFormSubmitMethod()


//--        NO RESULTS BODYCOPY        --//


	/**
	* Creates the no-results bodycopy
	*
	* @param boolean	$enable_on_create	if true, will set the bodycopy to be enabled
	*										(TYPE_2 link) when it is created. If false,
	*										creates it disabled (TYPE_3 link).
	*
	* @return boolean	indicating success (false if bodycopy already exists)
	* @access public
	*/
	function createNoResultsBodycopy($enable_on_create=true)
	{
		$bodycopy_link = reset($GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3, 'bodycopy', false, 'major', 'no_results'));

		// we already have a bodycopy link?!
		if ($bodycopy_link) {
			return false;
		} else {
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');

			$link_type = ($enable_on_create ? SQ_LINK_TYPE_2 : SQ_LINK_TYPE_3);

			$asset =& new Bodycopy();
			$copy_link = Array(
							'asset'			=> &$this,
							'value'			=> 'no_results',
							'link_type'		=> $link_type,
							'is_dependant'	=> 1,
							'is_exclusive'	=> 1,
						 );

			$asset->setAttrValue('name', 'Page Contents (No Results)');
			if (!$asset->create($copy_link)) return false;

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			unset($asset);
		}

		return true;

	}//end createNoResultsBodycopy()


	/**
	* Returns the no-results bodycopy
	*
	* @param boolean	$only_if_enabled	if true, will return null if the bodycopy is
	*										disabled. If false, will only return null if
	*										never created.
	*
	* @return mixed	object|null
	* @access public
	*/
	function &getNoResultsBodycopy($only_if_enabled=true)
	{
		$asset = null;

		if ($only_if_enabled) {
			$link_types = SQ_LINK_TYPE_2;
		} else {
			$link_types = SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3;
		}

		$bodycopy_link = reset($GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, $link_types, 'bodycopy', false, 'major', 'no_results'));

		if ($bodycopy_link) {
			$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link['minorid'], 'bodycopy');
		}

		return $asset;

	}//end getNoResultsBodycopy()


	/**
	* Returns whether the no-results bodycopy is enabled
	*
	* @return boolean
	* @access public
	*/
	function isNoResultsBodycopyEnabled()
	{
		$link_types = SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3;

		$bodycopy_link = reset($GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, $link_types, 'bodycopy', false, 'major', 'no_results'));

		if ($bodycopy_link) {
			return ($bodycopy_link['link_type'] == SQ_LINK_TYPE_2);
		} else {
			return false;
		}


	}//end isNoResultsBodycopyEnabled()


	/**
	* Handles the enabling or disabling of the no-results bodycopy
	*
	* @param boolean	$enabled	set to true to enable the format bodycopy,
	*								set to false to disable it
	*
	* @return boolean	indicating success
	* @access public
	*/
	function setUseNoResultsBodycopy($enabled)
	{
		$bodycopy_link = reset($GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3, 'bodycopy', false, 'major', 'no_results'));

		if (!$bodycopy_link) {
			// no bodycopy yet? If we're trying to set to disabled, then we don't
			// need to do anything - if not then we need to create it
			if ($enabled) {
				if (!$this->createNoResultsBodycopy()) return false;
			}
		} else {
			// set link type to either TYPE_2 if enabled or TYPE_3 if disabled
			$new_link_type = ($enabled) ? SQ_LINK_TYPE_2 : SQ_LINK_TYPE_3;
			if ($bodycopy_link['link_type'] != $new_link_type) {
				$GLOBALS['SQ_SYSTEM']->am->updateLink($bodycopy_link['linkid'], $new_link_type);
			}
		}

		return true;

	}//end setUseNoResultsBodycopy()


}//end class
?>
