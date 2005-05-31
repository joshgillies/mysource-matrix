<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: page_calendar.inc,v 1.125.2.13 2005/05/31 02:29:19 tbarrett Exp $
*
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
* @version $Revision: 1.125.2.13 $
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
	* @var Array(keyword => name)
	*/
	var $_available_keywords = Array(
								'calendar_contents' => 'Calendar Contents',
								'calendar_title'    => 'Calendar Title',
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

	}//end constructor


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
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return $linkid;

	}//end create()


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* For example, creating other child assets should be performed in this function
	*
	* @param array	&$link	information used to create the initial link<br/>
	* <PRE>
	* Array ('asset'         => [ref major asset to create link under],
	*        'link_type'     => SQ_LINK_?,
	*        'value'         => [link value],
	*        'sort_order'    => [link sort order],
	*        'is_dependant'  => [0|1],
	*        'is_exclusive'  => [0|1]
	*        )
	* </PRE>
	*
	* @access private
	* @return boolean
	*/
	function _createAdditional(&$link)
	{
		if (!Page::_createAdditional($link)) return false;

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');

		// create the page contents bodycopy
		$page_contents_data = Array('content' => '<h2 style="text-align: center">%calendar_title%</h2><p style="text-align: center">%prev_link%&nbsp;&nbsp;&nbsp;&nbsp;%up_link%&nbsp;&nbsp;&nbsp;&nbsp;%next_link%</p>%calendar_contents%');
		$asset = new Bodycopy();
		$copy_link = Array('asset' => &$this, 'value' => 'page_contents' ,'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);
		$asset->setAttrValue('name', 'Page Contents');
		if (!$asset->create($copy_link, $page_contents_data)) {
			$this->id = 0;
			return false;
		}
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
		unset($asset);

		// create the event formats folder
		$copy_link = Array('asset' => &$this, 'value' => 'type_formats' ,'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);
		$type_formats = new Folder();
		$type_formats->setAttrValue('name', 'Type Formats');
		if (!$type_formats->create($copy_link)) {
			$this->id = 0;
			return false;
		}

		// create the event format bodycopies
		$copy_link = Array('asset' => &$type_formats, 'value' => '' ,'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);
		$event_bodycopy_data = Array('content' => '%asset_contents%');
		$event_types = Array('single', 'recurring');
		foreach ($event_types as $event_type) {
			$copy_link['value'] = 'calendar_event_'.$event_type;
			$asset = new Bodycopy();
			$asset->setAttrValue('name', ucfirst($event_type).' Event Format');
			if (!$asset->create($copy_link, $event_bodycopy_data)) {
				$this->id = 0;
				return false;
			}
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			unset($asset);
		}

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($type_formats);
		unset($type_formats);

		return true;

	}//end _createAdditional()


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
			if (!$this->_validateRequest()) return false;

			// Try to get from cache
			$cache_key = $this->_getCacheKey();
			$cached_contents = '';
			if (!empty($cache_key)) {
				$cm = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
				$cached_contents = $cm->loadFromCache($this, $cache_key);
			}
			if (!empty($cached_contents)) {
				echo $cached_contents;
			} else {
				// have to generate from scratch

				$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'page_contents');
				if (empty($bodycopy_link)) return false;

				$format_bodycopy = &$GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link['minorid'], $bodycopy_link['minor_type_code']);
				if (is_null($format_bodycopy)) return false;

				// Get replacements.  Contents is first because it may change the view if,
				// for example, we have just finished editing an event
				$replacements = Array();
				$replacements['calendar_contents'] = $this->getCalendarContents();
				$replacements['calendar_title'] = $this->getCalendarTitle();
				$replacements += $this->_getNavLinkReplacements();

				ob_start();
					echo '<script>if (typeof cal != "undefined") cal.setDate(\''.$_REQUEST['SQ_CALENDAR_DATE'].'\')</script>';
					// print the contents of page - replacing the global keywords
					$format_bodycopy->setKeywordReplacements($replacements);
					$format_bodycopy->printBody();
					if (!empty($cache_key)) {
						$cm->saveToCache($this, $cache_key, ob_get_contents());
					}
				ob_end_flush();

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($format_bodycopy);

			}
		}

	}//end printBody()


	/**
	* Look at the request values SQ_CALENDAR_DATE and SQ_CALENDAR_VIEW,
	* check they are valid and tweak them if necessary
	*
	* @return boolean	false if a non-recoverable problem was encountered
	* @access private
	*/
	function _validateRequest()
	{
		// Validate View:
		// If we get a valid and available view type in the request, use it;
		// otherwise use the initial view attribute.
		$enabled_views = $this->attr('enabled_views');
		$enabled_views['edit'] = 1;
		$cal_view = array_get_index($_REQUEST, 'SQ_CALENDAR_VIEW', false);
		if (!$cal_view || empty($enabled_views[$cal_view])) {
			$cal_view = $this->attr('initial_view');
		}

		// Validate date:
		// If no date supplied, use today's
		// If view is month or year, some incomplete dates can be accepted
		$cal_date = array_get_index($_REQUEST,'SQ_CALENDAR_DATE', date('Y-m-d'));
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
		// if we've somehow ended up with an invalid date, use today's
		list($year, $month, $day) = explode('-', $cal_date);
		if (!checkdate($month, $day, $year)) {
			trigger_error('Date passed to calendar page is not valid, using today\'s date as fallback', E_USER_WARNING);
			$cal_date = date('Y-m-d');
		}

		// If the view is 'event' or 'edit' we need the eventid too
		$event_id = false;
		if (($cal_view == 'event') || ($cal_view == 'edit')) {
			if (!isset($_REQUEST['SQ_CALENDAR_EVENT_ID'])) {
				trigger_error(ucfirst($_cal_view).' View requested but no Event ID supplied', E_USER_WARNING);
				return false;
			}
			if (!assert_valid_assetid($_REQUEST['SQ_CALENDAR_EVENT_ID'])) {
				return false;
			}
		}

		// Store validated values in globals for future reference
		$_REQUEST['SQ_CALENDAR_VIEW'] = $cal_view;
		$_REQUEST['SQ_CALENDAR_DATE'] = $cal_date;
		return true;

	}//end _validateRequest()


	/**
	* Work out the unique cache key to represent the current page
	*
	* @return string
	* @access private
	*/
	function _getCacheKey()
	{
		// Cache key will be a letter representing the view type (Y, M, W, D)
		// followed by as precise an iso-date as necessary (2004, 2004-02, 2004-04-01)
		list($year, $month, $day) = explode('-', $_REQUEST['SQ_CALENDAR_DATE']);
		switch ($_REQUEST['SQ_CALENDAR_VIEW']) {
			case 'year':
				return 'Y'.$year;
			case 'month':
				return sprintf('M%04d-%02d', $year, $month);
			case 'week':
				return sprintf('W%04d-%02d-%02d', $year, $month, $day);
			case 'day':
				return sprintf('D%04d-%02d-%02d', $year, $month, $day);
		}
		return '';

	}//end _getCacheKey()


	/**
	* Get the replacements for the up, next and previous links
	*
	* @return array		keyword => replacement value
	* @access private
	*/
	function _getNavLinkReplacements()
	{
		$replacements = Array();
		$cal_view = $_REQUEST['SQ_CALENDAR_VIEW'];
		$cal_date = $_REQUEST['SQ_CALENDAR_DATE'];
		$event_id = array_get_index($_REQUEST, 'SQ_CALENDAR_EVENT_ID', false);
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

		return $replacements;

	}//end _getNavLinkReplacements()


	/**
	* Return the available keywords in the Page Contents Bodycopy for this asset
	*
	* The return value is in the form:
	* <pre>
	* Array(
	*     'keyword' => 'name',
	* )
	* </pre>
	*
	* @access public
	* @return array(string => string)
	*/
	function getContentsKeywords()
	{
		$keywords = Array(
							'calendar_contents' => 'Calendar Contents',
							'calendar_title'    => 'Calendar Title',
							'up_link'           => 'Up Link',
							'prev_link'         => 'Previous Link',
							'next_link'         => 'Next Link',

						);

		return $keywords;

	}//end getContentsKeywords()


