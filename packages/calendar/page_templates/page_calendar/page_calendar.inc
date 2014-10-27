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
* $Id: page_calendar.inc,v 1.254 2013/07/18 01:38:36 akarelia Exp $
*
*/

require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once SQ_FUDGE_PATH.'/general/datetime.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_PACKAGES_PATH.'/calendar/calendar_event/calendar_event.inc';
require_once SQ_PACKAGES_PATH.'/calendar/calendar_events/calendar_event_recurring/calendar_event_recurring.inc';
require_once SQ_PACKAGES_PATH.'/calendar/lib/calendar_common.inc';

/**
* Page_Calendar
*
* Purpose: To display event assets on the frontend by year, month, week, day or individually,
* in listing or calendar-style formats
*
* @author  Luke Wright <lwright@squiz.net>
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.254 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Page_Calendar extends Page
{


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
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	* @return object
	* @access public
	*/
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

	}//end constructor

	/**
	* Create this asset.
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return mixed int|boolean
	* @access public
	* @see Asset::create()
	*/
	function create(&$link)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$linkid = Page::create($link);
		if (!$linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			$this->id = 0;
			return FALSE;
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return $linkid;

	}//end create()


	/**
	* Returns TRUE if we can delete the passed link, or a string with the error msg
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return mixed boolean|string
	* @access public
	*/
	function isDeletableLink($linkid)
	{
		// the only reason why we would object to this link being
		// deleted is if it is the bodycopy linked to us
		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'major', 'page_contents', '1', '1');
		$bodycopy_no_results_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'major', 'page_contents_no_results', '1', '1');

		// if this is not a purge trash
		if (!$GLOBALS['SQ_PURGING_TRASH']) {

			// if the user is trying to delete the bodycopy or the no results body copy return an error message
			if ((!empty($bodycopy_link)) && ($bodycopy_link[0]['linkid'] == $linkid)) {
				return translate('You are now allow to delete this page contents');

			} else if ((!empty($bodycopy_no_results_link) && ($bodycopy_no_results_link[0]['linkid'] == $linkid))) {
				return translate('You are now allow to delete this page contents (no results)');

			}
		}

		return parent::isDeletableLink($linkid);

	}//end isDeletableLink()


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* For example, creating other child assets should be performed in this function
	*
	* @param array	&$link	information used to create the initial link<br/>
	* <pre>
	* Array ('asset'         => [ref major asset to create link under],
	*        'link_type'     => SQ_LINK_?,
	*        'value'         => [link value],
	*        'sort_order'    => [link sort order],
	*        'is_dependant'  => [0|1],
	*        'is_exclusive'  => [0|1]
	*        )
	* </pre>
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		if (!Page::_createAdditional($link)) return FALSE;

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');

		// create the page contents bodycopy
		$page_contents_data = Array('content' => '<h2 style="text-align: center">%calendar_title%</h2><p style="text-align: center">%prev_link%&nbsp;&nbsp;&nbsp;&nbsp;%up_link%&nbsp;&nbsp;&nbsp;&nbsp;%next_link%</p>%calendar_contents%');
		$asset = new Bodycopy();
		$copy_link = Array('asset' => &$this, 'value' => 'page_contents' ,'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);
		$asset->setAttrValue('name', 'Page Contents');
		if (!$asset->create($copy_link, $page_contents_data)) {
			$this->id = 0;
			return FALSE;
		}
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
		unset($asset);

		// create the event formats folder
		$copy_link = Array('asset' => &$this, 'value' => 'type_formats' ,'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);
		$type_formats = new Folder();
		$type_formats->setAttrValue('name', 'Type Formats');
		if (!$type_formats->create($copy_link)) {
			$this->id = 0;
			return FALSE;
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
				return FALSE;
			}
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			unset($asset);
		}

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($type_formats);
		unset($type_formats);

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
		unset($asset);

		return TRUE;

	}//end _createAdditional()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowedLinks()
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_TYPE_2]['folder']   = Array('card' => 'M', 'exclusive' => FALSE);
		$page_links[SQ_LINK_NOTICE]['asset']    = Array('card' => 'M', 'exclusive' => FALSE);
		$page_links[SQ_LINK_TYPE_2]['bodycopy'] = Array('card' => 2,   'exclusive' => TRUE);
		return $page_links;

	}//end _getAllowedLinks()


	/**
	* Print this asset on the frontend
	*
	* @return void
	* @access public
	*/
	function printFrontend()
	{

		return parent::printFrontend();

	}//end printFrontend()


	/**
	* Called by the design to print the head of this asset
	*
	* @return boolean
	* @access public
	*/
	function printHead()
	{
		if (!isset($this->_tmp['valid_request'])) {
			$this->_tmp['valid_request'] = $this->_validateRequest();
		}
		if (!$this->_tmp['valid_request']) return;

		$view = $_REQUEST['SQ_CALENDAR_VIEW'];
		$format = array_get_index($this->attr('enabled_views'), $view, '');
		if (in_array($view, Array('day', 'week')) &&  ($format == 'calendar_format')) {
			$partition_time = intval($this->attr('day_partition_time'));
			$partition_height = $this->attr('day_partition_height');
			$root_nodes = $this->attr('root_nodes');
			$width = ($view == 'day') ? $this->attr('day_column_width') : $this->attr('table_column_width');
			$click_to_add_ab = $this->attr('asset_builder_page');
			?>
			<style type="text/css">
				table#day tr td.sq-time-cell, table#week tr td.sq-time-cell {
					width: <?php echo $width; ?>px;
					height: <?php echo $partition_height; ?>px;
					padding: 0px 1px;
					border-width: 1px;
				}
				/* CSS Hack to cope with the unusual situation of strict-mode IE being better
				   than strict-mode Firefox.  The following rule will be picked up only by firefox
				   and compensates for its td-height bug. */
				html>body table#day tr td.sq-time-cell, html>body table#week tr td.sq-time-cell {
					height: <?php echo $partition_height+1; ?>px;
				}
				<?php
				if ($click_to_add_ab) {
					?>
					table#day tr td.sq-time-cell, table#week tr td.sq-time-cell, td.sq-allday-cell {
						cursor: pointer;
						cursor: hand;
					}
					<?php
				}
				?>
			</style>
			<?php
		}//end if
		?>
		<link rel="stylesheet" type="text/css" href="<?php echo sq_web_path('data'); ?>/asset_types/page_calendar/css/calendar_default.css" />
		<?php

	}//end printHead()


	/**
	* Called by the design to print the body of this asset
	*
	* @return boolean
	* @access public
	*/
	function printBody()
	{
		// start performance mode timer
		$GLOBALS['SQ_SYSTEM']->pm->startTimer($this);

		$root_node_ids = array_keys($this->attr('root_nodes'));
		$res = '';
		$action = array_get_index($_REQUEST,'SQ_CALENDAR_EVENT_ACTION',FALSE);
		$event_id = array_get_index($_REQUEST,'SQ_CALENDAR_EVENT_ID',FALSE);

		if ($event_id) {
			$event_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset((int)$_REQUEST['SQ_CALENDAR_EVENT_ID']);
			if (!$event_asset->readAccess()) return;
		}

		if ($action) {
			if ($event_asset->readAccess()) {
				$res = $event_asset->processEventAction($action);
				echo $res;
			} else {
				$this->_tmp['valid_request'] = FALSE;
			}
		}

		if (!$res) {

			if (!isset($this->_tmp['valid_request'])) {
				$this->_tmp['valid_request'] = $this->_validateRequest();
			}
			if (!$this->_tmp['valid_request']) return;

			// Try to get from cache
			$cache_key = $this->_getCacheKey();
			$cached_contents = FALSE;
			if (!empty($cache_key)) {
				$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
				$cached_contents = $cm->loadFromCache($this->id, $this->type(), $cache_key);
			}
			if ($cached_contents !== FALSE) {
				echo $cached_contents;
			} else {
				// have to generate from scratch

				// Get replacements.  Contents is first because it may change the view if,
				// for example, we have just finished editing an event
				$replacements = Array();

				// the following variable will store the event listing
				// this array will be passed to getCalendarContents.
				// getCalendarContents will make a call to
				// getEventYear, or getEvent Month, or getWeekEvents or getDayEvents to list all the event.

				$events_array = Array();

				// Save the current calendar date - we need to edit it in contents to make it work
				$calendar_date = $_REQUEST['SQ_CALENDAR_DATE'];
				$cal_view = $_REQUEST['SQ_CALENDAR_VIEW'];
				$replacements['calendar_contents'] = $this->getCalendarContents($events_array);
				$_REQUEST['SQ_CALENDAR_DATE'] = $calendar_date;

				$replacements['calendar_title'] = $this->getCalendarTitle();
				$replacements['current_user'] = $GLOBALS['SQ_SYSTEM']->user->name;

				$replacements += $this->_getNavLinkReplacements();

				// initialise the bodycopy link and contents bodycopy
				$bodycopy_link = '';
				$contents_bodycopy = '';
				// get the number of events we will need to display

				$nb_events = count($events_array);
				$event_view = FALSE;
				if ($cal_view == 'event') $event_view = TRUE;
				if ($cal_view == 'edit') $event_view = TRUE;

				// if there is no event and use_no_results_body_copy is true then we use the page_contents_no_results
				$use_no_results_body_copy = $this->attr('use_no_results_body_copy');

				if ( (!empty($use_no_results_body_copy)) && (($nb_events == 0) && !($event_view))) {
				  $bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'page_contents_no_results');
				  if (empty($bodycopy_link)) return FALSE;
				  $contents_bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link['minorid'], $bodycopy_link['minor_type_code']);
				} else {
				  $bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'page_contents');
				  if (empty($bodycopy_link)) return FALSE;
				  $contents_bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link['minorid'], $bodycopy_link['minor_type_code']);
				}

				if (is_null($contents_bodycopy)) return FALSE;



				ob_start();
					echo '<script type="text/javascript">if (typeof cal != "undefined") cal.setDate(\''.$_REQUEST['SQ_CALENDAR_DATE'].'\')</script>';
					// print the contents of page - replacing the global keywords
					$calendar_replacements = $replacements;
					$keywords = $contents_bodycopy->getKeywords();
					foreach ($keywords as $word) {
						if (isset($calendar_replacements[$word])) {
							$replacements[$word] = $calendar_replacements[$word];
						} else {
							$replacements[$word] = $this->getKeywordReplacement($word);
						}
					}

					$contents_bodycopy->setKeywordReplacements($replacements);
					$contents_bodycopy->printBody();
					if (!empty($cache_key)) {
						$cm->saveToCache($this->id, $this->type(), $cache_key, ob_get_contents());
					}
				ob_end_flush();

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($contents_bodycopy);

			}//end else no cache

		}//end if
		// stop performance mode timer
		$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this);
	}//end printBody()


	/**
	* Get bodycopy keyword replacement
	*
	* Provides a mechanism for providing keyword replacement functions by
	* looking up the function named 'getKeywordNameKeywordReplacement'
	* so an asset wishing to add support for 'best_ever' custom keyword
	* should implement a function getBestEverKeywordReplacement()
	* which returns a replacement string
	*
	* @param string $keyword	Keyword to find a replacement for
	*
	* @return string
	* @access public
	*/
	function getKeywordReplacement($keyword)
	{
		$prefix = $this->getPrefix();

		// No keywords here, just go and get the global keywords
		return parent::getKeywordReplacement($keyword);

	}//end getKeywordReplacement()


	/**
	* Look at the request values SQ_CALENDAR_DATE and SQ_CALENDAR_VIEW,
	* check they are valid and tweak them if necessary
	* It returns false if a non-recoverable problem was encountered
	*
	* @return boolean
	* @access private
	*/
	function _validateRequest()
	{
		// Validate View:
		// If we get a valid and available view type in the request, use it;
		// otherwise use the initial view attribute.
		$enabled_views = $this->attr('enabled_views');
		$enabled_views['edit'] = 1;
		$cal_view = array_get_index($_REQUEST, 'SQ_CALENDAR_VIEW', FALSE);
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
			trigger_localised_error('CAL0022', sprintf(translate('Date passed to calendar page (%s) is not valid, using today\'s date as fallback'), $cal_date), E_USER_WARNING);
			$cal_date = date('Y-m-d');
		}

		// If the view is 'event' or 'edit' we need the eventid too
		$event_id = FALSE;
		if (($cal_view == 'event') || ($cal_view == 'edit')) {
			if (!isset($_REQUEST['SQ_CALENDAR_EVENT_ID'])) {
				trigger_localised_error('CAL0023', sprintf(translate('%s View requested but no Event ID supplied'), ucfirst($cal_view)), E_USER_WARNING);
				return FALSE;
			}
			if (!assert_valid_assetid($_REQUEST['SQ_CALENDAR_EVENT_ID'])) {
				return FALSE;
			}
		}

		// Store validated values in globals for future reference
		$_REQUEST['SQ_CALENDAR_VIEW'] = $cal_view;
		$_REQUEST['SQ_CALENDAR_DATE'] = $cal_date;
		return TRUE;

	}//end _validateRequest()


	/**
	* Work out the unique cache key to represent the current page
	*
	* @return string
	* @access private
	*/
	function _getCacheKey()
	{
		$cache_key = '';
		$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();

		if (!isset($this->_tmp['valid_request'])) {
			$this->_tmp['valid_request'] = $this->_validateRequest();
		}

		if ($this->_tmp['valid_request']) {
			// Cache key will be a letter representing the view type (Y, M, W, D)
			// followed by as precise an iso-date as necessary (2004, 2004-02, 2004-04-01)
			list($year, $month, $day) = explode('-', $_REQUEST['SQ_CALENDAR_DATE']);
			switch ($_REQUEST['SQ_CALENDAR_VIEW']) {
				case 'year':
					$cache_key = 'Y'.$year;
				case 'month':
					$cache_key = sprintf('M%04d-%02d', $year, $month);
				case 'week':
					$cache_key = sprintf('W%04d-%02d-%02d', $year, $month, $day);
				case 'day':
					$cache_key = sprintf('D%04d-%02d-%02d', $year, $month, $day);
			}
		}

		// Append the context ID to the cache key, but only if we have a
		// cache key to begin with - otherwise we shouldn't be caching at all
		if ($cache_key !== '') {
			$cache_key .= '-ctx'.$contextid;
		}

		return $cache_key;

	}//end _getCacheKey()


	/**
	* Get the replacements for the up, next and previous links
	* This return an array of keyword => replacement value
	*
	* @return array
	* @access private
	*/
	function _getNavLinkReplacements()
	{
		$replacements = Array();
		$cal_view = $_REQUEST['SQ_CALENDAR_VIEW'];
		$cal_date = $_REQUEST['SQ_CALENDAR_DATE'];

		$event_id = array_get_index($_REQUEST, 'SQ_CALENDAR_EVENT_ID', FALSE);

		// the link to the prev/next/current month always points to
		// the first date of the month.
		if ($cal_view == 'month') {
			$date_format = 'Y-m-01';
		} else {
			$date_format = 'Y-m-d';
		}

		$prev_date = date($date_format, strtotime('-1 '.$cal_view, strtotime(date($date_format, strtotime($cal_date)))));
		$next_date = date($date_format, strtotime('+1 '.$cal_view, strtotime(date($date_format, strtotime($cal_date)))));

		list ($forward_limit,$forward_date) = $this->formatDatesToCompare($cal_view,$next_date,'forward','+');
		list ($backward_limit,$backward_date) = $this->formatDatesToCompare($cal_view,$prev_date,'backward','-');

		if ($forward_limit == '' || $forward_limit >= $forward_date) {
			$replacements['next_link'] = ($cal_view == 'event') ? '' : '<a class="calendarNavLink"  href="'.$this->getURL().'?SQ_CALENDAR_VIEW='.$cal_view.'&amp;SQ_CALENDAR_DATE='.$next_date.'" rel="nofollow">'.(str_replace('%v%', ucfirst($cal_view), htmlentities($this->attr('next_link_text'), ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET))).'</a>';
		} else {
			$replacements['next_link'] = '';
		}
		if ($backward_limit == '' || $backward_date >= $backward_limit) {
			$replacements['prev_link'] = ($cal_view == 'event') ? '' : '<a class="calendarNavLink"  href="'.$this->getURL().'?SQ_CALENDAR_VIEW='.$cal_view.'&amp;SQ_CALENDAR_DATE='.$prev_date.'" rel="nofollow">'.(str_replace('%v%', ucfirst($cal_view), htmlentities($this->attr('prev_link_text'), ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET))).'</a>';
		} else {
			$replacements['prev_link'] = '';
		}
		if ($backward_date > $forward_limit || $forward_date < $backward_limit) {
			$replacements['prev_link'] = '';
			$replacements['next_link'] = '';
		}

		$enabled_views = $this->attr('enabled_views');
		$replacements['today_link'] = ($cal_view == 'event') ? '' : '<a class="calendarNavLink calendarTodayLink" href="'.$this->getURL().'?SQ_CALENDAR_VIEW='.$cal_view.'" rel="nofollow">'.htmlentities($this->attr('today_link_text'), ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET).'</a>';

		if ($cal_view == 'week') {
			$cal_date = date('Y-m-01', iso8601_ts($cal_date));
		}

		$up_view = '';
		foreach ($enabled_views as $view => $type) {
			if ($view == $cal_view) break;
			if ($type) $up_view = $view;
		}

		// Check if the up view is an event, then we need to pass an event id
		$eventview = '';
		if ($up_view == 'event' && !empty($event_id)) {
			$eventview = '&amp;SQ_CALENDAR_EVENT_ID='.$event_id;
		}

		$replacements['up_link'] = $up_view ? '<a class="calendarNavLink"  href="'.$this->getHref().'?SQ_CALENDAR_VIEW='.$up_view.$eventview.'&amp;SQ_CALENDAR_DATE='.$cal_date.'" rel="nofollow">'.str_replace('%v%', ucfirst($up_view), htmlentities($this->attr('up_link_text'), ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)).'</a>' : '';

		return $replacements;

	}//end _getNavLinkReplacements()


	/**
	* Add valid keywords for this asset to an array of keywords when asked
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								we add keywords to the $vars['keywords'] array
	*
	* @return boolean
	* @access private
	*/
	function onRequestKeywords(&$broadcaster, $vars=Array())
	{
		if (!isset($vars['keywords'])) return;
		if (empty($vars['keywords'])) {
			parent::onRequestKeywords($broadcaster, $vars);
		}
		$bc_parents = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id, 'bodycopy', FALSE);
		if (empty($bc_parents)) return;

		$tf_folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, 'type_formats');
		$tf_bc_link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($tf_folder_link['minorid'], key($bc_parents), SQ_SC_LINK_ALL);
		if (empty($tf_bc_link)) {
			// presume it was the contents BC
			$vars['keywords'] = array_merge($vars['keywords'], $this->_getContentsKeywords());
		} else {
			// it's a type format BC
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($tf_bc_link['value']);
			$dummy = new $tf_bc_link['value']();
			$vars['keywords'] = array_merge($vars['keywords'], $dummy->getAvailableKeywords());
		}

	}//end onRequestKeywords()


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
	* @return array
	* @access public
	*/
	function _getContentsKeywords()
	{
		$keywords = Array(
						'calendar_contents'	=> translate('Calendar Contents'),

						'calendar_title'	=> translate('Calendar Title'),

						'up_link'			=> translate('Up Link'),

						'prev_link'			=> translate('Previous Link'),

						'next_link'			=> translate('Next Link'),

						'current_user'		=> translate('Name of Current User'),

						'today_link'		=> translate('Link to jump to today\'s date in the current view'),

						'root_nodes'		=> translate('Root Node'),

					);

		return $keywords;

	}//end _getContentsKeywords()


	/**
	* Returns an array listing all the view options available
	*
	* @return array
	* @access public
	*/
	function getViewOptions()
	{
		return Array(
				'year'	=> Array(
							''						=> translate('(none)'),

							'list_without_headings'	=> translate('List Without Headings'),

							'list_with_headings'	=> translate('List With Month Headings'),

							'calendar_format'		=> translate('Calendar Format'),

						   ),
				'month'	=> Array(
							''						=> translate('(none)'),

							'list_without_headings'	=> translate('List Without Headings'),

							'list_with_headings'	=> translate('List With Day Headings'),

							'calendar_format'		=> translate('Calendar Format'),

						   ),
				'week'	=> Array(
							''						=> translate('(none)'),

							'list_without_headings'	=> translate('List Without Headings'),

							'list_with_headings'	=> translate('List With Day Headings'),

							'calendar_format'		=> translate('Calendar Format'),

						   ),
				'day'	=> Array(
							''						=> translate('(none)'),

							'list_without_headings'	=> translate('List Without Headings'),

							'calendar_format'		=> translate('Calendar Format'),

						   ),
				'event'	=> Array(
							''					=> translate('(none)'),

							'single_event_view'	=> translate('Display individual events using this page'),

							'event_itself'		=> translate('Show individual event assets at their own URLs'),

						   ),
			   );

	}//end getViewOptions()


