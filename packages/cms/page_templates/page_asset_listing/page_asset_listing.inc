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
* $Id: page_asset_listing.inc,v 1.33 2004/08/26 07:19:22 tbarrett Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';

/**
* Page_Asset_Listing
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Asset_Listing extends Page
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
		$this->Page($assetid);

	}//end constructor


	/**
	* Create this asset
	*
	* @param array()	&$link	information used to create the initial link
	*
	* @see Asset::create()
	* @return mixed int or false
	* @access public
	*/
	function create(&$link)
	{
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$linkid = parent::create($link);

		if (!$linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		// add a bodycopy to this page when creating
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');

		$sub_assets= Array(
			'type_formats'  => 'folder',
			'asset_formats' => 'folder',
			'page_contents' => 'bodycopy',
		);

		$type_formats = null;
		foreach ($sub_assets as $name => $type) {
			$asset = new $type();
			$copy_link = Array('asset' => &$this, 'value' => $name ,'link_type' => SQ_LINK_TYPE_2, 'dependant' => 1, 'exclusive' => 1);

			$asset->setAttrValue('name', ucwords(str_replace('_',' ', $name)));
			if (!$asset->create($copy_link)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$this->id = 0;
				return false;
			}
			if ($name == 'type_formats') {
				$type_formats = $asset;
			}
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			unset($asset);
		}
		$asset = new Bodycopy();
		$copy_link = Array('asset' => &$type_formats, 'value' => 'default_format' ,'link_type' => SQ_LINK_TYPE_2, 'dependant' => 1, 'exclusive' => 1);
		$asset->setAttrValue('name', 'Default Format');

		if (!$asset->create($copy_link)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
		unset($asset);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return $linkid;

	}//end create()


	/**
	* Returns a list of lock types available for this asset type
	*
	* @return Array()
	* @access public
	*/
	function lockTypes()
	{
		$lock_types = parent::lockTypes();
		$lock_types['content'] = ($lock_types['attributes'] | $lock_types['links']);
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
		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'page_contents');
		if ($bodycopy_link['linkid'] == $linkid && !$GLOBALS['SQ_PURGING_TRASH']) return 'You cannot delete the link between the page contents and the asset listing page';

		$folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', true, 'type_formats');
		if ($folder_link['linkid'] == $linkid && !$GLOBALS['SQ_PURGING_TRASH']) return 'You cannot delete the link between the type formats folder and the asset listing page';

		$asset_folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', true, 'asset_formats');
		if ($asset_folder_link['linkid'] == $linkid && !$GLOBALS['SQ_PURGING_TRASH']) return 'You cannot delete the link between the asset formats folder and the asset listing page';

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
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		// this is what we are grouping by
		// grouping by number shows a certain number of results per page (eg. 10 assets per page)
		// grouping by letter makes this template an A-Z listing of assets
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
			$this->filterAssetList($children);

			// if there are no children available, just print the page bodycopy
			if (empty($children)) {
				$format_bodycopy->printBody();
				return;
			}

			  /////////////////////////
			 //  PAGINATING ASSETS  //
			/////////////////////////
			switch ($group_by) {
				case 'number' :
					$num_per_page = $this->attr('num_per_page');
					$result_page = (isset($_REQUEST['result_page'])) ? (int)$_REQUEST['result_page'] : 1;
					if ($result_page <= 0) $result_page = 1;
	
					// do it CHUNK Noris style
					if ($num_per_page > 0) $chunks = array_chunk($children, $num_per_page, true);
					else $chunks = Array('0' => $children);
	
					if ($result_page > count($chunks)) $result_page = count($chunks);
					$todo = $chunks[($result_page-1)];
	
					// previous page link
					if ($result_page <= 1) $replacements['previous_page'] = $this->attr('prev_page_text');
					else $replacements['previous_page'] = '<a href="'.$this->getURL().'?result_page='.($result_page-1).'">'.$this->attr('prev_page_text').'</a>';
	
					// next page link
					if ($result_page >= count($chunks)) $replacements['next_page'] = $this->attr('next_page_text');
					else $replacements['next_page'] = '<a href="'.$this->getURL().'?result_page='.($result_page+1).'">'.$this->attr('next_page_text').'</a>';
	
					$replacements['current_page'] = (int)$result_page;
					$replacements['total_pages'] = count($chunks);
	
					// list of pages and links to them
					$page_list = '';
					for ($i = 1; $i <= count($chunks); $i++) {
						if ($i == $result_page) $page_list .= " $i ";
						else $page_list .= ' <a href="'.$this->getURL().'?result_page='.$i.'">'.$i.'</a> ';
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
								$replacements['previous_page'] = '<a href="'.$this->getURL().'?result_page='.chr($i).'">'.$this->attr('prev_page_text').'</a>';
								break;
							}
						}
					}
	
					// next page link
					$replacements['next_page'] = 'Next';
					if ($result_letter < 90) {
						for ($i = ($result_letter+1); $i <= 90; $i++) {
							if (isset($todo[strtolower(chr($i))])) {
								$replacements['next_page'] = '<a href="'.$this->getURL().'?result_page='.chr($i).'">'.$this->attr('next_page_text').'</a>';
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
							$page_list .= ' <a href="'.$this->getURL().'?result_page='.chr($i).'">'.chr($i).'</a> ';
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

			ob_start();
				$this->printAssetList($todo);
				$global_contents = ob_get_contents();
			ob_end_clean();

			// if we have any blank keywords, replace them with nothing
			$global_contents = preg_replace('|%[^%\W]+%|', '', $global_contents);
			
			// the global replacement to print the asset listing
			$replacements['asset_listing'] = $global_contents;
	
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
	*						'firt_letter' => first_letter,
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
		if (empty($wanted_types)) return Array();

		// get the root asset whose sub-assets/children we are displaying
		// if there is no root asset, we will use ourselves and print our children
		$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', false, 'root');
		if (empty($root_link)) {
			$root_asset =& $this;
		} else {
			$root_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($root_link['minorid'], $root_link['minor_type_code']);
			if (is_null($root_asset)) return Array();
		}

		// this is what we are going to sort the reults by (passed directly to the getLinks
		// and getChildren functions)
		// must be a column in the asset table (null = no sorting)
		$sort_by = $this->attr('sort_by');
		if (empty($sort_by)) $sort_by = null;


		// if we are showing an A-Z listing but the user has not selected a proper sort field,
		// we will use the 'name' attribute of the asset to work out the listing
		if ($group_by == 'letter' && $sort_by != 'name' && $sort_by != 'short_name') $sort_by = 'name';

		$children = Array();

		if ($this->attr('subs_only')) {

			switch ($group_by) {
				case 'number' :
					$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($root_asset->id, SQ_SC_LINK_BACKEND_NAV, array_keys($wanted_types), false, 'major', null, null, null, $sort_by);
					foreach ($links as $link_info) $children[$link_info['minorid']] = $link_info['minor_type_code'];
					if ($this->attr('reverse_sort')) $children = array_reverse($children, true);
				break;

				case 'letter' :
					$query = $GLOBALS['SQ_SYSTEM']->am->generateGetLinksQuery($root_asset->id, SQ_SC_LINK_BACKEND_NAV, array_keys($wanted_types), false, 'major', null, null, null, $sort_by);
					if (empty($query)) return Array();
					$query['select'] .= ', SUBSTRING(a.'.$sort_by.', 1, 1) AS first_letter';

					$db = &$GLOBALS['SQ_SYSTEM']->db;
					$links = $db->getAll(implode(' ', $query));
					if (DB::isError($children)) {
						trigger_error($children->getMessage().'<br/>'.$children->getUserInfo(), E_USER_ERROR);
						return Array();
					}
					foreach ($links as $link_info) $children[$link_info['minorid']] = Array('type_code' => $link_info['minor_type_code'], 'first_letter' => $link_info['first_letter']);
				break;
				
				case 'random' :
					$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($root_asset->id, SQ_SC_LINK_BACKEND_NAV, array_keys($wanted_types), false, 'major', null, null, null, null);
					foreach ($links as $link_info) $children[$link_info['minorid']] = $link_info['minor_type_code'];
				break;
			}//end switch

		} else {

			switch ($group_by) {
				case 'number' :
					$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($root_asset->id, array_keys($wanted_types), false, $sort_by);
					if ($this->attr('reverse_sort')) $children = array_reverse($children, true);
				break;

				case 'letter' :
					$query = $GLOBALS['SQ_SYSTEM']->am->generateGetChildrenQuery($root_asset, array_keys($wanted_types), false, $sort_by);
					if (empty($query)) return Array();
					$query['select'] .= ', SUBSTRING(a.'.$sort_by.', 1, 1) AS first_letter';

					$db = &$GLOBALS['SQ_SYSTEM']->db;
					$children = $db->getAssoc(implode(' ', $query));
					if (DB::isError($children)) {
						trigger_error($children->getMessage().'<br/>'.$children->getUserInfo(), E_USER_ERROR);
						return;
					}
				break;
				
				case 'random' :
					$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($root_asset->id, array_keys($wanted_types), false, null);
				break;
			}//end switch

		}//end if subs_only

		// clean up after ourselves
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($root_asset);
		
		return $children;

	}//end getAssetList()


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

		  //////////////////////////////
		 //  REMOVE NON-LIVE ASSETS  //
		//////////////////////////////
		if (!empty($todo)) $this->filterAssetStatuses($todo);

		if (empty($todo)) return;
			
		  /////////////////////////////
		 //  REMOVE UNWANTED TYPES  //
		/////////////////////////////
		switch ($group_by) {
			case 'number' :
			case 'random' :
				$this->filterAssetTypes($todo);
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
	* Remove non-live assets from the list of assets to print
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
		$assetids = array_keys($todo);
		
		for (reset($assetids); null !== ($k = key($assetids)); next($assetids)) {
			$assetids[$k] = $db->quote($assetids[$k]);
		}
		
		$filtered_assets = $db->getCol('SELECT assetid
										FROM '.SQ_TABLE_PREFIX.'asset
										WHERE assetid IN ('.implode(',', $assetids).')
										  AND status >= '.$db->quote(SQ_STATUS_LIVE)
										);
		assert_valid_db_result($filtered_assets);
		
		for (reset($todo); null !== ($assetid = key($todo)); ) {
			if (!in_array($assetid, $filtered_assets)) {
				unset($todo[$assetid]);
			} else {
				next($todo);
			}
		}

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

		$past_start = false;
		for (reset($todo); null !== ($assetid = key($todo)); next($todo)) {
			$type_code =& $todo[$assetid];

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
					if ($past_start) prev($todo);
				} else {
					$type_code = $new_type_code;
				}
			}

			$past_start = true;

		}//end for all todo list entries

	}//end filterAssetTypes()


	/**
	* Print the list of assets that we are listing
	*
	* @param array	&$todo	an array of assets to list in the same format as the return
	*						value of getAssetList()
	*
	* @return void
	* @access protected
	* @see getAssetList()
	*/
	function printAssetList($todo)
	{
		$wanted_types = $this->attr('types');
		if (empty($wanted_types)) return;

		$formats         = Array();
		$is_default      = Array();
		$needs_asset	 = Array();
		$assets_info	 = Array();
		$folder          = &$this->getFolder('type_formats');
		$type_folder_id  = $folder->id;
		$default_format  = '';
		$customised      = $this->attr('customised');
		$keywords        = Array();
		$asset_info_fields = Array('assetid','type_code','version','name','short_name','status','languages','charset','force_secure','created','created_userid','updated','updated_userid','published','published_userid'); 
		$mm		 = NULL;

		foreach ($todo as $assetid => $type_code) {

			$contents = '';

			if (!isset($wanted_types[$type_code])) continue;

			if (!in_array($type_code, $formats)) {
				if (in_array($type_code, $customised)) {
					$link = $GLOBALS['SQ_SYSTEM']->am->getLink($type_folder_id, SQ_LINK_TYPE_2, 'bodycopy', true, $type_code);
					// maybe they deleted the bodycopy
					if (!$link) {
						$key = array_search($type_code, $customised, true);
						unset($customised[$key]);
						$is_default[] = $type_code;
					} else {
						$bodycopy = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], 'bodycopy');
						if (!is_null($bodycopy)) {
							// get the format for this type
							$formats[$type_code] = $bodycopy->getRawBodycopyContent();
							// see whether the keywords in it require asset loading or not
							$bc_keywords = retrieve_keywords_replacements($formats[$type_code]);
							$needs_asset[$type_code] = false;
							foreach($bc_keywords as $keyword) {
								if (!(strpos($keyword, 'asset_') === 0) || ((strpos($keyword, 'asset_metadata_') !== 0) && (!in_array(substr($keyword, 6), $asset_info_fields)))) {
									$needs_asset[$type_code] = true;
									break;
								}
							}
							// if we're not going to load the asset, make sure we have assetinfo instead
							if (!$needs_asset[$type_code] && empty($assets_info)) $assets_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($todo));
							$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bodycopy);
						}
					}
				} else {
					$is_default[] = $type_code;
				}
			}// end if

			if (in_array($type_code, $is_default)) {
				if (!$default_format) {
					$link = $GLOBALS['SQ_SYSTEM']->am->getLink($type_folder_id, SQ_LINK_TYPE_2, 'bodycopy', true, 'default_format');
					$bodycopy = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], 'bodycopy');
					if (!is_null($bodycopy)) {
						// get the bodycopy content
						$default_format = $bodycopy->getRawBodycopyContent();
						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bodycopy);
						// see whether the keywords in it require asset loading or not
						$bc_keywords = retrieve_keywords_replacements($default_format);
						$needs_asset['default'] = false;
						foreach($bc_keywords as $keyword) {
							if (!(strpos($keyword, 'asset_') === 0) || ((strpos($keyword, 'asset_metadata_') !== 0) && (!in_array(substr($keyword, 6), $asset_info_fields)))) {
								$needs_asset['default'] = true;
								break;
							}
						}
						// if we're not going to load the asset, make sure we have assetinfo instead
						if (!$needs_asset['default'] && empty($assets_info)) $assets_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($todo));
					}
				}
				$contents = $default_format;
				$this_needs_asset = $needs_asset['default'];
			} else {
				$contents = $formats[$type_code];
				$this_needs_asset = $needs_asset[$type_code];
			}
			if ($this_needs_asset) {
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				if (is_null($asset)) continue;
				$keywords = $asset->getAssetKeywords();
		
				if (preg_match('|%asset_contents%|is', $contents)) {
					ob_start();
						$asset->printBody();
						$keywords['asset_contents']['value'] = ob_get_contents();
					ob_end_clean();
				}
					
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				unset($asset);
			} else {
				$keywords = Array();
				foreach ($assets_info[$assetid] as $field => $value) {
					$keywords['asset_'.$field]['value'] = $value;
				}
			}

			if (preg_match_all('|%asset_metadata_([^%]+)%|is', $contents, $matches)) {
				if (is_null($mm)) $mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
				// get all the metadata keywords for this asset
				$metadata_keywords = $mm->getMetadataFieldValues($assetid, $matches[1]);

				foreach ($metadata_keywords as $field => $value) {
					$keywords['asset_metadata_'.$field]['value'] = $value;
				}
			}

			$keywords['page_href']['value'] = $this->getHref();

			foreach ($keywords as $keyword => $info) {
				$contents = preg_replace('|%((<([^>]+?)>)+)?'.$keyword.'((<([^>]+?)>)+)?%|is', '\\1%'.$keyword.'%\\4', $contents);
				$contents = str_replace("%{$keyword}%", $info['value'], $contents);
			}

			echo $contents;

		}//end foreach todo

	}//end printAssetList()


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
		if (!is_a($broadcaster, 'content_type_wysiwyg')) return;

		$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id, 'bodycopy', true);

		// the broadcaster could be our page contents bodycopy, in which case
		// we want to supply our own keyword replacements
		$page_contents_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'page_contents');
		if (!empty($page_contents_link)) {
			if (in_array($page_contents_link['minorid'], $parents)) {
				$vars['keywords'] += $this->getContentsKeywords();
				return;
			}
		}

		// otherwise, it must be from one of our format bodycopies, or something else altogether,
		// so we'll try to find out which of the format types it is.
		$folder = &$this->getFolder();
		if (is_null($folder)) return;

		$type_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($folder->id, SQ_LINK_TYPE_2, 'bodycopy');

		$type_codes = Array();
		foreach ($type_links as $link_info) {
			if (in_array($link_info['minorid'], $parents)) {
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
				$type_keywords = $dummy_asset->getAssetKeywords(true);
				unset($dummy_asset);
			} else {
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
				$dummy_asset = new $type_code();
				$type_keywords = $dummy_asset->getAssetKeywords(true);
				unset($dummy_asset);
			}

			foreach ($type_keywords as $keyword => $info) {
				if (empty($info['description'])) $info['description'] = ucfirst(str_replace('_', ' ', $keyword));
				$name = ucwords(str_replace('_', ' ', $keyword));
				$keywords[$keyword] = Array('name' => $name, 'default' => $info['description'], 'type' => 'inline');
			}
		}
		$keywords['asset_contents'] = Array('name'    => 'Asset Contents',
											'default' => 'Asset Contents',
											'type'    => 'block'
											);

		$vars['keywords'] = array_merge($vars['keywords'], $keywords);

	}//end onRequestKeywords()


	/**
	* Return the available keywords in the Page Contents Bodycopy for this asset
	*
	* The return value is in the form:
	* <pre>
	* Array(
	*     'keyword' => Array (
	*					'name'	=> '(name of the keyword)',
	*				    'default' => '(default value)',
	*					'type' => 'block' | 'inline'
	*                  )
	* )
	* </pre>
	*
	* @param boolean $descriptions if TRUE descriptions will be included in the array
	*
	* @access public
	* @return Array(mixed)
	*/
	function getContentsKeywords()
	{
		$keywords = Array();

		$keywords['asset_listing'] =	Array(
											'name'=>'Asset Listing',
											'default'=>'The Listing of Assets', 
											'type'=>'block');

		$keywords['previous_page'] =	Array(
											'name'=>'Previous Page Link',
											'default'=>'Link to the previous page of the listing', 
											'type'=>'inline');
			
		$keywords['next_page'] =		Array(
											'name'=>'Next Page Link',
											'default'=>'Link to the next page of the listing', 
											'type'=>'inline');
			
		$keywords['page_list'] =		Array(
											'name'=>'Page List',
											'default'=>'List of pages in the listing', 
											'type'=>'inline');

		return $keywords;
	}


}//end class

?>
