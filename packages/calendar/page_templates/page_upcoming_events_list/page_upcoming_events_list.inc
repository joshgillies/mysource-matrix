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
* $Id: page_upcoming_events_list.inc,v 1.45 2013/09/18 04:49:27 akarelia Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once SQ_PACKAGES_PATH.'/calendar/lib/calendar_common.inc';

/**
* Page_Upcoming_Events_List
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Revision: 1.45 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Page_Upcoming_Events_List extends Page
{


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

		// create a page contents bodycopy
		$am->includeAsset('bodycopy');
		$page_contents_bc = new Bodycopy();
		$page_contents_bc->setAttrValue('name', 'Page Contents');
		$link = Array('asset' => &$this, 'value' => 'page_contents', 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);
		if (!$page_contents_bc->create($link)) return FALSE;

		// create a type formats folder
		$am->includeAsset('folder');
		$type_formats_folder = new Folder();
		$type_formats_folder->setAttrValue('name', 'Type Formats');
		$link = Array('asset' => &$this, 'value' => 'type_formats', 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);
		if (!$type_formats_folder->create($link)) return FALSE;

		// create type format bodycopies
		$tf_link = Array('asset' => &$type_formats_folder, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 0);
		foreach (Array('calendar_event_single', 'calendar_event_recurring') as $event_type) {
			$tf_link['value'] = $event_type;
			$format_bc = new Bodycopy();
			$format_bc->setAttrValue('name', $am->getTypeInfo($event_type, 'name').' Format');
			if (!$format_bc->create($tf_link)) return FALSE;
			$format_bc = NULL;
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
		$page_links[SQ_LINK_TYPE_2]['bodycopy'] = Array('card' => 1, 'exclusive' => TRUE);
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
		$tf_folder = $this->getFolder('type_formats');
		$broadcaster_parents = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id);
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
		} else if (isset($broadcaster_parents[$this->id])) {
			// it's the contents BC
			$vars['keywords']['event_list'] = 'List of Events';
			$vars['keywords']['root_nodes'] = translate('Root Node');

			return TRUE;
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

		$_REQUEST['SQ_CALENDAR_DATE'] = date('Y-m-d');

		if (!empty($cache_key)) {
			$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
			$cached_contents = $cm->loadFromCache($this->id, $this->type(), $cache_key);
		}

		if ($cached_contents !== FALSE) {
			echo $cached_contents;
		} else {
			// No cached version, so carry out the calculations
			$am =& $GLOBALS['SQ_SYSTEM']->am;
			$mm = NULL;
			$page_links = $am->getLinks($this->id, SQ_LINK_TYPE_2, Array('folder', 'bodycopy'), TRUE, 'major', NULL, TRUE);
			$tf_folder_id = NULL;
			$pb_bodycopy_id = NULL;
			foreach ($page_links as $link) {
				switch ($link['minor_type_code']) {
					case 'folder':
						$this->_tmp['tf_folder_id'] = $link['minorid'];
					break;
					case 'bodycopy':
						$pb_bodycopy_id = $link['minorid'];
					break;
				}
			}
			if (is_null($this->_tmp['tf_folder_id'])) {
				trigger_localised_error('CAL0059', sprintf(translate('Type formats folder not found for Rolling Calendar Page #%s'), $this->id), E_USER_WARNING);
				return;
			}
			if (is_null($pb_bodycopy_id)) {
				trigger_localised_error('CAL0060', sprintf(translate('Page Contents Bodycopy not found for Rolling Calendar Page #%s'), $this->id), E_USER_WARNING);
				return;
			}

			ob_start();
				$pb = $am->getAsset($pb_bodycopy_id, 'bodycopy');
				$replacements = $this->_getPageContentsReplacements();
				$calendar_replacements = $replacements;
				$keywords = $pb->getKeywords();
				foreach ($keywords as $word) {
					if (isset($calendar_replacements[$word])) {
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

		$num_events = $this->attr('num_events');
		$horizon = $this->attr('horizon');

		// All attributes will have some contents of no need to check for empty
		$cache_key .= $num_events; 			// First entry so no colon out front like a little soldier
		$cache_key .= ':'.$horizon;

		// Include the dymanic root nodes as a part of the cache key
		$parameter_map = $this->getAttribute('parameter_map');
		$dynamic_root_nodes = $parameter_map->getParameterValue('replacement_root_node');
		if (!empty($dynamic_root_nodes)) {
			// Dynamic roots can be supplied as an array of asset ids or a comma delimited string of asset ids
			if (is_array($dynamic_root_nodes)) {
				$dynamic_root_nodes = implode(',', $dynamic_root_nodes);
			}
			$cache_key .= ':'.$dynamic_root_nodes;
		}

		$cache_key .= '-ctx'.$contextid;

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
						$sub_keyword = parse_keyword($keyword, $modifiers);
						// the keyword starts with 'asset_' but it might still require the asset
						if ((strpos($keyword, 'asset_metadata_') !== 0) && (!in_array(substr($sub_keyword, 6), $asset_info_fields))) {
							$needs_asset[$link['value']] = TRUE;
						}
					}
					if (strpos($keyword, 'asset_metadata_') === 0) {
						$metadata_keywords[$link['value']][] = $keyword;
					}
				}//end foreach
				if (!$needs_asset[$link['value']]) {
					$somebody_doesnt_need_asset = TRUE;
				}
			}
		}

		$events = $this->_getEvents();

		// Process multi-date events
		// Obtain the bodycopy types we have customised
		$tf_folder = $this->getFolder('type_formats');
		$tf_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($tf_folder->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'major');
		$tf_bcs = Array();
		$types_available = Array();
		foreach ($tf_links as $link) {
			$type = $link['value'];
			$tf_bcs[] = $type;
			$parent_types = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($type, FALSE);
			foreach ($parent_types as $parent_type) {
				$types_available[$parent_type][] = $type;
			}
		}

		$multi_date_events = Array();
		$multi_date_types = Array('calendar_event_multi_date');
		if (isset($types_available['calendar_event_multi_date'])) {
			foreach ($types_available['calendar_event_multi_date'] as $multi_date_type) {
				$multi_date_types[] = $multi_date_type;
			}
		}

		foreach ($events as $event) {
			$event_type_code = $event['type_code'];
			if (($event_type_code == 'calendar_event_single') || ($event_type_code == 'calendar_event_recurring')) {
				$event_parents = $GLOBALS['SQ_SYSTEM']->am->getParents($event['assetid'], $multi_date_types);
				foreach ($event_parents as $parent_id => $event_parent) {
					if (!isset($multi_date_events[$parent_id])) {
						 $multi_date_events[$parent_id] = 1;
					}
				}
			}
		}

		$event_list = '';
		$events_data = Array();
		$mm = NULL;

		// Print one instance of each Multi-Date Event
		foreach ($multi_date_events as $multi_date_event_id => $value_set) {
			$asset = $am->getAsset($multi_date_event_id);

			$type_code = $asset->type();
			$format_type = $asset->type();

			if (isset($required_keywords[$format_type])) {
				foreach ($required_keywords[$format_type] as $full_keyword) {
					$keyword = parse_keyword($full_keyword, $modifiers);
					if ($keyword == 'asset_contents') {
						ob_start();
							$asset->printBody();
							$value = ob_get_contents();
						ob_end_clean();
					} else {
						$value = $asset->getKeywordReplacement($keyword);
					}

					if ($value != '%'.$keyword.'%') {
						apply_keyword_modifiers($value, $modifiers, Array('assetid' => $asset->id));
					} else {
						$value = '%'.$full_keyword.'%';
					}
					$replacements[$full_keyword] = $value;
				}
			}

			// get metadata keywords
			if (!empty($metadata_keywords[$type_code])) {
				if (is_null($mm)) {
					$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
				}
				$metadata_fields = Array();
				foreach($metadata_keywords[$event['type_code']] as $full_keyword) {
					$keyword = parse_keyword($full_keyword, $modifiers);
					$metadata_fields[] = substr($keyword, 15);
				}
				// get all the metadata keywords for this asset
				$metadata_values = $mm->getMetadataFieldValues($id, $metadata_fields);

				foreach ($metadata_keywords[$type_code] as $full_keyword) {
					$keyword = parse_keyword($full_keyword, $modifiers);
					$field_name = substr($full_keyword, 15);
					$value = '';
					if (isset($metadata_values[$field_name])) {
						$value = $metadata_values[$field_name];
						apply_keyword_modifiers($value, $modifiers, Array('assetid' => $asset->id));
					}//end if
					$replacements[$full_keyword] = $value;
				}//end foreach
			}//end if

			if (isset($required_keywords[$format_type])) {
				$entry = $type_formats[$format_type];
			}
			replace_keywords($entry, $replacements);
			$event_list .= $entry;

		}//end foreach
		// End multi-date event processing


		if ($somebody_doesnt_need_asset) {
			$keys = Array();
			foreach ($events as $key => $value) {
				$key_assetid = strtok($key, ':');
				$keys[$key_assetid] = $key_assetid;
			}
			$asset_infos = $am->getAssetInfo($keys);
		}

		$recurring_event_descendants = array_merge($am->getTypeDescendants('calendar_event_recurring', TRUE), Array('calendar_event_modification'));
		$first_current_event_flag = TRUE;

		foreach($events as $id => $event) {
			$events[$id]['current_date'] = $_REQUEST['SQ_CALENDAR_DATE'];
		}

		foreach ($events as $id => $event) {
			$shadow_id = $id;
			$id = strtok($id, ':');
			$replacements = Array();

			// Use the Display Format of the asset type if it has been defined, otherwise use the
			// Display Format of the parent of the asset type. This means that custom Calendar Event
			// assets will be displayed using their own format if one is defined
			if (in_array($event['type_code'], $types_available)) {
				$format_type = $event_type_code;
			} else {
				$format_type = 'calendar_event_single';

				if (in_array($event['type_code'], $recurring_event_descendants)) {
					$format_type = 'calendar_event_recurring';
				}
			}

			$asset = $am->getAsset($id);
			$event_occurance = $event['start_date_ts'];
			if ($asset && $asset->type() == 'calendar_event_recurring') {

				$current_date = $events[$shadow_id]['current_date'];

				// this bit is tricky. Because our $event_occurance timestamp is stored with respect
				// to GMT we need to convert our current local datetime to GMT timestamp to  compare
				// the  correct       occurance time   or else  we won't get  the  correct  results.
				list($gmyear, $gmmonth, $gmday, $gmhour, $gmmin, $gmsec) = sscanf(date('Y-n-j H:i:s'), '%04d-%02d-%02d %02d:%02d:%02d');
				$gm_now = gmmktime($gmhour, $gmmin, $gmsec, $gmmonth, $gmday, $gmyear);

				// If event recurs at current_date move to next day for correct evaluation of next recurrance date
				// also check to see if the start time of the instance for today is after now. Only then the event
				// can be termed to be in future else we will have to skip that instance of the current day event
				if ($asset->hasOccurrenceOnDate($current_date) && (!$first_current_event_flag || $event_occurance < $gm_now)) {
					$current_date = add_days_to_iso($current_date, 1);
				}

				$next_occurance = $asset->getFirstOccurrenceAfter($current_date);
				if ($next_occurance === FALSE) continue; //we exceeded the stop date

				foreach($events as $temp_id => $temp_event) {
					$original_id = strtok($temp_id, ':');
					if ($original_id == $id) {
						$events[$temp_id]['current_date'] = $next_occurance;
						$_REQUEST['SQ_CALENDAR_DATE'] = $next_occurance;
					}
				}
				list($year, $month, $day) = sscanf($next_occurance, '%04d-%02d-%02d');
				if (!empty($day) && !empty($month) && !empty($year)) {
					$event_occurance = gmmktime(0, 0, 0, $month, $day, $year);
				} else {
					$event_occurance = strtotime($next_occurance);
				}
				$first_current_event_flag = FALSE;
			} else if ($asset && $asset->type() == 'calendar_event_modification') {

				$pattern = '/( [0-9\-]{2}:[0-9\-]{2}:[0-9\-]{2})/';

				$occurance_date = preg_replace($pattern, '', $asset->attr('start_date'));
				// for the modification event we ned to find what recurring
				// event we are modifying and update that events instances
				$link = $GLOBALS['SQ_SYSTEM']->am->getParents($id, 'calendar_event_recurring', NULL, NULL, TRUE, 1, 1);

				foreach($events as $temp_id => $temp_event) {
					$original_id = strtok($temp_id, ':');
					if (array_key_exists($original_id, $link)) {
						$events[$temp_id]['current_date'] = $occurance_date;
						$recuring_event_last_occurance_date = $_REQUEST['SQ_CALENDAR_DATE'];
						$_REQUEST['SQ_CALENDAR_DATE'] = $occurance_date;
					}
				}
				$event_occurance = strtotime($occurance_date);
			}

			// get asset keywords
			foreach ($required_keywords[$format_type] as $full_keyword) {
				$keyword = parse_keyword($full_keyword, $modifiers);
				$value = '';
				if ($asset && $needs_asset[$format_type]) {
					if ($keyword == 'asset_contents') {
						ob_start();
							$asset->printBody();
							$value = ob_get_contents();
						ob_end_clean();
					} else {
						$value = $asset->getKeywordReplacement($keyword);
					}
				} else {
					if (isset($asset_infos[$id][substr($keyword, 6)])) {
						$value = $asset_infos[$id][substr($keyword, 6)];
					} else if ($keyword == 'asset_assetid') {
						$value = $id;
					}
				}

				if ($value != '%'.$keyword.'%') {
					apply_keyword_modifiers($value, $modifiers, Array('assetid' => $id));
				} else {
					$value = '%'.$full_keyword.'%';
				}
				$replacements[$full_keyword] = $value;

			}//end foreach

			// get metadata keywords
			if (!empty($metadata_keywords[$event['type_code']])) {
				if (is_null($mm)) {
					$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
				}

				$metadata_fields = Array();
				foreach($metadata_keywords[$event['type_code']] as $full_keyword) {
					$keyword = parse_keyword($full_keyword, $modifiers);
					$metadata_fields[] = substr($keyword, 15);
				}
				// get all the metadata keywords for this asset
				$metadata_values = $mm->getMetadataFieldValues($id, $metadata_fields);

				foreach ($metadata_keywords[$event['type_code']] as $full_keyword) {
					$keyword = parse_keyword($full_keyword, $modifiers);
					$field_name = substr($keyword, 15);
					$value = '';
					if (isset($metadata_values[$field_name])) {
						$value = $metadata_values[$field_name];
						apply_keyword_modifiers($value, $modifiers, Array('assetid' => $id));
					}//end if
					$replacements[$full_keyword] = $value;
				}//end foreach
			}//end if

			$entry = $type_formats[$format_type];
			replace_keywords($entry, $replacements);
			$events_data[] = Array('data' => $entry, 'start_date_ts' => $event_occurance);

			if ($asset && $asset->type() == 'calendar_event_modification')
				$_REQUEST['SQ_CALENDAR_DATE'] = $recuring_event_last_occurance_date;

		}//end foreach events

		// Sort the resulting events based on their instance start time
		uasort($events_data, Array('Calendar_Common', 'compareStartDates'));
		foreach($events_data as $event_data) {
			$event_list .= $event_data['data'];
		}

		if (isset($_REQUEST['SQ_CALENDAR_DATE'])) {
			unset($_REQUEST['SQ_CALENDAR_DATE']);
		}

		return Array('event_list' => $event_list);

	}//end _getPageContentsReplacements()


	/**
	* Get Event Root nodes if specified
	*
	* @return array
	* @access private
	*/
	function _getEventRootNodeIds()
	{
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
					$dynamic_root_nodes[$dynamic_root] = 1;
				}
			}

			if (!empty($dynamic_root_nodes)) {
				return $dynamic_root_nodes;
			}

		}//end if
		return Array();

	}//end _getEventRootNodeIds()


	/**
	* Get the events that should be displayed in this calendar page
	* Returns: (array (assetid => array ( details )))
	*
	* @return array
	* @access protected
	*/
	function _getEvents()
	{
		$db = MatrixDAL::getDb();
		// build sql to restrict the tree locations
		$root_nodes = $this->_getEventRootNodeIds();
		if (empty($root_nodes)) {
			$root_nodes = $this->attr('root_nodes');
		}
		if (empty($root_nodes)) {
			trigger_localised_error('CAL0028', translate('WARNING: No valid root nodes are set for this Calendar Page'), E_USER_WARNING);
			return Array();
		}

		$start_ts = time();
		$horizon_ts = strtotime('+'.$this->attr('horizon').'days');
		$all_events = Array();

		while ((count($all_events) < $this->attr('num_events')) && ($start_ts < $horizon_ts)) {

			$end_ts = strtotime('+'.$this->attr('horizon').' days', $start_ts);
			if ($end_ts > $horizon_ts) break;
			$single_result = Array();
			$recur_result = Array();

			// get the single events
			$bind_vars = Array();
			$date_sql = '(cd.start_date_ts BETWEEN :start_date_ts AND :end_date_ts) OR (cd.start_date_ts < :start_date_ts_1 AND cd.end_date_ts >= :start_date_ts_2)';
			$sql = Calendar_Common::getSingleEventQueryBase(array_keys($root_nodes), 'calendar_event_single', TRUE, $bind_vars).' AND ('.$date_sql.')';

			$single_result = Array();
			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				foreach($bind_vars as $bind_value => $bind_var) {
					MatrixDAL::bindValueToPdo($query, $bind_var,  $bind_value);
				}
				MatrixDAL::bindValueToPdo($query, 'start_date_ts',   $start_ts, PDO::PARAM_INT);
				MatrixDAL::bindValueToPdo($query, 'start_date_ts_1', $start_ts, PDO::PARAM_INT);
				MatrixDAL::bindValueToPdo($query, 'start_date_ts_2', $start_ts, PDO::PARAM_INT);
				MatrixDAL::bindValueToPdo($query, 'end_date_ts',     $end_ts,   PDO::PARAM_INT);
				$single_result = MatrixDAL::executePdoAll($query);
			} catch (Exception $e) {
				throw new Exception($e->getMessage());
			}//end
			$single_result = Calendar_Common::condenseResultTreeids($single_result);

			// get the recurring events
			$bind_vars = Array();
			$sql = Calendar_Common::getRecurringEventQueryBase(array_keys($root_nodes), 'period', date('Y-m-d'), $this->attr('horizon'), 'calendar_event_recurring', $bind_vars);

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

			foreach ($single_result as $event_id => $single_event) {
				// don't expand the single events but the event
				// cancellations and modification needs to be expanded
				if ($single_event['type_code'] == 'calendar_event_single') continue;
				$recur_result[$event_id] = $single_event;
				unset($single_result[$event_id]);
			}
			$recur_events = $recur_result;
			$recur_result = Calendar_Common::expandEventList($recur_result, date('Y-m-d', $start_ts), date('Y-m-d', $end_ts));

			// If the recurring event's frequency is less than global frequency threshold set,
			// limit the recurring event instances to the first instance only
			$recur_result = Calendar_Common::limitRecurringEventInstances($recur_events, $recur_result);

			$new_events	  = $single_result + $recur_result;
			Calendar_Common::processExceptions($new_events);

			$all_events += $new_events;

			$start_ts = $end_ts - 1; // for next loop if necessary

		}

		uasort($all_events, Array('Calendar_Common', 'compareStartDates'));
		$res = Array();
		$i = 0;
		foreach ($all_events as $id => $event) {
			if ($i >= $this->attr('num_events')) break;
			$res[$id] = $event;
			$i++;
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
