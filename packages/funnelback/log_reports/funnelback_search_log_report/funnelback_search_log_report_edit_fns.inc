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
* $Id: funnelback_search_log_report_edit_fns.inc,v 1.2 2012/08/30 00:58:43 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/report/report_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_PACKAGES_PATH.'/funnelback/log_reports/funnelback_search_log_report/funnelback_log_iterator.inc';

/**
* Funnelback_Search_Log_Report_Edit_Fns
*
* Purpose
*	A report that lists searches by month, week, or day
*
* @author  Benjamin Pearson <bpearson@squiz.com.au>	
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage funnelback
*/
class Funnelback_Search_Log_Report_Edit_Fns extends Report_Edit_Fns
{


	/**
	* Generate this report until complete
	*
	* @param object	&$job		a reference to the array of information about
	* @param array	&$step_data	a reference to the array of information about the current step of the HIPO job that is running this report
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	public function generateReport(&$job, &$step_data, $prefix)
	{
		$job_vars =& $job->_running_vars;

		// No report set, bail...
		if (!isset($job_vars['report']) || is_null($job_vars['report'])) {
			trigger_error('Warning Warning', E_USER_WARNING);
			return FALSE;
		}//end if

		// default to build the cache of the log
		if (!isset($job_vars['mode'])) {
			$job_vars['mode'] = 'cache';
		}//end if

		// determine the mode
		$job_vars['mode'] = (string) $job_vars['mode'];
		switch ($job_vars['mode']) {
			case 'cleanup':
				$this->_maintainReport($job_vars);
			break;

			case 'report':
				$this->_generateReport($job_vars);
				$job_vars['mode'] = 'cleanup';

				// Update progress - we are finished!
				$step_data['percent_done'] = 100;
				$step_data['complete']     = TRUE;
			break;

			case 'total':
				$status = $this->_generateTotals($job_vars);
				$job_vars['mode'] = 'report';

				// Update progress
				$step_data['percent_done'] = 70;
				$step_data['complete'] = FALSE;
			break;

			case 'cache':
			default:
				$status = $this->_generateCacheEntries($job_vars);

				// Update progress
				if ($status === TRUE) {
					$step_data['percent_done'] = 40;
					$step_data['complete'] = FALSE;
				} else if ($status === FALSE) {
					$step_data['percent_done'] = 100;
					$step_data['complete'] = TRUE;
				}//end if
			break;
		}//end switch

		return TRUE;

	}//end generateReport()


	/**
	* Generate Cache Entries
	*
	* Generate the Cache for all the logs, ready for totalling
	*
	* @param array	&$job_vars	the job vars array
	*
	* @return boolean
	* @access private
	*/
	private function _generateCacheEntries(&$job_vars)
	{
		// Find the logs to scan
		if (isset($job_vars['logs'])) {
			$meta['report'] = $job_vars['report'];
			$logs = $job_vars['logs'];
			ksort($logs);
		} else {
			// Nothing to do, bail bail bail
			return FALSE;
		}//end if

		// check for the iterator, otherwise grab the first iterator
		if (!isset($job_vars['iterator'])) {
			reset($logs);
			$job_vars['current_collection'] = key($logs);
			$log_file = $logs[$job_vars['current_collection']];
			$job_vars['iterator'] = new Funnelback_Log_Iterator($log_file);
		}//end if

		// next check if the iterator is at the end of the file, if it is proceed to the next log until the logs have run out
		$progress = $job_vars['iterator']->getCurrentProgress();
		$total   = array_get_index($progress, 'total', 0);
		$current = array_get_index($progress, 'current', $total);
		if ($current >= $total && isset($job_vars['current_collection'])) {
			// Finished with this log, remove entry and proceed to the next entry
			unset($job_vars['logs'][$job_vars['current_collection']]);
			unset($job_vars['current_collection']);
			if (!empty($job_vars['logs'])) {
				reset($job_vars['logs']);
				$job_vars['current_collection'] = key($job_vars['logs']);
				$log_file = $job_vars['logs'][$job_vars['current_collection']];
				$job_vars['iterator'] = new Funnelback_Log_Iterator($log_file);
			} else {
				// All done, proceed to totalling
				$job_vars['mode'] = 'total';
				return TRUE;
			}//end if
		}//end if

		// Creating a cache entry for the current line
		$rollover_period = $job_vars['report']->attr('rollover_period');
		$line = $job_vars['iterator']->getNextEntry();
		if (isset($line['line']) && !empty($line['line'])) {
			$period = $this->_getPeriod($line['line']['date'], $rollover_period);
			$path = $job_vars['report']->data_path.'/cache/'.$period;
			$cache = Array();
			$cache['ip']				= $line['line']['ip'];
			$cache['date']				= $line['line']['date'];
			$cache['query']				= $line['line']['query'];
			$cache['full_matches']		= $line['line']['full_matches'];
			$cache['partial_matches']	= $line['line']['partial_matches'];
			$cache['collection']		= $job_vars['current_collection'];
			$this->_generateCacheEntry($path, $cache);
		}//end if

		return NULL;

	}//end _generateCacheEntries()


