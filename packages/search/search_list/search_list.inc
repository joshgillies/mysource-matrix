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
* $Id: search_list.inc,v 1.1 2004/08/19 05:52:22 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_asset_listing/page_asset_listing.inc';

/**
* Search_List
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
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
	* Create this asset
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return mixed int or false
	* @access public
	*/
	function create(&$link)
	{
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$linkid = page::create($link);

		if (!$linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		// add a bodycopy to this page when creating
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');

		$asset = new Bodycopy();
		$copy_link = Array('asset' => &$this, 'value' => 'page_contents', 'link_type' => SQ_LINK_TYPE_2, 'dependant' => 1, 'exclusive' => 1);
		$asset->setAttrValue('name', 'Page Contents');

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
		$base_sql = $sm->constructBaseSearchQuery($search_info);
		$base_sql['select'] = ' SELECT ai.value, ai.assetid, SUBSTRING(ai.value, 1, 1) AS first_letter';

		switch ($field_type) {
			case 'attribute' :
				list($name, $type_code) = split(':', $field_params);
				$base_sql['where'] .= ' AND ai.component = '.$db->quote('attr:'.$type_code.':'.$name);
				$this->_tmp['search_field_name'] = $name;
			break;
			case 'metadata' :
				list($name, $attrid) = split(':', $field_params);
				$base_sql['where'] .= ' AND ai.component = '.$db->quote('metadata:'.$attrid);
				$this->_tmp['search_field_name'] = $name;
			break;
			default :
				return Array();
			break;
		}

		$sql = implode(' ', $base_sql);
		$sql .= ' GROUP BY ai.value, ai.assetid';

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
		
		$format = $this->attr('default_format');
		$search_url = $search_page->getURL().'?mode=results&queries_'.$this->_tmp['search_field_name'].'_query=';
		
		foreach ($todo as $word => $data) {

			$keywords = Array('search_term' => '<a href="'.$search_url.$word.'">'.$word.'</a>',
							  'number_of_occurrences' => $this->_tmp['search_terms'][$word]['num_values']
							 );
			
			echo replace_keywords($format, $keywords);

		}//end foreach todo

	}//end printAssetList()


}//end class

?>
