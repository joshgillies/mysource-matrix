<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: search_list.inc,v 1.17 2007/01/02 22:05:59 rong Exp $
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
* @version $Revision: 1.17 $
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
	function Search_List($assetid=0)
	{
		$this->Listing_Engine($assetid);

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
		$search_page = NULL;
		$search_page_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', FALSE, 'search_page');
		if (!empty($search_page_link)) {
			$search_page =& $GLOBALS['SQ_SYSTEM']->am->getAsset($search_page_link['minorid']);
		}
		if (is_null($search_page)) return Array();

		// get information about the search field
		$search_page_fields = $search_page->attr('fields');
		$search_field_name = $this->attr('search_field');
		if (empty($search_page_fields) || empty($search_field_name)) {
			// if search field has not been setup properly
			return Array();
		}
		$search_field = $search_page_fields[$search_field_name];

		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$sm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');
		$search_info = $search_page->populateBaseSearchInfo();
		$query_comps = $sm->constructBaseSearchQuery($search_info);

		array_unshift($query_comps['from'], 'sq_sch_idx ai'); // need to have this first to avoid SQL error
		$query_comps['where'][] = 'ai.assetid = a.assetid';
		$query_comps['select'][] = 'ai.value';
		$query_comps['select'][] = 'ai.assetid';
		$query_comps['select'][] = 'SUBSTR(ai.value, 1, 1) AS first_letter';

		$data_source_comps = Array();

		foreach ($search_field['data_sources'] as $data_source) {
			switch ($data_source['type']) {
				case 'asset_attrib' :
					$type_code = $data_source['params']['asset_type'];
					$attrid = $data_source['params']['attrid'];

					$attr_info = $GLOBALS['SQ_SYSTEM']->am->getAttributeInfo(Array($attrid));
					$name = $attr_info[$attrid]['name'];
					$data_source_comps[] = 'ai.component = '.$db->quote('attr:'.$name);
				break;
				case 'metadata' :
					$assetid = $data_source['params']['assetid'];
					$data_source_comps[] = 'ai.component = '.$db->quote('metadata:'.$assetid);
				break;
				case 'standard' :
					$param_field = $data_source['params']['field'];
					$data_source_comps[] = 'ai.component = '.$db->quoteSmart('__'.$param_field.'__');
				break;

				default :
					return Array();
				break;
			}
		}

		if (!empty($data_source_comps)) {
			$query_comps['where'][] = '('.implode(') OR (', $data_source_comps).')';
		}

		$query_comps['group_by'][] = 'ai.value';
		$query_comps['group_by'][] = 'ai.assetid';
		$sql = implode_sql($query_comps);

		$results = $db->getAssoc($sql, FALSE, Array(), DB_FETCHMODE_DEFAULT, TRUE);
		assert_valid_db_result($results);

		$children = Array();
		foreach ($results as $word => $word_data) {
			$children[$word]['num_values'] = count($word_data);
			$base_data = array_pop($word_data);
			$children[$word]['first_letter'] = $base_data['first_letter'];
			$children[$word]['type_code'] = NULL;
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
			$search_page =& $GLOBALS['SQ_SYSTEM']->am->getAsset($search_page_link['minorid']);
		}
		if (is_null($search_page)) return;

		$this->_tmp['search_url'] = $search_page->getURL().'?mode=results&queries_'.$search_field_name.'_query=';

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