	/**
	* Generate Totals
	*
	* Generate the Total from the cache entries, ready for reporting
	*
	* @param array	&$job_vars	the job vars array
	*
	* @return boolean
	* @access private
	*/
	private function _generateTotals(&$job_vars)
	{
		$cache_date_entries = glob($job_vars['report']->data_path.'/cache/*');
		foreach ($cache_date_entries as $cache_date_entry) {
			$period = basename($cache_date_entry);
			$path = $job_vars['report']->data_path.'/total/'.$period;
			if (!is_dir($path)) {
				create_directory($path);
			}//end if
			$collections = glob($cache_date_entry.'/*');
			$collection_totals = Array();
			foreach ($collections as $collection_path) {
				$collection_id = basename($collection_path);
				$cache_entries = glob($collection_path.'/*/*/*.cache');
				foreach ($cache_entries as $cache_entry) {
					$query = basename($cache_entry, '.cache');
					$data = @unserialize(file_to_string($cache_entry));
					$report_data = $this->_processTotals($data);
					$actual_query = array_get_index($report_data, 'query', 0);
					$collection_totals = $this->_combineTotals($collection_totals, Array($actual_query => $report_data));

					// Save the report data
					if ($actual_query == '' && $query == 'blank') {
						$hash = '00/00';
					} else {
						$hash = $this->_getQueryHash($query);
					}//end if
					$file = $path.'/'.$collection_id.'/'.$hash.'/'.$query.'.results';
					if (!is_dir(dirname($file))) {
						create_directory(dirname($file));
					}//end if
					string_to_file(serialize($report_data), $file);
				}//end foreach

				// Save the collection totals
				string_to_file(serialize($collection_totals), $path.'/'.$collection_id.'/'.$collection_id.'.results');
			}//end foreach
		}//end foreach

		return TRUE;
	
	}//end _generateTotals()


	/**
	* Generate Report
	*
	* @param array	&$job_vars	the job vars
	*
	* @return boolean
	* @access private
	*/
	private function _generateReport(&$job_vars)
	{
		$total_entries = glob($job_vars['report']->data_path.'/total/*');
		$path = $job_vars['report']->data_path.'/report/';
		if (!is_dir($path)) {
			create_directory($path);
		}//end if
		foreach ($total_entries as $total_date_entry) {
			$period = basename($total_date_entry);
			$report_date = $this->_generateReportDate($period);	
			$report_content = $this->_generateReportContents($job_vars, $total_date_entry);
			file_put_contents($path.'report-'.date('d-m-Y', $report_date).'.html', $report_content);
		}//end foreach

	}//end _generateReport()


	/**
	* Generate Cache Entry 
	*
	* @param string	$path	path to create the cache entry
	* @param array	$cache	the data to cache
	*
	* @return boolean
	* @access private
	*/
	private function _generateCacheEntry($path, $cache)
	{
		if (empty($cache)) {
			return FALSE;
		}//end if

		if (empty($cache['query'])) {
			// Blank search
			$query = 'blank';
			$hash = '00/00';
		} else {
			$query = $cache['query'];
			$hash = $this->_getQueryHash($this->_cleanQuery($query));
		}//end if

		$path = $path.'/'.$cache['collection'].'/'.$hash;
		if (!is_dir($path)) {
			create_directory($path);
		}//end if

		$filename = $this->_cleanQuery($query).'.cache';
		$file = $path.'/'.$filename;

		// Check if writable
		if (!is_writable(dirname($file))) {
			return FALSE;
		}//end if

		// Load up the cache file
		if (file_exists($file)) {
			$data = @unserialize(file_to_string($file));
		} else {
			$data = Array();
		}//end if

		// Add in the new entry
		$data[] = $cache;

		// Save
		file_put_contents($file, serialize($data));

		return TRUE;

	}//end _generateCacheEntry()


