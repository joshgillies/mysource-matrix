<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | Squiz.net Commercial Module Licence                                |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: page_multiple_page.inc,v 1.8 2005/12/07 21:55:23 dmckee Exp $
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
* @version $Revision: 1.8 $
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
	function Page_Multiple_Page($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Page($assetid);

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
		$page_links[SQ_LINK_TYPE_2]['page_multiple_page_page'] = Array('card' => 'M', 'exclusive' => true);
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
		//This should never be called, unless there are no pages linked underneath this.
		//printFrontend will divert the attention of the output to a child asset before this is hit

		echo translate('currently_no_pages_to_display');

	}//end printBody()


	/**
	* Called by the child pages, when their url is accessed
	*
	* @param object	$asset	The asset object of the page to print
	*
	* @return void
	* @access public
	*/
	function printPage($asset)
	{
		$this->_tmp['current_page_asset'] = &$asset;

		//$cm = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
		//$cached_contents = $cm->loadFromCache($this->id, $this->type(), 'default');

		$keywords = $asset->getKeywords();

		$replacements = Array();

		foreach ($keywords as $value) {
			$replacements[$value] = parent::getKeywordReplacement($value);
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
		$contents = 'This asset uses its children to display its content. Getting here is a mistake';
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
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$page_links = $am->getLinks($this->id, SQ_LINK_TYPE_2|SQ_LINK_TYPE_1, 'page_multiple_page_page', true);
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
		$page_links = $am->getLinks($this->id, SQ_LINK_TYPE_2|SQ_LINK_TYPE_1, 'page_multiple_page_page', true);
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
		$page_asset = null;
		$links = $this->getPageLinks();
		foreach ($links as $link_data) {
			if ($link_data['sort_order'] == strval(intval($page_number) - 1)) {
				$page_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
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

		//Sort the links, so that the pages appear in the backend, in the same order as they appear in the aset map
		usort($page_links, $cmp);
		foreach ($page_links as $id => $data) {
			$ids[] = $data['minorid'];
		}
		return $ids;

	}//end getPageIds()


	/**
	* Allow any required processesing to occur when any link is updated for the asset
	*
	* @return boolean
	* @access protected
	*/
	function linksUpdated()
	{
		$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'attributes', 0, true);
		//This is bugging out so we can delay the updating of our web paths
		if (isset($this->_tmp['deleting_links']) && $this->_tmp['deleting_links'] == true) {
			return true;
		}
		$current_children = array_flip($this->attr('current_page_ids'));

		$new_page_children = Array();
		$page_links = $this->getPageLinks();
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$new_paths = Array();
		foreach ($page_links as $data) {
			$new_page_children[] = $data['minorid'];
			$page =& $am->getAsset($data['minorid']);
			unset($current_children[$data['minorid']]);
			$paths = $page->getWebPaths();
			$new_paths[strval(intval($data['sort_order']) + 1)] = &$page;
			$page->saveWebPaths(Array());
		}

		foreach ($current_children as $id => $bla) {
			$deleted =& $GLOBALS['SQ_SYSTEM']->am->getAsset($id);
			if (!is_null($deleted)) {
				$deleted->saveWebPaths(Array());
			}
		}

		parent::linksUpdated();

		foreach ($new_paths as $path => $asset) {
			$asset->saveWebPaths(Array($path));
		}

		// This will store all current pages,
		$this->setAttrValue('current_page_ids', $new_page_children);
		$this->saveAttributes();
		$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'attributes');
		return true;

	}//end linksUpdated()


	/**
	* Return the keywords for this asset type.
	*
	* @return array
	* @access public
	*/
	function getAvailableKeywords()
	{
		$keywords = parent::getAvailableKeywords();

		$keywords['previous_page_href'] = 'Previous Page Href';
		$keywords['next_page_href'] = 'Next Page Href';
		$keywords['previous_page_name'] = 'Previous Page Title';
		$keywords['next_page_name'] = 'Next Page Title';
		$keywords['previous_page_link'] = 'Previous Page Title';
		$keywords['next_page_link'] = 'Next Page Title';
		$keywords['total_page_count'] = 'Total Page Count';
		$keywords['page_name_x'] = 'Specific Page Name';
		$keywords['page_href_x'] = 'Specific Page Href';
		$keywords['page_link_x'] = 'Specific Page Link';
		$keywords['page_assetid_x'] = 'Specific Page Asset ID';
		$keywords['current_page_name'] = 'Current Page Name';
		$keywords['current_page_number'] = 'Current Page Number';
		$keywords['current_page_assetid'] = 'Current Page Asset ID';
		$keywords['toc_unordered'] = 'Table of Contents - Unordered';
		$keywords['toc_ordered'] = 'Table of Contents - Ordered';

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
		$page =& new Page_Multiple_Page_Page();
		$page->setAttrValue('name', $name);
		$page->create($import_link);

	}//end createChildPage()


	/**
	* Analyses the current URL, and works out what page we need to find.
	* This comes in handy when we need to perform functions without having the
	* printbody() or printPage() functions called, like Paint Layout keywords
	*
	* @return string
	* @access public
	*/
	function getCurrentPageId()
	{
		$url = $_SERVER['REQUEST_URI'];
		if (strpos ($url, '?') !== false) {
			$url = substr($url, 0, strpos($url, '?'));
		}
		$url = rtrim($url, '/');
		$stub = $url;
		if (strrpos($url, '/') !== false) {
			$stub = substr($url, strrpos($url, '/') + 1);
		}
		$all_paths = $this->getWebPaths();
		foreach ($all_paths as $path) {
			if ($path == $stub) {
				//this must be the base url, so we need the first page
				$stub = '1';
				break;
			}
		}
		return $stub;

	}//end getCurrentPageId()


	/**
	* Analyses the current URL, and works out what page we need to find.
	* This comes in handy when we need to perform functions without having the
	* printbody() or printPage() functions called, like Paint Layout keywords
	*
	* @return string
	* @access public
	*/
	function getCurrentPage()
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
		//Try this so we only have to initialise these values once for all keywords
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

		//Paint TOC here, so we dont have to go through the drama below
		if (strpos($value, 'toc_') !== false) {
			$ordered = true;
			if (strpos($value, '_unordered') !== false) {
				$ordered = false;
			}
			return $this->paintTOC($ordered);
		}

		$am = $GLOBALS['SQ_SYSTEM']->am;

		$replacement = '';

		//Get the ids of our previous and next pages
		$next_page_id = '';
		if (strpos($value, 'next_page_') !== false) {
			$found_one = false;
			foreach ($this->_tmp['current_page_links'] as $id => $data) {
				if ($found_one == true) {
					$next_page_id = $data['minorid'];
					break;
				}
				if ($data['sort_order'] == strval(intval($this->_tmp['current_page_id']) - 1)) {
					$found_one = true;
				}
			}
		}
		$prev_page_id = '';
		if (strpos($value, 'previous_page_') !== false) {
			$found_one = false;
			$prev_page = '';
			foreach ($this->_tmp['current_page_links'] as $id => $data) {
				if ($data['sort_order'] == strval(intval($this->_tmp['current_page_id']) - 1)) {
					$found_one = true;
				}
				if ($found_one == true) {
					$prev_page_id = $prev_page;
					break;
				}
				$prev_page = $data['minorid'];
			}
		}

		//We need to handle the page_bla_X stuff first, as a switch wont be suitable with the variable page numbers
		$page_number = '';
		$page_number_asset = null;
		if (strpos($value, 'page_link_') !== false) {
			$page_number = str_replace('page_link_', '', $value);
			$value = 'page_link_';
		}
		if (strpos($value, 'page_href_') !== false) {
			$page_number = str_replace('page_href_', '', $value);
			$value = 'page_href_';
		}
		if (strpos($value, 'page_name_') !== false) {
			$page_number = str_replace('page_name_', '', $value);
			$value = 'page_name_';
		}
		if (strpos($value, 'page_assetid_') !== false) {
			$page_number = str_replace('page_assetid_', '', $value);
			$value = 'page_assetid_';
		}
		if ($page_number != '') {
			$page_number_asset = $this->getPageAsset($page_number);
		}

		// Evaluate the values we need for keyword replacement
		switch ($value) {

			//Next Page Values
			case 'next_page_href':
				if (!empty($next_page_id)) {
					$replacement = current($am->getAssetUrl(Array($next_page_id)));
				}
			break;
			case 'next_page_name':
				if (!empty($next_page_id)) {
					$info = current($am->getAssetInfo(Array($next_page_id)));
					if (isset($info['name'])) {
						$replacement = $info['name'];
					}
				}
			break;
			case 'next_page_link':
				if (!empty($next_page_id)) {
					$name = '';
					$info = current($am->getAssetInfo(Array($next_page_id)));
					if (isset($info['name'])) $name = $info['name'];
					$href = current($am->getAssetUrl(Array($next_page_id)));
					if (!empty($name) && !empty($href)) {
						$replacement = '<a href="'.$href.'">'.$name.'</a>';
					}
				}
			break;

			//Previous Page Values
			case 'previous_page_href':
				if (!empty($prev_page_id)) {
					$replacement = current($am->getAssetUrl(Array($prev_page_id)));
				}
			break;
			case 'previous_page_name':
				if (!empty($prev_page_id)) {
					$info = current($am->getAssetInfo(Array($prev_page_id)));
					if (isset($info['name'])) {
						$replacement = $info['name'];
					}
				}
			break;
			case 'previous_page_link':
				if (!empty($prev_page_id)) {
					$name = '';
					$info = current($am->getAssetInfo(Array($prev_page_id)));
					if (isset($info['name'])) $name = $info['name'];
					$href = current($am->getAssetUrl(Array($prev_page_id)));
					if (!empty($name) && !empty($href)) {
						$replacement = '<a href="'.$href.'">'.$name.'</a>';
					}
				}
			break;

			//Some additional keywords for the overall page set
			case 'total_page_count':
				$replacement = $this->getNumberOfPages();
			break;

			//Provides individual links for
			case 'page_link_':
			if (!is_null($page_number_asset)) {
				$replacement = '<a href="'.$page_number_asset->getHref().'">'.$page_number_asset->name.'</a>';
			}
			break;
			case 'page_href_':
			if (!is_null($page_number_asset)) {
				$replacement = $page_number_asset->getHref();
			}
			break;
			case 'page_name_':
			if (!is_null($page_number_asset)) {
				$replacement = $page_number_asset->name;
			}
			break;
			case 'page_assetid_':
			if (!is_null($page_number_asset)) {
				$replacement = $page_number_asset->id;
			}
			break;

			//current page details
			case 'current_page_name':
				if (!is_null($this->_tmp['current_page_asset'])) {
					$replacement = $this->_tmp['current_page_asset']->name;
				}
			break;

			case 'current_page_assetid':
				if (!is_null($this->_tmp['current_page_asset'])) {
					$this->_tmp['current_page_asset']->id;
				}
			break;

			//Defaults, probably a standard keyword
			default:
				$replacement = parent::getKeywordReplacement($value);
		}

		return $replacement;

	}//end getKeywordReplacement()


	/**
	* Retrieves the first page for the series
	*
	* @return void
	* @access public
	*/
	function getFirstPageAsset()
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
		if (is_null($page_one)) $page_one = &$this;
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
	function paintTOC($ordered=false, $link_current=false)
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$links = $this->getPageLinks();
		$current = $this->getCurrentPageId();
		$current_id = $links[intval($current) - 1]['minorid'];
		$assetids = Array();
		foreach ($links as $id => $data) {
			$assetids[] = $data['minorid'];
		}
		$toc = '';
		$urls = $am->getAssetUrl($assetids);
		$infos = $am->getAssetInfo($assetids);
		ob_start();
		if ($ordered) {
			echo '<ol>';
		} else {
			echo '<ul>';
		}
		foreach ($infos as $id => $data) {
			?>
			<li>
			<?php
			if ($id == $current_id && !$link_current) {
				echo $data['name'];
			} else {
				?>
				<a href="<?php echo $urls[$id]; ?>"><?php echo $data['name']; ?></a>
				<?php
			}
			?>
			</li>
			<?php
		}
		if ($ordered) {
			echo '</ol>';
		} else {
			echo '</ul>';
		}
		$toc = ob_get_contents();
		ob_end_clean();

		return $toc;

	}//end paintTOC()


}//end class

?>
