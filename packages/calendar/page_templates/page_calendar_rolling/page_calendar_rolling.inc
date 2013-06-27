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
* $Id: page_calendar_rolling.inc,v 1.44 2013/06/27 01:37:50 akarelia Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once SQ_FUDGE_PATH.'/general/datetime.inc';
require_once SQ_PACKAGES_PATH.'/calendar/lib/calendar_common.inc';

/**
* Page_Calendar_Rolling
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @author  Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.44 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Page_Calendar_Rolling extends Page
{


	/**
	* An array of bodycopies and their names that are attached underneath this page asset
	* @var array
	*/
	var $bodycopies = Array(
						''			=> Array(
										'name'		=> 'Page Contents',
										'content'	=> Array(
														'content'	=> '%event_list%',
													   ),
									   ),
						'time_unit'	=> Array(
										'name'		=> 'Time Unit Format',
										'content'	=> Array(
														'content'	=> "<h3>%time_unit_start_datetime_Y-m-d%</h3>\n%time_unit_event_list%",
													   ),
									   ),
					  );


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);
	}//end constructor


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* For example, creating other child assets should be performed in this method
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		$am =& $GLOBALS['SQ_SYSTEM']->am;

		$link = Array(
					'asset'			=> &$this,
					'value'			=> 'type_formats',
					'link_type'		=> SQ_LINK_TYPE_2,
					'is_dependant'	=> 1,
					'is_exclusive'	=> 1,
				);

		$am->includeAsset('folder');
		$am->includeAsset('bodycopy');

		// create a type formats folder
		$type_formats_folder = new Folder();
		$type_formats_folder->setAttrValue('name', 'Type Formats');
		if (!$type_formats_folder->create($link)) return FALSE;

		// create type format bodycopies
		$tf_link = Array('asset' => &$type_formats_folder, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 0);
		foreach (Array('calendar_event_single', 'calendar_event_recurring') as $event_type) {
			$tf_link['value'] = $event_type;
			$content          = Array(
									'content'	=> '%asset_name% (%event_start_date%)',
								);
			$format_bc = new Bodycopy();
			$format_bc->setAttrValue('name', $am->getTypeInfo($event_type, 'name').' Format');
			if (!$format_bc->create($tf_link, $content)) {
				return FALSE;
			}
			$format_bc = NULL;
		}

		// create the page contents and time_unit bodycopies
		foreach ($this->bodycopies as $value => $details) {
			$copy_link = Array(
							'asset'			=> &$this,
							'link_type'		=> SQ_LINK_TYPE_2,
							'is_dependant'	=> 1,
							'is_exclusive'	=> 1,
							'value'			=> $value,
						 );

			$bodycopy = new Bodycopy();
			$bodycopy->setAttrValue('name', array_get_index($details, 'name'));
			if (!$bodycopy->create($copy_link, array_get_index($details, 'content'))) {
				return FALSE;
			}
		}

		return TRUE;

	}//end _createAdditional()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_TYPE_2]['bodycopy'] = Array('card' => 3, 'exclusive' => FALSE);
		$page_links[SQ_LINK_TYPE_3]['bodycopy'] = Array('card' => 1, 'exclusive' => FALSE);
		return $page_links;

	}//end _getAllowedLinks()


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
		if (!isset($vars['keywords'])) {
			$vars['keywords'] = Array();
		}

		$am =& $GLOBALS['SQ_SYSTEM']->am;

		$links               = $am->getLinks($this->id, SQ_LINK_TYPE_2, Array('bodycopy', 'folder'), TRUE, 'major', NULL, TRUE);
		$broadcaster_parents = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id);

		// check if the broadcaster is in the type formats folder
		$tf_folder = $this->getFolder('type_formats');
		if (isset($broadcaster_parents[$tf_folder->id])) {
			$tf_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($tf_folder->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'major');
			$tf_bcs = Array();
			foreach ($tf_links as $link) {
				$tf_bcs[$link['minorid']] = $link['value'];
			}
			$bc_id = reset(array_intersect(array_keys($tf_bcs), array_keys($broadcaster_parents)));
			if ($bc_id) {
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($tf_bcs[$bc_id]);
				$dummy_asset = new $tf_bcs[$bc_id];
				$asset_keywords = $dummy_asset->getAvailableKeywords();
				foreach ($asset_keywords as $kw => $desc) {
					$vars['keywords'][$kw] = $desc;
				}
				return TRUE;
			}
			return FALSE;
		}


		// get the bodycopy parent of the broadcaster
		$format_bodycopy         = FALSE;
		$format_bodycopies_links = $am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE);
		foreach ($format_bodycopies_links as $link) {
			if (isset($broadcaster_parents[$link['minorid']])) {
				$format_bodycopy = $link['value'];
			}
		}

		switch ($format_bodycopy) {
			case '':
				$vars['keywords']['event_list'] = translate('cal_rolling_keyword_event_list');
				$vars['keywords']['root_nodes'] = translate('root_node');
				return TRUE;
			break;
			case 'time_unit':
				$vars['keywords']['time_unit_event_list'] = translate('cal_rolling_keyword_time_unit_event_list', $this->attr('time_unit'));
				$vars['keywords']['time_unit_start_datetime_xxxx'] = translate('cal_rolling_keyword_time_unit_start_datetime_xxxx');
				$vars['keywords']['time_unit_end_datetime_xxxx'] = translate('cal_rolling_keyword_time_unit_end_datetime_xxxx');
				return TRUE;
			break;
		}

		return FALSE;

	}//end onRequestKeywords()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		$cache_key = $this->_getCacheKey();
		$cached_contents = '';

		if (!empty($cache_key)) {
			$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
			$cached_contents = $cm->loadFromCache($this->id, $this->type(), $cache_key);
		}

		if ($cached_contents !== FALSE) {
			echo $cached_contents;
		} else {
			// No cached version, so carry out the calculations
			$am =& $GLOBALS['SQ_SYSTEM']->am;

			$page_links = $am->getLinks($this->id, SQ_LINK_TYPE_2, Array('folder', 'bodycopy'), TRUE, 'major', NULL, TRUE);
			$tf_folder_id = NULL;
			$pb_bodycopy_id = NULL;

			foreach ($page_links as $link) {
				switch ($link['minor_type_code']) {
					case 'folder':
						$this->_tmp['tf_folder_id'] = $link['minorid'];
					break;
					case 'bodycopy':
						if ($link['value'] == '') {
							$pb_bodycopy_id = $link['minorid'];
						}
					break;
				}
			}
			if (is_null($this->_tmp['tf_folder_id'])) {
				trigger_localised_error('CAL0059', E_USER_WARNING, $this->id);
				return;
			}
			if (is_null($pb_bodycopy_id)) {
				trigger_localised_error('CAL0060', E_USER_WARNING, $this->id);
				return;
			}

			ob_start();
				$pb = $am->getAsset($pb_bodycopy_id, 'bodycopy');
				$replacements = $this->_getPageContentsReplacements();
				$calendar_replacements = $replacements;
				$keywords = $pb->getKeywords();
				foreach ($keywords as $word) {
					if (isset($calendar_replacements[$word]) && !empty($calendar_replacements[$word])) {
						$replacements[$word] = $calendar_replacements[$word];
					} else {
						$replacements[$word] = $this->getKeywordReplacement($word);
					}
				}
				$pb->setKeywordReplacements($replacements);
				$pb->printBody();

				if (!empty($cache_key)) {
					$cm->saveToCache($this->id, $this->type(), $cache_key, ob_get_contents());
				}
			ob_end_flush();
		}//end else

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
	* Work out the unique cache key to represent the current page
	*
	* @return string
	* @access private
	*/
	function _getCacheKey()
	{
		$cache_key = '';
		$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();

		// Hmmm, page_calendar_restricted call this function too.
		// So must make sure, we can tell the difference between the 2 entities
		if ($this->type() == 'page_calendar_rolling') {
			$from_date = $this->attr('duration_from');
			$to_date = $this->attr('duration_to');
			$day_filter = $this->attr('day_filter');
			$time_unit = $this->attr('time_unit');

			// All attributes will have some contents of no need to check for empty
			$cache_key .= $from_date; 			// First entry so no colon out front like a little soldier
			$cache_key .= ':'.$to_date;
			$cache_key .= ':'.$time_unit;
			$cache_key .= ':'.$day_filter;
		} else if ($this->type() == 'page_calendar_restricted') {
			$from_date = $this->attr('start_date');
			$to_date = $this->attr('end_date');

			$cache_key .= $from_date; 			// First entry so no colon out front like a little soldier

			// If the end date is set use it in the cache key
			$end_date = substr($to_date, 0, 10);
			if (!($end_date == '----------')) {
				$cache_key .= ':'.$to_date;
			}
		}

		if ($cache_key !== '') {
			$cache_key .= '-ctx'.$contextid;
		}

		return $cache_key;

	}//end _getCacheKey()


	/**
	* Get the keyword replacements to use in the page contents bodycopy
	*
	* @return array
	* @access protected
	*/
	function _getPageContentsReplacements()
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		// prepare keywords for the type format replacement we're going to perform later
		$type_format_links = $am->getLinks($this->_tmp['tf_folder_id'], SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'major', NULL, TRUE);
		$type_formats = Array();
		$needs_asset = Array();
		$somebody_doesnt_need_asset = FALSE;
		$metadata_keywords = Array();
		$asset_info_fields =  Array(
								'assetid',
								'type_code',
								'version',
								'name',
								'short_name',
								'status',
								'languages',
								'charset',
								'force_secure',
								'created',
								'created_userid',
								'updated',
								'updated_userid',
								'published',
								'published_userid',
							  );
		foreach ($type_format_links as $link) {
			if (!empty($link['value'])) {
				$needs_asset[$link['value']] = FALSE;
				$metadata_keywords[$link['value']] = Array();
				$bc = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid']);
				$type_formats[$link['value']] = $bc->getRawBodycopyContent();
				$required_keywords[$link['value']] = $bc->getKeywords();
				foreach ($required_keywords[$link['value']] as $keyword) {
					if (strpos($keyword, 'asset_') !== 0) {
						$needs_asset[$link['value']] = TRUE;
					} else {
						// the keyword starts with 'asset_' but it might still require the asset
						if ((strpos($keyword, 'asset_metadata_') !== 0) && (!in_array(substr($keyword, 6), $asset_info_fields))) {
							$needs_asset[$link['value']] = TRUE;
						}
					}
					if (strpos($keyword, 'asset_metadata_') === 0) {
						$metadata_keywords[$link['value']][] = $keyword;
					}
				}
				if (!$needs_asset[$link['value']]) {
					$somebody_doesnt_need_asset = TRUE;
				}
			}
		}

		// get the events to display
		$time = time();
		$all_events = $this->_getEvents(NULL, NULL, $time);

		$event_list = '';
		if (!empty($all_events)) {
			$all_ids = Array();
			foreach ($all_events as $day => $day_event_list) {
				$all_ids = array_merge($all_ids, array_keys($day_event_list));
			}
			if ($somebody_doesnt_need_asset) {
				$asset_infos = $am->getAssetInfo($all_ids);
			}

			// break up events into months, weeks, or days
			$events_by_time_unit = Array();
			$start_date = reset(array_keys($all_events));
			$start_time = iso8601_ts($start_date);
			switch ($this->attr('time_unit')) {
				case 'months':
					$current_year  = date('Y', $time);
					$current_month = date('m', $time);

					foreach ($all_events as $event_date => $events) {
						$event_time  = iso8601_ts($event_date);
						$event_year  = date('Y', $event_time);
						$event_month = date('m', $event_time);

						$difference = ($event_month - $current_month) + (($event_year - $current_year) * 12);
						$events_by_time_unit[$difference][$event_date] = $events;
					}
				break;

				case 'weeks':
					// assumes week starts on sunday; further development required in order to have weeks start on a different day
					$current_year  = date('Y', $time);

					// hack required - php 'W' assumes week starts on monday, 'w' assumes week starts on sunday
					// "correct" so that weeks start on sunday
					$current_week  = date('W', strtotime(date('Y-m-d', $time).' +1 day'));

					foreach ($all_events as $event_date => $events) {
						$event_time  = iso8601_ts($event_date);
						$event_year  = date('Y', $event_time);

						// hack required - same as above
						$event_week  = date('W', strtotime(date('Y-m-d', $event_time).' +1 day'));

						$difference = ($event_week - $current_week) + (($event_year - $current_year) * 52);
						$events_by_time_unit[$difference][$event_date] = $events;
					}
				break;

				case 'days':
				default:
					foreach ($all_events as $event_date => $events) {
						$difference = days_between_isos($event_date, date('Y-m-d', $time));
						$events_by_time_unit[$difference][$event_date] = $events;
					}
				break;
			}//end switch ($this->attr('time_unit'))


			$unit_bodycopy_link = $am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'time_unit');
			if (empty($unit_bodycopy_link)) return FALSE;

			$unit_bodycopy = $am->getAsset($unit_bodycopy_link['minorid'], $unit_bodycopy_link['minor_type_code']);
			if (is_null($unit_bodycopy)) return FALSE;

			$single_event_descendants = $am->getTypeDescendants('calendar_event_single', TRUE);
			$recurring_event_descendants = $am->getTypeDescendants('calendar_event_recurring', TRUE);
			$mm = NULL;

			// Store the date supplied in the URL (if supplied) so it can be restored after use
			$orig_date_param = '';
			if (isset($_REQUEST['SQ_CALENDAR_DATE'])) {
				$orig_date_param = $_REQUEST['SQ_CALENDAR_DATE'];
			}

			foreach ($events_by_time_unit as $unit_count => $events_by_date) {

				// get all the events for this time_unit (day/week/month), stuff them into $event_list
				$unit_contents = '';
				foreach ($events_by_date as $date => $events) {
					$_REQUEST['SQ_CALENDAR_DATE'] = $date;

					// loop through the events for this date
					foreach ($events as $id => $event) {

						$format_type = $event['type_code'];

						// Firstly, check whether a specific asset Display Format has been defined.
						// If not, we should default to its parent type
						if (!isset($type_formats[$format_type])) {
							if (in_array($format_type, $single_event_descendants)) {
								$format_type = 'calendar_event_single';
							} else if (in_array($format_type, $recurring_event_descendants) || $format_type == 'calendar_event_modification') {
								$format_type = 'calendar_event_recurring';
							}
						}

						// Now if a Display Format is not set for either the asset type or its parent type, then
						// we can't print this event
						if (!isset($type_formats[$format_type])) continue;

						// get asset keywords
						if ($needs_asset[$format_type]) {
							$asset = $am->getAsset($id);
							foreach ($required_keywords[$format_type] as $keyword) {
								if ($keyword == 'asset_contents') {
									ob_start();
										$asset->printBody();
										$replacements[$keyword] = ob_get_contents();
									ob_end_clean();
								} else {
									$replacements[$keyword] = $asset->getKeywordReplacement($keyword);
								}
							}
						} else {
							// we can do this without the asset
							foreach ($asset_infos[$id] as $field => $value) {
								$replacements['asset_'.$field] = $value;
							}
							$replacements['asset_assetid'] = $id;
						}

						// get metadata keywords
						if (!empty($metadata_keywords[$event['type_code']])) {
							if (is_null($mm)) {
								$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
							}
							// get all the metadata keywords for this asset
							$metadata_values = $mm->getMetadataFieldValues($id, array_values($metadata_keywords[$event['type_code']]));

							foreach ($metadata_values as $field => $value) {
								$replacements['asset_metadata_'.$field] = $value;
							}
						}

						$type_format    = $type_formats[$format_type];
						$unit_contents .= replace_keywords($type_format, $replacements);

					}//end foreach events in day
				}//end foreach ($events_by_date)


				$unit_keywords = $unit_bodycopy->getKeywords();
				foreach ($unit_keywords as $keyword) {
					switch ($keyword) {
						case 'time_unit_event_list':
							$replacements[$keyword] = $unit_contents;
						break;

						default:
							// if the keyword at this point starts with event_start_datetime_ or event_end_datetime_
							// it means this is a dynamic keyword to display a date
							preg_match('/^time_unit_(start|end)_datetime_(.+)/', $keyword, $matches);
							if ($matches != NULL) {
								$start_end   = $matches[1];
								$date_format = $matches[2];

								$time_unit              = $this->attr('time_unit');
								// construct a string in the format "YEAR/MONTH/DATE +UNIT_COUNT TIME_UNITs", and pass it to strtotime() to calculate
								$start_end_dates        = $this->_getStartEndDates($time_unit, $time, $unit_count);
								$replacements[$keyword] = date($date_format, strtotime($start_end_dates[$start_end]));
							}
						break;
					}//end switch ($keyword)
				}//end foreach ($unit_keywords)

				ob_start();
					// print the contents of page - replacing the global keywords
					$unit_bodycopy->setKeywordReplacements($replacements);
					$unit_bodycopy->printBody();
				$event_list .= ob_get_clean();

			}//end foreach ($events_by_time_unit)

			// Restore supplied date value
			if ($orig_date_param != '') {
				$_REQUEST['SQ_CALENDAR_DATE'] = $orig_date_param;
			} else {
				unset($_REQUEST['SQ_CALENDAR_DATE']);
			}
		} else {
			$no_result_link = $am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'no_results');
			// no bodycopy link found? return false
			if (empty($no_result_link)) return FALSE;

			$no_result_bc   = $am->getAsset($no_result_link['minorid'], $no_result_link['minor_type_code']);
			// no bodycopy found? return false
			if (is_null($no_result_bc)) return FALSE;

			$keywords = $no_result_bc->getKeywords();

			$replacements = Array();
			// don't worry about our keyword modifiers here our parent
			// function getKeywordReplacement is doing that for us
			foreach ($keywords as $keyword) {
				$replacements[$keyword] = $this->getKeywordReplacement($keyword);
			}

			$no_result_bc->setKeywordReplacements($replacements);
			ob_start();
				$no_result_bc->printBody();
			$event_list = ob_get_clean();

		}//end !empty($all_events)

		return Array('event_list' => $event_list);

	}//end _getPageContentsReplacements()


	/**
	* Gets the start and end dates by time unit (eg. months) for a passed time
	*
	* (eg. pass in 'months' and strtotime('2006-08-10'), and this function will return '2006-08-01' and '2006-08-30')
	*
	* @param string	$time_unit	either 'days', 'weeks', or 'months'
	* @param int	$time		the time to use as a point of reference, in unix timestamp
	* @param int	$modifier	modifies the time so that it's shifted $modifier days/weeks/months in the future/past
	*
	* @return mixed array|FALSE
	* @access private
	*/
	function _getStartEndDates($time_unit, $time, $modifier=0)
	{
		if (!is_int($time)) return FALSE;

		// use the modifier, calculate the new date
		$time = strtotime(date('Y-m-d', $time).' '.($modifier < 0 ? $modifier : '+'.$modifier).' '.$time_unit);

		// check for day/week/month, bail out if other
		preg_match('/(day|week|month)([s]?)/', $time_unit, $matches);
		if (empty($matches)) return FALSE;
		$unit = array_get_index($matches, 1);

		// calculate the start and end times for the time unit
		switch ($unit) {
			case 'month':
				// construct a date that looks like:  YEAR/MONTH/01 00:00.00
				$tmp_start = mktime(0, 0, 0, date('m', $time), 1, date('y', $time));
				$start     = date('Y-m-d', $tmp_start);
				$end       = date('Y-m-d', strtotime($start.' +1 month -1 day'));
			break;

			case 'week':
				// get current day, diff to monday/sunday
				$day_of_week = date('w', $time);
				$start       = date('Y-m-d', strtotime(date('Y-m-d', $time).' -'.$day_of_week.' days'));
				$end         = date('Y-m-d', strtotime($start.' +6 days'));
			break;

			case 'day':
			default:
				// just snip off anything beyond the day
				$start     = date('Y-m-d', $time);
				$end       = $start;
			break;
		}

		return Array(
				'start'	=> $start,
				'end'	=> $end,
			   );

	}//end _getStartEndDates()


	/**
	* Get the events that should be displayed in this calendar page
	* Returns: (date => array (assetid => array ( details )))
	*
	* @param string	$start_date	Start date
	* @param string	$end_date	End date
	* @param int	$time		The time to use as a point of reference when calculating start_date and end_date - if the latter two variables are set, $time goes unused
	*
	* @return array
	* @access protected
	*/
	function _getEvents($start_date=NULL, $end_date=NULL, $time=NULL)
	{
		$db = MatrixDAL::getDb();
		if (is_null($time)) $time = time();
		if (is_null($start_date)) {
			$unit = $this->attr('time_unit');
			$from = $this->attr('duration_from');
			$to   = $this->attr('duration_to');

			// switch if the values are the wrong way around
			if ($from > $to) {
				$tmp  = $from;
				$from = $to;
				$to   = $tmp;
			}

			$from_dates = $this->_getStartEndDates($unit, $time, $from);
			$to_dates   = $this->_getStartEndDates($unit, $time, $to);

			$start_date = $from_dates['start'];
			$end_date   = $to_dates['end'];
		}
		$length = round((iso8601_ts($end_date) - iso8601_ts($start_date)) / (60 * 60 * 24));

		// build sql to restrict the tree locations
		$root_nodes = $this->attr('root_nodes');
		if (empty($root_nodes)) {
			trigger_localised_error('CAL0028', E_USER_WARNING);
			return Array();
		}

		// get the single events
		$bind_vars = Array();
		$start_date_stamp = Calendar_Common::_getDayStamp($start_date);
		$end_date_stamp = $start_date_stamp + $length;
		$date_sql = '(cd.start_date_ds BETWEEN :start_date_ds AND :end_date_ds) OR (cd.start_date_ds < :start_date_ds_1 AND cd.end_date_ds >= :start_date_ds_2)';
		$sql = Calendar_Common::getSingleEventQueryBase(array_keys($root_nodes), 'calendar_event_single', TRUE, $bind_vars).' AND ('.$date_sql.') ORDER BY cd.start_date_ts';

		$single_result = Array();
		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			foreach($bind_vars as $bind_value => $bind_var) {
				MatrixDAL::bindValueToPdo($query, $bind_var,  $bind_value);
			}			
			MatrixDAL::bindValueToPdO($query, 'start_date_ds',   $start_date_stamp, PDO::PARAM_INT);
			MatrixDAL::bindValueToPdO($query, 'start_date_ds_1', $start_date_stamp, PDO::PARAM_INT);
			MatrixDAL::bindValueToPdO($query, 'start_date_ds_2', $start_date_stamp, PDO::PARAM_INT);
			MatrixDAL::bindValueToPdO($query, 'end_date_ds',     $end_date_stamp,   PDO::PARAM_INT);
			$single_result = MatrixDAL::executePdoAll($query);
		} catch (Exception $e) {
			throw new Exception($e->getMessage());
		}//end

		$single_result = Calendar_Common::condenseResultTreeids($single_result);

		// get the recurring events
		$bind_vars = Array();
		$sql = Calendar_Common::getRecurringEventQueryBase(array_keys($root_nodes), 'period', $start_date, (int) $length, 'calendar_event_recurring', $bind_vars);

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

		$all_events = $single_result + $recur_result;
		$all_events = Calendar_Common::expandEventList($all_events, $start_date, $end_date);
		Calendar_Common::processExceptions($all_events);

		$res = Array();
		if (!$this->isNoResultsBodycopyEnabled() || !empty($all_events)) {
			$index = $start_date;
			$res[$index] = Array();
			while ($index < $end_date) {
				$index = date('Y-m-d', strtotime($index.' +1 day'));
				$res[$index] = Array();
			}

			foreach ($all_events as $id => $event) {
				$event_date = sprintf('%04d-%02d-%02d', $event['start_date_year'], $event['start_date_mon'], $event['start_date_mday']);
				$res[$event_date][strtok($id, ':')] = $event;
			}

			// finally, filter by weekdays
			$day_filter = explode('|', $this->attr('day_filter'));
			foreach ($res as $day => $day_events) {
				if (!in_array(date('D', strtotime($day)), $day_filter)) {
					unset($res[$day]);
					continue;
				}
				uasort($res[$day], Array('Calendar_Common', 'compareStartDates'));
			}
		}

		return $res;

	}//end _getEvents()


	/**
	* Get a list of all formats of a certain type
	*
	* The results of this function are cached for the length of the script execution
	*
	* @param string	$type	the value on the link for the formats folder
	*
	* @return array
	* @access public
	*/
	function getFormats($type='type_formats')
	{
		if (!isset($this->_tmp['formats'][$type])) {
			$folder = $this->getFolder($type);
			$format_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($folder->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE);
			if (empty($format_links)) return Array();

			$formats = Array();
			foreach ($format_links as $link_data) {
				$formats[$link_data['minorid']] = $link_data['value'];
			}
			$this->_tmp['formats'][$type] = $formats;
		}
		return $this->_tmp['formats'][$type];

	}//end getFormats()


	/**
	* Return a reference to a folder linked to this asset determined by the passed in value on the link
	*
	* @param string	$type	the value on the link for this folder
	*
	* @return object
	* @access public
	*/
	function &getFolder($type='type_formats')
	{
		$null = NULL;
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, $type);
		if (empty($link)) return $null;

		$folder = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		if (is_null($folder)) return $null;

		return $folder;

	}//end getFolder()