//--        FUNCTIONS TO GET CALENDAR CONTENTS        --//


	/**
	* Get the replacenent for the %calendar_title% keyword
	*
	* The title to be displayed
	*
	* @return string
	* @access public
	*/
	function getCalendarTitle()
	{
		$title_formats = $this->attr('view_title_formats');
		$title_formats['edit'] = translate('Edit Event');

		$view = $_REQUEST['SQ_CALENDAR_VIEW'];
		if (($view == 'event') || ($view == 'edit')) {
			// _validateRequest already made sure we have a valid event id
			$info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($_REQUEST['SQ_CALENDAR_EVENT_ID']));
			return $info[$_REQUEST['SQ_CALENDAR_EVENT_ID']]['name'];
		} else {
			return date($title_formats[$view], strtotime($_REQUEST['SQ_CALENDAR_DATE']));
		}

	}//end getCalendarTitle()


	/**
	* Get the replacement for the %calendar_contents% keyword
	*
	* The contents to be displayed
	* @param array &$events_array 	will keep a list of all the events
	*
	* @return string
	* @access public
	*/
	function getCalendarContents(&$events_array)
	{
		$cal_view = $_REQUEST['SQ_CALENDAR_VIEW'];
		$formats = $this->attr('enabled_views');
		$format = array_get_index($formats, $cal_view, '');
		$res = $this->_getGeneralScript();
		if (strpos($format, 'list') === FALSE) {
			$res .= '<div id="calendarTable">';
		}
		$func = 'get'.ucfirst($cal_view).'View';
		$res .= (($cal_view == 'event') || ($cal_view == 'edit')) ? $this->$func($_REQUEST['SQ_CALENDAR_EVENT_ID']) : $this->$func($_REQUEST['SQ_CALENDAR_DATE'], $format, $events_array);
		if (strpos($format, 'list') === FALSE) {
			$res .= '</div>';
		}
		return $res;

	}//end getCalendarContents()


	/**
	* Get the contents of the year view for the year containing the specified day
	*
	* @param string	$date			the ISO 8601'd date that we are displaying (YYYY-MM-DD)
	* @param string	$format			the format of display
	* @param array &$events_array	will keep a list of all the events
	*
	* @return void
	* @access public
	*/
	function getYearView($date, $format, &$events_array)
	{
		ob_start();
		list($year, $month, $day) = explode('-', $date);
		$enabled_views = $this->attr('enabled_views');

		if ($format == 'list_without_headings') {
			$list_entry_types = $this->attr('list_entry_types');
			$events = $this->getYearEvents($year);
			$events_array = $events;
			uasort($events, Array('Calendar_Common', 'compareStartDates'));
			if ($list_entry_types['year'] == 'title') {
				echo $this->_getTitleList($events);
			} else {
				$this->_printEventList($events);
			}
		} else if ($format == 'list_with_headings') {
			$list_entry_types = $this->attr('list_entry_types');

			$events = $this->getYearEventsByMonth($year);

			// copy all the events to $events_array
			foreach ($events as $month => $month_events) {
			  $events_array = array_merge($events_array, $month_events);
			}

			foreach ($this->month_names_full as $month_num => $month_name) {
				$_REQUEST['SQ_CALENDAR_DATE']= $year.'-'.sprintf('%02d', $month_num).'-01';
				uasort($events[$month_num], Array('Calendar_Common', 'compareStartDates'));
				echo '<h3>';
				if ($enabled_views['month']) {
					echo '<a href="'.$this->getHref().'?SQ_CALENDAR_VIEW=month&amp;SQ_CALENDAR_DATE='.$year.'-'.sprintf('%02d', $month_num).'-01">';
				}
				echo $month_name;
				if ($enabled_views['month']) echo '</a>';
				echo '</h3>';

				if ($list_entry_types['year'] == 'title') {
					echo $this->_getTitleList($events[$month_num], $enabled_views['event']);
				} else {
					$this->_printEventList($events[$month_num]);
				}
			}
		} else {
			$events = $this->getYearEventsByMonthAndDay($year);

			// add all events to the $events_array
			foreach ($events as $month => $month_array) {
				foreach ($month_array as $day => $day_events) {
					$events_array = array_merge($events_array, $day_events);
				}
			}

			$week_start_day = $this->attr('week_starts_on');
			$today_iso = date('Y-m-d');
			?>
			<table id="year">
				<tr>
					<th class="dayLabel" style="width: 6ex">&nbsp;</th>
			<?php
			for ($i = $week_start_day; $i < $week_start_day + 37; $i++) {
				$day = strtolower(substr($this->day_names[$i % 7], 0, 3));
				$label = $this->attr('label_'.$day);
				?>
					<th class="dayLabel<?php echo (($i%7)==0 || ($i%7)==6) ? ' dayLabelWeekend' : ''; ?>" style="width: 3.5ex"><b><?php echo substr($label, 0, 1); ?></b></th>
				<?php
			}
			?>
				</tr>
			<?php
			for ($current_month=1; $current_month<13; $current_month++) {
				?>
				<tr>
					<th class="monthLabel">
						<?php
						// print the month name, linked to month view if it's available
						if ($enabled_views['month']) {
							echo '<a  href="'.$this->getHref().'?SQ_CALENDAR_VIEW=month&amp;SQ_CALENDAR_DATE='.$year.'-'.sprintf('%02d', $current_month).'-01">';
						}
						echo $this->month_names_short[$current_month];
						if ($enabled_views['month']) echo '</a>';
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
						$tag_attrs = Array();
						$classes = Array('date');
						if (sprintf('%04d-%02d-%02d', $year, $current_month, $current_day) == $today_iso) {
							$classes[] = 'today';
						}
						if (in_array(date('D', strtotime("$year-$current_month-$current_day")), Array('Sat', 'Sun'))) {
							$classes[] = 'weekend';
						}
						if (!empty($events[$current_month][$current_day])) {
							$classes[] = 'eventDate';
						}
						$tag_attrs[] = 'class="'.implode(' ', $classes).'"';
						$day_href = $this->getHref().'?SQ_CALENDAR_VIEW=day&amp;SQ_CALENDAR_DATE='.sprintf('%04d-%02d-%02d', $year, $current_month, $current_day);
						echo '<td '.implode(' ', $tag_attrs).'>';
						echo '<a href="'.$day_href.'">'.$current_day.'</a>';
						echo '</td>';

					}

					// print blank days after the end of our month
					while ($i < 37) {
						?>
						<td class="<?php echo (((($week_start_day + $i) % 7)==0) || ((($week_start_day + $i) % 7)==6)) ? ' weekend' : ''; ?>">&nbsp;</td>
						<?php
						$i++;
					}
					?>
				</tr>
			<?php
			}//end for
			?>
			</table>
			<?php
		}//end else
		$res = ob_get_contents();
		ob_end_clean();
		return $res;

	}//end getYearView()


	/**
	* Get the contents of the monthly view with for the month containing the specified day
	*
	* @param string	$date	the ISO 8601'd date that we are displaying (YYYY-MM-DD)
	* @param string	$format	the format of display
	* @param array &$events_array	will keep a list of all the events
	*
	* @return void
	* @access public
	*/
	function getMonthView($date, $format, &$events_array)
	{
		$popups_enabled = $this->attr('enable_description_popups');
		if ($popups_enabled) $this->_printPopupsScript();
		list($year, $month, $day) = explode('-', $date);
		$enabled_views = $this->attr('enabled_views');
		$drill_down_views = Array('week', 'day');
		$drill_down_view = current($drill_down_views);
		while (empty($enabled_views[$drill_down_view]) && ($drill_down_view !== FALSE)) {
			$drill_down_view = next($drill_down_views);
		}

		ob_start();
		if ($format == 'list_without_headings') {
			$list_entry_types = $this->attr('list_entry_types');
			$raw_events = $this->getMonthEvents($year, $month);
			$events_array = $raw_events;
			$this->_initEventLabels('month', $raw_events);
			$events = $this->groupMonthEventsByDay($raw_events, $year, $month);
			$month_length = days_in_month($month, $year);
			$done_events = Array();

			// loop through each day, but if it's been printed before then
			// don't print it again
			for ($current_day = 1; $current_day <= $month_length; $current_day++) {
				uasort($events[$current_day], Array('Calendar_Common', 'compareStartDates'));
				$current_date = sprintf('%04s-%02s-%02s', $year, $month, $current_day);
				$_REQUEST['SQ_CALENDAR_DATE'] = $current_date;
				foreach ($events[$current_day] as $event_id => $event_data) {
					list($event_assetid, ) = explode(':', $event_id);
					if (!isset($done_events[$event_assetid])) {
						$done_events[$event_assetid] = 1;
					} else {
						unset($events[$current_day][$event_id]);
					}
				}
				if ($list_entry_types['month'] == 'title') {
					echo $this->_getTitleList($events[$current_day], $enabled_views['event']);
				} else {
					$this->_printEventList($events[$current_day]);
				}
			}
		} else if ($format == 'list_with_headings') {
			// Dates in month view with 'Listing with day Headings' option should always have url with day view
			$drill_down_view = $enabled_views['day'] ? 'day' : '';

			$list_entry_types = $this->attr('list_entry_types');
			$raw_events = $this->getMonthEvents($year, $month);
			$events_array = $raw_events;
			$this->_initEventLabels('month', $raw_events);
			$events = $this->groupMonthEventsByDay($raw_events, $year, $month);
			$month_length = days_in_month($month, $year);
			$day_heading_format = 'l jS';
			$heading_formats = $this->attr('view_heading_formats');
			if (isset($heading_formats['month_day']) && !empty($heading_formats['month_day'])) {
				$day_heading_format = $heading_formats['month_day'];
			}
			for ($current_day = 1; $current_day <= $month_length; $current_day++) {
				uasort($events[$current_day], Array('Calendar_Common', 'compareStartDates'));
				echo '<h3>';
				if ($drill_down_view) {
					echo '<a href="'.$this->getHref().'?SQ_CALENDAR_VIEW='.$drill_down_view.'&amp;SQ_CALENDAR_DATE='.$year.'-'.$month.'-'.sprintf('%02d', $current_day).'">';
				}
				$current_date = sprintf('%04s-%02s-%02s', $year, $month, $current_day);
				$_REQUEST['SQ_CALENDAR_DATE'] = $current_date;
				echo date($day_heading_format, strtotime($current_date));
				if ($drill_down_view) echo '</a>';
				echo '</h3>';
				if ($list_entry_types['month'] == 'title') {
					echo $this->_getTitleList($events[$current_day], $enabled_views['event']);
				} else {
					$this->_printEventList($events[$current_day]);
				}
			}
		} else {
			$raw_events = $this->getMonthEvents($year, $month);
			$events_array = $raw_events;
			$this->_initEventLabels('month', $raw_events);
			$events = $this->groupMonthEventsByDay($raw_events, $year, $month);
			$column_width = $this->attr('month_column_width');
			$row_height = $this->attr('month_row_height');
			$week_start_day = $this->attr('week_starts_on');
			$week_length = 7;
			$month_length = days_in_month($month, $year);
			$current_day = 1;
			$current_date = $_REQUEST['SQ_CALENDAR_DATE'];
			$current_day_name = date('l',iso8601_ts($current_date));
			$today_iso = date('Y-m-d');
			?>
			<table id="month" style="width: <?php echo (7.35 * $column_width); ?>px">
				<tr>
					<?php
					// print the day headings
					if ($this->attr('month_links_to_week')) {
						echo '<th class="dayLabel">&nbsp;</th>';
					}
					for ($i=0; $i < $week_length; $i++) {
						$day_name = $this->day_names[($week_start_day + $i) % 7];
						$day = substr(strtolower($day_name), 0, 3);
						$day_label = $this->attr('label_'.$day);
						?>
						<th style="width: <?php echo $column_width; ?>px" class="dayLabel<?php echo (($day_name == 'Saturday') || ($day_name == 'Sunday')) ? 'Weekend' : ''; ?>">
							<?php echo $day_label; ?>
						</th>
						<?php
					}
					?>
				</tr>
				<tr>
					<?php
					// First Week
					if ($this->attr('month_links_to_week')) {
						$offset = 0;
						$day_in_seconds = 24 * 60 * 60;
						$first_week_link_date = $current_date;
						$week_start_index	= $this->attr('week_starts_on');
						$curr_day_index		= array_search(date('l', iso8601_ts($current_date)), $this->day_names);
						if ($week_start_index != $curr_day_index) {
							if ($week_start_index < $curr_day_index) {
								$offset = $curr_day_index - $week_start_index;
							} else {
								$offset = $curr_day_index + (7 - $week_start_index);
							}
						}
						$first_week_link_date = iso8601_date_component(ts_iso8601(iso8601_ts($current_date) - ($offset * $day_in_seconds)));
						?>
						<th class="weekLink" onclick="clickChild(this)"><a class="dateLink" href="<?php echo $this->getHref().'?SQ_CALENDAR_VIEW=week&amp;SQ_CALENDAR_DATE='.$first_week_link_date ?>">&raquo;</a></th>
						<?php
					}

					// print blank days if necessrary
					$i = 0;
					$blank_days = (date('w', strtotime("$year-".sprintf('%02d', $month).'-01')) + 7 - $week_start_day) % 7;
					for ($i = 0; $i < $blank_days; $i++) {
						?>
						<td class="<?php echo (((($week_start_day + $i) % 7)==0) || ((($week_start_day + $i) % 7)==6)) ? 'weekend' : ''; ?>">&nbsp;</td>
						<?php
					}

					// print the rest of the first week
					for (; $i < $week_length; $i++) {
						$tag_attrs = Array();
						$current_date = sprintf('%04d-%02d-%02d', $year, $month, $current_day);
						$_REQUEST['SQ_CALENDAR_DATE'] = $current_date;
						$contents = $this->_getMonthCellContents($events[$current_day], $current_date, $column_width*0.99);
						$classes = Array('date');
						if ($current_date == $today_iso) $classes[] = 'today';
						if ($contents) $classes[] = 'eventDate';
						if (in_array(date('D', strtotime("$year-$month-$current_day")), Array('Sat', 'Sun'))) {
							$classes[] = 'weekend';
						}
						$tag_attrs[] = 'class="'.implode(' ', $classes).'"';
						$tag_attrs[] = 'style="height: '.$row_height.'px"';
						$date_url = $this->getHref().'?SQ_CALENDAR_VIEW=day&amp;SQ_CALENDAR_DATE='.$current_date;
						?>
						<td <?php echo implode(' ', $tag_attrs); ?>>
							<a class="dateLink" href="<?php echo $date_url; ?>"><?php echo $current_day; ?></a>
							<br />
							<?php echo $contents; ?>
						</td>
						<?php
						$current_day++;
					}
					?>
				</tr>

			<?php
			// print the full weeks
			$num_full_weeks = floor(($month_length - $current_day) / 7);
			for ($w = 0; $w < $num_full_weeks; $w++) {
				?>
				<tr>
					<?php
					if ($this->attr('month_links_to_week')) {
						?>
						<th class="weekLink" onclick="clickChild(this)"><a class="dateLink"  href="<?php echo $this->getHref().'?SQ_CALENDAR_VIEW=week&amp;SQ_CALENDAR_DATE='.add_days_to_iso($current_date) ?>">&raquo;</a></th>
						<?php
					}
					for ($i=0; $i < $week_length; $i++) {
						$tag_attrs = Array();
						$current_date = sprintf('%04d-%02d-%02d', $year, $month, $current_day);
						$_REQUEST['SQ_CALENDAR_DATE'] = $current_date;
						$contents = $this->_getMonthCellContents($events[$current_day], $current_date, $column_width*0.99);
						$classes = Array('date');
						if ($current_date == date('Y-m-d')) {
							$classes[] = 'today';
						}
						if ($contents) $classes[] = 'eventDate';
						if (in_array(date('D', strtotime("$year-$month-$current_day")), Array('Sat', 'Sun'))) {
							$classes[] = 'weekend';
						}
						$tag_attrs[] = 'class="'.implode(' ', $classes).'"';
						$tag_attrs[] = 'style="height: '.$row_height.'px"';
						$date_url = $this->getHref().'?SQ_CALENDAR_VIEW=day&amp;SQ_CALENDAR_DATE='.$current_date;
						?>
						<td <?php echo implode(' ', $tag_attrs); ?>>
							<a class="dateLink" href="<?php echo $date_url; ?>"><?php echo $current_day; ?></a>
							<br />
							<?php echo $contents; ?>
						</td>
						<?php
						$current_day++;
					}//end for
					?>
				</tr>
				<?php
			}//end for
			$current_date = sprintf('%04d-%02d-%02d', $year, $month, $current_day);

			// print the final week, if necessary
			if ($current_day <= $month_length) {
				$i = 0;
				?>
				<tr>
					<?php
					if ($this->attr('month_links_to_week')) {
						?>
						<th class="weekLink" onclick="clickChild(this)"><a class="dateLink"  href="<?php echo $this->getHref().'?SQ_CALENDAR_VIEW=week&amp;SQ_CALENDAR_DATE='.$current_date ?>">&raquo;</a></th>
						<?php
					}
					while ($current_day <= $month_length) {
						$tag_attrs = Array();
						$current_date = sprintf('%04d-%02d-%02d', $year, $month, $current_day);
						$_REQUEST['SQ_CALENDAR_DATE'] = $current_date;
						$contents = $this->_getMonthCellContents($events[$current_day], $current_date, $column_width*0.99);
						$classes = Array('date');
						if ($current_date == date('Y-m-d')) {
							$classes[] = 'today';
						}
						if ($contents) $classes[] = 'eventDate';
						if (in_array(date('D', strtotime("$year-$month-$current_day")), Array('Sat', 'Sun'))) {
							$classes[] = 'weekend';
						}
						$tag_attrs[] = 'class="'.implode(' ', $classes).'"';
						$tag_attrs[] = 'style="height: '.$row_height.'px"';
						$date_url = $this->getHref().'?SQ_CALENDAR_VIEW=day&amp;SQ_CALENDAR_DATE='.$current_date;
						?>
						<td <?php echo implode(' ', $tag_attrs); ?>>
							<a class="dateLink" href="<?php echo $date_url; ?>"><?php echo $current_day; ?></a>
							<br />
							<?php echo $contents; ?>
						</td>
						<?php
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
					?>
				</tr>
				<?php
			}//end if
			?>
			</table>
			<?php

		}//end else

		$res = ob_get_contents();
		ob_end_clean();
		return $res;

	}//end getMonthView()


	/**
	* Get the contents of the week view for the week containing the specified day
	*
	* @param string	$date	the ISO 8601'd date that we are displaying (YYYY-MM-DD)
	* @param string	$format	the format of display
	* @param array &$events_array	will keep a list of all the events
	*
	* @return void
	* @access public
	*/
	function getWeekView($date, $format, &$events_array)
	{
		ob_start();
		$root_nodes = $this->attr('root_nodes');
		$enabled_views = $this->attr('enabled_views');
		if ($format == 'list_without_headings') {
			$list_entry_types = $this->attr('list_entry_types');
			$raw_events = $this->getWeekEvents($date);
			$events_array = $raw_events;
			$this->_initEventLabels('week', $raw_events);
			$events = $this->groupWeekEventsByDay($raw_events, $date);
			$done_events = Array();

			// loop through each day but exclude events that have already
			// been printed
			foreach ($events as $day_date => $day_events) {
				$_REQUEST['SQ_CALENDAR_DATE'] = $day_date;
				uasort($events[$day_date], Array('Calendar_Common', 'compareStartDates'));

				foreach ($events[$day_date] as $event_id => $event_data) {
					list($event_assetid, ) = explode(':', $event_id);
					if (!isset($done_events[$event_assetid])) {
						$done_events[$event_assetid] = 1;
					} else {
						unset($events[$day_date][$event_id]);
					}
				}

				if ($list_entry_types['week'] == 'title') {
					echo $this->_getTitleList($events[$day_date], $enabled_views['event']);
				} else {
					$this->_printEventList($events[$day_date]);
				}
			}

		} else if ($format == 'list_with_headings') {
			$list_entry_types = $this->attr('list_entry_types');
			$raw_events = $this->getWeekEvents($date);
			$events_array = $raw_events;
			$this->_initEventLabels('week', $raw_events);
			$events = $this->groupWeekEventsByDay($raw_events, $date);
			$day_heading_format = 'l jS';
			$heading_formats = $this->attr('view_heading_formats');
			if (isset($heading_formats['week_day']) && !empty($heading_formats['week_day'])) {
				$day_heading_format = $heading_formats['week_day'];
			}
			foreach ($events as $day_date => $day_events) {
				$_REQUEST['SQ_CALENDAR_DATE'] = $day_date;
				uasort($events[$day_date], Array('Calendar_Common', 'compareStartDates'));
				echo '<h3>'.(($enabled_views['day']) ? '<a href="'.$this->getHref().'?SQ_CALENDAR_VIEW=day&amp;SQ_CALENDAR_DATE='.$day_date.'">' : '');
				echo date($day_heading_format, strtotime($day_date));
				echo (($enabled_views['day']) ? '</a>' : '').'</h3>';
				if ($list_entry_types['week'] == 'title') {
					echo $this->_getTitleList($events[$day_date], $enabled_views['event']);
				} else {
					$this->_printEventList($events[$day_date]);
				}
			}
		} else {
			$raw_events = $this->getWeekEvents($date);
			$events_array = $raw_events;
			$this->_initEventLabels('week', $raw_events);
			$events = $this->groupWeekEventsByDayAndTime($raw_events, $date);
			$column_names = Array();
			$column_links = Array();
			foreach ($events as $day_date => $day_events) {
				$_REQUEST['SQ_CALENDAR_DATE'] = $day_date;
				$date = date('l', strtotime($day_date));
				$day = strtolower(substr($date, 0, 3));
				$label = $this->attr('label_'.$day);
				$column_names[$day_date] = $label.' '.date('jS', strtotime($day_date));
				if ($enabled_views['week']) {
					$column_links[$day_date] = $this->getHref().'?SQ_CALENDAR_VIEW=day&amp;SQ_CALENDAR_DATE='.$day_date;
				}
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
	* @param string	$date				the ISO 8601'd day (YYYY-MM-DD)
	* @param string	$format				'list_with_headings' | 'list_without_headings' | 'calendar_format'
	* @param array &$events_array		will keep a list of all the events
	*
	* @return void
	* @access public
	*/
	function getDayView($date, $format, &$events_array)
	{
		ob_start();
		if (strpos($format, 'list') === 0) {
			$list_entry_types = $this->attr('list_entry_types');
			$enabled_views = $this->attr('enabled_views');
			$events = $this->getDayEvents($date);
			$events_array = $events;
			$this->_initEventLabels('day', $events);
			if ($list_entry_types['day'] == 'title') {
				echo $this->_getTitleList($events, $enabled_views['event']);
			} else {
				$this->_printEventList($events);
			}
		} else {
			$column_names = Array();
			$column_links = Array();
			$column_ids = $this->attr('columnise_day_view_by_root_node');
			if (array_values($column_ids) != Array('*')) {
				$root_nodes = array_keys($this->attr('root_nodes'));
				$column_names_raw = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($column_ids, Array(), TRUE, 'name');
				foreach ($column_ids as $column_id) {
					if (in_array($column_id, $root_nodes)) {
						$column_names[$column_id] = $column_names_raw[$column_id];
					}
				}

				if (in_array('*', $column_ids)) {
					$column_names['*'] = ($this->attr('other_column_name') == '') ? translate('Other') : $this->attr('other_column_name');

				}
				$raw_events = $this->getDayEvents($date);
				$events_array = $raw_events;
				$this->_initEventLabels('day', $raw_events);
				$events = $this->groupDayEventsByRootNodeAndTime($raw_events);
			} else {
				$column_names['*'] = ($this->attr('other_column_name') == '') ? translate('Events') : $this->attr('other_column_name');

				$raw_events = $this->getDayEvents($date);
				$events_array = $raw_events;
				$this->_initEventLabels('day', $raw_events);
				$events = Array('*' => $this->groupDayEventsByTime($raw_events, $date));
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
	* @param int	$event_id	the assetid of the event to display
	*
	* @return void
	* @access public
	*/
	function getEventView($event_id)
	{
		$res = '';
		ob_start();
			$info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($event_id));
			if (!empty($info)) $this->_printEventList($info);

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
	* @param int	$event_id	the assetid of the event to display
	*
	* @return void
	* @access public
	*/
	function getEditView($event_id)
	{
		$event = $GLOBALS['SQ_SYSTEM']->am->getAsset($event_id);
		require_once SQ_INCLUDE_PATH.'/limbo_outputter.inc';
		$o = new Limbo_Outputter();
		if (!$event->id) {
			trigger_localised_error('CAL0024', sprintf(translate('Could not load calendar event asset #%s'), $event_id), E_USER_WARNING);
			return FALSE;
		}

		// Process the form if appropriate
		if (isset($_POST['asset_action'])) {
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			$link = Array();
			if ($event->processBackend($o, $link)) {
				// The form has been committed and there are no probs, so go to event view
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				$GLOBALS['SQ_SYSTEM']->am->releaseLock($event_id, 'attributes');
				$_REQUEST['SQ_CALENDAR_VIEW'] = 'event';
				$_REQUEST['SQ_CALENDAR_EVENT_ID'] = $event_id;
				return $this->getEventView($event_id);
			} else {
				// There was a problem, so roll back transaction and continue printing the form
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			}
		}

		// Acquire the lock and print the interface
		if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($event_id, 'attributes')) {
			trigger_localised_error('CAL0025', sprintf(translate('Couldn\'t acquire attributes lock to edit calendar event asset "%1$s" (#%2$s)'), $event->name, $event_id), E_USER_WARNING);
			return $this->getEventView($event_id);
		}
		$o->addHiddenField('SQ_CALENDAR_VIEW', 'edit');
		$o->addHiddenField('SQ_CALENDAR_EVENT_ID', $event_id);
		$o->addHiddenField('SQ_CALENDAR_DATE', array_get_index($_REQUEST, 'SQ_CALENDAR_DATE'));
		$o->addHiddenField('asset_action', 'limbo');
		$o->addJsInclude(sq_web_path('lib').'/html_form/html_form.js');
		$o->openRaw();
		$ei = $event->getEI();
		$ei->printEditInterface('screen_details', $event, $o);
		$o->closeRaw();
		$o->commitButton(translate('Update Event'));

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
	* @param array	$events	The events to show, $assetid => array of details
	*
	* @return string
	* @access private
	*/
	function _getTitleList($events)
	{
		$res = '';
		if (!empty($events)) {
			$res = '<ul>';
			foreach ($events as $id => $data) {
				$real_id = strtok($id, ':');
				$res .= '<li>';
				$event_date = sprintf('%04d-%02d-%02d', $data['start_date_year'], $data['start_date_mon'], $data['start_date_mday']);
				$event_url = $this->_getEventLinkHref($real_id, $event_date);
				if ($event_url) $res .= '<a href="'.htmlentities($event_url).'">';

				$current_view = htmlentities($_REQUEST['SQ_CALENDAR_VIEW']);
				switch ($current_view) {
					case 'week':
					case 'month':
					case 'day':
						$res .= $this->_getEventLabel($current_view, $data);
					break;
					default:
						$res .= $data['name'];
					break;
				}
				if ($event_url) $res .= '</a>';
				$res .= '</li>';
			}
			$res .= '</ul>';
		}//end if

		return $res;

	}//end _getTitleList()


	/**
	* Print a table with times within the day-starts-at to day-ends-at range, and columns
	* as specified.
	*
	* @param string	$view			The view we are in (week or day)
	* @param array	$column_names	A column_id=>name array of the columns in the table
	* @param array	$column_links	A column_id=>href of places to link column headers to
	* @param array	$events			A column_id=>array(hhmm=>array(events)) array of events to show
	*
	* @return void
	* @access private
	*/
	function _printTimedTable($view, $column_names, $column_links, $events)
	{
		$partition_time = intval($this->attr('day_partition_time'));
		$partition_height = $this->attr('day_partition_height');
		$columns_are_assets = ($view == 'day') && (array_keys($column_names) != Array('*'));

		$root_nodes = $this->attr('root_nodes');
		$width = ($view == 'day') ? $this->attr('day_column_width') : $this->attr('table_column_width');

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
		$today_iso = date('Y-m-d');

		$mousedown_code = $dragging_enabled ? 'onmousedown="'.($popups_enabled ? 'stopTrailingPopup(); ' : '').'startDragging(this)"' : '';
		$mouseup_code = $dragging_enabled ? 'onmouseup="stopDragging(); formSubmitted=true; setTimeout(\'formSubmitted=false\', 100)"' : '';

		$table_summary = Array(
			'week'  => translate('Events this week'),
			'day'   => translate('Events today'),
		);
		?>

		<table id="<?php echo $view; ?>" border="0" cellspacing="0" cellpadding="0" summary="<?php echo $table_summary[$view]; ?>" style="width: <?php echo ($width * count($events)) + 65; ?>px">
		<?php

		// PRINT HEADER ROW
		?>
		<tr>
			<th><?php echo translate('Time'); ?></th>

		<?php
		foreach ($column_names as $id => $name) {
			$today_class = ($id == $today_iso) ? ' today' : '';
			?>
			<th class="columnLabel<?php echo $today_class; ?>" style="width: <?php echo $width; ?>px;">
				<?php
				if (!empty($column_links)) {
					echo '<a href="'.$column_links[$id].'">';
				}
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
			$today_class = ($id == $today_iso) ? ' today' : '';
			$col_events = $events[$id];
			$event_link_date = is_iso8601($id) ? $id : $_REQUEST['SQ_CALENDAR_DATE'];
			$click_to_add_code = '';
			if ($click_to_add_ab) {
				$root_node_code = ($columns_are_assets && ($id != '*')) ? ', '.$id : '';
				$click_to_add_code = ' onclick="addEvent(\''.$event_link_date.'\', \'*\''.$root_node_code.');"';
			}
			$cell_id = $event_link_date.'_allday_'.($columns_are_assets ? $id : '');
			?>
			<td class="sq-allday-cell<?php echo $today_class; ?>" id="cell_<?php echo $cell_id; ?>"<?php echo $click_to_add_code; ?>>
				<?php
				if (empty($col_events['all_day'])) echo '&nbsp;';
				foreach ($col_events['all_day'] as $assetid => $event) {
					$assetid = strtok($assetid,':');
					$event_href = $this->_getEventLinkHref($assetid, $event_link_date);
					$event_classes = $this->_getClasses($event['treeid']);
					$event_classes[] = 'event';
					$onclick_code = $event_href ? ' onclick="sq_cal_redirect(this)"' : '';
					$cursor_code = $event_href ? ' cursor: pointer; cursor: hand' : '';
					$current_mouse_up_down_code = $mousedown_code.' '.$mouseup_code;
					if (!empty($mousedown_code) && (($event['type_code'] == 'calendar_event_recurring') || (isset($event['expanded'])))) {
						$current_mouse_up_down_code = 'onmousedown="statusBarMsg(\''.translate('Recurring, multi-day and trimmed events cannot be dragged').'\')" onmouseup="statusBarMsg(\'\');"';

					}
					$current_mouse_over_out_code = '';
					if (!empty($event['description'])) {
						if (!isset($this->_tmp['popup_printed'][$assetid])) {
							$popup_div_id = $assetid.'_'.$id;
							echo '<div id="event_'.$popup_div_id.'_description" class="popup" style="display: none;">'.$event['description'].'</div>';
							$this->_tmp['popup_printed'][$assetid] = 1;
						}
						$current_mouse_over_out_code = 'onmouseover="startTrailingPopup(\'event_'.$popup_div_id.'_description\')" onmouseout="stopTrailingPopup()"';
					}
					echo '<div id="event_'.$assetid.'" '.$current_mouse_up_down_code.' '.$current_mouse_over_out_code.' style="width: '.($width-2).'px;'.$cursor_code.'" class="'.implode(' ', $event_classes).'"'.$onclick_code.'>';

					// if drag-n-drop is enabled, click-through-to-event-view is supported
					// via javascript only, because otherwise the link elements cause trouble
					// with the dragging
					$label = $this->_getEventLabel($view, $event);
					if ($event_href && empty($mousedown_code)) {
						$label = '<a class="'.implode(' ', $event_classes).'" href="'.htmlentities($event_href).'">'.$label.'</a>';
					} else {
						// print a hidden link so we can store the HREF
						// in the right attr and have the browser
						// interpret the entities
						$label = '<span class="'.implode(' ', $event_classes).'">'.$label.'</span>';
						$label .= '<a style="display:none" href="'.htmlentities($event_href).'"></a>';
					}
					echo $label;
					echo '</div>';

				}//end foreach allday events
				?>
			</td>
			<?php

		}//end foreach
		?>
		</tr>
		<?php

		// PRINT THE ROWS WITH TIMES
		while (($current_hour*60 + $current_mins) < ($end_hour*60 + $end_mins)) {
			?>
			<tr>
				<th class="timeLabel"><?php printf('%d:%02d', (($current_hour % 12 == 0) ? 12 : ($current_hour % 12)), $current_mins); ?></th>
			<?php
			foreach ($column_names as $id => $name) {
				$today_class = ($id == $today_iso) ? ' today' : '';
				$col_events = $events[$id];
				$event_link_date  = is_iso8601($id) ? $id : $_REQUEST['SQ_CALENDAR_DATE'] ;
				$time_key = sprintf('%02d:%02d', $current_hour, $current_mins);
				if ($click_to_add_ab) {
					$root_node_code = ($columns_are_assets && ($id != '*')) ? ', '.$id : '';
					$click_to_add_code = ' onclick="addEvent(\''.$event_link_date.'\', \''.$time_key.'\''.$root_node_code.');"';
				}
				$cell_id = $event_link_date.'_'.$time_key.'_'.($columns_are_assets ? $id : '');
				?>
				<td id="cell_<?php echo $cell_id; ?>" class="sq-time-cell<?php echo $today_class; ?>" <?php echo $click_to_add_code; ?>>
					<?php
					$denominator = max($col_events['_overlap_'], $this->attr('column_capacity'));
					echo $this->_getTimedTableCellContents($view, $col_events[$time_key], $event_link_date, $root_nodes, ($current_mins + ($current_hour * 60)), $partition_time, $partition_height, $div_width / $denominator, $layout_maps[$id], $mousedown_code, $mouseup_code);
					?>
					<span style="font-size: 1px">&nbsp;</span>
				</td>
				<?php
			}
			?>
			</tr>
			<?php
			$current_mins += $partition_time;
			$current_hour += floor($current_mins / 60);
			$current_mins = $current_mins % 60;
		}//end while
		?>
		</table>
		<?php

		// print all the javascript last so it doesn't screw up spacing in mozilla
		if ($dragging_enabled) {
			$this->_printEventDraggingScript($view, ($view == 'day') ? $column_names : NULL);
		}
		if ($popups_enabled) $this->_printPopupsScript();
		if ($click_to_add_ab) {
			$this->_printAddEventScript($click_to_add_ab);
		}


	}//end _printTimedTable()


	/**
	* Get the contents of a cell in the week or day table
	*
	* @param string	$view				The view we are in (week or day)
	* @param array	&$events			The events for the cell, $assetid => details
	* @param string	$event_link_date	The date to the event link
	* @param array	&$root_nodes		The root nodes for this page, used for getting classnames
	* @param int	$cell_ts			The timestamp (seconds today) of the earliest time in the cell
	* @param int	$partition_time		The length of the partition in minutes
	* @param float	$cell_height		The height of the cell, in pixels
	* @param float	$event_width		The width of the event, in pixels
	* @param array	&$layout_map		The array of layouts
	* @param string	$mousedown_code		Javascript code to put in the onmousedown attribute of the event DIVs
	* @param string	$mouseup_code		Javascript code to put in the onmouseup attribute of the event DIVs
	*
	* @return string
	* @access private
	*/
	function _getTimedTableCellContents($view, &$events, $event_link_date, &$root_nodes, $cell_ts, $partition_time, $cell_height, $event_width, &$layout_map, $mousedown_code, $mouseup_code)
	{
		if (empty($events)) return '';

		$time_label = sprintf('%02d:%02d', (int)($cell_ts / 60), $cell_ts % 60);

		// get events in order
		uasort($events, Array('Calendar_Common', 'compareStartDates'));
		$res = '';
		foreach ($events as $assetid => $details) {

			$real_assetid = current(explode(':', $assetid));
			$event_href = $this->_getEventLinkHref($real_assetid, $event_link_date);

			// get horizontal position of div and update the layout map
			$k = reset($layout_map);
			while ((key($layout_map) != $time_label) && (FALSE !== $k)) {
				next($layout_map);
			}
			$horizontal_index = 0;
			if (FALSE !== $k) {
				while (isset($layout_map[$time_label][$horizontal_index])) {
					$horizontal_index++;
				}
				for ($i=0; $i < ceil(($details['end_date_minutes'] + $details['end_date_hours']*60 - $details['start_date_minutes']-$details['start_date_hours']*60) / $partition_time); $i++) {
					$layout_map[key($layout_map)][$horizontal_index] = 1;
					next($layout_map);
				}
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

			$event_classes = $this->_getClasses($details['treeid']);
			$event_classes[] = 'event';

			// mouse action handlers
			$onclick_code = $event_href ? 'onclick="sq_cal_redirect(this)"' : '';
			$current_mouse_up_down_code = $mousedown_code.' '.$mouseup_code;
			if (!empty($mousedown_code) && (($details['type_code'] == 'calendar_event_recurring') || (isset($details['expanded'])))) {
				$current_mouse_up_down_code = 'onmousedown="statusBarMsg(\''.translate('Recurring, multi-day and trimmed events cannot be dragged').'\')" onmouseup="statusBarMsg(\'\');"';

			}
			$current_mouse_over_out_code = '';

			if (!empty($details['description'])) {
				if (!isset($this->_tmp['popup_printed'][$assetid])) {
					$res .= '<div id="event_'.$assetid.'_description" class="popup" style="display: none;">'.$details['description'].'</div>';
					$this->_tmp['popup_printed'][$assetid] = 1;
				}
				$current_mouse_over_out_code = 'onmouseover="startTrailingPopup(\'event_'.$assetid.'_description\')"  onmouseout="stopTrailingPopup()"';
			}

			// other special CSS for cursors and zero-length events
			$zero_length_code = $event_height ? '' : 'border-style: dashed; border-bottom: 0px; border-left: 0px; border-right: 0px; padding-top: 0px; background-color: transparent;';
			$cursor_code = $event_href ? 'cursor: pointer; cursor: hand;' : '';

			// showtime!
			$res .= '<div id="event_'.$assetid.'" '.$current_mouse_up_down_code.' '.$current_mouse_over_out_code.' class="'.implode(' ', $event_classes).' event" style="float: left; position: absolute; width: '.($event_width-3).'px; margin-left: '.$event_left_margin.'px; margin-top: '.$event_top_margin.'px; height: '.$event_height.'px; border-width: 1px; '.$cursor_code.' '.$zero_length_code.'" '.$onclick_code.'>';
			$res .= '<div class="eventText" style="z-index: 30; position: absolute; width: '.($event_width-4).'px; border: none;  background-color: transparent">';

			// if drag-n-drop is enabled, click-through-to-event-view is supported
			// via javascript only, because otherwise the link elements cause trouble
			// with the dragging
			$label = $this->_getEventLabel($view, $details);
			if ($event_href && empty($mousedown_code)) {
				$label = '<a class="'.implode(' ', $event_classes).'" href="'.htmlentities($event_href).'">'.$label.'</a>';
			} else {
				$label = '<span class="'.implode(' ', $event_classes).'">'.$label.'</span>';
				// add a hidden link to store the HREF so that the browser
				// decodes the entities within it
				$label .= '<a style="display:none" href="'.htmlentities($event_href).'"></a>';
			}
			$res .= $label;

			$res .= '</div></div>';

		}//end foreach
		return $res;

	}//end _getTimedTableCellContents()


	/**
	* Get the contents of a cell in the month table
	*
	* @param array	&$events			The events for the cell, $assetid => details
	* @param string	$event_link_date	The date to the event link
	* @param float	$width				The width of the event, in pixels
	*
	* @return string
	* @access private
	*/
	function _getMonthCellContents(&$events, $event_link_date, $width)
	{
		$res = '';
		uasort($events, Array('Calendar_Common', 'compareStartDates'));

		foreach ($events as $assetid => $details) {
			$popup_div_id = str_replace(':', '_', $assetid);
			$assetid = strtok($assetid, ':');
			$event_url = htmlentities($this->_getEventLinkHref($assetid, $event_link_date));
			$event_classes = $this->_getClasses($details['treeid']);
			$event_classes[] = 'event';
			$onclick_code =$event_url ? ' onclick="clickChild(this); "' : '';
			$cursor_code = $event_url ? 'cursor: pointer; cursor: hand;' : '';

			$current_mouse_over_out_code = '';

			if (!empty($details['description'])) {
				if (!isset($this->_tmp['popup_printed'][$assetid])) {
					$res .= '<div id="event_'.$assetid.'_description" class="popup" style="display: none;">'.$details['description'].'</div>';
					$this->_tmp['popup_printed'][$assetid] = 1;
				}
				$current_mouse_over_out_code = 'onmouseover="startTrailingPopup(\'event_'.$assetid.'_description\')" onmouseout="stopTrailingPopup()"';
			}


			$res .= '<div style="width: '.$width.'px; '.$cursor_code.'"'.$onclick_code.' '.$current_mouse_over_out_code.'><div style="width: auto; margin-bottom: 1px;" class="'.implode(' ', $event_classes).'">';
			if ($event_url) {
				$res .= '<a href="'.$event_url.'" class="'.implode(' ', $event_classes).'">';
			}
			$res .= $this->_getEventLabel('month', $details);
			if ($event_url) $res .= '</a>';
			$res .= '</div></div>';
		}
		return $res;

	}//end _getMonthCellContents()


	/**
	* Print general javascript used by this page
	*
	* @return void
	* @access public
	*/
	function _getGeneralScript()
	{
		$res = '<script type="text/javascript" src="'.sq_web_path('lib').'/js/general.js"></script>';
		foreach ($GLOBALS['SQ_SYSTEM']->lm->getJavascriptIncludes() as $js_include) {
			$res .= '<script type="text/javascript" src="'.$js_include.'"></script>';
		}
		$res .= '

		<!-- GENERAL CALENDAR PAGE JAVASCRIPT, ALWAYS PRESENT -->
			<script type="text/javascript">
			<!--
			var formSubmitted = false;

			function clickChild(elt) {
				 if (formSubmitted) {
					 return;
				 }
				 formSubmitted = true;
				 linkChildren = elt.getElementsByTagName("A");
				 if (linkChildren.length > 0) {
					linkChildren.item(0).click();
				 }
			}
			function sq_cal_redirect(url) {
				if (url.getElementsByTagName) {
					url = url.getElementsByTagName("A")[0].href;
				}
				if ((typeof moved == "undefined") || !moved) {
					document.location.href = url;
					formSubmitted = true;
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
		return $res;

	}//end _getGeneralScript()


	/**
	* Print the javascript needed to support click-to-add-event behaviour
	*
	* @param int	$ab_id	The assetid of the asset builder page to redirect to
	*
	* @return void
	* @access private
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
				  if (time != '*') loc += '&SQ_CALENDAR_TIME='+time+'&SQ_CALENDAR_DURATION=<?php echo (int)$this->attr('day_partition_time');?>_i';
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
	* @param int	$table_id	The ID of the HTML table within which events are being dragged and dropped
	* @param array	$columns	The array of columns
	*
	* @return void
	* @access private
	*/
	function _printEventDraggingScript($table_id, $columns=NULL)
	{
		?>

		<!-- DRAG AND DROP SUPPORT -->
			<script type="text/javascript" src="<?php echo sq_web_path('data'); ?>/asset_types/page_calendar/js/drag_n_drop.js"></script>
			<script type="text/javascript"><!--
			//<![CDATA[
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
					document.onselectstart=new Function ("return false")
			//]]> -->
			</script>
			<form method="get" id="dragForm" action="<?php echo $_SERVER['PHP_SELF']; ?>">
				<input type="hidden" name="SQ_CALENDAR_NEW_LOC" id="SQ_CALENDAR_NEW_LOC" />
				<input type="hidden" name="SQ_CALENDAR_OLD_LOC" id="SQ_CALENDAR_OLD_LOC" />
				<input type="hidden" name="SQ_CALENDAR_EVENT_ID" id="SQ_CALENDAR_EVENT_ID" />
				<input type="hidden" name="SQ_CALENDAR_DATE" value="<?php echo htmlspecialchars($_REQUEST['SQ_CALENDAR_DATE']); ?>" />
				<input type="hidden" name="SQ_CALENDAR_NEW_DATE" id="SQ_CALENDAR_NEW_DATE" />
				<input type="hidden" name="SQ_CALENDAR_NEW_TIME" id="SQ_CALENDAR_NEW_TIME" />
				<input type="hidden" name="SQ_CALENDAR_VIEW" value="<?php echo htmlspecialchars(array_get_index($_REQUEST, 'SQ_CALENDAR_VIEW', '')); ?>" />
				<input type="hidden" name="SQ_CALENDAR_EVENT_ACTION" id="SQ_CALENDAR_EVENT_ACTION" value="move" />
			</form>
		<!-- /DRAG AND DROP SUPPORT -->

		<?php

	}//end _printEventDraggingScript()


	/**
	* Print the script used to support rollover description popups
	*
	* @return void
	* @access private
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
	* minimum of 1
	*
	* @param array	&$events	array of events containing at least start_date and
	*							end_date indices which only contain a time component
	*							(HH:MM:SS) as their value for both
	* @param string	$start		Earliest time to check for overlaps (HH:MM)
	* @param string	$end		Latest time to check for overlaps (HH:MM)
	* @param int	$spacing	Calendar spacing in MINUTES
	* @param int	$root_node	The root node that we are restricting this search to. If left
	*							NULL this will search all root nodes
	*
	* @return int
	* @access private
	*/
	function _countOverlappingEvents(&$events, $start, $end, $spacing, $root_node=NULL)
	{
		if (empty($events) || !is_array($events)) {
			return 1;
		}

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
	* @param float	&$current_hour	The timestamp of the current hour
	* @param float	&$current_mins	The timestamp of the current minutes
	* @param float	$add_mins		The timestamp of the minutes to add
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
	* @return void
	* @access private
	*/
	function _trimEvent(&$details, $day_start_dts, $day_end_dts)
	{
		$event_dts = $details['start_date_hours'] * 60 + $details['start_date_minutes'];
		$event_end_dts = $details['end_date_hours'] * 60 + $details['end_date_minutes'];
		if ((!empty($details['end_date_ts']) && ($event_end_dts < $day_start_dts)) || ($event_dts > $day_end_dts)) {
			$details = Array();
		}

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


	/**
	* Print the list of events
	*
	* The calendar can choose the format of the view as a list.
	* This function prints the events in the form of text list.
	*
	* @param array	&$events	an array of events to print
	*
	* @return void
	* @access public
	*/
	function _printEventList(&$events)
	{

		if (empty($events)) return;
		uasort($events, Array('Calendar_Common', 'compareStartDates'));

		$assetids = array_keys($events);
		$am =& $GLOBALS['SQ_SYSTEM']->am;


		if (!isset($this->_tmp['type_format_bcs'])) {
			// Get our two type format BCs
			$type_format_bodycopies = Array();
			$folder_link = $am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, 'type_formats');
			$links = $am->getLinks($folder_link['minorid'], SQ_LINK_TYPE_2, 'bodycopy', TRUE);
			$this->_tmp['list_needs_asset_info'] = FALSE;
			foreach ($links as $bodycopy) {
				$bodycopy_type_code = $bodycopy['value'];
				$type_format_bodycopies[$bodycopy_type_code] = Array();
				$type_format_bodycopies[$bodycopy_type_code]['obj'] = $am->getAsset($bodycopy['minorid'], 'bodycopy');
				$type_format_bodycopies[$bodycopy_type_code]['keywords'] = $type_format_bodycopies[$bodycopy_type_code]['obj']->getKeywords();
				$data_reqs = $this->_getKeywordDataRequirements($type_format_bodycopies[$bodycopy_type_code]['keywords']);
				$type_format_bodycopies[$bodycopy_type_code]['needs_asset'] = ($data_reqs == 2);
				if ($data_reqs == 1) {
					$this->_tmp['list_needs_asset_info'] = TRUE;
				}
			}
			$this->_tmp['type_format_bcs'] =& $type_format_bodycopies;
		} else {
			$type_format_bodycopies =& $this->_tmp['type_format_bcs'];
		}

		foreach ($assetids as $i => $assetid) {
			$assetids[$i] = strtok($assetid, ':');
		}
		$assetids = array_unique($assetids);

		if ($this->_tmp['list_needs_asset_info']) {
			$this->_tmp['asset_infos'] = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($assetids);
			foreach ($this->_tmp['asset_infos'] as $id => $info) {
				$asset_info[$id]['assetid'] = $id;
			}
		}

		// loop each event
		foreach ($events as $assetid => $event) {
			$event['assetid'] = $assetid;
			$type_code = $event['type_code'];
			$assetid = strtok($assetid, ':');

			// Determine which bodycopy to use
			$found_bodycopy = FALSE;
			if (isset($type_format_bodycopies[$type_code])) {
				$bodycopy =& $type_format_bodycopies[$type_code];
				$found_bodycopy = TRUE;
			} else if ($type_code == 'calendar_event_modification' ) {
				// Modification Event
				$link = $am->getLink($assetid, SQ_LINK_TYPE_2, 'calendar_event_recurring', TRUE, NULL, 'minor');
				$bodycopy =& $type_format_bodycopies[$link['major_type_code']];
				$found_bodycopy = TRUE;
			} else if ($type_code == 'calendar_event_cancellation' ) {
				continue;
			} else {
				// check parent type
				$type_ancestors = $am->getTypeAncestors($type_code);
				foreach ($type_ancestors as $parent_type) {
					if (isset($type_format_bodycopies[$parent_type])) {
						$type_format_bodycopies[$type_code] =& $type_format_bodycopies[$parent_type];
						$bodycopy =& $type_format_bodycopies[$parent_type];
						$found_bodycopy = TRUE;
						break;
					} else if ($parent_type == 'calendar_event_cancellation' ) {
						continue(2);
					}
				}
				if (!$found_bodycopy) {
					trigger_localised_error('CAL0061', sprintf(translate('Suitable bodycopy not fournt to print the event of %s'), $assetid), E_USER_WARNING);
					continue;
				}
			}

			$replacements = Array();
			if ($bodycopy['needs_asset']) {
				$event_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type_code);
				foreach ($bodycopy['keywords'] as $keyword) {
					if ($keyword == 'asset_contents') {
						ob_start();
							$event_asset->printBody();
						$replacements['asset_contents'] = ob_get_clean();
					} else {
						$replacements[$keyword] = $event_asset->getKeywordReplacement($keyword);
					}
				}
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($event_asset);
				unset($event_asset);
			} else {
				foreach ($bodycopy['keywords'] as $keyword) {
					$replacements[$keyword] = $this->_getEventKeywordReplacement($keyword, $event);
				}
			}

			// Allows listing within the Calendar Page (see #5966)
			if (isset($_SESSION[SQ_SESSION_SANDBOX_INDEX]['list_current_asset_id'])) {
				$prev_list_current_asset_id = $_SESSION[SQ_SESSION_SANDBOX_INDEX]['list_current_asset_id'];
			} else {
				$prev_list_current_asset_id = NULL;
			}
			$_SESSION[SQ_SESSION_SANDBOX_INDEX]['list_current_asset_id'] = $assetid;

			$bodycopy['obj']->setKeywordReplacements($replacements);
			$bodycopy['obj']->printBody();

			if ($prev_list_current_asset_id) {
				$_SESSION[SQ_SESSION_SANDBOX_INDEX]['list_current_asset_id'] = $prev_list_current_asset_id;
			} else {
				unset($_SESSION[SQ_SESSION_SANDBOX_INDEX]['list_current_asset_id']);
			}

		}//end foreach event

	}//end _printEventList()


	/**
	* Gets the list of current root nodes
	*
	* @return array
	* @access public
	*/
	function _getRootNodeIds()
	{
		$root_asset_ids = array_keys($this->attr('root_nodes'));
		if (empty($root_asset_ids)) {
			$root_asset_ids = Array($this->id);
		}
		return $root_asset_ids;

	}//end _getRootNodeIds()


	/**
	* Get the CSS classes that should be applied to an event with the supplied treeids
	*
	* @param array	$treeids	Array of treeids that the event asset has
	*
	* @return array
	* @access private
	*/
	function _getClasses($treeids)
	{
		$root_nodes = $this->attr('root_nodes');
		// No root nodes means no CSS classes, as classes are applied to the root nodes
		if (empty($root_nodes)) {
			return Array();
		}

		usort($treeids, Array(&$this, '_strlenCmp'));
		$treeid_key = implode(',', $treeids);
		if (!isset($this->_tmp['treeid_map'])) {
			$root_node_roots = $GLOBALS['SQ_SYSTEM']->am->getAssetTreeids(array_keys($root_nodes));
			$treeid_map = Array();
			foreach ($root_node_roots as $assetid => $root_treeids) {
				$root_treeid = $root_treeids[0];
				$treeid_map[reset($root_treeid)] = $assetid;
			}
			uksort($treeid_map, Array(&$this, '_strlenCmp'));
			$this->_tmp['treeid_map'] = $treeid_map;
		}
		$treeids = $treeids;
		$classes = Array();
		$res = Array();
		foreach ($treeids as $treeid) {
			foreach ($this->_tmp['treeid_map'] as $class_treeid => $assetid) {
				if (0 === strpos($treeid, $class_treeid) && !empty($root_nodes[$assetid]['class_name'])) {
					$res[] = $root_nodes[$assetid]['class_name'];
				}
			}
		}
		return array_unique($res);

	}//end _getClasses()


	/**
	* Used for treeid depth sorting
	*
	* @param string	$a	First comparee
	* @param string	$b	Second comparee
	*
	* @return boolean
	* @access private
	*/
	function _strlenCmp($a, $b)
	{
		if (strlen($a) == strlen($b)) return $a > $b;
		return strlen($a) > strlen($b);

	}//end _strlenCmp()


	/**
	* Get the HREF we should use when linking to an individual event
	*
	* The result depends on whether we have been told to link to the event itself, or to a
	* view within this calendar page that shows only that event
	*
	* @param int	$eventid	The ID of the event
	* @param string	$date		The date of the occurrence (optional)
	*
	* @return string
	* @access public
	*/
	function _getEventLinkHref($eventid, $date=NULL)
	{
		if (!isset($this->_tmp['single_event_view'])) {
			$enabled_views = $this->attr('enabled_views');
			$this->_tmp['single_event_view'] = $enabled_views['event'];
		}
		$res = '';
		switch ($this->_tmp['single_event_view']) {
			case '':
				return '';
			break;
			case 'single_event_view':
				$res = $this->getURL().'?SQ_CALENDAR_VIEW=event&SQ_CALENDAR_EVENT_ID='.$eventid;
				if (!empty($res) && !is_null($date)) {
					$res .= '&SQ_CALENDAR_DATE='.$date;
				}
			break;
			case 'event_itself':
				$res = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($eventid);
				if (!empty($res) && !is_null($date)) {
					$res .= '?SQ_CALENDAR_DATE='.$date;
				}
			break;
		}
		return $res;

	}//end _getEventLinkHref()


	/**
	* Initialise stuff to help us print event labels efficiently
	*
	* @param string	$view		The calendar view we are painting
	* @param array	&$events	Ids of the events we'll be painting
	*
	* @return void
	* @access private
	*/
	function _initEventLabels($view, &$events)
	{
		$this->_tmp['event_label_format'] = $this->attr($view.'_view_event_label_format');
		$this->_tmp['event_label_format_keywords'] = retrieve_keywords_replacements($this->_tmp['event_label_format']);
		$data_requirements = $this->_getKeywordDataRequirements($this->_tmp['event_label_format_keywords']);
		$this->_tmp['event_label_need_asset'] = ($data_requirements == 2);
		if ($data_requirements == 1) {
			$this->_tmp['asset_infos'] = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($events));
		}

	}//end _initEventLabels()


	/**
	* Get the data source requirements for the supplied keywords
	*
	* Returns 0 if the data from the event table is sufficient, 1 if we need to call
	* getAssetInfo, and 2 if we need to load the asset
	*
	* @param array	$keywords	The keywords needing replacemnet
	*
	* @return int
	* @access private
	*/
	function _getKeywordDataRequirements($keywords)
	{
		$calendar_result_keywords = Array('asset_attribute_start_date', 'asset_attribute_end_date');
		$asset_info_fields = $GLOBALS['SQ_SYSTEM']->am->getAssetInfoFields();
		$date_formats = get_date_formats();
		$need_asset_info = FALSE;
		$need_asset = FALSE;
		foreach ($keywords as $keyword) {
			if ($keyword == 'asset_name') continue;
			foreach ($calendar_result_keywords as $calendar_keyword) {
				if ($keyword == $calendar_keyword) continue 2;
				if (0 === strpos($keyword, $calendar_keyword.'_')) {
					$suffix = substr($keyword, strlen($calendar_keyword)+1);
					if (isset($date_formats[$suffix])) continue 2;
				}
			}
			if (0 === strpos($keyword, 'asset_')) {
				$fieldname = substr($keyword, 6);
				if (isset($asset_info_fields[$fieldname])) {
					$need_asset_info = TRUE;
					continue;
				} else {
					$need_asset = TRUE;
					continue;
				}
			}
			return 2;
		}

		if ($need_asset) return 2;
		if ($need_asset_info) return 1;

		return 0;


	}//end _getKeywordDataRequirements()


	/**
	* Get the event label for an event
	*
	* @param string	$view			The calendar view we are painting
	* @param array	&$event_data	Details of the event we are painting
	*
	* @return string
	* @access private
	*/
	function _getEventLabel($view, &$event_data)
	{
		$replacements = Array();
		if ($this->_tmp['event_label_need_asset']) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($event_data['assetid']);
			foreach ($this->_tmp['event_label_format_keywords'] as $keyword) {
				$replacements[$keyword] = $asset->getKeywordReplacement($keyword);
			}
		} else {
			foreach ($this->_tmp['event_label_format_keywords'] as $keyword) {
				$replacements[$keyword] = $this->_getEventKeywordReplacement($keyword, $event_data);
			}
		}
		$fmt = $this->_tmp['event_label_format'];
		replace_keywords($fmt, $replacements);
		return $fmt;

	}//end _getEventLabel()


	/**
	* Get the replacement for a keyword belonging to an individual event
	*
	* This method is called when we've previously determined that it's possible
	* to get this keyword's replacement without loading the asset
	*
	* @param string	$keyword	The keyword to replace
	* @param array	$event_data	This event's entry from the event-getting function
	*
	* @return string
	* @access private
	*/
	function _getEventKeywordReplacement($keyword, $event_data)
	{
		$date_formats = get_date_formats();
		if ($keyword == 'asset_name') {
			return $event_data['name'];
		}
		if ($keyword == 'asset_assetid') {
			return $event_data['assetid'];
		}
		$calendar_result_keywords = Array('start_date', 'end_date');
		foreach ($calendar_result_keywords as $calendar_field) {
			if ($keyword == 'asset_attribute_'.$calendar_field) {
				return ts_iso8601($event_data[$calendar_field.'_ts']);
			}
			if (0 === strpos($keyword, 'asset_attribute_'.$calendar_field.'_')) {
				$suffix = substr($keyword, strlen($calendar_field)+17);
				if (isset($date_formats[$suffix])) {
					return  date($date_formats[$suffix], $event_data[$calendar_field.'_ts']);
				}
			}
		}
		if (0 === strpos($keyword, 'asset_')) {
			$field = substr($keyword, 6);
			if (isset($this->_tmp['asset_infos'][$event_data['assetid']][$field])) {
				return $this->_tmp['asset_infos'][$event_data['assetid']][$field];
			}
		}

	}//end _getEventKeywordReplacement()


//--         FUNCTIONS TO EXTRACT EVENTS FROM DB        --//


	/**
	* Get the events to show in year view for the specified year
	* It return the array of assetid => asset_type_code
	*
	* @param int	$year	the year of the events
	*
	* @return array
	* @access private
	*/
	function getYearEvents($year)
	{
		$res = Array();
		$db = MatrixDAL::getDb();

		$root_ids = $this->_getRootNodeIds();
		if (empty($root_ids)) {
			trigger_localised_error('CAL0026', translate('WARNING: No valid root nodes are set for this Calendar Page'), E_USER_WARNING);
			return Array();
		}

		// formulate the date restriction clauses
		$start_ts = strtotime("$year-01-01 00:00:00");
		$next_year = $year + 1;
		$end_ts = strtotime("$next_year-01-01 00:00:00") - 1;

		// Get our single events
		$bind_vars = Array();
		$date_sql = '(cd.start_date_year = :start_date_year) OR (cd.end_date_year = :end_date_year) OR  ((cd.start_date_ts < :start_date_ts) AND (cd.end_date_ts > :end_date_ts))';
		$sql = Calendar_Common::getSingleEventQueryBase($root_ids, 'calendar_event_single', TRUE, $bind_vars)."\n AND (".$date_sql.')';
		$sql .= $this->_getRootNodeRestrictionSQL();

		$single_events = Array();
		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			foreach($bind_vars as $bind_value => $bind_var) {
				MatrixDAL::bindValueToPdo($query, $bind_var,  $bind_value);
			}
			MatrixDAL::bindValueToPdo($query, 'start_date_year', $year);
			MatrixDAL::bindValueToPdo($query, 'end_date_year',   $year);
			MatrixDAL::bindValueToPdo($query, 'start_date_ts',   $start_ts);
			MatrixDAL::bindValueToPdo($query, 'end_date_ts',     $start_ts);

			$single_events = MatrixDAL::executePdoAll($query);
		} catch (Exception $e) {
			throw new Exception($e->getMessage());
		}//end

		$single_events = Calendar_Common::condenseResultTreeids($single_events);

		// Get our recurring events
		$bind_vars = Array();
		$date_sql = 'cd.start_date_ts < :start_date_ts';
		$sql = Calendar_Common::getRecurringEventQueryBase($root_ids, 'year', $year, NULL, 'calendar_event_recurring', $bind_vars)."\nAND (".$date_sql.')';
		$sql .= $this->_getRootNodeRestrictionSQL();

		$recurring_events = Array();
		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			foreach($bind_vars as $bind_value => $bind_var) {
				MatrixDAL::bindValueToPdo($query, $bind_var,  $bind_value);
			}
			MatrixDAL::bindValueToPdo($query, 'start_date_ts', $end_ts);
			$recurring_events = MatrixDAL::executePdoAll($query);
		} catch (Exception $e) {
			throw new Exception($e->getMessage());
		}//end

		$recurring_events = Calendar_Common::condenseResultTreeids($recurring_events);

		$res = $single_events + $recurring_events;
		return $res;

	}//end getYearEvents()


	/**
	* Get the events to show in year view for the specified year, separated into months
	* It return the array of month_number => array ( assetid => asset_type_code )
	*
	* @param int	$year	the year of the events
	*
	* @return array
	* @access private
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

		$events =& Calendar_Common::expandEventList($plain_events, $cal_date, $last_day);

		foreach ($events as $id => $details) {
			if ($details['start_date_year'] == $year) {
				$res[$details['start_date_mon']][strtok($id, ':')] = $details; // use strtok because we only want to see one instance per month
			}
		}

		return $res;

	}//end getYearEventsByMonth()


	/**
	* Get the events to show in year view for the specified year, separated into months and days
	* It return the array of month_number => array
	* (month_number => array ( day_number => array (assetid => array(...))))
	*
	* @param int	$year	the year of the events
	*
	* @return array
	* @access private
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
		$events =& Calendar_Common::expandEventList($plain_events, $cal_date, $last_day);

		foreach ($events as $id => $details) {
			if ($details['start_date_year'] == $year) {
				$res[$details['start_date_mon']][$details['start_date_mday']][$id] = $details;
			}
		}
		foreach (array_keys($res) as $month) {
			foreach (array_keys($res[$month]) as $date) {
				Calendar_Common::processExceptions($res[$month][$date]);
			}
		}
		return $res;

	}//end getYearEventsByMonthAndDay()


	/**
	* Get the events to show in month view for the specified month
	* It returns the array of assetid => array(...)
	*
	* @param int	$year	the year of the events
	* @param int	$month	the month of the events
	*
	* @return array
	* @access private
	*/
	function getMonthEvents($year, $month)
	{
		$db = MatrixDAL::getDb();

		// get the where clauses ready for tree restrictions
		$root_ids = $this->_getRootNodeIds();
		if (empty($root_ids)) {
			trigger_localised_error('CAL0027', translate('WARNING: No valid root nodes are set for this Calendar Page'), E_USER_WARNING);
			return Array();
		}

		// formulate the date restriction clauses
		$start_ts = strtotime("$year-$month-01 00:00:00 GMT");
		$next_month = ($month % 12) + 1;
		$next_year = ($next_month == 1) ? $year + 1 : $year;
		$end_ts = strtotime("$next_year-$next_month-01 00:00:00 GMT") - 1;

		// Get our single events
		$bind_vars = Array();
		$date_sql = '((cd.start_date_year = :start_date_year AND (cd.start_date_mon = :start_date_month OR cd.end_date_mon = :end_date_month)) OR (cd.start_date_ts < :start_date_ts AND cd.end_date_ts > :end_date_ts))';
		$sql = Calendar_Common::getSingleEventQueryBase($root_ids, 'calendar_event_single', TRUE, $bind_vars)."\n AND (".$date_sql.')';
		$sql .= $this->_getRootNodeRestrictionSQL();

		$single_result = Array();
		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			foreach($bind_vars as $bind_value => $bind_var) {
				MatrixDAL::bindValueToPdo($query, $bind_var,  $bind_value);
			}
			MatrixDAL::bindValueToPdo($query, 'start_date_year',  $year);
			MatrixDAL::bindValueToPdo($query, 'start_date_month', $month);
			MatrixDAL::bindValueToPdo($query, 'end_date_month',   $month);
			MatrixDAL::bindValueToPdo($query, 'start_date_ts',    $start_ts);
			MatrixDAL::bindValueToPdo($query, 'end_date_ts'  ,    $start_ts);
			$single_result = MatrixDAL::executePdoAll($query);
		} catch (Exception $e) {
			throw new Exception($e->getMessage());
		}//end

		$single_result = Calendar_Common::condenseResultTreeids($single_result);

		$cal_date = sprintf('%04d-%02d-%02d', $year, $month, 1);
		$first_day = iso8601_ts($cal_date);
		$last_day = sprintf('%04d-%02d-%02d', $year, $month, days_in_month($month, $year));

		// Get our recurring events
		$sql = Calendar_Common::getRecurringEventQueryBase($root_ids, 'month', "$year-$month", NULL, 'calendar_event_recurring', $bind_vars);
		$sql .= $this->_getRootNodeRestrictionSQL();

		$recur_result = Array();
		 try {
			$query = MatrixDAL::preparePdoQuery($sql);
			foreach($bind_vars as $bind_value => $bind_var) {
				MatrixDAL::bindValueToPdo($query, $bind_var,  $bind_value);
			}
			$recur_result = MatrixDAL::executePdoAll($query);
		} catch (Exception $e) {
			throw new Exception($e->getMessage());
		}//end
		$recur_result = Calendar_Common::condenseResultTreeids($recur_result);

		$res = $single_result + $recur_result;
		if ($this->attr('enable_description_popups')) {
			$descriptions = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('description', 'calendar_event', array_keys($res));
			foreach ($descriptions as $assetid => $description) {
				$res[$assetid]['description'] = $description;
			}
		}
		return $res;

	}//end getMonthEvents()


	/**
	* Group the supplied event list by day
	*
	* @param array	$plain_events	The list of events to group
	* @param int	$year			The year of the events
	* @param int	$month			The month of the events
	*
	* @return array
	* @access private
	*/
	function groupMonthEventsByDay($plain_events, $year, $month)
	{
		$month_length = days_in_month($month, $year);

		$res = Array();
		for ($i = 1; $i <= $month_length; $i++) {
			$res[$i] = Array();
		}

		$cal_date = sprintf('%04d-%02d-%02d', $year, $month, 1);
		$last_day = sprintf('%04d-%02d-%02d', $year, $month, $month_length);

		$events =& Calendar_Common::expandEventList($plain_events, $cal_date, $last_day);

		foreach ($events as $id => $details) {
			if (($details['start_date_mon'] == $month) && ($details['start_date_year'] == $year)) {
				$res[$details['start_date_mday']][$id] = $details;
			}
		}

		foreach ($res as $date => $events) {
			Calendar_Common::processExceptions($res[$date]);
		}
		return $res;

	}//end groupMonthEventsByDay()


	/**
	* Get the events to show in week view for the week containing the given YYYY-MM-DD date
	* It returns the array of (assetid => Array(...) ))
	*
	* @param string	$date	The current date
	*
	* @return array
	* @access private
	*/
	function getWeekEvents($date)
	{
		$db = MatrixDAL::getDb();

		list($year,$month,$day) = sscanf($date, '%04d-%02d-%02d');

		$root_ids = $this->_getRootNodeIds();
		if (empty($root_ids)) {
			trigger_localised_error('CAL0028', translate('WARNING: No valid root nodes are set for this Calendar Page'), E_USER_WARNING);
			return Array();
		}

		// get the single events
		$bind_vars = Array();
		$start_date_stamp = Calendar_Common::_getDayStamp($date);
		$end_date_stamp = $start_date_stamp + 6;
		$date_sql = '(cd.start_date_ds BETWEEN :start_date_ds AND :end_date_ds) OR (cd.start_date_ds < :start_date_ds_1 AND cd.end_date_ds >= :start_date_ds_2)';
		$sql = Calendar_Common::getSingleEventQueryBase($root_ids, 'calendar_event_single', TRUE, $bind_vars).' AND ('.$date_sql.')';
		$sql .= $this->_getRootNodeRestrictionSQL();

		$single_result = Array();
		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			foreach($bind_vars as $bind_value => $bind_var) {
				MatrixDAL::bindValueToPdo($query, $bind_var,  $bind_value);
			}
			MatrixDAL::bindValueToPdo($query, 'start_date_ds',   $start_date_stamp, PDO::PARAM_INT);
			MatrixDAL::bindValueToPdo($query, 'start_date_ds_1', $start_date_stamp, PDO::PARAM_INT);
			MatrixDAL::bindValueToPdo($query, 'start_date_ds_2', $start_date_stamp, PDO::PARAM_INT);
			MatrixDAL::bindValueToPdo($query, 'end_date_ds',     $end_date_stamp,   PDO::PARAM_INT);
			$single_result = MatrixDAL::executePdoAll($query);
		} catch (Exception $e) {
			throw new Exception($e->getMessage());
		}//end

		$single_result = Calendar_Common::condenseResultTreeids($single_result);

		// get the recurring events
		$sql = Calendar_Common::getRecurringEventQueryBase($root_ids, 'week', $date, NULL, 'calendar_event_recurring', $bind_vars);
		$sql .= $this->_getRootNodeRestrictionSQL();

		$recur_result = Array();
		 try {
			$query = MatrixDAL::preparePdoQuery($sql);
			foreach($bind_vars as $bind_value => $bind_var) {
				MatrixDAL::bindValueToPdo($query, $bind_var,  $bind_value);
			}
			$recur_result = MatrixDAL::executePdoAll($query);
		} catch (Exception $e) {
			throw new Exception($e->getMessage());
		}//end

		$recur_result = Calendar_Common::condenseResultTreeids($recur_result);

		$plain_events = $single_result + $recur_result;

		$week_start_day = $this->attr('week_starts_on');
		$seconds_in_day = 86400;
		$first_day = iso8601_ts($date) - ((7+(date('w', iso8601_ts($date)) - $week_start_day))%7)*$seconds_in_day;
		$cal_date = date('Y-m-d', $first_day);

		$last_day = add_days_to_iso($cal_date, 6);

		$events =& Calendar_Common::expandEventList($plain_events, $cal_date, $last_day);

		$res = Array();
		foreach ($events as $id => $details) {
			$event_date = iso8601_ts(sprintf('%04d-%02d-%02d', $details['start_date_year'], $details['start_date_mon'], $details['start_date_mday']));
			list($assetid, ) = explode(':', $id);
			if ($event_date >= iso8601_ts($date) && $event_date < iso8601_ts($date)+7*86400) {
				$res[$assetid] = $plain_events[$assetid];
			}
		}

		if ($this->attr('enable_description_popups')) {
			$descriptions = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('description', 'calendar_event', array_keys($res));
			foreach ($descriptions as $assetid => $description) {
				$res[$assetid]['description'] = $description;
			}
		}

		return $res;

	}//end getWeekEvents()


	/**
	* Group the supplied event list by day
	*
	* @param array	$plain_events	The list of events to group
	* @param string	$date			The current date
	*
	* @return array
	* @access private
	*/
	function groupWeekEventsByDay($plain_events, $date)
	{
		$res = Array();
		$day_in_second = 24 * 60 * 60;	// 86400 seconds
		$days_to_show = $this->attr('week_view_show_days');

		$i = iso8601_ts($date);
		while ($i <= iso8601_ts(add_days_to_iso($date, 6))) {
			$iso_date = iso8601_date_component(ts_iso8601($i));
			if ($days_to_show[date('w', $i)]) {
				$res[$iso_date] = Array();
			}
			$i = iso8601_ts(add_days_to_iso($iso_date));
		}

		$week_start_day = $this->attr('week_starts_on');
		$first_day = iso8601_ts($date) - ((7+(date('w', iso8601_ts($date)) - $week_start_day))%7)*$day_in_second;
		$cal_date = date('Y-m-d', $first_day);

		$last_day = add_days_to_iso($cal_date, 6);
		$events =& Calendar_Common::expandEventList($plain_events, $cal_date, $last_day, $this->attr('day_starts_at'), $this->attr('day_ends_at'));

		foreach ($events as $id => $details) {
			$event_date = sprintf('%04d-%02d-%02d', $details['start_date_year'], $details['start_date_mon'], $details['start_date_mday']);
			if (isset($res[$event_date])) {
				$res[$event_date][$id] = $details;
			}
		}

		foreach ($res as $date => $events) {
			Calendar_Common::processExceptions($res[$date]);
		}

		return $res;

	}//end groupWeekEventsByDay()


	/**
	* Group the supplied event list by day and time-window
	*
	* @param array	$plain_events	The list of events to group
	* @param string	$date			The current date
	* @param string	$first_time		The date to start
	* @param int	$time_partition	The interval of time
	* @param string	$last_time		The date to end
	*
	* @return array
	* @access private
	*/
	function groupWeekEventsByDayAndTime($plain_events, $date, $first_time='', $time_partition='', $last_time='')
	{
		if (!$first_time) {
			$first_time = $this->attr('day_starts_at');
		}
		if (!$time_partition) {
			$time_partition = $this->attr('day_partition_time');
		}
		$days_to_show = $this->attr('week_view_show_days');

		if (!$last_time) {
			$last_time = $this->attr('day_ends_at');
		}
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
		foreach ($partitions as $key => $val) {
			$partitions[$key] = Array();
		}
		$partitions['all_day'] = Array();

		$res = Array();

		$week_start_day = $this->attr('week_starts_on');
		$first_day = iso8601_ts($date) - ((7+(date('w', iso8601_ts($date)) - $week_start_day))%7)*86400;
		$cal_date = date('Y-m-d', $first_day);

		$last_day = add_days_to_iso($cal_date, 6);

		$i = iso8601_ts($cal_date);
		while ($i <= iso8601_ts(add_days_to_iso($cal_date, 6))) {
			$iso_date = iso8601_date_component(ts_iso8601($i));
			if ($days_to_show[date('w', $i)]) {
				$res[$iso_date] = $partitions;
			}
			$i = iso8601_ts(add_days_to_iso($iso_date));
		}

		$plain_events = $this->groupWeekEventsByDay($plain_events, $date);
		foreach ($plain_events as $event_date => $events) {
			$res[$event_date]['_overlap_'] = $this->_countOverlappingEvents($events, $first_time, $last_time, $time_partition);
			foreach ($events as $id => $details) {
				if (isset($res[$event_date])) {
					if (is_null($details['start_date_hours']) || ($details['start_date_hours'] == '--')) {
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
			foreach ($partition_keys as $partition) {
				Calendar_Common::processExceptions($res[$date][$partition]);
			}
		}
		return $res;

	}//end groupWeekEventsByDayAndTime()


	/**
	* Get the events on the specified day
	*
	* It returns the array of (assetid => Array(...) ))
	*
	* @param string	$date	The date of the day to get events for, in ISO format (YYYY-MM-DD)
	*
	* @return array
	* @access private
	*/
	function getDayEvents($date)
	{
		$db = MatrixDAL::getDb();
		list($year,$month,$day) = sscanf($date, '%04d-%02d-%02d');

		$root_ids = $this->_getRootNodeIds();
		if (empty($root_ids)) {
			trigger_localised_error('CAL0029', translate('WARNING: No valid root nodes are set for this Calendar Page'), E_USER_WARNING);
			return Array();
		}

		// GET SINGLE EVENTS:
		$bind_vars = Array();
		$today_date_stamp = Calendar_Common::_getDayStamp($date);
		$date_sql = '(('.$today_date_stamp.' = cd.start_date_ds) OR ('.$today_date_stamp.' BETWEEN cd.start_date_ds AND cd.end_date_ds))';
		$sql = Calendar_Common::getSingleEventQueryBase($root_ids, 'calendar_event_single', TRUE, $bind_vars).' AND '.$date_sql;
		$sql .= $this->_getRootNodeRestrictionSQL();

		$single_result = Array();
		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			foreach($bind_vars as $bind_value => $bind_var) {
				MatrixDAL::bindValueToPdo($query, $bind_var,  $bind_value);
			}
			$single_result = MatrixDAL::executePdoAll($query);
		} catch (Exception $e) {
			throw new Exception($e->getMessage());
		}//end
		$single_result = Calendar_Common::condenseResultTreeids($single_result);

		// GET RECURRING EVENTS:
		$bind_vars = Array();
		$sql = Calendar_Common::getRecurringEventQueryBase($root_ids, 'day', $date, NULL, 'calendar_event_recurring', $bind_vars);
		$sql .= $this->_getRootNodeRestrictionSQL();

		$recur_result = Array();
		 try {
			$query = MatrixDAL::preparePdoQuery($sql);
			foreach($bind_vars as $bind_value => $bind_var) {
				MatrixDAL::bindValueToPdo($query, $bind_var,  $bind_value);
			}
			$recur_result = MatrixDAL::executePdoAll($query);
		} catch (Exception $e) {
			throw new Exception($e->getMessage());
		}//end
		$recur_result = Calendar_Common::condenseResultTreeids($recur_result);

		// PUT IT ALL TOGETHER:
		$res = $single_result + $recur_result;
		if ($this->attr('enable_description_popups')) {
			$descriptions = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('description', 'calendar_event', array_keys($res));
			foreach ($descriptions as $assetid => $description) {
				$res[$assetid]['description'] = $description;
			}
		}

		$res =& Calendar_Common::expandEventList($res, $date, $date, $this->attr('day_starts_at'), $this->attr('day_ends_at'));
		Calendar_Common::processExceptions($res);

		 uasort($res, Array('Calendar_Common', 'compareStartDates'));

		return $res;

	}//end getDayEvents()


	/**
	* Group events by time
	*
	* @param array	$events			The list of events to group
	* @param string	$date			The current date
	* @param string	$first_time		The date to start
	* @param int	$time_partition	The interval of time
	* @param string	$last_time		The date to end
	*
	* @return array
	* @access private
	*/
	function groupDayEventsByTime($events, $date, $first_time='', $time_partition='', $last_time='')
	{
		if (!$first_time) {
			$first_time = $this->attr('day_starts_at');
		}
		if (!$time_partition) {
			$time_partition = $this->attr('day_partition_time');
		}
		if (!$last_time) {
			$last_time = $this->attr('day_ends_at');
		}

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
		foreach ($partitions as $key => $val) {
			$partitions[$key] = Array();
		}
		$partitions['all_day'] = Array();

		$res = $partitions;

		foreach ($events as $id => $details) {
			$event_date = sprintf('%04d-%02d-%02d', $details['start_date_year'], $details['start_date_mon'], $details['start_date_mday']);
			if (($event_date == $date)) {
				if (is_null($details['start_date_hours']) || $details['start_date_hours'] == '--') {
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

	}//end groupDayEventsByTime()


	/**
	* Group events by root node and time
	*
	* @param array	$events			The list of events to group
	* @param string	$first_time		The date to start
	* @param int	$time_partition	The interval of time
	* @param string	$last_time		The date to end
	* @param string	$root_nodes		The root nodes of the event folder
	*
	* @return array
	* @access private
	*/
	function groupDayEventsByRootNodeAndTime($events, $first_time='', $time_partition='', $last_time='', $root_nodes='')
	{
		if (!$first_time) {
			$first_time = $this->attr('day_starts_at');
		}
		if (!$time_partition) {
			$time_partition = $this->attr('day_partition_time');
		}
		if (!$last_time) {
			$last_time = $this->attr('day_ends_at');
		}
		if (!$root_nodes) {
			$root_nodes = array_keys($this->attr('root_nodes'));
		}
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
		foreach ($partitions as $key => $val) {
			$partitions[$key] = Array();
		}
		$partitions['all_day'] = Array();

		Calendar_Common::processExceptions($events);
		$res = Array();

		$columnise_by_root = $this->attr('columnise_day_view_by_root_node');
		foreach ($columnise_by_root as $node_id) {
			if (($node_id == '*') || in_array($node_id, $root_nodes)) {
				$res[$node_id] = $partitions;
			}
		}
		$column_treeids = $GLOBALS['SQ_SYSTEM']->am->getAssetTreeids(array_diff($columnise_by_root, Array('*')));

		foreach ($events as $id => $details) {
			$event_date = sprintf('%04d-%02d-%02d', $details['start_date_year'], $details['start_date_mon'], $details['start_date_mday']);

			$cols = Array();
			foreach ($column_treeids as $column_id => $this_column_treeids) {
				foreach ($this_column_treeids[0] as $column_treeid) {
					foreach ($details['treeid'] as $event_treeid) {
						if (0 === strpos($event_treeid, $column_treeid)) {
							$cols[] = $column_id;
						}
					}
				}
			}
			if (empty($cols) && isset($res['*'])) {
				$cols[] = '*';
			}
			foreach ($cols as $col) {
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
		}//end foreach event

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

	}//end groupDayEventsByRootNodeAndTime()


	/**
	* Get SQL to add to queries to implement the root node restrictions
	*
	* @return string
	* @access private
	*/
	function _getRootNodeRestrictionSQL()
	{
		$restrictive_roots = $this->_getRestrictiveRootNodeIds();
		if (!empty($restrictive_roots)) {
			$db = MatrixDal::getDb();
			$root_treeids = $GLOBALS['SQ_SYSTEM']->am->getAssetTreeids($restrictive_roots);
			$treeid_options = Array();
			foreach ($root_treeids as $rootid => $this_root_treeids) {
				// bug fix : #3363  Event Personalisation on Calendar Page does not show events
				$treeid_options[] = 'salt.treeid LIKE '.MatrixDAL::quote(reset(reset($this_root_treeids)).'%');
			}
			$sql = '
					AND a.assetid IN (
						SELECT minorid
						FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk sal
							JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree salt ON (sal.linkid = salt.linkid)
						WHERE ('.implode(' OR ', $treeid_options).')
					)';
			return $sql;
		}
		return '';

	}//end _getRootNodeRestrictionSQL()


	/**
	* Get any root node restrictions that should be applied at the second level (after columnisation and styling)
	*
	* @return array
	* @access private
	*/
	function _getRestrictiveRootNodeIds()
	{
		// Personalisation is one option
		if ($this->attr('personalised')) {
			return Array($GLOBALS['SQ_SYSTEM']->user->id);
		}

		// Otherwise we might have a dynamic parameter
		$parameter_map = $this->getAttribute('parameter_map');
		$raw_dynamic_root_nodes = $parameter_map->getParameterValue('replacement_root_node');
		if (!empty($raw_dynamic_root_nodes)) {
			$dynamic_root_nodes = Array();
			// note that dynamic roots can be supplied as an array of asset ids or
			// as a comma delimited string of asset ids
			if (!is_array($raw_dynamic_root_nodes)) {
				$raw_dynamic_root_nodes = explode(',', $raw_dynamic_root_nodes);
			}

			$original_root_ids = array_keys($this->attr('root_nodes'));
			foreach ($raw_dynamic_root_nodes as $dynamic_root) {
				if (!strlen($dynamic_root)) continue;
				// Check to see if the dynamic parameter specified is a child of one of the root
				// nodes. If so, use the dynamic parameter as the root node; otherwise return an error.
				$dynamic_parents = $GLOBALS['SQ_SYSTEM']->am->getParents($dynamic_root);
				$matching_root_ids = array_intersect(array_keys($dynamic_parents), $original_root_ids);
				if (empty($matching_root_ids)) {
					trigger_localised_error('CAL0067', sprintf(translate('Supplied dynamic root node (Id: #%1$s) is in an unauthorised part of the system.  Defaulting to static root node. See Asset (Id: #%2$s)'), $dynamic_root, $this->id), E_USER_WARNING);
				} else {
					$dynamic_root_nodes[] = $dynamic_root;
				}
			}

			if (!empty($dynamic_root_nodes)) {
				return $dynamic_root_nodes;
			}

		}//end if
		return Array();

	}//end _getRestrictiveRootNodeIds()


	/**
	* Formats the limit and current dates for comparison and returns them in Array($limit_date,$compared_date)
	*
	* @param string		$view			the current view
	* #param string		$date			date to compare (next or prev)
	* @param string		$direction		the direction - forward or backward
	* @param string		$operator		+ or - 	*
	*
	* @return Array
	* @access public
	*/
	function formatDatesToCompare($view,$date,$direction, $operator)
	{
		if ($view == 'event' || $view == 'edit') {
			return Array('','');
		}

		$navi_limit = $this->attr($view.'_navi_limit');
		$curr_date = date('Y-m-d');

		// return true if no limit specified
		if ($navi_limit[$direction] == '') {
			return Array('','');
		}

		// depepnding on current view, we set limit date and date to compare
		switch ($view) {
			case 'year':
				$limit_date = date('Y',strtotime($operator.$navi_limit[$direction].' year',strtotime($curr_date)));
				$compared_date = date('Y',strtotime($date));
			break;
			case 'month':
				$limit_date = date('Y-m',strtotime($operator.$navi_limit[$direction].' month',strtotime($curr_date)));
				$compared_date = date('Y-m',strtotime($date));
			break;
			case 'week':
				$offset = 0;
				$day_in_seconds = 24 * 60 * 60;
				$week_start_index	= $this->attr('week_starts_on');
				$curr_day_index		= array_search(date('l', iso8601_ts($curr_date)), $this->day_names);
				if ($week_start_index != $curr_day_index) {
					if ($week_start_index < $curr_day_index) {
						$offset = $curr_day_index - $week_start_index;
					} else {
						$offset = $curr_day_index + (7 - $week_start_index);
					}
				}
				$limit_date = date('Y-m-d',strtotime($operator.$navi_limit[$direction].' week',strtotime($curr_date)));
				$limit_date = strtotime($limit_date) - ($offset * $day_in_seconds);
				$limit_date = date('Y-m-d',$limit_date);

				$offset = 0;
				$compared_day_index	= array_search(date('l', iso8601_ts($date)), $this->day_names);
				if ($week_start_index != $compared_day_index) {
					if ($week_start_index < $compared_day_index) {
						$offset = $compared_day_index - $week_start_index;
					} else {
						$offset = $compared_day_index + (7 - $week_start_index);
					}
				}
				$compared_date = strtotime($date) - ($offset * $day_in_seconds);
				$compared_date = date('Y-m-d',$compared_date);
			break;
			case 'day':
				$limit_date = date('Y-m-d',strtotime($operator.$navi_limit[$direction].' day',strtotime($curr_date)));
				$compared_date = $date;
			break;
		}

		return Array($limit_date,$compared_date);

	}//end formatDatesToCompare()


	/**
	* Create a "No Results" bodycopy under this asset
	*
	* @return boolean
	* @access public
	*/
	function createNoResultsBodycopy()
	{
		$bodycopy_name = "page_contents_no_results";

		// Check if "No result page" already exists
		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, $bodycopy_name);
		if (!empty($bodycopy_link)) {
			return TRUE;
		}

		// Being here means bodycopy doesn't exist, so create it
		$copy_link = Array(
						'asset' 	=> &$this,
						'link_type' => SQ_LINK_TYPE_2,
						'is_dependant' => 1,
						'is_exclusive' => 1,
						'value' => $bodycopy_name,
					);

		$page_contents_data = Array(
								'content' => '<h2 style="text-align: center">%calendar_title%</h2><p style="text-align: center">%prev_link%&nbsp;&nbsp;&nbsp;&nbsp;%up_link%&nbsp;&nbsp;&nbsp;&nbsp;%next_link%</p><p>No events</p>',
						);

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$bodycopy = new Bodycopy();
		$bodycopy->setAttrValue('name', "Page Contents (No Results)");

		if (!$bodycopy->create($copy_link, $page_contents_data)) {
			trigger_localised_error('CAL0062', translate('Could not link the contents to the no results bodycopy'), E_USER_WARNING);
			return FALSE;
		}

		return TRUE;

	}//end createNoResultsBodycopy()


	/**
	* Get the replacement for "root_nodes" keyword
	*
	* @return string
	* @access public
	*/
	function getRootNodesKeywordReplacement()
	{
		$root_nodes = array_keys($this->attr('root_nodes'));

		return implode(', ',$root_nodes);

	}//end getRootNodesKeywordReplacement()


}//end class

?>
