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
* $Id: page_calendar.inc,v 1.39 2004/09/01 00:46:23 tbarrett Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_asset_listing/page_asset_listing.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once SQ_FUDGE_PATH.'/general/datetime.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once  dirname(__FILE__).'/../../calendar_event/calendar_event.inc';
require_once  dirname(__FILE__).'/../../calendar_events/calendar_event_recurring/calendar_event_recurring.inc';

/**
* Page_Calendar
*
* Purpose
*
* @author  Luke Wright <lwright@squiz.net>
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.39 $ - 1.0
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
	var $month_names_full = Array(1=>'January', 2=>'February', 3=>'March', 4=>'April', 5=>'May', 6=>'June', 7=>'July', 8=>'August', 9=>'September', 10=>'October', 11=>'November', 12=>'December');
	
	
	/**
	* List of abbreviated month names, in chronological order
	* @var array
	*/
	var $month_names_short = Array(1=>'Jan', 2=>'Feb', 3=>'Mar', 4=>'Apr', 5=>'May', 6=>'Jun', 7=>'Jul', 8=>'Aug', 9=>'Sep', 10=>'Oct', 11=>'Nov', 12=>'Dec');

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

		$replacements = Array();


		// If we get a valid and available view type in the request, use it; otherwise use the initial view attribute.
		$enabled_views = $this->attr('enabled_views');
		$cal_view = (isset($_REQUEST['SQ_CALENDAR_VIEW']) && !empty($enabled_views[$_REQUEST['SQ_CALENDAR_VIEW']])) ? $_REQUEST['SQ_CALENDAR_VIEW'] : $this->attr('initial_view');
		
		// If we get a valid date in the request, use it; otherwise use today's date
		$cal_date = (isset($_REQUEST['SQ_CALENDAR_DATE']) && is_iso8601($_REQUEST['SQ_CALENDAR_DATE'])) ? $_REQUEST['SQ_CALENDAR_DATE'] : date('Y-m-d');
		list($year, $month, $day) = explode('-', $cal_date);
		if (!checkdate($month, $day, $year)) {
			trigger_error('Date passed to calendar page is not valid, using today\'s date as fallback', E_USER_WARNING);
			$cal_date = date('Y-m-d');
		}
		

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

		$replacements['calendar_contents'] = $this->getCalendarContents($cal_view, $cal_date, $event_id, $enabled_views[$cal_view]);
		$replacements['calendar_title'] = $this->getViewTitle($cal_view, $cal_date, $event_id);

		$prev_date = date('Y-m-d', strtotime('-1 '.$cal_view, strtotime($cal_date)));
		$next_date = date('Y-m-d', strtotime('+1 '.$cal_view, strtotime($cal_date)));
		$replacements['prev_link'] = $event_id ? '' : '<a class="calendarNavLink"  href="'.$this->getHref().'?SQ_CALENDAR_VIEW='.$cal_view.'&amp;SQ_CALENDAR_DATE='.$prev_date.'">'.htmlentities(str_replace('%v%', ucfirst($cal_view), $this->attr('prev_link_text'))).'</a>';
		$replacements['next_link'] = $event_id ? '' : '<a class="calendarNavLink"  href="'.$this->getHref().'?SQ_CALENDAR_VIEW='.$cal_view.'&amp;SQ_CALENDAR_DATE='.$next_date.'">'.htmlentities(str_replace('%v%', ucfirst($cal_view), $this->attr('next_link_text'))).'</a>';
		$enabled_views = $this->attr('enabled_views');

		$up_view = '';
		foreach ($enabled_views as $view => $type) {
			if ($view == $cal_view) break;
			if ($type) $up_view = $view;
		}
		$replacements['up_link'] = $up_view ? '<a class="calendarNavLink"  href="'.$this->getHref().'?SQ_CALENDAR_VIEW='.$up_view.'&amp;SQ_CALENDAR_DATE='.$cal_date.'">'.str_replace('%v%', ucfirst($up_view), $this->attr('up_link_text')).'</a>' : '';

		// print the contents of page - replacing the global keywords
		$format_bodycopy->setKeywordReplacements($replacements);
		$format_bodycopy->printBody();


	}//end printBody() 



