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
* $Id: page_calendar_edit_fns.inc,v 1.17 2004/10/08 03:24:49 tbarrett Exp $
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
* @version $Revision: 1.17 $ - 1.0
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
		?></table>
		<p style="width: 60ex; margin-left: 15ex">For each root node you can give the name of a CSS class you have defined, OR one of the CSS named colours to use when displaying events from that location.  (The CSS named colours are black, blue, fuchsia, gray, green, lime, maroon, navy, olive, purple, red, silver, teal, white, and yellow). </p>
		
		<?php
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
		$listing_entry_options = Array('title'=>'show title only', 'bodycopy'=>'use type format bodycopy');
		$current_list_entry_types = $asset->attr('list_entry_types');
		?><table class="sq-backend-table"><?php
		foreach ($asset->view_options as $category => $options) {
			?>
			<tr>
				<td class="sq-backend-table-cell">
					<b><label for="<?php echo $prefix.'_enabled_views['.$category.']'; ?>"><?php echo ucwords($category); ?> View</label></b>
				</td>
				<td class="sq-backend-table-cell"><?php
					if ($write_access) {
						combo_box($prefix.'_enabled_views['.$category.']', $options, 0,  $current_views[$category], 0, 'style="width: 25ex"');
					} else {
						echo $asset->view_options[$category][$current_views[$category]];
					}?>
				</td>
				<td class="sq-backend-table-cell"><?php
					if ($write_access && (strpos($current_views[$category], 'list') !== FALSE)) {
						echo 'For each list entry, ';
						combo_box($prefix.'_list_entry_types['.$category.']', $listing_entry_options, 0,  $current_list_entry_types[$category], 0);
					} elseif (strpos($current_views[$category], 'list') !== FALSE) {
						echo 'foo';
					} else {
						echo '&nbsp;';
					}?>
				</td>
			</tr><?php
		}?>
		</table><?php
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
		$list_entry_types = $asset->attr('list_entry_types');
		$unwanted_elts = array_diff(array_keys($_POST[$prefix.'_enabled_views']), array_keys($asset->view_options));
		if (empty($unwanted_elts)) {
			$asset->setAttrValue('enabled_views', $_POST[$prefix.'_enabled_views']);
			$initial_view = $asset->attr('initial_view');
			if (!($_POST[$prefix.'_enabled_views'][$initial_view])) {
				foreach ($_POST[$prefix.'_enabled_views'] as $category => $view) {
					if ($view) {
						$asset->setAttrValue('initial_view', $category);
						break;
					}
				}
			}
			if ($year_drill_down = $asset->attr('year_drill_down_view')) {
				if (!($_POST[$prefix.'_enabled_views'][$year_drill_down])) {
					$asset->setAttrValue('year_drill_down_view', '');
					foreach ($_POST[$prefix.'_enabled_views'] as $category => $view) {
						if (($category != 'year') && ($category != 'event') && $view) {
							$asset->setAttrValue('year_drill_down_view', $category);
							break;
						}
					}
				}
			}
			foreach ($list_entry_types as $key => $value) {
				if (isset($_POST[$prefix.'_list_entry_types'][$key]) && in_array($_POST[$prefix.'_list_entry_types'][$key], Array('title', 'bodycopy'))) {
					$list_entry_types[$key] = $_POST[$prefix.'_list_entry_types'][$key];
				}
			}
			$asset->setAttrValue('list_entry_types', $list_entry_types);

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
			foreach ($view_options as $key => $val) $view_options[$key] = ucfirst($key).' View';
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
		if ($asset->writeAccess('attributes')) {
			list($hour,$minute) = sscanf($asset->attr('day_starts_at'),'%02d:%02d');
			$is_pm = false;
			if ($hour >= 12) {
				$is_pm = true;
				$hour -= 12;
			}
			if ($hour == 0) $hour = 12;
			
			$this->_paint5MinTimeField($prefix.'_day_start_time', $hour, $minute, $is_pm);
		} else {
			echo $asset->attr('day_starts_at');
		}
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
		if ($asset->writeAccess('attributes'))	if ($asset->writeAccess('attributes')) {
			list($hour,$minute) = sscanf($asset->attr('day_ends_at'),'%02d:%02d');
			$is_pm = false;
			if ($hour >= 12) {
				$is_pm = true;
				$hour -= 12;
			}
			if ($hour == 0) $hour = 12;
			
			$this->_paint5MinTimeField($prefix.'_day_end_time', $hour, $minute, $is_pm);
		} else {
			echo $asset->attr('day_starts_at');
		}
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
		foreach ($asset->day_names as $day_number => $day_name) {
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
		<p style="width: 60ex; margin-left: 15ex">The Calendar Page has a default CSS file that controls the look and feel of the Calendar views.  A link to this CSS file is automatically 
		inserted into the head of the generated HTML document.</p>

		<p style="width: 60ex; margin-left: 15ex">If you want to customise some or all of the style rules you can <a href="<?php echo sq_web_path('data')?>/asset_types/page_calendar/css/calendar_default.css">download the default CSS file</a> and modify it.  Put a link to your modified version in the parse file of
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


	/**
	* Paint the interface where you choose the view you're taken to when you click on a day in year view
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintYearDrillDownView(&$asset, &$o, $prefix)
	{
		$current_view = $asset->attr('year_drill_down_view');
		echo '<select name="'.$prefix.'_year_drill_down_view">';
		echo '<option value="">(none)</option>';
		$options = $asset->attr('enabled_views');
		foreach ($options as $category => $type) {
			if (!(($category == 'year') || ($category == 'event') || ($type == ''))) {
				echo '<option '.(($category==$current_view) ? 'selected="selected" ' : '').'value="'.$category.'">'.$category.'</option>';
			}
		}
		echo '</select>';
		return true;

	}//end paintYearDrillDownView()


	/**
	* Paint the interface where you choose the view you're taken to when you click on a day in year view
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processYearDrillDownView(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_year_drill_down_view'])) {
			$enabled_views = $asset->attr('enabled_views');
			if ((isset($enabled_views[$_POST[$prefix.'_year_drill_down_view']]) && $enabled_views[$_POST[$prefix.'_year_drill_down_view']]) || empty($_POST[$prefix.'_year_drill_down_view'])) {
				$asset->setAttrValue('year_drill_down_view', $_POST[$prefix.'_year_drill_down_view']);
				return true;
			}
		}
		return false;

	}//end processYearDrillDownView()


	/**
	* Paint the interface that lets you specify the date format for the day view's title
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDayTitleFormat(&$asset, &$o, $prefix)
	{
		return $this->_paintTitleFormat('day', $asset, $o, $prefix);

	}//end paintDayTitleFormat()


	/**
	* Process the interface that lets you specify the date format for the day view's title
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processDayTitleFormat(&$asset, &$o, $prefix)
	{
		return $this->_processTitleFormat('day', $asset, $o, $prefix);

	}//end paintDayTitleFormat()
	

	/**
	* Paint the interface that lets you specify the date format for the week view's title
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintWeekTitleFormat(&$asset, &$o, $prefix)
	{
		return $this->_paintTitleFormat('week', $asset, $o, $prefix);

	}//end paintweekTitleFormat()


	/**
	* Process the interface that lets you specify the date format for the week view's title
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processWeekTitleFormat(&$asset, &$o, $prefix)
	{
		return $this->_processTitleFormat('week', $asset, $o, $prefix);

	}//end paintweekTitleFormat()
	

	/**
	* Paint the interface that lets you specify the date format for the month view's title
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintMonthTitleFormat(&$asset, &$o, $prefix)
	{
		return $this->_paintTitleFormat('month', $asset, $o, $prefix);

	}//end paintmonthTitleFormat()


	/**
	* Process the interface that lets you specify the date format for the month view's title
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processMonthTitleFormat(&$asset, &$o, $prefix)
	{
		return $this->_processTitleFormat('month', $asset, $o, $prefix);

	}//end paintmonthTitleFormat()
	

	/**
	* Paint the interface that lets you specify the date format for the year view's title
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintYearTitleFormat(&$asset, &$o, $prefix)
	{
		return $this->_paintTitleFormat('year', $asset, $o, $prefix);

	}//end paintyearTitleFormat()


	/**
	* Process the interface that lets you specify the date format for the year view's title
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processYearTitleFormat(&$asset, &$o, $prefix)
	{
		return $this->_processTitleFormat('year', $asset, $o, $prefix);

	}//end paintyearTitleFormat()


	/**
	* Generic function to paint the interface for specifying a view's title format
	*
	* @param string						$view	the view this is the interface for
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access private
	*/
	function _paintTitleFormat($view, &$asset, &$o, $prefix)
	{
		$title_formats = $asset->attr('view_title_formats');
		if ($asset->writeAccess('attributes')) {
			text_box($prefix.'_title_format_'.$view, $title_formats[$view]);
			?><div class="sq-backend-smallprint">The date format to use in this view's title, expressed using the date format symbols from the <a target="_blank" href="http://au.php.net/manual/en/function.date.php">PHP date function</a><?php
		} else {
			echo $title_formats[$view].'  ("'.date($title_formats[$view], ((($view == 'week') && (date('D') != 'Mon')) ? strtotime('last Monday') : time())).'")';
		}
		return true;

	}//end _paintTitleFormat()


	/**
	* Generic function to process the interface for specifying a view's title format
	*
	* @param string						$view	the view this is the interface for
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access private
	*/
	function _processTitleFormat($view, &$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_title_format_'.$view])) {
			$title_formats = $asset->attr('view_title_formats');
			$title_formats[$view] = $_POST[$prefix.'_title_format_'.$view];
			$asset->setAttrValue('view_title_formats', $title_formats);
			return true;
		} else {
			return false;
		}
	}//end _processTitleFormat()


	/**
	* Paint the interface that lets you choose which root nodes to columnise by in day view
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintColumnNodes(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$all_root_nodes = $asset->attr('root_nodes');
		$current_column_nodes = $asset->attr('columnise_day_view_by_root_node');
		$location_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($all_root_nodes));
		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header">Root Node</td>
				<td class="sq-backend-table-header">Show in Own Column?</td>
			</tr>
		<?php
		foreach ($all_root_nodes as $id => $details) {
			?>
			<tr>
				<td class="sq-backend-table-cell"><?php echo $location_info[$id]['name']; ?></td>
				<td class="sq-backend-table-cell">
					<?php
						if ($write_access) {
							check_box($prefix.'_root_nodes_columnised['.$id.']', '1', in_array($id, $current_column_nodes));
						} else {
							echo '<img src="'.sq_web_path('lib').'/web/images/'.(in_array($id, $current_column_nodes) ? 'tick' : 'cross').'.gif" width="15" height="15" />';
						}
					?>
				</td>
			</tr>
			<?php
		}
		?>
			<tr>
				<td class="sq-backend-table-cell">'Other'</td>
				<td class="sq-backend-table-cell">
					<?php
						if ($write_access) {
							check_box($prefix.'_root_nodes_columnised[other]', '1', in_array('other', $current_column_nodes));
						} else {
							echo '<img src="'.sq_web_path('lib').'/web/images/'.(in_array('other', $current_column_nodes) ? 'tick' : 'cross').'.gif" width="15" height="15" />';
						}
					?>
				</td>
			</tr>
		</table>
		<?php
	}


	/**
	* Process the interface that lets you choose which root nodes to columnise by in day view
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processColumnNodes(&$asset, &$o, $prefix)
	{
		$val = Array('other');
		if (isset($_REQUEST[$prefix.'_root_nodes_columnised']) && is_array($_REQUEST[$prefix.'_root_nodes_columnised']))  {
			if (!(isset($_REQUEST[$prefix.'_root_nodes_columnised']['other']) && $_REQUEST[$prefix.'_root_nodes_columnised']['other'])) { 
				$val = Array();
				unset($_REQUEST[$prefix.'_root_nodes_columnised']['other']);
			}
			foreach ($_REQUEST[$prefix.'_root_nodes_columnised'] as $id => $status) {
				if ($status) $val[] = $id;
			}
		}
		return $asset->setAttrValue('columnise_day_view_by_root_node', $val);
	}
	
}//end class

?>