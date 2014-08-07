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
* $Id: search_log_report_edit_fns.inc,v 1.19 2012/08/30 00:59:15 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/log_report/log_report_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Search_Log_Report_Edit_Fns
*
* Purpose
*	A report that lists searches by month, week, or day, and by assetid
*
* @author Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.19 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Search_Log_Report_Edit_Fns extends Log_Report_Edit_Fns
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
	function generateReport(&$job, &$step_data, $prefix)
	{
		$job_vars =& $job->_running_vars;

		// set metadata that doesn't require the line data
		$meta['report'] =& $job_vars['report'];

		// check job_vars, and set defaults if required
		if (isset($job_vars['iterator'])) {
			$iterator =& $job_vars['iterator'];
		} else {
			$iterator =& $GLOBALS['SQ_SYSTEM']->log_manager->getLogIterator($meta['report']->logname, 1);
		}

		// default to updating the terms
		if (!isset($job_vars['mode'])) {
			$job_vars['mode'] = 'terms';
		}

		// determine the mode
		// terms - read the terms from disk, update them with data from the log, and write it back out
		if ($job_vars['mode'] == 'terms') {

			// get the next line and format the data retrieved
			// if a carryover from last step exists, use that instead.
			if (!empty($job_vars['carryover'])) {
				$line = $job_vars['carryover'];
				unset($job_vars['carryover']);
			} else {
				$line = $iterator->getNextEntry();
			}

			if (!empty($line)) {
				$data = array_get_index($line, 'data', Array());

				// prepare the metadata to hand out to the term-processing functions
				$meta['userid']         = $line['userid'];
				$meta['user']           = $line['user'];
				$meta['date']           = $line['date'];
				$meta['level']          = $line['level'];
				$meta['search_assetid'] = array_get_index($data, 'assetid', 0);

				// Get previous date from last rotated log
				if (!isset($job_vars['previous_date'])) {
					$job_vars['previous_date'] = $this->_getPreviousDate($meta['report']);
				}

				// determine whether we need to rollover to the next day/week/month before processing the line
				$rollover = FALSE;
				if (!is_null(array_get_index($job_vars, 'previous_date', NULL))) {
					switch ($job_vars['report']->attr('rollover_period')) {
						case 'daily':
							// if the date of current line is greater than the date of the previous line, rollover
							if (date('Ymd', $meta['date']) > date('Ymd', $job_vars['previous_date'])) {
								$rollover = TRUE;
							}
						break;

						case 'monthly':
							// if the month of the current line is greater than the month of previous line, rollover
							if (date('Ym', $meta['date']) > date('Ym', $job_vars['previous_date'])) {
								$rollover = TRUE;
							}
						break;

						case 'weekly':
						default:
							// get unix time for the start of next week, do as above
							$next_week_start_time = $job_vars['previous_date'] + ((7 - date('w', $job_vars['previous_date'])) * 86400);
							if (date('Ymd', $meta['date']) >= date('Ymd', $next_week_start_time)) {
								$rollover = TRUE;
							}
						break;
					}

					if ($rollover) {
						// rollover needs to be performed, switch mode over to calculating the indices
						$job_vars['carryover'] = $line;
						$job_vars['mode'] = 'indices';
					}
				}//end if

				// set the previous date in the running vars to handle rollover
				$job_vars['previous_date'] = $line['date'];

				if (!$rollover) {
					// get the terms, strip the tags, then process them
					$terms = array_get_index($data, 'terms', Array());
					foreach ($terms as $term => $result) {
						while ($term != strip_tags($term)) {
							$term = strip_tags($term);
						}
						$this->_updateTerm($term, $result, $meta);
					}
				}
			}//end if (!empty($line))

			// update the progress bar
			$progress =& $iterator->getCurrentProgress();
			$total   = array_get_index($progress, 'total', 0);
			$current = array_get_index($progress, 'current', $total);
			if ($total == 0) {
				$step_data['percent_done'] = 50;
			} else {
				$step_data['percent_done'] = (int) ($current * 100 / $total) / 2;
			}

			// finished with updating the terms, switch to calculating the indices
			if ($current >= $total) $job_vars['mode'] = 'indices';

		}//end if ($job_vars['mode'] == 'terms')


		// indices - loads all the terms stored in the term cache, ranks them
		// and strips out all but the top $report->attr('row_count') terms
		if ($job_vars['mode'] == 'indices') {

			$term_dir_path = $meta['report']->data_path.'/terms';

			// if the term_index hasn't been constructed, generate it from disk
			if (!isset($job_vars['term_index'])) {
				// construct term array
				$ids = glob($term_dir_path.'/*');
				foreach ($ids as $id) {
					$id = str_replace($term_dir_path.'/', '', $id);
					// NOTE: assumes a depth of two (terms/assetid/12/34/foobar.term)
					$terms = glob($term_dir_path.'/'.$id.'/*/*/*');
					foreach ($terms as $term) {
						$term = basename($term, '.term');
						// by keeping only the term and its associated asset ids in memory, we keep the overhead down
						$job_vars['term_index'][$term][] = $id;
					}
				}
			}

			// set up the counters for the progress bar
			if (!isset($job_vars['term_index_total'])) {
				$job_vars['term_index_total'] = count($job_vars['term_index']);
			}

			if (!isset($job_vars['term_index_count'])) {
				$job_vars['term_index_count'] = 0;
			} else {
				if ($job_vars['term_index_count'] >= $job_vars['term_index_total']) {
					$job_vars['term_index_count'] = $job_vars['term_index_total'];
				} else {
					$job_vars['term_index_count']++;
				}
			}

			// process the next term on the list
			if (!empty($job_vars['term_index'])) {

				$term = end(array_keys($job_vars['term_index']));
				$ids  = array_pop($job_vars['term_index']);

				foreach ($ids as $id) {
					$data = $this->_readTerm($term, $term_dir_path.'/'.$id.'/'.$this->_getTermHash($term));
					if ($data) {
						foreach ($data as $date => $results) {
							// make sure the indices are set
							if (!isset($job_vars['indices'][$id][$date]['index_popular'])) {
								$job_vars['indices'][$id][$date]['index_popular'] = Array();
							}
							if (!isset($job_vars['indices'][$id][$date]['index_failed'])) {
								$job_vars['indices'][$id][$date]['index_failed'] = Array();
							}
							$row_count = $meta['report']->attr('row_count');

							// update the indices
							$total_value = array_get_index($results, 'total_searches', 0);
							$zero_value = array_get_index($results, 'zero_results', 0);
							if ($zero_value) {
								$this->_updateIndex($job_vars['indices'][$id][$date]['index_failed'], $term, $zero_value, $row_count);
							} else {
								$this->_updateIndex($job_vars['indices'][$id][$date]['index_popular'], $term, $total_value, $row_count);
							}
						}
					}
				}
				// remove the term from the list
				unset($job_vars['term_index'][$term]);
			} else {
				// we're finished, move onto the next step
				unset($job_vars['term_index']);
				$job_vars['mode'] = 'generate';
			}

			// set the progress bar
			$step_data['percent_done'] = (($job_vars['term_index_count'] * 100 / $job_vars['term_index_total']) / 2) + 50;

		}//end if ($job_vars['mode'] == 'indices')


		// generate - generate the report cache
		if ($job_vars['mode'] == 'generate') {

			$result = $this->_generateReportCache($meta, $job_vars['indices']);

			$progress =& $iterator->getCurrentProgress();
			$total   = array_get_index($progress, 'total', 0);
			$current = array_get_index($progress, 'current', $total);

			if ($current >= $total) {
				// we're done
				$step_data['percent_done'] = 100;
				$step_data['complete']     = TRUE;
			} else {
				// the only time we'd be here is if we haven't yet finished reading
				// the file and need to rollover
				$job_vars['mode'] = 'rollover';
			}
		}//end if($job_vars['mode'] == 'generate')


		// rollover - archive the current report so that we can start writing another one for the new day/week/month
		if ($job_vars['mode'] == 'rollover') {
			$this->_performRollover($meta['report'], $job_vars['previous_date']);
			$job_vars['indices'] = Array();

			$progress =& $iterator->getCurrentProgress();
			$total   = array_get_index($progress, 'total', 0);
			$current = array_get_index($progress, 'current', $total);
			if ($current >= $total && !isset($job_vars['carryover'])) {
				$step_data['percent_done'] = 100;
				$step_data['complete']     = TRUE;
			} else {
				// More terms to process, switch back to 'terms'
				$job_vars['mode'] = 'terms';
			}
		}//end if($job_vars['mode'] == 'rollover')


		$job_vars['iterator'] =& $iterator;
		$job_vars['complete'] = $step_data['complete'];

		return TRUE;

	}//end generateReport()


	/**
	* Updates the stats for a specific term
	*
	* The metadata matches the same format as that of the log_manager,
	* except for the addition of 'search_assetid', and the lack of 'data':
	* <pre>
	* Array(
	*	'userid'			=> int					// the id of the logging user
	*	'user'				=> string				// the name of the logging user
	*	'date'				=> int					// the date the line was logged
	*	'level'				=> int					// the level of the error
	*	'search_assetid'	=> string				// the assetid of the search page (optional)
	*	'report'			=> object Log_Report	// the search_log_report
	* );
	*</pre>
	*
	* @param string	$term	the term to update
	* @param int	$result	the numbers of results for this term
	* @param array	&$meta	the metadata array
	*
	* @return boolean
	* @access private
	*/
	function _updateTerm($term, $result, &$meta)
	{
		if (empty($term) || empty($meta)) {
			return FALSE;
		}

		$search_assetid = array_get_index($meta, 'search_assetid', '0');
		$meta_date      = date('Y-m-d H', $meta['date']);

		// include the current assetid as well the grand total
		$assetids = Array(
						$search_assetid,
						'total',
					);

		// same for dates
		$dates = Array(
					date('Y-m-d H', $meta['date']),
					'total',
				 );

		foreach ($assetids as $id) {
			$term_dir_path = $meta['report']->data_path.'/terms'.('/'.$id).$this->_getTermHash($term);

			// read, update and write for the specific assetid or total
			$data_collection[$id] = $this->_readTerm($term, $term_dir_path);
			foreach ($dates as $date) {
				// prepare data, and update the value for each term
				if (!isset($data_collection[$id][$date])) {
					$data_collection[$id][$date] = Array();
				}
				$this->_updateTermValues($term, $result, $data_collection[$id][$date]);
			}
			// refresh the term cache with the new data
			$this->_writeTerm($term, $term_dir_path, $data_collection[$id]);
		}

		return TRUE;

	}//end _updateTerm()


	/**
	* Updates the values for a term
	*
	* @param string	$term	the term to update
	* @param int	$result	the result for the term
	* @param array	&$data	the array containing data about the searches
	*
	* @return boolean
	* @access private
	*/
	function _updateTermValues($term, $result, &$data)
	{
		if (empty($term)) return FALSE;

		// update the stats, or create new ones if not already present
		$total_searches = array_get_index($data, 'total_searches', 0) + 1;

		if ($result == 0) {
			$zero_results = array_get_index($data, 'zero_results', 0) + 1;
		} else {
			$zero_results = array_get_index($data, 'zero_results', 0);
		}

		if ($total_searches > 0) {
			$average_result = ((array_get_index($data, 'average_result', 0) * ($total_searches - 1)) + $result) / $total_searches;
		} else {
			$average_result = $result;
		}

		if (is_null(array_get_index($data, 'max_result', NULL))) {
			$max_result = $result;
		} else {
			$max_result = ($result > $data['max_result'] ? $result : $data['max_result']);
		}

		if (is_null(array_get_index($data, 'min_result', NULL))) {
			$min_result = $result;
		} else {
			$min_result = ($result < $data['min_result'] ? $result : $data['min_result']);
		}

		$data = Array(
					'total_searches'	=> $total_searches,
					'zero_results'		=> $zero_results,
					'average_result'	=> round($average_result, 2),
					'max_result'		=> $max_result,
					'min_result'		=> $min_result,
				);

		return TRUE;

	}//end _updateTermValues()


	/**
	* Reads and returns the lines taken from a cache of term results
	*
	* @param string	$term			the term to read
	* @param string	$term_dir_path	the path to the term
	*
	* @return array
	* @access private
	*/
	function &_readTerm($term, $term_dir_path)
	{
		if (empty($term) || empty($term_dir_path)) {
			return Array();
		}

		$term_path = $term_dir_path.'/'.$term.'.term';

		// dump the contents of the file into an array, unserialising as we go
		$data = Array();
		if (file_exists($term_path)) {
			$fp = fopen($term_path, 'r');
			if ($fp) {
				while (!feof($fp)) {
					$data = unserialize(fgets($fp));
				}
				fclose($fp);
			}
		}

		return $data;

	}//end _readTerm()


	/**
	* Writes the updated results to the term's cache file
	*
	* @param string	$term			the term to write
	* @param string	$term_dir_path	the path to the term
	* @param array	&$data			the data to write
	*
	* @return boolean
	* @access private
	*/
	function _writeTerm($term, $term_dir_path, &$data)
	{
		if (empty($term) || empty($term_dir_path) || empty($data)) {
			return FALSE;
		}

		// create the directories as we need them
		if (!is_dir($term_dir_path) && !create_directory($term_dir_path)) {
			return FALSE;
		}
		// bug fix 3587 converting slashes in the search term (if any) to underscore
		// so that file system doesnt take it as directory structure
		$term = str_replace('/' , '_' , $term);
		$term = urlencode($term);
		$term = substr_replace($term, '', 250, strlen($term));

		$term_path = $term_dir_path.'/'.$term.'.term';

		// dump the into the file, serialising as we go
		$fp = fopen($term_path, 'w');
		if ($fp) {
			fwrite($fp, serialize($data));
			fclose($fp);
			return TRUE;
		}

		return FALSE;

	}//end _writeTerm()


	/**
	* Updates a specified index in a passed array, but only keeps the top $limit results.
	* NOTE: Assumes that $array is already sorted in DESCENDING order
	*
	* @param array	&$array	the array containing the values to update
	* @param string	$index	the index to update
	* @param int	$value	the value to set the index to
	* @param int	$limit	the number of elements to limit the array to
	*
	* @return boolean
	* @access private
	*/
	function _updateIndex(&$array, $index, $value, $limit)
	{
		if (empty($index) || $value <= 0 || $limit < 1) {
			return FALSE;
		}

		// if the element already exists, or the array isn't big enough, add the element
		if (isset($array[$index]) || count($array) < $limit) {
			$array[$index] = $value;
		} else {
			// assume at least one element in the array
			// calculate threshold
			$threshold = end($array);
			reset($array);

			// if the value is above what we need, pop off the last and add the new element.
			if ($value > $threshold) {
				array_pop($array);
				$array[$index] = $value;
			}
		}

		// re-sort
		arsort($array);
		return TRUE;

	}//end _updateIndex()


	/**
	* Generates the report cache from the search-term cache
	*
	* @param array	&$meta		the metadata array
	* @param array	&$indices	the indices array
	*
	* @return boolean
	* @access private
	*/
	function _generateReportCache(&$meta, &$indices)
	{
		if (empty($meta)) return FALSE;

		$logname            = $meta['report']->logname;
		$data_path          = $meta['report']->data_path;
		$term_dir_path      = $data_path.'/terms';
		$generated_dir_path = $data_path.'/generated';

		if (empty($data_path)) return FALSE;

		// get all the assetids in the terms
		$assetids = $this->_getSearchAssetids($term_dir_path);
		if (empty($assetids)) {
			$assetids = Array(
							'total',
						);
		}


		// loop through the asset ids, using the indices to prepare the output containing the top results
		foreach ($assetids as $id) {

			if (!isset($indices[$id]['total']['index_popular'])) {
				$indices[$id]['total']['index_popular'] = Array();
			}
			if (!isset($indices[$id]['total']['index_failed'])) {
				$indices[$id]['total']['index_failed']  = Array();
			}

			$index_popular = $indices[$id]['total']['index_popular'];
			$index_failed  = $indices[$id]['total']['index_failed'];

			// popular searches
			$output_index_popular = '';
			foreach ($index_popular as $term => $result) {
				$data_collection = $this->_readTerm($term, $term_dir_path.('/'.$id).$this->_getTermHash($term));
				if (isset($data_collection['total'])) {
					$data = $data_collection['total'];
					$output_index_popular .= '
						<tr>
							<td class="sq-backend-table-cell">'.urldecode($term).'</td>
							<td class="sq-backend-table-cell" style="width: 25px;">'.array_get_index($data, 'total_searches', 0).'</td>
							<td class="sq-backend-table-cell" style="width: 25px;">'.array_get_index($data, 'average_result', 0).'</td>
							<td class="sq-backend-table-cell" style="width: 25px;">'.array_get_index($data, 'min_result', 0).'</td>
							<td class="sq-backend-table-cell" style="width: 25px;">'.array_get_index($data, 'max_result', 0).'</td>
						</tr>
					';
				}
			}

			// failed searches
			$output_index_failed = '';
			foreach ($index_failed as $term => $result) {
				$output_index_failed .= '
					<tr>
						<td class="sq-backend-table-cell">'.urldecode($term).'</td>
						<td class="sq-backend-table-cell" style="width: 25px;">'.$result.'</td>
					</tr>
				';
			}


			// put it all together
			if ($id == 'total') {
				$output_heading = '<b>'.translate('sch_log_report_all_search_pages').'</b>';
			} else {
				if ($GLOBALS['SQ_SYSTEM']->am->assetExists($id)) {
					$asset = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($id), Array(), FALSE, 'name');
					$asset = $asset[$id];
				}

				if (!$asset) $asset = 'Asset';

				$output_heading = '<b>'.$asset.' (Id: #'.$id.')</b>';
			}
			$output[$id] = $output_heading.'

			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header" style="width: 200px">'.translate('sch_log_report_popular_search_terms').'</td>
					<td class="sq-backend-table-header">'.translate('sch_log_report_total_searches').'</td>
					<td class="sq-backend-table-header">'.translate('sch_log_report_average_results').'</td>
					<td class="sq-backend-table-header">'.translate('lowest').'</td>
					<td class="sq-backend-table-header">'.translate('highest').'</td>
				</tr>
				'.(!empty($output_index_popular) ? $output_index_popular : '<td class="sq-backend-table-cell" colspan="5">'.translate('sch_log_report_no_successful_searches').'</td>').'
			</table>

			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header" style="width: 200px">'.translate('sch_log_report_failed_search_terms').'</td>
					<td class="sq-backend-table-header">'.translate('sch_log_report_total_failed_searches').'</td>
				</tr>
				'.(!empty($output_index_failed) ? $output_index_failed : '<td class="sq-backend-table-cell" colspan="2">'.translate('sch_log_report_no_failed_searches').'</td>').'
			</table>
			<br />
			';

			// make sure we have somewhere to write to
			if (!file_exists($generated_dir_path)) {
				if (!create_directory($generated_dir_path)) {
					return FALSE;
				}
			}

		}//end foreach

		// make sure we always print the total first
		$report_output  = '
		'.$output['total'].'
		';

		unset($output['total']);
		$report_output .= implode("\n", $output);

		$result = string_to_file($report_output, $generated_dir_path.'/'.$logname.'.html');
		if ($result) {
			$meta['report']->setAttrValue('generated_date', date('F j, Y, g:i a'));
			$meta['report']->setAttrValue('generated_user', $GLOBALS['SQ_SYSTEM']->user->name.' (#'.$GLOBALS['SQ_SYSTEM']->user->id.')');

			$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
			$meta['report']->saveAttributes();
			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
		}
		return $result;

	}//end _generateReportCache()


	/**
	* Moves the generated report and updates the generated_reports attribute
	*
	* @param object	&$report	the report to use when determining the destination path, and saving settings
	* @param int	$date		the date that refers to the report and cache
	*
	* @return boolean
	* @access private
	*/
	function _performRollover(&$report, $date)
	{
		if (empty($report) || empty($date)) {
			return FALSE;
		}

		$formatted_date     = date('Y-m-d', $date);
		$logname            = $report->logname;
		$data_path          = $report->data_path;
		$term_dir_path      = $data_path.'/terms';
		$generated_dir_path = $data_path.'/generated';

		// remove term cache
		if (!delete_directory($term_dir_path)) {
			trigger_localised_error('SCH0025', E_USER_WARNING);
			return FALSE;
		}

		// move report to archives
		$from = $generated_dir_path.'/'.$logname.'.html';
		$to   = $generated_dir_path.'/'.$logname.'.'.$formatted_date.'.html';
		if (!rename($from, $to)) {
			trigger_localised_error('SCH0024', E_USER_WARNING);
			return FALSE;
		}

		// update settings
		$generated_reports = $report->attr('generated_reports');
		$generated_reports[$logname.'.'.$formatted_date] = $formatted_date.' ('.ucwords($report->attr('rollover_period')).')';
		arsort($generated_reports);

		$report->setAttrValue('generated_reports', $generated_reports);
		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
		$report->saveAttributes();
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

		return TRUE;

	}//end _performRollover()


	/**
	* Returns a list of the search_page assets that have logged search results
	*
	* @param string	$term_dir_path	the path to the terms
	*
	* @return array
	* @access private
	*/
	function _getSearchAssetids($term_dir_path)
	{
		if (empty($term_dir_path)) return FALSE;

		$dirs = Array();
		if (is_dir($term_dir_path)) {
			if ($dir_handle = opendir($term_dir_path)) {
				while (($file = readdir($dir_handle)) !== FALSE) {
					if ($file != '.' && $file != '..' && is_dir($term_dir_path.'/'.$file)) {
						$dirs[] = $file;
					}
				}
			}
		}
		return $dirs;

	}//end _getSearchAssetids()


	/**
	* Hashes a term into the format '/12/34', where the numbers are the ascii
	* values for the first two characters of the term
	*
	* @param string	$term	the term to hash
	*
	* @return mixed boolean|string
	* @access private
	*/
	function _getTermHash($term)
	{
		if (trim($term) == '') return FALSE;
		$term = urldecode($term);

		// how many directory levels to hash to (eg. 2 = '/12/34')
		$depth = 2;
		$hash_num = Array();
		for ($ii = 0; $ii < $depth; $ii++) {
			// if the word isn't long enough, pad any missing characters with 0
			if (strlen($term) > $ii) {
				$hash_num[] .= ord($term[$ii]);
			} else {
				$hash_num[] .= 0;
			}
		}

		$hash = implode('/', $hash_num);

		return '/'.$hash;

	}//end _getTermHash()


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
	function paintReport(&$asset, &$o, $prefix)
	{
		$generated_reports = $asset->attr('generated_reports');

		// there will always be a current report, so add it here
		$generated_reports[$asset->logname] = 'Current';
		arsort($generated_reports);

		if (isset($_REQUEST[$prefix]['generated_reports'])) {
			if (isset($generated_reports[$_REQUEST[$prefix]['generated_reports']])) {
				$report_path = $asset->data_path.'/generated/'.$_REQUEST[$prefix]['generated_reports'].'.html';
				$selected = $_REQUEST[$prefix]['generated_reports'];
			} else {
				$report_path = $asset->data_path.'/generated/'.$asset->logname.'.html';
				$selected = $asset->logname;
			}
		} else {
			$report_path = $asset->data_path.'/generated/'.$asset->logname.'.html';
			$selected = $asset->logname;
		}

		if (!is_file($report_path)) {
			echo translate('sch_log_report_generate_at_next_rotation');
			return;
		}

		echo '<h3>'.translate('sch_log_report_search_report').'</h3>';

		// print the report selector
		combo_box($prefix.'[generated_reports]', $generated_reports, FALSE, $selected);
		echo '&nbsp;';
		submit_button($prefix.'[select_report]', translate('select_report'));
		echo '<br /><br />';

		include_once($report_path);

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
	function paintRegenerate(&$asset, &$o, $prefix)
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


	/**
	* Get previous date from last rotated search report, if any
	*
	* @param object $report
	*
	* @return int|null
	* @accesss private
	*/
	function _getPreviousDate($report)
	{
		$generated_reports = $report->attr('generated_reports');
		$latest_report = current(array_keys($generated_reports));

		$last_date = NULL;
		if ($latest_report) {
			$last_date = strtotime(substr($latest_report, 7));
		}

		return $last_date;

	}//end _getPreviousDate()


}//end class

?>
