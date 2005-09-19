<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: search_list.inc,v 1.11 2005/09/19 23:40:01 lwright Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_asset_listing/page_asset_listing.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

/**
* Search_List
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.11 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Search_List extends Page_Asset_Listing
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Search_List($assetid=0)
	{
		$this->Page_Asset_Listing($assetid);

	}//end constructor


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Search lists create a bodycopy asset when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @access private
	* @return boolean
	*/
	function _createAdditional(&$link)
	{
		if (!page::_createAdditional($link)) return false;

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$asset =& new Bodycopy();
		$copy_link = Array('asset' => &$this, 'value' => 'page_contents', 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);
		$asset->setAttrValue('name', 'Page Contents');

		return $asset->create($copy_link);

	}//end _createAdditional()


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
		$search_page = null;
		$search_page_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', false, 'search_page');
		if (!empty($search_page_link)) {
			$search_page = &$GLOBALS['SQ_SYSTEM']->am->getAsset($search_page_link['minorid']);
		}
		if (is_null($search_page)) return Array();

		$search_field = $this->attr('search_field');
		$field_type = substr($search_field, 0, strpos($search_field, ':'));
		$field_params = substr($search_field, strpos($search_field, ':') +1);

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sm = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');
		$search_info = $search_page->populateBaseSearchInfo();
		$query_comps = $sm->constructBaseSearchQuery($search_info);
		$query_comps['from'][] = 'sq_sch_idx ai';
		$query_comps['where'][] = 'ai.assetid = a.assetid';
		$query_comps['select'][] = 'ai.value';
		$query_comps['select'][] = 'ai.assetid';
		$query_comps['select'][] = 'SUBSTR(ai.value, 1, 1) AS first_letter';

		switch ($field_type) {
			case 'attribute' :
				list($name, $type_code) = split(':', $field_params);
				$query_comps['where'][] = 'ai.component = '.$db->quote('attr:'.$name);
				$this->_tmp['search_field_name'] = $name;
			break;
			case 'metadata' :
				list($name, $attrid) = split(':', $field_params);
				$query_comps['where'][] = 'ai.component = '.$db->quote('metadata:'.$attrid);
				$this->_tmp['search_field_name'] = $name;
			break;
			default :
				return Array();
			break;
		}

		$query_comps['group_by'][] = 'ai.value';
		$query_comps['group_by'][] = 'ai.assetid';
		$sql = implode_sql($query_comps);

		$results = $db->getAssoc($sql, false, Array(), DB_FETCHMODE_DEFAULT, true);
		assert_valid_db_result($results);

		$children = Array();
		foreach ($results as $word => $word_data) {
			$children[$word]['num_values'] = count($word_data);
			$base_data = array_pop($word_data);
			$children[$word]['first_letter'] = $base_data['first_letter'];
			$children[$word]['type_code'] = null;
		}

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
	* @param array	&$todo	an array of assets to list in the same format as the return
	*						value of getAssetList()
	*
	* @return void
	* @access protected
	* @see getAssetList()
	*/
	function printAssetList($todo)
	{
		if (!isset($this->_tmp['search_field_name'])) return;

		$search_page = null;
		$search_page_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', false, 'search_page');
		if (!empty($search_page_link)) {
			$search_page = &$GLOBALS['SQ_SYSTEM']->am->getAsset($search_page_link['minorid']);
		}
		if (is_null($search_page)) return;

		$this->_tmp['search_url'] = $search_page->getURL().'?mode=results&queries_'.$this->_tmp['search_field_name'].'_query=';

		$this->_printAssetList($todo);

	}//end printAssetList()


	/**
	* Print a single word from the list
	*
	* @param string	$word			the search term being printed
	* @param string	$type_code		the type code of the asset we are printing
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
