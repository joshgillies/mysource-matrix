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
* $Id: page_calendar_edit_fns.inc,v 1.6 2004/08/19 23:37:16 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_asset_listing/page_asset_listing_edit_fns.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Page_Calendar_Edit_Fns
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.6 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Page_Calendar_Edit_Fns extends Page_Asset_Listing_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Page_Calendar_Edit_Fns()
	{
		$this->Page_Asset_Listing_Edit_Fns();

	}//end Page_Calendar_Edit_Fns()

	
	/**
	* Paint the interface for setting the root nodes from which this calendar page draws events
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintRootNodes(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header">Location to draw events from</td>
				<td class="sq-backend-table-header">CSS Class to apply to the events' display</td>
			</tr>
		<?php
		$locations = $asset->attr('root_nodes');
		$location_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($locations));
		$l = 1;
		
		foreach ($locations as $location => $details) {
			?>
			<tr>
				<td class="sq-backend-table-cell">
				<?php
					if ($write_access) {
						asset_finder($prefix.'_root_nodes['.$l.']', $location);
					} else {
						echo $location_info[$location]['name'].' [# '.$location.']';
					}
				?>
				</td>

				<td class="sq-backend-table-cell">
				<?php
					if ($write_access) {
						text_box($prefix.'_node_classes['.$l.']', $details['class_name']);
					} else {
						echo $details['class_name'];
					}
				?>
				</td>
			</tr>
			<?php
			$l++;
		}//end foreach
				
		if ($write_access) {
			?>
			<tr>
				<td class="sq-backend-table-cell"><?php asset_finder($prefix.'_root_nodes['.$l.']', 0); ?></td>
				<td class="sq-backend-table-cell"><?php text_box($prefix.'_node_classes['.$l.']', ''); ?></td>
			</tr>
			<?php
		}
		?></table><?php
		return true;
		
	}//end paintRootNodes()
	
	
	/**
	* Process the interface for setting the root nodes from which this calendar page draws events
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/	
	function processRootNodes(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_root_nodes'])) {
			$locations = Array();
			foreach ($_POST[$prefix.'_root_nodes'] as $index => $loc) {
				if ($locid = $loc['assetid']) {
					$locations[$locid] = isset($_POST[$prefix.'_node_classes'][$index]) ? Array('class_name' => $_POST[$prefix.'_node_classes'][$index]) : Array('class_name' => '');
				}
			}
			return $asset->setAttrValue('root_nodes', $locations);
		} else {
			return false;
		}
		
	}//end processRootNodes()


	/**
	* Paint the interface for choosing which calendar views are available on the frontend
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintViewsChooser(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$current_views = $asset->attr('enabled_views');
		foreach ($asset->view_options as $category => $options) {
			if ($write_access) {
				?>
				<br /><label style="width: 20ex" for="<?php echo $prefix.'_enabled_views['.$category.']'; ?>"><?php echo ucwords($category); ?> view type: </label>
				<?php
				combo_box($prefix.'_enabled_views['.$category.']', $options, 0,  $current_views[$category], 0, 'style="width: 25ex"');
			} else {
				?>
				<br /><div style="display: inline; width: 20ex"><?php echo ucwords($category); ?></div><?php echo $asset->view_options[$category][$current_views[$category]]; ?>
				<?php
			}
		}
		return true;
		
	}//end paintViewsChooser()
	

	/**
	* Process the interface for choosing which calendar views are available on the frontend
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processViewsChooser(&$asset, &$o, $prefix)
	{
		$unwanted_elts = array_diff(array_keys($_POST[$prefix.'_enabled_views']), array_keys($asset->view_options));
		if (empty($unwanted_elts)) {
			$asset->setAttrValue('enabled_views', $_POST[$prefix.'_enabled_views']);
			return true;
		}
		return false;

	}//end processViewsChooser()


	/**
	* Paint the interface for choosing which view the frontend user sees first
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintInitialView(&$asset, &$o, $prefix)
	{
		$selected_view = $asset->attr('initial_view');
		if ($asset->writeAccess('attributes')) {
			$view_options = $asset->view_options;
			$enabled_views = $asset->attr('enabled_views');
			foreach ($enabled_views as $name => $status) {
				if (empty($status)) unset($view_options[$name]);
			}
			foreach ($view_options as $key => $val) $view_options[$key] = $key;
			if (!in_array($selected_view, $view_options)) $selected_view = current($view_options);
			combo_box($prefix.'_initial_view', $view_options, 0, $selected_view, 0);
		} else {
			echo $selected_view;
		}
		return true;

	}//end paintInitialView()


	/**
	* Process the interface for choosing which view the frontend user sees first
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processInitialView(&$asset, &$o, $prefix)
	{
		$enabled_views = $asset->attr('enabled_views');
		if (isset($_POST[$prefix.'_initial_view']) && !empty($enabled_views[$_POST[$prefix.'_initial_view']])) {
			$asset->setAttrValue('initial_view', $_POST[$prefix.'_initial_view']);
		} else {
			$x = current($enabled_views);
			while (empty($x) && ($x !== FALSE)) {
				$x = next($enabled_views);
			}
			$asset->setAttrValue('initial_view', key($enabled_views));
		}
		
	}//end processInitialView()


	/**
	* Paint an interface where you can choose a point in time with 5-minute precision
	*
	* @param string		$prefix	prefix for the form elements
	* @param int		$default_hour	the default value for the hour field
	* @param int		$default_min	the default value for the minutes field
	* @param boolean	$default_is_pm	whether the default time is pm
	*
	* @return void
	* @access private
	*/
	function _paint5MinTimeField($prefix, $default_hour=9, $default_min=0, $default_is_pm=false) 
	{
		$hour_options = Array(12=>12, 1=>1, 2=>2, 3=>3, 4=>4, 5=>5, 6=>6, 7=>7, 8=>8, 9=>9, 10=>10, 11=>11, 12=>12);
		$min_options = Array();
		for ($i=0; $i < 60; $i += 5) $min_options[sprintf('%02d', $i)] = sprintf('%02d', $i);
		$is_pm_options = Array(0=>'am', 1=>'pm');
		combo_box($prefix.'[hours]', $hour_options, false, $default_hour, 0);
		echo ' : ';
		combo_box($prefix.'[minutes]', $min_options, false, sprintf('%02d', $default_min), 0);
		echo '&nbsp;';
		combo_box($prefix.'[is_pm]', $is_pm_options, false, (int)$default_is_pm, 0);

	}//end _paint5MinTimeField()


	/**
	* Process a 5-minute-precision time-choosing field
	*
	* @param string		$prefix	prefix for the form elements
	*
	* @return string	HH:MM value of the element, in 24 hour format
	* @access public
	*/
	function _process5MinTimeField($prefix) 
	{
		if (!isset($_POST[$prefix])) return false;
		if ($_POST[$prefix]['hours'] != (int)$_POST[$prefix]['hours']) return false;
		if ($_POST[$prefix]['minutes'] != (int)$_POST[$prefix]['minutes']) return false;
		if ($_POST[$prefix]['is_pm'] && ($_POST[$prefix]['hours'] != 12)) $_POST[$prefix]['hours'] += 12;
		if (!$_POST[$prefix]['is_pm'] && ($_POST[$prefix]['hours'] == 12)) $_POST[$prefix]['hours'] = 0;
		return sprintf('%02d', $_POST[$prefix]['hours']) . ':' . $_POST[$prefix]['minutes'];

	}//end _process5MinTimeField()


	/**
	* Paint the interface for choosing the earliest time to be displayed in a day
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDayStart(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes'))	$this->_paint5MinTimeField($prefix.'_day_start_time', 9, 0, false);
		else echo $asset->attr('day_starts_at');
		return true;

	}//end paintDayStart()


	/**
	* Process the interface for choosing the earliest time to be displayed in a day
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processDayStart(&$asset, &$o, $prefix)
	{
		if ($value = $this->_process5MinTimeField($prefix.'_day_start_time')) {
			$asset->setAttrValue('day_starts_at', $value);
			return true;
		} else {
			return false;
		}
		
	}//end processDayStart()


	/**
	* Paint the interface for choosing the latest time to be displayed in a day
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDayEnd(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes'))	$this->_paint5MinTimeField($prefix.'_day_end_time', 5, 0, true);
		else echo $asset->attr('day_ends_at');

	}//end paintDayEnd()


	/**
	* Process the interface for choosing the latest time to be displayed in a day
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processDayEnd(&$asset, &$o, $prefix)
	{
		if ($value = $this->_process5MinTimeField($prefix.'_day_end_time')) {
			$asset->setAttrValue('day_ends_at', $value);
			return true;
		} else {
			return false;
		}
		
	}//end processDayEnd()


	/**
	* Paint the interface for choosing the days to include in the week view
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintWeekDays(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$days_to_show = $asset->attr('week_view_show_days');
		foreach ($asset->day_names_full as $day_number => $day_name) {
			echo '<br />';
			if ($write_access) {
				check_box($prefix.'_show_days['.$day_number.']', 1, $days_to_show[$day_number]);
			} else {
				?>
				<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo $days_to_show[$day_number] ? 'tick' : 'cross'; ?>.gif" width="15" height="15" />
				<?php
			}
			echo $day_name;
		}
		return true;

	}//end paintWeekDays()


	/**
	* Process the interface for choosing the days to include in the week view
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processWeekDays(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_show_days']) && is_array($_POST[$prefix.'_show_days'])) {
			foreach (range(0, 6) as $day_number) {
				if (!isset($_POST[$prefix.'_show_days'][$day_number])) $_POST[$prefix.'_show_days'][$day_number] = 0;
			}
			$asset->setAttrValue('week_view_show_days', $_POST[$prefix.'_show_days']);
			return true;
		} else {
			return false;
		}
		
	}//end processWeekDays()


	/**
	* Paint the explanation of the CSS implementation and link to the CSS
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintLookAndFeel(&$asset, &$o, $prefix)
	{
		?>
		<p style="width: 80ex; margin-left: 15ex">The Calendar Page has a default CSS file that controls the look and feel of the Calendar views.  A link to this CSS file is automatically 
		inserted into the head of the generated HTML document.</p>

		<p style="width: 80ex; margin-left: 15ex">If you want to customise some or all of the style rules you can <a href="<?php echo sq_web_path('data')?>/asset_types/page_calendar/css/calendar_default.css">download the default CSS file</a> and modify it.  Put a link to your modified version in the parse file of
		the design that applies to this page, <i>after</i> the "head" design area tag.</p>
		<?php

	}//end paintLookAndFeel()
	
	
	/**
	* Does nothing
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processLookAndFeel(&$asset, &$o, $prefix)
	{
		return true;
		
	}//end processLookAndFeel()
	
}//end class

?>