	/**
	* Process Totals 
	*
	* Process the totals of a given query per collection
	*
	* @param array	$data	data of the query per collection
	*
	* @return array
	* @access private
	*/
	private function _processTotals($data)
	{
		$report = Array();

		// Get the results
		$report['queries'] = count($data);
		$query = NULL;
		foreach ($data as $line) {
			// This should NOT happen, so just in case
			if (is_null($query)) {
				// First case, set the query to the first known query
				$query = array_get_index($line, 'query', NULL);
			} else {
				// If current line doesn't match the first case, skip over
				$check = array_get_index($line, 'query', '');
				if ($check != $query) continue;
				$report['query'] = $query;
			}//end if
			$min = array_get_index($report, 'min_results', NULL);
			$max = array_get_index($report, 'max_results', NULL);
			$avg = array_get_index($report, 'avg_results', NULL);
			$date = array_get_index($line, 'date', NULL);
			$full_matches = array_get_index($line, 'full_matches', 0);
			$partial_matches = array_get_index($line, 'partial_matches', 0);
			$total_matches = $full_matches + $partial_matches;
			if (!isset($report['total_results'])) {
				$report['total_results'] = $total_matches;
			} else {
				$report['total_results'] += $total_matches;
			}//end if

			// Total stats
			if (is_null($min)) {
				$report['min_results'] = $total_matches;
			} else {
				$report['min_results'] = min($min, $total_matches);
			}//end if
			if (is_null($max)) {
				$report['max_results'] = $total_matches;
			} else {
				$report['max_results'] = max($max, $total_matches);
			}//end if
			$report['avg_results'] = ((!empty($total_matches)) ? ($report['total_results'] / $report['queries']) : 0);
		}//end foreach

		return $report;

	}//end _processTotals()


	/**
	* Combine Totals 
	*
	* Combine the totals of a each query in a collection
	*
	* @param array	$collection	data of the collection
	* @param array	$query		data of the query per collection
	*
	* @return array
	* @access private
	*/
	private function _combineTotals($collection, $query)
	{
		// Format a structure if not set
		if (!isset($collection['total_fail'])) $collection['total_fail'] = 0;
		if (!isset($collection['total_pass'])) $collection['total_pass'] = 0;
		if (!isset($collection['total_queries'])) $collection['total_queries'] = 0;
		if (!isset($collection['total_results'])) $collection['total_results'] = 0;

		foreach ($query as $query_term => $query_results) {
			// Never seen this query term, define a new results structure for this query
			if (empty($query_term)) $query_term = 0;
			if (!isset($collection[$query_term])) {
				$collection[$query_term] = Array();
			}//end if

			// Combine the total statistics per query
			$collection['total_queries'] += $query_results['queries'];
			if (isset($query_results['total_results'])) {
				$collection['total_results'] += $query_results['total_results'];
				if (is_null(array_get_index($collection[$query_term], 'total_queries', NULL))) {
					$collection[$query_term]['total_queries'] = $query_results['queries'];
				} else {
					$collection[$query_term]['total_queries'] += $query_results['queries'];
				}//end if
				if (is_null(array_get_index($collection[$query_term], 'total_results', NULL))) {
					$collection[$query_term]['total_results'] = $query_results['total_results'];
				} else {
					$collection[$query_term]['total_results'] += $query_results['total_results'];
				}//end if
				$collection[$query_term]['total_avg_results'] = ((!empty($collection[$query_term]['total_queries'])) ? ($collection[$query_term]['total_results'] / $collection[$query_term]['total_queries']) : 0);
				if (is_null(array_get_index($collection[$query_term], 'total_min_results', NULL))) {
					$collection[$query_term]['total_min_results'] = $query_results['min_results'];
				} else {
					$collection[$query_term]['total_min_results'] = min($collection[$query_term]['total_min_results'], $query_results['min_results']);
				}//end if
				if (is_null(array_get_index($collection[$query_term], 'total_max_results', NULL))) {
					$collection[$query_term]['total_max_results'] = $query_results['max_results'];
				} else {
					$collection[$query_term]['total_max_results'] = max($collection[$query_term]['total_max_results'], $query_results['max_results']);
				}//end if
				if (empty($query_results['total_results'])) {
					$collection['total_fail']++;
					if (is_null(array_get_index($collection[$query_term], 'total_fail', NULL))) {
						$collection[$query_term]['total_fail'] = 1;
					} else {
						$collection[$query_term]['total_fail']++;
					}//end if
				} else {
					$collection['total_pass']++;
					if (is_null(array_get_index($collection[$query_term], 'total_pass', NULL))) {
						$collection[$query_term]['total_pass'] = 1;
					} else {
						$collection[$query_term]['total_pass']++;
					}//end if
				}//end if
			}//end if
		}//end foreach

		return $collection;

	}//end _combineTotals()


