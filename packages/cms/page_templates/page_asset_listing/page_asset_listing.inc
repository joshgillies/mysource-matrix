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
* $Id: page_asset_listing.inc,v 1.60 2005/02/14 23:50:25 lwright Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/listing_engine/listing_engine.inc';

/**
* Page_Asset_Listing
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.60 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Asset_Listing extends Listing_Engine
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Page_Asset_Listing($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Listing_Engine($assetid);

	}//end constructor


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Creates the page contents bodycopy 
	*
	* @param array	&$link	information used to create the initial link
	*
	* @access private
	* @return boolean
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return false;

		// add a bodycopy to this page when creating
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		
		$asset = new Bodycopy();
		$copy_link = Array('asset' => &$this, 'value' => 'page_contents' ,'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);
		$asset->setAttrValue('name', 'Page Contents');

		return $asset->create($copy_link);

	}//end _createAdditional()


	/**
	* Returns TRUE if we can delete the passed link, or a string with the error msg
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return mixed TRUE or error msg string
	* @access public
	*/
	function isDeletableLink($linkid)
	{
		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'page_contents');
		if ($bodycopy_link['linkid'] == $linkid && !$GLOBALS['SQ_PURGING_TRASH']) return 'You cannot delete the link between the page contents and the asset listing page';

		return parent::isDeletableLink($linkid);

	}//end isDeletableLink()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return Array()
	* @access private
	* @see Asset::_getAllowedLinks()
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_TYPE_2]['bodycopy'] = Array('card' => 1,   'exclusive' => true);
		return $page_links;

	}//end _getAllowedLinks()
	
	
	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		// we need a unique cache key to represent the current result page
		$cache_key = $this->_getCacheKey();

		$cached_contents = '';
		if (!empty($cache_key)) {
			$cm = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
			$cached_contents = $cm->loadFromCache($this, $cache_key);
		}

		// if the cache is empty, go ahead and regenerate a new version
		if (empty($cached_contents)) {

			$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'page_contents');
			if (empty($bodycopy_link)) return false;

			$format_bodycopy = &$GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link['minorid'], $bodycopy_link['minor_type_code']);
			if (is_null($format_bodycopy)) return false;

			require_once SQ_FUDGE_PATH.'/general/text.inc';
			$replacements = retrieve_keywords_replacements($format_bodycopy->getRawBodycopyContent());
			if (!in_array('asset_listing', $replacements)) {
				// there is no asset listing keyword in the contents, so we dont
				// need to do all the extra processing for the listing
				$format_bodycopy->printBody();
				return;
			}

			$children = $this->getAssetList();
			$num_kids = count($children);
			$this->filterAssetList($children);
			
			// if there are no children available, just print the page bodycopy
			if (empty($children)) {
				$replacements = $this->getKeywordReplacements();
				$replacements['asset_count'] = $num_kids;
				$format_bodycopy->setKeywordReplacements($replacements);
				$format_bodycopy->printBody();
				return;
			}

			$num_per_page = $this->attr('num_per_page');
			$result_page = (isset($_REQUEST['result_page'])) ? (int)$_REQUEST['result_page'] : 1;
			if ($result_page <= 0) $result_page = 1;
				
			// get our page chunk here
			$todo =& $this->getChunk($children, $replacements, $result_page, $num_per_page);
			$num_assets_showing = count($todo);

			ob_start();
				$this->printAssetList($todo);
				$global_contents = ob_get_contents();
			ob_end_clean();

			// if we have any blank keywords, replace them with nothing
			$global_contents = preg_replace('|%[^%\W]+%|', '', $global_contents);

			// the global replacement to print the asset listing
			$replacements['asset_listing'] = $global_contents;
			$replacements['asset_count'] = $num_kids;
			$replacements['page_asset_count'] = $num_assets_showing;
			
			$other_replacements = $this->getKeywordReplacements();
			$replacements = array_merge($replacements, $other_replacements);

			// print the contents of page - replacing the global keywords
			$format_bodycopy->setKeywordReplacements($replacements);
			ob_start();
				$format_bodycopy->printBody();
				if (!empty($cache_key)) $cm->saveToCache($this, $cache_key, ob_get_contents());
			ob_end_flush();

		} else {

			// the contents have already been cached, so just output them
			echo $cached_contents;

		}//end if no cache

		// clean up after ourselves
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bodycopy);

	}//end printBody()
	
	
	/**
	* Get the list of asset that should be printed
	*
	* The return array is in the form Array(assetid => type_code) unless we are grouping by letter,
	* in which case the return array is
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
		$group_by = $this->attr('group_by');

		// this is the list of all type codes that we are going to show
		// we pass this list to the getLinks and getChildren functions (below) to narrow
		// the query down a bit
		// if there are no types to list, we cant list anything
		$wanted_types = $this->attr('types');
		if (empty($wanted_types)) {
			trigger_error('No type settings have been configured for this page - no assets will be listed', E_USER_NOTICE);
			return Array();
		}

		// get the root assets whose sub-assets/children we are displaying
		// This may come from the parameter map, but the dynamic value must be a child of the static value
		// if there are no root assets, we will use ourselves and print our children
		$root_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE, '', false, 'major', 'root');
		foreach ($root_links as $root_link) $root_asset_ids[] = (int)$root_link['minorid'];

		$parameter_map = &$this->getAttribute('parameter_map');
		$dynamic_root = $parameter_map->getParameterValue('root_node');
		
		// check if we have a dynamic parameter -- if so, let's do some security checks
		if (!is_null($dynamic_root)) {
			$dynamic_parents = $GLOBALS['SQ_SYSTEM']->am->getParents($dynamic_root);
			
			// first, check to see if the dynamic parameter specified is already one of the root nodes
			// if so, just use that single root node
			if (in_array($dynamic_root, $root_asset_ids)) {
				unset($root_asset_ids);
				$root_asset_ids[] = $dynamic_root;
			}
			
			// otherwise, check to see if the dynamic parameter specified is a child of one of the root
			// nodes. If so, use the dynamic parameter as the root node; otherwise return an error.
			$matching_root_ids = array_intersect(array_keys($dynamic_parents), $root_asset_ids);
			if (empty($matching_root_ids)) {
				trigger_error('Supplied root node is in an unauthorised part of the system.  Defaulting to static root node', E_USER_WARNING);
			} else {
				unset($root_asset_ids);
				$root_asset_ids[] = $dynamic_root;
			}
		}
		
		// this is what we are going to sort the reults by (passed directly to the getLinks
		// and getChildren functions)
		// must be a column in the asset table (null = no sorting)
		$sort_by = $this->attr('sort_by');
		if (empty($sort_by)) $sort_by = null;


		// if we are showing an A-Z listing but the user has not selected a proper sort field,
		// we will use the 'name' attribute of the asset to work out the listing
		if (($group_by == 'letter') && ($sort_by != 'name') && ($sort_by != 'short_name')) $sort_by = 'name';
		
		// if the sort by is set to use an attribute, sort by name at this
		// stage
		$assets_to_list = Array();

		if ($this->attr('subs_only')) {

			$side_of_link = ($this->attr('direction') == 'down') ? 'major' : 'minor';
			$link_type = ($side_of_link == 'major') ? 'minor' : 'major';
			$links = Array();

			switch ($group_by) {
				case 'number' :
					foreach ($root_asset_ids as $root_asset_id) {
						$links = array_merge($links, $GLOBALS['SQ_SYSTEM']->am->getLinks($root_asset_id, SQ_SC_LINK_BACKEND_NAV, array_keys($wanted_types), false, $side_of_link, null, null, null, (substr($sort_by,0,2) == '__' ? 'name' : $sort_by), SQ_PERMISSION_READ));
					}
					foreach ($links as $link_info) $assets_to_list[$link_info[$link_type.'id']] = $link_info[$link_type.'_type_code'];
					$assets_to_list = $this->sortAssetList($assets_to_list, $sort_by, array_keys($wanted_types));
				break;

				case 'letter' :
					foreach ($root_asset_ids as $root_asset_id) {
						$query = $GLOBALS['SQ_SYSTEM']->am->generateGetLinksQuery($root_asset_id, SQ_SC_LINK_BACKEND_NAV, array_keys($wanted_types), false, $side_of_link, null, null, null, $sort_by, SQ_PERMISSION_READ);
						if (empty($query)) return Array();
						$query['select'] .= ', SUBSTRING(a.'.$sort_by.', 1, 1) AS first_letter';

						$db = &$GLOBALS['SQ_SYSTEM']->db;
						$new_links = $db->getAll(implode(' ', $query));
						assert_valid_db_result($new_links);
						$links = array_merge($links, $new_links);
					}
					foreach ($links as $link_info) $assets_to_list[$link_info[$link_type.'id']] = Array('type_code' => $link_info[$link_type.'_type_code'], 'first_letter' => $link_info['first_letter']);
				break;

				case 'random' :
					foreach ($root_asset_ids as $root_asset_id) {
						$links = array_merge($links, $GLOBALS['SQ_SYSTEM']->am->getLinks($root_asset_id, SQ_SC_LINK_BACKEND_NAV, array_keys($wanted_types), false, $side_of_link, null, null, null, null, SQ_PERMISSION_READ));
					}
					foreach ($links as $link_info) $assets_to_list[$link_info[$link_type.'id']] = $link_info[$link_type.'_type_code'];
				break;
			}//end switch

		} else {

			switch ($group_by) {
				case 'number' :
					if ($this->attr('direction') == 'down') {
						foreach ($root_asset_ids as $root_asset_id) {
							$assets_to_list = $assets_to_list + $GLOBALS['SQ_SYSTEM']->am->getChildren($root_asset_id, array_keys($wanted_types), false, (substr($sort_by,0,2) == '__' ? 'name' : $sort_by), SQ_PERMISSION_READ);
						}
					} else {
						foreach ($root_asset_ids as $root_asset_id) {
							$assets_to_list = $assets_to_list + $GLOBALS['SQ_SYSTEM']->am->getParents($root_asset_id, array_keys($wanted_types), false, (substr($sort_by,0,2) == '__' ? 'name' : $sort_by), SQ_PERMISSION_READ);
						}
					}
					$assets_to_list = $this->sortAssetList($assets_to_list, $sort_by);
				break;

				case 'letter' :
					foreach ($root_asset_ids as $root_asset_id) {
						if ($this->attr('direction') == 'down') {
							$root_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($root_asset_id);
							$query = $GLOBALS['SQ_SYSTEM']->am->generateGetChildrenQuery($root_asset, array_keys($wanted_types), false, $sort_by, SQ_PERMISSION_READ);
							$GLOBALS['SQ_SYSTEM']->am->forgetAsset($root_asset);
						} else {
							$query = $GLOBALS['SQ_SYSTEM']->am->generateGetParentsQuery($root_asset_id, array_keys($wanted_types), false, $sort_by, SQ_PERMISSION_READ);
						}
						if (empty($query)) return Array();
						$query['select'] .= ', SUBSTRING(a.'.$sort_by.', 1, 1) AS first_letter';
						$db = &$GLOBALS['SQ_SYSTEM']->db;
						$new_assets_to_list = $db->getAssoc(implode(' ', $query));
						assert_valid_db_result($new_assets_to_list);
						$assets_to_list = $assets_to_list + $new_assets_to_list;
					}
				break;

				case 'random' :
					if ($this->attr('direction') == 'down') {
						foreach ($root_asset_ids as $root_asset_id) {
							$assets_to_list = $assets_to_list + $GLOBALS['SQ_SYSTEM']->am->getChildren($root_asset_id, array_keys($wanted_types), false, null, SQ_PERMISSION_READ);
						}
					} else {
						foreach ($root_asset_ids as $root_asset_id) {
							$assets_to_list = $assets_to_list + $GLOBALS['SQ_SYSTEM']->am->getParents($root_asset_id, array_keys($wanted_types), false, null, SQ_PERMISSION_READ);
						}
					}
				break;
			}//end switch

		}//end if subs_only

		return $assets_to_list;

	}//end getAssetList()


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
		if (!isset($vars['keywords'])) return;
		if (!is_a($broadcaster, 'content_type')) return;
		
		$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id, 'bodycopy', true);

		// the broadcaster could be our page contents bodycopy, in which case
		// we want to supply our own keyword replacements
		$page_contents_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'page_contents');
		if (!empty($page_contents_link)) {
			if (isset($parents[$page_contents_link['minorid']])) {
				$vars['keywords'] += $this->getContentsKeywords();
				return;
			}
		}
		
		// if it's not the page contents bodycopy, it's something that we've
		// inherited from the listing engine, so pass it on
		parent::onRequestKeywords($broadcaster, $vars);
		
	} //end onRequestKeywords()


}//end class

?>
