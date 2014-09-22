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
* $Id: hipo_job_clear_cache.inc,v 1.29 2012/08/30 01:09:18 ewang Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to clear cache
*
* <pre>
* Initial Running Vars :
*    int              assetid       - the assetids with whose children we are going to attempt to clear cache
*    string           level         - 'single', 'dependants' or 'children'
*    string|array     type_codes    - the type codes to clear
*    boolean          delete_all    - whether to clear all system cache
*    boolean          repopulate    - request the asset using CURL so that cache manager can recache it
*                                     this option only comes from the clear matrix cache tool
* </pre>
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @author  Rayn Ong <rong@squiz.net>
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.29 $
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Clear_Cache extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name		a unique codename the HIPO
	*/
	function HIPO_Job_Clear_Cache($code_name='')
	{
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
		// clear cache threshhold HIPO config entry
		$o->openField(translate('Clear Cache Threshold'));


		echo '<span class="sq-inline-fields-wrapper">';
		if ($write_access) {
			text_box($class.'[SQ_HIPO_CLEAR_CACHE_THRESHOLD]', SQ_HIPO_CLEAR_CACHE_THRESHOLD, 1);
		} else {
			echo SQ_HIPO_CLEAR_CACHE_THRESHOLD;
		}
		echo ' '.translate('assets');

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
		return Array(
				'SQ_HIPO_CLEAR_CACHE_THRESHOLD'	=> Array('editable'	=> 1, 'default'	=> 100),
			   );

	}//end getConfigVars()


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* @return string
	* @access public
	*/
	function getCodeName()
	{
		if (isset($this->_running_vars['linkid'])) {
			return parent::getCodeName().$this->_running_vars['linkid'];
		} else {
			if ($this->_running_vars['delete_all'] == 'yes') {
				// clear all system cache
				return parent::getCodeName().'delete_all';
			} else {
				return parent::getCodeName().md5(serialize($this->_running_vars['assetid']));
			}
		}

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('Clear Cache');


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
					'name'			=> 'Clear Cache Table',
					'function_call'	=> Array(
										'process_function'	=> 'processClearCacheTable',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('Clearing Cache'),

					'function_call'	=> Array(
										'process_function'	=> 'processClearCacheFile',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> 'Repopulating Cache',
					'function_call'	=> Array(
										'process_function'	=> 'processRepopulate',
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
		if (SQ_HIPO_CLEAR_CACHE_THRESHOLD == 0) return 0;
		if (!isset($this->_running_vars['total'])) return 0;
		return (($this->_running_vars['total'] + 1) / SQ_HIPO_CLEAR_CACHE_THRESHOLD) * 100;

	}//end getThresholdPercentageRequired()


	/**
	* Performs the duties of this hipo, without showing any output on the frontend.
	*
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		if ($this->_running_vars['delete_all'] == 'yes') {
			// run once to clear all cache
			$this->processClearCacheTable($this->_steps[0], get_class($this));
		} else {
			while (!empty($this->_running_vars['todo_assetids'])) {
				if (!$this->processClearCacheTable($this->_steps[0], get_class($this))) {
					return FALSE;
				}
			}
		}

		// todo paths populated, so process them
		while (!empty($this->_running_vars['todo_filepaths'])) {
			if (!$this->processClearCacheFile($this->_steps[1], get_class($this))) {
				return FALSE;
			}
		}

		// repopulate
		while (!empty($this->_running_vars['todo_repopulate'])) {
			if (!$this->processRepopulate($this->_steps[2], get_class($this))) {
				return FALSE;
			}
		}

		return TRUE;

	}//end freestyle()


	/**
	* Prepares the running vars and gets a list of the cache directories in the system
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		if (empty($this->_running_vars['assetid']) && $this->_running_vars['delete_all'] == 'no') {
			trigger_localised_error('HIPO0033', E_USER_WARNING);
			return FALSE;
		}

		if (isset($this->_running_vars['delete_all']) && $this->_running_vars['delete_all'] == 'no') {

			$assetids = $this->_running_vars['assetid'];
			// Just in case some old code somewhere still passing the assetid as a string and not array.
			if (!is_array($assetids)) {
				$assetids = Array ($assetids);
			}
			if (empty($assetids)) {
				trigger_localised_error('HIPO0032', E_USER_WARNING, $this->_running_vars['assetid']);
				return FALSE;
			}//end if

			$child_assets = Array();
			$this->_running_vars['todo_assetids'] = Array();
			foreach ($assetids as $assetid) {
				switch ($this->_running_vars['level']) {
					case 'single':
						$this->_running_vars['todo_assetids'][] = $assetid;
					break;
					case 'dependants':
						$child_assets += $GLOBALS['SQ_SYSTEM']->am->getDependantChildren($assetid, $this->_running_vars['type_codes']);
						$this->_running_vars['todo_assetids'][] = $assetid;
					break;
					case 'children':
						$child_assets += $GLOBALS['SQ_SYSTEM']->am->getChildren($assetid, $this->_running_vars['type_codes']);
						$this->_running_vars['todo_assetids'][] = $assetid;
					break;
				}//end switch

			}//end foreach

			if (!empty($child_assets)) {
				$child_assets	= array_keys($child_assets);
				$this->_running_vars['todo_assetids'] = array_merge($this->_running_vars['todo_assetids'], $child_assets);
			}//end if

			$this->_running_vars['delete_all'] = FALSE;
			$this->_running_vars['total'] = count($this->_running_vars['todo_assetids']);

		} else if (isset($this->_running_vars['delete_all']) && $this->_running_vars['delete_all'] == 'yes') {
			// empty todo_assetids, since we do clearAllCache in a single step
			$this->_running_vars['todo_assetids'] = Array();
			$this->_running_vars['delete_all'] = TRUE;
			$this->_running_vars['total'] = 1;
		}

		$this->_running_vars['done_assetids'] = Array();
		$this->_running_vars['todo_filepaths'] = Array();
		$this->_running_vars['done_filepaths'] = Array();

		// note that this option only presents in tool
		// and it does not work with 'delete_all' = yes
		$this->_running_vars['todo_repopulate'] = Array();
		$this->_running_vars['done_repopulate'] = Array();
		if (isset($this->_running_vars['repopulate']) && $this->_running_vars['repopulate']) {
			$this->_running_vars['todo_repopulate'] = $this->_running_vars['todo_assetids'];
		}

		return parent::prepare();

	}//end prepare()


	/**
	* Processes the clearing of the cache entries, and populate paths to be unlinked
	*
	* @param array	&$step_data	data the data for the steps
	* @param string	$prefix		the unique prefix
	*
	* @return boolean
	* @access public
	*/
	function processClearCacheTable(&$step_data, $prefix)
	{
		$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');

		if ($this->_running_vars['delete_all'] === 'yes' || $this->_running_vars['delete_all'] === TRUE) {
			// get all unique filepaths, and clear all system cache
			$this->_running_vars['todo_filepaths'] = $cm->getAllFilePaths();
			$cm->clearAllCache();

		} else {
			// prepare all related filepaths, and clear cache for one asset
			$current_assetid = array_pop($this->_running_vars['todo_assetids']);
			$file_paths = $cm->getFilePaths(Array($current_assetid));
			$cm->clearCache(Array($current_assetid));
			$this->_running_vars['todo_filepaths'] = array_merge($this->_running_vars['todo_filepaths'], $file_paths);
			$this->_running_vars['done_assetids'][] = $current_assetid;
		}

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($cm);

		if (empty($this->_running_vars['todo_assetids'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete'] = TRUE;
		} else {
			$done = count($this->_running_vars['done_assetids']);
			$total = count($this->_running_vars['todo_assetids']) + $done;
			$step_data['percent_done'] = ($done / $total) * 100;
			$step_data['complete'] = FALSE;
		}
		return TRUE;

	}//end processClearCacheTable()


	/**
	* Processes the clearing of the actual cache files
	*
	* @param array	&$step_data	data the data for the steps
	* @param string	$prefix		the unique prefix
	*
	* @return boolean
	* @access public
	*/
	function processClearCacheFile(&$step_data, $prefix)
	{
		if (!empty($this->_running_vars['todo_filepaths'])) {
			$paths = array_splice($this->_running_vars['todo_filepaths'], 0, SQ_HIPO_CLEAR_CACHE_THRESHOLD);
			foreach ($paths as $path) {
				$path_name = SQ_CACHE_PATH.'/'.$path['path'];
				if (is_file($path_name)) {
					if (!unlink($path_name)) {
						$this->_addError('Failed to delete cache file, '.$path_name, TRUE);
					}
				} else {
					// missing cache file, probably corrupted.
					// $this->_addError('The cache file can not be found : '.$path_name, TRUE);
				}
			}
			$this->_running_vars['done_filepaths'] = array_merge($this->_running_vars['done_filepaths'], $paths);
		}

		if (empty($this->_running_vars['todo_filepaths'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete'] = TRUE;
		} else {
			$done = count($this->_running_vars['done_filepaths']);
			$total = count($this->_running_vars['todo_filepaths']) + $done;
			$step_data['percent_done'] = ($done / $total) * 100;
			$step_data['complete'] = FALSE;
		}
		return TRUE;

	}//end processClearCacheFile()


	/**
	* Repopulate the cache
	*
	* @param array	&$step_data	data the data for the steps
	* @param string	$prefix		the unique prefix
	*
	* @return boolean
	* @access public
	*/
	function processRepopulate(&$step_data, $prefix)
	{
		if (!empty($this->_running_vars['todo_repopulate'])) {
			$current_assetid = array_pop($this->_running_vars['todo_repopulate']);
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($current_assetid);
			$urls_info = $asset->getURLs();
			$urls = Array();
			if (!empty($urls_info)) {
				foreach ($urls_info as $url_info) {
					if ($url_info['http'] == '1') {
						$urls[] = 'http://'.$url_info['url'];
					}
					if ($url_info['https'] == '1') {
						$urls[] = 'https://'.$url_info['url'];
					}
				}
			}

			// Re-generate the cache per URL
			$headers = Array(
						'Cache-Control: public',
						'Pragma: cache',
					   );
			foreach ($urls as $url) {
				fetch_url($url, array('RETURNTRANSFER' => 1), $headers, array(), FALSE);
			}

			$this->_running_vars['done_repopulate'][] = $current_assetid;
		}//end if

		if (empty($this->_running_vars['todo_repopulate'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete'] = TRUE;
		} else {
			$done = count($this->_running_vars['done_repopulate']);
			$total = count($this->_running_vars['todo_repopulate']) + $done;
			$step_data['percent_done'] = ($done / $total) * 100;
			$step_data['complete'] = FALSE;
		}
		return TRUE;

	}//end processRepopulate()


}//end class
?>
