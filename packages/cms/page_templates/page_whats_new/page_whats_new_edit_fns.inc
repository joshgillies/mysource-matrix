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
* $Id: page_whats_new_edit_fns.inc,v 1.2 2004/09/15 00:40:02 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../page_templates/page_asset_listing/page_asset_listing_edit_fns.inc';

/**
* Page_Whats_New_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @author Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Whats_New_Edit_Fns extends Page_Asset_Listing_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Page_Whats_New_Edit_Fns()
	{
		$this->Page_Asset_Listing_Edit_Fns();
		
	}//end constructor


	/**
	* Paints the current ranges that have been added to this asset
	*
	* @param object Asset				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCurrentRanges(&$asset, &$o, $prefix)
	{
		$ranges = $asset->attr('date_ranges');

		if (empty($ranges)) {
			echo 'There are no ranges';
		} else {
			foreach ($ranges as $id => $info) $options[$id] = $info['name'];
			$options = array_reverse($options);
			$options[] = '-- Please Select --';
			$options = array_reverse($options);

			combo_box($prefix.'_range_id', $options, false, '', 0 , 'onchange="this.form.submit();"');
		}

		return false;

	}//end paintCurrentRanges()


	/**
	* Proccess the current ranges that have been added to this asset
	*
	* @param object Asset				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processCurrentRanges(&$asset, &$o, $prefix)
	{
		return false;
		
	}//end processCurrentRanges()


	/**
	* Paints the checkbox to add a new range
	*
	* @param object Asset				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAddRange(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return false;
		check_box($prefix.'_new_range');

		return true;

	}//end paintAddRange()


	/**
	* Processes the add range interface
	*
	* @param object Asset				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAddRange(&$asset, &$o, $prefix)
	{
		return false;

	}//end processAddRange()


	/**
	* Paints the range options interface
	*
	* @param object Asset				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintRangeOptions(&$asset, &$o, $prefix)
	{
		require_once SQ_ATTRIBUTES_PATH.'/datetime/datetime.inc';
		$write_access = $asset->writeAccess('attributes');
		$ranges       = $asset->attr('date_ranges');

		if (isset($_POST[$prefix.'_new_range'])) {
			$rangeid = -1;
		} else if (isset($_POST[$prefix.'_range_id']) && $_POST[$prefix.'_range_id'] != 0 && isset($ranges[$_POST[$prefix.'_range_id'] - 1])) {
			// remove 1 from the index because of the please select option
			$rangeid = $_POST[$prefix.'_range_id'] - 1;
		} else {
			echo 'Please select a date range to customise';
			return false;
		}

		$range_name = isset($ranges[$rangeid]['name']) ? $ranges[$rangeid]['name'] : '';
		$o->addHiddenField($prefix.'_curr_range_id', $rangeid);

		// field opened by XML

			if ($write_access) {
				text_box($prefix.'_range_name', $range_name);
			} else {
				echo $range_name;
			}

		$o->closeField();

		foreach (Array('oldest_date', 'newest_date') as $type) {

			$o->openField(ucwords(str_replace('_', ' ', $type)));
				$datetime = new Asset_Attribute_DateTime();

				// set some params for the datetime field so that it paints properly
				$datetime->_edit_params['allow_period'] = true;
				$datetime->_edit_params['show'] = Array('y','m','d','h','i','s');

				foreach($datetime->_edit_params['show'] as $u) $datetime->_edit_params['style'][$u] = 's';
				if (trim($range_name) != '') $datetime->value = $ranges[$rangeid][$type];

				$datetime->paint($prefix.'_'.$type, !$write_access);

			$o->closeField();
		}

		// only add an option to remove this range if it has a name (it has been saved)
		if ($asset->writeAccess('attributes') && trim($range_name) != '') {
			$o->openField('Remove ?');
					check_box($prefix.'_remove', false);
			$o->closeField();
		}

		return true;

	}//end paintRangeOptions()


	/**
	* Processes the range options interface
	*
	* @param object Asset				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processRangeOptions(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_curr_range_id']) || !isset($_POST[$prefix.'_range_name']) ) return false;
		require_once SQ_ATTRIBUTES_PATH.'/datetime/datetime.inc';

		$ranges     = $asset->attr('date_ranges');
		$rangeid    = $_POST[$prefix.'_curr_range_id'];
		$range_name = $_POST[$prefix.'_range_name'];
		$save       = false;

		if (isset($_POST[$prefix.'_remove'])) {
			unset($ranges[$rangeid]);
			$save = true;
		} else {

			// if the rangeid is -1, then we are creating a new range
			if ($rangeid == -1) {
				// if there is no name set for this, just return
				if (trim($range_name) == '') return false;
				$ranges[count($ranges)] = Array('name' => $range_name);
				$current_range =& $ranges[count($ranges) - 1];

			} else {
				$current_range =& $ranges[$rangeid];
				if (trim($range_name) != '' && $range_name != $current_range['name']) {
					$current_range['name'] = $range_name;
					$save = true;
				}
			}

			foreach (Array('oldest_date', 'newest_date') as $type) {
				$datetime = new Asset_Attribute_DateTime();
				$datetime->process($prefix.'_'.$type);

				if (isset($current_range[$type]) && $current_range[$type] == $datetime->value) continue;
				$current_range[$type] = $datetime->value;
				$save = true;
			}
		}//end if remove

		if ($save) {
			$ranges = array_values($ranges);
			$asset->setAttrValue('date_ranges', $ranges);
		}
		return $save;

	}//end processRangeOptions()


}//end class

?>
