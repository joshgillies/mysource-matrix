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
* $Id: page_asset_listing.inc,v 1.6 2004/02/02 00:32:51 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../page/page.inc';

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
	* Creates a link between this asset and the passed asset, of a certain type
	* and returns the linkid of the created link
	*
	* @return int
	* @access public
	* @see Asset::createLink();
	*/
	function createLink(&$minor, $link_type, $value='', $sort_order=-1, $dependant='0', $exclusive='0')
	{
		// if this is a bodycopy table then we need to make it a dependant link
		if (is_a($minor, 'bodycopy')) {
			$dependant = '1';
		}

		return parent::createLink($minor, $link_type, $value, $sort_order, $dependant, $exclusive);

	}//end createLink()


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
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', true, $type);
		if (empty($link)) return null;

		$folder = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		if (is_null($folder)) return null;

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
		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'page_contents');
		if (empty($bodycopy_link)) return false;

		$format_bodycopy = &$GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link['minorid'], $bodycopy_link['minor_type_code']);
		if (is_null($format_bodycopy)) return false;


		// this is the list of all type codes that we are going to show
		// we pass this list to the getLinks and getChildren functions (below) to narrow
		// the query down a bit
		// if there are no types to list, we cant list anything
		$wanted_types = $this->attr('types');
		if (empty($wanted_types)) return;


		// get the root asset whose sub-assets/children we are displaying
		// if there is no root asset, we will use ourselves and print our children
		$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', false, 'root');
		if (empty($root_link)) {
			$root_asset =& $this;
		} else {
			$root_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($root_link['minorid'], $root_link['minor_type_code']);
			if (is_null($root_asset)) return;
		}


		// this is what we are going to sort the reults by (passed directly to the getLinks
		// and getChildren functions)
		// must be a column in the asset table (null = no sorting)
		$sort_by = $this->attr('sort_by');
		if (empty($sort_by)) $sort_by = null;


		// this is what we are grouping by
		// grouping by number shows a certain number of results per page (eg. 10 assets per page)
		// grouping by letter makes this template an A-Z listing of assets
		$group_by = $this->attr('group_by');


		// if we are showing an A-Z listing but the user has not selected a proper sort field,
		// we will use the 'name' attribute of the asset to work out the listing
		if ($group_by == 'letter' && $sort_by != 'name' && $sort_by != 'short_name') $sort_by = 'name';


		  //////////////////////////////////////////////
		 //  GET THE LIST OF ASSETS WE ARE PRINTING  //
		//////////////////////////////////////////////
		$children = Array();

		if ($this->attr('subs_only')) {

			switch ($group_by) {
				case 'number' :
					$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($root_asset->id, SQ_SC_LINK_BACKEND_NAV, $wanted_types, false, 'major', null, null, null, $sort_by);
					foreach ($links as $link_info) $children[$link_info['minorid']] = $link_info['minor_type_code'];
					if ($this->attr('reverse_sort')) $children = array_reverse($children, true);
				break;

				case 'letter' :
					$query = $GLOBALS['SQ_SYSTEM']->am->generateGetLinksQuery($root_asset->id, SQ_SC_LINK_BACKEND_NAV, $wanted_types, false, 'major', null, null, null, $sort_by);
					if (empty($query)) return;
					$query['select'] .= ', SUBSTRING(a.'.$sort_by.', 1, 1) AS first_letter';

					$db = &$GLOBALS['SQ_SYSTEM']->db;
					$links = $db->getAll(implode(' ', $query));
					if (DB::isError($children)) {
						trigger_error($children->getMessage().'<br/>'.$children->getUserInfo(), E_USER_ERROR);
						return;
					}
					foreach ($links as $link_info) $children[$link_info['minorid']] = Array('type_code' => $link_info['minor_type_code'], 'first_letter' => $link_info['first_letter']);
				break;
			}//end switch

		} else {

			switch ($group_by) {
				case 'number' :
					$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($root_asset->id, $wanted_types, false, $sort_by);
					if ($this->attr('reverse_sort')) $children = array_reverse($children, true);
				break;

				case 'letter' :
					$query = $GLOBALS['SQ_SYSTEM']->am->generateGetChildrenQuery($root_asset->id, $wanted_types, false, $sort_by);
					if (empty($query)) return;
					$query['select'] .= ', SUBSTRING(a.'.$sort_by.', 1, 1) AS first_letter';

					$db = &$GLOBALS['SQ_SYSTEM']->db;
					$children = $db->getAssoc(implode(' ', $query));
					if (DB::isError($children)) {
						trigger_error($children->getMessage().'<br/>'.$children->getUserInfo(), E_USER_ERROR);
						return;
					}
				break;
			}//end switch

		}//end if subs_only


		// clean up after ourselves
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($root_asset);
		if (empty($children)) return false;


		  ////////////////////////////////////////////////////////////
		 //  STUFF WE ARE GOING TO NEED TO MAKE THIS MAGIC HAPPEN  //
		////////////////////////////////////////////////////////////
		$customised			= $this->attr('customised');			// the array of asset types with customised formats
		$keywords			= Array();								// an array of keyword replacements per asset base
		$formats			= Array();								// an array of customised format bodycopies
		$default_format		= '';									// the default format html
		$is_default			= Array();								// array of types using the default format
		$folder				= &$this->getFolder('type_formats');	// the folder asset for type formats
		$type_folder_id		= $folder->id;							// the format types folder id
		$global_contents	= '';									// the entire contents of the SAL
		$mm					= null;									// metadata manager
		$replacements		= Array();								// global keyword replacements


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
				if ($result_page <= 1) $replacements['previous_page'] = 'Previous';
				else $replacements['previous_page'] = '<a href="'.$this->getURL().'?result_page='.($result_page-1).'">Previous</a>';

				// next page link
				if ($result_page >= count($chunks)) $replacements['next_page'] = 'Next';
				else $replacements['next_page'] = '<a href="'.$this->getURL().'?result_page='.($result_page+1).'">Next</a>';

				// list of pages and links to them
				$page_list = '';
				for ($i = 1; $i <= count($chunks); $i++) $page_list .= ' <a href="'.$this->getURL().'?result_page='.$i.'">'.$i.'</a> ';
				$replacements['page_list'] = $page_list;

				unset($chunks);
			break;

			case 'letter' :
				foreach ($children as $childid => $child_data) {
					$todo[strtolower($child_data['first_letter'])][$childid] = $child_data;
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
							$replacements['previous_page'] = '<a href="'.$this->getURL().'?result_page='.chr($i).'">Previous</a>';
							break;
						}
					}
				}

				// next page link
				$replacements['next_page'] = 'Next';
				if ($result_letter < 90) {
					for ($i = ($result_letter+1); $i <= 90; $i++) {
						if (isset($todo[strtolower(chr($i))])) {
							$replacements['next_page'] = '<a href="'.$this->getURL().'?result_page='.chr($i).'">Next</a>';
							break;
						}
					}
				}

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

				$todo = $todo[strtolower($result_page)];

			break;

		}//end switch


		// clean up after ourselves
		unset($children);

		  ////////////////////////////
		 //  PRINT THE ASSET LIST  //
		////////////////////////////
		foreach ($todo as $assetid => $type_code) {

			$contents = ''; // temp contents for this asset

			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			if (is_null($asset)) continue;

			// we need to work out if this child has an explicit format defined for it
			// or if its relying on its ansestor's format
			if (!in_array($type_code, $wanted_types)) {
				foreach ($wanted_types as $wt) {
					if (trim($wt) == '') continue;
					$desc = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($wt);
					if (in_array($type_code, $desc)) $type_code = $wt;
				}
			}

			$keywords = $asset->getAssetKeywords();
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
							$formats[$type_code] = $bodycopy->getRawBodycopyContent();
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
						$default_format = $bodycopy->getRawBodycopyContent();
						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bodycopy);
					}
				}
				$contents = $default_format;
			} else {
				$contents = $formats[$type_code];
			}

			if (preg_match_all('|%asset_metadata_([^%]+)%|is', $contents, $matches)) {
				if (is_null($mm)) $mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
				// get all the metadata keywords for this asset
				$replacements = $mm->getMetadataFieldValues($assetid, $matches[1]);

				foreach ($replacements as $field => $value) {
					$keywords['asset_metadata_'.$field]['value'] = $value;
				}
			}

			if (preg_match('|%asset_contents%|is', $contents)) {
				ob_start();
					$asset->printBody();
					$keywords['asset_contents']['value'] = ob_get_contents();
				ob_end_clean();
			}

			foreach ($keywords as $keyword => $info) {
				$contents = preg_replace('|%((<([^>]+?)>)+)?'.$keyword.'((<([^>]+?)>)+)?%|is', '\\1%'.$keyword.'%\\4', $contents);
				$contents = str_replace("%{$keyword}%", $info['value'], $contents);
			}

			$global_contents .= $contents;

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			unset($asset);

		}//end foreach


		// if we have any blank keywords, replace them with nothing
		$global_contents = preg_replace('|%[^%\W]+%|', '', $global_contents);


		// the global replacement to print the asset listing
		$replacements['asset_listing'] = $global_contents;


		// print the contents of page - replacing the global keywords
		$format_bodycopy->setKeywordReplacements($replacements);
		$format_bodycopy->printBody();

		// clean up after ourselves
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bodycopy);

	}//end printBody()


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

		// lets work out which format types the WYSIWYG cell that wants our keywords
		// is in so we can return the correct keywords for the asset type
		$folder = &$this->getFolder();
		if (is_null($folder)) return;

		$type_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($folder->id, SQ_LINK_TYPE_2, 'bodycopy');
		$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id, 'bodycopy', true);

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


}//end class

?>
