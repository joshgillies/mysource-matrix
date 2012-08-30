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
* $Id: search_list.inc,v 1.23 2012/08/30 00:59:15 ewang Exp $
*
*/

require_once SQ_PACKAGES_PATH.'/cms/listing_engine/listing_engine.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

/**
* Search_List
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.23 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Search_List extends Listing_Engine
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Override this to stop it creating type folders, etc but retain page contents
	* and no results bodycopies
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		if (!Page::_createAdditional($link)) return FALSE;
		return $this->_createBodycopies(Array('content' => '%asset_listing%'));

	}//end _createAdditional()


	/**
	* Get the list of asset that should be printed
	*
	* The return array is in the form Array(assetid => type_code) unless we are grouping by letter,
	* in which case the return array is
	* <pre>
	* Array(word => Array(
	*					'type_code' => '',
	*					'firt_letter' => first_letter,
	*					'num_values' => hit counts,
	*				);
	*		);
	* </pre>
	*
	* @return array
	* @access protected
	*/
	function getAssetList()
	{
		$search_page = NULL;
		$search_page_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', FALSE, 'search_page');
		if (!empty($search_page_link)) {
			$search_page = $GLOBALS['SQ_SYSTEM']->am->getAsset($search_page_link['minorid']);
		}
		if (is_null($search_page)) return Array();

		$sm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');
		$plugin =& $sm->_getDBPlugin();
		$children = $plugin->getSearchListAssets($this, $search_page);

		// See printContents() in Listing Engine. It needs to set this flag to by-pass
		// certain asset info caching since the format of returning array is not the same as
		// the parent.
		$this->_tmp['no_asset_info_url_cache'] = TRUE;

		$this->_tmp['search_terms'] = $children;

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
		return;

	}//end filterAssetList()


	/**
	* Print the list of assets that we are listing
	*
	* @param array	$todo	an array of assets to list in the same format as the return
	*						value of getAssetList()
	*
	* @return void
	* @access protected
	* @see getAssetList()
	*/
	function printAssetList($todo)
	{
		$search_field_name = $this->attr('search_field');
		if (empty($search_field_name)) return;

		$search_page = NULL;
		$search_page_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', FALSE, 'search_page');
		if (!empty($search_page_link)) {
			$search_page = $GLOBALS['SQ_SYSTEM']->am->getAsset($search_page_link['minorid']);
		}
		if (is_null($search_page)) return;

		$this->_tmp['search_url'] = $search_page->getURL().'?mode=results&amp;queries_'.$search_field_name.'_query=';

		$this->_printAssetList($todo);

	}//end printAssetList()


	/**
	* Print a single word from the list
	*
	* @param string	$word			the search term being printed
	* @param int	$list_position	the position of this asset in the list
	*
	* @return void
	* @access private
	* @see printAssetList()
	*/
	function _printAsset($word, $list_position)
	{
		$format = $this->attr('default_format');
		$keywords = Array(
						'search_term'			=> '<a href="'.$this->_tmp['search_url'].$word.'">'.$word.'</a>',
						'number_of_occurrences'	=> $this->_tmp['search_terms'][$word]['num_values'],
					);

		echo replace_keywords($format, $keywords);

	}//end _printAsset()


}//end class

?>
