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
* $Id: page_calendar.inc,v 1.23 2004/08/24 07:01:03 lwright Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_asset_listing/page_asset_listing.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once SQ_FUDGE_PATH.'/general/datetime.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Page_Calendar
*
* Purpose
*
* @author  Luke Wright <lwright@squiz.net>
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.23 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Calendar extends Page_Asset_Listing
{
	
	
	/**
	* All the view options available
	* @var array
	*/
	var $view_options = Array(	
		'year'	=> Array(
			''						=> '(none)',
			'list_without_headings'	=> 'List without Headings', 
			'list_with_headings'	=> 'List with Month Headings', 
			'calendar_format'		=> 'Calendar Format'),
		'month' => Array(
			''						=> '(none)',
			'list_without_headings'	=> 'List without Headings',
			'list_with_headings'	=> 'List with Day Headings',
			'calendar_format'		=> 'Calendar Format'),
		'week' => Array(	
			''						=> '(none)',
			'list_without_headings'	=> 'List without Headings',
			'list_with_headings'	=> 'List with Day Headings',
			'calendar_format'		=> 'Calendar Format'),
		'day' => Array(
			''						=> '(none)',
			'list_without_headings'	=> 'List without Headings',
			'calendar_format'		=> 'Calendar Format'),
		'event' => Array(	
			''						=> '(none)',
			'single_event'			=> 'Single Event Display')
	);

	
	/**
	* List of full day names, in order of date() weekday number
	* @var array
	*/
	var $day_names_full = Array('Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday');
	
	
	/**
	* List of abbreviated day names, in order of date() weekday number
	* @var array
	*/
	var $day_names_short = Array('Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat');
	
	
	/**
	* List of single-character day names, in order of date() weekday number
	* @var array
	*/
	var $day_names_very_short = Array('S', 'M', 'T', 'W', 'T', 'F', 'S');

	
	/**
	* List of full month names, in chronological order
	* @var array
	*/
	var $month_names_full = Array('', 'January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September', 'October', 'November', 'December');
	
	
	/**
	* List of abbreviated month names, in chronological order
	* @var array
	*/
	var $month_names_short = Array('', 'Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec');

	/**
	* An array of keywords that this asset makes available for use along with a default replacement
	* @var Array(keyword => Array('name' => short description, 'default' => default replacement, 'type' => '[inline|block]'))
	*/
	var $_available_keywords = Array('calendar_contents' => Array(
										'name' => 'The calendar content for the current view',
										'default' => '(calendar content)',
										'type' => 'block'),
									 'calendar_title' => Array(
										'name' => 'The calendar title for the current view',
										'default' => 'Calendar Title',
										'type' => 'inline')
									);
	

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Page_Calendar($assetid=0)
	{
		$this->Page_Asset_Listing($assetid);

	}//end Page_Calendar()




	/**
	* Create this asset
	*
	* @param array()	&$link	information used to create the initial link
	*
	* @see Asset::create()
	* @return mixed int or false
	* @access public
	*/
	function create(&$link)
	{
		// note our event types and that they're all customised
		$this->setAttrValue('types', Array('calendar_event_single'=>1, 'calendar_event_recurring'=>1));
		$this->setAttrValue('customised', Array('calendar_event_single', 'calendar_event_recurring'));

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$linkid = Page::create($link);
		if (!$linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');

		// create the page contents bodycopy and the event formats folder
		$sub_assets= Array(
			'type_formats'  => 'folder',
			'page_contents' => 'bodycopy',
		);
		$type_formats = null;
		foreach ($sub_assets as $name => $type) {
			$asset = new $type();
			$copy_link = Array('asset' => &$this, 'value' => $name ,'link_type' => SQ_LINK_TYPE_2, 'dependant' => 1, 'exclusive' => 1);

			$asset->setAttrValue('name', ucwords(str_replace('_',' ', $name)));
			if (!$asset->create($copy_link)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$this->id = 0;
				return false;
			}
			if ($name == 'type_formats') {
				$type_formats = $asset;
			}
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			unset($asset);
		}

		// put the default contents into the page contents bodycopy
		$content_ids = array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($this->id, 'content_type', false));
		if (empty($content_ids)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}
		$content_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($content_ids[0]);
		if (!$content_asset->id) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}
		if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($content_asset->id, 'attributes')) { 
			trigger_error('Asset Builder Upgrade [to 0.4] Failed - Unable to Acquire Lock on "'.$asset->name.'" (#'.$asset->id.')', E_USER_WARNING);
			return false;
		}
		$content_asset->setAttrValue('html', '<h2 style="text-align: center">%calendar_title%</h2><p style="text-align: center">%prev_link%&nbsp;&nbsp;&nbsp;&nbsp;%up_link%&nbsp;&nbsp;&nbsp;&nbsp;%next_link%</p>%calendar_contents%');
		$content_asset->saveAttributes();
		$content_parents = $GLOBALS['SQ_SYSTEM']->am->getParents($content_asset->id, 'bodycopy_container', false);
		$parent_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($content_parents[0]);
		$edit_fns = &$parent_asset->getEditFns();
		$edit_fns->generateContentFile($parent_asset);
		$GLOBALS['SQ_SYSTEM']->am->releaseLock($content_asset->id, 'attributes'); 

		// create the event format bodycopies
		$copy_link = Array('asset' => &$type_formats, 'value' => '' ,'link_type' => SQ_LINK_TYPE_2, 'dependant' => 1, 'exclusive' => 1);
		$event_types = Array('single', 'recurring');
		foreach ($event_types as $event_type) {
			$copy_link['value'] = 'calendar_event_'.$event_type;
			$asset = new Bodycopy();
			$asset->setAttrValue('name', ucfirst($event_type).' Event Format');
			if (!$asset->create($copy_link)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$this->id = 0;
				return false;
			}
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			unset($asset);
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return $linkid;

	}//end create()


	/**
	* Called by the design to print the head of this asset
	* 
	* @return boolean
	* @access public
	*/
	function printHead()
	{
		echo '<link rel="stylesheet" type="text/css" href="' . sq_web_path('data').'/asset_types/page_calendar/css/calendar_default.css" />';
	}


	/**
	* Called by the design to print the body of this asset
	* 
	* @return boolean
	* @access public
	*/
	function printBody()
	{

		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'page_contents'); 
		if (empty($bodycopy_link)) return false;

		$format_bodycopy = &$GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link['minorid'], $bodycopy_link['minor_type_code']);
		if (is_null($format_bodycopy)) return false;

		// If we get a valid and available view type in the request, use it; otherwise use the initial view attribute.
		$enabled_views = $this->attr('enabled_views');
		$cal_view = empty($enabled_views[$_REQUEST['SQ_CALENDAR_VIEW']]) ? $this->attr('initial_view') : $_REQUEST['SQ_CALENDAR_VIEW'];
		
		// If we get a valid date in the request, use it; otherwise use today's date
		$cal_date = (isset($_REQUEST['SQ_CALENDAR_DATE']) && is_iso8601($_REQUEST['SQ_CALENDAR_DATE'])) ? $_REQUEST['SQ_CALENDAR_DATE'] : date('Y-m-d');
		list($year, $month, $day) = explode('-', $cal_date);
		if (!checkdate($month, $day, $year)) $cal_date = date('Y-m-d');

		// BUT, if the view is 'event' we want the eventid rather than the date
		$event_id = false;
		if ($cal_view == 'event') {
			if (isset($_REQUEST['SQ_CALENDAR_EVENT_ID'])) {
				$event_id = (int)$_REQUEST['SQ_CALENDAR_EVENT_ID'];
			} else {
				trigger_error('Event View requested but no Event ID supplied', E_USER_WARNING);
				return false;
			}
		}

		$func = 'get'.ucfirst($cal_view).'View';
		$replacements['calendar_contents'] = '<div id="calendarContents">'.(($cal_view == 'event') ? $this->$func($event_id) : $this->$func($cal_date)).'</div>';

		$replacements['calendar_title'] = $this->getViewTitle($cal_view, $cal_date, $event_id);

		$prev_date = date('Y-m-d', strtotime('-1 '.$cal_view, strtotime($cal_date)));
		$next_date = date('Y-m-d', strtotime('+1 '.$cal_view, strtotime($cal_date)));
		$replacements['prev_link'] = $event_id ? '' : '<a class="calendarNavLink"  href="'.$this->getHref().'?SQ_CALENDAR_VIEW='.$cal_view.'&SQ_CALENDAR_DATE='.$prev_date.'">'.str_replace('%v%', ucfirst($cal_view), $this->attr('prev_link_text')).'</a>';
		$replacements['next_link'] = $event_id ? '' : '<a class="calendarNavLink"  href="'.$this->getHref().'?SQ_CALENDAR_VIEW='.$cal_view.'&SQ_CALENDAR_DATE='.$next_date.'">'.str_replace('%v%', ucfirst($cal_view), $this->attr('next_link_text')).'</a>';
		$enabled_views = $this->attr('enabled_views');

		$up_view = '';
		foreach ($enabled_views as $view => $type) {
			if ($view == $cal_view) break;
			if ($type) $up_view = $view;
		}
		$replacements['up_link'] = $up_view ? '<a class="calendarNavLink"  href="'.$this->getHref().'?SQ_CALENDAR_VIEW='.$up_view.'&SQ_CALENDAR_DATE='.$cal_date.'">'.str_replace('%v%', ucfirst($up_view), $this->attr('up_link_text')).'</a>' : '';

		// print the contents of page - replacing the global keywords
		$format_bodycopy->setKeywordReplacements($replacements);
		$format_bodycopy->printBody();


	}//end printBody() 



