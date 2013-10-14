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
* $Id: page_multiple_page.inc,v 1.33 2013/10/14 04:23:16 cupreti Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';

/**
* Page_Multiple_Page
*
* Purpose
*
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.33 $
* @package MySource_Matrix_Packages
* @subpackage CMS
*/
class Page_Multiple_Page extends Page
{


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
	* Returns an array of all the permitted link types, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_TYPE_2]['page_multiple_page_page'] = Array('card' => 'M', 'exclusive' => TRUE);
		return $page_links;

	}//end _getAllowedLinks()


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
		if (($side_of_link == 'major') && ($asset instanceof Page_Multiple_Page_Page)) {
			if (($link_type != SQ_LINK_TYPE_2) || ($exclusive != TRUE)) {
				$link_type = SQ_LINK_TYPE_2;
				$exclusive = TRUE;
				return TRUE;
			}
		}
		return FALSE;

	}//end prepareLink()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		// This should never be called, unless there are no pages linked underneath this.
		// printFrontend will divert the attention of the output to a child asset before this is hit

		echo translate('currently_no_pages_to_display');

	}//end printBody()


	/**
	* Called by the child pages, when their url is accessed
	*
	* @param object	&$asset	The asset object of the page to print
	*
	* @return void
	* @access public
	*/
	function printPage(&$asset)
	{
		$this->_tmp['current_page_asset'] = &$asset;

		$keywords = $asset->getKeywords();

		$replacements = Array();

		foreach ($keywords as $value) {
			$replacements[$value] = $this->getKeywordReplacement($value);
		}

		$asset->setKeywordReplacements($replacements);

		$content = $asset->getContent();
		echo $content;

	}//end printPage()


	/**
	* Returns the raw content of this asset
	*
	* @return string
	* @access public
	*/
	function getContent()
	{
		$contents = translate('cms_page_multiple_page_get_content_msg');
		return $contents;

	}//end getContent()


	/**
	* Returns a list of lock types available for this asset type
	*
	* @return array
	* @access public
	*/
	function lockTypes()
	{
		$lock_types = parent::lockTypes();
		$lock_types['content'] = ($lock_types['attributes'] | $lock_types['links']);
		return $lock_types;

	}//end lockTypes()


	/**
	* Return any locks that are needed for editing, based on a set of custom keywords
	*
	* @param array	$keywords	the keywords to govern what locks are required
	*
	* @return array
	* @access public
	*/
	function getEditingLocks($keywords)
	{
		$locks = Array();
		if (!is_array($keywords) || empty($keywords)) {
			return Array();
		}
		if (!in_array('contents', $keywords)) return Array();
		return Array('content');

	}//end getEditingLocks()


	/**
	* Get the number of pages we currently have
	*
	* @return int
	* @access public
	* @see Asset::processCustomKeywords()
	*/
	function getNumberofPages()
	{
		return count($this->getPageLinks());

	}//end getNumberofPages()


	/**
	* Get links to all the page children of this asset
	*
	* @return int
	* @access public
	* @see Asset::processCustomKeywords()
	*/
	function getPageLinks()
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$page_links = $am->getLinks($this->id, SQ_LINK_TYPE_2|SQ_LINK_TYPE_1, 'page_multiple_page_page', TRUE);
		return $page_links;

	}//end getPageLinks()


	/**
	* Get a multiple page page that resides underneath us
	*
	* @param int	$page_number	The number of page you wish to retrieve
	*
	* @return object
	* @access public
	*/
	function &getPageAsset($page_number)
	{
		$page_asset = NULL;
		$links = $this->getPageLinks();
		foreach ($links as $link_data) {
			if ($link_data['sort_order'] == strval(intval($page_number) - 1)) {
				$page_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
				return $page_asset;
			}
		}
		return $page_asset;

	}//end getPageAsset()


	/**
	* Get the IDs of all the pages that reside underneath
	*
	* @return object
	* @access public
	*/
	function getPageIds()
	{
		$page_links = $this->getPageLinks();
		$ids = Array();
		$cmp = create_function('$a,$b', 'return $a[\'sort_order\'] < $b[\'sort_order\'] ? -1 : 1;');

		// Sort the links, so that the pages appear in the backend, in the same order as they appear in the aset map
		usort($page_links, $cmp);
		foreach ($page_links as $id => $data) {
			$ids[] = $data['minorid'];
		}
		return $ids;

	}//end getPageIds()


	/**
	* Allow any required processesing to occur when any link is updated for the asset
	*
	* @return void
	* @access protected
	*/
	function linksUpdated()
	{
		// This is bugging out so we can delay the updating of our web paths
		if (!empty($this->_tmp['deleting_links'])) {
			return TRUE;
		}
		$current_children = array_flip($this->attr('current_page_ids'));

		$am = $GLOBALS['SQ_SYSTEM']->am;

		// Work out new ordering and web paths
		$new_page_children = Array();
		$page_links = $this->getPageLinks();
		$new_paths = Array();
		foreach ($page_links as $data) {
			$new_page_children[] = $data['minorid'];
			$page = $am->getAsset($data['minorid']);
			unset($current_children[$data['minorid']]);
			$new_paths[$data['minorid']] = $this->getPageNewWebpaths($page, strval(intval($data['sort_order']) + 1));
			if (!empty($this->_tmp['no_sort_order_paths'])) {
				$page->saveWebPaths(Array());
			}
			$am->forgetAsset($page);
		}

		// whatever is left in current_children now are the deleted children
		foreach (array_keys($current_children) as $id) {
			$asset_to_delete_paths_from = $am->getAsset($id);
			if (!is_null($asset_to_delete_paths_from)) {
				$asset_to_delete_paths_from->saveWebPaths(Array());
				$am->forgetAsset($asset_to_delete_paths_from);
			}
		}

		parent::linksUpdated();

		if($this->getURL() == '') {
			// Bug fix #3659 'Multiple Page' cloning loses 'Multiple Page Page' web paths
			// force assign the url to the newly cloned asset as next block will try to 
			// assign webpath to child pages, but if there is no URL on the parent asset
			// then child assets wont have current URL.
			$valid_paths = make_valid_web_paths(Array(strtolower($this->name)));
			$good_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($this, $valid_paths, $this->id, TRUE);
			$this->saveWebPaths($good_paths, FALSE);
		}

		// when trying to rearrange we specifically dont want to run the following code because of 
		// bug #4845 'Multiple Page' asset throws MySource Warning when trying to re-arrange the pages
		// after re-arranging the pages, adding or deleting new pages linksUpdate() is called again 
		// which is when the following block of code should be executed
		if ((isset($this->_tmp['no_sort_order_paths']) && $this->_tmp['no_sort_order_paths'] ) && (isset($new_paths) && !empty($new_paths))) {
			foreach ($new_paths as $pageid => $new_webpaths) {
				$page = $am->getAsset($pageid, '', TRUE);
				if (!empty($page)) {
					$page->saveWebPaths($new_webpaths);
					$am->forgetAsset($page);
				}
			}
		}

		// This will store all current pages,
		$GLOBALS['SQ_SYSTEM']->setRunLevel($GLOBALS['SQ_SYSTEM']->getRunLevel() - SQ_SECURITY_PERMISSIONS - SQ_SECURITY_LOCKING);
		$this->setAttrValue('current_page_ids', $new_page_children);
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

	}//end linksUpdated()


	/**
	* Return the new webpaths for a page under this asset
	* It will remove all numeric paths and if there are no paths left it will set it to $order
	*
	* @param object	&$page	a reference to an asset
	* @param int	$order	a number
	*
	* @return array
	* @access protected
	*/
	function getPageNewWebpaths(&$page, $order)
	{
		$webpaths = $page->getWebPaths();

		foreach ($webpaths as $idx => $webpath) {
			if (is_numeric($webpath)) unset($webpaths[$idx]);
		}

		if (empty($webpaths)) $webpaths = Array($order);

		return $webpaths;

	}//end getPageNewWebpaths()


	/**
	* Return the keywords for this asset type.
	*
	* @return array
	* @access public
	*/
	function getAvailableKeywords()
	{
		$keywords = parent::getAvailableKeywords();

		$keywords['previous_page_href'] = translate('cms_page_multiple_page_previous_page_href');
		$keywords['next_page_href'] = translate('cms_page_multiple_page_next_page_href');
		$keywords['previous_page_name'] = translate('cms_page_multiple_page_previous_page_name');
		$keywords['next_page_name'] = translate('cms_page_multiple_page_next_page_name');
		$keywords['previous_page_link'] = translate('cms_page_multiple_page_previous_page_link');
		$keywords['next_page_link'] = translate('cms_page_multiple_page_next_page_link');
		$keywords['total_page_count'] = translate('cms_page_multiple_page_total_page_count');
		$keywords['page_name_x'] = translate('cms_page_multiple_page_page_name_x');
		$keywords['page_href_x'] = translate('cms_page_multiple_page_page_href_x');
		$keywords['page_link_x'] = translate('cms_page_multiple_page_page_link_x');
		$keywords['page_assetid_x'] = translate('cms_page_multiple_page_page_assetid_x');
		$keywords['current_page_name'] = translate('cms_page_multiple_page_current_page_name');
		$keywords['current_page_attribute_name'] = translate('cms_page_multiple_page_current_page_attribute_name');
		$keywords['current_page_number'] = translate('cms_page_multiple_page_current_page_number');
		$keywords['current_page_assetid'] = translate('cms_page_multiple_page_current_page_assetid');
		$keywords['toc_unordered'] = translate('cms_page_multiple_page_toc_unordered');
		$keywords['toc_ordered'] = translate('cms_page_multiple_page_toc_ordered');
		$keywords['navigation_menu_selection_list'] = translate('cms_page_multiple_page_navigation_menu_selection_list');
		$keywords['multiple_page_assetid'] = 'Multiple Page Asset ID';

		return $keywords;

	}//end getAvailableKeywords()


	/**
	* Create a new Child Page Underneath this
	*
	* @param string	$name		The name of the new page to create
	* @param int	$link_type	The link type to create it with
	*
	* @return array
	* @access public
	*/
	function createChildPage($name='page', $link_type=SQ_LINK_TYPE_2)
	{
		$import_link = Array('asset' => &$this, 'link_type' => $link_type, 'value' => '', 'sort_order' => $this->getNumberofPages(), 'is_dependant' => 1, 'is_exclusive' => 1);
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('page_multiple_page_page');
		$page = new Page_Multiple_Page_Page();
		$page->setAttrValue('name', $name);
		$page->create($import_link);

	}//end createChildPage()


	/**
	* Return the index (1-based) of the Multiple Page Page corresponding to the current URL
	* If the current URL is the Multiple Page itself, return 1
	* Return 0 if no match found
	*
	* Analyses the current URL, and works out what page we need to find.
	* This comes in handy when we need to perform functions without having the
	* printbody() or printPage() functions called, like Paint Layout keywords
	*
	* @return string
	* @access public
	*/
	function getCurrentPageId()
	{
		if (!isset($this->_tmp['current_page_id'])) {
			$this->_tmp['current_page_id'] = 0;
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL(NULL, NULL, TRUE, TRUE);
			if (!empty($asset)) {
				if ($asset->id == $this->id) {
					// this is the Multiple Page Asset itself, so we need the first page
					$this->_tmp['current_page_id'] = 1;
				} else {
					$links = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($this->id, $asset->id, SQ_LINK_TYPE_2|SQ_LINK_TYPE_1, NULL, 'major', TRUE);
					if (!empty($links) && $links[0]['minorid']) {
						$this->_tmp['current_page_id'] = $links[0]['sort_order'] + 1;
					}
				}
			}
		}

		return $this->_tmp['current_page_id'];

	}//end getCurrentPageId()


	/**
	* Analyses the current URL, and works out what page we need to find.
	* This comes in handy when we need to perform functions without having the
	* printbody() or printPage() functions called, like Paint Layout keywords
	*
	* @return string
	* @access public
	*/
	function &getCurrentPage()
	{
		return $this->getPageAsset($this->getCurrentPageId());

	}//end getCurrentPage()


	/**
	* Replaces all keywords for both itself and the child
	* Instantiates the current page asset if it doesnt exist.
	*
	* @param string	$value	The Value of the keyword that needs replacing
	*
	* @return array
	* @access public
	*/
	function getKeywordReplacement($value)
	{
		// Try this so we only have to initialise these values once for all keywords
		if (!isset($this->_tmp['current_page_id']) || empty($this->_tmp['current_page_id'])) {
			$this->_tmp['current_page_id'] = $this->getCurrentPageId();
		}

		if ($value == 'current_page_number') {
			return $this->_tmp['current_page_id'];
		}

		if (!isset($this->_tmp['current_page_asset']) || empty($this->_tmp['current_page_asset'])) {
			$this->_tmp['current_page_asset'] = $this->getCurrentPage();
		}
		if (!isset($this->_tmp['current_page_links']) || empty($this->_tmp['current_page_links'])) {
			$this->_tmp['current_page_links'] = $this->getPageLinks();
		}

		// Navigation menus
		if ($value == 'navigation_menu_selection_list') {
			return $this->paintNavSelectionList();
		}

		// Paint TOC here, so we dont have to go through the drama below
		if (strpos($value, 'toc_') !== FALSE) {
			$ordered = TRUE;
			if (strpos($value, '_unordered') !== FALSE) {
				$ordered = FALSE;
			}
			return $this->paintTOC($ordered);
		}

		$am = $GLOBALS['SQ_SYSTEM']->am;

		$replacement = '';

		// Get the ids of our previous and next pages
		$next_page_id = '';
		if (strpos($value, 'next_page_') !== FALSE) {
			$found_one = FALSE;
			foreach ($this->_tmp['current_page_links'] as $id => $data) {
				if ($found_one == TRUE) {
					$next_page_id = $data['minorid'];
					break;
				}
				if ($data['sort_order'] == strval(intval($this->_tmp['current_page_id']) - 1)) {
					$found_one = TRUE;
				}
			}
		}
		$prev_page_id = '';
		if (strpos($value, 'previous_page_') !== FALSE) {
			$found_one = FALSE;
			$prev_page = '';
			foreach ($this->_tmp['current_page_links'] as $id => $data) {
				if ($data['sort_order'] == strval(intval($this->_tmp['current_page_id']) - 1)) {
					$found_one = TRUE;
				}
				if ($found_one == TRUE) {
					$prev_page_id = $prev_page;
					break;
				}
				$prev_page = $data['minorid'];
			}
		}

		// We need to handle the page_bla_X stuff first, as a switch wont be suitable with the variable page numbers
		$page_number = '';
		$page_number_asset = NULL;
		if (strpos($value, 'page_link_') !== FALSE) {
			$page_number = str_replace('page_link_', '', $value);
			$value = 'page_link_';
		}
		if (strpos($value, 'page_href_') !== FALSE) {
			$page_number = str_replace('page_href_', '', $value);
			$value = 'page_href_';
		}
		if (strpos($value, 'page_name_') !== FALSE) {
			$page_number = str_replace('page_name_', '', $value);
			$value = 'page_name_';
		}
		if (strpos($value, 'page_assetid_') !== FALSE) {
			$page_number = str_replace('page_assetid_', '', $value);
			$value = 'page_assetid_';
		}
		if ($page_number != '') {
			$page_number_asset = $this->getPageAsset($page_number);
		}

		// Evaluate the values we need for keyword replacement
		switch ($value) {

			// Next Page Values
			case 'next_page_href':
				if (!empty($next_page_id)) {
					$replacement = current($am->getAssetUrl(Array($next_page_id)));
				}
			break;
			case 'next_page_name':
				if (!empty($next_page_id)) {
					$next_page_asset = $am->getAsset($next_page_id);
					if (!is_null($next_page_asset)) {
						$replacement = $next_page_asset->_getName();
					}
				}
			break;
			case 'next_page_link':
				if (!empty($next_page_id)) {
					$next_page_asset = $am->getAsset($next_page_id);
					if (!is_null($next_page_asset)) {
						$href = $next_page_asset->getURL();
						if (!empty($href)) {
							$replacement = '<a href="'.$href.'">'.$next_page_asset->_getName().'</a>';
						}
					}
				}
			break;

			// Previous Page Values
			case 'previous_page_href':
				if (!empty($prev_page_id)) {
					$replacement = current($am->getAssetUrl(Array($prev_page_id)));
				}
			break;
			case 'previous_page_name':
				if (!empty($prev_page_id)) {
					$prev_page_asset = $am->getAsset($prev_page_id);
					if (!is_null($prev_page_asset)) {
						$replacement = $prev_page_asset->_getName();
					}
				}
			break;
			case 'previous_page_link':
				if (!empty($prev_page_id)) {
					$prev_page_asset = $am->getAsset($prev_page_id);
					if (!is_null($prev_page_asset)) {
						$href = $prev_page_asset->getURL();
						if (!empty($href)) {
							$replacement = '<a href="'.$href.'">'.$prev_page_asset->_getName().'</a>';
						}
					}
				}
			break;

			// Some additional keywords for the overall page set
			case 'total_page_count':
				$replacement = $this->getNumberOfPages();
			break;

			// Provides individual links for
			case 'page_link_':
			if (!is_null($page_number_asset)) {
				$href = $page_number_asset->getURL();
				if (!empty($href)) {
					$replacement = '<a href="'.$href.'">'.$page_number_asset->_getName().'</a>';
				}
			}
			break;
			case 'page_href_':
			if (!is_null($page_number_asset)) {
				$replacement = $page_number_asset->getHref();
			}
			break;
			case 'page_name_':
			if (!is_null($page_number_asset)) {
				$replacement = $page_number_asset->_getName();
			}
			break;
			case 'page_assetid_':
			if (!is_null($page_number_asset)) {
				$replacement = $page_number_asset->id;
			}
			break;

			// current page details
			case 'current_page_name':
			case 'current_page_attribute_name':
				if (!is_null($this->_tmp['current_page_asset'])) {
					$replacement = $this->_tmp['current_page_asset']->_getName();
				}
			break;

			case 'current_page_assetid':
				if (!is_null($this->_tmp['current_page_asset'])) {
					$replacement = $this->_tmp['current_page_asset']->id;
				}
			break;

			// parent page of all subpages, which is this asset
			case 'multiple_page_assetid':
				$replacement = $this->id;
			break;

			// Defaults, probably a standard keyword
			default:
				$replacement = parent::getKeywordReplacement($value);
		}//end switch keyword

		return $replacement;

	}//end getKeywordReplacement()


	/**
	* Retrieves the first page for the series
	*
	* @return void
	* @access public
	*/
	function &getFirstPageAsset()
	{
		$page_one = $this->getPageAsset('1');
		return $page_one;

	}//end getFirstPageAsset()


	/**
	* Prints out the Frontend for this asset
	* it's up to the kids to override
	*
	* @return void
	* @access public
	*/
	function printFrontend()
	{
		$page_one = $this->getFirstPageAsset();
		if (is_null($page_one)) $page_one = $this;
		$this->printFrontendAsset($page_one);

	}//end printFrontend()


	/**
	* Prints out a Table of Contents for this set of pages
	*
	* @param boolean	$ordered		Whether to display an OL or UL
	* @param boolean	$link_current	Show current page as a link?
	*
	* @return string
	* @access public
	*/
	function paintTOC($ordered=FALSE, $link_current=FALSE)
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$links = $this->getPageLinks();
		$current = $this->getCurrentPageId();
		$current_assetid = $links[intval($current) - 1]['minorid'];
		ob_start();
		if ($ordered) {
			echo '<ol>';
		} else {
			echo '<ul>';
		}
		foreach ($links as $link_data) {
			$assetid = $link_data['minorid'];
			$page_asset = $am->getAsset($assetid);
			if (is_null($page_asset)) {
				continue;
			}
			
			$name = $page_asset->_getName();
			echo '<li>';
			if ($assetid == $current_assetid && !$link_current) {
				echo $name;
			} else {
				$href = $page_asset->getURL();
				echo '<a href="'.$href.'">'.$name.'</a>';
			}
			echo '</li>';
		}
		if ($ordered) {
			echo '</ol>';
		} else {
			echo '</ul>';
		}
		$toc = ob_get_clean();

		return $toc;

	}//end paintTOC()


	/**
	* Prints a Selection List with onChange event to navigate between pages.
	*
	* @param boolean	$link_current	Show current page as a link?
	*
	* @return string
	* @access public
	*/
	function paintNavSelectionList($link_current=FALSE)
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$links = $this->getPageLinks();
		$current = $this->getCurrentPageId();
		$current_assetid = $links[intval($current) - 1]['minorid'];
		ob_start();
		echo '<select id="'.$this->getPrefix().'nav_selection_list'.'" onchange="location.href=this.options[selectedIndex].value">';

		foreach ($links as $link_data) {
			$assetid = $link_data['minorid'];
			$page_asset = $am->getAsset($assetid);
			if (is_null($page_asset)) {
				continue;
			}
			
			$name = $page_asset->_getName();
			if ($assetid == $current_assetid && !$link_current) {
				echo '<option value="" selected="selected">'.$name;
			} else {
				$href = $page_asset->getURL();
				echo '<option value="'.$href.'">'.$name;
			}
			echo '</option>';
		}

		echo '</select>';
		$nav = ob_get_clean();

		return $nav;

	}//end paintNavSelectionList()


	/**
	* Prints a Selection List with onChange event to navigate between pages.
	*
	* @param int		$linkid		The id of the link being passed
	* @param int		$sort_order	The order of the link to be changed
	* @param boolean	$switch		Boolean value to determine whether saveWebPath() should be executed.
	*
	* @return string
	* @access public
	*/
	function sortOrder($linkid, $sort_order, $switch)
	{
		$this->_tmp['no_sort_order_paths'] = $switch;
		$GLOBALS['SQ_SYSTEM']->am->updateLink($linkid,NULL,NULL,$sort_order);

	}//end sortOrder()

	/**
	* Clones certain specified components of the asset
	*
	* @param object     &$clone     the clone asset shell
	* @param array      $components the wanted components to clone eg.
	* <pre>
	*                               Array(
	*                                   'attributes',
	*                                   'metadata_schemas',
	*                                   'metadata',
	*                                   'workflow',
	*                                   'permissions',
	*                                   'data',
	*                                   'content_tags',
	*                                   'roles',
	*                               );
	*                               or alternately
	*                               Array('all');
	* </pre>
	* @param boolean    $override   whether or not to override the existing permission, metadata schemas, workflow schemas with the new ones.
	*
	* @return boolean
	* @access public
	* @see asset::cloneComponents()
	*/
	function cloneComponents(&$clone, $components, $override=FALSE)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!parent::cloneComponents($clone, $components, $override)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_CLONE_COMPONENTS'] = TRUE;

		if (in_array('attributes', $components) || in_array('all', $components)) {
			$current_ids = Array();
			// save the information
			// Bug fix #3659 'Multiple Page' cloning loses 'Multiple Page Page' web paths
			// do this because, page_multiple_page saves its current child page asset ids 
			// is the attributes, if we dont make it empty, newly cloned asset takes 
			// child pages from the sorce as its own child
			$clone->setAttrValue('current_page_ids', $current_ids);
			$clone->saveAttributes();

			// Change the content of the content type to replace question id.
		}
		// if cloning we need this to update webpaths later
		$this->_tmp['no_sort_order_paths'] = TRUE;

		$GLOBALS['SQ_CLONE_COMPONENTS'] = FALSE;

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end cloneComponents()

}//end class

?>
