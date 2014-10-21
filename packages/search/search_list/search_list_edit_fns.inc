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
* $Id: search_list_edit_fns.inc,v 1.15 2012/08/30 00:59:15 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/listing_engine/listing_engine_edit_fns.inc';

/**
* Search_List_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.15 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Search_List_Edit_Fns extends Listing_Engine_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

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
				echo get_asset_tag_line($search_page_link['minorid']);
			} else {
				echo translate('No search page specified');

			}

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
		if (!isset($_POST[$prefix.'_search_page']['assetid'])) {
			return false;
		}

		$new_search_page  = $_POST[$prefix.'_search_page']['assetid'];

		// getting existing search page
		$search_page_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', false, 'search_page');

		// dont process anything if the search page has not changed
		if (!empty($search_page_link) && $search_page_link['minorid'] == $new_search_page) {
			return false;
		}

		$ok = true;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// deleting old search page link if any found...
		if (!empty($search_page_link) && !$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($search_page_link['linkid'])) {
			$ok = false;
		} else if ($new_search_page) {
			$search_page = $GLOBALS['SQ_SYSTEM']->am->getAsset($new_search_page);
			if (!is_null($search_page)) {
				if (!$asset->createLink($search_page, SQ_LINK_NOTICE, 'search_page')) {
					$ok = false;
					trigger_localised_error('SCH0011', sprintf(translate('Failed adding new search page "%1$s" [#%2$s]'), $search_page->name, $search_page->id), E_USER_WARNING);
				}
			}
		}

		if ($ok) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return $ok;

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
			$search_page = $GLOBALS['SQ_SYSTEM']->am->getAsset($search_page_link['minorid']);
		}

		if (is_null($search_page)) {
			echo translate('You must select a search page before a search field can be set');

			return false;
		}

		if ($asset->writeAccess('attributes')) {
			$fields = $search_page->attr('fields');

			$options = Array();
			foreach ($fields as $field_name => $field) {
				$can_use = true;

				// loop through each data source and if it contains an 'include all'
				// or 'exclude' anywhere, then it cannot be used
				foreach($field['data_sources'] as $data_source) {
					if (($data_source['type'] == 'include_all') || ($data_source['type'] == 'exclude')) {
						$can_use = false;
						break;
					}
				}

				if ($can_use) {
					$options[$field_name] = $field['name'];
				}
			}

			$options = array_merge(Array('' => translate('None')), $options);

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
	* @param object		&$asset		the searchbox design area assetmetadata
	* @param object		&$o			the backend outputter
	* @param string		$prefix		the prefix for this field
	*
	* @access public
	* @return boolean
	*/
	function processSearchField(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_field_name'])) {
			return false;
		}

		return $asset->setAttrValue('search_field', $_POST[$prefix.'_field_name']);

	}//end processSearchField()


}//end class

?>