//--        FUNCTIONS TO PRINT CALENDAR VIEWS        --//


	/**
	* Get the contents of the year view for the year containing the specified day
	*
	* @return void
	* @access public
	*/
	function getYearView($date)
	{
		ob_start();
		$enabled_views = $this->attr('enabled_views');
		if (strpos('list', $enabled_views) == 0) {
			// dummy data
			$events = Array(48 => 'calendar_event_single', 49 => 'calendar_event_recurring');
			$this->printAssetList($events);
		} else {
			list($year, $month, $day) = explode('-', $date);
			$week_start_day = $this->attr('week_starts_on');
			$enabled_views = $this->attr('enabled_views');
			$drill_down_view = $this->attr('year_drill_down_view');

			$this->_printClickChildScript();
			?>
			<table id="year" border="0" cellspacing="0" cellpadding="0" style="width: 118ex; font-size: 90%">
				<tr>
					<th class="dayLabel" style="width: 6ex">&nbsp;</th>
			<?php
			for ($i = $week_start_day; $i < $week_start_day + 36; $i++) {
				?>
					<th class="dayLabel<?php echo (($i%7)==0 || ($i%7)==6) ? ' dayLabelWeekend' : ''; ?>" style="width: 3ex; text-align: center"><b><?php echo $this->day_names_very_short[$i % 7]; ?></b></th>
				<?php
			}
			?>
				</tr>
			<?php
			for ($current_month=1; $current_month<13; $current_month++) {
			?>
				<tr>
					<th class="monthLabel" <?php if ($enabled_views['month']) { ?> onclick="clickChild(this);" style="cursor: pointer; cursor: hand" <?php } ?>
					<?php
						// print the month name, linked to month view if it's available
						if ($enabled_views['month']) {
							echo '<a  href="'.$this->getHref().'?SQ_CALENDAR_VIEW=month&SQ_CALENDAR_DATE='.$year.'-'.sprintf('%02d', $current_month).'-01">';
						}
						echo $this->month_names_short[$current_month];
						if ($enabled_views['month']) {
							echo '</a>';
						}
					?>
					</th>

				<?php
				// print blank days of week before the day our month starts on
				$i=0;
				$blank_days = (date('w', strtotime("$year-".sprintf('%02d', $current_month).'-01')) + 7 - $week_start_day) % 7;
				for ($i = 0; $i < $blank_days; $i++) {
				?>
					<td class="<?php echo (((($week_start_day + $i) % 7)==0) || ((($week_start_day + $i) % 7)==6)) ? ' weekend' : ''; ?> ">&nbsp;</td>
				<?php
				}


				// print the days of the month
				for ($current_day = 1; $current_day < days_in_month($current_month, $year); $current_day++) {
					$i++;
					$occupied = (bool)(rand(0,100) < 5); // GET DATA FROM DB HERE
					$class = 'date';
					if (in_array(date('D', strtotime("$year-$current_month-$current_day")), Array('Sat', 'Sun')))  $class .= ' weekend';
					if ($occupied) $class .= ' eventDate';
					$onclick = ($occupied && $drill_down_view) ? 'onclick="clickChild(this)" style="cursor: pointer; cursor: hand"' : '';
					echo '<td '.$onclick.' class="'.$class.'">';

					if ($occupied && $drill_down_view) echo '<a  href="'.$this->getHref().'?SQ_CALENDAR_VIEW='.$drill_down_view."&SQ_CALENDAR_DATE=$year-".sprintf('%02d', $current_month).'-'.sprintf('%02d', $current_day).'">';
					echo $current_day;
					if ($occupied && $drill_down_view) echo '</a>';
					echo '</td>';

				}

				// print blank days after the end of our month
				while ($i < 36) {
				?>
					<td class="<?php echo (((($week_start_day + $i) % 7)==0) || ((($week_start_day + $i) % 7)==6)) ? ' weekend' : ''; ?>">&nbsp</td>
				<?php
					$i++;
				}
				?>
				</tr>
			<?php
			}
			?>
			</table>
			<?
		}
		$res = ob_get_contents();
		ob_end_clean();
		return $res;
		
	}//end getYearView();


	/**
	* Get the contents of the monthly view with for the month containing the specified day
	*
	* @param string	$iso	the ISO 8601'd date that we are displaying (YYYY-MM-DD)
	*
	* @return void
	* @access public
	*/
	function getMonthView($date)
	{
		ob_start();
		list($year, $month, $day) = explode('-', $date);
		$column_width = $this->attr('month_column_width');
		$row_height = $this->attr('month_row_height');
		$week_start_day = $this->attr('week_starts_on');
		$root_nodes = $this->attr('root_nodes');
		$root_node_keys = array_keys($root_nodes);
		$week_length = 7;
		$month_length = days_in_month($month, $year);
		$current_day = 1;
		$enabled_views = $this->attr('enabled_views');
		$drill_down_views = Array('week', 'day');
		$drill_down_view = current($drill_down_views);
		while (empty($enabled_views[$drill_down_view]) && ($drill_down_view !== FALSE)) $drill_down_view = next($drill_down_views);

		$this->_printClickChildScript();
		?>
		<table id="month" border="0" cellspacing="0" cellpadding="0" style="width: <?php echo $column_width*$week_length; ?>ex">
			<tr>
		<?php
			// print the day headings
			for ($i=0; $i < $week_length; $i++) {
				$day_name = $this->day_names_full[($week_start_day + $i) % 7];
		?>
				<th style="width: <?php echo $column_width; ?>ex" class="dayLabel <?php echo (($day_name == 'Saturday') || ($day_name == 'Sunday')) ? 'dayLabelWeekend' : ''; ?>"><?php echo $day_name; ?></th>
		<?php
			}
		?>
			</tr>
			<tr style="height: <?php echo $row_height; ?>ex">
		<?php

		// print blank days if necessrary
		$i = 0;
		$blank_days = (date('w', strtotime("$year-".sprintf('%02d', $month).'-01')) + 7 - $week_start_day) % 7;
		for ($i = 0; $i < $blank_days; $i++) {
		?>
				<td class="<?php echo (((($week_start_day + $i) % 7)==0) || ((($week_start_day + $i) % 7)==6)) ? ' weekend' : ''; ?>">&nbsp;</td>
		<?php
		}
		
		// print the rest of the first week
		while ($i < $week_length) {
			$contents = $this->_getRandomMonthEvent();
			$weekend_class = in_array(date('D', strtotime("$year-$month-$current_day")), Array('Sat', 'Sun')) ? 'weekend' : '';
			$occupied_class = ($contents) ? 'eventDate' : '';
			$onclick = $contents ? 'clickChild(this)' : '';
			echo '<td onclick="'.$onclick.'" class="date '.$occupied_class.' '.$weekend_class.'">';
			if ($contents && $drill_down_view) echo '<a class="dateLink"  href="'.$this->getHref().'?SQ_CALENDAR_VIEW='.$drill_down_view.'&SQ_CALENDAR_DATE='.$year.'-'.$month.'-'.sprintf('%02d', $current_day).'">';
			echo $current_day;
			if ($contents && $drill_down_view) echo '</a><br />';
			echo $contents;
			echo '</td>';
			$current_day++;
			$i++;
		}

		echo '</tr>';

		// print the full weeks
		$num_full_weeks = floor(($month_length - $current_day) / 7);
		for ($w = 0; $w < $num_full_weeks; $w++) {
			echo '<tr style="height: '.$row_height.'ex">';
			for ($i=0; $i < $week_length; $i++) {
				$contents = $this->_getRandomMonthEvent();
				$weekend_class = in_array(date('D', strtotime("$year-$month-$current_day")), Array('Sat', 'Sun')) ? 'weekend' : '';
				$occupied_class = ($contents) ? 'eventDate' : '';
				$onclick = $contents ? 'clickChild(this)' : '';
				echo '<td onclick="'.$onclick.'" class="date '.$occupied_class.' '.$weekend_class.'">';
				if ($contents && $drill_down_view) echo '<a class="dateLink"  href="'.$this->getHref().'?SQ_CALENDAR_VIEW='.$drill_down_view.'&SQ_CALENDAR_DATE='.$year.'-'.$month.'-'.sprintf('%02d', $current_day).'">';
				echo $current_day;
				if ($contents && $drill_down_view) echo '</a><br />';
				echo $contents;
				echo '</td>';
				$current_day++;
			}
			echo '</tr>';
		}

		// print the final week, if necessary
		if ($current_day < $month_length) {
			$i = 0;
			echo '<tr style="height: '.$row_height.'ex">';
			while ($current_day <= $month_length) {
				$contents = $this->_getRandomMonthEvent();
				$weekend_class = in_array(date('D', strtotime("$year-$month-$current_day")), Array('Sat', 'Sun')) ? 'weekend' : '';
				$occupied_class = ($contents) ? 'eventDate' : '';
				$onclick = $contents ? 'clickChild(this)' : '';
				echo '<td onclick="'.$onclick.'" class="'.$occupied_class.' '.$weekend_class.' date">';
				if ($contents && $drill_down_view) echo '<a class="dateLink"  href="'.$this->getHref().'?SQ_CALENDAR_VIEW='.$drill_down_view.'&SQ_CALENDAR_DATE='.$year.'-'.$month.'-'.sprintf('%02d', $current_day).'">';
				echo $current_day;
				if ($contents && $drill_down_view) echo '</a><br />';
				echo $contents;
				echo '</td>';
				$current_day++;
				$i++;
			}

			// print blank days, if necessary
			while ($i < $week_length) {
			?>
				<td class="<?php echo (((($week_start_day + $i) % 7)==0) || ((($week_start_day + $i) % 7)==6)) ? ' weekend' : ''; ?>">&nbsp;</td>
			<?php
				$i++;
			}

			echo '</tr>';
		}
		echo '</table>';

		$res = ob_get_contents();
		ob_end_clean();
		return $res;
		
	}//end getMonthView()


	/**
	* Get the contents of the week view for the week containing the specified day
	*
	* @param string	$iso	the ISO 8601'd date of the first day (YYYY-MM-DD)
	*
	* @return void
	* @access public
	*/
	function getWeekView($date)
	{
		ob_start();
		$root_nodes = $this->attr('root_nodes');
		list($year, $month, $day) = explode('-', $date);

		// get the list of columns using the week-starts-on and week-days-to-show attributes
		$week_start_day = $this->attr('week_starts_on');
		$enabled_days = $this->attr('week_view_show_days');
		$columns = Array();
		$ordered_days = Array();
		for ($i = $week_start_day; $i < $week_start_day + 7; $i++) {
			if ($enabled_days[$i%7]) $columns[$i%7] = $this->day_names_full[$i%7];
			$ordered_days[] = $i%7;
		}
		
		// if day view is enabled, figure out where the day headings should link to, 
		// bearing in mind that the 'now' date could be anywhere in the week, including
		// a hidden day.
		$column_links = Array();
		$enabled_views = $this->attr('enabled_views');
		if ($enabled_views['day']) {
			$ordered_days = array_flip($ordered_days);
			$today_index = $ordered_days[date('w', strtotime($date))];
			foreach ($columns as $id => $name) {
				$target_time = strtotime((($ordered_days[$id] < $today_index) ? 'last ' : '') . $name, strtotime($date));
				$target_date = date('Y-m-d', $target_time);
				$column_links[$target_date] = $this->getHref().'?SQ_CALENDAR_VIEW=day&SQ_CALENDAR_DATE='.$target_date;
				$columns[$target_date] = $columns[$id].' '.add_ordinal_suffix(date('j', $target_time));
				unset($columns[$id]);
			}
		}

		$result =& $this->_multipleDaySingleQuery($root_nodes, $date, 7);
		
		$xresult =& $this->_multipleDayRecurringQuery($root_nodes, $date, 'week');
		
		$result = $this->_massageCalendarEvents($result, $xresult);
		pre_echo($result);
		
		$this->_printTimedTable('week', $result, $columns, $column_links, $enabled_views['event']);

		$res = ob_get_contents();
		ob_end_clean();
		return $res;
		
	}//end getWeekView()


	/**
	* Get the contents of the day view for the specified day
	*
	* @param string	$iso	the ISO 8601'd day (YYYY-MM-DD)
	*
	* @return void
	* @access public
	*/
	function getDayView($date)
	{
		ob_start();
		$root_nodes = $this->attr('root_nodes');

		$columns = ((count($root_nodes > 1) && $this->attr('columnise_day_view_by_root_node'))) ? array_keys($root_nodes) : Array();
		
		if (!empty($columns)) {
			$location_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($columns);
			$new_cols = Array();
			foreach ($columns as $id) $new_cols[$id] = $location_info[$id]['name'];
		}

		list($year, $month, $day) = explode('-', $date);
		
		$result =& $this->_multipleDaySingleQuery($root_nodes, $date, 1);
		
		$xresult =& $this->_multipleDayRecurringQuery($root_nodes, $date, 'day');
		//$result = array_merge($xresult, $result);

		$result = $this->_massageCalendarEvents($result, $xresult);
		pre_echo($result);
		
		$enabled_views = $this->attr('enabled_views');
		$this->_printTimedTable('day', $result, $new_cols, Array(), $enabled_views['event']);

		$res = ob_get_contents();
		ob_end_clean();
		return $res;

	}//end getDayView()


	/**
	* Get the contents of the details of the specified event by itself
	*
	* @return void
	* @access public
	*/
	function getEventView($event_id)
	{
		ob_start();


		$res = ob_get_contents();
		ob_end_clean();
		return $res;


	}//end getEventView()


	function getViewTitle($view, $date, $event_id) 
	{
		$title_formats = $this->attr('view_title_formats');
		if ($view == 'event') {
			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($event_id);
			return str_replace(Array('t', 'i'), Array($asset->attr('name'), $date), $title_formats['event']);
		} elseif ($view == 'week') {
			// convert the date into the first day of the week's date
			$week_start_day = $this->day_names_full[$this->attr('week_starts_on')];
			if (date('l',strtotime('date')) != $week_start_day) $date = date('Y-m-d', strtotime('last '.$week_start_day, strtotime($date)));
		}		
		return date($title_formats[$view], strtotime($date));
	}