//--        FUNCTIONS TO GET CALENDAR CONTENTS        --//


	/**
	* Get the replacement for the %calendar_contents% keyword for the specified view, date, format
	* and perhaps event id
	*/
	function getCalendarContents($cal_view, $cal_date, $event_id, $format) 
	{
		$res = '<div id="calendar'.(((strpos($format, 'list') === 0) || $cal_view == 'event') ? 'List' : 'Table').'">';
		$func = 'get'.ucfirst($cal_view).'View';
		$res .= ($cal_view == 'event') ? $this->$func($event_id) : $this->$func($cal_date, $format);
		$res .= '</div>';
		return $res;

	}


	/**
	* Get the contents of the year view for the year containing the specified day
	*
	* @return void
	* @access public
	*/
	function getYearView($date, $format)
	{
		ob_start();
		list($year, $month, $day) = explode('-', $date);
		$enabled_views = $this->attr('enabled_views');
		$drill_down_view = $this->attr('year_drill_down_view');

		if ($format == 'list_without_headings') {
			$list_entry_types = $this->attr('list_entry_types');
			$events = $this->getYearEvents($year);
			if ($list_entry_types['year'] == 'title') {
				echo $this->_getTitleList($events, $enabled_views['event']);
			} else {
				// hack for now until we mod asset listing
				foreach ($events as $id => $details) $events[$id] = $details['type_code'];
				$this->printAssetList($events);
			}
		} elseif ($format == 'list_with_headings') {
			$list_entry_types = $this->attr('list_entry_types');
			$events = $this->getYearEventsByMonth($year);
			foreach ($this->month_names_full as $month_num => $month_name) {
				echo '<h3>';
				if ($enabled_views['month']) echo '<a href="'.$this->getHref().'?SQ_CALENDAR_VIEW=month&amp;SQ_CALENDAR_DATE='.$year.'-'.sprintf('%02d', $month_num).'-01">';
				echo $month_name;
				if ($enabled_views['month']) echo '</a>';
				echo '</h3>';

				if ($list_entry_types['year'] == 'title') {
					echo $this->_getTitleList($events[$month_num], $enabled_views['event']);
				} else {
					// hack for now until we mod asset listing
					foreach ($events[$month_num] as $id => $details) $events[$month_num][$id] = $details['type_code'];
					$this->printAssetList($events[$month_num]);
				}
			}
		} else {
			$events = $this->getYearEventsByMonthAndDay($year);
			$week_start_day = $this->attr('week_starts_on');
			$this->_printClickChildScript();
			?>
			<table id="year" border="0" cellspacing="0" cellpadding="0" style="width: 131ex; font-size: 90%">
				<tr>
					<th class="dayLabel" style="width: 6ex">&nbsp;</th>
			<?php
			for ($i = $week_start_day; $i < $week_start_day + 36; $i++) {
				?>
					<th class="dayLabel<?php echo (($i%7)==0 || ($i%7)==6) ? ' dayLabelWeekend' : ''; ?>" style="width: 3.5ex; text-align: center"><b><?php echo $this->day_names_very_short[$i % 7]; ?></b></th>
				<?php
			}
			?>
				</tr>
			<?php
			for ($current_month=1; $current_month<13; $current_month++) {
			?>
				<tr>
					<th class="monthLabel" <?php if ($enabled_views['month']) { ?> onclick="clickChild(this);" style="cursor: pointer; cursor: hand" <?php } ?>>
					<?php
						// print the month name, linked to month view if it's available
						if ($enabled_views['month']) {
							echo '<a  href="'.$this->getHref().'?SQ_CALENDAR_VIEW=month&amp;SQ_CALENDAR_DATE='.$year.'-'.sprintf('%02d', $current_month).'-01">';
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
					$class = 'date';
					if (in_array(date('D', strtotime("$year-$current_month-$current_day")), Array('Sat', 'Sun')))  $class .= ' weekend';
					$occupied = (bool)$events[$current_month][$current_day];
					if ($occupied) $class .= ' eventDate';
					$onclick = ($occupied && $drill_down_view) ? 'onclick="clickChild(this)" style="cursor: pointer; cursor: hand"' : '';
					echo '<td '.$onclick.' class="'.$class.'">';

					if ($occupied && $drill_down_view) echo '<a  href="'.$this->getHref().'?SQ_CALENDAR_VIEW='.$drill_down_view."&amp;SQ_CALENDAR_DATE=$year-".sprintf('%02d', $current_month).'-'.sprintf('%02d', $current_day).'">';
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
	function getMonthView($date, $format)
	{
		list($year, $month, $day) = explode('-', $date);
		$enabled_views = $this->attr('enabled_views');
		$drill_down_views = Array('week', 'day');
		$drill_down_view = current($drill_down_views);
		while (empty($enabled_views[$drill_down_view]) && ($drill_down_view !== FALSE)) $drill_down_view = next($drill_down_views);
		ob_start();
		if ($format == 'list_without_headings') {
			$list_entry_types = $this->attr('list_entry_types');
			$events = $this->getMonthEvents($year, $month);
			if ($list_entry_types['month'] == 'title') {
				echo $this->_getTitleList($events, $enabled_views['event']);
			} else {
				// hack for now until we mod asset listing
				foreach ($events as $id => $details) $events[$id] = $details['type_code'];
				$this->printAssetList($events);
			}
		} elseif ($format == 'list_with_headings') {
			$list_entry_types = $this->attr('list_entry_types');
			$events = $this->getMonthEventsByDay($year, $month);
			$month_length = days_in_month($month, $year);
			for ($current_day = 1; $current_day <= $month_length; $current_day++) {
				echo '<h3>';
				if ($drill_down_view) echo '<a href="'.$this->getHref().'?SQ_CALENDAR_VIEW='.$drill_down_view.'&amp;SQ_CALENDAR_DATE='.$year.'-'.$month.'-'.sprintf('%02d', $current_day).'">';
				echo date('l jS', strtotime("$year-$month-$current_day"));
				if ($drill_down_view) echo '</a>';
				echo '</h3>';
				if ($list_entry_types['month'] == 'title') {
					echo $this->_getTitleList($events[$current_day], $enabled_views['event']);
				} else {
					// hack for now until we mod asset listing
					foreach ($events[$current_day] as $id => $details) $events[$id] = $details['type_code'];
					$this->printAssetList($events);
				}
			}
		} else {
			$show_event_links = (bool)$enabled_views['event'];
			$events = $this->getMonthEventsByDay($year, $month);
			$column_width = $this->attr('month_column_width');
			$row_height = $this->attr('month_row_height');
			$week_start_day = $this->attr('week_starts_on');
			$root_nodes = $this->attr('root_nodes');
			$root_node_keys = array_keys($root_nodes);
			$week_length = 7;
			$month_length = days_in_month($month, $year);
			$current_day = 1;
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
				$contents = $this->_getMonthCellContents($events[$current_day], $show_event_links, $root_nodes);
				$weekend_class = in_array(date('D', strtotime("$year-$month-$current_day")), Array('Sat', 'Sun')) ? 'weekend' : '';
				$occupied_class = ($contents) ? 'eventDate' : '';
				$onclick = ($contents && $drill_down_view) ? 'clickChild(this)' : '';
				$style = ($contents && $drill_down_view) ? '' : '';
				echo '<td class="date '.$occupied_class.' '.$weekend_class.'" style="'.$style.'">';
				if ($contents && $drill_down_view) echo '<a class="dateLink"  href="'.$this->getHref().'?SQ_CALENDAR_VIEW='.$drill_down_view.'&amp;SQ_CALENDAR_DATE='.$year.'-'.$month.'-'.sprintf('%02d', $current_day).'">';
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
					$contents = $this->_getMonthCellContents($events[$current_day], $show_event_links, $root_nodes);
					$weekend_class = in_array(date('D', strtotime("$year-$month-$current_day")), Array('Sat', 'Sun')) ? 'weekend' : '';
					$occupied_class = ($contents) ? 'eventDate' : '';
					$onclick = ($contents && $drill_down_view) ? 'clickChild(this)' : '';
					$style = ($contents && $drill_down_view) ? '' : '';
					echo '<td class="date '.$occupied_class.' '.$weekend_class.'" style="'.$style.'">';
					if ($contents && $drill_down_view) echo '<a class="dateLink"  href="'.$this->getHref().'?SQ_CALENDAR_VIEW='.$drill_down_view.'&amp;SQ_CALENDAR_DATE='.$year.'-'.$month.'-'.sprintf('%02d', $current_day).'">';
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
					$contents = $this->_getMonthCellContents($events[$current_day], $show_event_links, $root_nodes);
					$weekend_class = in_array(date('D', strtotime("$year-$month-$current_day")), Array('Sat', 'Sun')) ? 'weekend' : '';
					$occupied_class = ($contents) ? 'eventDate' : '';
					$onclick = ($contents && $drill_down_view) ? 'clickChild(this)' : '';
					$style = ($contents && $drill_down_view) ? '' : '';
					echo '<td class="date '.$occupied_class.' '.$weekend_class.'" style="'.$style.'">';
					if ($contents && $drill_down_view) echo '<a class="dateLink"  href="'.$this->getHref().'?SQ_CALENDAR_VIEW='.$drill_down_view.'&amp;SQ_CALENDAR_DATE='.$year.'-'.$month.'-'.sprintf('%02d', $current_day).'">';
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
		}

		$res = ob_get_contents();
		ob_end_clean();
		return $res;
		
	}//end getMonthView()


	/**
	* Get the contents of the week view for the week containing the specified day
	*
	* @param string	$date one of the dates in the week to show
	*
	* @return void
	* @access public
	*/
	function getWeekView($date, $format)
	{
		ob_start();
		$root_nodes = $this->attr('root_nodes');
		$enabled_views = $this->attr('enabled_views');
		if ($format == 'list_without_headings') {
			$list_entry_types = $this->attr('list_entry_types');
			$events = $this->getWeekEvents($date);
			if ($list_entry_types['week'] == 'title') {
				echo $this->_getTitleList($events, $enabled_views['event']);
			} else {
				// hack for now until we mod asset listing
				foreach ($events as $id => $details) $events[$id] = $details['type_code'];
				$this->printAssetList($events);
			}
		} elseif ($format == 'list_with_headings') {
			$list_entry_types = $this->attr('list_entry_types');
			$events = $this->getWeekEventsByDay($date);
			foreach ($events as $day_date => $day_events) {
				echo '<h3>'.(($enabled_views['day']) ? '<a href="'.$this->getHref().'?SQ_CALENDAR_VIEW=day&amp;SQ_CALENDAR_DATE='.$day_date.'"">' : '');
				echo date('l jS', strtotime($day_date));
				echo (($enabled_views['day']) ? '</a>' : '').'</h3>';
				if ($list_entry_types['week'] == 'title') {
					echo $this->_getTitleList($events[$day_date], $enabled_views['event']);
				} else {
					// hack for now until we mod asset listing
					foreach ($events[$day_date] as $id => $details) $events[$day_date][$id] = $details['type_code'];
					$this->printAssetList($events);
				}
			}
		} else {
			$events = $this->getWeekEventsByDayAndTime($date);
			$column_names = Array();
			$column_links = Array();
			foreach ($events as $day_date => $day_events) {
				$column_names[$day_date] = date('l jS', strtotime($day_date));
				if ($enabled_views['week']) $column_links[$day_date] = $this->getHref().'?SQ_CALENDAR_VIEW=day&amp;SQ_CALENDAR_DATE='.$day_date;
			}
			$this->_printTimedTable('week', $column_names, $column_links, $events, $enabled_views['event']);
		}
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
	function getDayView($date, $format)
	{
		ob_start();
		if (strpos($format, 'list') === 0) {
			$list_entry_types = $this->attr('list_entry_types');
			$enabled_views = $this->attr('enabled_views');
			$events = $this->getDayEvents($date);
			if ($list_entry_types['day'] == 'title') {
				echo $this->_getTitleList($events, $enabled_views['event']);
			} else {
				// hack for now until we mod asset listing
				foreach ($events as $id => $details) $events[$id] = $details['type_code'];
				$this->printAssetList($events);
			}
		} else {
			$column_names = Array();
			$column_links = Array();
			if ($this->attr('columnise_day_view_by_root_node')) {
				$columns_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($this->attr('root_nodes')));
				foreach ($columns_info as $id => $info) $column_names[$id] = $info['name'];
				$events = $this->getDayEventsByRootNodeAndTime($date);
			} else {
				$column_names[1] = 'Events';
				$events = Array(1 => $this->getDayEventsByTime($date));
			}
			$enabled_views = $this->attr('enabled_views');
			$this->_printTimedTable('day', $column_names, $column_links, $events, $enabled_views['event']);
		}

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
			$info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($event_id));
			$this->printAssetList(Array($event_id => $info[$event_id]['type_code']));
			$res = ob_get_contents();
		ob_end_clean();
		return $res;


	}//end getEventView()


	/**
	* Get the replacenent for the %calendar_title% keyword for the specified view, date and eventid
	*/
	function getViewTitle($view, $date, $event_id) 
	{
		$title_formats = $this->attr('view_title_formats');
		if ($view == 'event') {
			$info = &$GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($event_id));
			return $info[$event_id]['name'];
		} elseif ($view == 'week') {
			// convert the date into the first day of the week's date
			$week_start_day = $this->day_names_full[$this->attr('week_starts_on')];
			if (date('l',strtotime('date')) != $week_start_day) $date = date('Y-m-d', strtotime('last '.$week_start_day, strtotime($date)));
		}		
		return date($title_formats[$view], strtotime($date));
	}


//--        HELPER FUNCTIONS FOR VIEW PRINTING FUNCTIONS        --//

	
	function _getTitleList($events, $show_links=true) {
		$details = $link = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($events));
		$res = '<ul>';
		foreach ($details as $id => $data) {
			$res .= '<li>';
			if ($show_links) $res .= '<a href="'.$this->getHref().'?SQ_CALENDAR_VIEW=event&amp;SQ_CALENDAR_EVENT_ID='.$id.'">';
			$res .= $data['name'];
			if ($show_links) $res .= '</a>';
			$res .= '</li>';
		}
		$res .= '</ul>';
		return $res;
	}


	/**
	* Get the contents of a cell in the month table
	*/
	function _getMonthCellContents($events, $show_event_link, $root_nodes) 
	{
		$res = '';
		foreach ($events as $assetid => $details) {
			$event_class = (isset($root_nodes[$details['root_node']]['class_name'])) ? $root_nodes[$details['root_node']] ['class_name'] : '';
			$onclick_code = $show_event_link ? ' onclick="clickChild(this); return false;"' : '';
			$res .= '<div style="margin-bottom: 1px; '.($show_event_link ? 'cursor: pointer; cursor: hand; style="width: 100%" ' : '').'" class="event '.$event_class.'"'.$onclick_code.'>';
			if ($show_event_link) $res .= '<a href="'.$this->getHref().'?SQ_CALENDAR_VIEW=event&amp;SQ_CALENDAR_DATE='.$_REQUEST['SQ_CALENDAR_DATE'].'&amp;SQ_CALENDAR_EVENT_ID='.$assetid.'" class="event '. $event_class.'">';

			$res .= $details['name'];
			$res .= $show_event_link ? '</a>' : '</div>';
		}
		return $res;
	}


	/**
	* Get the contents of a cell in the week or day table
	*/
	function _getTimedTableCellContents($events, $show_event_links, $root_nodes, $cell_ts, $partition_time, $partition_height, $event_width) 
	{
		$res = '';
		foreach ($events as $assetid => $details) {
			// get top position of div
			$event_ts = $details['start_date_minutes'] + (60 * $details['start_date_hours']);
			$end_ts = $details['end_date_minutes'] + (60 * $details['end_date_hours']);
			$event_top_margin = (($event_ts - $cell_ts) / $partition_time) * $partition_height;

			// get height of div
			$event_duration = is_null($details['end_date_ts']) ? 0 : ($details['end_date_ts'] - $details['start_date_ts'])/60;
			$event_height = ($event_duration / $partition_time) * $partition_height;
			if ($event_height < 0) continue;
			
			// get class to use for div
			$event_class = $root_nodes[$details['root_node']]['class_name'];

			// other layout things
			$zero_length_code = $event_height ? '' : 'border-style: dashed; border-width: 2px; border-bottom: 0px; border-left: 0px; border-right: 0px; height: 0px; padding-top: 0px; background-color: transparent;';
			$event_left_margin = $event_width * 0.02;
			$inner_width = $event_width * 0.95;
			$cursor_code = $show_event_links ? 'cursor: pointer; cursor: hand;' : '';
			$inner_margin_top = $event_height ? 1 : 0;
			$onclick_code = $show_event_links ? 'onclick="clickChild(this);"' : '';
			$this_event_width = $event_width / $this->_maxOverlappingEvents($events, gmdate('H:i', $event_ts*60), gmdate('H:i', $end_ts*60), $partition_time);
			
			$res .= '<div class='.$event_class.' event" style="float:left; position: absolute; width: '.$this_event_width.'ex; margin-left: '.$event_left_margin.'ex; margin-top: '.$event_top_margin.'ex; height: '.$event_height.'ex; '.$cursor_code.' '.$zero_length_code.'" '.$onclick_code.'><div class="eventText" style="position: absolute; width: '.$inner_width.'ex; border: none; margin-top: '.$inner_margin_top.'px background-color: transparent">';
			$res . $assetid.' ';
			$res .= $show_event_links ? '<a class="event '.$event_class.'"  href="'.$this->getHref().'?SQ_CALENDAR_VIEW=event&amp;SQ_CALENDAR_EVENT_ID='.$assetid.'">' : '<span class="event '.$event_class.'">';
			$res .= $details['name'];
			$res .= $show_event_links ? '</a>' : '</span>';

			$res .= '</div></div>';
		}
		return $res;

	}//end _getTimedTableCellContents()


	/**
	* Print a table with times within the day-starts-at to day-ends-at range, and columns
	* as specified.
	*
	* @param string		$table_id			The id to use for the HTML table element
	* @param Array		$column_names		A column_id=>name array of the columns in the table
	* @param Array		$column_links		A column_id=>href of places to link column headers to
	* @param Array		$events				A column_id=>array(hhmm=>array(events)) array of events to show
	* @param boolean	$show_event_links	Whether to link the event names to the event view
	*
	* @return void
	* @access private
	*/
	function _printTimedTable($table_id, $column_names, $column_links, $events, $show_event_links=false)
	{
		$partition_time = $this->attr('day_partition_time');
		$partition_height = $this->attr('day_partition_height');
		$root_nodes = $this->attr('root_nodes');
		$this->_printClickChildScript();
		$width = $this->attr('table_column_width');
		?>
		<table id="<?php echo $table_id; ?>" border="0" cellspacing="0" cellpadding="0" summary="events in this <?php echo $table_id ?>" style="width: <?php echo ($width * count($column_names))+6; ?>ex">
		<?php
		if (!empty($column_names)) {
			// if we have several columns, print the column headers
			?>
			<tr>
				<th style="width: 6ex">Time</td>
			<?php
			foreach ($column_names as $id => $name) {
				?>
				<th onclick="<?php echo (!empty($column_links)) ? 'clickChild(this); ' : ''; ?>" class="columnLabel" style="width: <?php echo $width; ?>ex;  <?php echo (!empty($column_links)) ? 'cursor: pointer; cursor: hand' : ''; ?>">
					<?php 
						if (!empty($column_links)) echo '<a  href="'.$column_links[$id].'">';
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
		$width = $width * 0.90;
		list($current_hour, $current_mins) = explode(':', $this->attr('day_starts_at'));
		list($end_hour, $end_mins) = explode(':', $this->attr('day_ends_at'));
		?>
			<tr style="height: <?php echo $partition_height; ?>ex;">
			<th class="timeLabel">*</th>
			<?php
			foreach ($events as $id => $col_events) {
				?>
				<td id="<?php echo $id.'_all_day'; ?>">
				<?php
					foreach($col_events['all_day'] as $assetid => $event) {
						// get class to use for div
						$event_class = $root_nodes[$event['root_node']]['class_name'];
						echo '<div class="'.$event_class.' event">';
						echo $show_event_links ? '<a class="event '.$event_class.'"  href="'.$this->getHref().'?SQ_CALENDAR_VIEW=event&SQ_CALENDAR_EVENT_ID='.strtok($assetid,':').'">' : '<span class="event '.$event_class.'">';
						echo $event['name'];
						echo $show_event_links ? '</a>' : '</span>';

						echo '</div>';	
					}
				?>
				&nbsp;
				</td>
				<?php
			}
			?>
			</tr>
		<?php
		while (($current_hour*60 + $current_mins) < ($end_hour*60 + $end_mins)) {
			?>
			<tr style="height: <?php echo $partition_height; ?>ex;">
				<th class="timeLabel"><?php echo sprintf('%d:%02d', (($current_hour % 12 == 0) ? 12 : ($current_hour % 12)), $current_mins); ?></th>
			<?php
			foreach ($events as $id => $col_events) {
				?>
				<td id="<?php echo $id.'_'.sprintf('%02d:%02d', $current_hour, $current_mins) ?>">
				<?php
					echo $this->_getTimedTableCellContents($col_events[sprintf('%02d:%02d', $current_hour, $current_mins)], $show_event_links, $root_nodes, ($current_mins + ($current_hour * 60)), $partition_time, $partition_height, $width);
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
		<script language="JavaScript" type="text/javascript">
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

	}//end _printClickChildScript()


//--         FUNCTIONS TO GET EVENTS FOR DISPLAY        --//


	/**
	* Get the events to show in year view for the specified year
	*
	* @return array	( assetid => asset_type_code )
	*/
	function getYearEvents($year)
	{
		$res = Array();
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		
		// get the where clauses ready for tree restrictions
		$root_nodes = $this->attr('root_nodes');
		$trees =& $this->_treesQuery($root_nodes);
		$tree_clauses = Array();
		foreach($trees as $tree) {
			$tree_clauses[] .= '(SUBSTRING(t.treeid from 1 for '.strlen($tree).') LIKE '.$db->quote($tree.'%').')';
		}

		// formulate the date restriction clauses
		$start_ts = strtotime("$year-01-01 00:00:00");
		$next_year = $year + 1;
		$end_ts = strtotime("$next_year-01-01 00:00:00") - 1;
		
		// Get our single events
		$date_sql = '(cd.start_date_year = '.$db->quote($year).') OR (cd.end_date_year = '.$db->quote($year).') OR  ((cd.start_date_ts < '.$db->quote($start_ts).') AND (cd.end_date_ts > '.$db->quote($start_ts).'))';
		$sql = $this->_buildSingleEventQuery($trees)."\n AND (".$date_sql.")\n AND (".implode(' OR ', $tree_clauses).')';	
		$single_events = $db->getAssoc($sql);
		assert_valid_db_result($single_events);


		// Get our recurring events
		$date_sql = 'cd.start_date_ts < '.$db->quote($end_ts);
		$sql = $this->_buildRecurringEventQuery(&$trees, 'year', $year)."\n AND (".$date_sql.")\n AND (".implode(' OR ', $tree_clauses).')';	
		$recurring_events = $db->getAssoc($sql);
		assert_valid_db_result($recurring_events);

		return $single_events + $recurring_events;

	}


	/**
	* Get the events to show in year view for the specified year, separated into months
	*
	* @return array ( month_number => array ( assetid => asset_type_code ))
	*/
	function getYearEventsByMonth($year)
	{
		$all_events = $this->getYearEvents($year);    
        $start_ts = strtotime("$year-01-01 00:00:00");    
        $next_year = $year + 1;    
        $end_ts = strtotime("$next_year-01-01 00:00:00") - 1;    
		$res = Array();

		foreach (range(1, 12) as $month_num) {
			$res[$month_num] = Array();
		}
		
		foreach ($all_events as $id => $details) {
			if ($details['type_code'] == 'calendar_event_single') {
				if ($details['start_date_year'] == $year) {
					$res[$details['start_date_mon']][$id] = $details;
					for ($current_month = $details['start_date_mon']; ($current_month <= 12) && ($current_month <= $details['start_date_mon'] + $details['end_date_ms'] - $details['start_date_ms']); $current_month++) {
						$res[$current_month][$id] = $details;
					}
				} else {
					// must have carried over from last year
					$current_month = 1;
					while ($current_month <= 12 && (strtotime("$year-$current_month-01") <= $details['end_date_ts'])) {
						$res[$current_month][$id] = $details;
						$current_month++;
					}
				}
			} else {
				// recurring event
				$next_occurrence_date = Calendar_Event_Recurring::getFirstOccurrenceAfter(sprintf('%04d-%02d-%02d', $year, 1, 1), sprintf('%04d-%02d-%02d', $details['start_date_year'], $details['start_date_mon'], $details['start_date_mday']), $details['frequency'], $details['period']);
				while (strtotime($next_occurrence_date) < $end_ts) {
					$real_date_comps = Calendar_Event::getDateComponents($next_occurrence_date, 'start_date_');
					$res[$real_date_comps['start_date_mon']][$id] = $details + $real_date_comps;
					list($ne_year, $ne_month, $ne_day) = explode('-', $next_occurrence_date);
					increment_month($ne_month, $ne_year);				
					$next_occurrence_date =  Calendar_Event_Recurring::getFirstOccurrenceAfter(sprintf('%04d-%02d-%02d', $ne_year, $ne_month, 1), sprintf('%04d-%02d-%02d', $details['start_date_year'], $details['start_date_mon'], $details['start_date_mday']), $details['frequency'], $details['period']);
				}
			}
		}
		return $res;
	}

	/**
	* Get the events to show in year view for the specified year, separated into months and days
	*
	* @return array ( month_number => array ( day_number => array (assetid => array(...))))
	*/
	function getYearEventsByMonthAndDay($year)
	{
		$res = Array();
		$all_events = $this->getYearEventsByMonth($year);  
		foreach (range(1, 12) as $month_num) {
			$res[$month_num] = Array();
			foreach (range(1, days_in_month($month_num, $year)) as $day_num) $res[$month_num][$day_num] = Array();
		}
		foreach ($all_events as $month_num => $month_events) {
			$start_ts = strtotime("$year-$month_num-01 00:00:00");
			$next_year = ($month_num == 12) ? $year + 1 : $year;
			$end_ts = strtotime($next_year.'-'.sprintf('%02d', ($month_num+1)%12).'-01 00:00:00') - 1;
			$month_length = days_in_month($month_num, $year);
			foreach ($month_events as $id => $details) {
				if ($details['type_code'] == 'calendar_event_single') {
					if ($details['start_date_mon'] == $month_num) {
						$res[$month_num][$details['start_date_mday']][$id] = $details;
						for ($current_day = $details['start_date_mday']; ($current_day <= $month_length) && (strtotime("$year-$month_num-$current_day") < $details['end_date_ts']); $current_day++) {
							$res[$month_num][$current_day][$id] = $details;
						}
					} else {
						// must be a carryover
						for ($current_day = 1; ($current_day <= $month_length) && (strtotime("$year-$month_num-$current_day") < $details['end_date_ts']); $current_day++) {
							$res[$month_num][$current_day][$id] = $details;
						}
					}
				} else {
					// recurring event
					$next_occurrence_date = Calendar_Event_Recurring::getFirstOccurrenceAfter(sprintf('%04d-%02d-%02d', $year, $month_num, 1), sprintf('%04d-%02d-%02d', $details['start_date_year'], $details['start_date_mon'], $details['start_date_mday']), $details['frequency'], $details['period']);
					while (strtotime($next_occurrence_date) < $end_ts) {
						$real_date_comps = Calendar_Event::getDateComponents($next_occurrence_date, 'start_date_');
						$res[$month_num][$real_date_comps['start_date_mday']][$id] = $details + $real_date_comps;
						list($ne_year, $ne_month, $ne_day) = explode('-', $next_occurrence_date);
						increment_date($ne_day, $ne_month, $ne_year);
						$next_occurrence_date = Calendar_Event_Recurring::getFirstOccurrenceAfter(sprintf('%04d-%02d-%02d', $ne_year, $ne_month, $ne_day), sprintf('%04d-%02d-%02d', $details['start_date_year'], $details['start_date_mon'], $details['start_date_mday']), $details['frequency'], $details['period']);
					}
				}
			}
		}
		return $res;
	}


	/**
	* Get the events to show in month view for the specified month
	*
	* @return array	( assetid => array(...))
	*/
	function getMonthEvents($year, $month)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		
		// get the where clauses ready for tree restrictions
		$root_nodes = $this->attr('root_nodes');
		$trees =& $this->_treesQuery($root_nodes);
		$tree_clauses = Array();
		foreach($trees as $tree) {
			$tree_clauses[] .= '(SUBSTRING(t.treeid from 1 for '.strlen($tree).') LIKE '.$db->quote($tree.'%').')';
		}

		// formulate the date restriction clauses
		$start_ts = strtotime("$year-$month-01 00:00:00");
		$next_month = ($month % 12) + 1;
		$next_year = ($next_month == 1) ? $year + 1 : $year;
		$end_ts = strtotime("$next_year-$next_month-01 00:00:00") - 1;
		
		// Get our single events
		$date_sql = '((cd.start_date_year = '.$db->quote($year).' AND (cd.start_date_mon = '.$db->quote($month).' OR cd.end_date_mon = '.$db->quote($month).')) OR (cd.start_date_ts < '.$db->quote($start_ts).' AND cd.end_date_ts > '.$db->quote($start_ts).'))';

		$sql = $this->_buildSingleEventQuery($trees)."\n AND (".$date_sql.")\n AND (".implode(' OR ', $tree_clauses).')';		
		$single_events = $db->getAssoc($sql);
		assert_valid_db_result($single_events);


		// Get our recurring events
		$date_sql = 'cd.start_date_ts < '.$db->quote($end_ts);
		$sql = $this->_buildRecurringEventQuery(&$trees, 'month', "$year-$month")."\n AND (".$date_sql.")\n AND (".implode(' OR ', $tree_clauses).')';	
		$recurring_events = $db->getAssoc($sql);
		//bam("Recurring SQL: $sql");
		assert_valid_db_result($recurring_events);

		return $single_events + $recurring_events;
	}


	/**
	* Get the events to show in month view for the specified month, separated by day
	*
	* @return array	$day => array ( assetid => array(...) )
	*/
	function getMonthEventsByDay($year, $month)
	{
		$all_events = $this->getMonthEvents($year, $month);
		$month_length = days_in_month($month, $year);
		$month_start_ts = strtotime(sprintf('%04d-%02d-', $year, $month).'-01 00:00:00 ');
		$month_end_ts = strtotime(sprintf('%04d-%02d-', (($month == 12) ? ($year+1) : $year), ($month+1)%12).'-01 00:00:00 GMT')-1;

		$res = Array();
		for ($i = 1; $i <= $month_length; $i++) {
			$res[$i] = Array();
		}

		foreach ($all_events as $id => $details) {
			if ($details['type_code'] == 'calendar_event_single') {
				if ($details['start_date_mon'] == $month) {
					$res[$details['start_date_mday']][$id] = $details;
					// now figure out if it goes beyond this day
					for ($current_day = $details['start_date_mday']; $current_day <= $details['start_date_mday'] + ($details['end_date_ds'] - $details['start_date_ds']); $current_day++) {
						if ($current_day > $month_length) break;
						$res[$current_day][$id] = $details;
					}

				} else {
					// must have carried on since last month
					$current_day = 1;
					while (($current_day <= $month_length) && (strtotime("$year-$month-$current_day 12:00:00 GMT")/60/60/24 < $details['end_date_ts'])) {
						//bam("Event $id ends at ".$details['end_date_ts'].'('.gmdate('r', $details['end_date_ts']).' and now is '.strtotime("$year-$month-$current_day").'('.gmdate('r',strtotime("$year-$month-$current_day GMT")));
						$res[$current_day][$id] = $details;
						$current_day++;
					}
				}
			} else {
				// recurring event
				$next_occurrence_date = Calendar_Event_Recurring::getFirstOccurrenceAfter(sprintf('%04d-%02d-%02d', $year, $month, 1), sprintf('%04d-%02d-%02d', $details['start_date_year'], $details['start_date_mon'], $details['start_date_mday']), $details['frequency'], $details['period']);
				while (strtotime($next_occurrence_date.' GMT') < $month_end_ts) {
					$real_date_comps = Calendar_Event::getDateComponents($next_occurrence_date, 'start_date_');
					$res[$real_date_comps['start_date_mday']][$id] = $details + $real_date_comps;
					list($ne_year, $ne_month, $ne_day) = explode('-', $next_occurrence_date);
					increment_date($ne_day, $ne_month, $ne_year);
					$next_occurrence_date = Calendar_Event_Recurring::getFirstOccurrenceAfter(sprintf('%04d-%02d-%02d', $ne_year, $ne_month, $ne_day), sprintf('%04d-%02d-%02d', $details['start_date_year'], $details['start_date_mon'], $details['start_date_mday']), $details['frequency'], $details['period']);
				}
			}

		}
		return $res;
	}


	/*
	* Get the events to show in week view for the week containing the given YYYY-MM-DD date
	*
	* @return array (assetid => Array(...) ))
	*/
	function getWeekEvents($date)
	{
		$root_nodes = $this->attr('root_nodes');
		$week_start_day = $this->attr('week_starts_on');
		
		$first_day = iso8601_ts($date) - ((7+(date('w', iso8601_ts($date)) - $week_start_day))%7)*86400;
		$cal_date = date('Y-m-d', $first_day);
		$last_day = date('Y-m-d', $first_day + 6*86400);
		
		$single_result =& $this->_multipleDaySingleQuery($root_nodes, $cal_date, 7);		
		$recur_result =& $this->_multipleDayRecurringQuery($root_nodes, $cal_date, 'week');
		
		$res = $single_result;

		foreach(array_keys($recur_result) as $key) {
			$event =& $recur_result[$key];
			$event_start_date = sprintf('%04d-%02d-%02d', $event['start_date_year'], $event['start_date_mon'], $event['start_date_mday']);
			$event_end_date = sprintf('%04d-%02d-%02d', $event['end_date_year'], $event['end_date_mon'], $event['end_date_mday']);
			
			$event_last_day = $last_day;
			
			if ($event['stop_date'] != str_repeat('-',10)) {
				if ($event['stop_date'] < $last_day) {
					$event_last_day = $event['stop_date'];
				}
				//$event_last_day = min($event['stop_date'], $last_day);
			}
			
			$next_occur = Calendar_Event_Recurring::getFirstOccurrenceAfter($cal_date, $event_start_date, $event['frequency'], $event['period'], $event_last_day);
			$i = 0;
			
			while (($next_occur !== false) && ($next_occur <= $last_day)) {				
				$next_occur_ds = round(iso8601_ts($next_occur) / 86400);
				// days between occurrences
				$difference = $next_occur_ds - $event['start_date_ds'];
				
				if (!is_null($event['end_date_ds'])) {
					// end date is specified
					if ($event['start_date_ds'] != $event['end_date_ds']) {
						
						// multiple day
						$start_time = mktime(12, 0, 0, $event['start_date_mon'], $event['start_date_mday'], $event['start_date_year'])+$difference*86400;
						$start_date = date('Y-m-d', $start_time);
						
						$end_time = mktime(12, 0, 0, $event['end_date_mon'], $event['end_date_mday'], $event['end_date_year'])+$difference*86400;
						$end_date = date('Y-m-d', $end_time);
							
						$res[$key.':'.$i] = array_merge($event, Array(
							'start_date_year' => date('Y', $start_time), 'start_date_mon' => date('n', $start_time),
							'start_date_mday' => date('j', $start_time),
							'end_date_year' => date('Y', $start_time), 'end_date_mon' => date('n', $start_time),
							'end_date_mday' => date('j', $start_time),
							'end_date_hours' => 24, 'end_date_minutes' => 0));
						
						$i++;
						
						for ($inter_time = $start_time + 86400; $inter_time < $end_time; $inter_time += 86400) {
							$inter_date = date('Y-m-d', $inter_time);
							$res[$key.':'.$i] = array_merge($event, Array(
								'start_date_year' => date('Y', $inter_time), 'start_date_mon' => date('n', $inter_time),
								'start_date_mday' => date('j', $inter_time),
								'end_date_year' => date('Y', $inter_time), 'end_date_mon' => date('n', $inter_time),
								'end_date_mday' => date('j', $inter_time),
								'start_date_hours' => 0, 'start_date_minutes' => 0,
								'end_date_hours' => 24, 'end_date_minutes' => 0,
								'partition_start' => $this->attr('day_starts_at')));
							$i++;
						}
						
						$res[$key.':'.$i] = array_merge($event, Array(
							'start_date_year' => date('Y', $end_time), 'start_date_mon' => date('n', $end_time),
								'start_date_mday' => date('j', $end_time),
							'end_date_year' => date('Y', $end_time), 'end_date_mon' => date('n', $end_time),
								'end_date_mday' => date('j', $end_time),
							'start_date_hours' => 0, 'start_date_minutes' => 0,
							'partition_start' => $this->attr('day_starts_at')));
						$i++;
						
					} else {
						$start_time = mktime(12, 0, 0, $event['start_date_mon'], $event['start_date_mday'], $event['start_date_year'])+$difference*86400;
						$start_date = date('Y-m-d', $start_time);
						// single day
						$res[$key.':'.$i] = array_merge($event, Array(
							'start_date_year' => date('Y', $start_time), 'start_date_mon' => date('n', $start_time), 'start_date_mday' => date('j', $start_time),
							'end_date_year' => date('Y', $start_time), 'end_date_mon' => date('n', $start_time),
							'end_date_mday' => date('j', $start_time),
							));
						$i++;
					}
				} else {
					$start_time = mktime(12, 0, 0,  $event['start_date_mon'], $event['start_date_mday'], $event['start_date_year'])+$difference*86400;
					$start_date = date('Y-m-d', $start_time);
					// single day in any case
					$res[$key.':'.$i] = array_merge($event, Array(
							'start_date_year' => date('Y', $start_time), 'start_date_mon' => date('n', $start_time), 'start_date_mday' => date('j', $start_time),
							'end_date_ds' => round(iso8601_ts($key) / 86400),
							'end_date_year' => date('Y', $start_time), 'end_date_mon' => date('n', $start_time),
							'end_date_mday' => date('j', $start_time),
							));
					$i++;
					
				}
			
				// get the next occurrence
				$next_d = increment_iso_by_days($next_occur, 1);
				$next_occur = Calendar_Event_Recurring::getFirstOccurrenceAfter( increment_iso_by_days($next_occur, 1), $event_start_date, $event['frequency'], $event['period'], $event_last_day);
			
			}//end while
			
			
			
		}
		
		return $res;

	}


	/*
	* Get the events to show in week view for the week containing the given YYYY-MM-DD date, separated by day
	*
	* @return array ($iso_date => array ( $assetid => Array(...) ))
	*/
	function getWeekEventsByDay($date)
	{
		$res = Array();
		// get the list of columns using the week-starts-on and week-days-to-show attributes
		$today_ts = strtotime($date);
		$today_day = date('w', $today_ts);
		$week_start_day = $this->attr('week_starts_on');
		$enabled_days = $this->attr('week_view_show_days');
		$ordered_days = Array();
		for ($i = $week_start_day; $i < $week_start_day + 7; $i++) {
			$ordered_days[] = $i%7;
		}
		$passed_today = false;
		$events = $this->getWeekEvents($date);
		foreach ($ordered_days as $current_day) {
			if ($current_day == $today_day) $passed_today = true;
			if ($enabled_days[$current_day]) {
				$key = date('Y-m-d', strtotime(($passed_today ? '' : 'last ').$this->day_names_full[$current_day], $today_ts));
				$res[$key] = Array();
				foreach(array_keys($events) as $event_key) {		
					$event =& $events[$event_key];
					$start_iso = sprintf('%04d-%02d-%02d', $event['start_date_year'], $event['start_date_mon'], $event['start_date_mday']);
					$end_iso = sprintf('%04d-%02d-%02d', $event['end_date_year'], $event['end_date_mon'], $event['end_date_mday']);
					
					if (($key >= $start_iso) && (is_null($event['end_date_ds']) || ($key <= $end_iso))) {
						// make a copy of the current event which we will then modify
						$new_event = $event;
						list($new_event['start_date_year'], $new_event['start_date_mon'], $new_event['start_date_mday']) = explode('-', $key);
						list($new_event['end_date_year'], $new_event['end_date_mon'], $new_event['end_date_mday']) = explode('-', $key);
						
						if ($key != $start_iso) {
							list($new_event['start_date_hours'], $new_event['start_date_minutes']) = explode(':', $this->attr('day_starts_at'));
							$new_event['start_date_ts'] = iso8601_ts($key.' '.$this->attr('day_starts_at').':00');
						}
						
						if ($key != $end_iso) {
							list($new_event['end_date_hours'], $new_event['end_date_minutes']) = explode(':', $this->attr('day_ends_at'));
							$new_event['end_date_ts'] = iso8601_ts($key.' '.$this->attr('day_ends_at').':00');
						}
						$res[$key][$event_key] = $new_event;
					}
				}
			}
		}
		return $res;
	}


	/*
	* Get the events to show in week view for the week containing the given YYYY-MM-DD date, 
	* separated by day and time
	*
	* @return array ($iso_date => array ( $assetid => Array(...) ))
	*/
	function getWeekEventsByDayAndTime($date, $first_time='', $time_partition='', $last_time='') 
	{
		if (!$first_time) $first_time = $this->attr('day_starts_at');
		if (!$time_partition) $time_partition = $this->attr('day_partition_time');
		if (!$last_time) $last_time = $this->attr('day_ends_at');
		$events = $this->getWeekEventsByDay($date);
		$partition_keys = Array();
		list($hour, $mins) = explode(':', $last_time);
		$end_ts = $mins + (60 * $hour);
		list($hour, $mins) = explode(':', $first_time);
		$ts = $mins + (60 * $hour);
		$start_ts = $ts;
		while ($ts < $end_ts) {
			$partition_keys[$ts] = sprintf('%02d', $ts/60).':'.sprintf('%02d', $ts%60);
			$ts += $time_partition;
		}
		$partitions = array_flip($partition_keys);
		foreach($partitions as $key => $val) $partitions[$key] = Array();
		$partitions['all_day'] = Array();

		$res = Array();
		foreach ($events as $day => $day_events) {
			$res[$day] = $partitions; // nb assignment by copy
			foreach ($day_events as $assetid => $details) {
				$event_ts = $details['start_date_minutes']+ (60 * $details['start_date_hours']);
				if (is_null($details['start_date_hours'])) {
					// all day event we think
					$res[$day]['all_day'][$assetid] = $details;
				} else {
					if ($event_ts < $start_ts) continue;
					foreach ($partition_keys as $partition_ts => $key) {
						if (($event_ts - $partition_ts) < $time_partition) {
							$res[$day][$key][$assetid] = $details;
							break;
						}
					}
				}
			}
		}
		return $res;
	}


	/**
	* Get the events on the specified day
	*
	* @param string	$date	The date of the day to get events for, in ISO format (YYYY-MM-DD)
	*
	* @return array (assetid => Array(...) ))
	*/
	function getDayEvents($date)
	{
		$root_nodes = $this->attr('root_nodes');
		
		$single_result =& $this->_multipleDaySingleQuery($root_nodes, $date, 1);		
		$recur_result =& $this->_multipleDayRecurringQuery($root_nodes, $date, 'day');
		
		print '<span style="border:1px solid gold">';
		pre_echo($single_result);
		print '</span>';
		
		print '<span style="border:1px solid red">';
		pre_echo($recur_result);
		print '</span>';
		
		$res = $single_result + $recur_result;
		print '<div style="border:3px solid blue">';
		pre_echo($res);
		print '</div>';		
		$date_stamp = floor(iso8601_ts($date.' 12:00:00') / 86400);
		
		// reset the results so that those which go over multiple days are appropriately
		//  handled
		foreach(array_keys($res) as $key) {
			$res_item =& $res[$key];
			list($res_item['start_date_year'], $res_item['start_date_mon'], $res_item['start_date_mday']) = explode('-', $date);
			list($res_item['end_date_year'], $res_item['end_date_mon'], $res_item['end_date_mday']) = explode('-', $date);
			if ($date_stamp != $res_item['start_date_ds']) {
				list($res_item['start_date_hours'],$res_item['start_date_minutes']) = explode(':', $this->attr('day_starts_at'));
				$res_item['start_date_ts'] = iso8601_ts($date.' '.$this->attr('day_starts_at').':00');
			}
			if ($date_stamp != $res_item['end_date_ds']) {
				list($res_item['end_date_hours'],$res_item['end_date_minutes']) = explode(':', $this->attr('day_ends_at'));
				$res_item['end_date_ts'] = iso8601_ts($date.' '.$this->attr('day_ends_at').':00');
			}
		}
		

		
		return $res;
	}


	/**
	* Get the events on the specified day, separated by time into the partitions specified
	*
	* @return array ( partition_start_time => array ( assetid => array(...)))
	*/
	function getDayEventsByTime($date, $first_time='', $time_partition='', $last_time='') 
	{
		if (!$first_time) $first_time = $this->attr('day_starts_at');
		if (!$time_partition) $time_partition = $this->attr('day_partition_time');
		if (!$last_time) $last_time = $this->attr('day_ends_at');
		$events = $this->getDayEvents($date);
		$partition_keys = Array();
		list($hour, $mins) = explode(':', $last_time);
		$end_ts = $mins + (60 * $hour);
		list($hour, $mins) = explode(':', $first_time);
		$ts = $mins + (60 * $hour);
		$start_ts = $ts;
		while ($ts < $end_ts) {
			$partition_keys[$ts] = sprintf('%02d', $ts/60).':'.sprintf('%02d', $ts%60);
			$ts += $time_partition;
		}
		$partitions = array_flip($partition_keys);
		foreach($partitions as $key => $val) $partitions[$key] = Array();

		$res = $partitions;
		foreach ($events as $assetid => $details) {
			$event_ts = $details['start_date_minutes']+ (60 * $details['start_date_hours']);
			if ($event_ts < $start_ts) continue;
			foreach ($partition_keys as $partition_ts => $key) {
				if (($event_ts - $partition_ts) < $time_partition) {
					$res[$key][$assetid] = $details;
					break;
				}
			}
		}
		return $res;
	}


	/**
	* Get events on the specified date, separated by root node and time partition
	*
	* @return array ( root_node_id => array ( partition_start_time => array ( assetid => array (...)))
	*/
	function getDayEventsByRootNodeAndTime($date, $first_time='', $time_partition='', $last_time='', $root_nodes='') 
	{
		if (!$first_time) $first_time = $this->attr('day_starts_at');
		if (!$time_partition) $time_partition = $this->attr('day_partition_time');
		if (!$last_time) $last_time = $this->attr('day_ends_at');
		if (!$root_nodes) $root_nodes = array_keys($this->attr('root_nodes'));
		$events = $this->getDayEvents($date);
		$partition_keys = Array();
		list($hour, $mins) = explode(':', $last_time);
		$end_ts = $mins + (60 * $hour);
		list($hour, $mins) = explode(':', $first_time);
		$ts = $mins + (60 * $hour);
		$start_ts = $ts;
		while ($ts < $end_ts) {
			$partition_keys[$ts] = sprintf('%02d', $ts/60).':'.sprintf('%02d', $ts%60);
			$ts += $time_partition;
		}
		$partitions = array_flip($partition_keys);
		foreach($partitions as $key => $val) $partitions[$key] = Array();

		$res = Array();
		foreach ($root_nodes as $node_id) $res[$node_id] = $partitions;

		foreach ($events as $assetid => $details) {
			$event_ts = $details['start_date_minutes']+ (60 * $details['start_date_hours']);
			if ($event_ts < $start_ts) continue;
			foreach ($partition_keys as $partition_ts => $key) {
				if (($event_ts - $partition_ts) < $time_partition) {
					$res[$details['root_node']][$key][$assetid] = $details;
					break;
				}
			}
		}
		
		return $res;
	}


//--        HELPER FUNCTIONS FOR EVENT EXTRACTION        --//


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
	
	* @return int				minimum of 1
	* @access private
	*/
	function _maxOverlappingEvents(&$events, $start, $end, $spacing)
	{
		if (empty($events) || !is_array($events)) return 1;
		
		list($start_time_h, $start_time_m) = explode(':',$start);
		list($end_time_h, $end_time_m) = explode(':',$end);
		
		// turn these times into number of minutes since midnight
		$start_time = $start_time_h * 60 + $start_time_m;
		$end_time   = $end_time_h   * 60 + $end_time_m;
		
		$end_section = (ceil(($end_time - $start_time) / $spacing) - 1);
		
		$event_lists = Array();
		
		foreach($events as $event) {			
			$event_start_time = $event['start_date_hours'] * 60 + $event['start_date_minutes'];
			if (is_null($event['end_date_hours']) || is_null($event['end_date_minutes'])) {
				$event_end_time   = $event_start_time;
			} else {
				$event_end_time   = $event['end_date_hours']   * 60 + $event['end_date_minutes'];
			}
			$event_start_section = floor(($event_start_time - $start_time) / $spacing);
			$event_end_section = (ceil(($event_end_time - $start_time) / $spacing) - 1);
			
			for($i = $event_start_section; $i <= min($end_section, $event_end_section); $i++) {
				if (!isset($event_lists[$i])) $event_lists[$i] = 0;
				$event_lists[$i]++;
			}
			
		}
			
		array_multisort($event_lists, SORT_DESC, SORT_NUMERIC);
		return max(1,reset($event_lists));
				
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
		'SELECT a.assetid, a.type_code, a.name, ('.$this->_treeCaseQuery('t.treeid',$trees).') as root_node, cd.* FROM ('.SQ_TABLE_RUNNING_PREFIX.'asset a join ('.SQ_TABLE_RUNNING_PREFIX.'asset_link l join '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree t on l.linkid = t.linkid) on l.minorid = a.assetid) join '.SQ_TABLE_RUNNING_PREFIX."calendar_date_value cd on a.assetid = cd.assetid WHERE a.type_code = ".$db->quote('calendar_event_single');
		
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
		list($year,$month,$day) = sscanf(substr($iso,0,10), '%4d-%2d-%2d');
		if (!$month) $month = 12;
		if (!$day) $day = days_in_month($month, $year);
		$iso_ds = floor(strtotime("$year-$month-$day 12:00:00 GMT") / 86400);
		
		$weekday = date('w', $iso_ts);
		$wdom_list = weekday_dates_in_month($weekday, $month, $year);
		$wdom = array_search($day, $wdom_list) + 1;
		$rwdom = count($wdom_list) - $wdom + 1;
		
		$ms = $year * 12 + $month;
				
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		
		$sql = 
		'SELECT a.assetid, a.type_code, a.name, ('.$this->_treeCaseQuery('t.treeid',$trees).") as root_node, cd.* \nFROM (".SQ_TABLE_RUNNING_PREFIX.'asset a join ('.SQ_TABLE_RUNNING_PREFIX.'asset_link l join '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree t on l.linkid = t.linkid) on l.minorid = a.assetid) join '.SQ_TABLE_RUNNING_PREFIX."calendar_date_value cd on a.assetid = cd.assetid \nWHERE (a.type_code = ".$db->quote('calendar_event_recurring').") \nAND ";
		
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
				$sql_freqs_m[] = '(frequency = '.$db->quote('MFN').' AND ('.$db->quote(intval($day)).' = start_date_mday))';
				  
				// every X months on xth weekday of the month ('2nd Thursday')
				$sql_freqs_m[] = '(frequency = '.$db->quote('MFW').' AND ('.$db->quote(date('w', $iso_ts)).' = start_date_wday) and ('.$db->quote($wdom).' = start_date_wdom))';
				  
				// every X months on xth last of the month
				$sql_freqs_m[] = '(frequency = '.$db->quote('MRN').' AND ('.$db->quote(days_in_month(substr($iso,5,2), substr($iso,0,4)) - intval(substr($iso,8,2)) + 1).' = start_date_rmday))';
				  
				// every X months on xth last weekday of the month
				$sql_freqs_m[] = '(frequency = '.$db->quote('MRW').' AND ('.$db->quote($rwdom).' = start_date_rwdom))';
				  
				// combine all the month rules together and add a period clause
				$sql_freqs[] = '(('.implode(' OR ', $sql_freqs_m).') AND MOD('.$db->quote($ms).' - start_date_ms,period) = 0)';
				
			break;
			
			case 'week' :
				// if we go into next week, we must put the next week's stuff into it
				// if we go into next month, we must put the 1st week into it				
				$days_in_month = days_in_month($month,$year);
				
				$req_next_week = ($day % 7 != 1);
				$req_rev_next_week = (($days_in_month - $day + 1) % 7 != 1);
				$req_next_month = ($day + 6 > $days_in_month);
				
				$last_day = $iso_ds + 6;
				
				
			
				$sql_freqs[] = 'frequency = '.$db->quote('DWD');
				  
				// note: the slightly redundant "*1.0" is so Postgres interprets it as a
				// double instead of an int, blowing away our attempts to use floor()
				$sql_freqs[] = '(frequency = '.$db->quote('DED').' AND (floor('.$this->_dayDiffFragment($db->quote($iso_ds-1), 'start_date_ds').'/(period*1.0)) < floor('. $this->_dayDiffFragment($db->quote($iso_ds+6), 'start_date_ds').'/(period*1.0))))';
				 
				$sql_freqs[] = '(frequency = '.$db->quote('WEW').' AND (floor('.$this->_dayDiffFragment($db->quote($iso_ds-1), 'start_date_ds').'/(period*7.0)) < floor('. $this->_dayDiffFragment($db->quote($iso_ds+6), 'start_date_ds').'/(period*7.0))))';
				  
				$sql_freqs_m = Array();
				  
				// every X months on xth of the month
				//$this_month = 'start_date_mon = '.$db->quote($month);
				//$days_in_month = days_in_month($month, $year);
				$sql_freqs_m[] = '(frequency = '.$db->quote('MFN').' AND start_date_mday BETWEEN '.$db->quote(intval($day)).' AND '. $db->quote(min($day+6, $days_in_month)) .')';
				  
				// every X months on xth weekday of the month
				$sql_freqs_m[] = '(frequency = '.$db->quote('MFW').' AND start_date_wdom IN ( '.$db->quote($wdom).($req_next_week ? ','.$db->quote($wdom+1) : '').'))';
				  
				// every X months on xth last day of the month
				$sql_freqs_m[] = '(frequency = '.$db->quote('MRN').' AND start_date_rmday BETWEEN '.$db->quote(max($days_in_month - $day - 5, 1)).' AND '.$db->quote(intval($days_in_month - $day + 1)).')';
				
				// every X months on xth last weekday of the month
				$sql_freqs_m[] = '(frequency = '.$db->quote('MRW').' AND start_date_rwdom IN ( '.$db->quote($rwdom).($req_rev_next_week ? ','.$db->quote($rwdom-1) : '').'))';
				
				$sql_freqs[] = '(('.implode("\n\t".' OR ', $sql_freqs_m).') AND (MOD('.$db->quote($ms).' - start_date_ms,period) = 0))';
				
				// rolling over to a new month? Oh crap :o
				if($req_next_month) {
					if ($month == 12) {
						$new_month = 1;
						$new_year = $year + 1;
					} else {
						$new_month = $month + 1;
						$new_year = $year;						
					}
					
					$req_rev_next_week = (($day + 28) > days_in_month($new_month, $new_year));
					$wdom_list = weekday_dates_in_month($weekday, $new_month, $new_year);
					$days_in_new_month = days_in_month($new_month,$new_year);
					
					// wipe the table clean
					$sql_freqs_m = Array();
					
					$sql_freqs_m[] = '(frequency = '.$db->quote('MFN').' AND start_date_mday BETWEEN '.$db->quote(1).' AND '. $db->quote($day+6-$days_in_month) .')';
				
				  	// every X months on xth weekday of the month - it's always going to
					// be the first week of a new month
					$sql_freqs_m[] = '(frequency = '.$db->quote('MFW').' AND start_date_wdom = 1)';
				
				  	// every X months on xth last day of the month
					$sql_freqs_m[] = '(frequency = '.$db->quote('MRN').' AND start_date_rmday BETWEEN '.$db->quote($days_in_month * 2 - $day - 5).' AND '.$db->quote(intval($days_in_month)).')';
				
					// every X months on xth last weekday of the month
					$sql_freqs_m[] = '(frequency = '.$db->quote('MRW').' AND start_date_rwdom IN ( '.$db->quote(count($wdom_list)).($req_rev_next_week ? ','.$db->quote(count($wdom_list)-1) : '').'))';
				
					$sql_freqs[] = '(('.implode("\n\t".' OR ', $sql_freqs_m).') AND (MOD('.$db->quote($ms+1).' - start_date_ms,period) = 0))';	
				}
				
			break;
			
			case 'month' :
				$last_day = $iso_ds + days_in_month($month,$year) - 1;
			
				$sql_freqs[] = '(frequency = '.$db->quote('DWD').')';
				
				$sql_freqs[] = '(frequency = '.$db->quote('DED').' AND (floor('.$this->_dayDiffFragment($db->quote($iso_ds-1), 'start_date_ds').'/period) < floor('. $this->_dayDiffFragment($db->quote($last_day), 'start_date_ds').'/period)))';
				  
				$sql_freqs[] = '(frequency = '.$db->quote('WEW').' and period = 1)';
				  
				$sql_freqs[] = '(frequency = '.$db->quote('WEW').' AND (floor('.$this->_dayDiffFragment($db->quote($iso_ds-1), 'start_date_ds').'/(period*7)) < floor('. $this->_dayDiffFragment($db->quote($last_day), 'start_date_ds').'/(period*7))))';
				  
				// all monthly views can be lumped together on this one
				$sql_freqs[] = '(frequency LIKE '.$db->quote('M%').' AND MOD('.$db->quote($ms).' - start_date_ms,period) = 0)';

				$iso = $year.'-'.sprintf('%02d', $month).'-'.days_in_month($month, $year);
				
			break;
			
			case 'year' :
				$last_day = $iso_ds+364+is_leap_year($year);
			
				$sql_freqs[] = 'frequency = '.$db->quote('DWD');
				
				$sql_freqs[] = '(frequency = '.$db->quote('DED').' AND (floor('.$this->_dayDiffFragment($db->quote($iso_ds-1), 'start_date_ds').'/period) < floor('. $this->_dayDiffFragment($db->quote($last_day), 'start_date_ds').'/period)))';
				  
				$sql_freqs[] = '(frequency = '.$db->quote('WEW').' and period = 1)';
				  
				$sql_freqs[] = '(frequency = '.$db->quote('WEW').' AND (floor('.$this->_dayDiffFragment($db->quote($iso_ds-1), 'start_date_ds').'/(period*7)) < floor('. $this->_dayDiffFragment($db->quote($last_day), 'start_date_ds').'/(period*7))))';
				  
				// all monthly views can be lumped together on this one
				// but we need to do something like the 'every day' type thing to work out
				// whether an occurrence will happen this year
				$sql_freqs[] = '(frequency like '.$db->quote('M%').' AND (floor(('.$this->_dayDiffFragment($year * 12, 'start_date_ms').') / period) < floor(('.$this->_dayDiffFragment(($year + 1) * 12, 'start_date_ms').') / period)))';
			break;
			
		}//endswitch;
		
		$sql .= '('.implode("\n".' OR ', $sql_freqs).')';
		
		// stop date
		$sql .= ' AND ('.$last_day.' > start_date_ds)';
		$sql .= ' AND (stop_date = '.$db->quote('----------').' OR stop_date >= '.$db->quote($iso).')';
		return $sql;
	
	}//end _buildRecurringEventQuery()
	
	

	/**
	* Add valid keywords for this asset to an array of keywords when asked
	*
	* @param &object Asset	$asset	the asset that triggered the event
	* @param Array			$vars	the vars that get submitted by the broadcaster
	*								we add keywords to the $vars['keywords'] array
	*
	* @return boolean
	* @access private
	*/
	function onRequestKeywords(&$broadcaster, $vars=Array())
	{
		parent::onRequestKeywords($broadcaster, $vars);
		$vars['keywords']['event_link_begin'] = Array('name'    => 'Open Link to Event Display',
											'default' => '&gt;open link&lt;',
											'type'    => 'inline'
											);
		$vars['keywords']['event_link_end'] = Array('name'    => 'Close Link to Event Display',
											'default' => '&gt;close link&lt;',
											'type'    => 'inline'
											);
	}

	
	/**
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
	* For the $period parameter, this is in days:
	* for DAY	send 1
	* for WEEK	send 7
	* for MONTH	send days_in_month($month, $year)
	* for YEAR	send 365 + is_leap_year($year);
	*
	* @param array	&$root_nodes	the root nodes to search under
	* @param string	$iso			the ISO date (Y-M-D) of the first date to show
	* @param int	$period			the number of consecutive days to show
	*
	* @return array of records from calendar date value table 
	*/
	function &_multipleDaySingleQuery(&$root_nodes, $iso, $period=1) {
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		
		$trees =& $this->_treesQuery($root_nodes);
		
		list($year,$month,$day) = sscanf($iso, '%04d-%02d-%02d');
		
		$start_date_stamp = floor(strtotime($iso.' 12:00:00 GMT')/86400);
		$end_date_stamp = $start_date_stamp + $period - 1; 
		
		$date_sql = '(cd.start_date_ds BETWEEN '.$db->quote($start_date_stamp).' and '.$db->quote($end_date_stamp).') OR (cd.end_date_ds BETWEEN '.$db->quote($start_date_stamp).' and '.$db->quote($end_date_stamp).')';
		
		// base sql query FOR SINGLE EVENTS
		$sql = $this->_buildSingleEventQuery($trees).' and ('.$date_sql.')';
		
		$tree_sql = Array();
		foreach($trees as $tree) {
			$tree_sql[] = 'substring(t.treeid from 1 for '.strlen($tree).') LIKE '.$db->quote($tree.'%');
		}
		$sql .= ' AND ('.implode(' OR ', $tree_sql).')';
		
		$result = $db->getAssoc($sql);
				
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
		
		$result = $db->getAssoc($sql);
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
		$sql = "SELECT a.assetid, t.treeid \n FROM sq_asset a join (sq_asset_link l join sq_asset_link_tree t on l.linkid = t.linkid) on l.minorid = a.assetid \n WHERE a.assetid in (";
		
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


}//end class

?>