	/**
	* Generate a Report Date from the given period
	*
	* @param string	$period	the period of the report
	*
	* @return string
	* @access private
	*/
	private function _generateReportDate($period)
	{
		$period_type = substr($period, 0, 1);
		$period_date = substr($period, 1);

		$period_type = (string) $period_type;
		switch ($period_type) {
			case 'W':
				$year = substr($period_date, 0, 4);
				$week = substr($period_date, 4);

				// Calculating the start of the week
				$soy = strtotime('first monday january '.$year);
				$soyw = date('W', $soy);
				$days_diff = ($week - $soyw) * 7;
				$sow = strtotime('+'.$days_diff.' days', $soy);
				return mktime(0, 0, 0, date('m', $sow), date('d', $sow), $year);
			break;

			case 'M':
				$year = substr($period_date, 0, 4);
				$month = substr($period_date, 4);
				return mktime(0, 0, 0, $month, 1, $year);
			break;

			case 'D':
				$year = substr($period_date, 0, 4);
				$month = substr($period_date, 4, 2);
				$day = substr($period_date, 6, 2);
				return mktime(0, 0, 0, $month, $day, $year);
			break;

			default:
				// Playing safe and returning the current date/time
				return time();
			break;
		}//end switch

	}//end _generateReportDate()


	/**
	* Generate Report Contents
	*
	* @param array	&$job_vars	the job vars
	* @param string	$path		the path to the data for the report to generate
	*
	* @return string
	* @access private
	*/
	private function _generateReportContents(&$job_vars, $path)
	{
		$overall = Array();
		$contents = '';
		$total_contents = '';
		$collection_data_paths = glob($path.'/*');
		foreach ($collection_data_paths as $collection_data_path) {
			$collection_id = basename($collection_data_path);

			// Printing the collection header
			$contents .= $this->_printCollectionHeader($collection_id);

			// Getting the totals/summary for the current collection
			$totals = glob($collection_data_path.'/*/*/*.results');
			$good = Array();
			$fail = Array();
			$failed_query_totals = Array();
			$success_query_totals = Array();
			$fail_limit = Array();
			$good_limit = Array();
			$row_limit = $job_vars['report']->attr('row_count');
			foreach ($totals as $total) {
				// This is the total for the current collection
				// Working out the top XX results per good/fail
				$filename = basename($total, '.results');
				$query_totals = @unserialize(file_get_contents($total));
				$query = array_get_index($query_totals, 'query', 0);
				if (empty($query)) continue;
				$query_results = array_get_index($query_totals, 'total_results', 0);
				$query_queries = array_get_index($query_totals, 'queries', 0);
				if (empty($query_results)) {
					if (count($fail_limit) < $row_limit) {
						// We have not reached the limit yet
						$fail_limit[$query] = $query_queries;
						$failed_query_totals[$query] = $query_totals;
					} else {
						// Reached the limit, see if we have made in it
						reset($fail_limit);
						$fail_threshold = end($fail_limit);
						if ($query_queries > $fail_threshold) {
							unset($failed_query_totals[key($fail_limit)]);
							array_pop($fail_limit);
							$fail_limit[$query] = $query_queries;
							$failed_query_totals[$query] = $query_totals;
						}//end if
					}//end if
					arsort($fail_limit);
				} else {
					if (count($good_limit) < $row_limit) {
						// We have not reached the limit yet
						$good_limit[$query] = $query_queries;
						$success_query_totals[$query] = $query_totals;
					} else {
						// Reached the limit, see if we have made in it
						reset($good_limit);
						$good_threshold = end($good_limit);
						if ($query_queries > $good_threshold) {
							unset($success_query_totals[key($good_limit)]);
							array_pop($good_limit);
							$good_limit[$query] = $query_queries;
							$success_query_totals[$query] = $query_totals;
						}//end if
					}//end if
					arsort($good_limit);
				}//end if
			}//end foreach

			// Top XX failed results
			foreach ($fail_limit as $query => $totals) {
				$query_totals = $failed_query_totals[$query];
				$line = $this->_formatLine($query_totals, FALSE);
				if (!empty($line)) {
					$fail[] = $line;
				}//end if
				$this->_combineOverallResults($query_totals, $overall);
			}//end foreach

			// Top XX good results
			foreach ($good_limit as $query => $totals) {
				$query_totals = $success_query_totals[$query];
				$line = $this->_formatLine($query_totals, TRUE);
				if (!empty($line)) {
					$good[] = $line;
				}//end if
				$this->_combineOverallResults($query_totals, $overall);
			}//end foreach
			$contents .= $this->_printQueryResults($good, $fail);
		}//end foreach

		// Print overall results
		$total_contents .= $this->_printCollectionHeader(FALSE);
		$total_good = Array();
		$total_fail = Array();
		$total_good_limit = Array();
		$total_fail_limit = Array();
		$row_limit = $job_vars['report']->attr('row_count');
		foreach ($overall as $query => $query_data) {
			if (!is_array($query_data)) {
				// Total stats for the collection
			} else if ($query === 0) {
				// Blank query searches
			} else {
				// Query stats for this collection
				$query_data['query'] = $query;
				$query_results = array_get_index($query_data, 'total_results', 0);
				$query_queries = array_get_index($query_data, 'total_queries', 0);
				if (empty($query_results)) {
					if (count($total_fail_limit) < $row_limit) {
						// Add to the Top XX
						$total_fail_limit[$query] = $query_queries;
					} else {
						reset($total_fail_limit);
						$threshold = end($total_fail_limit);
						if ($query_queries > $threshold) {
							array_pop($total_fail_limit);
							$total_fail_limit[$query] = $query_queries;
						}//end if
					}//end if
					arsort($total_fail_limit);
				} else {
					if (count($total_good_limit) < $row_limit) {
						// Add to the Top XX
						$total_good_limit[$query] = $query_queries;
					} else {
						reset($total_good_limit);
						$threshold = end($total_good_limit);
						if ($query_queries > $threshold) {
							array_pop($total_good_limit);
							$total_good_limit[$query] = $query_queries;
						}//end if
					}//end if
					arsort($total_good_limit);
				}//end if
			}//end if
		}//end foreach

		arsort($total_fail_limit);
		foreach ($total_fail_limit as $query => $total) {
			$query_data = $overall[$query];
			$query_data['query'] = $query;
			$line = $this->_formatLine($query_data, FALSE, TRUE);
			if (!empty($line)) {
				$total_fail[] = $line;
			}//end if
		}//end foreach

		arsort($total_good_limit);
		foreach ($total_good_limit as $query => $total) {
			$query_data = $overall[$query];
			$query_data['query'] = $query;
			$line = $this->_formatLine($query_data, TRUE, TRUE);
			if (!empty($line)) {
				$total_good[] = $line;
			}//end if
		}//end foreach
		$total_contents .= $this->_printQueryResults($total_good, $total_fail);

		// Combine
		$contents = $contents.$total_contents;

		return $contents;

	}//end _generateReportContents()


