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
* $Id: cron_job_send_submission_log.inc,v 1.3.4.2 2013/09/11 03:04:46 cupreti Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job.inc';

/**
* Cron_Job_Send_Submission_Log
*
* Purpose
*
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.3.4.2 $
* @package MySource_Matrix_Packages
* @subpackage __cms__
*/
class Cron_Job_Send_Submission_Log extends Cron_Job
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor
	
	
	/**
	* Perform any additional processing required during the creation of this asset
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _createAdditional(Array &$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		if (!empty($this->_tmp['asset_in_link'])) {
			if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'links')) {
				trigger_localised_error('CRON0018', E_USER_WARNING);
				return FALSE;
			}

			$link = $this->_tmp['asset_in_link'];
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($asset) || !$this->setAssetToUpdate($asset)) {
				trigger_localised_error('CRON0017', E_USER_WARNING);
				return FALSE;
			}

			unset($this->_tmp['asset_in_link']);
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'links');
		}
		return TRUE;

	}//end _createAdditional()


	/**
	* Return name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE)
	{
		$asset = $this->getAssetToUpdate();
		if (is_null($asset)) {
			return 'Asset-less '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name');
		} else if ($short_name) {
			return 'Send Submission Log Job for "'.$asset->short_name.'" [#'.$asset->id.']';
		} else {
			return 'Send Submission Log Job for "'.$asset->name.'" [#'.$asset->id.']';
		}

	}//end _getName()
	

	/**
	* Get cron job display name
	*
	* @return string
	* @access public
	*/
	public function displayName()
	{
		return $this->attr('display_name');

	}//end displayName()
	


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
	{
		return Array(SQ_LINK_NOTICE => Array('form_email' => Array('card' => 1)));

	}//end _getAllowedLinks()


	/**
	* Can this asset have its last significant link removed (putting it in the trash)?
	*
	* For Cron Jobs this can only be done by the running user or a sys admin
	* For Send Form Logs job, admins of the asset to update can also remove jobs
	*
	* @return boolean
	* @access public
	*/
	public function canDelete()
	{
		if (parent::canDelete()) return TRUE;

		// read-only?
		if ($this->attr('read_only')) return FALSE;

		$asset = $this->getAssetToUpdate();
		if (!is_null($asset)) return $asset->adminAccess('');
		return FALSE;

	}//end canDelete()


	/**
	* Sets the asset that this cron job will be altering the status on
	*
	* @param Form Email	$form_email	a reference to the Form Email asset
	*
	* @return boolean
	* @access public
	*/
	public function setAssetToUpdate(Form_Email $form_email)
	{
		if ($this->id) {
			$existing_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE, '', TRUE, 'major', $value='updating');
			$link_exists = FALSE;
			foreach ($existing_links as $old_link) {
				if ($old_link['minorid'] == $form_email->id) {
					$link_exists = TRUE;
				} else {
					$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($old_link['linkid']);
				}
			}
			if ($link_exists) {
				return TRUE;
			} else {
				return (bool)  $this->createLink($form_email, SQ_LINK_NOTICE, 'updating');
			}
		} else {
			if (!isset($this->_tmp['asset_in_link'])) {
				$this->_tmp['asset_in_link'] = Array();
			}
			$this->_tmp['asset_in_link'] = Array('minorid' => $form_email->id, 'minor_type_code' => $form_email->type());
			return TRUE;
		}

	}//end setAssetToUpdate()


	/**
	* Get the asset that this cron job will be altering the status on
	*
	* @return mixed object|NULL
	* @access public
	*/
	public function getAssetToUpdate()
	{
		// because we need to return by ref
		$null = NULL;

		if ($this->id) {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, 'form_email', TRUE, 'updating');
		} else {
			$link = (isset($this->_tmp['asset_in_link'])) ? $this->_tmp['asset_in_link'] : Array();
		}
		if (empty($link)) return $null;

		return $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], 'form_email');

	}//end getAssetToUpdate()


	/**
	* Actually executes the functionality for this job, called by run()
	*
	* Returns a bitmask of the SQ_CRON_JOB_* constants to tell the cron manager what to do
	*
	* @param string	&$msg	used to return any messages about completion etc from _exec
	*
	* @return int
	* @access private
	* @see Cron_Job::_exec(), Cron_Job::run()
	*/
	protected function _exec(&$msg)
	{
		$form = $this->getAssetToUpdate();		
		
		if (!is_null($form) && $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($form->id, 'form_email')) {			
			
			$log_file_info = $this->_getFormSubmissionLogFile($form);				
			
			if (is_array($log_file_info)) {
		
				$email_format = $this->getAttribute('email_format');
				$email_format_val = @unserialize($email_format->value);
				
				$text_email = array_get_index($email_format_val, 'text_format', '');
				$html_email = array_get_index($email_format_val, 'html_format', '');
				$email_subject = array_get_index($email_format_val, 'subject', '');
				$email_from = array_get_index($email_format_val, 'from', '');
				$email_reply_to = array_get_index($email_format_val, 'from', '');				
				
				$keywords = extract_keywords($text_email.$html_email.$email_subject.$email_from.$email_reply_to);
				$keyword_replacements = Array();
				foreach($keywords as $full_keyword) {
					$modifiers = NULL;
					$part_keyword = parse_keyword($full_keyword, $modifiers);
					if ($part_keyword == 'submission_log_from_date') {
						$keyword_value = date('Y-m-d H:i', $log_file_info['from']);
					} else if ($part_keyword == 'submission_log_to_date') {
						$keyword_value = date('Y-m-d H:i', $log_file_info['till']);
					} else {
						$keyword_value = $form->getKeywordReplacement($part_keyword);
					}
					if ($part_keyword != $full_keyword) {
						apply_keyword_modifiers($keyword_value, $modifiers, Array('assetid' => $form->id));
					}
					$keyword_replacements[$full_keyword] = $keyword_value;
				}
				
				$email_format->sendMail($keyword_replacements, Array($log_file_info['file']));
				
				// Mark the run time
				$this->setAttrValue('previous_run', $log_file_info['till']);
				$this->saveAttributes();

				// Clean up the temporary log file
				if (file_exists($log_file_info['file'])) {
					unlink($log_file_info['file']);
				}
				
				$msg = 'Submission log sent for form #'.$form->id.' (job #'.$this->id.')';
				
				return SQ_CRON_JOB_COMPLETED;
			}
		} else {
			// asset not found
			trigger_localised_error('CMS0120', E_USER_WARNING);
		}
		
		return SQ_CRON_JOB_ERROR;

	}//end _exec()


	/**
	* Returns an array of "Send Submission Logs" jobs that are to be run on the passed asset
	* in chronological order
	*
	* @param Asset	$form_email	the asset to get the "Send Submission Logs" jobs for
	*
	* @return array
	* @access public
	*/
	public static function &getActiveJobs(Form_Email $form_email)
	{
		$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($form_email->id, SQ_LINK_NOTICE, strtolower(__CLASS__), TRUE, 'minor', 'updating');
		$assetids = Array();
		$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		foreach ($links as $link) {
			$cm_links = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($cm->id, $link['majorid']);
			if (!empty($cm_links)) {
				// This asset is still linked to the cron manager, so its still active.
				$assetids[] = $link['majorid'];
			}
		}
		$dates = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('when', 'cron_job_send_submission_log', $assetids);
		asort($dates, SORT_STRING);
		$res = Array();
		foreach ($dates as $assetid => $date) {
			$res[] = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, strtolower(__CLASS__));
		}
		return $res;

	}//end getActiveJobs()

	
		/**
	* Runs this job
	*
	* This method is overridden because we want to make sure the cron manager never deletes the
	* send_submission_log cron job
	* Returns a bitmask of the SQ_CRON_JOB_* constants to tell the cron manager what to do
	*
	* @return int
	* @access public
	*/
	function run()
	{
		$res = parent::run();
		if ($res & SQ_CRON_JOB_REMOVE) {
			$res = $res - SQ_CRON_JOB_REMOVE;
		}
		return $res;

	}//end run()

	
	/**
	* Get the sumbission log CSV file for the given form
	*
	* @param object $form
	*
	* @return array|boolean
	* @access private
	*/
	function _getFormSubmissionLogFile($form)
	{	
		$edit_fns = $form->getEditFns();
		
		// Pick the submissions starting from this date
		$start_date = str_replace('--:', '00:', str_replace(':--', ':00', $this->attr('submissions_from_date')));
		// If the specified start date is relative
		if (preg_match('|^([\-\+])(\d+)([ymwdhi])$|', $start_date, $match)) {
			switch ($match[3]) {
				case 'y':
					$format_type = 'years';
				break;
				case 'm':
					$format_type = 'months';
				break;
				case 'w':
					$format_type = 'weeks';
				break;
				case 'd':
					$format_type = 'days';
				break;
				case 'h':
					$format_type = 'hours';
				break;
				case 'i':
					$format_type = 'minutes';
				break;
				default:
					$format_type = '';
				break;
			}
			$pick_from_date = $format_type ? strtotime($match[1].$match[2].' '.$format_type) : '';
		} else {
			// If any date component is not setup, dont use that date
			$pick_from_date = strpos($start_date, '--') !== FALSE ? '' : strtotime($start_date);
		}

		// Last time the job actually ran
		$previous_run = $this->attr('previous_run');
		
		if ($this->attr('submissions_range') == 'last_run') {			
			if (!empty($previous_run)) {
				// If the job ran previosuly, pick it from there
				$from_ts = $previous_run;
			} else if ($pick_from_date) {
				// The job is running for first time, pick since the "Start" date
				$from_ts = $pick_from_date;
			} else {
				// No "Start" date configured, pick submissions since the creation of the form
				$from_ts = $form->created;
			}
		} else {
			$from_ts = !empty($pick_from_date) ? $pick_from_date : $form->created;
		}
		
		if (!$from_ts) {
			trigger_localised_error('CMS0121', E_USER_WARNING);
			return FALSE;
		}		
		
		$till_ts = time();
		
		switch($this->attr('logs_format')) {
			case 'csv_nhr':
			case 'csv':	
				$csv = $edit_fns->createCSVSubmissionLogs($form, $from_ts, $till_ts);				
				$delimiter = $this->attr('csv_delimiter');
				$csv->setDeliminator(!empty($delimiter) ? $delimiter : ',');
				
				if ($this->attr('logs_format') == 'csv_nhr') {
					$empty_header = FALSE;
					$csv->setFieldHeaders($empty_header);
				}				
				ob_start();						
					@$csv->export(TRUE);
					$output = ob_get_contents();
				ob_end_clean();
			break;
			
			case 'xml':
				$o = NULL;
				ob_start();		
					@$this->_processExportXML($form, $from_ts, $till_ts);
					$output = ob_get_contents();
				ob_end_clean();
			break;			
		
		}//end switch
		
		$job_name = trim(preg_replace('|[^a-z0-9\-_]+|', '_', strtolower($this->displayName())), '_');		
		$file_name = SQ_TEMP_PATH.'/submissions_log_'.$job_name.'('.date('Y-m-d H:i', $from_ts).' to '.date('Y-m-d H:i', $till_ts).')';
		file_put_contents($file_name, $output);

		return Array(
				'file' => $file_name,
				'from' => $from_ts,
				'till' => $till_ts,
			);
		
	}//end getFormSubmissionLogFile()
	
	
	/**
	* Exports chosen submissions to an XML format
	*
	* @param object	$asset		Form asset of whose submissions we are exporting
	* @param string $from_ts 	Timestamp since when to pick the logs
	* @param string $to_ts 		Timestamp till when to pick the logs
	*
	* @return void
	* @access private
	*/
	function _processExportXML($asset, $from_ts, $to_ts)
	{
		$edit_fns = $asset->getEditFns();
		
		$from_time = ts_iso8601($from_ts);
		$to_time = ts_iso8601($to_ts);
		$sub_folder = $asset->getSubmissionsFolder();
		$assetids = Array($asset->id, $sub_folder->id);

		try {
			$bind_vars = Array(
							'assetids'	=> $assetids,
							'from_time'	=> $from_time,
							'to_time'	=> $to_time,
							'name'		=> 'submitted',
							'context'	=> $GLOBALS['SQ_SYSTEM']->getContextId(),
						 );
			$result = $edit_fns->_getSubmissionLog($bind_vars);
		} catch (Exception $e) {
			throw new Exception("Unable to get submission log due to database error: ".$e->getMessage());			
		}

		echo "<submissions>\n";
		foreach ($result as $asset_info) {
			$assetid = $asset_info['assetid'];
			$type_code = $asset_info['type_code'];

			$sub_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type_code);
			if (!$sub_asset->attr('complete')) continue;

			if (trim($sub_asset->attr('xml')) == '') continue;
			echo "\t".$sub_asset->attr('xml')."\n";
		}
		echo '</submissions>';		

	}//end processExportXML()
	
	

}//end class

?>
