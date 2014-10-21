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
* $Id: design_area_searchbox_edit_fns.inc,v 1.28 2012/08/30 00:59:15 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';

/**
* Design_Area_SearchBox_Edit_Fns
*
* Edit functions for searchbox.
*
* @see Design_Area_Edit_Fns
*
* @author  Dominic Wong <dwong@squiz.net>
* @version $Revision: 1.28 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Design_Area_Searchbox_Edit_Fns extends Design_Area_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor


	/**
	* Paint the field interface for selecting the search page to delegate to
	*
	* @param object	&$asset	the searchbox design area asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for this field
	*
	* @return boolean
	* @access public
	*/
	function paintBackendSearchPage(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			asset_finder($prefix.'_search_page', $asset->attr('search_page_assetid'), Array('search_page' => 'D'));
			return TRUE;
		} else {
			$search_pageid = $asset->attr('search_page_assetid');
			if ($search_pageid > 0) {
				echo get_asset_tag_line($search_pageid);
			} else {
				echo translate('No search page specified');

			}
			return FALSE;
		}

	}//end paintBackendSearchPage()


	/**
	* Process the field interface for selecting the search page to delegate to
	*
	* @param object	&$asset	the searchbox design area asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for this field
	*
	* @return boolean
	* @access public
	*/
	function processBackendSearchPage(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_search_page']['assetid'])) {
			if ($_POST[$prefix.'_search_page']['assetid'] > 0) {
				$asset->setAttrValue('search_page_assetid', $_POST[$prefix.'_search_page']['assetid']);
			} else {
				$asset->setAttrValue('search_page_assetid', 0);
			}
		}
		return TRUE;

	}//end processBackendSearchPage()


	/**
	* Paint the field interface for specifying whether the submit button should be a button or an image.
	*
	* @param object	&$asset	the searchbox design area asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for this field
	*
	* @return boolean
	* @access public
	*/
	function paintBackendSubmitImage(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			asset_finder($prefix.'_submit_image', $asset->attr('submit_image_assetid'), Array('file' => 'D'));
			return TRUE;
		} else {
			$submit_imageid = $asset->attr('submit_image_assetid');
			if ($submit_imageid > 0) {
				echo get_asset_tag_line($submit_imageid);
			} else {
				echo translate('No submit image specified');

			}
			return FALSE;
		}

	}//end paintBackendSubmitImage()


	/**
	* Process the field interface for specifying whether the submit button should be a button or an image
	*
	* @param object	&$asset	the searchbox design area asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for this field
	*
	* @return boolean
	* @access public
	*/
	function processBackendSubmitImage(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_submit_image']['assetid'])) {
			return FALSE;
		}

		if ($_POST[$prefix.'_submit_image']['assetid'] > 0) {
			return $asset->setAttrValue('submit_image_assetid', $_POST[$prefix.'_submit_image']['assetid']);
		} else {
			return $asset->setAttrValue('submit_image_assetid', 0);
		}

	}//end processBackendSubmitImage()


	/**
	* Paints the search category set
	*
	* @param object	&$asset	the searchbox design area asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for this field
	*
	* @return boolean
	* @access public
	*/
	function paintSearchCategorySet(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) {
			echo $asset->attr('category_set');
			return FALSE;
		}

		$search_assetid = $asset->attr('search_page_assetid');
		if (empty($search_assetid)) return FALSE;

		$search_page = $GLOBALS['SQ_SYSTEM']->am->getAsset($search_assetid);

		$categories[''] = '-- Please Select --';
		foreach (array_keys($search_page->attr('categories')) as $category_id) {
			$categories[$category_id] = $category_id;
		}

		combo_box($prefix.'_category_set', $categories, FALSE, $asset->attr('category_set'));
		return TRUE;

	}//end paintSearchCategorySet()


	/**
	* Processes search category set
	*
	* @param object	&$asset	the searchbox design area asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for this field
	*
	* @return boolean
	* @access public
	*/
	function processSearchCategorySet(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;

		if (!isset($_POST[$prefix.'_category_set'])) {
			return FALSE;
		}

		$new_category_set = $_POST[$prefix.'_category_set'];

		// If we are changing the category then clear other category settings
		if ($new_category_set != $asset->attr('category_set')) {
			$asset->setAttrValue('category_show_empty', TRUE);
			$asset->setAttrValue('category_default', '');
		}
		$asset->setAttrValue('category_set', $new_category_set);

		return TRUE;

	}//end processSearchCategorySet()


	/**
	* Paints the default search category
	*
	* @param object	&$asset	the searchbox design area asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for this field
	*
	* @return boolean
	* @access public
	*/
	function paintSearchCategoryDefault(&$asset, &$o, $prefix)
	{
		$search_assetid = $asset->attr('search_page_assetid');
		if (empty($search_assetid)) return FALSE;

		$search_page = $GLOBALS['SQ_SYSTEM']->am->getAsset($search_assetid);

		$categories = Array();
		$category_sets = $search_page->attr('categories');
		$category_set = $category_sets[$asset->attr('category_set')];

		if ($asset->attr('category_show_empty')) {
			$categories[''] = $category_set['unselected_text'];
		}

		foreach ($category_set['options'] as $category_id => $category_data) {
			$categories[$category_id] = $category_data['full_name'];
		}

		if (!$asset->writeAccess('attributes')) {
			echo $categories[$asset->attr('category_default')];
			return FALSE;
		} else {
			combo_box($prefix.'_category_default', $categories, FALSE, $asset->attr('category_default'));
		}
		return TRUE;

	}//end paintSearchCategoryDefault()


	/**
	* Process the default search category
	*
	* @param object	&$asset	the searchbox design area asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for this field
	*
	* @return boolean
	* @access public
	*/
	function processSearchCategoryDefault(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;

		if (!isset($_POST[$prefix.'_category_default'])) {
			return FALSE;
		}

		$asset->setAttrValue('category_default', $_POST[$prefix.'_category_default']);
		return TRUE;

	}//end processSearchCategoryDefault()


	/**
	* Paint the field interface for specifying the search field to which this search query corresponds to on the search page
	*
	* @param object	&$asset	the searchbox design area asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for this field
	*
	* @return boolean
	* @access public
	*/
	function paintSearchFieldName(&$asset, &$o, $prefix)
	{
		// TODO: This needs to be updated to better handle the multiple sources,
		//      it'd likely show yellow errors and 'unknown' for all of them
		if ($asset->writeAccess('attributes')) {
			$search_page = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->attr('search_page_assetid'));
			$fields = $search_page->attr('fields');
			if (is_scalar($fields)) {
				$fields = unserialize($fields);
			}

			$options = Array();
			foreach ($fields as $field_name => $field) {
				$field_description = $fields[$field_name];
				$field_description = $field_name.' ';
				$options[$field_name] = $field_description;
			}

			if ($asset->attr('field_name') != array_keys($fields)) {
				$options = array_merge(Array('' => 'None'), $options);
			}
			combo_box($prefix.'_field_name', $options, FALSE, $asset->attr('field_name'));
			return TRUE;
		} else {
			echo $asset->attr('field_name');
			return FALSE;
		}

	}//end paintSearchFieldName()


	/**
	* Process the field interface for specifying the search field to which this search query corresponds to on the search page
	*
	* @param object	&$asset	the searchbox design area asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for this field
	*
	* @return boolean
	* @access public
	*/
	function processSearchFieldName(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_field_name'])) {
			return FALSE;
		}

		return $asset->setAttrValue('field_name', $_POST[$prefix.'_field_name']);

	}//end processSearchFieldName()


	/**
	* Validate the search page asset
	*
	* Used to test if appropriate fields should be shown
	*
	* @param object	&$owner	the searchbox design area asset
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access public
	*/
	function hasValidSearchPage(&$owner, $prefix)
	{
		$assetid = $owner->attr('search_page_assetid');
		if (!$assetid) return FALSE;
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (!$asset->id) return FALSE;
		return $asset->type() == 'search_page';

	}//end hasValidSearchPage()


	/**
	* Validate the category set
	*
	* Used to test if appropriate fields should be shown
	*
	* @param object	&$owner	the searchbox design area asset
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access public
	*/
	function hasValidCategorySet(&$owner, $prefix)
	{
		if (!$this->hasValidSearchPage($owner, $prefix)) {
			return FALSE;
		}

		return $owner->attr('category_set') != '';

	}//end hasValidCategorySet()


	/**
	* Test whether a button or an image is shown when submitting
	*
	* Used to test if appropriate fields should be shown
	*
	* @param object	&$owner	the searchbox design area asset
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access public
	*/
	function isSubmitButton(&$owner, $prefix)
	{
		return !$owner->attr('submit_button');

	}//end isSubmitButton()


}//end class

?>