//--		NO RESULTS BODYCOPY		   --//


	/**
	* Creates the no-results bodycopy
	*
	* @param boolean	$enable_on_create	if TRUE, will set the bodycopy to be enabled
	*										(TYPE_2 link) when it is created. If FALSE,
	*										creates it disabled (TYPE_3 link).
	*
	* @return boolean
	* @access public
	*/
	function createNoResultsBodycopy($enable_on_create=TRUE)
	{
		$bodycopy_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3, 'bodycopy', FALSE, 'major', 'no_results');
		$bodycopy_link = reset($bodycopy_links);

		// we already have a bodycopy link?!
		if ($bodycopy_link) {
			return FALSE;
		} else {
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');

			$link_type = ($enable_on_create ? SQ_LINK_TYPE_2 : SQ_LINK_TYPE_3);

			$asset = new Bodycopy();
			$copy_link = Array(
							'asset'			=> &$this,
							'value'			=> 'no_results',
							'link_type'		=> $link_type,
							'is_dependant'	=> 1,
							'is_exclusive'	=> 1,
						 );

			$asset->setAttrValue('name', 'Page Contents (No Results)');
			$args = Array('content' => 'No events found');
			if (!$asset->create($copy_link, $args)) return FALSE;

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			unset($asset);
		}

		return TRUE;

	}//end createNoResultsBodycopy()


	/**
	* Returns the no-results bodycopy
	*
	* @param boolean	$only_if_enabled	if TRUE, will return NULL if the bodycopy is
	*										disabled. If FALSE, will only return NULL if
	*										never created.
	*
	* @return mixed object|NULL
	* @access public
	*/
	function &getNoResultsBodycopy($only_if_enabled=TRUE)
	{
		$asset = NULL;

		if ($only_if_enabled) {
			$link_types = SQ_LINK_TYPE_2;
		} else {
			$link_types = SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3;
		}

		$tmp_bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, $link_types, 'bodycopy', FALSE, 'major', 'no_results');
		$bodycopy_link = reset($tmp_bodycopy_link);

		if ($bodycopy_link) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link['minorid'], 'bodycopy');
		}

		return $asset;

	}//end getNoResultsBodycopy()


	/**
	* Returns whether the no-results bodycopy is enabled
	*
	* @return boolean
	* @access public
	*/
	function isNoResultsBodycopyEnabled()
	{
		$link_types = SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3;

		$bodycopy_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, $link_types, 'bodycopy', FALSE, 'major', 'no_results');
		$bodycopy_link = reset($bodycopy_links);

		if ($bodycopy_link) {
			return ($bodycopy_link['link_type'] == SQ_LINK_TYPE_2);
		} else {
			return FALSE;
		}


	}//end isNoResultsBodycopyEnabled()


	/**
	* Handles the enabling or disabling of the no-results bodycopy
	*
	* @param boolean	$enabled	set to TRUE to enable the format bodycopy,
	*								set to FALSE to disable it
	*
	* @return boolean
	* @access public
	*/
	function setUseNoResultsBodycopy($enabled)
	{
		$tmp_body_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3, 'bodycopy', FALSE, 'major', 'no_results');
		$bodycopy_link = reset($tmp_body_link);

		if (!$bodycopy_link) {
			// no bodycopy yet? If we're trying to set to disabled, then we don't
			// need to do anything - if not then we need to create it
			if ($enabled) {
				if (!$this->createNoResultsBodycopy()) return FALSE;
			}
		} else {
			// set link type to either TYPE_2 if enabled or TYPE_3 if disabled
			$new_link_type = ($enabled) ? SQ_LINK_TYPE_2 : SQ_LINK_TYPE_3;
			if ($bodycopy_link['link_type'] != $new_link_type) {
				$GLOBALS['SQ_SYSTEM']->am->updateLink($bodycopy_link['linkid'], $new_link_type);
			}
		}

		return TRUE;

	}//end setUseNoResultsBodycopy()


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
