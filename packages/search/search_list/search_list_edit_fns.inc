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
* $Id: search_list_edit_fns.inc,v 1.1 2004/08/19 05:52:22 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_asset_listing/page_asset_listing_edit_fns.inc';

/**
* Search_List_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Search_List_Edit_Fns extends Page_Asset_Listing_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Search_List_Edit_Fns()
	{
		$this->Page_Asset_Listing_Edit_Fns();

	}//end constructor


	/**
	* Paints the search page selection box
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSearchPage(&$asset, &$o, $prefix)
	{
		$search_page_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', false, 'search_page');
		$write_access = $asset->writeAccess('links');

		if (!$write_access) {
			$search_page = null;
			if (!empty($search_page_link)) {
				$search_page = &$GLOBALS['SQ_SYSTEM']->am->getAsset($search_page_link['minorid']);
			}
			if (!is_null($search_page)) echo $search_page->name . ' (Id: #' . $search_page->id . ')';
			else echo 'No search page has been selected';
		} else {
			asset_finder($prefix.'_search_page', (!empty($search_page_link)) ? $search_page_link['minorid'] : '', Array('search_page' => 'I'));
		}

		return $write_access;

	}//end paintSearchPage()


	/**
	* Processes the value input from the search page selection box
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for form elements
	*
	* @return void
	* @access public
	*/
	function processSearchPage(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_search_page']['assetid'])) return false;
		$new_search_page  = $_POST[$prefix.'_search_page']['assetid'];

		// getting existing search page
		$search_page_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', false, 'search_page');

		// dont process anything if the search page has not changed
		if (!empty($search_page_link) && $search_page_link['minorid'] == $new_search_page) return false;

		$ok = true;

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// deleting old search page link if any found...
		if (!empty($search_page_link) && !$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($search_page_link['linkid'])) {
			$ok = false;
		} else if ($new_search_page) {
			$search_page = &$GLOBALS['SQ_SYSTEM']->am->getAsset($new_search_page);
			if (!is_null($search_page)) {
				if (!$asset->createLink($search_page, SQ_LINK_NOTICE, 'search_page')) {
					$ok = false;
					trigger_error('Failed adding new search page "'.$search_page->name.'" [#'.$search_page->id.']', E_USER_WARNING);
				}
			}
		}

		if ($ok) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return true;
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

	}//end processSearchPage()
	
	
	/**
	* Paint the interface for specifying the search field
	*
	* @param object	Asset	&$asset		asset being painted
	* @param object			&$o			backend outputter
	* @param string			$prefix		prefix for form elements
	*
	* @access public
	* @return boolean
	*/
	function paintSearchField(&$asset, &$o, $prefix)
	{
		$search_page = null;
		$search_page_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', false, 'search_page');
		if (!empty($search_page_link)) {
			$search_page = &$GLOBALS['SQ_SYSTEM']->am->getAsset($search_page_link['minorid']);
		}
		
		if (is_null($search_page)) {
			echo 'You must select a search page before a search field can be set';
			return false;
		}

		if ($asset->writeAccess('attributes')) {

			$fields = $search_page->attr('fields');

			$options = Array();
			foreach ($fields as $field_name => $field) {
				$field_description = $fields[$field_name];
				$field_description = $field_name.' ';

				switch ($field['type']) {
					case 'asset_attrib' :
						$field_description .= '(asset attribute)';
						$field_name = 'attribute:'.$field_name.':'.$field['params']['searchtype'];
					break;
					case 'metadata' :
						$field_description .= '(metadata)';
						$field_name = 'metadata:'.$field_name.':'.$field['params']['metadata_field']['assetid'];
					break;
					default:
						// note that we cant use exclude fields because, by themsevles, they never return results
						continue(2);
					break;
				}
				$options[$field_name] = $field_description;
			}

			$options = array_merge(Array('' => 'None'), $options);
			combo_box($prefix.'_field_name', $options, false, $asset->attr('search_field'));
			return true;
		} else {
			echo $asset->attr('search_field');
			return false;
		}

	}//end paintSearchField()


	/**
	* Process the interface for specifying the search field
	*
	* @param object		&$asset		the searchbox design area asset
	* @param object		&$o			the backend outputter
	* @param string		$prefix		the prefix for this field
	*
	* @access public
	* @return boolean
	*/
	function processSearchField(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_field_name'])) return false;
		return $asset->setAttrValue('search_field', $_POST[$prefix.'_field_name']);

	}//end processSearchField()


}//end class

?>
