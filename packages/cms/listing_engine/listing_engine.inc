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
* $Id: listing_engine.inc,v 1.14 2005/02/16 01:19:32 lwright Exp $
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
* @version $Revision: 1.14 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Listing_Engine extends Page
{
	
	function Listing_Engine($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Page($assetid);
		
	}//end Listing_Engine()
	
	
	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Asset listings create bopycopy and folder asset when they are created
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
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');

		$sub_assets= Array(
			'type_formats'     => 'folder',
			'position_formats' => 'folder',
		);

		$type_formats = null;
		foreach ($sub_assets as $name => $type) {
			$asset = new $type();
			$copy_link = Array('asset' => &$this, 'value' => $name ,'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

			$asset->setAttrValue('name', ucwords(str_replace('_',' ', $name)));
			if (!$asset->create($copy_link)) return false;

			if ($name == 'type_formats') $type_formats = $asset;

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			unset($asset);
		}
		$asset = new Bodycopy();
		$copy_link = Array('asset' => &$type_formats, 'value' => 'default_format' ,'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);
		$asset->setAttrValue('name', 'Default Format');

		return $asset->create($copy_link);

	}//end _createAdditional()
	
	
	/**
	* Returns a list of lock types available for this asset type
	*
	* Only maps Content lock type to the Menu lock type (Attributes|Links) for
	* backward compatibilty purposes with Asset Listing. {@internal If this
	* mapping is not required then this function can be removed}
	*
	* @return Array()
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
	* @param object Asset	$asset			the major or minor asset that we are linking to
	* @param string			$side_of_link	the side of the link we are on (major or minor)
	* @param string			$link_type		the type of link we are creating
	* @param string			$value			the value that is to be associated with the link
	* @param string			$sort_order		the position in the links list that this link should take,
	* @param string			$dependant		'0' / '1' on whether the this asset is dependant on the
	*										asset that will be linked by the new link
	* @param string			$exclusive		'0' / '1' on whether the this asset is linked exclusivly
	*										to the asset that will be linked by the new link
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
				return 'The root node of the Asset Listing';
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
	* @return mixed TRUE or error msg string
	* @access public
	*/
	function isDeletableLink($linkid)
	{
		$folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', true, 'type_formats');
		if ($folder_link['linkid'] == $linkid && !$GLOBALS['SQ_PURGING_TRASH']) return 'You cannot delete the link between the type formats folder and the '.str_replace('_',' ', $this->type()).' page';

		$asset_folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', true, 'position_formats');
		if ($asset_folder_link['linkid'] == $linkid && !$GLOBALS['SQ_PURGING_TRASH']) return 'You cannot delete the link between the asset formats folder and the '.str_replace('_',' ', $this->type()).' page';

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
		$page_links[SQ_LINK_TYPE_2]['folder']   = Array('card' => 'M', 'exclusive' => false);
		$page_links[SQ_LINK_NOTICE]['asset']    = Array('card' => 'M', 'exclusive' => false);
		return $page_links;

	}//end _getAllowedLinks()

	
	/**
	* Return a reference to a folder linked to this asset determined by the passed in value on the link
	*
	* @param string $type the value on the link for this folder
	*
	* @return object Folder
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
	* @access public
	* @return array
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
		// we will have to work out exactly how things are printed with this
		// the common section will probably appear here - then the kids will
		// override - but where will we work out where the assets come from?

	}//end printBody()	
	
	
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
		return $cache_key;

	}//end _getCacheKey()
		
		
	/**
	* Returns keyword replacements to be replaced in the body
	* of the format bodycopy
	*
	* @return Array()
	* @access public
	*/
	function getKeywordReplacements()
	{
		return Array();

	}//end getKeywordReplacements()
	
	
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
	* @param array			$asset_list 	an array of assets to be sorted
	* @param string			$sort_by		the sort order
	* @param boolean		$reverse_sort	whether to reverse the sort order
	* @param array | string	$type_code		list of type_codes to restrict the search
	*
	* @return array
	*/
	function sortAssetList($asset_list, $sort_by, $type_code=Array())
	{
		// this needs to be implemented, possibly by using similar from asset listing
		// but it's likely that sorting will need to be rewritten to take metadata
		// and attributes into account
		if ((!is_array($asset_list)) || (empty($asset_list))) return false;
		if (count($asset_list) == 1) return $asset_list;

		$assetids = array_keys($asset_list);
		$sort_assets = Array();
		$skipped_assets = Array();
		
		// default to a numeric sort until we are proven otherwise
		$sort_numeric = true;
		
		if ($sort_by == '__attr__') {
			// sorting by attributes?
			$sort_by_attr = $this->attr('sort_by_attr');
			foreach($this->attr('types') as $type_code => $value) {
				if (!$value) {		// we aren't inheriting
					$attr_name = array_get_index($sort_by_attr, $type_code, '');
				} else {
					// we have to find our closest ancestor with a sort field
					$type_anc = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($type_code);
					array_unshift($type_anc, $type_code);
					
					foreach($type_anc as $this_type_code) {
						$attr_name = array_get_index($sort_by_attr, $this_type_code, '');
						if (!empty($attr_name)) break;
					}
				}
				
				// get the assetids that may be affected
				$rem_assetids = array_keys($GLOBALS['SQ_SYSTEM']->am->getAssetInfo($assetids, $type_code, !$value));
				
				// do we have an attribute to sort on for this type code?
				// if we don't, we'll have to come back to it as a skipped asset
				if (!empty($attr_name)) {
					if (in_array($value, Array('int', 'float'))) {	// not a number?
						$sort_numeric = false;
					}
					
					$attr_info = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName($attr_name, 'asset', $rem_assetids);
				
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
			$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($assetids, $type_code);
		
			foreach ($assetids as $assetid) {
				$sort_assets[$assetid] = strtolower($asset_info[$assetid][$sort_by]);
			}
			
			if ($sort_by != 'assetid') {
				$sort_numeric = false;
			}
		}
		
		if ($sort_numeric) {
			uasort($sort_assets, create_function('$a,$b','return '.($this->attr('reverse_sort') ? '$b - $a' : '$a - $b').';'));
		} else {
			(!$this->attr('reverse_sort')) ? asort($sort_assets) : arsort($sort_assets);
		}
		
		$sorted_assets = array_keys($sort_assets);
		foreach($sorted_assets as $asset) $assets_to_list[$asset] = $asset_list[$asset];
		
		// add skipped assets to the bottom of the list
		foreach($skipped_assets as $asset) $assets_to_list[$asset] = $asset_list[$asset];
		
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
		// copied over from asset listing
		// is this fine for what we need, or are we expanding this to include
		// specific statuses (as in Search Folder)
		
		
		// first check to see if this is a special user without restrictions
		$user_restrictions = (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin());
		if (!$user_restrictions) return;

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$assetids = array_keys($todo);

		for (reset($assetids); null !== ($k = key($assetids)); next($assetids)) {
			$assetids[$k] = $db->quote($assetids[$k]);
		}

		// get a list of assets that are not live
		$sql = 'SELECT assetid from '.SQ_TABLE_PREFIX.'ast
				WHERE assetid IN ('.implode(',', $assetids).')
				  AND status < '.$db->quote(SQ_STATUS_LIVE);
		$non_live_assets = $db->getCol($sql);
		assert_valid_db_result($non_live_assets);

		foreach ($non_live_assets as $assetid) {
			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $todo[$assetid]);
			if (!$asset->writeAccess()) {
				unset($todo[$assetid]);
			}
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			unset($asset);
		}

		return;

	}//end filterAssetStatuses()

	
	/**
	* Remove assets of unwanted types from the list of assets to print
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
		$wanted_types = $this->attr('types');
		if (empty($wanted_types)) {
			$todo = Array();
			return;
		}

		// changing this to a foreach because it wasn't looping through
		// everything for some reason
		foreach ($todo as $assetid => $type_code) {
			
			if (!isset($wanted_types[$type_code])) {
				$new_type_code = null;
				foreach ($wanted_types as $wt => $inherit) {
					if (!$inherit) continue;
					if (trim($wt) == '') continue;
					$desc = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($wt);
					if (in_array($type_code, $desc)) $new_type_code = $wt;
				}

				if (is_null($new_type_code) || !isset($wanted_types[$new_type_code])) {
					// this type is not required in the results
					unset($todo[$assetid]);
				} else {
					$type_code = $new_type_code;
				}
			}

		}//end for all todo list entries

	}//end filterAssetTypes()


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

		if (!empty($asset_positions)) {
			$adjusted_assetids = array_values($asset_positions);
			$adjusted_type_codes = Array();

			// replace the original todo list because we are going to build another one
			$original_todo = $todo;
			$todo = Array();

			reset($original_todo);
			$key = key($original_todo);

			for ($i = 1; $i <= count($original_todo) + count($asset_positions); $i++) {
				if (!empty($asset_positions[$i])) {
					// this position in the list is to be filled by a specific asset

					// we get the type code either from the $adjusted_type_code array
					// set in the ELSE statement below, or we leave it empty to be
					// filled in when it is found
					$assetid = $asset_positions[$i];
					$todo[$assetid] = array_get_index($adjusted_type_codes, $assetid, '');
				} else {
					// skip any entries that are being moved to specific list positions
					while (in_array($key, $adjusted_assetids)) {
						if (isset($todo[$key])) {
							// we have already inserted this asset into its position
							// so just set its type code correctly
							$todo[$key] = $original_todo[$key];
						} else {
							// we have not yet inserted this asset into its position
							// so keep a record of the type code that can be used when
							// it is inserted
							$adjusted_type_codes[$key] = $original_todo[$key];
						}
						unset($original_todo[$key]);
						$key = key($original_todo);
						if ($key === null) break(2);
					}
					$todo[$key] = $original_todo[$key];
					next($original_todo);
					$key = key($original_todo);
					if ($key === null) break;
				}
			}//end for
		}//end if we have specific asset positions

	}//end adjustAssetPositions()

	
	/**
	* Gets the specified chunk based on group-by and pagination options
	*
	* @param array	&$children		the list of assets that qualify to be
	*								printed
	* @param array	&$replacements	the current replacements array, which will
	*								be added to by this function
	*
	* @return array	the chunk that we are going to work with
	* @access public
	*/
	function &getChunk(&$children, &$replacements, $result_page, $num_per_page, $group_by=null)
	{
		// grouping by number shows a certain number of results per page (eg. 10 assets per page)
		// grouping by letter makes this template an A-Z listing of assets
		// grouping by 'random' means that we grab X random assets
		if (is_null($group_by)) {
			$group_by = $this->attr('group_by');
		}

		switch ($group_by) {
			case 'number' :
				// do it CHUNK Noris style
				if ($num_per_page > 0) $chunks = array_chunk($children, $num_per_page, true);
				else $chunks = Array('0' => $children);
	
				if ($result_page > count($chunks)) $result_page = count($chunks);
				$todo = $chunks[($result_page-1)];

				// previous page link
				if ($result_page <= 1) $replacements['previous_page'] = $this->attr('prev_page_text');
				else $replacements['previous_page'] = '<a href="'.replace_query_string_vars(Array('result_page' => $result_page-1)).'">'.$this->attr('prev_page_text').'</a>';
	
				// next page link
				if ($result_page >= count($chunks)) $replacements['next_page'] = $this->attr('next_page_text');
				else $replacements['next_page'] = '<a href="'.replace_query_string_vars(Array('result_page' => $result_page+1)).'">'.$this->attr('next_page_text').'</a>';
	
				$replacements['current_page'] = (int)$result_page;
				$replacements['total_pages'] = count($chunks);
	
				// list of pages and links to them
				$page_list = '';
				for ($i = 1; $i <= count($chunks); $i++) {
					if ($i == $result_page) $page_list .= " $i ";
					else $page_list .= ' <a href="'.replace_query_string_vars(Array('result_page' => $i)).'">'.$i.'</a> ';
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
				$replacements['previous_page'] = 'Previous';
				if ($result_letter > 65) {
					for ($i = ($result_letter-1); $i >= 65; $i--) {
						if (isset($todo[strtolower(chr($i))])) {
							$replacements['previous_page'] = '<a href="'.replace_query_string_vars(Array('result_page' => chr($i))).'">'.$this->attr('prev_page_text').'</a>';
							break;
						}
					}
				}
	
				// next page link
				$replacements['next_page'] = 'Next';
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
	
				if (isset($todo[strtolower($result_page)])) $todo = $todo[strtolower($result_page)];
				else $todo = Array();
	
			break;
	
			case 'random' :
				// random does things differently, as there is no pagination. But this is a good place
				// to grab the random asset
				if ($this->attr('num_per_page') <= 1) {
					// grab a single asset to display
					$todo = Array();
					$random_asset = array_rand($children);
					$todo[$random_asset] = $children[$random_asset];
				} else if ($this->attr('num_per_page') > count($children)) {
					// if you try to use array_rand with a value higher than the number of elements
					// in the array, it returns nothing. So, we check for that and return only the
					// maximum number of children available
					$random_assets = array_rand($children, count($children));
					if (count($children) == 1) $random_assets = Array($random_assets);
					foreach ($random_assets as $value) $todo[$value] = $children[$value];
				} else {
					// this will return a subset of the children, as defined by the num_per_page
					// attribute. This occurs when the number is less than the total number of
					// children available.
					$random_assets = array_rand($children, $this->attr('num_per_page'));
					foreach ($random_assets as $value) $todo[$value] = $children[$value];
				}
			break;
		}//end switch
		
		return $todo;
		
	}//end getChunk()
	
	
	/**
	* Print the list of assets that we are listing
	*
	* @param array	&$todo	an array of assets to list in the same format as the return
	*						value of getAssetList()
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

		$this->_printAssetList($todo);

	}//end printAssetList()


	/**
	* Called by printAssetList to take care of the layout of the asset list
	*
	* This method ensures the list is printed in columns if required, and keeps
	* track of what position in the list we are printing
	*
	* @param array	&$todo	an array of assets to list in the same format as the return
	*						value of getAssetList()
	*
	* @return void
	* @access private
	* @see printAssetList()
	* @see _printAsset()
	*/
	function _printAssetList($todo)
	{
		$column_layout = $this->attr('column_layout');
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
				$this->_printAsset($assetid, $type_code, $list_position);
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


		if ($contents == '') {

			// our format will be based on type code
		
			if (!isset($this->_tmp['formats'][$type_code])) {
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
							$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bodycopy);
						}
					}
				} else {
					$this->_tmp['is_default'][] = $type_code;
				}
			}//end if type_code format not loaded
			
			if (in_array($type_code, $this->_tmp['is_default'])) {
				if (!$this->_tmp['default_format']) {
					$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->_tmp['type_folder_id'], SQ_LINK_TYPE_2, 'bodycopy', true, 'default_format');
					$bodycopy = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], 'bodycopy');
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
				}
				$contents = $this->_tmp['default_format'];
				$this_needs_asset = $this->_tmp['needs_asset']['default'];
			} else {
				$contents = $this->_tmp['formats'][$type_code];
				$this_needs_asset = $this->_tmp['needs_asset'][$type_code];
			}//end if using default format

		}//end if ($contents == '')

		if ($this_needs_asset) {
			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			if (is_null($asset)) return;
			$keywords = $asset->getAssetKeywords();

			if (preg_match('|%asset_contents%|is', $contents)) {
				ob_start();
					$asset->printBody();
					$keywords['asset_contents']['value'] = ob_get_contents();
				ob_end_clean();
			}
			
			// stuff carried over results body replacements
			
			$keywords['asset_name_linked']['value'] = '<a href="'.$asset->getUrl().'">'.$asset->name.'</a>';
	
			$keywords['asset_short_name_linked']['value'] = '<a href="'.$asset->getUrl().'">'.$asset->short_name.'</a>';
		
			$lineage = '';
			$linked_lineage = '';
			$first = true;
			$found_root = false;
			$parent_asset_ids = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($asset->id));
	
			// searching for the root asset
			$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', false, 'root');
			$root_assetid = empty($root_link) ? 0 : $root_link['minorid'];
	
			foreach ($parent_asset_ids as $parent_asset_id) {
				if ($parent_asset_id == $root_assetid) $found_root = true;
				if (!$found_root) continue;
				if (!$first) {
					$lineage .= $this->attr('lineage_seperator');
					$linked_lineage .= $this->attr('lineage_seperator');
				}
				$first = false;
				$ancestor_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($parent_asset_id);
				$linked_lineage .= '<a href="'.$ancestor_asset->getUrl().'">'.$ancestor_asset->name.'</a>';
				$lineage .= $ancestor_asset->name;
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($ancestor_asset);
			}
			$keywords['asset_lineage']['value'] = $lineage;
			$keywords['asset_lineage_linked']['value'] = $linked_lineage;
			// end results body stuff

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			unset($asset);
		} else {
			$keywords = Array();
			foreach ($this->_tmp['assets_info'][$assetid] as $field => $value) {
				$keywords['asset_'.$field]['value'] = $value;
			}
		}//end if needs asset
		if (preg_match_all('|%asset_metadata_([^%]+)%|is', $contents, $matches)) {
			if (is_null($mm)) $mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
			// get all the metadata keywords for this asset
			$metadata_keywords = $mm->getMetadataFieldValues($assetid, $matches[1]);

			foreach ($metadata_keywords as $field => $value) {
				$keywords['asset_metadata_'.$field]['value'] = $value;
			}
		}

		$keywords['page_href']['value'] = $this->getHref();
		$keywords['result_number']['value'] = $keywords['asset_position']['value'] = (array_get_index($_REQUEST, 'result_page', 1) - 1) * $this->attr('num_per_page') + $list_position;

		foreach ($keywords as $keyword => $info) {
			$contents = preg_replace('|%((<([^>]+?)>)+)?'.$keyword.'((<([^>]+?)>)+)?%|is', '\\1%'.$keyword.'%\\4', $contents);
			$contents = str_replace("%{$keyword}%", $info['value'], $contents);
		}

		echo $contents;

	}//end _printAsset()


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

		$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id, 'bodycopy', true);

		// if we're being handled here (instead of one of our extensions), 
		// it must be from one of our format bodycopies, or something else altogether,
		// so we'll try to find out which of the format types it is.
		$folder = &$this->getFolder();
		if (is_null($folder)) return;

		$type_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($folder->id, SQ_LINK_TYPE_2, 'bodycopy');

		$type_codes = Array();
		foreach ($type_links as $link_info) {
			if (isset($parents[$link_info['minorid']])) {
				$type_codes[] = $link_info['value'];
			}
		}
		if (empty($type_codes)) return;

		$keywords = Array();
		foreach ($type_codes as $type_code) {
			if ($type_code == 'default_format') {
				// special case for the default format where we dont
				// have to do as much processing
				$dummy_asset = new Asset();
				$type_keywords = $dummy_asset->getAssetKeywords(false);
				unset($dummy_asset);
			} else {
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
				$dummy_asset = new $type_code();
				$type_keywords = $dummy_asset->getAssetKeywords(false);
				unset($dummy_asset);
			}

			foreach ($type_keywords as $keyword => $info) {
				$name = ucwords(str_replace('_', ' ', $keyword));
				$keywords[$keyword] = $name;
			}
		}
		$keywords['asset_contents'] = 'Asset Contents';
		$keywords['asset_name_linked'] = 'Asset Name Linked';
		$keywords['asset_short_name_linked'] = 'Asset Short Name Linked';
		$keywords['asset_position'] = 'Asset Position';

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
	* @access public
	* @return array(string => string)
	*/
	function getContentsKeywords()
	{
		$keywords = Array(
							'asset_listing' => 'Asset Listing',
							'previous_page' => 'Previous Page Link',
							'next_page'     => 'Next Page Link',
							'page_list'     => 'Page List',

						);

		return $keywords;

	}//end getContentsKeywords()
	
	
	/**
	* Gets the list of available asset info fields that can be used
	*
	* @return Array(string)
	* @access public
	*/
	function getSortableAssetInfo()
	{
		return Array('assetid','type_code','version','name','short_name',
						'status','languages','charset','force_secure','created',
						'created_userid','updated','updated_userid','published',
						'published_userid');
		
	}
	
}//end class
?>