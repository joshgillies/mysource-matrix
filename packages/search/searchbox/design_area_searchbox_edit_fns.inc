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
* $Id: design_area_searchbox_edit_fns.inc,v 1.7 2003/12/17 04:47:41 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Design_Area_SearchBox_Edit_Fns
*
* Edit functions for searchbox.
* 
* @see Design_Area_Edit_Fns
*
* @author  Dominic Wong <dwong@squiz.net>
* @version $Revision: 1.7 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage search
*/

require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';

class Design_Area_SearchBox_Edit_Fns extends Design_Area_Edit_Fns
{	
	/** 
	* Constructor
	*
	* @access public
	*/ 
	function Design_Area_SearchBox_Edit_Fns()
	{
		$this->Design_Area_Edit_Fns();
	} // end Design_Area_SearchBox_Edit_Fns()

	/**
	* Paints the field interface for selecting the search page to delegate to.
	*
	* @param object $asset		The searchbox design area asset.
	* @param object $o			The backend outputter.
	* @param string $prefix	The prefix for this field.
	*
	* @access public
	* @return void
	*/
	function paintBackendSearchPage(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			asset_finder($prefix.'_search_page', $asset->attr('search_page_assetid'), Array('search_page' => 'D'));
		} else {
			$search_pageid = $asset->attr('search_page_assetid');
			if ($search_pageid > 0) {
				$search_page_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($search_pageid);
				echo $search_page_asset->name ." #" .$search_pageid;
			} else {
				echo 'No search page specified.';
			}
		}
	} // end paintBackendSearchPage()

	/**
	* Processes the field interface for selecting the search page to delegate to.
	*
	* @param object $asset		The searchbox design area asset.
	* @param object $o			The backend outputter.
	* @param string $prefix	The prefix for this field.
	*
	* @access public
	* @returns boolean	Returns whether any changes have been made.
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
		return true;
	} // end processBackendSearchPage

	/**
	* Paints the field interface for specifying whether the submit 
	* button should be a button or an image.
	*
	* @param object $asset		The searchbox design area asset.
	* @param object $o			The backend outputter.
	* @param string $prefix		The prefix for this field.
	*
	* @access public
	* @return void
	*/
	function paintBackendSubmitImage(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			asset_finder($prefix.'_submit_image', $asset->attr('submit_image_assetid'), Array('file' => 'D'));
		} else {
			$submit_imageid = $asset->attr('submit_image_assetid');
			if ($submit_imageid > 0) {
				$submit_image =& $GLOBALS['SQ_SYSTEM']->am->getAsset($submit_imageid);
				echo $submit_image->name ." #". $submit_imageid;
			} else {
				echo 'No submit image specified.';
			}
		}
	} // end paintBackendSubmitImage()

	/**
	* Processes the field interface for specifying whether the submit 
	* button should be a button or an image.
	*
	* @param object $asset		The searchbox design area asset.
	* @param object $o			The backend outputter.
	* @param string $prefix		The prefix for this field.
	*
	* @access public
	* @returns boolean	Returns whether any changes have been made.
	*/
	function processBackendSubmitImage(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_submit_image']['assetid']))
			return false;

		if ($_POST[$prefix.'_submit_image']['assetid'] > 0)
			$asset->setAttrValue('submit_image_assetid', $_POST[$prefix.'_submit_image']['assetid']);
		else 
			$asset->setAttrValue('submit_image_assetid', 0);
		return true;
	} // end processBackendSubmitImage()

	/**
	* Paints the field interface for specifying the search field
	* to which this search query corresponds to on the search page.
	*
	* @param object $asset		The searchbox design area asset.
	* @param object $o			The backend outputter.
	* @param string $prefix		The prefix for this field.
	*
	* @access public
	* @return void
	*/

	function paintSearchFieldName(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			$search_page = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->attr('search_page_assetid'));
			$fields = $search_page->attr('fields');
			if (is_scalar($fields)) {
				$fields = unserialize($fields);
			}

			$options = Array();
			foreach($fields as $field_name => $field) {
				$field_description = $fields[$field_name];
				$field_description = $field_name." ";
				switch($field['type']) {
					case 'include_all':		$field_description .= "(include all)";		break;
					case 'exclude':			$field_description .= "(exclude)";			break;
					case 'exclude':			$field_description .= "(exclude all)";		break;
//					case 'metadata':		$metadata_field =& $GLOBALS['SQ_SYSTEM']->am->getAsset($field['params']['metadata_field'])$field_description .= "(metadata on field 
					default:				$field_description .= "(unknown)";			break;
				}
				$options[$field_name] = $field_description;
			}

			if ($asset->attr('field_name') != array_keys($fields)) {
				$options = array_merge(Array('' => 'None'), $options);
			}
			combo_box($prefix.'_field_name', $options, false, $asset->attr('field_name'));
		} else {
			echo $asset->attr('field_name');
		}
	} // end paintSearchFieldName()

	/**
	* Processes the field interface for specifying the search field
	* to which this search query corresponds to on the search page.
	*
	* @param object $asset		The searchbox design area asset.
	* @param object $o			The backend outputter.
	* @param string $prefix		The prefix for this field.
	*
	* @access public
	* @return boolean
	*/
	function processSearchFieldName(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_field_name'])) return false;

		return $asset->setAttrValue('field_name', $_POST[$prefix.'_field_name']);
	} // end processSearchFieldName()

	/**
	* Validates the search page asset. Used to test if appropriate 
	* fields should be shown.
	*
	* @param object $owner		The searchbox design area asset.
	*
	* @access public
	* @return boolean 
	*/
	function hasValidSearchPage(&$owner)
	{
		$assetid = $owner->attr('search_page_assetid');
		$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (!$asset->id)
			return false;
		return $asset->type() == 'search_page';
	} // end hasValidSearchPage()

	/**
	* Tests whether a button or an image is shown when submitting. Used to 
	* test if appropriate fields should be shown.
	*
	* @param object $owner		The searchbox design area asset.
	*
	* @access public
	* @return boolean
	*/
	function isSubmitButton(&$owner)
	{
		return !$owner->attr('submit_button');
	} // end isSubmitButton()

}//end class
?>