//--        FUNCTIONS TO GET CALENDAR CONTENTS        --//


	/**
	* Get the replacenent for the %calendar_title% keyword
	*
	* @return string	The title to be displayed
	* @access public
	*/
	function getCalendarTitle()
	{
		$title_formats = $this->attr('view_title_formats');
		$title_formats['edit'] = 'Edit Event';
		$view = $_REQUEST['SQ_CALENDAR_VIEW'];
		if (($view == 'event') || ($view == 'edit')) {
			// _validateRequest already made sure we have a valid event id
			$info = &$GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($_REQUEST['SQ_CALENDAR_EVENT_ID']));
			return $info[$_REQUEST['SQ_CALENDAR_EVENT_ID']]['name'];
		} else {
			return date($title_formats[$view], strtotime($_REQUEST['SQ_CALENDAR_DATE']));
		}

	}//end getCalendarTitle()


	/**
	* Get the replacement for the %calendar_contents% keyword
	*
	* @return string	The contents to be displayed
	* @access public
	*/
	function getCalendarContents()
	{
		$cal_view = $_REQUEST['SQ_CALENDAR_VIEW'];
		$formats = $this->attr('enabled_views');
		$format = array_get_index($formats, $cal_view, '');
		$res = $this->_getGeneralScript();
		if (strpos($format, 'list') === false) $res .= '<div id="calendarTable">';
		$func = 'get'.ucfirst($cal_view).'View';
		$res .= (($cal_view == 'event') || ($cal_view == 'edit')) ? $this->$func($_REQUEST['SQ_CALENDAR_EVENT_ID']) : $this->$func($_REQUEST['SQ_CALENDAR_DATE'], $format);
		if (strpos($format, 'list') === false) $res .= '</div>';
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

		if ($format == 'list_without_headings') {
			$list_entry_types = $this->attr('list_entry_types');
			$events = $this->getYearEvents($year);
			uasort($events, 'compare_start_dates');

			if ($list_entry_types['year'] == 'title') {
				echo $this->_getTitleList($events, $enabled_views['event']);
			} else {
				foreach ($events as $id => $details) $events[$id] = $details['type_code'];
				$this->filterAssetList($events);
				$this->printAssetList($events);
			}
		} else if ($format == 'list_with_headings') {
			$list_entry_types = $this->attr('list_entry_types');
			$events = $this->getYearEventsByMonth($year);
			foreach ($this->month_names_full as $month_num => $month_name) {
				uasort($events[$month_num], 'compare_start_dates');
				echo '<h3>';
				if ($enabled_views['month']) echo '<a href="'.$this->getHref().'?SQ_CALENDAR_VIEW=month&amp;SQ_CALENDAR_DATE='.$year.'-'.sprintf('%02d', $month_num).'-01">';
				echo $month_name;
				if ($enabled_views['month']) echo '</a>';
				echo '</h3>';

				if ($list_entry_types['year'] == 'title') {
					echo $this->_getTitleList($events[$month_num], $enabled_views['event']);
				} else {
					$new_list = Array();
					foreach ($events[$month_num] as $id => $details) {
						$new_id = strtok($id, ':');
						$new_list[$new_id] = $details['type_code'];
					}
					$this->filterAssetList($new_list);
					$this->printAssetList($new_list);
				}
			}
		} else {
			$events = $this->getYearEventsByMonthAndDay($year);
			$week_start_day = $this->attr('week_starts_on');
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
					$onclick = 'onclick="clickChild(this)" style="cursor: pointer; cursor: hand"';
					echo '<td '.$onclick.' class="'.$class.'">';
					echo '<a  href="'.$this->getHref().'?SQ_CALENDAR_VIEW=day&amp;SQ_CALENDAR_DATE='.sprintf('%04d-%02d-%02d', $year, $current_month, $current_day).'">'.$current_day.'</a>';
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
		$drill_down_view = empty($enabled_views['day']) ? '' : 'day';
		ob_start();
		if ($format == 'list_without_headings') {
			$list_entry_types = $this->attr('list_entry_types');
			$events = $this->getMonthEvents($year, $month);
			uasort($events, 'compare_start_dates');
			if ($list_entry_types['month'] == 'title') {
				echo $this->_getTitleList($events, $enabled_views['event']);
			} else {
				foreach ($events as $id => $details) $events[$id] = $details['type_code'];
				$this->filterAssetList($events);
				$this->printAssetList($events);
			}
		} else if ($format == 'list_with_headings') {
			$list_entry_types = $this->attr('list_entry_types');
			$events = $this->getMonthEventsByDay($year, $month);
			$month_length = days_in_month($month, $year);
			for ($current_day = 1; $current_day <= $month_length; $current_day++) {
				echo '<h3>';
				if ($drill_down_view) echo '<a href="'.$this->getHref().'?SQ_CALENDAR_VIEW='.$drill_down_view.'&amp;SQ_CALENDAR_DATE='.$year.'-'.$month.'-'.sprintf('%02d', $current_day).'">';
				echo date('l jS', strtotime("$year-$month-$current_day"));
				if ($drill_down_view) echo '</a>';
				echo '</h3>';
				uasort($events[$current_day], 'compare_start_dates');
				if ($list_entry_types['month'] == 'title') {
					echo $this->_getTitleList($events[$current_day], $enabled_views['event']);
				} else {
					$new_list = Array();
					foreach ($events[$current_day] as $id => $details) {
						$new_id = strtok($id, ':');
						$new_list[$new_id] = $details['type_code'];
					}
					$this->filterAssetList($new_list);
					$this->printAssetList($new_list);
				}
			}
		} else {
			$show_event_links = (bool)$enabled_views['event'];
			$events = $this->getMonthEventsByDay($year, $month);
			$column_width = $this->attr('month_column_width');
			$row_height = $this->attr('month_row_height');
			$week_start_day = $this->attr('week_starts_on');
			$root_nodes = $this->_getRootNodes();
			$root_node_keys = array_keys($root_nodes);
			$week_length = 7;
			$month_length = days_in_month($month, $year);
			$current_day = 1;
			$current_date = $_REQUEST['SQ_CALENDAR_DATE'];
			if ($this->attr('enable_description_popups')) $this->_printPopupsScript();
			?>
			<table id="month" border="0" cellspacing="0" cellpadding="0" style="width: <?php echo (7.35 * $column_width); ?>px">
				<tr>
			<?php
			// print the day headings
			if ($this->attr('month_links_to_week')) {
				echo '<th class="dayLabel">&nbsp;</th>';
			}
			for ($i=0; $i < $week_length; $i++) {
				$day_name = $this->day_names[($week_start_day + $i) % 7];
					?>
					<th style="width: <?php echo $column_width; ?>px" class="dayLabel<?php echo (($day_name == 'Saturday') || ($day_name == 'Sunday')) ? 'Weekend' : ''; ?>">
						<?php echo $day_name; ?>
					</th>
					<?php
			}
			?>
				</tr>
				<tr>
			<?php
			if ($this->attr('month_links_to_week')) {
				?>
				<th class="weekLink" onclick="location.href = this.childNodes[0].href;"><a class="dateLink" href="<?php echo $this->getHref().'?SQ_CALENDAR_VIEW=week&amp;SQ_CALENDAR_DATE='.add_days_to_iso($current_date) ?>">&raquo;</a></th>
				<?php
			}

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
				$current_date = sprintf('%04d-%02d-%02d', $year, $month, $current_day);
				$contents = $this->_getMonthCellContents($events[$current_day], $show_event_links, $current_date, $root_nodes, $column_width*0.99);
				$weekend_class = in_array(date('D', strtotime("$year-$month-$current_day")), Array('Sat', 'Sun')) ? 'weekend' : '';
				$occupied_class = ($contents) ? 'eventDate' : '';
				echo '<td class="date '.$occupied_class.' '.$weekend_class.'" style="height: '.$row_height.'px">';
				echo '<a class="dateLink"  href="'.$this->getHref().'?SQ_CALENDAR_VIEW=day&amp;SQ_CALENDAR_DATE='.$current_date.'">'.$current_day.'</a><br />';
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
				if ($this->attr('month_links_to_week')) {
					?>
					<th class="weekLink" onclick="location.href = this.childNodes[0].href;"><a class="dateLink" href="<?php echo $this->getHref().'?SQ_CALENDAR_VIEW=week&amp;SQ_CALENDAR_DATE='.add_days_to_iso($current_date) ?>">&raquo;</a></th>
					<?php
				}
				for ($i=0; $i < $week_length; $i++) {
					$current_date = sprintf('%04d-%02d-%02d', $year, $month, $current_day);
					$contents = $this->_getMonthCellContents($events[$current_day], $show_event_links, $current_date, $root_nodes, $column_width*0.99);
					$weekend_class = in_array(date('D', strtotime("$year-$month-$current_day")), Array('Sat', 'Sun')) ? 'weekend' : '';
					$occupied_class = ($contents) ? 'eventDate' : '';
					echo '<td class="date '.$occupied_class.' '.$weekend_class.'" style="height: '.$row_height.'px">';
					echo '<a class="dateLink"  href="'.$this->getHref().'?SQ_CALENDAR_VIEW=day&amp;SQ_CALENDAR_DATE='.$current_date.'">'.$current_day.'</a><br />';
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
				if ($this->attr('month_links_to_week')) { 
					?>
					<th class="weekLink" onclick="location.href = this.childNodes[0].href;"><a  class="dateLink"  href="<?php echo $this->getHref().'?SQ_CALENDAR_VIEW=week&amp;SQ_CALENDAR_DATE='.$current_date ?>">&raquo;</a></th>
					<?php
				}
				while ($current_day <= $month_length) {
					$current_date = sprintf('%04d-%02d-%02d', $year, $month, $current_day);
					$contents = $this->_getMonthCellContents($events[$current_day], $show_event_links, $current_date, $root_nodes, $column_width*0.99);
					$weekend_class = in_array(date('D', strtotime("$year-$month-$current_day")), Array('Sat', 'Sun')) ? 'weekend' : '';
					$occupied_class = ($contents) ? 'eventDate' : '';
					echo '<td class="date '.$occupied_class.' '.$weekend_class.'" style="height: '.$row_height.'px">';
					echo '<a class="dateLink"  href="'.$this->getHref().'?SQ_CALENDAR_VIEW=day&amp;SQ_CALENDAR_DATE='.$current_date.'">'.$current_day.'</a><br />';
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
		$root_nodes = $this->_getRootNodes();
		$enabled_views = $this->attr('enabled_views');
		if ($format == 'list_without_headings') {
			$list_entry_types = $this->attr('list_entry_types');
			$events = $this->getWeekEvents($date);
			uasort($events, 'compare_start_dates');
			if ($list_entry_types['week'] == 'title') {
				echo $this->_getTitleList($events, $enabled_views['event']);
			} else {
				foreach ($events as $id => $details) $events[$id] = $details['type_code'];
				$this->filterAssetList($events);
				$this->printAssetList($events);
			}
		} else if ($format == 'list_with_headings') {
			$list_entry_types = $this->attr('list_entry_types');
			$events = $this->getWeekEventsByDay($date);
			foreach ($events as $day_date => $day_events) {
				echo '<h3>'.(($enabled_views['day']) ? '<a href="'.$this->getHref().'?SQ_CALENDAR_VIEW=day&amp;SQ_CALENDAR_DATE='.$day_date.'"">' : '');
				echo date('l jS', strtotime($day_date));
				echo (($enabled_views['day']) ? '</a>' : '').'</h3>';
				if ($list_entry_types['week'] == 'title') {
					echo $this->_getTitleList($events[$day_date], $enabled_views['event']);
				} else {
					$real_list = Array();
					foreach ($day_events as $id => $details) {
						$new_id = strtok($id, ':');
						$real_list[$new_id] = $details['type_code'];
					}
					$this->filterAssetList($real_list);
					$this->printAssetList($real_list);
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
			uasort($events, 'compare_start_dates');
			if ($list_entry_types['day'] == 'title') {
				echo $this->_getTitleList($events, $enabled_views['event']);
			} else {
				$new_events = Array();
				foreach ($events as $id => $details) {
					$new_events[strtok($id, ':')] = $details['type_code'];
				}
				$this->filterAssetList($new_events);
				$this->printAssetList($new_events);
			}
		} else {
			$column_names = Array();
			$column_links = Array();
			if (array_values($this->attr('columnise_day_view_by_root_node')) != Array('*')) {
				$root_nodes = array_keys($this->_getRootNodes());
				$columns_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($this->attr('columnise_day_view_by_root_node'));
				foreach (array_keys($columns_info) as $k) {
					if (!in_array($k, $root_nodes)) {
						unset($columns_info[$k]);
					}
				}

				foreach ($columns_info as $id => $info) $column_names[$id] = $info['name'];
				if (in_array('*', $this->attr('columnise_day_view_by_root_node'))) {
					$column_names['*'] = ($this->attr('other_column_name') == '') ? 'Other' : $this->attr('other_column_name');
				}
				$events = $this->getDayEventsByRootNodeAndTime($date);
			} else {
				$column_names[1] = ($this->attr('other_column_name') == '') ? 'Events' : $this->attr('other_column_name');
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
			$asset_list = Array($event_id => $info[$event_id]['type_code']);
			$this->filterAssetList($asset_list);
			$this->printAssetList($asset_list);
			$res = ob_get_contents();
		ob_end_clean();
		return $res;

	}//end getEventView()


	/**
	* Get the edit form for the specified event, or process form and redirect to event view
	*
	* If the form has been submitted it is processed, and if the processing was successful
	* event view is returned
	*
	* @return void
	* @access public
	*/
	function getEditView($event_id)
	{
		$event =& $GLOBALS['SQ_SYSTEM']->am->getAsset($event_id);
		require_once SQ_INCLUDE_PATH.'/limbo_outputter.inc';
		$o = new Limbo_Outputter();
		if (!$event->id) {
			trigger_error('Could not load calendar event asset #'.$eventid);
			return false;
		}

		// Process the form if appropriate
		if (isset($_POST['asset_action'])) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			if ($event->processBackend($o, $link)) {
				// The form has been committed and there are no probs, so go to event view
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$GLOBALS['SQ_SYSTEM']->am->releaseLock($event_id, 'attributes');
				$_REQUEST['SQ_CALENDAR_VIEW'] = 'event';
				$_REQUEST['SQ_CALENDAR_EVENT_ID'] = $event_id;
				return $this->getEventView($event_id);
			} else {
				// There was a problem, so roll back transaction and continue printing the form
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			}
		}

		// Acquire the lock and print the interface
		if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($event_id, 'attributes')) {
			trigger_error("Couldn't acquire lock", E_USER_WARNING);
			return $this->getEventView($event_id);
		}
		$o->addHiddenField('SQ_CALENDAR_VIEW', 'edit');
		$o->addHiddenField('SQ_CALENDAR_EVENT_ID', $event_id);
		$o->addHiddenField('asset_action', 'limbo');
		$o->addJsInclude(sq_web_path('lib').'/html_form/html_form.js');
		$o->openRaw();
		$ei = &$event->getEI();
		$ei->printEditInterface($ei->findFile('screen_details'), $event, $o);
		$o->closeRaw();
		$o->commitButton('Update Event');
		ob_start();
			$o->_paintHeader();
			$o->paint();
			$res = ob_get_contents();
		ob_end_clean();
		return $res;

	}//end getEditView()


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
		$res = '<ul>';
		foreach ($events as $id => $data) {
			$real_id = strtok($id, ':');
			$res .= '<li>';
			if ($show_links) $res .= '<a href="'.$this->getHref().'?SQ_CALENDAR_VIEW=event&amp;SQ_CALENDAR_DATE='.sprintf('%04d-%02d-%02d', $data['start_date_year'], $data['start_date_mon'], $data['start_date_mday']).'&amp;SQ_CALENDAR_EVENT_ID='.$real_id.'">';
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
		$columns_are_assets = ($table_id == 'day') && (array_keys($column_names) != Array('*'));

		$root_nodes = $this->_getRootNodes();
		$width = ($table_id == 'day') ? $this->attr('day_column_width') : $this->attr('table_column_width');

		$layout_maps =  Array();
		foreach ($events as $date => $times_events) {
			$layout_maps[$date] = Array();
			foreach (array_keys($times_events) as $time) {
				$layout_maps[$date][$time] = Array();
			}
		}

		$dragging_enabled = $this->attr('enable_event_dragging');
		$popups_enabled = $this->attr('enable_description_popups');
		$click_to_add_ab = $this->attr('asset_builder_page');

		$mousedown_code = $dragging_enabled ? 'onmousedown="'.($popups_enabled ? 'stopTrailingPopup(); ' : '').'startDragging(this)"' : '';
		$mouseup_code = $dragging_enabled ? 'onmouseup="stopDragging(); formSubmitted=true; setTimeout(\'formSubmitted=false\', 100)"' : '';
		?>
		<style>
			table#day tr td.sq-time-cell, table#week tr td.sq-time-cell {
				width: <?php echo $width; ?>px;
				height: <?php echo $partition_height; ?>px;
				padding: 0px 1px;
				border-width: 1px;
			<?php
			if ($click_to_add_ab) {
				?>
				cursor: pointer;
				cursor: hand;
				<?php
			}
			?>
			}
			/* CSS Hack to cope with the unusual situation of strict-mode IE being better
			   than strict-mode Firefox.  The following rule will be picked up only by firefox
			   and compensates for its td-height bug. */
			html>body table#day tr td.sq-time-cell, html>body table#week tr td.sq-time-cell {
				height: <?php echo $partition_height+1; ?>px;
			}
			td.sq-allday-cell {
				<?php
				if ($click_to_add_ab) {
					?>
					cursor: pointer;
					cursor: hand;
					<?php
				}
				?>
			}
		</style>
		<table id="<?php echo $table_id; ?>" border="0" cellspacing="0" cellpadding="0" summary="events in this <?php echo $table_id ?>" style="width: <?php echo ($width * count($events)) + 65; ?>px">
		<?php

		// PRINT HEADER ROW
		?>
		<tr>
			<th>Time</th>
		<?php
		foreach ($column_names as $id => $name) {
			?>
			<th onclick="<?php echo (!empty($column_links)) ? 'clickChild(this); ' : ''; ?>" class="columnLabel" nowrap="nowrap" style="width: <?php echo $width; ?>px;  <?php echo (!empty($column_links)) ? 'cursor: pointer; cursor: hand' : ''; ?>">
				<?php
					if (!empty($column_links)) echo '<a href="'.$column_links[$id].'">';
					echo str_replace(' ', '&nbsp;', $name);
					if (!empty($column_links)) echo '</a>';
				?>
			</th>
			<?php
		}
		?>
		</tr>
		<?php
		$div_width = $width * 0.99;
		list($current_hour, $current_mins) = explode(':', $this->attr('day_starts_at'));
		list($end_hour, $end_mins) = explode(':', $this->attr('day_ends_at'));

		// PRINT THE 'ALL DAY' ROW
		?>
			<tr>
			<th class="timeLabel">*</th>
			<?php
			foreach ($column_names as $id => $name) {
				$col_events = $events[$id];
				$event_link_date = is_iso8601($id) ? $id : $_REQUEST['SQ_CALENDAR_DATE'];
				$click_to_add_code = '';
				if ($click_to_add_ab) {
					$root_node_code = ($columns_are_assets && ($id != '*')) ? ', '.$id : '';
					$click_to_add_code = ' onclick="addEvent(\''.$event_link_date.'\', \'*\''.$root_node_code.');"';
				}
				echo '<td class="sq-allday-cell" id="'.$event_link_date.'_allday_'.($columns_are_assets ? $id : '').'"'.$click_to_add_code.'>';
				if (empty($col_events['all_day'])) echo '&nbsp;';
				foreach ($col_events['all_day'] as $assetid => $event) {
					$assetid = strtok($assetid,':');
					$event_href = $this->getHref().'?SQ_CALENDAR_VIEW=event&amp;SQ_CALENDAR_EVENT_ID='.$assetid.'&amp;SQ_CALENDAR_DATE='.$event_link_date;
					$event_class = $root_nodes[$event['root_node']]['class_name'];
					$onclick_code = $show_event_links ? ' onclick="sq_cal_redirect(\''.$event_href.'\')"' : '';
					$cursor_code = $show_event_links ? ' cursor: pointer; cursor: hand' : '';
					$current_mouse_up_down_code = $mousedown_code.' '.$mouseup_code;
					if (!empty($mousedown_code) && (($event['type_code'] == 'calendar_event_recurring') || (isset($event['expanded'])))) {
						$current_mouse_up_down_code = 'onmousedown="statusBarMsg(\'Recurring, multi-day and trimmed events cannot be dragged\')" onmouseup="statusBarMsg(\'\');"';
					}
					$current_mouse_over_out_code = '';
					if (isset($event['description']) && !empty($event['description'])) {
						echo '<div id="event_'.$assetid.'_description" class="popup">'.$event['description'].'</div>';
						$current_mouse_over_out_code = 'onmouseover="startTrailingPopup(\'event_'.$assetid.'_description\')" onmouseout="stopTrailingPopup()"';
					}
					echo '<div id="'.$assetid.'" '.$current_mouse_up_down_code.' '.$current_mouse_over_out_code.' style="margin: 0px 0px 1px 0px; text-align: center; padding: 0px; width: '.($width-2).'px;'.$cursor_code.'" class="'.$event_class.' event"'.$onclick_code.'>';

					// if drag-n-drop is enabled, click-through-to-event-view is supported
					// via javascript only, because otherwise the link elements cause trouble
					// with the dragging
					$label = $event['name'];
					if ($show_event_links && empty($mousedown_code)) {
						$label = '<a class="event '.$event_class.'" href="'.$event_href.'">'.$label.'</a>';
					} else {
						$label = '<span class="event '.$event_class.'">'.$label.'</span>';
					}
					echo $label;

					echo '</div>';
				}
				echo '</td>';
			}
			?>
			</tr>
		<?php

		// PRINT THE ROWS WITH TIMES
		while (($current_hour*60 + $current_mins) < ($end_hour*60 + $end_mins)) {
			?>
			<tr>
				<th class="timeLabel"><?php echo sprintf('%d:%02d', (($current_hour % 12 == 0) ? 12 : ($current_hour % 12)), $current_mins); ?></th>
			<?php
			foreach ($column_names as $id => $name) {
				$col_events = $events[$id];
				$event_link_date  = is_iso8601($id) ? $id : $_REQUEST['SQ_CALENDAR_DATE'] ;
				$time_key = sprintf('%02d:%02d', $current_hour, $current_mins);
				if ($click_to_add_ab) {
					$root_node_code = ($columns_are_assets && ($id != '*')) ? ', '.$id : '';
					$click_to_add_code = ' onclick="addEvent(\''.$event_link_date.'\', \''.$time_key.'\''.$root_node_code.');"';
				}
				?>
				<td id="<?php echo $event_link_date.'_'.$time_key.'_'.($columns_are_assets ? $id : ''); ?>" class="sq-time-cell" <?php echo $click_to_add_code; ?>>
					<?php
						$denominator = max($col_events['_overlap_'], $this->attr('column_capacity'));
						echo $this->_getTimedTableCellContents($col_events[$time_key], $event_link_date, $show_event_links, $root_nodes, ($current_mins + ($current_hour * 60)), $partition_time, $partition_height, $div_width / $denominator, $layout_maps[$id], $mousedown_code, $mouseup_code);
					?>
					<span style="font-size: 1px">&nbsp;</span>
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

		// print all the javascript last so it doesn't screw up spacing in mozilla
		if ($dragging_enabled) $this->_printEventDraggingScript($table_id, ($table_id == 'day') ? $column_names : null);
		if ($popups_enabled) $this->_printPopupsScript();
		if ($click_to_add_ab) $this->_printAddEventScript($click_to_add_ab);


	}//end _printTimedTable()


	/**
	* Get the contents of a cell in the week or day table
	*
	* @param array		$events				The events for the cell, $assetid => details
	* @param boolean	$show_event_links	Whether to link the event's name to its event view
	* @param array		$root_nodes			The root nodes for this page, used for getting classnames
	* @param int		$cell_ts			The timestamp (seconds today) of the earliest time in the cell
	* @param int		$partition_time		The length of the partition in minutes
	* @param float		$cell_height	The height of the cell, in px-es
	* @param float		$event_width		The width of the event, in px-es
	* @param string		$mousedown_code		Javascript code to put in the onmousedown attribute of the event DIVs
	* @param string		$mouseup_code		Javascript code to put in the onmouseup attribute of the event DIVs
	*
	* @return string	The HTML for the contents of the table cell
	* @access private
	*/
	function _getTimedTableCellContents(&$events, $event_link_date, $show_event_links, &$root_nodes, $cell_ts, $partition_time, $cell_height, $event_width, &$layout_map, $mousedown_code, $mouseup_code)
	{
		if (empty($events)) return '';
		$time_label = sprintf('%02d:%02d', (int)($cell_ts / 60), $cell_ts % 60);

		// get events in order
		$cmp_func = create_function('$a, $b', 'return ($b[\'end_date_ts\'] - $b[\'start_date_ts\']) - ($a[\'end_date_ts\'] - $a[\'start_date_ts\']);');
		uasort($events, $cmp_func);
		$res = '';

		foreach ($events as $assetid => $details) {

			$real_assetid = current(explode(':', $assetid));
			$event_href = $this->getHref().'?SQ_CALENDAR_VIEW=event&amp;SQ_CALENDAR_DATE='.$event_link_date.'&amp;SQ_CALENDAR_EVENT_ID='.$real_assetid;

			// get horizontal position of div and update the layout map
			$k = reset($layout_map);
			while ((key($layout_map) != $time_label) && (false !== $k)) {
				next($layout_map);
			}
			$horizontal_index = 0;
			while (isset($layout_map[$time_label][$horizontal_index])) $horizontal_index++;
			for ($i=0; $i < ceil(($details['end_date_minutes'] + $details['end_date_hours']*60 - $details['start_date_minutes']-$details['start_date_hours']*60) / $partition_time); $i++) {
				$layout_map[key($layout_map)][$horizontal_index] = 1;
				next($layout_map);
			}
			$event_left_margin = ($event_width * $horizontal_index);

			// get top position of div
			$event_ts = $details['start_date_minutes'] + (60 * $details['start_date_hours']);
			$end_ts = $details['end_date_minutes'] + (60 * $details['end_date_hours']);
			$event_top_margin = (($event_ts - $cell_ts) / $partition_time) * $cell_height;

			// get height of div
			$event_duration = is_null($details['end_date_ts']) ? 0 : ($end_ts - $event_ts);
			$event_height = ($event_duration / $partition_time) * $cell_height;
			if ($event_height < 0) continue;
			$event_height += floor($event_duration / $partition_time);
			$event_height -= 2;

			// get class to use for div
			$root_node_treeids = get_treeids($root_nodes);
			$event_class = '';
			uasort($root_node_treeids, create_function('$a,$b', 'return strlen($b)-strlen($a);'));
			foreach ($root_node_treeids as $root_node_assetid => $root_node_treeid) {
				if (substr($details['treeid'], 0, strlen($root_node_treeid)) === $root_node_treeid) {
					// if this root node has a formatting style, use it and shove off
					if ($root_nodes[$root_node_assetid]['class_name'] != '') {
						$event_class = $root_nodes[$root_node_assetid]['class_name'];
						break;
					}
				}
			}

			// mouse action handlers
			$onclick_code = $show_event_links ? 'onclick="sq_cal_redirect(\''.$event_href.'\')"' : '';
			$current_mouse_up_down_code = $mousedown_code.' '.$mouseup_code;
			if (!empty($mousedown_code) && (($details['type_code'] == 'calendar_event_recurring') || (isset($details['expanded'])))) {
				$current_mouse_up_down_code = 'onmousedown="statusBarMsg(\'Recurring, multi-day and trimmed events cannot be dragged\')" onmouseup="statusBarMsg(\'\');"';
			}
			$current_mouse_over_out_code = '';
			if (isset($details['description']) && !empty($details['description'])) {
				$res .= '<div id="event_'.$assetid.'_description" class="popup">'.$details['description'].'</div>';
				$current_mouse_over_out_code = 'onmouseover="startTrailingPopup(\'event_'.$assetid.'_description\')"  onmouseout="stopTrailingPopup()"';
			}

			// other special CSS for cursors and zero-length events
			$zero_length_code = $event_height ? '' : 'border-style: dashed; border-bottom: 0px; border-left: 0px; border-right: 0px; padding-top: 0px; background-color: transparent;';
			$cursor_code = $show_event_links ? 'cursor: pointer; cursor: hand;' : '';

			// showtime!
			$res .= '<div id="'.$assetid.'" '.$current_mouse_up_down_code.' '.$current_mouse_over_out_code.' class="'.$event_class.' event" style="float: left; position: absolute; width: '.($event_width-3).'px; margin-left: '.$event_left_margin.'px; margin-top: '.$event_top_margin.'px; height: '.$event_height.'px; border-width: 1px; '.$cursor_code.' '.$zero_length_code.'" '.$onclick_code.'>';
			$res .= '<div class="eventText" style="z-index: 30; position: absolute; width: '.($event_width-4).'px; border: none;  background-color: transparent">';

			// if drag-n-drop is enabled, click-through-to-event-view is supported
			// via javascript only, because otherwise the link elements cause trouble
			// with the dragging
			$label = $details['name'];
			if ($show_event_links && empty($mousedown_code)) {
				$label = '<a class="event '.$event_class.'" href="'.$event_href.'">'.$label.'</a>';
			} else {
				$label = '<span class="event '.$event_class.'">'.$label.'</span>';
			}
			$res .= $label;

			$res .= '</div></div>';

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
	function _getMonthCellContents(&$events, $show_event_link, $event_link_date, &$root_nodes, $width)
	{
		$res = '';
		foreach ($events as $assetid => $details) {
			$assetid = strtok($assetid, ':');
			$event_class = (isset($root_nodes[$details['root_node']]['class_name'])) ? $root_nodes[$details['root_node']] ['class_name'] : '';
			$onclick_code =$show_event_link ? ' onclick="clickChild(this); "' : '';
			$cursor_code = $show_event_link ? 'cursor: pointer; cursor: hand;' : '';

			$current_mouse_over_out_code = '';
			if (isset($details['description']) && !empty($details['description'])) {
				$res .= '<div id="event_'.$assetid.'_description" class="popup">'.$details['description'].'</div>';
				$current_mouse_over_out_code = 'onmouseover="startTrailingPopup(\'event_'.$assetid.'_description\')" onmouseout="stopTrailingPopup()"';
			}

			$res .= '<div style="width: '.$width.'px; '.$cursor_code.'"'.$onclick_code.' '.$current_mouse_over_out_code.'><div style="width: auto; margin-bottom: 1px;" class="event '.$event_class.'">';
			if ($show_event_link) $res .= '<a href="'.$this->getHref().'?SQ_CALENDAR_VIEW=event&amp;SQ_CALENDAR_DATE='.$event_link_date.'&amp;SQ_CALENDAR_EVENT_ID='.$assetid.'" class="event '. $event_class.'">';
			$res .= $details['name'];
			if ($show_event_link) $res .= '</a>';
			$res .= '</div></div>';
		}
		return $res;

	}//end _getMonthCellContents()


	/**
	* Print general javascript used by this page
	*
	* @access public
	* @return void
	*/
	function _getGeneralScript()
	{
		return '

		<!-- GENERAL CALENDAR PAGE JAVASCRIPT, ALWAYS PRESENT -->
			<script language="JavaScript" type="text/javascript">
			<!--
			var formSubmitted = false;

			function clickChild(elt) {
				 if (formSubmitted) {
					 return;
				 }
				 formSubmitted = true;
				 linkChildren = elt.getElementsByTagName("A");
				 if (linkChildren.length > 0) {
					 document.location = linkChildren.item(0).href;
				 }
			}
			function sq_cal_redirect(url) {
				if ((typeof moved == "undefined") || !moved) {
					document.location.href = url;
					formSubmitted = true;
				} else {
					log("moved, not redirecting to "+url);
				}
			}
			function statusBarMsg(m) {
				window.status = m;
				setTimeout(\'window.status=""\', 2000);
			}
			//-->
			</script>
		<!-- /GENERAL CALENDAR PAGE JAVASCRIPT, ALWAYS PRESENT -->

		';

	}//end _getGeneralScript()


	/**
	* Print the javascript needed to support click-to-add-event behaviour
	*
	* @param $ab_id		The assetid of the asset builder page to redirect to
	*
	* @access private
	* @return void
	*/
	function _printAddEventScript($ab_id)
	{
		?>

		<!-- SCRIPT FOR CLICK-TO-ADD-EVENT -->
			<script type="text/javascript">
			<!--
			  function addEvent(date, time, node) {
				  if (formSubmitted) return;
				  loc = "<?php echo $GLOBALS['SQ_SYSTEM']->am->getAssetHref($ab_id); ?>?SQ_CALENDAR_DATE="+date;
				  if (time != '*') loc += '&SQ_CALENDAR_TIME='+time+'&SQ_CALENDAR_DURATION=<?php echo (int)$this->attr('day_partition_time');?>_min';
				  if (typeof node != 'undefined') loc += '&SQ_CALENDAR_LOCATION='+node;
				  document.location = loc;
			  }
			//-->
			</script>
		<!-- /SCRIPT FOR CLICK-TO-ADD-EVENT -->

		<?php

	}//end _printAddEventScript()


	/**
	* Print the javascript needed to support drag and drop editing of events
	*
	* @param $table_id	The ID of the HTML table within which events are being dragged and dropped
	*
	* @access private
	* @return void
	*/
	function _printEventDraggingScript($table_id, $columns=null)
	{
		?>

		<!-- DRAG AND DROP SUPPORT -->
			<script type="text/javascript" src="<?php echo sq_web_path('data'); ?>/asset_types/page_calendar/js/drag_n_drop.js"></script>
			<script type="text/javascript">
			<!--

					destinationTableId = '<?php echo $table_id; ?>';
					columnNames = new Array(<?php echo count($columns); ?>);
					<?php
					if (!is_null($columns)) {
						foreach ($columns as $id => $name) {
							?>
							columnNames['<?php echo $id; ?>'] = '<?php echo $name; ?>';
							<?php
						}
					}
					?>

					function confirmDrag(movingElt, newCell)
					{
						var source_comps = originalParent.id.split('_');
						var target_comps = newCell.id.split('_');
						old_date = source_comps[0];
						old_time = source_comps[1];
						new_date = target_comps[0];
						new_time = target_comps[1];
						old_loc = (source_comps.length == 3) ?  source_comps[2] : '';
						new_loc = (target_comps.length == 3) ?  target_comps[2] : '';

						// can't drag in or out of the 'other' column
						if (Boolean(old_time == 'allday') != Boolean(new_time == 'allday')) {
							statusBarMsg('Events cannot be dragged in or out of the "all day" row');
							return false;
						}

						// can't drag in or out of the 'all day' row
						if (Boolean(old_loc == '*') != Boolean(new_loc == '*')) {
							statusBarMsg('Events cannot be dragged in or out of the "other" column');
							return false;
						}
						toggleFlashing(movingElt.id);
						var locationName = (new_loc != '') ? columnNames[String(new_loc)] + ' on ' : '';
						if (!confirm('Move this event to ' + locationName + new_date
							+ ((new_time != 'allday') ? (' at ' + new_time) : '') + '?')) {
							toggleFlashing(movingElt.id);
							return false;
						} else {
							return true;
						}

					}

					function onDragFinish(movingElt, newCell)
					{
						log('Drag finishing');
						var source_comps = originalParent.id.split('_');
						var target_comps = newCell.id.split('_');
						if ((target_comps.length > 2) && (source_comps[2] != target_comps[2])) {
							document.getElementById('SQ_CALENDAR_OLD_LOC').value = source_comps[2];
							document.getElementById('SQ_CALENDAR_NEW_LOC').value = target_comps[2];
						}
						document.getElementById('SQ_CALENDAR_DATE').value = target_comps[0];
						if (target_comps[1] != 'allday') {
							document.getElementById('SQ_CALENDAR_TIME').value = target_comps[1];
						}
						document.getElementById('SQ_CALENDAR_EVENT_ID').value = movingElt.id;
						document.getElementById('dragForm').submit();
						formSubmitted = true;
					}

					document.onselectstart=new Function ("return false")
			//-->
			</script>

			<form method="GET" id="dragForm" action="<?php echo $_SERVER['PHP_SELF']; ?>">
			  <input type="hidden" name="SQ_CALENDAR_NEW_LOC" id="SQ_CALENDAR_NEW_LOC" />
			  <input type="hidden" name="SQ_CALENDAR_OLD_LOC" id="SQ_CALENDAR_OLD_LOC" />
			  <input type="hidden" name="SQ_CALENDAR_EVENT_ID" id="SQ_CALENDAR_EVENT_ID" />
			  <input type="hidden" name="SQ_CALENDAR_DATE" id="SQ_CALENDAR_DATE" />
			  <input type="hidden" name="SQ_CALENDAR_TIME" id="SQ_CALENDAR_TIME" />
			  <input type="hidden" name="SQ_CALENDAR_EVENT_ACTION" id="SQ_CALENDAR_EVENT_ACTION" value="move" />
			  <input type="hidden" name="SQ_CALENDAR_VIEW" value="<?php echo $_REQUEST['SQ_CALENDAR_VIEW']; ?>" />
			</form>
		<!-- /DRAG AND DROP SUPPORT -->

		<?php

	}//end _printEventDraggingScript()


	/**
	* Print the script used to support rollover description popups
	*
	* @access private
	* @return void
	*/
	function _printPopupsScript()
	{
		?>

		<!-- DESCRIPTION POPUPS SUPPORT -->
			<script type="text/javascript" src="<?php echo sq_web_path('data'); ?>/asset_types/page_calendar/js/popup.js"></script>
		<!-- /DESCRIPTION POPUPS SUPPORT -->

		<?php

	}//end _printPopupsScript()


	/**
	* Returns the maximum number of overlapping events in a certain event list
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

		foreach (array_values($events) as $event) {
			if (empty($event['start_date_hours'])) continue; // no-time event
			$event_start_time = $event['start_date_hours'] * 60 + $event['start_date_minutes'];
			if (is_null($event['end_date_hours']) || is_null($event['end_date_minutes'])) {
				$event_end_time   = $event_start_time;
			} else {
				$event_end_time   = $event['end_date_hours']   * 60 + $event['end_date_minutes'];
			}
			$event_start_section = floor(($event_start_time - $start_time) / $spacing);
			$event_end_section = (ceil(($event_end_time - $start_time) / $spacing) - 1);

			for ($i = $event_start_section; $i <= min($end_section, $event_end_section); $i++) {
				if ((!is_null($root_node)) && ($event['root_node'] != $root_node)) {
					continue;
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
	*
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
	function _addMinutes(&$current_hour, &$current_mins, $add_mins)
	{
		$current_mins += $add_mins;
		$current_hour += floor($current_mins / 60);
		$current_mins = $current_mins % 60;

	}//end _addMinutes()


	/**
	* Modify the passed event so that it fits within the specified timeframe
	*
	* If no part of the event fits inside the timeframe, the details array is emptied
	*
	* @param array	&$details		The details of the event
	* @param array	$day_start_dts	The day-timestamp (minutes since midnight) of the day start time
	* @param array	$day_end_dts	The day-timestamp of the day end time
	*
	* @access private
	* @return void
	*/
	function _trimEvent(&$details, $day_start_dts, $day_end_dts)
	{
		$event_dts = $details['start_date_hours'] * 60 + $details['start_date_minutes'];
		$event_end_dts = $details['end_date_hours'] * 60 + $details['end_date_minutes'];
		if (($event_end_dts < $day_start_dts) || ($event_dts > $day_end_dts)) $details = Array();

		// start-of-table / end-of-table adjustments to make sure that those events
		// partly outside the bounds of the table are (a) kept inside the table and
		// (b) show on the table in the first place (in case of start time adjustment)
		if (($event_dts < $day_start_dts) && ($event_end_dts > $day_start_dts)) {
			$event_dts = $day_start_dts;
			$details['start_date_hours'] = floor($day_start_dts / 60);
			$details['start_date_minutes'] = $day_start_dts % 60;
			$details['trimmed'] = 1;
		}
		if (($event_dts < $day_end_dts) && ($event_end_dts > $day_end_dts)) {
			$event_end_dts = $day_end_dts;
			$details['end_date_hours'] = floor($day_end_dts / 60);
			$details['end_date_minutes'] = $day_end_dts % 60;
			$details['trimmed'] = 1;
		}

	}//end _trimEvent()


//--         FUNCTIONS TO EXTRACT EVENTS FROM DB        --//


	/**
	* Returns the current root node attribute
	*
	* This is in its separate function so that if this page is subclassed it can
	* place restrictions or additions on what root nodes are used.
	*
	* @return Array
	* @access protected
	*/
	function _getRootNodes()
	{
		return $this->attr('root_nodes');

	}//end _getRootNodes()


	/**
	* Get the events to show in year view for the specified year
	*
	* @return array	( assetid => asset_type_code )
	*/
	function getYearEvents($year)
	{
		$res = Array();
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$root_nodes = $this->_getRootNodes();
		$trees =&get_treeids($root_nodes);

		if (empty($trees)) {
			trigger_error('WARNING: No valid root nodes are set for this Calendar Page', E_USER_WARNING);
			return Array();
		}

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
		foreach ($res as $date => $events) {
			process_exceptions($res[$date]);
		}

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
			foreach (range(1, days_in_month($month_num, $year)) as $dom) {
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
		$root_nodes = $this->_getRootNodes();
		$trees =& get_treeids($root_nodes);

		if (empty($trees)) {
			trigger_error('WARNING: No valid root nodes are set for this Calendar Page', E_USER_WARNING);
			return Array();
		}

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
		uasort($res, 'compare_start_dates');
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
		$root_nodes = $this->_getRootNodes();
		$trees = &get_treeids($root_nodes);

		if (empty($trees)) {
			trigger_error('WARNING: No valid root nodes are set for this Calendar Page', E_USER_WARNING);
			return Array();
		}

		$tree_sql = build_tree_clause($trees);

		// get the single events
		$start_date_stamp = floor(strtotime($date.' 12:00:00 GMT')/86400);
		$end_date_stamp = $start_date_stamp + 6;
		$date_sql = '(cd.start_date_ds BETWEEN '.$db->quote($start_date_stamp).' AND '.$db->quote($end_date_stamp).') OR (cd.start_date_ds < '.$db->quote($start_date_stamp).' AND cd.end_date_ds >= '.$db->quote($start_date_stamp).')';
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
		list($e_hour, $e_mins) = explode(':', $last_time);
		$end_ts = $e_mins + (60 * $e_hour);
		list($hour, $mins) = explode(':', $first_time);
		$ts = $mins + (60 * $hour);
		$start_ts = $ts;
		while ($ts < $end_ts) {
			$partition_keys[$ts] = sprintf('%02d', $ts/60).':'.sprintf('%02d', $ts%60);
			$ts += $time_partition;
		}
		$partitions = array_flip($partition_keys);
		foreach ($partitions as $key => $val) $partitions[$key] = Array();
		$partitions['all_day'] = Array();

		$res = Array();

		$week_start_day = $this->attr('week_starts_on');
		$first_day = iso8601_ts($date) - ((7+(date('w', iso8601_ts($date)) - $week_start_day))%7)*86400;
		$cal_date = date('Y-m-d', $first_day);

		$last_day = iso8601_date_component(ts_iso8601(iso8601_ts($cal_date) + 6*86400));

		for ($i = iso8601_ts($cal_date); $i <= iso8601_ts($cal_date) + 6*86400; $i += 86400) {
			if ($days_to_show[date('w', $i)]) $res[iso8601_date_component(ts_iso8601($i))] = $partitions;
		}

		$plain_events = $this->getWeekEventsByDay($cal_date);
		foreach ($plain_events as $event_date => $events) {
			$res[$event_date]['_overlap_'] = $this->_countOverlappingEvents($events, $first_time, $last_time, $time_partition);
			foreach ($events as $id => $details) {
				if (isset($res[$event_date])) {
					if (is_null($details['start_date_hours'])) {
						$res[$event_date]['all_day'][$id] = $details;
					} else {
						$this->_trimEvent($details, $start_ts, $end_ts);
						if (empty($details)) continue;
						$event_ts = $details['start_date_hours'] * 60 + $details['start_date_minutes'];
						foreach ($partition_keys as $partition_ts => $key) {
							if (($event_ts - $partition_ts) < $time_partition) {
								$res[$event_date][$key][$id] = $details;
								break;
							}
						}
					}
				}
			}
		}

		foreach ($res as $date => $events) {
			foreach ($partition_keys as $partition) process_exceptions($res[$date][$partition]);
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
		$root_nodes = $this->_getRootNodes();
		$trees =& get_treeids($root_nodes);

		if (empty($trees)) {
			trigger_error('WARNING: No valid root nodes are set for this Calendar Page', E_USER_WARNING);
			return Array();
		}

		$tree_sql = build_tree_clause($trees);

		// GET SINGLE EVENTS:

		// build date restriction SQL
		$today_date_stamp = floor(strtotime($date.' 12:00:00 GMT')/86400);
		$date_sql = '(('.$today_date_stamp.' = cd.start_date_ds) OR ('.$today_date_stamp.' BETWEEN cd.start_date_ds AND cd.end_date_ds))';

		// combine with the common SQL for single events and get result from DB
		$sql = build_single_event_query_base($trees).' AND '.$date_sql.' AND '.$tree_sql;
		$single_result = $db->getAssoc($sql);
		assert_valid_db_result($single_result);
/*
		foreach ($single_result as $id => $details) {
			$single_result[$id]['type_code'] = 'calendar_event_single';
		}
*/
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

		$compare_start_ts = create_function('$a, $b', 'if ($a[\'start_date_ts\'] < $b[\'start_date_ts\']) return -1; else return 1;');
		uasort($res, $compare_start_ts);

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
		foreach ($partitions as $key => $val) $partitions[$key] = Array();
		$partitions['all_day'] = Array();

		$res = $partitions;

		$events = $this->getDayEvents($date);

		foreach ($events as $id => $details) {
			$event_date = sprintf('%04d-%02d-%02d', $details['start_date_year'], $details['start_date_mon'], $details['start_date_mday']);
			if (($event_date == $date)) {
				if (is_null($details['start_date_hours'])) {
					$res['all_day'][$id] = $details;
				} else {
					$this->_trimEvent($details, $start_ts, $end_ts);
					if (empty($details)) continue;
					$event_ts = $details['start_date_hours'] * 60 + $details['start_date_minutes'];
					foreach ($partition_keys as $partition_ts => $key) {
						if (($event_ts - $partition_ts) < $time_partition) {
							$res[$key][$id] = $details;
							break;
						}
					}
				}
			}
		}
		$res['_overlap_'] = $this->_countOverlappingEvents($events, $first_time, $last_time, $time_partition);
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
		if (!$root_nodes) $root_nodes = array_keys($this->_getRootNodes());
		$partition_keys = Array();
		list($e_hour, $e_mins) = explode(':', $last_time);
		$end_ts = $e_mins + (60 * $e_hour);
		list($hour, $mins) = explode(':', $first_time);
		$ts = $mins + (60 * $hour);
		$start_ts = $ts;
		while ($ts < $end_ts) {
			$partition_keys[$ts] = sprintf('%02d', $ts/60).':'.sprintf('%02d', $ts%60);
			$ts += $time_partition;
		}
		$partitions = array_flip($partition_keys);
		foreach ($partitions as $key => $val) $partitions[$key] = Array();
		$partitions['all_day'] = Array();

		$events = &$this->getDayEvents($date);
		process_exceptions($events);
		$res = Array();

		$columnise_by_root = $this->attr('columnise_day_view_by_root_node');
		foreach ($columnise_by_root as $node_id) {
			if (($node_id == '*') || in_array($node_id, $root_nodes)) $res[$node_id] = $partitions;
		}

		$root_node_treeids = get_treeids(array_flip($root_nodes));
		uasort($root_node_treeids, create_function('$a,$b', 'return strlen($b)-strlen($a);'));

		foreach ($events as $id => $details) {
			$event_date = sprintf('%04d-%02d-%02d', $details['start_date_year'], $details['start_date_mon'], $details['start_date_mday']);

			// get class to use for root node
			$col = (isset($res['*']) ? '*' : false);

			foreach ($root_node_treeids as $root_node_assetid => $root_node_treeid) {
				if (substr($details['treeid'], 0, strlen($root_node_treeid)) === $root_node_treeid) {
					if (in_array($root_node_assetid, $columnise_by_root)) {
						$col = $root_node_assetid;
						break;
					}
				}
			}

			if ($col) {
				if (empty($details['start_date_hours'])) {
					$res[$col]['all_day'][$id] = $details;
				} else {
					$this->_trimEvent($details, $start_ts, $end_ts);
					if (empty($details)) continue;
					$event_ts = $details['start_date_hours'] * 60 + $details['start_date_minutes'];
					foreach ($partition_keys as $partition_ts => $key) {
						if (($event_ts - $partition_ts) < $time_partition) {
							$res[$col][$key][$id] = $details;
							break;
						}
					}
				}
			}
		}

		foreach ($res as $root_node => $null) {
			$max_events = 1;
			$rrn = Array();
			foreach ($res[$root_node] as $key => $null) {
				foreach ($res[$root_node][$key] as $id => $null) {
					$rrn[] = &$res[$root_node][$key][$id];
				}
			}

			$max_events = max($max_events, $this->_countOverlappingEvents($rrn, $first_time, $last_time, $time_partition));

			$res[$root_node]['_overlap_'] = $max_events;
		}
		return $res;

	}//end getDayEventsByRootNodeAndTime()


}//end class
?>
