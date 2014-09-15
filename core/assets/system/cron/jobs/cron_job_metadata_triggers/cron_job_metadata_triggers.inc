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
* $Id: cron_job_metadata_triggers.inc,v 1.10 2012/08/30 01:09:11 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job.inc';

/**
* Cron_Job_Metadata_Triggers
*
* This cron jobs fire Metadata Time Events for all the assets in the system
* which have an expired metadata date field.
*
* Note that the default values are currently ignored by this cron job.
* (they wont fire any event even they fall within the expiry time interval)
* We can add this as a new feature later if required.
*
* @see _exec() for the algorithm
*
* @author  Geoffroy Noel <gnoel@squiz.co.uk>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Metadata_Triggers extends Cron_Job
{

	/**
	* Constructor
	*
	* @param string	$assetid	the asset id to be loaded
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
	* Can this asset have its last significant link removed (putting it in the trash)?
	*
	* For the expired workflow cron job, NO!  There must always be exactly one of these here.
	*
	* @return boolean
	* @access public
	*/
	public function canDelete()
	{
		return FALSE;

	}//end canDelete()


	/**
	* Returns name of the asset
	*
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @see Asset::_getName()
	* @access private
	*/
	function _getName($short_name=FALSE)
	{
		return translate('cron_job_metadata_triggers');

	}//end _getName()


	/**
	* Runs this job
	*
	* This method is overridden because we want to make sure
	* the cron manager never deletes this cron job
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
	* Actually executes the functionality for this job, called by run()
	*
	* Broadcast a trigger event for all assets which date is expired
	*
	* @param string	$msg	accepted by reference, used to return any messages
	* about completion etc from _exec
	*
	* @return int	bitmask of the SQ_CRON_JOB_* constants to tell the cron manager what to do
	* @see Cron_Job::_exec(), Cron_Job::run()
	* @access private
	*/
	function _exec(&$msg)
	{
		$ret = SQ_CRON_JOB_COMPLETED;

		$db = $GLOBALS['SQ_SYSTEM']->db;

		// work out min date - if 'use_cron_last_run' is true, use cron manager's last run as min date here
		// otherwise use 'ignore_date' attribute value
		if ($this->Attr('use_cron_last_run')) {
			$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
			$ignore_date = date('Y-m-d H:i:s', $cron_mgr->getLastRun());
		} else {
			$ignore_date = $this->Attr('ignore_date');
		}

		$max_date = date('Y-m-d H:i:s');
		if (empty($ignore_date)) return $ret;

		// 1. get all the triggers with a Metadata Time event
		// and get all the metadsta fields for each one
		//
		// xxx: note that this code is highly dependent on the internal structure of the triggers
		// This code could be made more independent once the the trigger manager provides the desired functions
		// to browse its triggers
		$mdfieldids = Array();
		$tm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trigger_manager');
		$triggers = $tm->getTriggerList();
		foreach ($triggers as $trigger_info) {
			$trigger = $tm->getTrigger($trigger_info['id']);
			// we would like to know the field id of metadata specified in active triggers that use metadata time event
			if ($trigger['active'] && !empty($trigger['data']['events']['trigger_event_metadata_time'])) {
				foreach ($trigger['data']['conditions'] as $trigger_condition) {
					if ($trigger_condition['type'] == 'trigger_condition_metadata_time') {
						if (isset($trigger_condition['data']['trigger_metadata_date_fieldid'])) {
							$mdfieldids[$trigger_condition['data']['trigger_metadata_date_fieldid']] = 1;
						}
					}
				}
			}
		}

		if (!$this->Attr('use_cron_last_run')) {
			$period_options = Array(
								'i' => translate('minutes'),
								'h' => translate('hours'),
								'd' => translate('days'),
								'w' => translate('weeks'),
								'm' => translate('months'),
								'y' => translate('years'),
							);

			// check to see if the $ignore_time is relative or fixed. If relative lets make a proper time out of it
			$found = preg_match('/^([\-\+])([0-9]+)(['.implode('', array_keys($period_options)).'])$/', $ignore_date, $matches);

			if($found === 1) {
				$ignore_date = date('Y-m-d H:i:s', strtotime($max_date.' '.$matches[1].$matches[2].$period_options[$matches[3]]));
			}
		}

		// if we are having the cut off date in past then we should swap min_date <=> max_date or else the
		// sql will be wrong and we would not get desired results
		if(strtotime($ignore_date) < strtotime($max_date) || ($found === 1 && $matchs[1] == '-')) {
			$temp = $ignore_date;
			$ignore_date = $max_date;
			$max_date = $temp;
		}

		// 2. get all the asset which have an expired date for those mdata fields
		if (!empty($mdfieldids)) {
			$mdfieldids_str = '';
			foreach (array_keys($mdfieldids) as $mdfieldid) {
				if (!empty($mdfieldids_str)) $mdfieldids_str .= ', ';
				$mdfieldids_str .= MatrixDAL::quote($mdfieldid);
			}

			// for performance reasons we directly query the database instead of using AM/MM functions
			// Note that this query doesn't catch default metadata values
			$result = Array();
			$db = MatrixDAL::getDb();
			$sql = 'SELECT
						mv.assetid, mv.fieldid
					FROM
						sq_ast_mdata_val mv
					WHERE
						mv.fieldid IN ('.$mdfieldids_str.')';
			if (MatrixDAL::getDbType() === 'oci') {
				$sql .= ' AND CAST(mv.value AS varchar(255)) < :min_date AND CAST(mv.value AS varchar(255)) > :max_date';
			} else {
				$sql .= ' AND mv.value < :min_date AND mv.value > :max_date';
			}

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'min_date', $ignore_date, PDO::PARAM_STR);
				MatrixDAL::bindValueToPdo($query, 'max_date', $max_date, PDO::PARAM_STR);
				$result = MatrixDAL::executePdoAll($query);
			} catch (Exception $e) {
				throw new Exception('Cron Job Metadata Trigger failed to execute the DB Query: '.$e->getMessage());
				$ret = SQ_CRON_JOB_ERROR;
			}

			// 3. broadcast trigger_event_metadata_time event for all the expired assets
			foreach ($result as $asset_info) {
				// fire the trigger for that mdata field and asset
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset_info['assetid']);
				if (is_null($asset)) continue;
				$parameters = Array('trigger_metadata_date_fieldid' => $asset_info['fieldid']);
				$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_metadata_time', $asset, $parameters);
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			}
		}

		return $ret;

	}//end _exec()


}//end class

?>