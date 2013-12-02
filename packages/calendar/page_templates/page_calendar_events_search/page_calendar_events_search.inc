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
* $Id: page_calendar_events_search.inc,v 1.15.2.1 2013/08/22 07:25:11 ewang Exp $
*
*/

require_once SQ_PACKAGES_PATH.'/search/search_page/search_page.inc';
require_once SQ_PACKAGES_PATH.'/calendar/lib/calendar_common.inc';


/**
* Page_Calendar_Events_Search
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.15.2.1 $
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
								'upcoming_events' => 0,
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
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printContents()
	{
		if (isset($_REQUEST[$this->getPrefix().'_submit_button']) && empty($_REQUEST['mode'])) {
			$_REQUEST['mode'] = 'results';
		}

		$mode = array_get_index($_REQUEST, 'mode', '');

		if (empty($mode)) {
			// we dont know what button was pressed, so see if we have any query vars
			// that show evidence that a search has been performed without submitting
			// the button value
			$queries = $this->_getEventSearchQuery();
			foreach ($queries as $field_name => $value) {
				if (!empty($value)) {
					$mode = 'results';
					break;
				} else {
					unset($queries[$field_name]);
				}
			}
			$this->_tmp['searched_fqueries'] = $queries;
		}

		if (empty($_SESSION['SQ_LAST_EVENT_SEARCH'][$this->id])) {
			$_SESSION['SQ_LAST_EVENT_SEARCH'][$this->id] = Array();
		} else if ($mode=='results') {
				// preserve the previous query
				$this->_tmp['potential_searched_fqueries'] = $_SESSION['SQ_LAST_EVENT_SEARCH'][$this->id];
		}
		
		parent::printContents();
		
	}//end printContents()
	

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
		// Get the stored search parameters relevant to the event filters
		$event_search = Array();
		foreach($search as $key => $val) {
			if (strpos($key, 'f:') === 0) {
				unset($search[$key]);
				$event_search[substr($key, 2)] = $val;
			}
		}
		
		$sm_result = parent::processSearch($search);
		$sm_queries = $this->_getSearchedQueries();		
		
		// Pick the multi-date assets from the root node
		$multi_date_assets = Array();
		$root_nodes = $this->getRootNodes(FALSE);
		foreach($root_nodes as $root_node) {
			$multi_date_assets += $GLOBALS['SQ_SYSTEM']->am->getChildren($root_node, 'calendar_event_multi_date', TRUE);
		}		
		
		// Map the "single" and "recurring" events to their respective multi-date event asset
		$multi_date_events = Array();		
		foreach($multi_date_assets as $event_assetid => $val) {
			// Include the events assoicated with the multi-date event into the search result
			$multi_events = array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($event_assetid, 'calendar_event', FALSE, TRUE));
			foreach($multi_events as $multi_event_id) {
				if (isset($sm_result[$event_assetid])) {
					if (!isset($sm_result[$multi_event_id])) {
						$sm_result[$multi_event_id] = 1;
					}
				}
				$multi_date_events[$multi_event_id] = $event_assetid;
			}//end foreach			
		}//end foreach		
		
		$sm_result_assetids = array_keys($sm_result);		
		
		$date_filter_result = $this->processEventSearch($event_search);
		
		// Grouping logic for combing date filters and search field results		
		$result = Array();
		if ($this->attr('main_logic') == 'AND') {
			foreach($date_filter_result as $event => $start_ts) {
				$assetid = substr($event, 0, strpos($event, ':'));
				if (isset($multi_date_events[$assetid]) && strpos($event, '|type_code=') !== FALSE) {					
					$event = substr($event, 0, strpos($event, '|type_code=')).'|type_code=calendar_event_multi_date';
				}
				if (!isset($result[$event]) && (empty($sm_queries) || in_array($assetid, $sm_result_assetids)))  {
					$result[$event] = array_get_index($sm_result, $assetid, 1);
				}
			}//end foreach
		} else {
			// Get the event instances for the events in the regular search result
			$sm_fields_result = $this->getEventInstances($sm_result_assetids);
			$combined_result = array_merge($sm_fields_result, $date_filter_result);
			foreach($combined_result as $event => $start_ts) {
				$assetid = substr($event, 0, strpos($event, ':'));
				if (isset($multi_date_events[$assetid]) && strpos($event, '|type_code=') !== FALSE) {					
					$event = substr($event, 0, strpos($event, '|type_code=')).'|type_code=calendar_event_multi_date';
				}
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
		$query = $this->_getEventSearchQuery();
		$events = $this->_getEvents();
		
		$relevant_events = Array();
		foreach($events as $event) {
			if (isset($event['assetid']) && in_array($event['assetid'], $event_assetids)) {
				$relevant_events[] = $event;
			}
		}//end foreach
		if (empty($relevant_events)) {
			return Array();
		}
		
		$grouping_logic = $this->attr('events_filter_logic');
		$result = FALSE;		
		$instances = FALSE;
		if ($this->isEmptyQuery($query)) {
			$instances = Calendar_Common::getWholeEventInstances($relevant_events, '1970-01-01', '2030-12-31');
		} else {
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
					$field_result = array_merge($field_result, Calendar_Common::getWholeEventInstances($relevant_events, $query_date['from'], $query_date['to']));				
				}//end foreach				

				// Combine the submitted field result based on the selected logical grouping
				if ($instances === FALSE) {
					$instances = $field_result;
				} else {
					$instances = $grouping_logic == 'AND' ? array_intersect($result, $field_result) : array_merge($result, $field_result);
				}			
			}//end foreach
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
		$query = $this->_getEventSearchQuery($search);
		
		// See if we have result in the cache
		$cache_key = md5(serialize($query).$this->attr('main_logic').$this->attr('events_filter_logic'));
		$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');		
		$result = $cm->loadFromCache($this->id, $this->type(), $cache_key);
		if ($result !== FALSE) {
			return unserialize($result);
		}
		
		// Get all the relevant events for the searching
		$events = $this->_getEvents();
		
		// Save the search terms into the session so that we can use them for
		// search within search results
		$_SESSION['SQ_LAST_EVENT_SEARCH'][$this->id] = $query;
		
		// If empty Date search is allowed then search needs to be carried out 
		// in all the event instances, regardless of date filter query submitted
		if ($this->attr('events_filter_allow_empty_search') && $this->isEmptyQuery($query)) {
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
		
		if (is_array($result)) {
			$cm->saveToCache($this->id, $this->type(), $cache_key, serialize($result));
		}
		
		return $result ? $result : Array();
	
	}//end processEventSearch()
	

	/*
	* Check if the submitted date filter query is empty
	*
	* @param array $query
	*
	* @return boolean
	* @access public
	*/
	function isEmptyQuery($query)
	{
		// Date filter queries grouping logic
		$grouping_logic = $this->attr('events_filter_logic');
		
		$empty = $grouping_logic == 'AND' ? TRUE : FALSE;
		foreach($query as $field => $val) {
			if ($grouping_logic == 'OR' && empty($val)) {
				$empty = TRUE;
				break;
			}
			foreach($val as $query_data) {
				if ($grouping_logic == 'AND' && !empty($query_data['from']) && !empty($query_data['to'])) {
					$empty = FALSE;
					break 2;
				} else if ($grouping_logic == 'OR' && empty($query_data['from']) && empty($query_data['to'])) {
					$empty = TRUE;
					break 2;
				}
			}
		}
			
		return $empty;
		
	}//end isEmptyQuery()
	
	
	/*
	* Get "date" query data submitted in Event Search page
	*	
	* @return array
	* @access private
	*/	
	function _getEventSearchQuery($search=Array())
	{	
		if (isset($this->_tmp['searched_fqueries'])) {
			return $this->_tmp['searched_fqueries'];
		}		
		
		$filters = $this->attr('events_filters');		
		$queries = Array();
		
		$pmap = $this->getAttribute('stored_query_session');
		$pmap_params = $pmap->getParameters();		
		
		foreach ($filters as $field_name => $field_details) {
			$queries[$field_name] = Array();
			$query_var = 'queries_'.$field_name.'_fquery';
			$clean_query_var = str_replace('.', '_', $query_var);

			$queries[$field_name] = $this->_processDateFilter($query_var, $field_details);
			if (!isset($queries[$field_name][0]['from'])) {
				$queries[$field_name] = $this->_getStoredQueryValue($field_name, $filters[$field_name]);
			}

		}//end foreach fields
		
		// if it seems that nothing is submitted, check if there was a previous query
		// and use its details
		if (empty($queries) && !empty($this->_tmp['potential_searched_fqueries'])) {
			$queries = $this->_tmp['potential_searched_fqueries'];
		}
		
		return $queries;
	
	}//end _getEventSearchQuery()	
	
	
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
		$bind_vars = Array();
		$sql = Calendar_Common::getSingleEventQueryBase($root_nodes, 'calendar_event_single', TRUE, $bind_vars);		
		$single_result = Array();
		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			foreach($bind_vars as $bind_value => $bind_var) {
				MatrixDAL::bindValueToPdo($query, $bind_var,  $bind_value);
			}

			$single_result = MatrixDAL::executePdoAll($query);
		} catch (Exception $e) {
				throw new Exception($e->getMessage());
		}
		$single_result = Calendar_Common::condenseResultTreeids($single_result);
	
		// Get recurring calendar events
		$bind_vars = Array();
		$sql = Calendar_Common::getSingleEventQueryBase($root_nodes, 'calendar_event_recurring', TRUE, $bind_vars);
		$recurring_result = Array();
		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			foreach($bind_vars as $bind_value => $bind_var) {
				MatrixDAL::bindValueToPdo($query, $bind_var,  $bind_value);
			}
			$recurring_result = MatrixDAL::executePdoAll($query);
		} catch (Exception $e) {
				throw new Exception($e->getMessage());
		}
		$recurring_result = Calendar_Common::condenseResultTreeids($recurring_result);
		
		return array_merge($single_result, $recurring_result);	
	
	}//end _getEvents()	
	
	
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
	
	/*
	* Get the value of a stored stored session variable 
	* for the given field, if any
	*
	* @param string $filter_name
	* @param string $details
	*
	* @return array
	* @access private
	*/	
	function _getStoredQueryValue($filter_name, $details)
	{
		$query = Array();		

		// Don't touch it if there was some SESSION var already set
		$pmap = $this->getAttribute('stored_query_session');
		$pmap_params = $pmap->getParameters();
		
		$filter_type = $details['type'];
		if (is_array($pmap_params) && !empty($pmap_params)) {
			// Check for current field
			if (in_array('f:'.$filter_name, $pmap_params)) {
				$key_field = array_search('f:'.$filter_name, $pmap_params);
				$raw_val = $pmap->getParameterValue($pmap_params[$key_field]);
				replace_global_keywords($raw_val);	
				// Values are expected to be in following format:
				//
				// Single date: [DATE]
				// Range date: [FROM_DATE],[TO_DATE]
				// Fuzzy dropdown: fuzzy_type
				// Fuzzy checkbox: fuzzy_type1,fuzzy_type2,...
				// where date are in YYYY-MM-DD format
				
				switch ($filter_type) {
					case 'single_date':
						$query[] = Array(
									'from' => trim($raw_val),
									'to' => trim($raw_val),
								);
					break;
					
					case 'date_range':
						$pq_data = explode(',', $raw_val);						
						list($from, $to) = $pq_data;
						$query[] = Array(
									'from' => trim($from),
									'to' => trim($to),
								);
					break;
			
					case 'fuzzy_dropdown':
					$raw_val = trim($raw_val);
					if (isset($this->fuzzy_date_types[$raw_val])) {
						$event_horizon = array_get_index($details, 'event_horizon', 180);
						$query[] = $this->_processFuzzyDate($raw_val, $event_horizon);
					}
					break;
			
					case 'fuzzy_checkboxes':
						$fuzzy_types = explode(',', $raw_val);
						$event_horizon = array_get_index($details, 'event_horizon', 180);
						foreach($fuzzy_types as $fuzzy_type) {
							$fuzzy_type = trim($fuzzy_type);
							if (isset($this->fuzzy_date_types[$fuzzy_type])) {
								$query[] = $this->_processFuzzyDate($fuzzy_type, $event_horizon);
							}
						}
					break;
				}
			}
		}
		
		return $query;
	
	}//end _getStoredQueryValue()
	

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
		$query = Array();

		if (isset($_SESSION['SQ_LAST_EVENT_SEARCH'][$this->id])) {
			$query = array_get_index($_SESSION['SQ_LAST_EVENT_SEARCH'][$this->id], $filter_name, Array());
		}
		$read_only = FALSE;
		
		$current_value = $this->_processDateFilter('queries_'.$query_var, $filters[$filter_name]);
		if (!isset($current_value[0]['from'])) {
			$current_value = $this->_getStoredQueryValue($filter_name, $filters[$filter_name]);
		}

		$fuzzy_date_options = array_get_index($filters[$filter_name], 'fuzzy_options', Array());
		foreach($fuzzy_date_options as $key => $val) {
			$fuzzy_date_options[$val] = str_replace('_', ' ', ucfirst($val));
			unset($fuzzy_date_options[$key]);
		}
		
		ob_start();
			
			switch($filters[$filter_name]['type']) {
				case 'single_date':	
					$this->_printDateRange('queries_'.$query_var, array_get_index($current_value, 0, Array()), TRUE, $filters[$filter_name]['use_picker']);
				break;
				
				case 'date_range':				
					$this->_printDateRange('queries_'.$query_var, array_get_index($current_value, 0, Array()), FALSE, $filters[$filter_name]['use_picker']);
				break;
				
				case 'fuzzy_dropdown':
					$fuzzy_date_options = array_reverse($fuzzy_date_options, TRUE);
					$fuzzy_date_options['none'] = '';
					$fuzzy_date_options = array_reverse($fuzzy_date_options, TRUE);
					combo_box('queries_'.$query_var, $fuzzy_date_options, FALSE, isset($current_value[0]) ? array_get_index($current_value[0], 'fuzzy_type', '') : '');
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
						check_box('queries_'.$query_var.'_raw[]', $key, in_array($key, $selected_opts));
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
					echo '<td align="right">'.ucfirst(translate('from')).':</td>';
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
						} else {
						
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
	* @param string $details	field details
	*
	* @return array
	* @access private
	*/
	function _processDateFilter($field_name, $details)
	{
		$result = Array();
		
		$type = $details['type'];
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
					$event_horizon = array_get_index($details, 'event_horizon', 180);
					$result[0] = $this->_processFuzzyDate($_REQUEST[$field_name], $event_horizon);
				}
			break;
			
			case 'fuzzy_checkboxes':			
				$fuzzy_types = Array();
				if (isset($_REQUEST[$field_name.'_raw'])) {
					// submitted via form
					$fuzzy_types = $_REQUEST[$field_name.'_raw'];
				} else if (isset($_REQUEST[$field_name])) {
					// submitted via stored search
					$fuzzy_types = explode(',', $_REQUEST[$field_name]);
				}
				
				if (!empty($fuzzy_types)) {
					$event_horizon = array_get_index($details, 'event_horizon', 180);
					foreach($fuzzy_types as $index => $selected_date_type) {
						$result[$index] = $this->_processFuzzyDate(trim($selected_date_type), $event_horizon);
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
	* @param string $event_horizon
	*
	* @return array
	* @access private
	*/
	function _processFuzzyDate($type, $event_horizon=180)
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
				if (date('m') == 12) {
					$to = date("Y-m-d", strtotime('01/01/'.(date('Y')+1))-86400); // 86400 = 24*60*60
				} else {
					$to = date("Y-m-d", strtotime((date('m')+1).'/01/'.date('Y'))-86400); // 86400 = 24*60*60
				}
			break;
			
			case 'next_month':
				if (date('m') == 12) {
					$month = 1;
					$year = date('Y')+1;
				} else {
					$month = date('m')+1;
					$year = date('Y');
				}
				$from = date("Y-m-d", strtotime($month.'/01/'.$year));
				if ($month == 12) {
					$to = date("Y-m-d", strtotime('01/01/'.($year+1))-86400);
				} else {
					$to = date("Y-m-d", strtotime(($month+1).'/01/'.$year)-86400);
				}
			break;
			
			case 'previous_month':
				if (date('m') == 1) {
					$month = 12;
					$year = date('Y')-1;
				} else {
					$month = date('m')-1;
					$year = date('Y');
				}
				$from = date("Y-m-d", strtotime($month.'/01/'.$year));
				if ($month == 12) {
					$to = date("Y-m-d", strtotime('01/01/'.($year+1))-86400);
				} else {
					$to = date("Y-m-d", strtotime(($month+1).'/01/'.$year)-86400);
				}
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

			case 'upcoming_events':
				$event_horizon = (int) $event_horizon;
				if ($event_horizon < 0) {
					$event_horizon = 180;
				}
				$from = date("Y-m-d", strtotime($today. " +1 day"));
				$to = date("Y-m-d", strtotime($from. " +".($event_horizon-1)." day"));
			break;

			case 'anytime':
				$from = '1970-01-01';
				$to = '2030-12-31';
			break;

			case 'none':
				$from = '';
				$to = '';
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
