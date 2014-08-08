<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: hipo_job_tool_clear_squid_cache.inc,v 1.18 2013/09/05 01:52:20 ewang Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to clear squid cache
*
* Initial Running Vars :
*	assetid		-	the id of the asset which URLs to be purged
*	cascade		-	this asset only, or include its children
*	repopulate	-	whether to repopulate the cache
*
*
* @author Rayn Ong <rong@squiz.net>
* @version $Revision: 1.18 $
* @package MySource_Matrix
* @subpackage squid
*/
class HIPO_Job_Tool_Clear_Squid_Cache extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name		a unique codename the HIPO
	*
	*/
	function __construct($code_name='')
	{
		$this->uses_trans = FALSE;
		parent::__construct($code_name);
		$GLOBALS['SQ_SYSTEM']->lm->includePackageStrings('squid');

	}//end constructor


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* The Regenerate Metdata HIPO appends the ID of the schema that was updated so that any other
	* regeneration jobs for this schema do not start while it is running.
	*
	* @return string
	* @access public
	*/
	public function getCodeName()
	{
		return parent::getCodeName().'-'.$this->_running_vars['assetid'];

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	public function getHipoName()
	{
		return translate('squid_hipo_name_clear_squid_cache');

	}//end getHipoName()


	/**
	* Returns the steps in this hipo, possibly with localised step names
	* and messages
	*
	* @return string
	* @access public
	*/
	public function getInitialStepData()
	{
		return Array(
				Array(
					'name'			=> translate('squid_hipo_clearing_squid_cache'),
					'function_call'	=> Array(
										'process_function'	=> 'processClearCache',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
					'allow_cancel'	=> TRUE,
				),
			   );

	}//end getInitialStepData()


	/**
	* Performs the duties of this hipo, without showing any output on the frontend.
	*
	* @return boolean
	* @access public
	*/
	public function freestyle()
	{
		while (!empty($this->_running_vars['todo_urls'])) {
			if (!$this->processClearCache($this->_steps[0], get_class($this))) {
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
	public function prepare()
	{
		require_once SQ_DATA_PATH.'/private/conf/tools.inc';

		// get the prefs setting for squidclient
		$this->_running_vars['hostnames'] = explode("\n", trim(SQ_TOOL_SQUID_CACHE_HOSTNAMES));
		$this->_running_vars['path'] = SQ_TOOL_SQUID_CACHE_PATH;
		if ($this->_running_vars['path'] == '') {
			$this->_running_vars['path'] = '/usr/bin/squidclient';
		}
		$this->_running_vars['port'] = SQ_TOOL_SQUID_CACHE_PORT;
		if ($this->_running_vars['port'] != '') {
			$this->_running_vars['port'] = '-p '.$this->_running_vars['port'];
		}
		$this->_running_vars['option'] = SQ_TOOL_SQUID_CACHE_OPTION;
		$this->_running_vars['slash'] = SQ_TOOL_SQUID_CACHE_SLASH;
		$this->_running_vars['url_port'] = SQ_TOOL_SQUID_URL_PORT;

		// For backward compatibility
		if ($this->_running_vars['cascade'] === '1' || $this->_running_vars['cascade'] === TRUE ) {
			$this->_running_vars['cascade'] = 'children';
		} else if ($this->_running_vars['cascade'] === '0' || $this->_running_vars['cascade'] === FALSE ) {
			$this->_running_vars['cascade'] = 'single';
		}
		

		// if cascade option is enabled, get the children of the selected asset
		$assetids = is_array($this->_running_vars['assetid']) ? $this->_running_vars['assetid'] : Array($this->_running_vars['assetid']);
		$type_codes = isset($this->_running_vars['type_codes']) ? $this->_running_vars['type_codes'] : NULL;
		if ($this->_running_vars['cascade'] !== 'single') {
			foreach($assetids as $assetid) {
				if ($this->_running_vars['cascade'] === 'children') {
					$children = array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($assetid, $type_codes));
				} else if ($this->_running_vars['cascade']) {
					$children = array_keys($GLOBALS['SQ_SYSTEM']->am->getDependantChildren($assetid, $type_codes));
				}
				$assetids = array_merge($assetids, $children);
			}//end foreach
		}//end if

		$assetids = array_unique($assetids);

		// add urls applied to each asset into the todo array
		$this->_running_vars['todo_urls'] = Array();
		foreach ($assetids as $assetid) {
			$result = $GLOBALS['SQ_SYSTEM']->am->getURLs($assetid);
			$this->_running_vars['todo_urls'] = array_merge($result, $this->_running_vars['todo_urls']);
		}
		$this->_running_vars['done_urls'] = Array();

		return parent::prepare();

	}//end prepare()


	/**
	* Clears the Squid cache for all URLs applied to the selected asset
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	public function processClearCache(Array &$step_data, $prefix)
	{
		if (!empty($this->_running_vars['todo_urls'])) {

			$hostnames = $this->_running_vars['hostnames'];
			$path = $this->_running_vars['path'];
			$port = $this->_running_vars['port'];
			$options = $this->_running_vars['option'];
			$cascade = $this->_running_vars['cascade'];
			$index = array_shift(array_keys($this->_running_vars['todo_urls']));
			$urls = Array($this->_running_vars['todo_urls'][$index]['url']);


			// For the top level URL, include the one with trailing slash too (bug #5996)
			if ($this->_running_vars['slash'] || strpos($this->_running_vars['todo_urls'][$index]['url'], '/') === FALSE) {
				// include the same url, with a trailing slash
				$urls[] = $this->_running_vars['todo_urls'][$index]['url'].'/';
			}
			$protocols = Array();
			if ($this->_running_vars['todo_urls'][$index]['http']) {
				$protocols[] = 'http';
			}
			if ($this->_running_vars['todo_urls'][$index]['https']) {
				$protocols[] = 'https';
			}

			$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();

			foreach ($hostnames as $hostname) {

				$hostname = trim($hostname);
				foreach ($urls as $url) {
					foreach ($protocols as $protocol) {
						$output = Array();

						if ($this->_running_vars['url_port'] <> '') {

							// Explode into an array based on the first slash it finds (the '2' in the third
							// parameter means only two array elements will be created)
							$path_parts = explode('/', $url, 2);

							// Add the port to the host name
							$path_parts[0] .= ':'.$this->_running_vars['url_port'];

							// Put it back together again
							$url = implode('/', $path_parts);

						} //end if

						$system_log_command = "$path -h $hostname $port $options -m PURGE $protocol://$url";
						error_log('Running: '.$system_log_command);
						exec(escapeshellcmd($system_log_command), $output, $status);

						// error_log("\n\t"."$path -h $hostname $port $options -m PURGE $protocol://$url \n". serialize($output)."\n");
						if (empty($output)) {
							trigger_localised_error('SQUID0002', E_USER_WARNING);
						} else {
							// add a system log message if successful
							$msg = $ms->newMessage();
							$msg->type = 'purge.squid.cache';
							$msg->from = 0; // a system message

							$command_str = $system_log_command;
							$repopulate = ($this->_running_vars['repopulate']) ? translate('yes') : translate('no');
							$command_str .= "\nRepopulate Cache: ".$repopulate;
							$cascade = ($this->_running_vars['cascade']) ? translate('yes') : translate('no');
							$command_str .= ', Cascade: '.$cascade;
							$system_log_output = "Output : \n";
							foreach ($output as $key=>$line) {
								$system_log_output .= "$line\n";
							}
							$msg_reps = Array(
											'command'	=> $command_str,
											'result'	=> $system_log_output,
										);

							$msg->replacements = $msg_reps;
							$ms->send(Array($msg));
						}//end if

					}//end foreach - protocols

				}//end foreach - urls

			}//end foreach - squid hostnames

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($ms);

			// re-request the page to recache it
			if ($this->_running_vars['repopulate']) {
				foreach ($urls as $url) {
					$this->requestPage("$protocol://$url", $hostname, $port);
				}
			}

			// add this url to the done array
			$step_data['message'] = translate('squid_clearing_squid_cache', $url);
			$this->_running_vars['done_urls'][] = $url;

			// clean up
			unset($this->_running_vars['todo_urls'][$index]);

		}//end if

		// update progress
		if (empty($this->_running_vars['todo_urls'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
		} else {
			$total = count($this->_running_vars['todo_urls']) + count($this->_running_vars['done_urls']);
			$step_data['percent_done'] =(count($this->_running_vars['done_urls']) / $total) * 100;
			$step_data['complete']     = FALSE;
		}
		return TRUE;

	}//end processClearCache()


	/**
	* Repopulate Squid cache using CURL request
	*
	* @param string	$url	the URL of the page that we are processing
	* @param string	$host	the host of the proxy
	* @param string	$port	the port used by the proxy
	*
	* @return void
	* @access public
	*/
	public function requestPage($url, $host, $port)
	{
		// set the correct port if it isn't the default one
		if ($port != '') {
			// port variable contains '-p ' which needs to be stripped
			$port = ':'.trim(str_replace('-p','', $port));
		}

		// set correct cacheable headers
		$headers = Array(
					'Cache-Control: public',
					'Pragma: cache',
				   );

		$options = array(
				    'PROXY' => $host.$port,
				   );

		ob_start();
			$response = fetch_url($url, $options, $headers, FALSE);
		ob_end_clean();

	}//end requestPage()


}//end class


?>