//--        HELPER FUNCTIONS FOR VIEW PRINTING FUNCTIONS        --//


	function _getRandomMonthEvent()
	{
		$root_nodes = $this->attr('root_nodes');
		$enabled_views = $this->attr('enabled_views');
		$root_node_keys = array_keys($root_nodes);
		$event_class = $root_nodes[$root_node_keys[rand(0, count($root_node_keys)-1)]]['class_name'];

		if (rand(0,100) < 12) {
			$res = ($enabled_views['event']) ? ('<a href="'.$this->getHref().'?SQ_CALENDAR_VIEW=event&SQ_CALENDAR_DATE='.$_GET['SQ_CALENDAR_DATE'].'&SQ_CALENDAR_EVENT_ID=123" class="event '. $event_class.'">') : ('<div class="event '.$event_class.'">');
			$res .= 'Random Event';
			$res .=  $enabled_views['event'] ? '</a>' : '</div>';
			return $res;
		} else {
			return '';
		}
	}


	/**
	* Print a table with times within the day-starts-at to day-ends-at range, and columns
	* as specified.
	*
	* @param string		$id					The id to use for the HTML table element
	* @param Array		$data				The event data to put in the table
	* @param Array		$columns			A date=>name array of the columns in the table
	* @param Array		$column_links		An id=>href array of things to link the column headers to
	* @param boolean	$show_event_links	Whether to link the event names to the event view
	*
	* @return void
	* @access private
	*/
	function _printTimedTable($id, $data, $columns=Array(1=>'Events'), $column_links=Array(), $show_event_links=false)
	{
		$partition_time = $this->attr('day_partition_time');
		$partition_height = $this->attr('day_partition_height');
		$root_nodes = $this->attr('root_nodes');
		$this->_printClickChildScript();
		$width = $this->attr('table_column_width');
		?>
		<table id="<?php echo $id; ?>" border="0" cellspacing="0" cellpadding="0" summary="events in this <?php echo $id ?>" style="width: <?php echo ($width * count($columns))+6; ?>ex">
		<?php
		if (count($columns) > 1) {
			// if we have several columns, print the column headers
			?>
			<tr>
				<th style="width: 6ex">Time</td>
			<?php
			foreach ($columns as $date => $name) {
				?>
				<th onclick="<?php echo (!empty($column_links)) ? 'clickChild(this); ' : ''; ?>" class="columnLabel" style="width: <?php echo $width; ?>ex;  <?php echo (!empty($column_links)) ? 'cursor: pointer; cursor: hand' : ''; ?>">
					<?php 
						if (!empty($column_links)) echo '<a  href="'.$column_links[$date].'">';
						echo $name;
						if (!empty($column_links)) echo '</a>';
					?>
				</th>
				<?php
			}
			?>
			</tr>
			<?php
		}
		$left_margin = $width * 0.02;
		$width = $width * 0.90;
		list($current_hour, $current_mins) = explode(':', $this->attr('day_starts_at'));
		list($end_hour, $end_mins) = explode(':', $this->attr('day_ends_at'));
		print '<div style="border:5px solid red">';
		pre_echo($data);
		print '</div>';
		while (($current_hour*60 + $current_mins) < ($end_hour*60 + $end_mins)) {
			?>
			<tr style="height: <?php echo $partition_height; ?>ex;">
				<th class="timeLabel"><?php echo (($current_hour > 12) ? ($current_hour % 12) : (int)$current_hour).':'.sprintf('%02d', $current_mins); ?></th>
			<?php
			foreach ($columns as $date => $name) {
				?>
				<td>
				<?php
					foreach($data[$date] as $result_item) {
						$event_class = '';						
						$event_start_time = $result_item['start_date_ts'] / $partition_time;
						$event_length = is_null($result_item['end_date_ts']) ? 0 : ($result_item['end_date_ts'] - $result_item['start_date_ts']);
						$zero_length_code = $event_length ? '' : 'border-style: dashed; border-width: 2px; border-bottom: 0px; border-left: 0px; border-right: 0px; height: 0px; padding-top: 0px; background-color: transparent;';
						echo "<div class=\"{$event_class} event\" style=\"position: absolute; width: {$width}ex; margin-left: {$event_left_margin}ex; margin-top: {$event_margin_top}ex; height: {$event_height}ex; $cursor_code $zero_length_code\"".($show_event_links ? ' onclick="clickChild(this)"' : '').'><div class="eventText" style="position: absolute; width: '.($width*0.99).'ex; border: none; margin-top: '.($event_length ? '1px' : '0px').'; background-color: transparent">';


						echo $show_event_links ? '<a class="event '.$event_class.'"  href="'.$this->getHref().'?SQ_CALENDAR_VIEW=event&SQ_CALENDAR_DATE='.$date.'&SQ_CALENDAR_EVENT_ID='.$event_id.'">' : '<span class="event '.$event_class.'">';

						
						echo "$event_length minute Event";
	
						if ($show_event_links) echo '</a>';
	
						echo '</div></div>';
					}
				?>
				&nbsp;
				</td>
				<?php
			}
			?>
			</tr>
			<?php
			$this->_addMinutes($current_hour, $current_mins, $partition_time);
		}
		?>
		</table>
		<?php
		
	}//end printTimedTable()


	/**
	* Add the specified number of minutes to the time given
	*
	* @return void
	* @access private
	*/
	function _addMinutes(&$current_hour, &$current_mins, $add_mins) {
		$current_mins += $add_mins;
		$current_hour += floor($current_mins / 60);
		$current_mins = $current_mins % 60;

	}//end _addMinutes()


	/**
	* Print the javascript function used to make DIVs clickable
	*
	* @return void
	*/
	function _printClickChildScript()
	{
		?>
		<script language="JavaScript">
		<!--
		  function clickChild(elt) {
			 linkChildren = elt.getElementsByTagName("A");
			 if (linkChildren.length > 0) {
		         document.location = linkChildren.item(0).href;
		      }
		  }
		//-->
		</script>
		<?php

	}//end _printClickChildScript