	/**
	 * Combine the overall results 
	 *
	 * @param array	$query_totals	The totals of the current query
	 * @param array &$overall		The overall results
	 *
	 * @return void
	 * @access private
	 */
	private function _combineOverallResults($query_totals, &$overall)
	{
		$query = array_get_index($query_totals, 'query', NULL);
		if (!is_null($query) && !empty($query)) {
			if (!isset($overall[$query])) $overall[$query] = Array();
			foreach ($query_totals as $key => $value) {
				if ($key == 'query') continue;

				// The maths...
				if ($key == 'avg_results') {
					// This is done later
				} else if ($key == 'min_results') {
					$overall_total = array_get_index($overall[$query], 'total_min_results', 0);
					$overall[$query]['total_min_results'] = min($overall_total, $value);
				} else if ($key == 'max_results') {
					$overall_total = array_get_index($overall[$query], 'total_max_results', 0);
					$overall[$query]['total_max_results'] = max($overall_total, $value);
				} else {
					// Everything else, just calculate
					$overall_key = ((strpos($key, 'total_') === 0) ? $key : 'total_'.$key);
					$overall_total = array_get_index($overall[$query], $overall_key, 0);
					$overall[$query][$overall_key] = $overall_total + $value;
				}//end if
			}//end foreach

			// Finally, update the averages
			$total_results = array_get_index($query_totals, 'total_results', 0);
			$overall_results = array_get_index($overall[$query], 'total_results', 0);
			$total_queries = array_get_index($query_totals, 'queries', 0);
			$overall_queries = array_get_index($overall[$query], 'total_queries', 0);
			if (!empty($total_queries) && !empty($overall_queries)) {
				$new_average = ($total_results + $overall_results) / ($total_queries + $overall_queries);
				$overall[$query]['total_avg_results'] = $new_average;
			}//end if
		}//end if

	}//end _combineOverallResults()


