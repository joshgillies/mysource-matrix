<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: page_calendar_events_search.inc,v 1.2 2012/01/27 05:41:33 cupreti Exp $
*
*/

require_once SQ_PACKAGES_PATH.'/search/search_page/search_page.inc';
require_once SQ_PACKAGES_PATH.'/calendar/lib/calendar_common.inc';


/**
* Page_Calendar_Events_Search
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Page_Calendar_Events_Search extends Search_Page
{

	/* Fuzzy date type options
	*
	* @var array
	*/
	var $fuzzy_date_types = Array(
								'today' => 1, 
								'tomorrow'=> 1, 
								'yesterday' => 1, 
								'this_weekend' => 0, 
								'next_weekend' => 0, 
								'previous_weekend' => 0, 
								'this_week' => 1, 
								'next_week' => 1, 
								'previous_week' => 1, 
								'this_fortnight' => 0, 
								'next_fortnight' => 0, 
								'previous_fortnight' => 0, 
								'this_month' => 1, 
								'next_month' => 1, 
								'previous_month' => 1, 
								'this_year' => 0, 
								'next_year' => 0, 
								'previous_year' => 0, 
								'anytime' => 0
							);
	
	
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
	* Processes a search request
	*
	* Note that all search queries are constructed and run in the Search Manager
	* From the posted variables, it formulates search info and returns results
	*
	* @param array	$search	The search variables used for stored searches
	*
	* @return array
	* @access public
	* @see Search_Manager:processSearch()
	*/
	function processSearch($search=Array())
	{	
		$sm_result = parent::processSearch($search);		
		
		// Pick the multi-date assets from the search result
		$multi_date_assets = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($sm_result), Array('calendar_event_multi_date'), TRUE, 'type_code');		
		foreach($multi_date_assets as $event_assetid => $val) {
			// Include the events assoicated with the multi-date event into the search result
			$multi_events = array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($event_assetid, 'calendar_event', FALSE, TRUE));
			foreach($multi_events as $multi_event_id) {
				if (isset($sm_result[$multi_event_id])) {
					$sm_result[$multi_event_id]++;
				} else {
					$sm_result[$multi_event_id] = 1;
				}
			}//end foreach			
		}//end foreach		
		$sm_result_assetids = array_keys($sm_result);		
		
		// Process and get the event instances based on the date filters
		$date_filter_result = array_keys($this->processEventSearch($search));		
		
		// Grouping logic for combing date filters and search field results		
		$result = Array();
		if ($this->attr('main_logic') == 'AND') {			
			foreach($date_filter_result as $event) {
				$assetid = substr($event, 0, strpos($event, ':'));
				if (in_array($assetid, $sm_result_assetids) && !isset($result[$event]))  {
					$result[$event] = $sm_result[$assetid];
				}
			}//end foreach
		} else {
			// Get the event instances for the events in the regular search result
			$sm_fields_result = array_keys($this->getEventInstances($sm_result_assetids));
			$combined_result = array_merge($sm_fields_result, $date_filter_result);
			foreach($combined_result as $event) {
				$assetid = substr($event, 0, strpos($event, ':'));
				$result[$event] = array_get_index($sm_result, $assetid, 1);
			}//end foreach
		}

		return $result;

	}//end processSearch()
	
	/*
	* Get all the instances for supplied events
	*
	* @param array	$event_assetids		Event assetids
	*
	* @return array
	* @access public
	*/		
	function getEventInstances($event_assetids)
	{
		$events = $this->_getEvents();
		
		$relevent_events = Array();
		foreach($events as $event) {
			if (isset($event['assetid']) && in_array($event['assetid'], $event_assetids)) {
				$relevent_events[] = $event;
			}
		}//end foreach
		
		$instances = Calendar_Common::getWholeEventInstances($relevent_events, '1970-01-01', '2030-12-31');
		
		// Sort the events by event start timestamp
		if (!empty($instances)) {
			asort($instances);
		}
		
		return $instances ? $instances : Array();
	
	}//end getEventInstances()
	
	
	/*
	* Process events search
	*
	* @param array	$search	The search variables used for stored searches
	*
	* @return array
	* @access public
	*/	
	function processEventSearch($search=Array())
	{	
		// Get all the relevant events for the searching
		$events = $this->_getEvents();		
		$query = $this->_getEventSearchQuery();
	
		// Save the search terms into the session so that we can use them for
		// search within search results
		// NOTE: This session var is initialised in parent::processSearch()
		$_SESSION['SQ_LAST_SEARCH'][$this->id] = array_merge($_SESSION['SQ_LAST_SEARCH'][$this->id], $query);
		
		// If empty Date search is allowed then search needs to be carried out 
		// in all the event instances, regardless of date filter query submitted
		if ($this->attr('events_filter_allow_empty_search')) {
			$query = Array(
						'default_field' => Array(
											0 => Array(													
													'from' => '1970-01-01',
													'to' => '2030-12-31',
												)
											)
				);
		}//end if		
		
		if (empty($events) || empty($query)) {
			return Array();
		}
		
		// Grouping logic for the filters, AND/OR
		$grouping_logic = $this->attr('events_filter_logic');
		
		$result = FALSE;
		
		foreach($query as $query_field => $query_data) {
			if (empty($query_data)) {
				continue;
			}
			
			// Get event result for each filter
			$field_result = Array();			
			foreach($query_data as $query_date) {
				if (empty($query_date['from']) || empty($query_date['to'])) {
					continue 2;
				}
				$field_result = array_merge($field_result, Calendar_Common::getWholeEventInstances($events, $query_date['from'], $query_date['to']));				
			}//end foreach			

			// Combine the submitted field result based on the selected logical grouping
			if ($result === FALSE) {
				$result = $field_result;
			} else {
				$result = $grouping_logic == 'AND' ? array_intersect($result, $field_result) : array_merge($result, $field_result);
			}			
		}//end foreach
		
		// Sort the events by event start timestamp
		if (!empty($result)) {
			asort($result);
		}

		return $result ? $result : Array();
	
	}//end processEventSearch()
	
	
	/*
	* Get "date" query data submitted in Event Search page
	*	
	* @return array
	* @access private
	*/	
	function _getEventSearchQuery()
	{
		if (isset($this->_tmp['searched_fqueries'])) {
			return $this->_tmp['searched_fqueries'];
		}		

		$filters = $this->attr('events_filters');
		$queries = Array();		
		foreach ($filters as $field_name => $field_details) {

			$queries[$field_name] = '';
			$query_var = 'queries_'.$field_name.'_fquery';
			$clean_query_var = str_replace('.', '_', $query_var);

			$queries[$field_name] = $this->_processDateFilter($query_var, $field_details['type']);

		}//end foreach fields
		
		// if it seems that nothing is submitted, check if there was a previous query
		// and use its details
		if (empty($queries) && !empty($this->_tmp['potential_searched_queries'])) {
			$queries = $this->_tmp['potential_searched_queries'];
		}

		return $queries;
	
	}//end _getEventSearchQuery()
	
	
	/**
	* Prints the individual asset in the asset list
	*
	* This is being overridden to cache the results of each asset, and add
	* the score and result number keyword replacements
	*
	* @param string	$assetid		The assetid of the asset we are printing
	* @param int	$list_position	Asset's position in the list
	* @param int	$num_items		the number of items in the current list (required by Listing Engine)
	*
	* @return void
	* @access protected
	*/
	function _printAsset($assetid, $list_position, $num_items=0)
	{		
		$cache_key = $assetid;

		// the $this->_tmp[] variables are set in getResultsBodyReplacements()
		$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
		$element_contents = $cm->loadFromCache($this->id, $this->type(), $cache_key);

		if ($element_contents === FALSE) {
			// keyword replacements for %result_number% and %score% are now handled
			// by the "extended asset" keywords replacements defined in getExtendedAssetKeywordReplacements()

			ob_start();
			$this->_printEventAsset($assetid, $list_position, $num_items);

			$element_contents = ob_get_contents();
			ob_end_clean();
			$cm->saveToCache($this->id, $this->type(), $cache_key, $element_contents);
		}

		// summon the scores at this point. and work with new DAL result format
		$score =& $this->_tmp['search_results'];
		$score = (isset($score['type_code'])) ? $score['type_code'] : $score;
		$weight = (is_array($score[$assetid])) ? $score[$assetid][0]['weight'] : $score[$assetid];

		$element_replacements = Array(
									'score'			=> $weight,
									'result_number'	=> $this->_tmp['start_element'] + $list_position,

								);

		replace_keywords($element_contents, $element_replacements);

		echo $element_contents;

	}//end _printAsset()
	
	
	/**
	* Prints the individual event asset in the asset list
	*
	* @param string	$assetid		The assetid of the asset we are printing
	* @param int	$list_position	Asset's position in the list
	* @param int	$num_items		the number of items in the current list (required by Listing Engine)
	*
	* @return void
	* @access private
	*/	
	function _printEventAsset($assetid, $list_position, $num_items=0)
	{	
		$assetid_parts = explode(':', $assetid);
		
		if (!isset($assetid_parts[1])) {
			return;
		}		
		
		// allows for asset listings within asset listings, see #2727
		if (isset($_SESSION[SQ_SESSION_SANDBOX_INDEX]['list_current_asset_id'])) {
			$prev_list_current_asset_id = $_SESSION[SQ_SESSION_SANDBOX_INDEX]['list_current_asset_id'];
		} else {
			$prev_list_current_asset_id = null;
		}
		$_SESSION[SQ_SESSION_SANDBOX_INDEX]['list_current_asset_id'] = $assetid;

		$mm			= NULL;
		$contents	= '';
		$keywords	= Array();
		$customised = $this->getFormats('type_formats');

		$asset_info_fields = $GLOBALS['SQ_SYSTEM']->am->getAssetInfoFields();
		$bodycopy = NULL;

		// check to see if there is a specific position format for us
		if (isset($this->_tmp['position_formats'][$list_position])) {
			$bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_tmp['position_formats'][$list_position], 'bodycopy');
		} else if ($num_items > 0) {
			/*
			* Secondly, check if there is a "negative position" that is applicable.
			* Negative position values are used to dynamically control display of list entries by using an offset from the last list element.
			* For example: -1 is a position format for the last entry, -2 is for the second-last entry.
			*
			* Positive position formats take precedence over "negative" ones.
			* Ok, now back to the code...
			*/
			$negative_offset = -1 + ($list_position - $num_items);
			if (isset($this->_tmp['position_formats'][$negative_offset])) {
				$bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_tmp['position_formats'][$negative_offset], 'bodycopy');
			}
		}//end else

		// if we can't find a position format, use the default format
		if (is_null($bodycopy)) {
			if (!isset($this->_tmp['default_format_bc'])) {
				$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->_tmp['type_folder_id'], SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'default_format');
				if ($link) {
					$this->_tmp['default_format_bc'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], 'bodycopy');
				} else {					
					trigger_error('Cannot Find the Default Bodycopy');
					return;
				}
			}
			$bodycopy = $this->_tmp['default_format_bc'];
		}


		// by the time we reach this point we already have a bodycopy for this asset
		$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		if (isset($this->_tmp['bc_keywords'][$bodycopy->id][$contextid])) {
			$bc_keywords = $this->_tmp['bc_keywords'][$bodycopy->id][$contextid];
		} else {
			$bc_keywords = $bodycopy->getKeywords();
			$this->_tmp['bc_keywords'][$bodycopy->id][$contextid] = $bc_keywords;
		}
		
		$keywords    = Array();
		// Asset contents keywords that we want contexted versions thereof
		// These will be stored in the keywords array as "keyword^context:contextid"
		// and accessed later on when we replace the keyword.
		$context_bc_keywords      = Array();
		$paint_layouts_to_resolve = Array();
		$all_contexts             = $GLOBALS['SQ_SYSTEM']->getAllContexts();

		// For each keyword starting with 'asset_contents', ensure there is an entry
		// in $bc_keywords for both the plain keyword and any equivalents with modifiers.
		//
		// This way plain replacement works as expected, and the modifiers can be run
		// later in this method
		foreach ($bc_keywords as $kw) {
			if (substr($kw, 0, strlen('asset_contents')) == 'asset_contents') {
				$kw = parse_keyword($kw, $modifiers);
				$contextid = extract_context_modifier($modifiers);

				if (!in_array($kw, $bc_keywords)) {
					array_push($bc_keywords, $kw);
				}

				// Store which context we need to get this keyword for
				if (array_key_exists($kw, $context_bc_keywords) === FALSE) {
					$context_bc_keywords[$kw] = Array();
				}

				if ($contextid === NULL) {
					$context_bc_keywords[$kw]['current'] = NULL;
				} else {
					$context_bc_keywords[$kw][$contextid] = $all_contexts[$contextid]['name'];
				}

				// Contents being generated with a paint layout - work out which ones
				// we have to resolve (to get around bug #4374, which previously limited
				// support to one arbitrary paint layout per bodycopy)
				if (substr($kw, 0, strlen('asset_contents_paint_')) == 'asset_contents_paint_') {
					$paint_layouts_to_resolve[] = substr($kw, strlen('asset_contents_paint_'));
				}
			}
		}
		
		$this_needs_asset = FALSE;
		foreach ($bc_keywords as $keyword) {
			if (in_array($keyword, Array('asset_url', 'asset_name_linked', 'asset_short_name_linked')) && !SQ_ROLLBACK_VIEW) {
				continue;
			}
			if (!isset($this->_tmp['assets_info'][$assetid][substr($keyword, 6)])) {
				$this_needs_asset = TRUE;
				break;
			}
		}

		if ($this_needs_asset) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid_parts[0]);
			if (is_null($asset)) return;
			
			// Get the event instance data from the shadow bit of the assetid
			$event_data_raw = explode(',', $assetid_parts[1]);
			$event_data = Array();
			
			foreach($event_data_raw as $event_key_val) {			
				list($key, $val) = explode("=", $event_key_val);
				$event_data[$key] = $val;
			}
			
			foreach ($bc_keywords as $keyword) {
				if (strpos($keyword, "instance_start_datetime") !== 0 && strpos($keyword, "instance_end_datetime") !== 0) {
					$keywords[$keyword] = $asset->getKeywordReplacement($keyword);
				} else {
					$keywords[$keyword] = $asset->getEventInstanceKeywordReplacement($keyword, $event_data);

				}
			}//end foreach

			// Body with paint layout applied, if any (inherited from the
			// listing engine asset, usually)
			if (array_key_exists('asset_contents', $keywords)) {
				foreach ($context_bc_keywords['asset_contents'] as $contextid => $context_name) {
					$kw = 'asset_contents';
					if ($context_name !== NULL) {
						$kw .= '^context:'.$context_name;
					}

					$keywords[$kw] = $this->_getAssetContentsBCKeywordReplacement($asset, $contextid);
				}
			}

			// Body with paint layout(%asset_contents_paint_ASSETID%) or user-defined paint layout(%asset_contents_paint_NAME%) applied
			foreach ($paint_layouts_to_resolve as $paint_layout) {
				$base_kw = 'asset_contents_paint_'.$paint_layout;
				foreach ($context_bc_keywords[$base_kw] as $contextid => $context_name) {
					$kw = $base_kw;
					if ($context_name !== NULL) {
						$kw .= '^context:'.$context_name;
					}
					$keywords[$kw] = $this->_getAssetContentsPaintBCKeywordReplacement($asset, $paint_layout, $contextid);
				}

			}

			// %asset_contents_raw% will print the asset's body WITHOUT a
			// paint layout
			if (array_key_exists('asset_contents_raw', $keywords)) {
				foreach ($context_bc_keywords['asset_contents_raw'] as $contextid => $context_name) {
					$kw = 'asset_contents_raw';
					if ($context_name !== NULL) {
						$kw .= '^context:'.$context_name;
					}

					$keywords[$kw] = $this->_getAssetContentsRawBCKeywordReplacement($asset, $contextid);
				}
			}

			// stuff carried over results body replacements
			$keywords['asset_url'] = array_get_index($this->_tmp['asset_urls'], $assetid, './?a='.$assetid);
			$keywords['asset_name_linked'] = '<a href="'.$keywords['asset_url'].'">'.htmlspecialchars($asset->name).'</a>';
			$keywords['asset_short_name_linked'] = '<a href="'.$keywords['asset_url'].'">'.htmlspecialchars($asset->short_name).'</a>';

			if (in_array('asset_lineage', $bc_keywords) || in_array('asset_lineage_linked', $bc_keywords)) {
				preg_match('/(http[s]?:\/\/)(.*)/', $keywords['asset_url'], $matches);
				$lineage_array = Array();
				if (isset($matches[2])) {
					$lineage_array = $GLOBALS['SQ_SYSTEM']->am->getLineageFromUrl(NULL, $matches[2]);
				}//end if

				$lineage = '';
				$linked_lineage = '';

				if (!empty($lineage_array)) {
					$show_root = $this->attr('root_in_lineage');
					$show_self = $this->attr('self_in_lineage');
					$first = TRUE;

					foreach ($lineage_array as $lineage_item) {
						if ($first && !$show_root) {
							$first = FALSE;
							continue;
						}//end if

						if (!$first && $lineage != '') {
							$lineage .= $this->attr('lineage_seperator');
							$linked_lineage .= $this->attr('lineage_seperator');
						}//end if
						$lineage .= $lineage_item['short_name'];
						$linked_lineage .= '<a href="'.$lineage_item['protocol'].'://'.$lineage_item['url'].'">'.htmlspecialchars($lineage_item['short_name']).'</a>';
						$first = FALSE;
					}//end if
				}//end if
				$keywords['asset_lineage'] = htmlspecialchars($lineage);
				$keywords['asset_lineage_linked'] = $linked_lineage;
			}//end if

				// end results body stuff

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			unset($asset);
		} else {			
			$keywords = Array();
			foreach ($this->_tmp['assets_info'][$assetid] as $field => $value) {
				$keywords['asset_'.$field] = htmlspecialchars($value);
			}
			$keywords['asset_url'] = array_get_index($this->_tmp['asset_urls'], $assetid, './?a='.$assetid);
			$keywords['asset_name_linked'] = '<a href="'.$keywords['asset_url'].'">'.$keywords['asset_name'].'</a>';
			$keywords['asset_short_name_linked'] = '<a href="'.$keywords['asset_url'].'">'.$keywords['asset_short_name'].'</a>';

		}//end if needs asset
		
		$group_keywords = preg_grep('/group[0-9]+.*/', $bc_keywords);
		foreach ($group_keywords as $group_keyword) {
			preg_match('/group([0-9]+)_(.*)/', $group_keyword, $matches);
			if (isset($this->_tmp['group_levels'][$matches[1] - 1])) {
				$parent_id = $this->_tmp['group_levels'][$matches[1] - 1];
				if (assert_valid_assetid($parent_id, '', TRUE, FALSE)) {
					// we have an assetid, so do what we want with the parent
					switch ($matches[2]) {
						case 'link_value':
							$link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($parent_id, $assetid);
							$keywords[$group_keyword] = array_get_index($link, 'value', '');
						break;
						case 'link_type':
						case 'dependant':
						case 'exclusive':
						default:
							// other possible uses
						break;
					}
				}
			}//end if
		}

		$keywords['page_href'] = $this->getHref();

		if ($this->attr('group_by') == 'letter') {
			$keywords['asset_position'] = $list_position;
		} else {
			$keywords['asset_position']  = (array_get_index($this->_tmp, 'result_page', 1) - 1) * $this->_getNumPerPage() + $list_position;
		}

		if (($list_position % 2) == 0) {
			$keywords['asset_odd_even'] = translate('cms_listing_asset_position_even');
		} else {
			$keywords['asset_odd_even'] = translate('cms_listing_asset_position_odd');
		}

		// asset selections
		$selections			= $this->attr('asset_selections');
		$selection_defaults = $this->getAssetSelectionDefaults();

		if (is_array($selections) && !empty($selections)) {

			foreach ($selections as $selection_name => $settings) {

				$buffering = in_array($selection_name.'_asset_selection', $bc_keywords);
				$selection_prefix = $this->_getSelectionFieldNamePrefix($selection_name);

				if ($buffering) {
					ob_start();
					$this->registerFormField($selection_prefix);
				}

				// predefine some commonly used settings, for brevity's sake
				$selection_options = array_get_index($settings, 'options', $selection_defaults['options']);
				$selection_restrict = array_get_index($selection_options, 'restrict_range', $selection_defaults['options']['restrict_range']);
				$selection_restrict_enable = array_get_index($selection_restrict, 'enable', $selection_defaults['options']['restrict_range']['enable']);
				$selection_restrict_min = array_get_index($selection_restrict, 'min', $selection_defaults['options']['restrict_range']['min']);
				$selection_restrict_max = array_get_index($selection_restrict, 'max', $selection_defaults['options']['restrict_range']['max']);
				$selection_allow_negative = array_get_index($selection_options, 'allow_negative', $selection_defaults['options']['allow_negative']);
				$selection_allow_multiple = array_get_index($selection_options, 'allow_multiple', $selection_defaults['options']['allow_multiple']);

				// grab the values for the field to display
				$selection_value = $this->getAssetSelectionValue($selection_name, $assetid);

				if ($selection_restrict_enable) {
					$selection_value = ($selection_restrict_min > $selection_value ? $selection_restrict_min : $selection_value);
				}

				switch (array_get_index($settings, 'type', $selection_defaults['type'])) {
					case 'textbox':
						$selection_name_field = $selection_prefix.'['.$assetid.']';
						$selection_id = make_valid_html_id($selection_prefix.'_'.$assetid, '_');

						if ($buffering) {
							if ($selection_restrict_enable) {
								int_text_box($selection_name_field, $selection_value, $selection_allow_negative, 3, $selection_restrict_min, $selection_restrict_max, '', FALSE, FALSE, 'id="'.$selection_id.'"');
							} else {
								int_text_box($selection_name_field, $selection_value, $selection_allow_negative, 3, NULL, NULL, '', FALSE, FALSE, 'id="'.$selection_id.'"');
							}
						}
					break;

					case 'selection':
						$selection_name_field = $selection_prefix.'['.$assetid.']';
						$selection_id = make_valid_html_id($selection_prefix.'_'.$assetid, '_');

						if ($buffering) {
							// list contents
							$selection_list = Array();
							if (!empty($settings['options']['restrict_range']['enable'])) {
								for ($ii = $selection_restrict_min; $ii <= $selection_restrict_max; $ii++) {
									$selection_list[$ii] = $ii;
								}
							} else {
								$selection_list = Array('0');
							}

							// check that the value is on the list
							if (!isset($selection_list[$selection_value])) {
								$selection_value = reset($selection_list);
							}

							// list display
							combo_box($selection_name_field, $selection_list, $selection_allow_multiple, $selection_value, 0, 'id="'.$selection_id.'"');
						}
					break;

					case 'radio':
					default:
						if ($selection_allow_multiple) {
							$selection_name_field = $selection_prefix.'['.$assetid.']';
							$selection_id = make_valid_html_id($selection_prefix.'_'.$assetid, '_');

							if ($buffering) {
								check_box($selection_name_field, '1', ($selection_value > 0 ? TRUE : FALSE), "sq_listing_check_state(this, '".$selection_prefix."')", 'id="'.$selection_id.'"');
								$this->registerFormField($selection_name_field);

								// the flag is used to determine whether a checkbox is
								// present and not ticked, or not present at all
								// only print this once, otherwise it ends up bloating the query string
								if (empty($this->_tmp['asset_selection_checkbox_flag'][$selection_name])) {
										hidden_field('selection_flag_'.$selection_prefix, '1');
										$this->_tmp['asset_selection_checkbox_flag'][$selection_name] = TRUE;
								}
							}
						} else {
							$selection_name_field = $selection_prefix;
							$selection_id = make_valid_html_id($selection_prefix.'_'.$assetid, '_');

							if ($buffering) {
								radio_button($selection_name_field, $assetid, ($selection_value == $assetid ? TRUE : FALSE), '', 'id="'.$selection_id.'"');
							}
						}
					break;
				}//end switch

				// set selection and its ids keywords
				if ($buffering) {
					$keywords[$selection_name.'_asset_selection'] = ob_get_clean();
				}
				$keywords[$selection_name.'_selection_name'] = $selection_name_field;
				$keywords[$selection_name.'_selection_id'] = $selection_id;
			}//end foreach
		}//end if

		// print asset ID always
		$keywords['asset_assetid'] = $assetid;		

		// kept for backwards compatibility
		$keywords['result_no']	=& $keywords['asset_position'];
		$keywords['root_nodes'] = $this->getRootNodesKeywordReplacement();

		$keywords += $this->getExtendedAssetKeywordReplacements();
		// For every keyword with modifiers starting with 'asset_contents', apply the modifiers..
		foreach ($keywords as $kw => $val) {
			if (substr($kw, 0, strlen('asset_contents')) == 'asset_contents') {
				$plain_keyword = parse_keyword($kw, $modifiers);
				if (!empty($modifiers)) {
						// If this is a keyword with modifiers, first see if context
						// is one of them, and if so, augment the plain keyword
						// so we get the version in the correct context
						$contextid = extract_context_modifier($modifiers);

						if ($contextid !== NULL) {
							$plain_keyword .= '^context:'.$all_contexts[$contextid]['name'];
						}

						// Start the modified keyword off with the plain replaced value
						$keywords[$kw] = $keywords[$plain_keyword];

						// Apply modifiers
						apply_keyword_modifiers($keywords[$kw], $modifiers);
				}//end if
			}//end if
		}//end foreach		
		
		$bodycopy->setKeywordReplacements($keywords);
		$bodycopy->printBody();

		if ($prev_list_current_asset_id) {
			$_SESSION[SQ_SESSION_SANDBOX_INDEX]['list_current_asset_id'] = $prev_list_current_asset_id;
		} else {
			unset($_SESSION[SQ_SESSION_SANDBOX_INDEX]['list_current_asset_id']);
		}
		
	}//end _printEventAsset()
	
	
	/*
	* Get all the "Single" and "Recurring" events under the root node(s)
	*	
	* @return array
	* @access private
	*/
	function _getEvents()
	{
		$root_nodes = $this->getRootNodes();
		
		// Get single calendar events
		$sql = Calendar_Common::getSingleEventQueryBase($root_nodes, 'calendar_event_single');		
		$single_result = Array();
		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			$single_result = MatrixDAL::executePdoAll($query);
		} catch (Exception $e) {
				throw new Exception($e->getMessage());
		}
		$single_result = Calendar_Common::condenseResultTreeids($single_result);
	
		// Get recurring calendar events
		$sql = Calendar_Common::getSingleEventQueryBase($root_nodes, 'calendar_event_recurring');
		$recurring_result = Array();
		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			$recurring_result = MatrixDAL::executePdoAll($query);
		} catch (Exception $e) {
				throw new Exception($e->getMessage());
		}
		$recurring_result = Calendar_Common::condenseResultTreeids($recurring_result);
		
		return array_merge($single_result, $recurring_result);	
	
	}//end _getEvents()	
	

	/**
	* Sort the event instances. Event assets are sorted based sorting settings 
	* while each event instances are sorted in ascending order based on the 
	* instance start time
	*
	* @param array	$asset_list	an array of assets to be sorted
	* @param array	$sort_info	sorting information as provided by getSortInfo()
	*
	* @return array
	* @access public
	* @see Listing_Engine::sortAssetList()
	* @see Listing_Engine::getSortInfo()
	*/
	function sortAssetList($asset_list, $sort_info)
	{
		if (count($asset_list) == 1 || empty($sort_info)) {
			return $asset_list;
		}
		
		// Get the events assetids in the result
		$event_assetids = Array();
		foreach($asset_list as $event_instance => $weight) {
			// Event instances in search result are represnted as shadow assets
			if (strpos($event_instance, ':') === FALSE) {
				unset($asset_list[$event_instance]);
				continue;
			}
			$event_assetid = substr($event_instance, 0, strpos($event_instance, ':'));
			$event_assetids[$event_assetid]=$weight;			 
		}
		
		// Sort the event assets based on sort settings
		$sorted_events = parent::sortAssetList($event_assetids, $sort_info);
		
		// Sort each instances based on instance's start time		
		$sorted_event_instances = Array();
		foreach($sorted_events as $event_assetid => $val) {
			foreach($asset_list as $event_instance => $weight) {
				if (strpos($event_instance, $event_assetid.':') === 0) {
					$sorted_event_instances[$event_instance] = $weight;
				}//end if
			}//end foreach
		}//end foreach
		
		return $sorted_event_instances;

	}//end sortAssetList()	
	
	
	/**
	* Returns replacements for keywords that are not replaced by the parent
	* This includes event filter keywords and keywords common to the search 
	* page bodycopies
	*
	* @param string	$keyword	the keyword of the wanted replacement
	*
	* @return string
	* @access public
	*/
	function getGeneralReplacement($keyword)
	{
		if (strpos($keyword, 'event_filter_') !== FALSE) {			
			$replacement = $this->getEventFilterField(substr($keyword, strlen('event_filter_')));
		} else {
			$replacement = parent::getGeneralReplacement($keyword);
		}

		return $replacement;

	}//end getGeneralReplacement()	
	

	/**
	* Get the html content for the given event filter field
	*
	* @param string	$filter_name	the event filter name
	*
	* @return string
	* @access public
	*/
	function getEventFilterField($filter_name)
	{		
		$query_var = $filter_name.'_fquery';
		$filters = $this->attr('events_filters');
		if (!isset($filters[$filter_name]['type'])) {
			return '';
		}
		$this->registerFormField('queries_'.$query_var.'%');
		$query = '';

		if (isset($_SESSION['SQ_LAST_SEARCH'][$this->id])) {
			$query = array_get_index($_SESSION['SQ_LAST_SEARCH'][$this->id], $filter_name, '');
		}
		$read_only = FALSE;

		// Check if a stored stored session variable is set and set query to match
		// But don't touch it if there was some SESSION var already set
		$pmap = $this->getAttribute('stored_query_session');
		$pmap_params = $pmap->getParameters();
		if (is_array($pmap_params) && !empty($pmap_params)) {
			// Check for all fields
			if (in_array('stored_query', $pmap_params)) {
				$key_field = array_search('stored_query', $pmap_params);
				$query = $pmap->getParameterValue($pmap_params[$key_field]);
			}

			// Check for current field
			if (in_array($filter_name, $pmap_params)) {
				$key_field = array_search($filter_name, $pmap_params);
				$query = $pmap->getParameterValue($pmap_params[$key_field]);
			}
			replace_global_keywords($query);
		}
		
		$current_value = array_get_index($_SESSION['SQ_LAST_SEARCH'][$this->id], $filter_name, Array());
		
		$fuzzy_date_options = array_get_index($filters[$filter_name], 'fuzzy_options', Array());		
		foreach($fuzzy_date_options as $key => $val) {
			$fuzzy_date_options[$val] = str_replace('_', ' ', ucfirst($val));
			unset($fuzzy_date_options[$key]);
		}
		
		ob_start();
			
			switch($filters[$filter_name]['type']) {
				case 'single_date':				
					$this->_printDateRange('queries_'.$query_var, $current_value, TRUE, $filters[$filter_name]['use_picker']);
				break;
				
				case 'date_range':
					$this->_printDateRange('queries_'.$query_var, $current_value, FALSE, $filters[$filter_name]['use_picker']);
				break;
				
				case 'fuzzy_dropdown':					
					combo_box('queries_'.$query_var, $fuzzy_date_options, FALSE, array_get_index($current_value[0], 'fuzzy_type', ''));
				break;
				
				case 'fuzzy_checkboxes':
					$selected_opts = Array();
					foreach($current_value as $selected_val) {
						if (isset($selected_val['fuzzy_type'])) {
							$selected_opts[] = $selected_val['fuzzy_type'];
						}//end if						
					}//end foreach
					foreach($fuzzy_date_options as $key => $val) {
						echo "<br />";
						check_box('queries_'.$query_var.'[]', $key, in_array($key, $selected_opts));
						echo "&nbsp;$val";
					}//end foreach
				
				break;
			}//end switch

			if (!$read_only) {
				$this->registerFormField('queries_'.$query_var);
			}

			$replacement = ob_get_contents();

		ob_end_clean();

		return $replacement;

	}//end getGeneralReplacement()	
	
	
	/**
	* Print a date range selection
	*
	* @param string		$field_name		the name of the field we are painting	#
	* @param array		$current_value	the values that should be printed instead of the date selection boxes
	* @param boolean	$single_field	whether to output single field only
	* @param boolean	$user_picker	whether to include date picker
	* @param boolean	$read_only		should we print read only if we have a value
	*
	* @return void
	* @access private
	*/
	function _printDateRange($field_name, $current_value=Array(), $single_field=TRUE, $use_picker=TRUE, $read_only=FALSE)
	{
		if (!isset($current_value['from'])) {
			$current_value['from'] = '---------- --:--:--';
		}

		if (!isset($current_value['to'])) {
			$current_value['to']   = '---------- --:--:--';
		}

		if ($use_picker) {
			require_once SQ_LIB_PATH.'/js_calendar/js_calendar.inc';
			
			if (!JS_Calendar::isInit()) {			
				?><script src="<?php echo sq_web_path('lib'); ?>/html_form/html_form.js"
				type="text/javascript"></script><?php
			}
		}

		?>
		<table border="0" cellspacing="0" cellpadding="0">
			<tr>
				<?php 
				if (!$single_field) {
					echo '<td align="right">'.ucfirst(translate('from')).'</td>';
				}
				?>
				<td>
					<?php
					if ($read_only && $current_value['from'] != '---------- --:--:--') {
						$from = iso8601_ts($current_value['from']);
						echo date('j M Y', $from);
					} else {
						$field = $this->_getDateField($field_name.'_from');
						if (!$field->processField() || $field->value == '---------- --:--:--') {
							$field->setValue($current_value['from']);
						}
						$field->printField();
					}
					?>
				</td>
				<?php
				if ($use_picker && (!$read_only || $current_value['from'] == '---------- --:--:--')) {
					?>
					<td>
						&nbsp;
						<?php
						// print the JS calendar popup date selecta
						$calendar = new JS_Calendar();
						$calendar->changeSetting('onDayClick', 'datetime_set_date');
						$calendar->paint($field_name.'_from', '', TRUE);
						?>
					</td>
					<?php
				}
				?>
			</tr>
			<?php
				if (!$single_field) {				
			?>
			<tr>
				<td align="right"><?php echo ucfirst(translate('to')) ?>:</td>
				<td>
					<?php
					if ($read_only && $current_value['to'] != '---------- --:--:--') {
						$to = iso8601_ts($current_value['to']);
						echo date('j M Y', $to);
					} else {
						$field = $this->_getDateField($field_name.'_to');
						if (!$field->processField() || $field->value == '---------- --:--:--') {
							$field->setValue($current_value['to']);
						}
						$field->printField();
					}
					?>
				</td>
				<?php
				if ($use_picker && (!$read_only || $current_value['to'] == '---------- --:--:--')) {
					?>
					<td>
						&nbsp;
						<?php
						// print the JS calendar popup date selecta
						$calendar = new JS_Calendar();
						$calendar->changeSetting('onDayClick', 'datetime_set_date');
						$calendar->paint($field_name.'_to', '', TRUE);
						?>
					</td>
					<?php
				}
				?>
			</tr>
			<?php
				}
			?>
		</table>
		<?php

	}//end _printDateRange()


	/**
	* Get a datetime field for painting and processing
	*
	* @param string	$field_name	the name of the field we are creating
	*
	* @return object
	* @access private
	*/
	function &_getDateField($field_name)
	{
		$parameters = Array(
						'min'			=> '0000-01-01 00:00:00',
						'max'			=> '9999-12-31 23:59:59',
						'allow_circa'	=> '0',
						'print_format'	=> '',
						'show'			=> Array('y', 'm', 'd'),
						'null'			=> Array('y', 'm', 'd'),
						'style'			=> Array(
											'y'	=> 't',
											'm'	=> 's',
											'd'	=> 's',
										   ),
					  );

		$value = '---------- --:--:--';
		$field = new DateTime_Field($field_name, $value, $parameters, TRUE);
		return $field;

	}//end _getDateField()


	/**
	* Print a description for a given date range
	*
	* @param array	$current_value	the values that should be printed instead of the date selection boxes
	*
	* @return void
	* @access private
	*/
	function _printDateRangeDescription($current_value)
	{
		if (!isset($current_value['from'])) {
			$current_value['from'] = '---------- --:--:--';
		}

		if (!isset($current_value['to'])) {
			$current_value['to']   = '---------- --:--:--';
		}

		$from_string = '';
		if ($current_value['from'] != '---------- --:--:--') {
			$from = iso8601_ts($current_value['from']);
			$from_string = date('j M Y', $from);
		}

		$to_string = '';
		if ($current_value['to'] != '---------- --:--:--') {
			$to = iso8601_ts($current_value['to']);
			$to_string = date('j M Y', $to);
		}

		if (empty($from_string) && empty($to_string)) {
			return;
		} else if (empty($from_string)) {
			echo translate('sch_page_date_range_prior_to', $to_string);
		} else if (empty($to_string)) {
			echo translate('sch_page_date_range_onwards', $from_string);
		} else if ($from_string == $to_string) {
			echo $from_string;
		} else {
			echo $from_string.' - '.$to_string;
		}

	}//end _printDateRangeDescription()
	
	
	/**
	* Process the date range selection
	*
	* @param string	$field_name	the name of the field we are processing
	* @param string $type
	*
	* @return array
	* @access private
	*/
	function _processDateFilter($field_name, $type)
	{
		$result = Array();
		
		switch ($type) {
			case 'single_date':
			case 'date_range':
				$field = $this->_getDateField($field_name.'_from');
				if ($field->processField()) {
					$result[0]['from'] = rtrim($field->value, "-: ");
				}

				$field = $this->_getDateField($field_name.'_to');
				if ($field->processField()) {
					$result[0]['to'] = rtrim($field->value, "-: ");
				}
			
				if (isset($result[0]['from']) && !isset($result[0]['to'])) {
					$result[0]['to'] = $result[0]['from'];
				}
			break;
			
			case 'fuzzy_dropdown':
				if (isset($_REQUEST[$field_name])) {
					$result[0] = $this->_processFuzzyDate($_REQUEST[$field_name]);
				}
			break;
			
			case 'fuzzy_checkboxes':
				if (isset($_REQUEST[$field_name])) {
					foreach($_REQUEST[$field_name] as $index => $selected_date_type) {
						$result[$index] = $this->_processFuzzyDate($selected_date_type);
					}//end foreach
				}
			break;			
		
		}//end switch			

		return $result;

	}//end _processDateFilter()
	
	
	/**
	* Returns the date range for given fuzzy date type
	*
	* @param string $type
	*
	* @return array
	* @access private
	*/
	function _processFuzzyDate($type)
	{
		// Current date
		$today = date("Y-m-d");
		
		switch ($type) {
			case 'today':				
				$from = $today;
				$to = $today;
			break;
			
			case 'tomorrow':
				$from = date("Y-m-d", strtotime($today. " +1 day"));
				$to = date("Y-m-d", strtotime($today. " +1 day"));
			break;
			
			case 'yesterday':
				$from = date("Y-m-d", strtotime($today. " -1 day"));
				$to = date("Y-m-d", strtotime($today. " -1 day"));
			break;
			
			case 'this_weekend':
				$from = date("N") == 7 ? $today : date("Y-m-d", strtotime('next Saturday'));
				$to = date("N") == 7 ? $today : date("Y-m-d", strtotime('next Sunday'));
			break;
			
			case 'next_weekend':
				$from = date("N") == 7 ? date("Y-m-d", strtotime('next Saturday')) : date("Y-m-d", strtotime('next Saturday +1 week'));
				$to = date("Y-m-d", strtotime('this Sunday +1 week'));
			break;

			case 'previous_weekend':
				$from = date("N") == 7 ? date("Y-m-d", strtotime('last Saturday -1 week')) : date("Y-m-d", strtotime('last Saturday'));
				$to = date("Y-m-d", strtotime('last Sunday -1 week'));
			break;
			
			case 'this_week':
				// "2" for Day week means its first day of the week i.e. Monday
				$from = date("N") == 2 ? $today : date("Y-m-d", strtotime('last Monday'));
				$to = date("Y-m-d", strtotime($from. " +1 week"));
			break;
			
			case 'next_week':
				$from = date("Y-m-d", strtotime('next Monday'));
				$to = date("Y-m-d", strtotime($from. " +1 week"));
			break;
			
			case 'previous_week':
				$from = date("N") == 2 ? date("Y-m-d", strtotime('last Monday')) : date("Y-m-d", strtotime('last Monday -1 week'));
				$to = date("Y-m-d", strtotime($from. " +1 week"));
			break;
			
			case 'this_fortnight':
				$from = date("N") == 2 ? $today : date("Y-m-d", strtotime('last Monday'));
				$to = date("Y-m-d", strtotime($from. " +2 weeks"));			
			break;
			
			case 'next_fortnight':
				$from = date("Y-m-d", strtotime('next Monday +1 week'));
				$to = date("Y-m-d", strtotime($from. " +2 weeks"));
			break;
			
			case 'previous_fortnight':
				$from = date("N") == 2 ? date("Y-m-d", strtotime('last Monday -1 week')) : date("Y-m-d", strtotime('last Monday -2 weeks'));
				$to = date("Y-m-d", strtotime($from. " +2 weeks"));
			break;
			
			case 'this_month':
				$from = date("Y-m-d", strtotime(date('m').'/01/'.date('Y')));
				$to = date("Y-m-d", strtotime((date('m')+1).'/01/'.date('Y'))-86400); // 86400 = 24*60*60
			break;
			
			case 'next_month':
				if (date('m') == 12) {
					$month = 1;
					$year = date('Y')+1;
				} else {
					$month = date('m');
					$year = date('Y');
				}
				$from = date("Y-m-d", strtotime($month.'/01/'.$year));
				$to = date("Y-m-d", strtotime(($month+1).'/01/'.$year)-86400);
			break;
			
			case 'previous_month':
				if (date('m') == 1) {
					$month = 12;
					$year = date('Y')-1;
				} else {
					$month = date('m');
					$year = date('Y');
				}
				$from = date("Y-m-d", strtotime($month.'/01/'.$year));
				$to = date("Y-m-d", strtotime(($month == 12 ? 1 : $month+1).'/01/'.$year)-86400);
			break;
			
			case 'this_year':
				$year = date('Y');
				$from = date("Y-m-d", strtotime('01/01/'.$year));
				$to = date("Y-m-d", strtotime('12/31/'.$year));
			break;
			
			case 'next_year':
				$year = date('Y')+1;
				$from = date("Y-m-d", strtotime('01/01/'.$year));
				$to = date("Y-m-d", strtotime('12/31/'.$year));			
			break;
			
			case 'previous_year':
				$year = date('Y')-1;
				$from = date("Y-m-d", strtotime('01/01/'.$year));
				$to = date("Y-m-d", strtotime('12/31/'.$year));			
			break;
			
			case 'anytime':
				$from = '1970-01-01';
				$to = '2030-12-31';
			break;			
		
		}//end switch		
		
		return Array (
				"from" => $from,
				"to" => $to,
				"fuzzy_type" => $type,
			) ;
	
	}//end _processFuzzyDate()
	

	
	//--        KEYWORD DESCRIPTION        --//


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
		$vars['keywords'] = isset($vars['keywords']) ? $vars['keywords'] : Array();

		$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id, 'bodycopy', TRUE);
		$bodycopy_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy');

		// if there is a search in search bodycopy add it to the bodycopy list
		// it will need some of the initial page keywords
		$sis_bodycopy_link    = $this->getFormatBodycopyLink('sis_form');
		if (!empty($sis_bodycopy_link)) {
			$bodycopy_links[] = $sis_bodycopy_link;
		}

		// if there is a search in search bodycopy add it to the bodycopy list
		// it will need some of the initial page keywords
		$search_bodycopy_link    = $this->getFormatBodycopyLink('search_form');
		if (!empty($search_bodycopy_link)) {
			$bodycopy_links[] = $search_bodycopy_link;
		}

		$bodycopies   = Array();
		$type_formats = Array();
		$keywords     = Array();

		foreach ($bodycopy_links as $link_info) {
			if (isset($parents[$link_info['minorid']])) {
				$bodycopies[] = $link_info['value'];
			}
		}

		// ATTENTION!
		// this check relies on the fact that ALL single asset formats like type, default or position
		// are located in subfolders of this asset, and only its context bodycopies are its direct children
		if (empty($bodycopies)) {
			parent::onRequestKeywords($broadcaster, $vars);
			return;
		}

		// work out if forms are customised
		$search_bodycopy_link	= $this->getFormatBodycopyLink('search_form');
		$search_customised		= !empty($search_bodycopy_link) && $search_bodycopy_link['link_type'] != SQ_LINK_TYPE_3;
		$sis_customised			= !empty($sis_bodycopy_link) && $sis_bodycopy_link['link_type'] != SQ_LINK_TYPE_3;

		foreach ($bodycopies as $bodycopy) {
			if ($bodycopy == 'initial' || $bodycopy == 'sis_form' || $bodycopy == 'search_form') {
				// work out the query field keywords
				$fields = $this->attr('fields');
				foreach ($fields as $field => $field_data) {
					$keywords[$field.'_query'] = translate('sch_page_keyword_search_field', $field);
					if ($bodycopy == 'initial') {
						$keywords[$field.'_query_logic'] = translate('sch_page_keyword_search_field_logic', $field);
					}
				}
				// work out the calendar events filter keywords
				$filters = $this->attr('events_filters');
				foreach ($filters as $filter => $filter_data) {
					if (isset($filter_data['type'])) {
						$keywords['event_filter_'.$filter] = translate('page_events_search_filter_field', $filter);
					}
				}				
				// work out search category keywords
				foreach ($this->attr('categories') as $cat_name => $cat_details) {
					$keywords[$cat_name.'_category_chooser'] = 'Dropdown box for category set '.$cat_name;
				}
			}

			// add in the buttons for the customisation bodycopies
			if ($bodycopy == 'search_form') {
				$keywords['submit_button'] = translate('cms_listing_keyword_submit_button');
			}
			if ($bodycopy == 'sis_form') {
				$keywords['search_in_search_button'] = translate('sch_page_keyword_sis_button');
			}

			if ($bodycopy != 'sis_form' && $bodycopy != 'search_form') {
				foreach ($this->keywords[$bodycopy] as $keyword) {
					$keywords[$keyword] = ucwords(str_replace('_', ' ', $keyword));
				}

				// customised forms
				if ($search_customised) {
					$keywords += $this->_getCustomisedFormKeywords();
				}

				// search in search
				if ($sis_customised && $bodycopy == 'results') {
					$keywords += $this->_getSearchInSearchKeywords();
				}
			}
		}//end foreach

		$vars['keywords'] = array_merge($vars['keywords'], $keywords);

	}//end onRequestKeywords()

	
}//end class
?>
