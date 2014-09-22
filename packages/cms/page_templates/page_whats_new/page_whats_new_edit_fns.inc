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
* $Id: page_whats_new_edit_fns.inc,v 1.23 2012/08/30 00:58:09 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/listing_engine/listing_engine_edit_fns.inc';

/**
* Page_Whats_New_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @author Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.23 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Whats_New_Edit_Fns extends Listing_Engine_Edit_Fns
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
	* Paints the current ranges that have been added to this asset
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCurrentRanges(&$asset, &$o, $prefix)
	{
		$ranges = $asset->attr('date_ranges');

		if (empty($ranges)) {
			echo translate('There are no ranges');

		} else {
			foreach ($ranges as $id => $info) {
				$options[$id] = $info['name'];
			}
			$options = array_reverse($options);
			$options[] = '-- '.translate('Please Select').' --';

			$options = array_reverse($options);

			combo_box($prefix.'_range_id', $options, FALSE, '', 0 , 'onchange="this.form.submit();"');
		}

		return FALSE;

	}//end paintCurrentRanges()


	/**
	* Proccess the current ranges that have been added to this asset
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processCurrentRanges(&$asset, &$o, $prefix)
	{
		return FALSE;

	}//end processCurrentRanges()


	/**
	* Paints the checkbox to add a new range
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAddRange(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		check_box($prefix.'_new_range');

		return TRUE;

	}//end paintAddRange()


	/**
	* Processes the add range interface
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAddRange(&$asset, &$o, $prefix)
	{
		return FALSE;

	}//end processAddRange()


	/**
	* Paints the range options interface
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintMainDateRange(&$asset, &$o, $prefix)
	{
		require_once SQ_ATTRIBUTES_PATH.'/datetime/datetime.inc';
		$write_access = $asset->writeAccess('attributes');

		if ($asset->attr('range_type') !== 'default') {
			$range = $asset->attr('range_type');
			$date_range_from = '---------- --:--:--';
			$date_range_to   = '---------- --:--:--';
		} else {
			$range = '';
			$date_range_from = $asset->attr('date_range_from');
			$date_range_to   = $asset->attr('date_range_to');
		}

		$range_types = Array(
						'default'	=> 'Specified Range',
						'today'		=> 'Today',
						'week'		=> 'This Week',
						'month'		=> 'This Month',
						'year'		=> 'This Year',
					   );

		$o->openField('Range Type');
			if ($write_access === TRUE) {
				combo_box($prefix.'_range_type', $range_types, FALSE, $range);
			} else {
				if(!empty($range))
					echo $range_types[$range];
			}
		$o->note(translate('When this option is set, the Oldest Date and Newest Date options are ignored, and this predefined range is used.<br/><br/>For these ranges, the Newest Date is set to "now", and the Oldest Date is set to 00:00:00 on the first day of the specified period. For "This Week", weeks start on Monday, in line with the ISO definition.'));

		$o->closeField();

		foreach (Array('date_range_from', 'date_range_to') as $type) {

			$o->openField(ucwords(str_replace('_', ' ', $type)));
				$datetime = new Asset_Attribute_DateTime();

				// set some params for the datetime field so that it paints properly
				$datetime->_edit_params['allow_period'] = TRUE;
				$datetime->_edit_params['allow_period_today'] = TRUE;
				$datetime->_edit_params['use_picker'] = TRUE;
				$datetime->_edit_params['show'] = Array('y','m','d','h','i','s');
				$datetime->_edit_params['null'] = Array('y','m','d','h','i','s');

				foreach ($datetime->_edit_params['show'] as $u) {
					$datetime->_edit_params['style'][$u] = 's';
				}
				$datetime->_edit_params['style']['y'] = 't';
				$datetime->value = $$type;

				$datetime->paint($prefix.'_'.$type, !$write_access);

			$o->closeField();
		}

		return $write_access;

	}//end paintMainDateRange()


	/**
	* Processes the range options interface
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processMainDateRange(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes') === FALSE) {
			return FALSE;
		}

		require_once SQ_ATTRIBUTES_PATH.'/datetime/datetime.inc';

		$range_type = array_get_index($_POST, $prefix.'_range_type', 'default');
		$save       = FALSE;

		if ($range_type !== 'default') {
			if ($asset->attr('range_type') !== $range_type) {
				$asset->setAttrValue('range_type', $range_type);
				$asset->setAttrValue('date_range_from', '---------- --:--:--');
				$asset->setAttrValue('date_range_to',   '---------- --:--:--');
				$save = TRUE;
			}
		} else {
			$asset->setAttrValue('range_type', 'default');
			foreach (Array('date_range_from', 'date_range_to') as $type) {
				$datetime = new Asset_Attribute_DateTime();
				$datetime->process($prefix.'_'.$type);

				if ($asset->attr($type) !== $datetime->value) {
					$asset->setAttrValue($type, $datetime->value);
					$save = TRUE;
				}
			}
		}

		return $save;

	}//end processMainDateRange()


	/**
	* Paints the range options interface
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
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
			echo translate('Please select a date range to customise');

			return FALSE;
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

		if ($rangeid === -1) {
			$range = 'default';
			$oldest_date = '---------- --:--:--';
			$newest_date = '---------- --:--:--';
		} else if (array_key_exists('range_type', $ranges[$rangeid]) === TRUE) {
			$range = $ranges[$rangeid]['range_type'];
			$oldest_date = isset($ranges[$rangeid]['oldest_date']) ? $ranges[$rangeid]['oldest_date'] : '---------- --:--:--';
			$newest_date = isset($ranges[$rangeid]['newest_date']) ? $ranges[$rangeid]['newest_date'] : '---------- --:--:--';
		} else {
			$range = 'default';
			$oldest_date = '---------- --:--:--';
			$newest_date = '---------- --:--:--';
		}

		$range_types = Array(
						'default'	=> 'Specified Range',
						'today'		=> 'Today',
						'week'		=> 'This Week',
						'month'		=> 'This Month',
						'year'		=> 'This Year',
					   );

		$o->openField('Range Type');
			if ($write_access === TRUE) {
				combo_box($prefix.'_range_type', $range_types, FALSE, $range);
			} else {
				echo $range_types[$range];
			}
			$o->note(translate('When this option is set, the Oldest Date and Newest Date options are ignored, and this predefined range is used.<br/><br/>For these ranges, the Newest Date is set to "now", and the Oldest Date is set to 00:00:00 on the first day of the specified period. For "This Week", weeks start on Monday, in line with the ISO definition.'));

		$o->closeField();

		foreach (Array('oldest_date', 'newest_date') as $type) {

			$o->openField(ucwords(str_replace('_', ' ', $type)));
				$datetime = new Asset_Attribute_DateTime();

				// set some params for the datetime field so that it paints properly
				$datetime->_edit_params['allow_period'] = TRUE;
				$datetime->_edit_params['allow_period_today'] = TRUE;
				$datetime->_edit_params['use_picker'] = TRUE;
				$datetime->_edit_params['show'] = Array('y','m','d','h','i','s');
				$datetime->_edit_params['null'] = Array('y','m','d','h','i','s');

				foreach ($datetime->_edit_params['show'] as $u) {
					$datetime->_edit_params['style'][$u] = 's';
				}
				$datetime->value = $$type;

				$datetime->paint($prefix.'_'.$type, !$write_access);

			$o->closeField();
		}

		// only add an option to remove this range if it has a name (it has been saved)
		if ($asset->writeAccess('attributes') && trim($range_name) != '') {
			$o->openField(translate('Remove?'));

				check_box($prefix.'_remove', FALSE);
			$o->closeField();
		}

		return $write_access;

	}//end paintRangeOptions()


	/**
	* Processes the range options interface
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processRangeOptions(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_curr_range_id']) || !isset($_POST[$prefix.'_range_name']) ) {
			return FALSE;
		}
		require_once SQ_ATTRIBUTES_PATH.'/datetime/datetime.inc';

		$ranges     = $asset->attr('date_ranges');
		$rangeid    = $_POST[$prefix.'_curr_range_id'];
		$range_name = $_POST[$prefix.'_range_name'];
		$range_type = array_get_index($_POST, $prefix.'_range_type', 'default');
		$save       = FALSE;

		if (isset($_POST[$prefix.'_remove'])) {
			unset($ranges[$rangeid]);
			$save = TRUE;
		} else {

			// if the rangeid is -1, then we are creating a new range
			if ($rangeid == -1) {
				// if there is no name set for this, just return
				if (trim($range_name) == '') return FALSE;
				$ranges[count($ranges)] = Array('name' => $range_name);
				$current_range =& $ranges[count($ranges) - 1];

			} else {
				$current_range =& $ranges[$rangeid];
				if (trim($range_name) != '' && $range_name != $current_range['name']) {
					$current_range['name'] = $range_name;
					$save = TRUE;
				}
			}

			if ($range_type !== 'default') {
				if (array_get_index($current_range, 'range_type', 'default') !== $range_type) {
					$current_range['range_type'] = $range_type;
					unset($current_range['oldest_date']);
					unset($current_range['newest_date']);
					$save = TRUE;
				}
			} else {
				$current_range['range_type'] = 'default';
				foreach (Array('oldest_date', 'newest_date') as $type) {
					$datetime = new Asset_Attribute_DateTime();
					$datetime->process($prefix.'_'.$type);

					if (isset($current_range[$type]) && $current_range[$type] == $datetime->value) {
						continue;
					}
					$current_range[$type] = $datetime->value;
					$save = TRUE;
				}
			}
		}//end if remove

		if ($save) {
			$ranges = array_values($ranges);
			$asset->setAttrValue('date_ranges', $ranges);
		}
		return $save;

	}//end processRangeOptions()


	/**
	* Paint the interface for selecting searched statuses
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintLinkTypes(&$asset, &$o, $prefix)
	{
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$write_access = $asset->writeAccess('attributes');

		$current = $asset->attr('link_types');
		$link_types = get_link_type_names();

		if ($write_access) {
			require_once SQ_LIB_PATH.'/html_form/html_form.inc';

			$selected = Array();
			foreach ($link_types as $link_type => $link_type_name) {
				if ($current & $link_type) {
					$selected[] = $link_type;
				}
			}

			combo_box($prefix.'_link_types', $link_types, TRUE, $selected);
		} else {
			if ($current == 0) {
				echo translate('No link types have been selected - <b>no assets will be displayed</b>');

			} else {
				echo '<ul>';
				foreach ($link_types as $link_type => $link_type_name) {
					if ($current & $link_type) {
						echo '<li>'.$link_type_name.'</li>';
					}
				}
				echo '</ul>';
			}
		}

		return $write_access;

	}//end paintLinkTypes()


	/**
	* Processes the interface for selecting searched statuses
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processLinkTypes(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return FALSE;

		$link_types = 0;
		if (isset($_POST[$prefix.'_link_types'])) {
			foreach ($_POST[$prefix.'_link_types'] as $link_type) {
				$link_types |= $link_type;
			}
		}

		return $asset->setAttrValue('link_types', $link_types);

	}//end processLinkTypes()


}//end class

?>