	/**
	 * Print the Collection Header 
	 *
	 * @param string|boolean	$collection_id	The id of the collection to print
	 *
	 * @return string
	 * @access private
	 */
	private function _printCollectionHeader($collection_id)
	{
		if ($collection_id === FALSE) {
			// The total header
			$header = '<br /><strong>Totals</strong>';
		} else {
			$fm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('funnelback_manager');
			$name = $fm->getCollectionName($collection_id);
			if (empty($name)) {
				$header = '<br /><strong>Collection "'.$collection_id.'"</strong>';
			} else {
				$header = '<br /><strong>Collection: '.$name.'</strong>';
			}//end if
		}//end if

		return $header;

	}//end _printCollectionHeader()


	/**
	 * Format a report line
	 *
	 * @param array		$data	The data to show on the report line
	 * @param boolean	$type	The type of line to print (FALSE for failure, TRUE for good)
	 *
	 * @return string
	 * @access private
	 */
	private function _formatLine($data, $type=TRUE, $total=FALSE)
	{
		$query = array_get_index($data, 'query', NULL);
		if (is_null($query)) return '';
		ob_start();
			if ($total) {
			?>
				<tr>
					<td class="sq-backend-table-cell"><?php echo $query; ?></td>
					<td class="sq-backend-table-cell" style="width: 25px;"><?php echo array_get_index($data, 'total_queries', 0); ?></td>
					<?php if ($type) { ?>
					<td class="sq-backend-table-cell" style="width: 25px;"><?php echo array_get_index($data, 'total_results', 0); ?></td>
					<td class="sq-backend-table-cell" style="width: 25px;"><?php echo round(array_get_index($data, 'total_avg_results', 0), 2); ?></td>
					<td class="sq-backend-table-cell" style="width: 25px;"><?php echo array_get_index($data, 'total_min_results', 0); ?></td>
					<td class="sq-backend-table-cell" style="width: 25px;"><?php echo array_get_index($data, 'total_max_results', 0); ?></td>
					<?php } ?>
				</tr>
			<?php
			} else {
			?>
				<tr>
					<td class="sq-backend-table-cell"><?php echo $query; ?></td>
					<td class="sq-backend-table-cell" style="width: 25px;"><?php echo array_get_index($data, 'queries', 0); ?></td>
					<?php if ($type) { ?>
					<td class="sq-backend-table-cell" style="width: 25px;"><?php echo array_get_index($data, 'total_results', 0); ?></td>
					<td class="sq-backend-table-cell" style="width: 25px;"><?php echo round(array_get_index($data, 'avg_results', 0), 2); ?></td>
					<td class="sq-backend-table-cell" style="width: 25px;"><?php echo array_get_index($data, 'min_results', 0); ?></td>
					<td class="sq-backend-table-cell" style="width: 25px;"><?php echo array_get_index($data, 'max_results', 0); ?></td>
					<?php } ?>
				</tr>
			<?php
			}//end if
			$line = ob_get_contents();
		ob_end_clean();

		return $line;

	}//end _formatLine()


	/**
	 * Print the Query Results
	 *
	 * @param array		$good	The good search results
	 * @param array		$fail	The failed search results
	 *
	 * @return string
	 * @access private
	 */
	private function _printQueryResults($good, $fail)
	{
		ob_start();
		?>
		<br />
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header" style="width: 200px"><?php echo translate('fnb_log_popular_queries'); ?></td>
				<td class="sq-backend-table-header"><?php echo translate('fnb_log_total_queries'); ?></td>
				<td class="sq-backend-table-header"><?php echo translate('fnb_log_total_results'); ?></td>
				<td class="sq-backend-table-header"><?php echo translate('fnb_log_avg_results'); ?></td>
				<td class="sq-backend-table-header"><?php echo translate('lowest'); ?></td>
				<td class="sq-backend-table-header"><?php echo translate('highest'); ?></td>
			</tr>
			<?php echo (!empty($good) ? implode("\n", $good) : '<td class="sq-backend-table-cell" colspan="6">'.translate('fnb_log_no_successful_queries').'</td>'); ?>
		</table>

		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header" style="width: 200px"><?php echo translate('fnb_log_failed_queries'); ?></td>
				<td class="sq-backend-table-header"><?php echo translate('fnb_log_total_failed_queries'); ?></td>
			</tr>
			<?php echo (!empty($fail) ? implode("\n", $fail) : '<td class="sq-backend-table-cell" colspan="2">'.translate('fnb_log_no_failed_queries').'</td>'); ?>
		</table>
		<br />
		<?php
			$line = ob_get_contents();
		ob_end_clean();

		return $line;

	}//end _printQueryResults()


