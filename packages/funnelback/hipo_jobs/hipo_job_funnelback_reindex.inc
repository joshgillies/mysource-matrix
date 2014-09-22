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
* $Id: hipo_job_funnelback_reindex.inc,v 1.6 2012/08/30 00:58:43 ewang Exp $
*
*/


require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to reindex sites
*
* Initial Running Vars :
*    root_assetid	-	the ID of the root asset of the tree that will be reindexed
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix
* @subpackage funnelback
*/
class HIPO_Job_Funnelback_Reindex extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name		a unique codename the HIPO
	*/
	function HIPO_Job_Funnelback_Reindex($code_name='')
	{
		$this->uses_trans = FALSE;
		$this->HIPO_Job($code_name);
		$GLOBALS['SQ_SYSTEM']->lm->includePackageStrings('funnelback');
		$this->_hipo_vars['job_dir'] = SQ_PACKAGES_PATH.'/funnelback/hipo_jobs';

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
		$o->openField(translate('Funnelback Reindex'));


		echo '<span class="sq-inline-fields-wrapper">';
		if ($write_access) {
			text_box($class.'[SQ_HIPO_REINDEX_FUNNELBACK]', SQ_HIPO_REINDEX_FUNNELBACK, 1);
		} else {
			echo SQ_HIPO_REINDEX_FUNNELBACK;
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
				'SQ_HIPO_REINDEX_FUNNELBACK'	=> Array('editable'	=> 1, 'default'	=> 5),
			   );

	}//end getConfigVars()


	/**
	* Returns the current Threshold.
	*
	* Returns a value that represents the total percentage that this hipo would run in
	* freestyle mode in its current state.
	*
	* @return float
	* @access public
	*/
	function getThresholdPercentageRequired()
	{
		return 0;

	}//end getThresholdPercentageRequired()


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
		return parent::getCodeName().'-funnelback-reindex';

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('Funnelback Reindex');


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
					'name'			=> translate('Reindexing'),

					'function_call'	=> Array(
										'process_function'	=> 'processReindex',
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
	function freestyle()
	{
		// Put the HIPO in some context ;)
		if (array_get_index('contextid', $this->_running_vars)) {
			$this->_running_vars['contextid'] = $GLOBALS['SQ_SYSTEM']->getContextId();
		}

		while (!empty($this->_running_vars['todo_collections'])) {
			if (!$this->processReindex($this->_steps[0], get_class_lower($this))) {
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
		// Defaults
		$this->_running_vars['todo_collections'] = Array();
		$this->_running_vars['done_collections'] = Array();
		$fm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('funnelback_manager');
		if (is_null($fm)) {
			trigger_localised_error('FNB0020', E_USER_WARNING);
			return FALSE;
		}//end if
		$collections = $fm->getCollections();

		// If we have not been told which context to re-index, assume we're in
		// it already (but this is not recommended if not freestyling, since it
		// could be stuck somewhere where the context you want is not set)
		if (array_get_index('contextid', $this->_running_vars)) {
			$this->_running_vars['contextid'] = $GLOBALS['SQ_SYSTEM']->getContextId();
		}

		// To reindex, get a collection or if none specified do the whole system
		if (!isset($this->_running_vars['collections']) || (is_null($this->_running_vars['collections']) || empty($this->_running_vars['collections']))) {
			// No URL specified, do the lot, fool!
			foreach ($collections as $collection_id => $collection_info) {
				// Meta collections not reindex here, so skip
				$collection_type = array_get_index($collection_info, 'type', 'web');
				if ($collection_type == 'meta') continue;

				$collection_valid = array_get_index($collection_info, 'valid', FALSE);
				if ($collection_valid === TRUE) {
					$this->_running_vars['todo_collections'][] = Array($collection_id => $collection_info);
				}//end if
			}//end foreach
		} else {
			// Specified URL(s), do these ones, fool!
			if (!is_array($this->_running_vars['collections'])) {
				// If one specified, make it an array
				$this->_running_vars['collections'] = Array($this->_running_vars['collections']);
			}//end if

			foreach ($this->_running_vars['collections'] as $todo_collection) {
				// we have the collection, go thru each collection and find the correct entry (make sure the collection is valid)
				foreach ($collections as $collection_id => $collection_info) {
					// Meta collections not reindex here, so skip
					$collection_type = array_get_index($collection_info, 'type', 'web');
					if ($collection_type == 'meta') continue;

					$collection_valid = array_get_index($collection_info, 'valid', FALSE);
					if ($collection_valid === TRUE && $collection_id == $todo_collection) {
						$this->_running_vars['todo_collections'][] = Array($collection_id => $collection_info);
					}//end if
				}//end foreach
			}//end foreach
		}//end if

		return parent::prepare();

	}//end prepare()


	/**
	* Reindex using Funnelback
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processReindex(&$step_data, $prefix)
	{
		$GLOBALS['SQ_SYSTEM']->changeContext(array_get_index($this->_running_vars, 'contextid', 0));

		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$fm = $am->getSystemAsset('funnelback_manager');
		if (is_null($fm)) {
			return FALSE;
		}//end if

		// get next url to reindex, if none to do just flow through and finish the job neat and tidy ;)
		if (!empty($this->_running_vars['todo_collections'])) {
			reset($this->_running_vars['todo_collections']);
			$next = key($this->_running_vars['todo_collections']);
			$collection_id   = key($this->_running_vars['todo_collections'][$next]);
			$collection_info = $this->_running_vars['todo_collections'][$next][$collection_id];
			$url = array_get_index($collection_info, 'url', Array());
			$assetid = array_get_index($collection_info, 'asset', 0);
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, '', TRUE);
			unset($this->_running_vars['todo_collections'][$next]);

			if (!empty($url) && !is_null($asset)) {
				// Let people know what you are thinking ;-)
				$step_data['message'] = sprintf(translate('Reindexing URL: "%s"'), htmlspecialchars($asset->attr('name')));


				// Build the parameters
				$output = '';
				$xml = $fm->generateXMLCache($url, $assetid, FALSE);
				$log_file = SQ_LOG_PATH.'/funnelback.log';
				$bin_path = $fm->getFunnelbackPath();
				$coll_path = $fm->getCollectionDataPath($collection_id);
				$coll_log_path = $fm->getCollectionDataPath($collection_id, 'log');
				$data_path = $fm->getCollectionDataPath($collection_id, 'idx_new');
				$live_path = $fm->getCollectionDataPath($collection_id, 'idx');
				if (empty($data_path)) return FALSE;
				$options = $fm->getIndexingOptions();

				// Check for permissions and show a fail if cannot write
				if (!is_writable($data_path)) {
					trigger_localised_error('FNB0030', E_USER_WARNING);
					return FALSE;
				}//end if

				// Create the XML on the filesystem
				file_put_contents($data_path.'/sitemap.xml', $xml);
				// Calculate max line length of metadata summary
				preg_match('/[0-9]+/', exec('wc -L '.$data_path.'/sitemap.xml'), $matches);
				$maxLineLength = isset($matches[0]) ? $matches[0] : NULL;

				// Apply login credentials
				$user_id = $fm->attr('user');
				if (!empty($user_id)) {
					$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($user_id, '', TRUE);
					if (!is_null($user)) {
						$username = $user->attr('username');
						$password = $fm->attr('password');
						file_put_contents($data_path.'/creds', $username.' '.$password);
						$options[] = '-CF='.$data_path.'/creds';
						$options[] = '-loginPrefix='.SQ_CONF_LOGIN_SUFFIX;
					}//end if
				}//end if

				// Add in the click log if exists
				if (file_exists($coll_log_path.'/click.log')) {
					$options[] = '-F'.$coll_log_path.'/click.log';
				}//end if

				// Make sure we ONLY index with at least one key (public assets will have a 'public' key)
				$options[] = '-security_level=1';

				// Build the command line
				$command = $bin_path.'/bin/padre-iw';
				$command .= ' -secmap '.escapeshellarg($data_path.'/sitemap.xml');
				$command .= ' '.escapeshellarg($collection_id);		// Index Stem
				$command .= ' '.implode(' ', $options);				// Command line Options
				$command .= isset($maxLineLength) ? ' -mdsfml'.$maxLineLength : ''; // Max line length of metadata summary file

				// Start the reindex process
				$status = $fm->runCommand($command, $output, NULL, $log_file, $data_path, Array('SITE_SEARCH_ROOT'=>$coll_path));

				// Regardless of status, remove the credential file
				if (file_exists($data_path.'/creds')) {
					unlink($data_path.'/creds');
				}//end if

				if ($status === 0) {
					// Successfully completed
					$make_live_cmd = 'rm -rf idx_old; mv idx idx_old; mv idx_new idx';
					$final_status = $fm->runCommand($make_live_cmd, $output, NULL, $log_file, $coll_path);
					if ($status !== 0) {
						// Failed making live
						trigger_localised_error('FNB0032', E_USER_WARNING);
						return FALSE;
					}//end if
				} else {
					// Fail....
					trigger_localised_error('FNB0031', E_USER_WARNING);
					return FALSE;
				}//end if

				// Forget some information
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset, TRUE);
			}//end if

			// add this assetid to the done array
			$this->_running_vars['done_collections'][] = $collection_id;
		}//end if

		$GLOBALS['SQ_SYSTEM']->restoreContext();

		if (empty($this->_running_vars['todo_collections'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
		} else {
			$total = count($this->_running_vars['todo_collections']) + count($this->_running_vars['done_collections']);
			$step_data['percent_done'] =(count($this->_running_vars['done_collections']) / $total) * 100;
			$step_data['complete']     = FALSE;
		}

		return TRUE;

	}//end processReindex()


}//end class

?>
