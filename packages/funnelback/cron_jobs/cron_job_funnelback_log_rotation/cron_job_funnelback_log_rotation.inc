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
* $Id: cron_job_funnelback_log_rotation.inc,v 1.7 2012/08/30 00:58:43 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job.inc';
require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

/**
* Cron_Job_Funnelback_Log_Rotation
*
* This cron job handles the log rotation for funnelback's query logs in the OEM version.
* The log rotation cannot be handled by log manager as the logs are location in the $SITE_SEARCH_HOME directory
* Also after log rotation, it handles the report generation
*
* @author  Benjamin Pearson <bpearson@squiz.com.au>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage funnelback
*/
class Cron_Job_Funnelback_Log_Rotation extends Cron_Job
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
	* Create this asset
	*
	* Return either the linkid or FALSE on failure
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return mixed int|boolean
	* @access public
	* @see Asset::create()
	*/
	function create(&$link)
	{
		require_once SQ_CORE_PACKAGE_PATH.'/system/system_asset_fns.inc';
		if (!system_asset_fns_create_pre_check($this)) {
			return FALSE;
		}
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($linkid = parent::create($link)) {
			if (!system_asset_fns_create_cleanup($this)) {
				$linkid = FALSE;
			}
		}

		if ($linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return $linkid;

	}//end create()


	/**
	* Return name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=FALSE)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name');

	}//end _getName()


	/**
	* returns FALSE always because we don't want to be able to clone this cron job
	*
	* @return boolean
	* @access public
	*/
	function canClone()
	{
		return FALSE;

	}//end canClone()


	/**
	* Morphs this asset to either one of its parent types or one of its children types.
	*
	* Only succeeds if $new_type_code assets are allowed to link to all children that this asset has
	*
	* @param string	$new_type_code	the new type_code to attempt to cast this asset to
	*
	* @return boolean
	* @access public
	* @see Asset::morph()
	*/
	function morph($new_type_code)
	{
		trigger_localised_error('CRON0024', sprintf(translate('Unable to morph %s'), $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name')), E_USER_WARNING)
		return FALSE;

	}//end morph()


	/**
	* Can this asset have its last significant link removed (putting it in the trash)?
	*
	* For the Manage Pending Accounts cron job, NO!  There must always be exactly one of these here.
	*
	* @return boolean
	* @access public
	*/
	function canDelete()
	{
		return FALSE;

	}//end canDelete()


	/**
	* Run this cron job
	*
	* Looks at all the currently-running workflows and processes any whose current step has
	* expired since the last run of the cron manager
	* Returns a bitmask of the SQ_CRON_JOB_* constants to tell the cron manager what to do
	*
	* @param string	&$msg	used to return any messages about completion etc from _exec
	*
	* @return int
	* @access private
	* @see Cron_Job::_exec(), Cron_Job::run()
	*/
	function _exec(&$msg)
	{
		$fm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('funnelback_manager');

		$collections = $fm->getCollections();
		$rotated = Array();

		// Rotate the logs for each collection
		foreach ($collections as $collection_id => $collection) {
			$log_path = $fm->getCollectionDataPath($collection_id, 'log');

			$log_files = glob($log_path.'/*.log');
			if (!empty($log_files)) {
				foreach ($log_files as $log_file) {
					$log_name = basename($log_file);

					// Click logs are not rotated yet, remove this line if needed
					if ($log_name == 'click.log') continue;

					// Archive
					$archive_name = $log_name.'.'.date('Ymd');
					$archive_path = $log_path.'/'.$archive_name;
					$alternative  = 1;
					while (file_exists($archive_path)) {
						$archive_path = $log_path.'/'.$archive_name.'.'.((string) $alternative);
						$alternative++;
					}//end while
					require_once SQ_FUDGE_PATH.'/general/file_system.inc';
					move_file($log_file, $archive_path);
					touch($log_file);

					// Keep a list for the report to work against
					$rotated[$collection_id] = $archive_path;

					// Clean up
					$period = $fm->attr('keep_log_period');
					if (!empty($period)) {
						$expiry_date = strtotime('-'.((int) $period).' months');
						$archive_logs = glob($log_path.'/'.$log_name.'.*');
						foreach ($archive_logs as $archive_log) {
							$found_archive_log_name = basename($archive_log);
							preg_match('/log\.([0-9]+)/i', $found_archive_log_name, $match);
							$log_date = array_get_index($match, 1, 0);
							if (!empty($log_date)) {
								$log_date_expiry = strtotime($log_date);
								if ($log_date_expiry < $expiry_date) {
									// Log expired, removing....
									unlink($archive_log);

									// Also remember to remove from rotated (no point trying to generate a report if a log is missing)
									if (isset($rotated[$collection_id]) && $rotated[$collection_id] == $found_archive_log_name) {
										unset($rotated[$collection_id]);
									}//end if
								}//end if
							}//end if
						}//end foreach
					}//end if
				}//end foreach
			}//end if
		}//end foreach

		if (!empty($rotated)) {
			// Logs were rotated, now to generate some reports
			$log_reports  = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('funnelback_search_log_report', FALSE, TRUE);

			// start up a hipo for each report
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();

			foreach ($log_reports as $reportid => $info) {
					$report = $GLOBALS['SQ_SYSTEM']->am->getAsset($reportid);
					$vars   = Array(
								'assetid'	=> $reportid,
								'report'		=> $report,
								'logs'		=> $rotated,
							  );
					$status = $hh->freestyleHIPO('hipo_job_generate_report', $vars);
			}//end foreach
		}//end if

		return SQ_CRON_JOB_COMPLETED;

	}//end _exec()


	/**
	* Runs this job
	*
	* This method is overridden because we want to make sure the cron manager never deletes the
	* expired_workflow cron job
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


}//end class

?>