//--        FUNCTIONS TO RETRIEVE EVENTS FROM THE DB        --//


	/**
	* Builds a full or partial ISO8601 format date (for use in database searching)
	*
	* This function uses mktime() to construct the date, therefore months/days outside the
	* valid range will be quietly handled as mktime() would (eg. params of 2004,8,32 would
	* return the date 2004-09-01). This is done mainly to simplify the week view script
	*
	* @param int	$year	year of date
	* @param int	$month  the month
	* @param int	$day	the day (optional)
	*
	* @return string	YYYY-MM if $day omitted, YYYY-MM-DD if $day included
	* @access private
	*/
	function _buildDate($year, $month, $day=null)
	{
		$date_value = mktime(0,0,0,$month,(is_null($day) ? 1 : $day), $year);
		
		return $date_value;
		
	}//end _buildDate()
	
	
	/**
	* Returns the maximum number of overlapping events in a certain event list
	* 
	* This is meant to determine whether there are double and triple bookings (and adjust
	* the display of the week or day view to suit), hence the other parameters. This
	* method doesn't have much meaning outside the day (including resource day) and
	* week views.
	*
	* @param Array	&$events	array of events containing at least start_date and
	*							end_date indices which only contain a time component
	*							(HH:MM:SS) as their value for both
	* @param string	$start		Earliest time to check for overlaps (HH:MM)
	* @param string	$end		Latest time to check for overlaps (HH:MM)
	* @param int	$spacing	Calendar spacing in MINUTES
	
	* @return int				= 0 if events is not an array or is empty
	* @access private
	*/
	function _maxOverlappingEvents(&$events, $start, $end, $spacing)
	{
		if (empty($events) || !is_array($events)) return 0;
		
		list($start_time_h, $start_time_m) = explode(':',$start);
		list($end_time_h, $end_time_m) = explode(':',$end);
		
		// turn these times into number of minutes since midnight
		$start_time = $start_time_h * 60 + $start_time_m;
		$end_time   = $end_time_h   * 60 + $end_time_m;
		
		$end_section = (ceil(($end_time - $start_time) / $spacing) - 1);
		
		$event_lists = Array();
		
		foreach($events as $event) {
			list($event_start_time_h, $event_start_time_m) = explode(':',$event['start_date']);
			list($event_end_time_h, $event_end_time_m) = explode(':',$event['end_date']);
			
			$event_start_time = $event_start_time_h * 60 + $event_start_time_m;
			$event_end_time   = $event_end_time_h   * 60 + $event_end_time_m;
			
			$event_start_section = floor(($event_start_time - $start_time) / $spacing);
			$event_end_section = (ceil(($event_end_time - $start_time) / $spacing) - 1);
			
			for($i = $event_start_section; $i <= min($end_section, $event_end_section); $i++) {
				if (!isset($event_lists[$i])) $event_lists[$i] = 0;
				$event_lists[$i]++;
			}
			
		}
			
		array_multisort($event_lists, SORT_DESC, SORT_NUMERIC);
		return reset($event_lists);
				
	}//end _maxOverlappingEvents()
	

	/**
	* Returns the number of $spacing minute periods between times
	*
	* @param string	$start		Start time (HH:MM)
	* @param string	$end		End time (HH:MM)
	* @param int	$spacing	Calendar spacing in MINUTES
	
	* @return int
	* @access private
	*/
	function _numPeriodsBetweenTimes($start, $end, $spacing)
	{
		list($start_time_h, $start_time_m) = explode(':',$start);
		list($end_time_h, $end_time_m) = explode(':',$end);
		
		// turn these times into number of minutes since midnight
		$start_time = $start_time_h * 60 + $start_time_m;
		$end_time   = $end_time_h   * 60 + $end_time_m;
		
		return ceil(($end_time - $start_time) / $spacing);
		
	}//end _numPeriodsBetweenTimes()
	
	
	/**
	* Builds the common part of the single events query
	*
	* @param array	&$trees	(int => string) an array of tree ids indexed by root node
	*
	* @return string	the common part of the SQL query
	* @access private
	*/
	function _buildSingleEventQuery(&$trees)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		
		return 
		'select a.name, a.assetid, cd.*, ('.$this->_treeCaseQuery('t.treeid',$trees).') as first_root_node from ('.SQ_TABLE_RUNNING_PREFIX.'asset a join ('.SQ_TABLE_RUNNING_PREFIX.'asset_link l join '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree t on l.linkid = t.linkid) on l.minorid = a.assetid) join '.SQ_TABLE_RUNNING_PREFIX.'calendar_date_value cd on a.assetid = cd.assetid where a.type_code = '.$db->quote('calendar_event_single');
		
	}//end _buildSingleEventQuery()
	
	
	/**
	* generates the date-related SQL for the recurring events
	*
	* The ISO 8601 date is taken as passed to the calling function - this means the:
	* - YYYY-MM-DD of the day being shown in day view
	* - YYYY-MM-DD of the <strong>first</strong> day in week view
	* - the YYYY-MM (only, no day) of the month involved in month view
	* - the YYYY (only) of the year involved in the year view
	*
	* @param array	&$trees	(int => string) an array of tree ids indexed by root node
	* @param string	$view	the type of view we are requesting (day|week|month|year)
	* @param string	$iso	ISO 8601 date as passed to the calling function
	*
	* @return string
	* @access private
	*/
	function _buildRecurringEventQuery(&$trees, $view, $iso) {
		$iso_ts = iso8601_ts($iso);
		$iso_ds = floor(strtotime(substr($iso,0,10).' 12:00:00 GMT') / 86400);
		list($year,$month,$day) = sscanf(substr($iso,0,10), '%4d-%2d-%2d');
		
		$weekday = date('w', $iso_ts);
		$wdom_list = weekday_dates_in_month($weekday, $month, $year);
		$wdom = array_search($day, $wdom_list) + 1;
		$rwdom = count($wdom_list) - $wdom + 1;
		
		$ms = $year * 12 + $month;
				
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		
		$sql = 
		'select a.name, a.assetid, cd.*, ('.$this->_treeCaseQuery('t.treeid',$trees).') as first_root_node from ('.SQ_TABLE_RUNNING_PREFIX.'asset a join ('.SQ_TABLE_RUNNING_PREFIX.'asset_link l join '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree t on l.linkid = t.linkid) on l.minorid = a.assetid) join '.SQ_TABLE_RUNNING_PREFIX.'calendar_date_value cd on a.assetid = cd.assetid where a.type_code = '.$db->quote('calendar_event_recurring').' AND ('.$iso_ds.' > start_date_ds) AND ';
		
		$sql_freqs = Array();
		
		switch($view) {
			case 'day' :
				$last_day = $iso_ds;
			
				if ((date('w', $iso_ts) != 0) && (date('w', $iso_ts) != 6)) {
					$sql_freqs[] = 'frequency = '.$db->quote('DWD');
				}
				  
				$sql_freqs[] = '(frequency = '.$db->quote('DED').' AND mod('.$this->_dayDiffFragment($db->quote($iso_ds), 'start_date_ds').', period) = 0)';
				  
				$sql_freqs[] = '(frequency = '.$db->quote('WEW').' AND mod('.$this->_dayDiffFragment($db->quote($iso_ds), 'start_date_ds').', period*7) = 0)';
				  
				$sql_freqs_m = Array();
				  
				// every X months on xth of the month
				$sql_freqs_m[] = '(frequency LIKE '.$db->quote('MFN').' AND ('.$db->quote(intval($day)).' = start_date_mday))';
				  
				// every X months on xth last of the month
				$sql_freqs_m[] = '(frequency LIKE '.$db->quote('MFW').' AND ('.$db->quote(date('w', $iso_ts)).' = start_date_wday) and ('.$db->quote($wdom).' = start_date_wdom))';
				  
				// every X months on xth last of the month
				$sql_freqs_m[] = '(frequency LIKE '.$db->quote('MRN').' AND ('.$db->quote(days_in_month(substr($iso,5,2), substr($iso,0,4)) - intval(substr($iso,8,2)) + 1).' = start_date_rmday))';
				  
				// every X months on xth last of the month
				$sql_freqs_m[] = '(frequency LIKE '.$db->quote('MRW').' AND ('.$db->quote($rwdom).' = start_date_rwdom))';
				  
				// combine all the month rules together and add a period clause
				$sql_freqs[] = '(('.implode(' OR ', $sql_freqs_m).') AND MOD('.$db->quote($ms).' - start_date_ms,period) = 0)';
				
			break;
			
			case 'week' :
				$last_day = $iso_ds + 6;
			
				$sql_freqs[] = 'frequency = '.$db->quote('DWD');
				  
				$sql_freqs[] = '(frequency = '.$db->quote('DED').' AND (floor('.$this->_dayDiffFragment($db->quote($iso_ds-1), 'start_date_ds').'/period) < floor('. $this->_dayDiffFragment($db->quote($iso_ds+6), 'start_date_ds').'/period)))';
				 
				$sql_freqs[] = '(frequency = '.$db->quote('WEW').' AND (floor('.$this->_dayDiffFragment($db->quote($iso_ts-1), 'start_date_ds').'/(period*7)) < floor('. $this->_dayDiffFragment($db->quote($iso_ts+6), 'start_date_ds').'/(period*7))))';
				  
				$sql_freqs_m = Array();
				  
				// every X months on xth of the month
				
				$this_month = 'start_date_mon = '.$db->quote($month);
				$days_in_month = days_in_month($month, $year);
				
				// rolling over to a new month ?
				if(($day + 6) > $days_in_month) {
					$this_month = '(start_date_mon = '.$db->quote($month).' AND start_date_mday BETWEEN '.$db->quote($day).' AND '.$db->quote($days_in_month).') OR (';
					
					// check whether we will be rolling over to January next year
					// or just a new month
					if ($month == 12) {
						$this_month .= 'start_date_mon = 1';
					} else {
						$this_month .= 'start_date_mon = '.$db->quote($month + 1);
					}
					$this_month .= ' AND start_date_mday BETWEEN 1 AND '.$db->quote($day + 6 - $days_in_month).')';
				} else {
					$this_month = 'start_date_mon = '.$db->quote($month).' AND start_date_mday BETWEEN '.$db->quote($day).' AND '.$db->quote($day+6);
				}
				print '<span style="color:olivegreen">';
				pre_echo($this_month);
				print '</span>';
				
				$sql_freqs_m[] = '(frequency LIKE '.$db->quote('MFN').' AND ('.$db->quote(intval($day)).' = start_date_mday))';
				  
				// every X months on xth last of the month
				$sql_freqs_m[] = '(frequency LIKE '.$db->quote('MFW').' AND ('.$db->quote($wdom).' = start_date_wdom))';
				  
				// every X months on xth last of the month
				$sql_freqs_m[] = '(frequency LIKE '.$db->quote('MRN').' AND ('.$db->quote(days_in_month(substr($iso,5,2), substr($iso,0,4)) - intval(substr($iso,8,2)) + 1).' = start_date_rmday))';
				  
				// every X months on xth last of the month
				$sql_freqs_m[] = '(frequency LIKE '.$db->quote('MRW').' AND ('.$db->quote($rwdom).' = start_date_rwdom))';
				  
				// combine all the month rules together and add a period clause
				$sql_freqs[] = '(('.implode(' OR ', $sql_freqs_m).') AND MOD('.$db->quote($ms).' - start_date_ms,period) = 0)';
				
			break;
			
			case 'month' :
				$last_day = $iso_ds + days_in_month($month,$year) - 1;
			
				$sql_freqs[] = 'frequency = '.$db->quote('DWD');
				
				$sql_freqs[] = '(frequency = '.$db->quote('DED').' AND (floor('.$this->_dayDiffFragment($db->quote($iso_ds-1), 'start_date_ds').'/period) < floor('. $this->_dayDiffFragment($db->quote($last_day), 'start_date_ds').'/period)))';
				  
				$sql_freqs[] = '(frequency = '.$db->quote('WEW').' and period = 1';
				  
				$sql_freqs[] = '(frequency = '.$db->quote('WEW').' AND (floor('.$this->_dayDiffFragment($db->quote($iso_ds-1), 'start_date_ds').'/(period*7)) < floor('. $this->_dayDiffFragment($db->quote($last_day), 'start_date_ds').'/(period*7))))';
				  
				// all monthly views can be lumped together on this one
				$sql_freqs[] = '(frequency LIKE '.$db->quote('M%').' AND MOD('.$db->quote($ms).' - start_date_ms,period) = 0)';
				
			break;
			
			case 'year' :
				$last_day = $iso_ds+364+is_leap_year($year);
			
				$sql_freqs[] = 'frequency = '.$db->quote('DWD');
				
				$sql_freqs[] = '(frequency = '.$db->quote('DED').' AND (floor('.$this->_dayDiffFragment($db->quote($iso_ds-1), 'start_date_ds').'/period) < floor('. $this->_dayDiffFragment($db->quote($last_day), 'start_date_ds').'/period)))';
				  
				$sql_freqs[] = '(frequency = '.$db->quote('WEW').' and period = 1';
				  
				$sql_freqs[] = '(frequency = '.$db->quote('WEW').' AND (floor('.$this->_dayDiffFragment($db->quote($iso_ds-1), 'start_date_ds').'/(period*7)) < floor('. $this->_dayDiffFragment($db->quote($last_day), 'start_date_ds').'/(period*7))))';
				  
				// all monthly views can be lumped together on this one
				// but we need to do something like the 'every day' type thing to work out
				// whether an occurrence will happen this year
				$sql_freqs[] = '(frequency like '.$db->quote('M%').' AND (floor(('.$this->_dayDiffFragment($year * 12).' - start_date_ms) / period) < floor(('.$this->_dayDiffFragment(($year + 1) * 12).' - start_date_ms) / period)';
			break;
			
		}//endswitch;
		
		$sql .= '('.implode(' OR ', $sql_freqs).')';
		
		// stop date
		$sql .= ' AND (stop_date = '.$db->quote('----------').' OR stop_date >= '.$db->quote($iso).')';
		return $sql;
	
	}//end _buildRecurringEventQuery()
	
	
	/*
	* Generates a 'day/occurrence difference' query fragment
	*
	* Note that the arguments put into this function are NOT quoted, because it is
	* a possibility that one or both could be a database field. If you are planning to
	* use constants in this function it is recommended that you $db->quote() them there.
	*
	* @param mixed	$date		either a UNIX timestamp or the name of a database field
	*							which contains a UNIX timestamp
	* @param mixed	$comp_date	same, but this should hold the date or more likely date
	*							*field* it is being compared to
	* @param int	$period		the number of days between each occurrence
	*
	* @return string	the query fragment
	* @access private
	*/
	function _dayDiffFragment($date, $comp_date) {
		// reason the 86400 seconds per day is a float is to force the ceil() argument
		// to be a float, instead of being integer-div'd and truncated a la PostgreSQL
		return '('.$date.' - '.$comp_date.')';
		
	}//end _dayDiffFragment()
	
	
	/**
	* builds SQL and returns results for multiple day (incl week) queries [single events]
	*
	* @param array	&$root_nodes	the root nodes to search under
	* @param string	$iso			the ISO date (Y-M-D) of the first date to show
	* @param int	$period			the number of consecutive days to show (eg. 1 for
	*								day, 5 for working week, 7 for full week, etc)
	*
	* @return array of records from calendar date value table 
	*/
	function &_multipleDaySingleQuery(&$root_nodes, $iso, $period=1) {
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		
		$trees =& $this->_treesQuery($root_nodes);
		
		list($year,$month,$day) = sscanf($iso, '%04d-%02d-%02d');
		
		$date_sql = '(cd.start_date_mday = '.$db->quote($day).' and cd.start_date_mon = '.$db->quote($month).' and cd.start_date_year = '.$db->quote($year).' and cd.end_date_ts IS NULL) OR (cd.start_date_ts <= '.$db->quote($this->_buildDate($year,$month,$day+$period)).' and cd.end_date_ts >= '.$db->quote($this->_buildDate($year,$month,$day)).')';
		
		// base sql query FOR SINGLE EVENTS
		$sql = $this->_buildSingleEventQuery($trees).' and ('.$date_sql.')';
		
		$tree_sql = Array();
		foreach($trees as $tree) {
			$tree_sql[] = 'substring(t.treeid from 1 for '.strlen($tree).') LIKE '.$db->quote($tree.'%');
		}
		$sql .= ' AND ('.implode(' OR ', $tree_sql).')';
		
		$result = $db->getAll($sql);
		print $sql;
		assert_valid_db_result($result);
		return $result;
			
	}//end _multipleDaySingleQuery()
	
	
	/**
	* builds SQL and returns results for multiple day (incl week) queries [recur events]
	*
	* @param array	&$root_nodes	the root nodes to search under
	* @param string	$iso			the ISO date (Y-M-D) of the first date to show
	* @param int	$period			the number of consecutive days to show (eg. 1 for
	*								day, 5 for working week, 7 for full week, etc)
	*
	* @return array of records from calendar date value table 
	*/
	function &_multipleDayRecurringQuery(&$root_nodes, $iso, $type) {
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		
		$trees =& $this->_treesQuery($root_nodes);
					
		// ask for our 'base' query
		$sql = $this->_buildRecurringEventQuery($trees, $type, $iso);
		
		$tree_sql = Array();
		foreach($trees as $tree) {
			$tree_sql[] = 'substring(t.treeid from 1 for '.strlen($tree).') LIKE '.$db->quote($tree.'%');
		}
		$sql .= ' AND ('.implode(' OR ', $tree_sql).')';
		
		$result = $db->getAll($sql);
		assert_valid_db_result($result);
		return $result;
			
	}//end _multipleDayRecurringQuery()
	
	
	/**
	* Returns the treeids that stem from the root nodes passed to it
	*
	* @param array	&$root_nodes	array of asset ids which represent root nodes
	*
	* @return array(int => string) a list of tree branch ids indexed by root assetid
	* @access private
	*/
	function &_treesQuery(&$root_nodes) {
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		
		// this query to get the treeids of the root nodes to plug into the main query/ies
		$sql = 'select a.assetid, t.treeid from sq_asset a join (sq_asset_link l join sq_asset_link_tree t on l.linkid = t.linkid) on l.minorid = a.assetid where a.assetid in (';
		
		$quoted_root_nodes = array_keys($root_nodes);
		
		// quote the root nodes manually before we implode the thing		
		foreach($quoted_root_nodes as $k => $root_node) {
			$quoted_root_nodes[$k] = $db->quote($quoted_root_nodes[$k]);
		}	
		
		$sql .= implode(', ', $quoted_root_nodes).')';	
		
		$trees = $db->getAssoc($sql);
		assert_valid_db_result($trees);
		return $trees;
		
	}//end _treesQuery()
	
	
	/**
	* Generates an SQL CASE statement for mapping root nodes to tree ids
	*
	* @param array	&$trees		array of (asset id => tree branch id)
	*
	* @return string
	* @access private
	*/
	function &_treeCaseQuery($comparison, &$trees) {
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		
		// this query to get the treeids of the root nodes to plug into the main query/ies
		$sql = 'case';
		
		foreach($trees as $root_node => $treeid) {
			$sql .= ' when '.$comparison.' LIKE '.$db->quote($treeid.'%').' THEN '.$db->quote($root_node);
		}	
		$sql .= ' ELSE 0';
		$sql .= ' END';	
		
		return $sql;
		
	}//end _treeCaseQuery()
	
	
	/**
	* Massage the single and recurring events into a format usable by the display functions
	*
	* The structure of the event arrays are determined by the structure of the
	* calendar_date_value table, which contains too many fields to list here. 
	*
	* @param array	&$single	an array of single events
	* @param array	&$recurring	an array of recurring events
	*
	* @return array	A list of events indexed by ISO day (YYYY-MM-DD)
	* @access private
	*/
	function &_massageCalendarEvents(&$single, &$recurring) {
		$cal_date = (isset($_REQUEST['SQ_CALENDAR_DATE']) && is_iso8601($_REQUEST['SQ_CALENDAR_DATE'])) ? $_REQUEST['SQ_CALENDAR_DATE'] : date('Y-m-d');
		$first_day = iso8601_ts($cal_date);
		list($year,$month,$day) = explode('-',$cal_date);
	
		// If we get a valid and available view type in the request, use it; otherwise
		// use the initial view attribute
		$enabled_views = $this->attr('enabled_views');	
		$cal_view = empty($enabled_views[$_REQUEST['SQ_CALENDAR_VIEW']]) ? $this->attr('initial_view') : $_REQUEST['SQ_CALENDAR_VIEW'];
		
		switch($cal_view) {
			case 'day':
				$last_day = $cal_date;
			break;
			
			case 'week':
				$first_day -= ((7+(date('w', $first_day) - $date_starts_on))%7)*86400;
				$cal_date = date('Y-m-d', $first_day);
				$last_day = date('Y-m-d', $first_day + 6*86400);
			break;
			
			case 'month':
				$cal_date = date('Y-m', $first_day).'-01';
				$last_day = date('Y-m', $first_day).'-'.days_in_month($month, $year);
			break;
				
			case 'year':
				$cal_date = date('Y', $first_day).'-01';
				$last_day = date('Y', $first_day).'-12-31';
			break;
				
		}

		$formatted_events = Array();
	
		list($starts_at_hour, $starts_at_mins) = explode(':', $this->attr('day_starts_at'));
		$partition_time = $this->attr('day_partition_time');
		$starts_at = $starts_at_hour * 60 + $starts_at_mins;
	
		  //////////////////////
		 //  SINGLE EVENTS   //
		//////////////////////
		
		foreach(array_keys($single) as $key) {
			$event =& $single[$key];
			
			if (is_null($event['start_date_hours']) || is_null($event['start_date_minutes'])) {
				$event_start_time = null;
			} else {
				$event_start_time = $event['start_date_hours'] * 60 + $event['start_date_minutes'];
				$event['partition_offset'] = ($event_start_time - $starts_at) % $partition_time;
				$partition_start = $event_start_time - $event['partition_offset'];
				$event['partition_start'] = str_pad(floor($partition_start / 60), 2, '0', STR_PAD_LEFT).':'.str_pad($partition_start % 60, 2, '0', STR_PAD_LEFT);
				pre_echo($partition_start);				
			}
			
			if (!is_null($event['end_date_ds'])) {
				// end date is specified
				if ($event['start_date_ds'] != $event['end_date_ds']) {
					// multiple day
					$start_time = mktime(12, 0, 0, $event['start_date_mon'], $event['start_date_mday'], $event['start_date_year']);
					$start_date = date('Y-m-d', $start_time);
					
					$end_time = mktime(12, 0, 0, $event['end_date_mon'], $event['end_date_mday'], $event['end_date_year']);
					$end_date = date('Y-m-d', $end_time);
						
					$formatted_events[$start_date][] = array_merge($event, Array(
					'end_date_hours' => 24, 'end_date_minutes' => 0));
					
					for ($inter_time = $start_time + 86400; $inter_time < $end_time; $inter_time += 86400) {						
						$inter_date = date('Y-m-d', $inter_time);
						$formatted_events[$inter_date][] = array_merge($event, Array(
							'start_date_hours' => 0, 'start_date_minutes' => 0,
							'end_date_hours' => 24, 'end_date_minutes' => 0,
							'partition_start' => $this->attr('day_starts_at')));
					}
					
					$formatted_events[$end_date][] = array_merge($event, Array(
					'start_date_hours' => 0, 'start_date_minutes' => 0,
					'partition_start' => $this->attr('day_starts_at')));
					
					
				} else {
					$start_time = mktime(12, 0, 0, $event['start_date_mon'], $event['start_date_mday'], $event['start_date_year']);
					$start_date = date('Y-m-d', $start_time);
					
					// single day
					$formatted_events[$start_date][] = $event;
				}
			} else {
				$start_time = mktime(12, 0, 0, $month, $day, $year);
				$start_date = date('Y-m-d', $start_time);
				// single day in any case
				$formatted_events[$start_date][] = $event;
			}			
		}

		  ////////////////////////
		 //  RECURRING EVENTS  //
		////////////////////////
		
		foreach(array_keys($recurring) as $key) {
			$event =& $recurring[$key];
			
			$next_occur = $this->_getEventFirstOccurrenceAfter($event['start_date_ts'], $event['frequency'], $event['period'], strtotime($cal_date));
			
			
			pre_echo(date('Y-m-d h:i:s', $next_occur));
			pre_echo('LAST DAY:'.$last_day);
			pre_echo('NEXT OCCUR IS NOT IDENTICAL TO FALSE FLAG:');
			var_export(($next_occur !== false));
			pre_echo('NEXT OCCUR IS LESS THAN OR EQUAL TO LAST DAY FLAG:');
			var_export(($next_occur <= $last_day));
			
			while (($next_occur !== false) && (date('Y-m-d', $next_occur) <= $last_day)) {
				$next_occur_ds = intval(strtotime(date('Y-m-d', $next_occur).' 12:00:00 GMT') / 86400);
				pre_echo('NEXT OCCUR DS:'.$next_occur_ds);
				// days between occurrences
				$difference = $next_occur_ds - $event['start_date_ds'];
				pre_echo('DIFFERENCE:'.$difference);
			
				if (is_null($event['start_date_hours']) || is_null($event['start_date_minutes'])) {
					$event_start_time = null;
				} else {
					$event_start_time = $event['start_date_hours'] * 60 + $event['start_date_minutes'];
					$event['partition_offset'] = ($event_start_time - $starts_at) % $partition_time;
					$partition_start = $event_start_time - $event['partition_offset'];
					$event['partition_start'] = str_pad(floor($partition_start / 60), 2, '0', STR_PAD_LEFT).':'.str_pad($partition_start % 60, 2, '0', STR_PAD_LEFT);
					pre_echo($partition_start);				
				}
				
				if (!is_null($event['end_date_ds'])) {
					// end date is specified
					if ($event['start_date_ds'] != $event['end_date_ds']) {
						// multiple day
						$start_time = mktime(12, 0, 0, $event['start_date_mon'], $event['start_date_mday'], $event['start_date_year'])+$difference*86400;
						$start_date = date('Y-m-d', $start_time);
						
						$end_time = mktime(12, 0, 0, $event['end_date_mon'], $event['end_date_mday'], $event['end_date_year'])+$difference*86400;
						$end_date = date('Y-m-d', $end_time);
							
						$formatted_events[$start_date][] = array_merge($event, Array(
						'end_date_hours' => 24, 'end_date_minutes' => 0));
						
						for ($inter_time = $start_time + 86400; $inter_time < $end_time; $inter_time += 86400) {						
							$inter_date = date('Y-m-d', $inter_time);
							$formatted_events[$inter_date][] = array_merge($event, Array(
								'start_date_hours' => 0, 'start_date_minutes' => 0,
								'end_date_hours' => 24, 'end_date_minutes' => 0,
								'partition_start' => $this->attr('day_starts_at')));
						}
						
						$formatted_events[$end_date][] = array_merge($event, Array(
						'start_date_hours' => 0, 'start_date_minutes' => 0,
						'partition_start' => $this->attr('day_starts_at')));
						
					} else {
						$start_time = mktime(12, 0, 0, $event['start_date_mon'], $event['start_date_mday'], $event['start_date_year'])+$difference*86400;
						$start_date = date('Y-m-d', $start_time);
						
						// single day
						$formatted_events[$start_date][] = $event;
					}
				} else {
					$start_time = mktime(12, 0, 0, $month, $day, $year)+$difference*86400;
					$start_date = date('Y-m-d', $start_time);
					// single day in any case
					$formatted_events[$start_date][] = $event;
					
				}
			
				// get the next occurrence
				$next_occur = substr($this->_getEventFirstOccurrenceAfter($event['start_date_ts'], $event['frequency'], $event['period'], $next_occur+86400),0,10);
			
			pre_echo(date('Y-m-d', $next_occur));
			//die('KILL!!!');
			
			pre_echo(date('Y-m-d h:i:s', $next_occur));
			pre_echo('LAST DAY:'.$last_day);
			pre_echo('NEXT OCCUR IS NOT IDENTICAL TO FALSE FLAG:');
			var_export(($next_occur !== false));
			pre_echo('NEXT OCCUR IS LESS THAN OR EQUAL TO LAST DAY FLAG:');
			pre_echo(date('Y-m-d',$next_occur)." <= $last_day");
			var_export(($next_occur <= $last_day));
			
			}//end while
			
			
			
		}
		pre_echo($formatted_events);		
				
				pre_echo(str_repeat('=', 80));
				pre_echo(str_repeat('=', 80));
				pre_echo(str_repeat('=', 80));
				
		return $formatted_events;
				
	}//end _massageCalendarEvents()
	
	
	/**
	* 
	*/
	function _getEventFirstOccurrenceAfter($start_ts, $freq_type, $interval, $after=null, $stop_date='2030-12-31')
	{
		pre_echo(Array(date('r', $start_ts), $freq_type, $interval, date('r', $after), $stop_date));
		
		if (is_null($after)) $after = time();
		if (!$interval)	$interval = 1;
		$seconds_per_day = 60 * 60 * 24;
		//$start_ts = strtotime($start_date.' GMT');
		
		// if after date is before first occurrence - return the first occurrence
		if ($after < $start_ts) {
			print 'Bailing with "AFTER"';
			return $start_ts;
		}

		switch ($freq_type) {
			case 'DED':
			pre_echo('DED!!!');
				$interval_length = $seconds_per_day * $interval;
				$periods_so_far = (($after - $start_ts) / ($interval_length));
				$result = $start_ts + ($interval_length * ceil($periods_so_far));
				print '$result = '."$start_ts + ($interval_length * ceil($periods_so_far + 1))";
				print 'Bailing with "DED"';
				print 'Periods so Far is "'.$periods_so_far.'"';
				print 'Interval Length is "'.$interval_length.'"';
				print 'Result is "'.$result.'"';
			break;

			case 'DWD':
				$interval_length = $seconds_per_day * $interval;
				$periods_so_far = (int)(($after - $start_ts) / ($interval_length));
				$res = $start_ts + ($interval_length * ($periods_so_far + 1));
				while (in_array(gmdate('D', $res), Array('Sat', 'Sun'))) {
					$res += $seconds_per_day;
				}
				$result =  $res;
			break;

			case 'WEW':
				$interval_length = 7 * $seconds_per_day * $interval;
				$periods_so_far = (int)(($after - $start_ts) / ($interval_length));
				$result =  $start_ts + ($interval_length * ($periods_so_far + 1));
			break;

			case 'MFN':
				$res_month = gmdate('n', $after);
				$res_year = gmdate('Y', $after);
				$month_diff = (12 * (gmdate('Y', $after) - gmdate('Y', $start_ts))) + $res_month - gmdate('n', $start_ts);
				if (($month_diff % $interval) != 0) {
					increment_month($res_month, $res_year, ($interval - ($month_diff % $interval)));
				} elseif (((int)gmdate('j', $after) > (int)gmdate('j', $start_ts))) {
					increment_month($res_month, $res_year, $interval);
				} elseif((((int)gmdate('j', $after) == (int)gmdate('j', $start_ts)) && ($this->_secondsToday($start_ts) < $this->_secondsToday($after)))) {
					increment_month($res_month, $res_year, $interval);
				}
				while (gmdate('j', $start_ts) > days_in_month($res_month, $res_year))
					increment_month($res_month, $res_year, $interval);
				$result =  strtotime($res_year.'-'.sprintf('%02d', $res_month).gmdate('-d H:i:00', $start_ts).' GMT');
			break;

			case 'MFW':
				$res_month = gmdate('n', $after);
				$res_year = gmdate('Y', $after);
				$month_diff = (12 * (gmdate('Y', $after) - gmdate('Y', $start_ts))) + $res_month - gmdate('n', $start_ts);
				$target_week_of_month = (int)((gmdate('j', $start_ts) - 1) / 7);
				$target_day_of_week = (gmdate('w', $start_ts));
				$res_day = ($target_week_of_month * 7) + 1;

				if (($month_diff % $interval) != 0) {
					// go to the next valid month
					increment_month($res_month, $res_year, $interval - ($month_diff % $interval));
				} elseif ((int)((gmdate('j', $after) - 1) / 7) > $target_week_of_month) {
					// we're in a valid month, but too late in it, so go to next valid month
					increment_month($res_month, $res_year, $interval);
				} elseif (((int)((gmdate('j', $after) - 1) / 7) == $target_week_of_month) && (gmdate('w', $after) == $target_day_of_week) && ($this->_secondsToday($after) > $this->_secondsToday($start_ts))) {
					// we're on the right day of a valid month, but too late in the day, 
					// so go to next valid month
					increment_month($res_month, $res_year, $interval);
				} elseif ((int)((gmdate('j', $after) - 1) / 7) == $target_week_of_month) {
					$res_day = gmdate('d', $after);
				}

				$month_length = days_in_month($res_month, $res_year);
				while ((gmdate('w', strtotime($res_year.'-'.sprintf('%02d', $res_month).'-'.$res_day.' 12:00:00 GMT'))) != $target_day_of_week) {
					$res_day++;
					if (($res_day > $month_length) || ($res_day >= (($target_week_of_month+1) * 7))) {
						// run out of days, so we'll have to start again in the next valid month
						increment_month($res_month, $res_year, $interval);
						$month_length = days_in_month($res_month, $res_year);
						$res_day = ($target_week_of_month * 7)+1;
					}
				}
				$result =  strtotime($res_year.'-'.sprintf('%02d', $res_month).'-'.sprintf('%02d', $res_day).gmdate(' H:i', $start_ts).' GMT');
			break;

			case 'MRN':
				$res_month = gmdate('m', $after);
				$res_year = gmdate('Y', $after);
				$target_days_from_end = days_in_month(gmdate('n', $start_ts), gmdate('Y', $start_ts)) - gmdate('j', $start_ts);

				$month_diff = (12 * (gmdate('Y', $after) - gmdate('Y', $start_ts))) + $res_month - gmdate('n', $start_ts);
				if (($month_diff % $interval) != 0) {
					increment_month($res_month, $res_year, $interval - ($month_diff % $interval));
				} elseif ((days_in_month($res_month, $res_year) - (int)gmdate('j', $after)) < $target_days_from_end) {
					increment_month($res_month, $res_year, $interval);
				} elseif (((days_in_month($res_month, $res_year) - (int)gmdate('j', $after)) == $target_days_from_end) && ($this->_secondsToday($after) > $this->_secondsToday($start_ts))) {
					increment_month($res_month, $res_year, $interval);
				}

				while ((days_in_month($res_month, $res_year) - $target_days_from_end) < 0) {
					increment_month($res_month, $res_year, $interval);
				}

				$result =  strtotime($res_year.'-'.$res_month.'-'.sprintf('%02d', days_in_month($res_month, $res_year) - $target_days_from_end).gmdate(' H:i:00', $start_ts).' GMT');
			break;

			case 'MRW':
				$res_month = gmdate('m', $after);
				$res_year = gmdate('Y', $after);
				$min_day = 1;
				$target_weeks_from_end = (int)((days_in_month(gmdate('n', $start_ts), gmdate('Y', $start_ts)) - gmdate('j', $start_ts)) / 7);
				$target_day_of_week = (gmdate('w', $start_ts));

				$month_diff = (12 * (gmdate('Y', $after) - gmdate('Y', $start_ts))) + $res_month - gmdate('n', $start_ts);
				if (($month_diff % $interval) != 0) {
					// go to the next valid month
					$res_month = gmdate('n', $after) + $interval - ($month_diff % $interval);
				} elseif ((int)((days_in_month(gmdate('n', $after), gmdate('Y', $after)) - gmdate('j', $after)) / 7) < $target_weeks_from_end) {
					// we're too far into this month, so go to the next valid month
					increment_month($res_month, $res_year, $interval);
				} elseif (((int)((days_in_month(gmdate('n', $after), gmdate('Y', $after)) - gmdate('j', $after)) / 7) == $target_weeks_from_end) && (gmdate('w', $after) == $target_day_of_week) && ($this->_secondsToday($after) > $this->_secondsToday($start_ts))) {
					// we're on the right day but too late so go to next valid month
					increment_month($res_month, $res_year, $interval);
				} elseif ((int)((days_in_month(gmdate('n', $after), gmdate('Y', $after)) - gmdate('j', $after)) / 7) == $target_weeks_from_end) {
					// we're in the right week so start from 'now', not the begining of the week,
					$min_day = gmdate('d', $start_ts);
				}
				$res_day = days_in_month($res_month, $res_year) - ($target_weeks_from_end * 7);

				while ((gmdate('w', strtotime($res_year.'-'.$res_month.'-'.$res_day.' 12:00:00 GMT'))) != $target_day_of_week) {
					$res_day--;
					if ($res_day < $min_day) {
						// can't get the nth-last xday in this month, either because
						// it's already passed or because it doesn't exist, so goto next month
						increment_month($res_month, $res_year, $interval);
						$res_day = days_in_month($res_month, $res_year) - ($target_weeks_from_end * 7);
						$min_day = 1;
					}
				}
				$result =  strtotime($res_year.'-'.sprintf('%02d', $res_month).'-'.$res_day.gmdate(' H:i:00', $start_ts).' GMT');
			break;
		}
		if ($result > iso8601_ts($stop_date)) {
			print 'You stupid bastard';
			return false;
		} else {
			pre_echo("RESULT!!!");
			return $result;
		}
		
	}//end _getEventFirstOccurrenceAfter()
	
	