	/**
	* Clean Query String ... making it safe for filesystem use (#&;`|*?~<>^()[]{}$)
	*
	* @param string	$query	The query string to cleanup
	*
	* @return string
	* @access private
	*/
	private function _cleanQuery($query)
	{
		$invalid = Array('#', '&', ';', '`', '|', '*', '?', '~', '<', '>', '^', '(', ')', '[', ']', '{', '}', '$');
		$query = str_replace($invalid, '', $query);
		$query = str_replace(' ', '', $query);
		$query = str_replace('"', '', $query);
		$query = str_replace("'", '', $query);
		$query = str_replace('.', '_', $query);
		$query = trim($query);

		return $query;

	}//end _cleanQuery()


	/**
	* Get Period returns the period of the current line 
	*
	* @param string	$date	The date of the current line (timestamp)
	* @param array	$type	The rollover period type (daily, weekly, monthly)
	*
	* @return string
	* @access private
	*/
	private function _getPeriod($date, $type)
	{
		$type = (string) $type;
		switch ($type) {
			case 'daily':
				$period = 'D'.date('Ymd', $date);
			break;

			case 'monthly':
				$period = 'M'.date('Ym', $date);
			break;

			case 'weekly':
			default:
				$period = 'W'.date('YW', $date);
			break;
		}//end switch

		return $period;

	}//end _getPeriod()


	/**
	* Get Query Hash - Find a 2 level deep hash based on the current query 
	*
	* @param string	$query	The query
	*
	* @return string
	* @access private
	*/
	private function _getQueryHash($query)
	{
		$hash = Array();

		if (strlen($query) <= 4) {
			// The Query is less than 4 chars, so pad it up
			$query = str_pad($query, 4, 'a');
		}//end if

		// Hashing up based on the first 4 chars
		$hash_bits = str_split(substr($query, 0, 4), 2);
		foreach ($hash_bits as $bit) {
			$bit = (string) $bit;
			do {
				$hash_part = 0;
				for ($i=0; $i<strlen($bit); $i++) {
					$hash_part += ord($bit{$i});
				}//end foreach
				$bit = (string) $hash_part;
			} while ($hash_part < 100);
			$hash[] = (int) $hash_part;
		}//end foreach

		if (!empty($hash) && count($hash) == 2) {
			return implode('/', $hash);
		} else {
			return 'na';
		}//end if

	}//end _getQueryHash()


	/**
	* Maintain Report - runs any maintenance duties for the report, like cleaning up old entries, creating base directories etc 
	*
	* @param array 	&$job_vars	The jobs vars of the hipo
	*
	* @return void
	* @access private
	*/
	private function _maintainReport(&$job_vars)
	{
		$data_path = $job_vars['report']->data_path;

		$type = $job_vars['report']->attr('rollover_period');
		$type = (string) $type;
		$match = strtoupper(substr($type, 0, 1));
		switch ($type) {
			case 'daily':
				$valid = Array(
							date('Ymd', strtotime('-2 days')),
							date('Ymd', strtotime('-1 day')),
							date('Ymd'),
						 );
			break;

			case 'monthly':
				$valid = Array(
							date('Ym', strtotime('-1 month')),
							date('Ym'),
						 );
			break;

			case 'weekly':
			default:
				$valid = Array(
							date('YW', strtotime('-1 week')),
							date('YW'),
						 );
			break;
		}//end switch

		// Create base directories
		$base_directories = Array('cache', 'total', 'report');
		foreach ($base_directories as $dir) {
			create_directory($data_path.'/'.$dir);
		}//end foreach

		// Clean cache files
		$cache_files = glob($data_path.'/cache/*');
		foreach ($cache_files as $cache_dir) {
			$cache_file = basename($cache_dir);
			// Invalid cache files
			$cache_match = strtoupper(substr($cache_file, 0, 1));
			if ($cache_match != $match) {
				delete_directory($cache_dir);
			}//end if

			// Too old
			$date_match = substr($cache_file, 1);
			if (!in_array($date_match, $valid)) {
				delete_directory($cache_dir);
			}//end if
		}//end foreach

		// Clean total files
		$total_files = glob($data_path.'/total/*');
		foreach ($total_files as $total_dir) {
			$total_file = basename($total_dir);
			// Invalid total files
			$total_match = strtoupper(substr($total_file, 0, 1));
			if ($total_match != $match) {
				delete_directory($total_dir);
			}//end if

			// Too old
			$date_match = substr($total_file, 1);
			if (!in_array($date_match, $valid)) {
				delete_directory($total_dir);
			}//end if
		}//end foreach

	}//end _maintainReport()


