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
* $Id: page_calendar.inc,v 1.97 2004/10/07 06:38:36 tbarrett Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_asset_listing/page_asset_listing.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once SQ_FUDGE_PATH.'/general/datetime.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once dirname(__FILE__).'/../../calendar_event/calendar_event.inc';
require_once dirname(__FILE__).'/../../calendar_events/calendar_event_recurring/calendar_event_recurring.inc';
require_once dirname(__FILE__).'/../../lib/calendar_common.inc';

/**
* Page_Calendar
*
* Purpose: To display event assets on the frontend by year, month, week, day or individually, 
* in listing or calendar-style formats
*
* @author  Luke Wright <lwright@squiz.net>
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.97 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage calendar
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
	var $day_names = Array('Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday');


	/**
	* List of full month names, in chronological order
	* @var array
	*/
	var $month_names_full = Array(1=>'January', 2=>'February', 3=>'March', 4=>'April', 5=>'May', 6=>'June', 7=>'July', 8=>'August', 9=>'September', 10=>'October', 11=>'November', 12=>'December');


	/**
	* List of abbreviated month names, in chronological order
	* @var array
	*/
	var $month_names_short = Array(1=>'Jan', 2=>'Feb', 3=>'Mar', 4=>'Apr', 5=>'May', 6=>'June', 7=>'July', 8=>'Aug', 9=>'Sept', 10=>'Oct', 11=>'Nov', 12=>'Dec');

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

	}//end getContentsKeywords()


	/**
	* Called by the design to print the head of this asset
	*
	* @return boolean
	* @access public
	*/
	function printHead()
	{
		echo '<link rel="stylesheet" type="text/css" href="' . sq_web_path('data').'/asset_types/page_calendar/css/calendar_default.css" />';

	}//end printHead()


	/**
	* Called by the design to print the body of this asset
	*
	* @return boolean
	* @access public
	*/
	function printBody()
	{
		$res = '';
		$action = array_get_index($_REQUEST,'SQ_CALENDAR_EVENT_ACTION',false);
		if ($action) {
			$event_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset((int)$_REQUEST['SQ_CALENDAR_EVENT_ID']);
			$res = $event_asset->processEventAction($action);
			echo $res;
		}
		if (!$res) {

			$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'page_contents');
			if (empty($bodycopy_link)) return false;

			$format_bodycopy = &$GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link['minorid'], $bodycopy_link['minor_type_code']);
			if (is_null($format_bodycopy)) return false;

			$replacements = Array();


			// If we get a valid and available view type in the request, use it; otherwise use the initial view attribute.
			$enabled_views = $this->attr('enabled_views');
			$cal_view = array_get_index($_REQUEST,'SQ_CALENDAR_VIEW',false);
			if (!$cal_view || empty($enabled_views[$cal_view])) {
				$cal_view = $this->attr('initial_view');
				$_REQUEST['SQ_CALENDAR_VIEW'] = $cal_view;
			}

			$cal_date = array_get_index($_REQUEST,'SQ_CALENDAR_DATE', date('Y-m-d'));
			
			// If view is month or year, some incomplete dates can be accepted
			// (accepted for month: eg. 2004-09, accepted for year: eg. 2006)
			if (!is_iso8601($cal_date)) {
				if (($cal_view == 'month') && (strlen($cal_date) >= 7)) {
					list($year, $month) = sscanf($cal_date, '%04d-%02d');
					$cal_date = sprintf('%04d-%02d-%02d', $year, $month, 1);
				} else if (($cal_view == 'year') && (strlen($cal_date) >= 4)) {
					// still need list() here because sscanf returns array, no exceptions
					list($year) = sscanf($cal_date, '%04d');
					$cal_date = sprintf('%04d-%02d-%02d', $year, 1, 1);
				} else {
					// no good, use today's date
					$cal_date = date('Y-m-d');
				}
			}

			// if view is week, we want the first day in that week
			if ($cal_view == 'week') {
				$week_start_day = $this->day_names[$this->attr('week_starts_on')];
				if (date('l',strtotime($cal_date)) != $week_start_day) { 
					$cal_date = date('Y-m-d', strtotime('last '.$week_start_day, strtotime($cal_date)));
				}
			}

			list($year, $month, $day) = explode('-', $cal_date);
			if (!checkdate($month, $day, $year)) {
				trigger_error('Date passed to calendar page is not valid, using today\'s date as fallback', E_USER_WARNING);
				$cal_date = date('Y-m-d');
			}

			// store globally for future reference
			$_REQUEST['SQ_CALENDAR_DATE'] = $cal_date;


			// If the view is 'event' we want the eventid too
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

			echo "<script>if (typeof cal != 'undefined') cal.setDate('$cal_date')</script>";

			// print the contents of page - replacing the global keywords
			$format_bodycopy->setKeywordReplacements($replacements);
			$format_bodycopy->printBody();
		}

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

	}//end getCalendarContents()


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
			for ($i = $week_start_day; $i < $week_start_day + 37; $i++) {
				?>
					<th class="dayLabel<?php echo (($i%7)==0 || ($i%7)==6) ? ' dayLabelWeekend' : ''; ?>" style="width: 3.5ex; text-align: center"><b><?php echo substr($this->day_names[$i % 7], 0, 1); ?></b></th>
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
				for ($current_day = 1; $current_day <= days_in_month($current_month, $year); $current_day++) {
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
				while ($i < 37) {
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
			<?php
		}
		$res = ob_get_contents();
		ob_end_clean();
		return $res;

	}//end getYearView()


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
			if ($this->attr('enable_description_popups')) $this->_printPopupsScript();
			?>
			<table id="month" border="0" cellspacing="0" cellpadding="0">
				<tr>
			<?php
				// print the day headings
				for ($i=0; $i < $week_length; $i++) {
					$day_name = $this->day_names[($week_start_day + $i) % 7];
			?>
					<th style="width: <?php echo $column_width; ?>ex" class="dayLabel<?php echo (($day_name == 'Saturday') || ($day_name == 'Sunday')) ? 'Weekend' : ''; ?>">
						<?php echo $day_name; ?>
					</th>
			<?php
				}
			?>
				</tr>
				<tr>
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
				$_REQUEST['SQ_CALENDAR_DATE'] = sprintf('%04d-%02d-%02d', $year, $month, $current_day);
				$contents = $this->_getMonthCellContents($events[$current_day], $show_event_links, $root_nodes, $column_width*0.99);
				$weekend_class = in_array(date('D', strtotime("$year-$month-$current_day")), Array('Sat', 'Sun')) ? 'weekend' : '';
				$occupied_class = ($contents) ? 'eventDate' : '';
				echo '<td class="date '.$occupied_class.' '.$weekend_class.'" style="height: '.$row_height.'ex">';
				if ($contents && $drill_down_view) echo '<a class="dateLink"  href="'.$this->getHref().'?SQ_CALENDAR_VIEW='.$drill_down_view.'&amp;SQ_CALENDAR_DATE='.$_REQUEST['SQ_CALENDAR_DATE'].'">';
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
				echo '<tr>';
				for ($i=0; $i < $week_length; $i++) {
					$_REQUEST['SQ_CALENDAR_DATE'] = sprintf('%04d-%02d-%02d', $year, $month, $current_day);
					$contents = $this->_getMonthCellContents($events[$current_day], $show_event_links, $root_nodes, $column_width*0.99);
					$weekend_class = in_array(date('D', strtotime("$year-$month-$current_day")), Array('Sat', 'Sun')) ? 'weekend' : '';
					$occupied_class = ($contents) ? 'eventDate' : '';
					echo '<td class="date '.$occupied_class.' '.$weekend_class.'" style="height: '.$row_height.'ex">';
					if ($contents && $drill_down_view) echo '<a class="dateLink"  href="'.$this->getHref().'?SQ_CALENDAR_VIEW='.$drill_down_view.'&amp;SQ_CALENDAR_DATE='.$_REQUEST['SQ_CALENDAR_DATE'].'">';
					echo $current_day;
					if ($contents && $drill_down_view) echo '</a><br />';
					echo $contents;
					echo '</td>';
					$current_day++;
				}
				echo '</tr>';
			}

			// print the final week, if necessary
			if ($current_day <= $month_length) {
				$i = 0;
				echo '<tr>';
				while ($current_day <= $month_length) {
					$_REQUEST['SQ_CALENDAR_DATE'] = sprintf('%04d-%02d-%02d', $year, $month, $current_day);
					$contents = $this->_getMonthCellContents($events[$current_day], $show_event_links, $root_nodes, $column_width*0.99);
					$weekend_class = in_array(date('D', strtotime("$year-$month-$current_day")), Array('Sat', 'Sun')) ? 'weekend' : '';
					$occupied_class = ($contents) ? 'eventDate' : '';
					echo '<td class="date '.$occupied_class.' '.$weekend_class.'" style="height: '.$row_height.'ex">';
					if ($contents && $drill_down_view) echo '<a class="dateLink"  href="'.$this->getHref().'?SQ_CALENDAR_VIEW='.$drill_down_view.'&amp;SQ_CALENDAR_DATE='.$_REQUEST['SQ_CALENDAR_DATE'].'">';
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
	* @param string	$format	'list_with_headings' | 'list_without_headings' | 'calendar_format'
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
				$new_events = Array();
				foreach ($events as $id => $details) $new_events[strtok($id, ':')] = $details['type_code'];
				$this->printAssetList($new_events);
			}
		} else {
			$column_names = Array();
			$column_links = Array();
			if (array_values($this->attr('columnise_day_view_by_root_node')) != Array('other')) {
				$columns_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($this->attr('columnise_day_view_by_root_node'));
				foreach ($columns_info as $id => $info) $column_names[$id] = $info['name'];
				if (in_array('other', $this->attr('columnise_day_view_by_root_node'))) $column_names['other'] = 'other';
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
		// Might be better to do this ourselves, but for now we'll palm it off to Asset Builder
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
		} else {
			return date($title_formats[$view], strtotime($date));
		}
	}