/**
	* Return the available keywords in the Page Contents Bodycopy for this asset
	*
	* The return value is in the form:
	* <pre>
	* Array(
	*     'keyword' => Array (
	*					'name'	=> '(name of the keyword)',
	*				    'default' => '(default value)',
	*					'type' => 'block' | 'inline'
	*                  )
	* )
	* </pre>
	*
	* @param boolean $descriptions if TRUE descriptions will be included in the array
	*
	* @access public
	* @return Array(mixed)
	*/
	function getContentsKeywords()
	{
		
		$keywords = Array();
		$keywords['calendar_contents'] = Array(
										'name'=>'Calendar Contents',
										'default'=>'The contents of the current view', 
										'type'=>'block');
		$keywords['calendar_title'] = Array(
										'name'=>'Calendar Title',
										'default'=>'The title of the current calendar view', 
										'type'=>'inline');
		$keywords['up_link'] = Array(
										'name'=>'Up Link',
										'default'=>'Link to the next-broadest view', 
										'type'=>'inline');
		$keywords['prev_link'] = Array(
										'name'=>'Previous Link',
										'default'=>'Link to the previous period', 
										'type'=>'inline');
		$keywords['next_link'] = Array(
										'name'=>'Next Link',
										'default'=>'Link to the next period', 
										'type'=>'inline');
		return $keywords;
	}

}//end class

?>
