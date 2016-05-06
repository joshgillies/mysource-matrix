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
* $Id: hipo_job_regenerate_metadata.inc,v 1.39 2012/08/30 01:09:19 ewang Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to regenerate canned data on
* an Google Analytics View asset on demand
*
* Initial Running Vars :
*    assetid	-	the ID of the asset that is been updated
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.39 $
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Refresh_Google_Analytics_View extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name		a unique codename the HIPO
	*/
	function HIPO_Job_Regenerate_Metadata($code_name='')
	{
		$this->uses_trans = FALSE;
		$this->HIPO_Job($code_name);

	}//end constructor


	/**
	* Paints backend of this job
	*
	* @param object	&$o		reference to the backend outputter
	* @param string	$class	calling class name
	*
	* @return void
	* @access private
	*/
	public static function paintConfig(&$o, $class, $write_access)
	{
		// metadata regeneration threshhold HIPO config entry
		$o->openField(translate('Google Analytics Query Threshold'));


		echo '<span class="sq-inline-fields-wrapper">';
		if ($write_access) {
			text_box($class.'[SQ_HIPO_REFRESH_GA_DATA_THRESHOLD]', SQ_HIPO_REFRESH_GA_DATA_THRESHOLD, 4);
		} else {
			echo SQ_HIPO_REFRESH_GA_DATA_THRESHOLD;
		}
		echo ' '.translate('queries');

		echo '</span>';

		$o->closeField();

	}//end paintConfig()


	/**
	* Sets some info about thresholds that this hipo requires.
	*
	* @return array
	* @access public
	*/
	public static function getConfigVars()
	{
		return Array('SQ_HIPO_REFRESH_GA_DATA_THRESHOLD' => Array('editable' => 1, 'default' => 5));

	}//end getConfigVars()


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* The Regenerate Metdata HIPO appends the ID of the schema that was updated so that any other
	* regeneration jobs for this schema do not start while it is running.
	*
	* @return string
	* @access public
	*/
	function getCodeName()
	{
		return parent::getCodeName().'-'.$this->_running_vars['assetid'];

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('Refresh Google Analytics View');


	}//end getHipoName()


	/**
	* Returns the steps in this hipo, possibly with localised step names
	* and messages
	*
	* @return string
	* @access public
	*/
	function getInitialStepData()
	{
		return Array(
				Array(
					'name'			=> translate('Refreshing Analytics Data'),

					'function_call'	=> Array(
										'process_function'	=> 'refreshGAData',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
			   );

	}//end getInitialStepData()


	/**
	* Returns the current Threshold.
	*
	* Returns a value that represents the total percentage that this hipo would run in freestyle mode in its current state.
	*
	* @return float
	* @access public
	*/
	function getThresholdPercentageRequired()
	{
		if (SQ_HIPO_REFRESH_GA_DATA_THRESHOLD == 0) {
			return 0;
		}
		if (!isset($this->_running_vars['todo_queries'])) {
			return 0;
		}
		return ((count($this->_running_vars['todo_queries']) / SQ_HIPO_REFRESH_GA_DATA_THRESHOLD) * 100);

	}//end getThresholdPercentageRequired()


	/**
	* Performs the duties of this hipo, without showing any output on the frontend.
	*
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		while ($this->_steps[0]['complete'] === FALSE) {
			if (!$this->refreshGAData($this->_steps[0], get_class($this))) {
				return FALSE;
			}
		}
		return TRUE;

	}//end freestyle()


	/**
	* Prepares the running vars and gets the children of the asset in question.
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		// to start regenerating metadata, we need to have a schema that has been updated
		if (empty($this->_running_vars['assetid'])) {
			trigger_localised_error('GANA0003', translate('Unable to initialise Refresh Google Analytics View HIPO, no asset ID specified'), E_USER_WARNING);
			return FALSE;
		}

		$viewid  = $this->_running_vars['assetid'];
		$view    = $GLOBALS['SQ_SYSTEM']->am->getAsset($viewid);

		if ($view instanceof Google_Analytics_View === FALSE) {
			trigger_localised_error('GANA0004', translate('Unable to initialise Refresh Google Analytics View HIPO, asset is not a Google Analytics View'), E_USER_WARNING);
			return FALSE;
		}

		$queries = $this->_getQueries($view);
		$this->_running_vars['todo_queries'] = $queries;
		$this->_running_vars['query_count']  = count($queries);

		// if nothing to do, bail out
		if (empty($this->_running_vars['todo_queries'])) {
			return FALSE;
		}

		return parent::prepare();

	}//end prepare()


	/**
	* Regenerate Metadata for a single asset in the todo list
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function refreshGAData(&$step_data, $prefix)
	{
		$pct_complete = 0;
		
		$viewid = $this->_running_vars['assetid'];
		$view   = $GLOBALS['SQ_SYSTEM']->am->getAsset($viewid);
	
		if (count($this->_running_vars['todo_queries']) === 0) {
			$conn = $view->getConnector();
			$conn->replaceFetchCache('tmp');
			$pct_complete = 100;
		} else {
			$viewid = $this->_running_vars['assetid'];
			$query  = array_shift($this->_running_vars['todo_queries']);
			$queryid = $query['queryid'];
			unset($query['queryid']);
			$view->getCannedReport($queryid, $query, FALSE, 'tmp');
			$pct_complete = ($this->_running_vars['query_count'] - count($this->_running_vars['todo_queries'])) / ($this->_running_vars['query_count'] + 1) * 100;
		}

		$step_data['percent_done'] = floor($pct_complete);
		if ($pct_complete === 100) {
			$step_data['complete'] = TRUE;
		} else {
			$friendly_context = $query['context'];
			if ($friendly_context === 'all') {
				$friendly_context = translate('Admin Mode');
			}

			$step_data['complete'] = FALSE;

			if (array_key_exists('duration', $query) === TRUE) {
				$friendly_duration = substr($query['duration'], 1, -1);
				$step_data['message']  = sprintf(translate('Refreshing %s-month %s data for %s (Id: #%s)'), $friendly_duration, $friendly_context, $view->attr('name'), $view->id);
			} else {
				$step_data['message']  = sprintf(translate('Refreshing %s data for %s (Id: #%s)'), $friendly_context, $view->attr('name'), $view->id);
			}
		}

		return TRUE;

	}//end refreshGAData()


	/**
	 * Get the query list for a certain view asset.
	 *
	 * Returns an array of query data that can then be run through
	 * generateReport().
	 *
	 * @param Google_Analytics_View $view The view asset.
	 *
	 * @return array
	 */
	private function _getQueries(Google_Analytics_View $view)
	{
		$queries = Array();

		// Get the list of lookups for this site.
		$lookups = Array();
		$sites = $GLOBALS['SQ_SYSTEM']->am->getParents($view->id, 'site', FALSE);
		foreach ($sites as $site_assetid => $this_site) {
			$site = $GLOBALS['SQ_SYSTEM']->am->getAsset($site_assetid);
		
			$site_lookups = $site->getLookups();
			foreach ($site_lookups as $site_lookup) {
				$lookups[] = $site_lookup['url'];
			}
		
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($site);
			unset($site);
		}

		$base_query = Array(
		    'assetid' => $view->id,
		    'context' => 'site',
		);

		$periods = Array(
			'P1M',
			'P3M',
			'P6M',
		);
		
		$site_queries = Array(
            'current',
            'previous',
            'campaigns',
            'page_path',
            'previous_path',
            'by_day',
            'by_day_alt',
            'device',
            'referral_pages',
            'browser',
            'os',
            'device_daily',
            'most_viewed_device',
            'location',
            'language',
            'orgs',
		);

		foreach ($lookups as $lookup) {
			$base_query['url'] = $lookup;

			foreach ($periods as $period) {
				$base_query['duration'] = $period;

				foreach ($site_queries as $queryid) {
					$this_query = $base_query;
					$this_query['queryid'] = $queryid;
					$queries[] = $this_query;
				}//end foreach site query
			}//end foreach duration
		}//end foreach lookup

		foreach ($periods as $period) {
			foreach ($site_queries as $queryid) {
				$queries[] = Array(
					'assetid' => $view->id,
					'queryid' => $queryid,
					'context' => 'all',
					'duration' => $period,
				);
			}//end foreach site query

			$queries[] = Array(
				'assetid' => $view->id,
				'queryid' => 'goal_summary',
				'context' => 'all',
				'duration' => $period,
			);
		}//end foreach period

		// Get goal queries.
		$goal_queries = Array(
			'current',
			'previous',
			'campaigns',
			'previous_path',
			'by_day',
			'device',
			'page_path',
			'page_path_conversions',
		);

		$goal_all_queries = Array(
			'goal_summary',
			'goal_summary_previous',
			'current',
			'previous',
			'device',
			'browser',
			'os',
			'location',
			'campaigns',
			'most_viewed_device',
		);

		unset($base_query['url']);

		$goals = $view->getGoals();
		foreach ($periods as $period) {
			$base_query['duration'] = $period;
			unset($base_query['start_period']);

			foreach ($goal_all_queries as $queryid) {
				$this_query = $base_query;
				if ($queryid === 'goal_summary_previous') {
					$queryid = 'goal_summary';
					$this_query['start_period'] = 'previous';
				}
				$this_query['context'] = 'all';
				$this_query['queryid'] = $queryid;
				$queries[] = $this_query;
			}//end foreach goal "all" query

			foreach ($goals as $goal) {
				foreach ($goal_queries as $queryid) {
					$this_query = $base_query;
					$this_query['context'] = 'goal';			
					$this_query['goalid']  = $goal['id'];
					$this_query['queryid'] = $queryid;
					$queries[] = $this_query;
				}//end foreach goal query
			}//end foreach goal

			foreach ($goal_queries as $queryid) {
				$this_query = $base_query;
				$this_query['context'] = 'goal';			
				$this_query['goalid']  = 'all';
				$this_query['queryid'] = $queryid;
				$queries[] = $this_query;
			}//end foreach goal query
		}//end foreach duration

		// Experiment queries.
		$experiment_queries = Array(
			'current',
			'previous',
			'campaigns',
			'previous_path',
			'by_day',
			'device'
		);
		
		// Unset duration because we want the period where
		// the experiment was active.
		unset($base_query['duration']);
		$experiments = $view->getExperiments();

		foreach ($experiments as $experiment) {
			if ($experiment['status'] === 'RUNNING' || $experiment['status'] === 'ENDED') {
				// Only get experiment stats if the experiment is actually running.
				// TODO:
				foreach ($experiment_queries as $queryid) {
					$this_query = $base_query;
					$this_query['context'] = 'experiment';			
					$this_query['experimentid']  = $experiment['id'];
					$this_query['queryid'] = $queryid;
					$queries[] = $this_query;
				}//end foreach goal query
			}
		}

		return $queries;

	}//end getQueries()


}//end class

?>
