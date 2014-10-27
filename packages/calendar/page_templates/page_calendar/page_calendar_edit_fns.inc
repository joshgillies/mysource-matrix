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
* $Id: page_calendar_edit_fns.inc,v 1.47 2012/08/30 00:57:28 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Page_Calendar_Edit_Fns
*
* Purpose
*	Functions to help with painting and processing the calendar page backend interface
*
* @author  Luke Wright <lwright@squiz.net>
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.47 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Page_Calendar_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		$this->static_screens['details']['force_unlock'] = 0;

	}//end constructor


	/**
	* Paint the interface for setting the root nodes from which this calendar page draws events
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
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
				<td class="sq-backend-table-header"><?php echo translate('Location to draw events from'); ?></td>

				<td class="sq-backend-table-header"><?php echo translate('CSS Class to apply to the events\' display'); ?></td>

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
						echo get_asset_tag_line($location);
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
		<?php
		return TRUE;

	}//end paintRootNodes()


	/**
	* Process the interface for setting the root nodes from which this calendar page draws events
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processRootNodes(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_root_nodes']) || (isset($asset->_tmp['reverting_to_system_version']) && $asset->_tmp['reverting_to_system_version'])) {
			$locations = Array();
			foreach ($_POST[$prefix.'_root_nodes'] as $index => $loc) {
				if ($locid = $loc['assetid']) {
					$locations[$locid] = isset($_POST[$prefix.'_node_classes'][$index]) ? Array('class_name' => $_POST[$prefix.'_node_classes'][$index]) : Array('class_name' => '');
				}
			}
			return $asset->setAttrValue('root_nodes', $locations);
		} else {
			return FALSE;
		}

	}//end processRootNodes()


	/**
	* Paint the interface for choosing which calendar views are available on the frontend
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintViewsChooser(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$current_views = $asset->attr('enabled_views');
		$listing_entry_options = Array(
									'title'		=> translate('Show Title Only'),
									'bodycopy'	=> translate('Use Type Format Bodycopy'),
								 );

		$view_names = Array(
			'year'  => translate('Year View'),
			'month' => translate('Month View'),
			'week'  => translate('Week View'),
			'day'   => translate('Day View'),
			'event' => translate('Individual Event View'),
		);

		$current_list_entry_types = $asset->attr('list_entry_types');
		?>
		<table class="sq-backend-table">
			<?php
			$view_options = $asset->getViewOptions();
			foreach ($view_options as $category => $options) {
				?>
				<tr>
					<td>
						<b><label for="<?php echo $prefix.'_enabled_views['.$category.']'; ?>"><?php echo $view_names[$category]; ?></label></b>
					</td>
					<td>
						<?php
						if ($write_access) {
							combo_box($prefix.'_enabled_views['.$category.']', $options, 0,  $current_views[$category], 0, 'style="width: 46ex;"');
						} else {
							echo $view_options[$category][$current_views[$category]];
						}
						?>
					</td>
					<td><?php
						if ($write_access && (strpos($current_views[$category], 'list') !== FALSE)) {
							ob_start();
							combo_box($prefix.'_list_entry_types['.$category.']', $listing_entry_options, 0,  $current_list_entry_types[$category], 0);
							$list_entry_cb = ob_get_contents();
							ob_end_clean();
							echo sprintf(translate('For each list entry, %s'), $list_entry_cb);


						} else if (strpos($current_views[$category], 'list') !== FALSE) {
							echo sprintf(translate('For each entry, %s'), strtolower($listing_entry_options[$current_list_entry_types[$category]]));

						} else {
							echo '&nbsp;';
						}
						?>
					</td>
				</tr><?php
			}//end foreach
			?>
		</table>
		<?php
		return TRUE;

	}//end paintViewsChooser()


	/**
	* Process the interface for choosing which calendar views are available on the frontend
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processViewsChooser(&$asset, &$o, $prefix)
	{
		$list_entry_types = $asset->attr('list_entry_types');
		$unwanted_elts = array_diff(array_keys($_POST[$prefix.'_enabled_views']), array_keys($asset->getViewOptions()));
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
			foreach ($list_entry_types as $key => $value) {
				if (isset($_POST[$prefix.'_list_entry_types'][$key]) && in_array($_POST[$prefix.'_list_entry_types'][$key], Array('title', 'bodycopy'))) {
					$list_entry_types[$key] = $_POST[$prefix.'_list_entry_types'][$key];
				}
			}
			$asset->setAttrValue('list_entry_types', $list_entry_types);

			return TRUE;
		}
		return FALSE;

	}//end processViewsChooser()


	/**
	* Paint the interface for choosing which view the frontend user sees first
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintInitialView(&$asset, &$o, $prefix)
	{
		$selected_view = $asset->attr('initial_view');
		if ($asset->writeAccess('attributes')) {
			$view_options = $asset->getViewOptions();
			$enabled_views = $asset->attr('enabled_views');
			foreach ($enabled_views as $name => $status) {
				if (empty($status)) unset($view_options[$name]);
			}
			foreach ($view_options as $key => $val) {
				$view_options[$key] = ucfirst($key).' View';
			}
			if (!in_array($selected_view, array_keys($view_options))) {
				$selected_view = current($view_options);
			}
			combo_box($prefix.'_initial_view', $view_options, 0, $selected_view, 0);
		} else {
			echo $selected_view;
		}
		return TRUE;

	}//end paintInitialView()


	/**
	* Process the interface for choosing which view the frontend user sees first
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
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
	* @param string		$prefix			prefix for the form elements
	* @param int		$default_hour	the default value for the hour field
	* @param int		$default_min	the default value for the minutes field
	* @param boolean	$default_is_pm	whether the default time is pm
	*
	* @return void
	* @access private
	*/
	function _paint5MinTimeField($prefix, $default_hour=9, $default_min=0, $default_is_pm=FALSE)
	{
		$hour_options = Array(12=>12, 1=>1, 2=>2, 3=>3, 4=>4, 5=>5, 6=>6, 7=>7, 8=>8, 9=>9, 10=>10, 11=>11, 12=>12);
		$min_options = Array();
		for ($i=0; $i < 60; $i += 5) {
			$min_options[sprintf('%02d', $i)] = sprintf('%02d', $i);
		}
		$is_pm_options = Array(0=>'am', 1=>'pm');
		combo_box($prefix.'[hours]', $hour_options, FALSE, $default_hour, 0);
		echo ' : ';
		combo_box($prefix.'[minutes]', $min_options, FALSE, sprintf('%02d', $default_min), 0);
		echo '&nbsp;';
		combo_box($prefix.'[is_pm]', $is_pm_options, FALSE, (int)$default_is_pm, 0);

	}//end _paint5MinTimeField()


	/**
	* Process a 5-minute-precision time-choosing field
	* Returns: HH:MM value of the element, in 24 hour format
	*
	* @param string	$prefix	prefix for the form elements
	*
	* @return string
	* @access public
	*/
	function _process5MinTimeField($prefix)
	{
		if (!isset($_POST[$prefix])) return FALSE;
		if ($_POST[$prefix]['hours'] != (int)$_POST[$prefix]['hours']) {
			return FALSE;
		}
		if ($_POST[$prefix]['minutes'] != (int)$_POST[$prefix]['minutes']) {
			return FALSE;
		}
		if ($_POST[$prefix]['is_pm'] && ($_POST[$prefix]['hours'] != 12)) {
			$_POST[$prefix]['hours'] += 12;
		}
		if (!$_POST[$prefix]['is_pm'] && ($_POST[$prefix]['hours'] == 12)) {
			$_POST[$prefix]['hours'] = 0;
		}
		return sprintf('%02d', $_POST[$prefix]['hours']).':'.$_POST[$prefix]['minutes'];

	}//end _process5MinTimeField()


	/**
	* Paint the interface for choosing the earliest time to be displayed in a day
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDayStart(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			list($hour,$minute) = sscanf($asset->attr('day_starts_at'),'%02d:%02d');
			$is_pm = FALSE;
			if ($hour >= 12) {
				$is_pm = TRUE;
				$hour -= 12;
			}
			if ($hour == 0) $hour = 12;

			$this->_paint5MinTimeField($prefix.'_day_start_time', $hour, $minute, $is_pm);
		} else {
			echo $asset->attr('day_starts_at');
		}
		return TRUE;

	}//end paintDayStart()


	/**
	* Process the interface for choosing the earliest time to be displayed in a day
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processDayStart(&$asset, &$o, $prefix)
	{
		if ($value = $this->_process5MinTimeField($prefix.'_day_start_time')) {
			$asset->setAttrValue('day_starts_at', $value);
			return TRUE;
		} else {
			return FALSE;
		}

	}//end processDayStart()


	/**
	* Paint the interface for choosing the latest time to be displayed in a day
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDayEnd(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			list($hour,$minute) = sscanf($asset->attr('day_ends_at'),'%02d:%02d');
			$is_pm = FALSE;
			if ($hour >= 12) {
				$is_pm = TRUE;
				$hour -= 12;
			}
			if ($hour == 0) $hour = 12;

			$this->_paint5MinTimeField($prefix.'_day_end_time', $hour, $minute, $is_pm);
		} else {
			echo $asset->attr('day_ends_at');
		}

	}//end paintDayEnd()


	/**
	* Process the interface for choosing the latest time to be displayed in a day
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processDayEnd(&$asset, &$o, $prefix)
	{
		if ($value = $this->_process5MinTimeField($prefix.'_day_end_time')) {
			if ($value == '00:00') $value = '24:00';
			$asset->setAttrValue('day_ends_at', $value);
			return TRUE;
		} else {
			return FALSE;
		}

	}//end processDayEnd()


	/**
	* Paint the interface for choosing the days to include in the week view
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
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
		return TRUE;

	}//end paintWeekDays()


	/**
	* Process the interface for choosing the days to include in the week view
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processWeekDays(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_show_days']) && is_array($_POST[$prefix.'_show_days'])) {
			foreach (range(0, 6) as $day_number) {
				if (!isset($_POST[$prefix.'_show_days'][$day_number])) {
					$_POST[$prefix.'_show_days'][$day_number] = 0;
				}
			}
			$asset->setAttrValue('week_view_show_days', $_POST[$prefix.'_show_days']);
			return TRUE;
		} else {
			return FALSE;
		}

	}//end processWeekDays()


	/**
	* Paint the explanation of the CSS implementation and link to the CSS
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintLookAndFeel(&$asset, &$o, $prefix)
	{
		?>
		<p style="width: 80%; margin-left: 15ex"><?php echo translate('The Calendar Page has a default CSS file that controls the look and feel of the Calendar views.  A link to this CSS file is automatically inserted into the head of the generated HTML document.'); ?></p>


		<p style="width: 80%; margin-left: 15ex"><?php echo sprintf(translate('If you want to customise some or all of the style rules you can <a href="%s">download the default CSS file</a> and modify it.  Put a link to your modified version in the parse file of the design that applies to this page, <i>after</i> the "head" design area tag.'), sq_web_path('data').'/asset_types/page_calendar/css/calendar_default.css'); ?></p>


		<p style="width: 80%; margin-left: 15ex"><?php echo translate('<b>Note:</b> For week and day view calendar tables to render correctly, your site design <i>must</i> put the browser in a mode that correctly implements the W3C box model, eg. by specifying the XHTML Strict doctype.'); ?></p>

		<?php

	}//end paintLookAndFeel()


	/**
	* Does nothing
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processLookAndFeel(&$asset, &$o, $prefix)
	{
		return TRUE;

	}//end processLookAndFeel()


	/**
	* Paint the interface that lets you specify the date format for the day view's title
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
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
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processDayTitleFormat(&$asset, &$o, $prefix)
	{
		return $this->_processTitleFormat('day', $asset, $o, $prefix);

	}//end processDayTitleFormat()


	/**
	* Paint the interface that lets you specify the date format for the week view's title
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintWeekTitleFormat(&$asset, &$o, $prefix)
	{
		return $this->_paintTitleFormat('week', $asset, $o, $prefix);

	}//end paintWeekTitleFormat()


	/**
	* Process the interface that lets you specify the date format for the week view's title
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processWeekTitleFormat(&$asset, &$o, $prefix)
	{
		return $this->_processTitleFormat('week', $asset, $o, $prefix);

	}//end processWeekTitleFormat()


	/**
	* Paint the interface that lets you specify the date format for the month view's title
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintMonthTitleFormat(&$asset, &$o, $prefix)
	{
		return $this->_paintTitleFormat('month', $asset, $o, $prefix);

	}//end paintMonthTitleFormat()


	/**
	* Process the interface that lets you specify the date format for the month view's title
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processMonthTitleFormat(&$asset, &$o, $prefix)
	{
		return $this->_processTitleFormat('month', $asset, $o, $prefix);

	}//end processMonthTitleFormat()


	/**
	* Paint the interface that lets you specify the date format for the days shown in the month view
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintMonthDayHeadingFormat(&$asset, &$o, $prefix)
	{
		return $this->_paintHeadingFormat('month', 'day', $asset, $o, $prefix);

	}//end paintMonthDayHeadingFormat()


	/**
	* Process the interface that lets you specify the date format for the days shown in the month view
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processMonthDayHeadingFormat(&$asset, &$o, $prefix)
	{
		return $this->_processHeadingFormat('month', 'day', $asset, $o, $prefix);

	}//end processMonthDayHeadingFormat()


	/**
	* Paint the interface that lets you specify the date format for the days shown in the week view
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintWeekDayHeadingFormat(&$asset, &$o, $prefix)
	{
		return $this->_paintHeadingFormat('week', 'day', $asset, $o, $prefix);

	}//end paintWeekDayHeadingFormat()


	/**
	* Process the interface that lets you specify the date format for the days shown in the week view
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processWeekDayHeadingFormat(&$asset, &$o, $prefix)
	{
		return $this->_processHeadingFormat('week', 'day', $asset, $o, $prefix);

	}//end processWeekDayHeadingFormat()


	/**
	* Paint the interface that lets you specify the date format for the year view's title
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintYearTitleFormat(&$asset, &$o, $prefix)
	{
		return $this->_paintTitleFormat('year', $asset, $o, $prefix);

	}//end paintYearTitleFormat()


	/**
	* Process the interface that lets you specify the date format for the year view's title
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processYearTitleFormat(&$asset, &$o, $prefix)
	{
		return $this->_processTitleFormat('year', $asset, $o, $prefix);

	}//end processYearTitleFormat()


	/**
	* Generic function to paint the interface for specifying a view's title format
	*
	* @param string	$view	the view this is the interface for
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access private
	*/
	function _paintTitleFormat($view, &$asset, &$o, $prefix)
	{
		$title_formats = $asset->attr('view_title_formats');
		if ($asset->writeAccess('attributes')) {
			text_box($prefix.'_title_format_'.$view, $title_formats[$view]);
			?><div class="sq-backend-smallprint"><?php echo sprintf(translate('The date format to use in this view\'s title, expressed using the date format symbols from the <a target="_blank" href="%s">PHP date function</a>'), 'http://au.php.net/manual/en/function.date.php'); ?></div><?php

		} else {
			echo $title_formats[$view].'  ("'.date($title_formats[$view], ((($view == 'week') && (date('D') != 'Mon')) ? strtotime('last Monday') : time())).'")';
		}
		return TRUE;

	}//end _paintTitleFormat()


	/**
	* Generic function to process the interface for specifying a view's title format
	*
	* @param string	$view	the view this is the interface for
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
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
			return TRUE;
		} else {
			return FALSE;
		}

	}//end _processTitleFormat()


	/**
	* Generic function to paint the interface for specifying a view's heading format
	*
	* @param string	$view			the view this is the interface for
	* @param string	$time_period	the time period used (eg; 'day' for day heading)
	* @param object	&$asset			the asset whose interface we are painting
	* @param object	&$o				the outputter class
	* @param string	$prefix			prefix for the form elements
	*
	* @return boolean
	* @access private
	*/
	function _paintHeadingFormat($view, $time_period, &$asset, &$o, $prefix)
	{
		$heading_index = $view.'_'.$time_period;
		$heading_formats = $asset->attr('view_heading_formats');
		if ($asset->writeAccess('attributes')) {
			text_box($prefix.'_heading_format_'.$heading_index, $heading_formats[$heading_index]);
			?><div class="sq-backend-smallprint"><?php echo sprintf(translate('The date format to use for an element of this view (eg; day in month), expressed using the date format symbols from the <a target="_blank" href="%s">PHP date function</a>'), 'http://au.php.net/manual/en/function.date.php'); ?><?php

		} else {
			echo $heading_formats[$heading_index].'  ("'.date($heading_formats[$heading_index], ((($view == 'week') && (date('D') != 'Mon')) ? strtotime('last Monday') : time())).'")';
		}
		return TRUE;

	}//end _paintHeadingFormat()


	/**
	* Generic function to process the interface for specifying a view's heading format
	*
	* @param string	$view			the view this is the interface for
	* @param string	$time_period	the time period used (eg; 'day' for day heading)
	* @param object	&$asset			the asset whose interface we are painting
	* @param object	&$o				the outputter class
	* @param string	$prefix			prefix for the form elements
	*
	* @return boolean
	* @access private
	*/
	function _processHeadingFormat($view, $time_period, &$asset, &$o, $prefix)
	{
		$heading_index = $view.'_'.$time_period;
		if (isset($_POST[$prefix.'_heading_format_'.$heading_index])) {
			$heading_formats = $asset->attr('view_heading_formats');
			$heading_formats[$heading_index] = $_POST[$prefix.'_heading_format_'.$heading_index];
			$asset->setAttrValue('view_heading_formats', $heading_formats);
			return TRUE;
		} else {
			return FALSE;
		}

	}//end _processHeadingFormat()


	/**
	* Generic function to paint the interface for specifying limit range
	*
	* @param string	$view		view of navigation limit attribute
	* @param object	&$asset		the asset whose interface we are painting
	* @param object	&$o			the outputter class
	* @param string	$prefix		prefix for the form elements
	*
	* @return boolean
	* @access private
	*/
	function _paintNavigationLimit($view, &$asset, &$o, $prefix)
	{
		$date_parts = Array(
			'year'  => 'years',
			'month' => 'months',
			'week'  => 'weeks',
			'day'   => 'days',
		);

		$navi_limit = $asset->attr($view.'_navi_limit');
		if ($asset->writeAccess('attributes')) {
			text_box($prefix.$view.'_limit_backward', $navi_limit['backward'],2,2);
			label('&nbsp;'.sprintf(translate('%s backward'), $date_parts[$view]), $prefix.$view.'_limit_backward');
			echo '&nbsp;';
			text_box($prefix.$view.'_limit_forward', $navi_limit['forward'],2,2);
			label('&nbsp;'.sprintf(translate('%s forward'), $date_parts[$view]), $prefix.$view.'_limit_forward');
		} else {
			echo $navi_limit['forward'].' '.sprintf(translate('%s forward'), $date_parts[$view]).', ';
			echo $navi_limit['backward'].' '.sprintf(translate('%s backward'), $date_parts[$view]);
		}
		return TRUE;

	}//end _paintNavigationLimit()


	/**
	* Generic function to process the interface for specifying limit range
	*
	* @param string	$view			view of navigation limit attribute
	* @param object	&$asset			the asset whose interface we are painting
	* @param object	&$o				the outputter class
	* @param string	$prefix			prefix for the form elements
	*
	* @return boolean
	* @access private
	*/
	function _processNavigationLimit($view, &$asset, &$o, $prefix)
	{
		$navi_limit = $asset->attr($view.'_navi_limit');
		if (($_POST[$prefix.$view.'_limit_forward'] == $navi_limit['forward']) && ($_POST[$prefix.$view.'_limit_backward'] == $navi_limit['backward'])) {
			return FALSE;
		} else {
			if (isset($_POST[$prefix.$view.'_limit_forward'])) {
				$navi_limit['forward'] = $_POST[$prefix.$view.'_limit_forward'];
			}
			if (isset($_POST[$prefix.$view.'_limit_backward'])) {
				$navi_limit['backward'] = $_POST[$prefix.$view.'_limit_backward'];
			}
			$asset->setAttrValue($view.'_navi_limit', $navi_limit);
			return TRUE;
		}

	}//end _processNavigationLimit()


	/**
	* Paint the interface that lets you choose which root nodes to columnise by in day view
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
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
				<td class="sq-backend-table-header"><?php echo translate('Event Source'); ?></td>

				<td class="sq-backend-table-header"><?php echo translate('Show in Own Column ?'); ?></td>

			</tr>
		<?php
		foreach ($all_root_nodes as $id => $details) {
			?>
			<tr>
				<td class="sq-backend-table-cell"><?php echo get_asset_tag_line($id); ?></td>
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
							check_box($prefix.'_root_nodes_columnised[other]', '1', in_array('*', $current_column_nodes));
						} else {
							echo '<img src="'.sq_web_path('lib').'/web/images/'.(in_array('*', $current_column_nodes) ? 'tick' : 'cross').'.gif" width="15" height="15" />';
						}
					?>
				</td>
			</tr>
		</table>
		<?php

	}//end paintColumnNodes()


	/**
	* Process the interface that lets you choose which root nodes to columnise by in day view
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processColumnNodes(&$asset, &$o, $prefix)
	{
		$val = Array();
		if (isset($_REQUEST[$prefix.'_root_nodes_columnised']) && is_array($_REQUEST[$prefix.'_root_nodes_columnised'])) {
			if ((isset($_REQUEST[$prefix.'_root_nodes_columnised']['other']) && $_REQUEST[$prefix.'_root_nodes_columnised']['other'])) {
				$val = Array('*');
				unset($_REQUEST[$prefix.'_root_nodes_columnised']['other']);
			}
			foreach ($_REQUEST[$prefix.'_root_nodes_columnised'] as $id => $status) {
				if ($status) $val[] = $id;
			}
		}
		if (empty($val)) $val = Array('*');

		return $asset->setAttrValue('columnise_day_view_by_root_node', $val);

	}//end processColumnNodes()


	/**
	* Paint the interface that lets you specify the year limit for navigating calendar page
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintYearNaviLimit(&$asset, &$o, $prefix)
	{
		return $this->_paintNavigationLimit('year', $asset, $o, $prefix);

	}//end paintYearNaviLimit()


	/**
	* Process the interface that lets you specify the year limit for navigating calendar page
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processYearNaviLimit(&$asset, &$o, $prefix)
	{
		return $this->_processNavigationLimit('year', $asset, $o, $prefix);

	}//end processYearNaviLimit()


	/**
	* Paint the interface that lets you specify the month limit for navigating calendar page
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintMonthNaviLimit(&$asset, &$o, $prefix)
	{
		return $this->_paintNavigationLimit('month', $asset, $o, $prefix);

	}//end paintMonthNaviLimit()


	/**
	* Process the interface that lets you specify the month limit for navigating calendar page
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processMonthNaviLimit(&$asset, &$o, $prefix)
	{
		return $this->_processNavigationLimit('month', $asset, $o, $prefix);

	}//end processMonthNaviLimit()


	/**
	* Paint the interface that lets you specify the week limit for navigating calendar page
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintWeekNaviLimit(&$asset, &$o, $prefix)
	{
		return $this->_paintNavigationLimit('week', $asset, $o, $prefix);

	}//end paintWeekNaviLimit()


	/**
	* Process the interface that lets you specify the week limit for navigating calendar page
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processWeekNaviLimit(&$asset, &$o, $prefix)
	{
		return $this->_processNavigationLimit('week', $asset, $o, $prefix);

	}//end processWeekNaviLimit()


	/**
	* Paint the interface that lets you specify the day limit for navigating calendar page
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDayNaviLimit(&$asset, &$o, $prefix)
	{
		return $this->_paintNavigationLimit('day', $asset, $o, $prefix);

	}//end paintDayNaviLimit()


	/**
	* Process the interface that lets you specify the day limit for navigating calendar page
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processDayNaviLimit(&$asset, &$o, $prefix)
	{
		return $this->_processNavigationLimit('day', $asset, $o, $prefix);

	}//end processDayNaviLimit()


	/**
	* Paint "use no result" bodycopy interface
	*
	* Return boolean FALSE if write access to links is not held, TRUE otherwise
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintUseNoResultBodycopy(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('links');
		$current_value = $asset->attr('use_no_results_body_copy');
		$options = Array(1 => 'Yes', 0 => 'No');

		if ($write_access) {
			combo_box($prefix.'_use_no_results_body_copy', $options, FALSE,  $current_value);
		} else {
			echo $options[$current_value];
		}

		return $write_access;

	}//end paintUseBodycopy()


	/**
	* Process "use no result" bodycopy interface
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processUseNoResultBodycopy(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_use_no_results_body_copy'])) {
			if ($asset->createNoResultsBodycopy()) {
				$asset->setAttrValue('use_no_results_body_copy',$_POST[$prefix.'_use_no_results_body_copy']);
				$asset->saveAttributes();
			}
		}

		return TRUE;

	}//end processUseBodycopy()

}//end class
?>