	/**
	* Get the latest report
	*
	* @param object	&$asset	The report asset
	*
	* @return mixed boolean|string
	* @access private
	*/
	private function _getLatest(&$asset)
	{
		$latest = FALSE;
		$path = $asset->data_path.'/report';
		$reports = glob($path.'/report-*.html');
		foreach ($reports as $report) {
			$report_name = basename($report, '.html');
			$report_date = strtotime(str_replace('report-', '', str_replace('_', '-', $report_name)));
			if ($latest === FALSE || $report_date > $latest) {
				$latest = $report_date;
			}//end if
		}//end foreach

		return $latest;

	}//end _getLatest()


	/**
	* Get the report specified by the date passed in
	*
	* @param object	&$asset	The report asset
	* @param string	$date	The date of the report
	*
	* @return string
	* @access private
	*/
	private function _getReport(&$asset, $date)
	{
		$date = date('d-m-Y', $date);
		$report_path = $asset->data_path.'/report/report-'.$date.'.html';
		if (!file_exists($report_path)) {
			$report_path = '';
		}//end if

		return $report_path;

	}//end _getReport()


	/**
	* Paint the interface for viewing a report
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintReport(&$asset, &$o, $prefix)
	{
		$printed = FALSE;
		$latest = $this->_getLatest($asset);
		$all_reports_raw = glob($asset->data_path.'/report/report-*.html');	
		$all_reports = Array();
		foreach ($all_reports_raw as $report_path) {
			$report_name = basename($report_path);
			$report_date = strtotime(str_replace('.html', '', str_replace('report-', '', str_replace('_', '-', $report_name))));
			if ($report_date !== FALSE) {
				if ($report_date == $latest) {
					$all_reports[$report_date] = translate('current');
					$all_reports[$report_date] = translate('fb_sch_log_report_report_item', date('d-m-Y', $report_date));
				} else {
					$all_reports[$report_date] = translate('fb_sch_log_report_report_item', date('d-m-Y', $report_date));
				}//end if
			}//end if
		}//end foreach

		$selected = array_get_index($_POST, $prefix.'_select_report', '');

		if (!empty($all_reports)) {
			krsort($all_reports);
			combo_box($prefix.'_select_report', $all_reports, FALSE, $selected);
			submit_button($prefix.'submit_report', translate('select_report'));
			echo '<br />';
		}//end if

		// Show the selected (or latest if none selected)
		if (array_key_exists($selected, $all_reports)) {
			include_once($asset->data_path.'/report/report-'.date('d-m-Y', $selected).'.html');
			$printed = TRUE;
		} else {
			if ($latest !== FALSE) {
				$report_path = $this->_getReport($asset, $latest);
				if (is_file($report_path)) {
					include_once($report_path);
					$printed = TRUE;
				}//end if
			}//end if
		}//end if

		// No reports printed... show something
		if ($printed === FALSE) {
			echo translate('fb_sch_log_report_generate_at_next_rotation');
		}//end if

	}//end paintReport()


	/**
	* Paint the interface for regenerating the report
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintRegenerate(&$asset, &$o, $prefix)
	{
		$generated_user = $asset->attr('generated_user');
		if (empty($generated_user)) {
			echo translate('report_not_generated').'<br />';
		} else {
			echo '<p>';
			echo '    <b>'.translate('report_generated').'</b><br />';
			echo '    '.translate('date').': '.$asset->attr('generated_date');
			echo '    <br />';
			echo '    '.translate('user').': '.$asset->attr('generated_user');
			echo '</p>';
		}

		return FALSE;

	}//end paintRegenerate()


}//end class

?>