//--        HELPER FUNCTIONS FOR CONTENTS-GETTING FUNCTIONS        --//


	/**
	* Get the HTML for an unordered list of the titles of the given events, with links if required
	*
	* @param $events		array	The events to show, $assetid => array of details
	* @param $show_links	boolean	Whether to link the names to the events' event view
	* 
	* @return string	The HTML of the list
	* @access private
	*/
	function _getTitleList($events, $show_links=true) 
	{
		$ids = Array();
		foreach ($events as $key => $stuff) $ids[] = strtok($key, ':');
		$details = $link = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($ids);
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
    
	}//end _getTitleList()


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
		$partition_time = intval($this->attr('day_partition_time'));
		$partition_height = $this->attr('day_partition_height');
		$root_nodes = $this->attr('root_nodes');
		$this->_printClickChildScript();
		$width = $this->attr('table_column_width');
		$dragging_enabled = $this->attr('enable_event_dragging');
		$mouseup_code = $dragging_enabled ? 'onmouseup="endDrag(this)"' : '';
		$mousedown_code = $dragging_enabled ? 'onmousedown="beginDrag(this)"' : '';
		if ($dragging_enabled) $this->_printEventDraggingScript();
		if ($this->attr('enable_description_popups')) $this->_printPopupsScript();
		?>
		<script type="text/javascript">
		<!--
		  function addEvent(date, time) {
			<?php
			if ($ab_id = $this->attr('asset_builder_page')) {
			?>
				  loc = "<?php echo $GLOBALS['SQ_SYSTEM']->am->getAssetHref($this->attr('asset_builder_page')); ?>?SQ_CALENDAR_DATE="+date;
				  if (time != '*') loc += '&SQ_CALENDAR_TIME='+time+'&SQ_CALENDAR_DURATION=<?php echo (int)$this->attr('day_partition_time');?>_min';
				  document.location = loc;
			<?php
			}
			?>
		  }
		//-->
		</script>
		<table id="<?php echo $table_id; ?>" border="0" cellspacing="0" cellpadding="0" summary="events in this <?php echo $table_id ?>">
		<?php
		if (!empty($column_names)) {
			// if we have several columns, print the column headers
			?>
			<tr>
				<th>Time</th>
			<?php
			foreach ($column_names as $date => $name) {
				?>
				<th onclick="<?php echo (!empty($column_links)) ? 'clickChild(this); ' : ''; ?>" class="columnLabel" nowrap="nowrap" style="width: <?php echo $width; ?>ex;  <?php echo (!empty($column_links)) ? 'cursor: pointer; cursor: hand' : ''; ?>">
					<?php
						if (!empty($column_links)) echo '<a  href="'.$column_links[$date].'">';
						echo str_replace(' ', '&nbsp;', $name);
						if (!empty($column_links)) echo '</a>';
					?>
				</th>
				<?php
			}
			?>
			</tr>
			<?php
		}
		$div_width = $width * 0.95;
		list($current_hour, $current_mins) = explode(':', $this->attr('day_starts_at'));
		list($end_hour, $end_mins) = explode(':', $this->attr('day_ends_at'));
		?>
			<tr>
			<th class="timeLabel">*</th>
			<?php
			// print the "all day" events
			foreach ($column_names as $id => $name) {
				$col_events = $events[$id];
				$event_link_date = is_iso8601($id) ? $id : $_REQUEST['SQ_CALENDAR_DATE'];
				echo '<td '.$mouseup_code.' id="'.$id.'_allday"'. (empty($col_events['all_day']) ? 'onclick="addEvent(\''.$event_link_date.'\', \'*\');"' : '').'>';
				if (empty($col_events['all_day'])) echo '&nbsp;';
				foreach($col_events['all_day'] as $assetid => $event) {
					$assetid = strtok($assetid,':');
					$event_class = $root_nodes[$event['root_node']]['class_name'];
					$onclick_code = $show_event_links ? ' onclick="clickChild(this);"' : '';
					$cursor_code = $show_event_links ? ' cursor: pointer; cursor: hand' : '';
					$current_mousedown_code = ($event['type_code'] != 'calendar_event_recurring') ? $mousedown_code : '';
					$current_mouse_over_out_code = '';
					if (isset($event['description']) && !empty($event['description'])) {
						echo '<div id="event_'.$assetid.'_description" class="popup">'.$event['description'].'</div>';
						$current_mouse_over_out_code = 'onmouseover="startTrailingPopup(\'event_'.$assetid.'_description\')" onmouseout="stopTrailingPopup(\'event_'.$assetid.'_description\')"';
					}
					echo '<div id="'.$assetid.'" '.$current_mousedown_code.' '.$current_mouse_over_out_code.' style="margin: 0px 0px 1px 0px; text-align: center; padding: 0px; width: '. $width*0.98.'ex;'.$cursor_code.'" class="'.$event_class.' event"'.$onclick_code.'>';
					echo $show_event_links ? '<a class="event '.$event_class.'"  href="'.$this->getHref().'?SQ_CALENDAR_VIEW=event&amp;SQ_CALENDAR_EVENT_ID='.$assetid.'&amp;SQ_CALENDAR_DATE='.$event_link_date.'">' : '<span class="event '.$event_class.'">';
					echo $event['name'];
					echo $show_event_links ? '</a>' : '</span>';
					echo '</div>';
				}
				echo '</td>';
			}
			?>
			</tr>
		<?php
		while (($current_hour*60 + $current_mins) < ($end_hour*60 + $end_mins)) {
			?>
			<tr>
				<th class="timeLabel"><?php echo sprintf('%d:%02d', (($current_hour % 12 == 0) ? 12 : ($current_hour % 12)), $current_mins); ?></th>
			<?php
			foreach ($column_names as $id => $name) {
				$col_events = $events[$id];
				$time_key = sprintf('%02d:%02d', $current_hour, $current_mins);
				if ($show_event_links && is_iso8601($id)) $_REQUEST['SQ_CALENDAR_DATE'] = $id;
				?>
				<td <?php echo $mouseup_code; ?> id="<?php echo $id.'_'.$time_key ?>" <?php echo empty($col_events[$time_key]) ? 'onclick="addEvent(\''.$_REQUEST['SQ_CALENDAR_DATE'].'\', \''.$time_key.'\');"' : ''; ?> style="<?php echo $width; ?>ex; height: <?php echo $partition_height; ?>ex;">
				<?php
					echo $this->_getTimedTableCellContents($col_events[$time_key], $show_event_links, $root_nodes, ($current_mins + ($current_hour * 60)), $partition_time, $partition_height, $div_width, $mousedown_code);
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
	* Get the contents of a cell in the week or day table
	*
	* @param array		$events				The events for the cell, $assetid => details
	* @param boolean	$show_event_links	Whether to link the event's name to its event view
	* @param array		$root_nodes			The root nodes for this page, used for getting classnames
	* @param int		$cell_ts			The timestamp of the earliest time in the cell
	* @param int		$partition_time		The length of the partition in minutes
	* @param float		$cell_height	The height of the cell, in ex-es
	* @param float		$event_width		The width of the event, in ex-es
	* @param string		$mousedown_code		Javascript code to put in the onmousedown attribute of the event DIVs
	*
	* @return string	The HTML for the contents of the table cell
	* @access private
	*/
	function _getTimedTableCellContents(&$events, $show_event_links, &$root_nodes, $cell_ts, $partition_time, $cell_height, $event_width, $mousedown_code)
	{

		$res = '';
		$cumulative_margin = $event_width * 0.02;
		foreach ($events as $assetid => $details) {
			$assetid_bits = current(explode(':', $assetid));
			// get top position of div
			$event_ts = $details['start_date_minutes'] + (60 * $details['start_date_hours']);
			$end_ts = $details['end_date_minutes'] + (60 * $details['end_date_hours']);
			$event_top_margin = (($event_ts - $cell_ts) / $partition_time) * $cell_height;

			// get height of div
			$event_duration = is_null($details['end_date_ts']) ? 0 : ($end_ts - $event_ts);
			$event_height = ($event_duration / $partition_time) * $cell_height;
			if ($event_height < 0) continue;

			// get class to use for div
			$event_class = $root_nodes[$details['root_node']]['class_name'];

			// other layout things
			$zero_length_code = $event_height ? '' : 'border-style: dashed; border-width: 2px; border-bottom: 0px; border-left: 0px; border-right: 0px; height: 0px; padding-top: 0px; background-color: transparent;';
			$cursor_code = $show_event_links ? 'cursor: pointer; cursor: hand;' : '';
			$inner_margin_top = $event_height ? 1 : 0;
			$onclick_code = $show_event_links ? 'onclick="clickChild(this);"' : '';
			$this_event_width = $event_width / $this->_countOverlappingEvents($events, gmdate('H:i', $event_ts*60), gmdate('H:i', $end_ts*60), $partition_time);
			$inner_width = ($this_event_width * 0.96) - 0.25;
			$current_mousedown_code = ($details['type_code'] != 'calendar_event_recurring') ? $mousedown_code : '';

			$current_mouse_over_out_code = '';
			if (isset($details['description']) && !empty($details['description'])) {
				$res .= '<div id="event_'.$assetid.'_description" class="popup">'.$details['description'].'</div>';
				$current_mouse_over_out_code = 'onmouseover="startTrailingPopup(\'event_'.$assetid.'_description\')" onmouseout="stopTrailingPopup(\'event_'.$assetid.'_description\')"';
			}

			$res .= '<div id="'.$assetid.'" '.$current_mousedown_code.' '.$current_mouse_over_out_code.' class='.$event_class.' event" style="float:left; position: absolute; width: '.$this_event_width.'ex; margin-left: '.$cumulative_margin.'ex; margin-top: '.$event_top_margin.'ex; height: '.$event_height.'ex; '.$cursor_code.' '.$zero_length_code.'" '.$onclick_code.'><div class="eventText" style="position: absolute; width: '.$inner_width.'ex; border: none; margin-top: '.$inner_margin_top.'px background-color: transparent">';
			$res .= $show_event_links ? '<a class="event '.$event_class.'"  href="'.$this->getHref().'?SQ_CALENDAR_VIEW=event&amp;SQ_CALENDAR_DATE='.$_REQUEST['SQ_CALENDAR_DATE'].'&amp;SQ_CALENDAR_EVENT_ID='.$assetid.'">' : '<span class="event '.$event_class.'">';
			$res .= $details['name'];
			$res .= $show_event_links ? '</a>' : '</span>';
			$res .= '</div></div>';
			$cumulative_margin += $this_event_width + 0.05;

		}
		return $res;

	}//end _getTimedTableCellContents()
	

	/**
	* Get the contents of a cell in the month table
	*
	* @param array		$events				The events for the cell, $assetid => details
	* @param boolean	$show_event_link	Whether to link the event's name to its event view
	* @param array		$root_nodes			The root nodes for this page, used for getting classnames
	*
	* @return string	The HTML for the contents of the table cell
	* @access private
	*/
	function _getMonthCellContents(&$events, $show_event_link, &$root_nodes, $width)
	{
		$res = '';
		foreach ($events as $assetid => $details) {
			$event_class = (isset($root_nodes[$details['root_node']]['class_name'])) ? $root_nodes[$details['root_node']] ['class_name'] : '';
			$onclick_code = $show_event_link ? ' onclick="clickChild(this); "' : '';
			$cursor_code = $show_event_link ? 'cursor: pointer; cursor: hand;' : '';

			$current_mouse_over_out_code = '';
			if (isset($details['description']) && !empty($details['description'])) {
				$res .= '<div id="event_'.$assetid.'_description" class="popup">'.$details['description'].'</div>';
				$current_mouse_over_out_code = 'onmouseover="startTrailingPopup(\'event_'.$assetid.'_description\')" onmouseout="stopTrailingPopup(\'event_'.$assetid.'_description\')"';
			}

			$res .= '<div style="width: '.$width.'ex; '.$cursor_code.'"'.$onclick_code.' '.$current_mouse_over_out_code.'><div style="width: auto; margin-bottom: 1px;" class="event '.$event_class.'">';
			if ($show_event_link) $res .= '<a href="'.$this->getHref().'?SQ_CALENDAR_VIEW=event&amp;SQ_CALENDAR_DATE='.$_REQUEST['SQ_CALENDAR_DATE'].'&amp;SQ_CALENDAR_EVENT_ID='.$assetid.'" class="event '. $event_class.'">';
			$res .= $details['name'];
			if ($show_event_link) $res .= '</a>';
			$res .= '</div></div>';
		}
		return $res;

	}//end _getMonthCellContents()


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


	function _printEventDraggingScript()
	{
		?>
		<script language="JavaScript">
		<!--
		document.onselectstart=new Function ("return false")
		var movingElt = null;
		var movingType = null;

		function beginDrag(elt) {
		  movingElt = elt;
		  movingType = (elt.parentNode.id.indexOf('allday') == -1) ? 'timed' : 'allday';
		  document.body.style.cursor='move';
		  return 0;
		}

		function endDrag(elt) {
		  destType = (elt.id.indexOf('allday') == -1) ? 'timed' : 'allday';
		  if ((movingElt != null) && (movingElt.parentNode != elt) && (destType == movingType)) {
			oldParent = movingElt.parentNode;
			for (i=0; i<elt.childNodes.length; i++) {
			  if (elt.childNodes.item(i).nodeName == '#text') {
				elt.removeChild(elt.childNodes.item(i));
			  }
			}
			if (oldParent.childNodes.length == 1) {
			  oldParent.appendChild(document.createTextNode("\u00A0"));
			}
			oldParent.removeChild(movingElt);
			elt.appendChild(movingElt);
			date_comps = elt.id.split('_');
			document.getElementById('SQ_CALENDAR_DATE').value = date_comps[0];
			if (date_comps[1] != 'allday') document.getElementById('SQ_CALENDAR_TIME').value = date_comps[1];
			document.getElementById('SQ_CALENDAR_EVENT_ID').value = movingElt.id;
			if (confirm('Move this event to ' + date_comps[0] + ' at ' + date_comps[1] + '?')) {
			  document.getElementById('dragForm').submit();
			} else {
			  for (i=0; i<oldParent.childNodes.length; i++) {
				if (oldParent.childNodes.item(i).nodeName == '#text') {
				  oldParent.removeChild(oldParent.childNodes.item(i));
				}
			  }
			  oldParent.appendChild(movingElt);
			}
		  }
		  movingElt = null;
		  document.body.style.cursor='auto';
		}
		//-->
		</script>
		<form method="GET" id="dragForm" action="<?php echo $_SERVER['PHP_SELF']; ?>">
		  <input type="hidden" name="SQ_CALENDAR_EVENT_ID" id="SQ_CALENDAR_EVENT_ID" />
		  <input type="hidden" name="SQ_CALENDAR_DATE" id="SQ_CALENDAR_DATE" />
		  <input type="hidden" name="SQ_CALENDAR_TIME" id="SQ_CALENDAR_TIME" />
		  <input type="hidden" name="SQ_CALENDAR_EVENT_ACTION" id="SQ_CALENDAR_EVENT_ACTION" value="move" />
		  <input type="hidden" name="SQ_CALENDAR_VIEW" value="<?php echo $_REQUEST['SQ_CALENDAR_VIEW']; ?>" />
		</form>
	<?php

	}//end _printEventDraggingScript()


	function _printPopupsScript()
	{
		?>
		<script type="text/javascript">
		// Funky-ass mouse-following popup code based on the script from http://javascriptkit.com

		var offsetfrommouse=[12,4] //image x,y offsets from cursor position in pixels. Enter 0,0 for no offset
		var popupObj = null;

		function getTrailObj()
		{
			if (popupObj == null) return null;
			if (document.getElementById)
			  return document.getElementById(popupObj).style
			else if (document.all)
			  exec('return document.all.'+popupObj+'.style');
		}

		function stopTrailingPopup()
		{
			document.onmousemove=null;
			getTrailObj().display='none';
		}

		function startTrailingPopup(name)
		{
			popupObj = name;
			document.onmousemove = followMouse;
		}

		function truebody()
		{
			return (!window.opera && document.compatMode && document.compatMode!="BackCompat")? document.documentElement : document.body
		}

		function followMouse(e)
		{
			if (!(document.getElementById || document.all)) return false;
			var xcoord=offsetfrommouse[0];
			var ycoord=offsetfrommouse[1];
			if (typeof e != "undefined")
			{
				xcoord+=e.pageX;
				ycoord+=e.pageY;
			}
			else if (typeof window.event !="undefined")
			{
				xcoord+=truebody().scrollLeft+event.clientX;
				ycoord+=truebody().scrollTop+event.clientY;
			}
			var docwidth = document.all ? truebody().scrollLeft + truebody().clientWidth : pageXOffset + window.innerWidth-15;
			var docheight = document.all ? Math.max(truebody().scrollHeight, truebody().clientHeight) : Math.max(document.body.offsetHeight, window.innerHeight);
			trailObj = getTrailObj();
			if (((trailObj.width != 0) && ((xcoord + trailObj.width + 3) > docwidth)) || ((trailObj.height != 0) && ((ycoord + trailObj.height) > docheight))) {
				trailObj.display="none"
			} else {
				trailObj.display="block"
			}
			trailObj.left=xcoord+"px"
			trailObj.top=ycoord+"px"
		}

		</script>
		<?php
	}


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
	* @param int	$root_node	The root node that we are restricting this search to. If left
	*							NULL this will search all root nodes
	*
	* @return int				minimum of 1
	* @access private
	*/
	function _countOverlappingEvents(&$events, $start, $end, $spacing, $root_node=null)
	{
		if (empty($events) || !is_array($events)) return 1;

		list($start_time_h, $start_time_m) = explode(':',$start);
		list($end_time_h, $end_time_m) = explode(':',$end);

		// turn these times into number of minutes since midnight
		$start_time = $start_time_h * 60 + $start_time_m;
		$end_time   = $end_time_h   * 60 + $end_time_m;

		$end_section = (ceil(($end_time - $start_time) / $spacing) - 1);

		$event_lists = Array();

		foreach(array_values($events) as $event) {
			$event_start_time = $event['start_date_hours'] * 60 + $event['start_date_minutes'];
			if (is_null($event['end_date_hours']) || is_null($event['end_date_minutes'])) {
				$event_end_time   = $event_start_time;
			} else {
				$event_end_time   = $event['end_date_hours']   * 60 + $event['end_date_minutes'];
			}
			$event_start_section = floor(($event_start_time - $start_time) / $spacing);
			$event_end_section = (ceil(($event_end_time - $start_time) / $spacing) - 1);

			for($i = $event_start_section; $i <= min($end_section, $event_end_section); $i++) {
				if((!is_null($root_node)) && ($event['root_node'] != $root_node)) {
					print '$$'; continue;
				}
				if (!isset($event_lists[$i])) $event_lists[$i] = 0;
				$event_lists[$i]++;
			}

		}

		array_multisort($event_lists, SORT_DESC, SORT_NUMERIC);
		return max(1,reset($event_lists));

	}//end _countOverlappingEvents()


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



//--         FUNCTIONS TO EXTRACT EVENTS FROM DB        --//


	/**
	* Get the events to show in year view for the specified year
	*
	* @return array	( assetid => asset_type_code )
	*/
	function getYearEvents($year)
	{
		$res = Array();
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$root_nodes = $this->attr('root_nodes');
		$trees =&get_treeids($root_nodes);
		$tree_sql = build_tree_clause($trees);

		// formulate the date restriction clauses
		$start_ts = strtotime("$year-01-01 00:00:00");
		$next_year = $year + 1;
		$end_ts = strtotime("$next_year-01-01 00:00:00") - 1;

		// Get our single events
		$date_sql = '(cd.start_date_year = '.$db->quote($year).') OR (cd.end_date_year = '.$db->quote($year).') OR  ((cd.start_date_ts < '.$db->quote($start_ts).') AND (cd.end_date_ts > '.$db->quote($start_ts).'))';
		$sql = build_single_event_query_base($trees)."\n AND (".$date_sql.")\n AND ".$tree_sql;
		$single_events = $db->getAssoc($sql);
		assert_valid_db_result($single_events);


		// Get our recurring events
		$date_sql = 'cd.start_date_ts < '.$db->quote($end_ts);
		$sql = build_recurring_event_query_base($trees, 'year', $year)."\n AND (".$date_sql.")\n AND ".$tree_sql;
		$recurring_events = $db->getAssoc($sql);
		assert_valid_db_result($recurring_events);

		$res = $single_events + $recurring_events;
		return $res;

	}//end getYearEvents()


	/**
	* Get the events to show in year view for the specified year, separated into months
	*
	* @return array ( month_number => array ( assetid => asset_type_code ))
	*/
	function getYearEventsByMonth($year)
	{
		$res = Array();

		foreach (range(1, 12) as $month_num) {
			$res[$month_num] = Array();
		}

		$cal_date = sprintf('%04d-%02d-%02d', $year, 1, 1);
		$last_day = sprintf('%04d-%02d-%02d', $year, 12, 31);

		$plain_events = $this->getYearEvents($year);
		$events =& expand_event_list($plain_events, $cal_date, $last_day);

		foreach ($events as $id => $details) {
			if ($details['start_date_year'] == $year) {
				$res[$details['start_date_mon']][$id] = $details;
			}
		}
		foreach ($res as $date => $events) process_exceptions($res[$date]);

		return $res;

	}//end getYearEventsByMonth()


	/**
	* Get the events to show in year view for the specified year, separated into months and days
	*
	* @return array ( month_number => array ( day_number => array (assetid => array(...))))
	*/
	function getYearEventsByMonthAndDay($year)
	{
		$res = Array();

		foreach (range(1, 12) as $month_num) {
			$res[$month_num] = Array();
			foreach(range(1, days_in_month($month_num, $year)) as $dom) {
				$res[$month_num][$dom] = Array();
			}
		}

		$cal_date = sprintf('%04d-%02d-%02d', $year, 1, 1);
		$last_day = sprintf('%04d-%02d-%02d', $year, 12, 31);

		$plain_events = $this->getYearEvents($year);
		$events =& expand_event_list($plain_events, $cal_date, $last_day);

		foreach ($events as $id => $details) {
			if ($details['start_date_year'] == $year) {
				$res[$details['start_date_mon']][$details['start_date_mday']][$id] = $details;
			}
		}
		foreach (array_keys($res) as $month) {
			foreach (array_keys($res[$month]) as $date) process_exceptions($res[$month][$date]);
		}

		return $res;

	}//end getYearEventsByMonthAndDay()


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
		$trees =& get_treeids($root_nodes);
		$tree_sql = build_tree_clause($trees);

		// formulate the date restriction clauses
		$start_ts = strtotime("$year-$month-01 00:00:00 GMT");
		$next_month = ($month % 12) + 1;
		$next_year = ($next_month == 1) ? $year + 1 : $year;
		$end_ts = strtotime("$next_year-$next_month-01 00:00:00 GMT") - 1;

		// Get our single events
		$date_sql = '((cd.start_date_year = '.$db->quote($year).' AND (cd.start_date_mon = '.$db->quote($month).' OR cd.end_date_mon = '.$db->quote($month).')) OR (cd.start_date_ts < '.$db->quote($start_ts).' AND cd.end_date_ts > '.$db->quote($start_ts).'))';

		$sql = build_single_event_query_base($trees)."\n AND (".$date_sql.")\n AND ".$tree_sql;
		$single_result = $db->getAssoc($sql);
		assert_valid_db_result($single_result);

		$cal_date = sprintf('%04d-%02d-%02d', $year, $month, 1);
		$first_day = iso8601_ts($cal_date);
		$last_day = sprintf('%04d-%02d-%02d', $year, $month, days_in_month($month, $year));

		// Get our recurring events
		$date_sql = 'cd.start_date_ts < '.$db->quote($end_ts);
		$sql = build_recurring_event_query_base($trees, 'month', "$year-$month")."\n AND ".$tree_sql;
		$recur_result = $db->getAssoc($sql);
		assert_valid_db_result($recur_result);

		$res = $single_result + $recur_result;
		if ($this->attr('enable_description_popups')) {
			$descriptions = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('description', 'calendar_event', array_keys($res));
			foreach ($descriptions as $assetid => $description) $res[$assetid]['description'] = $description;
		}
		return $res;

	}//end getMonthEvents()


	/**
	* Get the events to show in month view for the specified month, separated by day
	*
	* @return array	$day => array ( assetid => array(...) )
	*/
	function getMonthEventsByDay($year, $month)
	{
		$month_length = days_in_month($month, $year);

		$res = Array();
		for ($i = 1; $i <= $month_length; $i++) {
			$res[$i] = Array();
		}

		$cal_date = sprintf('%04d-%02d-%02d', $year, $month, 1);
		$last_day = sprintf('%04d-%02d-%02d', $year, $month, $month_length);

		$plain_events = $this->getMonthEvents($year, $month);
		$events =& expand_event_list($plain_events, $cal_date, $last_day);

		foreach ($events as $id => $details) {
			if (($details['start_date_mon'] == $month) && ($details['start_date_year'] == $year)) {
				$res[$details['start_date_mday']][$id] = $details;
			}
		}

		foreach ($res as $date => $events) process_exceptions($res[$date]);
		return $res;

	}//end getMonthEventsByDay()


	/*
	* Get the events to show in week view for the week containing the given YYYY-MM-DD date
	*
	* @return array (assetid => Array(...) ))
	*/
	function getWeekEvents($date)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		list($year,$month,$day) = sscanf($date, '%04d-%02d-%02d');

		// get the treeid restriction SQL
		$root_nodes = $this->attr('root_nodes');
		$trees = &get_treeids($root_nodes);
		$tree_sql = build_tree_clause($trees);

		// get the single events
		$start_date_stamp = floor(strtotime($date.' 12:00:00 GMT')/86400);
		$end_date_stamp = $start_date_stamp + 6;
		$date_sql = '(cd.start_date_ds BETWEEN '.$db->quote($start_date_stamp).' and '.$db->quote($end_date_stamp).') OR (cd.start_date_ds < '.$db->quote($start_date_stamp).' AND cd.end_date_ds >= '.$db->quote($start_date_stamp).')';
		$sql = build_single_event_query_base($trees).' AND ('.$date_sql.') AND '.$tree_sql;
		$single_result = $db->getAssoc($sql);
		assert_valid_db_result($single_result);

		// get the recurring events
		$sql = build_recurring_event_query_base($trees, 'week', $date).' AND '.$tree_sql;
		$recur_result = $db->getAssoc($sql);
		assert_valid_db_result($recur_result);
		
		$res = $single_result + $recur_result;

		if ($this->attr('enable_description_popups')) {
			$descriptions = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('description', 'calendar_event', array_keys($res));
			foreach ($descriptions as $assetid => $description) $res[$assetid]['description'] = $description;
		}



		return $res;

	}//end getWeekEvents()


	/*
	* Get the events to show in week view for the week containing the given YYYY-MM-DD date, separated by day
	*
	* @return array ($iso_date => array ( $assetid => Array(...) ))
	*/
	function getWeekEventsByDay($date)
	{
		$res = Array();
		$days_to_show = $this->attr('week_view_show_days');
		for ($i = iso8601_ts($date); $i <= iso8601_ts($date) + 6*86400; $i += 86400) {
			if ($days_to_show[date('w', $i)]) $res[iso8601_date_component(ts_iso8601($i))] = Array();
		}

		$week_start_day = $this->attr('week_starts_on');
		$first_day = iso8601_ts($date) - ((7+(date('w', iso8601_ts($date)) - $week_start_day))%7)*86400;
		$cal_date = date('Y-m-d', $first_day);

		$last_day = iso8601_date_component(ts_iso8601(iso8601_ts($cal_date) + 6*86400));

		$plain_events = $this->getWeekEvents($cal_date);
		$events =& expand_event_list($plain_events, $cal_date, $last_day, $this->attr('day_starts_at'), $this->attr('day_ends_at'));

		foreach ($events as $id => $details) {
			$event_date = sprintf('%04d-%02d-%02d', $details['start_date_year'], $details['start_date_mon'], $details['start_date_mday']);
			if (isset($res[$event_date])) {
				$res[$event_date][$id] = $details;
			}
		}

		foreach ($res as $date => $events) process_exceptions($res[$date]);

		return $res;

	}//end getWeekEventsByDay()


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
		$days_to_show = $this->attr('week_view_show_days');

		if (!$last_time) $last_time = $this->attr('day_ends_at');
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

		$week_start_day = $this->attr('week_starts_on');
		$first_day = iso8601_ts($date) - ((7+(date('w', iso8601_ts($date)) - $week_start_day))%7)*86400;
		$cal_date = date('Y-m-d', $first_day);

		$last_day = iso8601_date_component(ts_iso8601(iso8601_ts($cal_date) + 6*86400));

		for ($i = iso8601_ts($cal_date); $i <= iso8601_ts($cal_date) + 6*86400; $i += 86400) {
			if ($days_to_show[date('w', $i)]) $res[iso8601_date_component(ts_iso8601($i))] = $partitions;
		}


		$plain_events = $this->getWeekEvents($cal_date);
		$events =& expand_event_list($plain_events, $cal_date, $last_day, $this->attr('day_starts_at'), $this->attr('day_ends_at'));

		foreach ($events as $id => $details) {
			$event_date = sprintf('%04d-%02d-%02d', $details['start_date_year'], $details['start_date_mon'], $details['start_date_mday']);

			if (isset($res[$event_date])) {
				if (is_null($details['start_date_hours'])) {
					$res[$event_date]['all_day'][$id] = $details;
				} else {
					$event_ts = $details['start_date_hours'] * 60 + $details['start_date_minutes'];
					if ($event_ts < $start_ts) continue;
					foreach ($partition_keys as $partition_ts => $key) {
						if (($event_ts - $partition_ts) < $time_partition) {
							$res[$event_date][$key][$id] = $details;
							break;
						}
					}
				}
			}
		}

		foreach ($res as $date => $events) {
			foreach($partition_keys as $partition) process_exceptions($res[$date][$partition]);
		}

		return $res;

	}//end getWeekEventsByDayAndTime()


	/**
	* Get the events on the specified day
	*
	* @param string	$date	The date of the day to get events for, in ISO format (YYYY-MM-DD)
	*
	* @return array (assetid => Array(...) ))
	*/
	function getDayEvents($date)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		list($year,$month,$day) = sscanf($date, '%04d-%02d-%02d');

		// build treeid restriction SQL
		$root_nodes = $this->attr('root_nodes');
		$trees =& get_treeids($root_nodes);
		$tree_sql = build_tree_clause($trees);

		// GET SINGLE EVENTS:

		// build date restriction SQL
		$today_date_stamp = floor(strtotime($date.' 12:00:00 GMT')/86400);
		$date_sql = '(('.$today_date_stamp.' = cd.start_date_ds) OR ('.$today_date_stamp.' BETWEEN cd.start_date_ds AND cd.end_date_ds))';

		// combine with the common SQL for single events and get result from DB
		$sql = build_single_event_query_base($trees).' AND '.$date_sql.' AND '.$tree_sql;
		$single_result = $db->getAssoc($sql);
		assert_valid_db_result($single_result);

		// GET RECURRING EVENTS:

		// combine tree sql with the common SQL for recurring events and get result from DB
		$sql = build_recurring_event_query_base($trees, 'day', $date).' AND '.$tree_sql;
		$recur_result = $db->getAssoc($sql);
		assert_valid_db_result($recur_result);

		// PUT IT ALL TOGETHER:
		
		$res = $single_result + $recur_result;

		if ($this->attr('enable_description_popups')) {
			$descriptions = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('description', 'calendar_event', array_keys($res));
			foreach ($descriptions as $assetid => $description) $res[$assetid]['description'] = $description;
		}

		$res =& expand_event_list($res, $date, $date, $this->attr('day_starts_at'), $this->attr('day_ends_at'));
		process_exceptions($res);
		return $res;

	}//end getDayEvents()


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

		$res = $partitions;

		$events = $this->getDayEvents($date);

		foreach ($events as $id => $details) {
			$event_date = sprintf('%04d-%02d-%02d', $details['start_date_year'], $details['start_date_mon'], $details['start_date_mday']);
			if (($event_date == $date)) {
				if (is_null($details['start_date_hours'])) {
					$res['all_day'][$id] = $details;
				} else {
					$event_ts = $details['start_date_hours'] * 60 + $details['start_date_minutes'];
					if ($event_ts < $start_ts) continue;
					foreach ($partition_keys as $partition_ts => $key) {
						if (($event_ts - $partition_ts) < $time_partition) {
							$res[$key][$id] = $details;
							break;
						}
					}
				}
			}
		}
		return $res;

	}//end getDayEventsByTime()


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

		$events = &$this->getDayEvents($date);
		$res = Array();

		foreach ($this->attr('columnise_day_view_by_root_node') as $node_id) $res[$node_id] = $partitions;
		
		foreach ($events as $id => $details) {
			$event_date = sprintf('%04d-%02d-%02d', $details['start_date_year'], $details['start_date_mon'], $details['start_date_mday']);
			$col = (isset($res[$details['root_node']])) ? $details['root_node'] : (isset($res['other']) ? 'other' : false);
			if ($col) {
				if (empty($details['start_date_hours'])) {
					$res[$col]['all_day'][$id] = $details;
				} else {
					$event_ts = $details['start_date_hours'] * 60 + $details['start_date_minutes'];
					if ($event_ts < $start_ts) continue;
					foreach ($partition_keys as $partition_ts => $key) {
						if (($event_ts - $partition_ts) < $time_partition) {
							$res[$col][$key][$id] = $details;
							break;
						}
					}
				}
			}
		}
		return $res;

	}//end getDayEventsByRootNodeAndTime()


}//end class
?>