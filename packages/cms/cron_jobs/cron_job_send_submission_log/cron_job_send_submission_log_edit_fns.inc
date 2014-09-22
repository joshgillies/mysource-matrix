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
* $Id: cron_job_send_submission_log_edit_fns.inc,v 1.2 2012/08/30 00:58:07 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job_edit_fns.inc';

/**
* Cron_Job_Send_Submission_Log
*
* Purpose
*
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Send_Submission_Log_Edit_Fns extends Cron_Job_Edit_Fns
{


	/**
	* Constructor
	*
	* @see Asset_Edit_Fns::__construct()
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* paintScheduleInterface.
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintScheduleInterface(&$asset, &$o, $prefix)
	{
		// Null asset means, we are painting the interface to create a new asset
		if (!is_null($asset)) {
			echo $asset->readableWhen();
		}

		if (is_null($asset) || $asset->writeAccess('attributes')) {
			$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');

			// paint the start time using a one off job's edit fns
			$cron_job = new Cron_Job();
			$edit_fns = $cron_job->getEditFns();
			echo '<br />';
			echo '<br />'.translate('Send the submission logs at').' ';

			$edit_fns->_paintWhenBox($cron_job, $o, $prefix, is_null($asset) ? '' : $asset->attr('first_run'));

			// paint the repeat frequency using repeating time interval job's edit fns
			$cron_job_repeat = new Cron_Job();
			$cron_job_repeat->setAttrValue('type', 'repeating');
			echo '<br />'.translate('and repeat this process every');

			$this->_paintWhenBox($cron_job_repeat, $o, $prefix, is_null($asset) ? '' : $asset->attr('when'));

			require_once SQ_FUDGE_PATH.'/general/datetime.inc';
			echo '<br/>';
			echo sprintf(translate('The Scheduled Jobs Manager runs every <b>%1$s</b>. It will next run on the <b>%2$s</b>.'), $cron_mgr->readableRefreshTime(), readable_datetime($cron_mgr->timeOfNextRun()));

		}

		return TRUE;

	}// paintScheduleInterface()


	/**
	* processScheduleInterface.
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processScheduleInterface(&$asset, &$o, $prefix)
	{
		$cron_job = new Cron_Job();
		$edit_fns = $cron_job->getEditFns();

		$when_oo = $edit_fns->_processWhenBox($cron_job, $o, $prefix);

		if (!$when_oo) {
			trigger_localised_error('CMS0122', E_USER_WARNING);
			return FALSE;
		}

		if ($_REQUEST[$prefix.'_when_interval_mins'] == '--'
			&& $_REQUEST[$prefix.'_when_interval_hours'] == '--'
			&& $_REQUEST[$prefix.'_when_interval_days'] == '-') {

			trigger_localised_error('CMS0119', E_USER_WARNING);

			return FALSE;
		} else {
			// repeat frequency is set, but start time not specified, error
			if (!$when_oo) {
				trigger_localised_error('CRON0050', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->type(), 'name'));
				return FALSE;
			}

			// work out the period/duration start time (one off job code reuse)
			// mute warning as we are only using this bit to working out the start time
			if (!@$cron_job->setAttrValue('when', $when_oo)) {
				$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
				trigger_localised_error('CRON0048', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->type(), 'name'), easy_datetime($cron_mgr->timeOfNextRun()));
				return FALSE;
			}
			$when_oo_str = $cron_job->attr('when');
			// Save job's first run time set by the user
			// NOTE: First run time is not necessarily the time for job's very first run
			$asset->setAttrValue('first_run', $when_oo_str);

			// This is actual first run time
			$start_time = $cron_job->getNextRun($when_oo_str);

			// Ass

			// update the start time of this repeating time interval job
			$cron_job_repeat = new Cron_Job();
			$cron_job_repeat->setAttrValue('type', 'repeating');
			$when_ti = $this->_processWhenBox($cron_job_repeat, $o, $prefix);
			if ($when_ti) {
				$when_ti_arr = explode(',', $when_ti);
				$when_ti_arr[0] = 'TI='.$start_time;
				$when_ti_str = implode(',', $when_ti_arr);
				$asset->setAttrValue('when', $when_ti_str);

				return TRUE;
			}
		}

		return FALSE;

	}// processScheduleInterface()


}//end Cron_Job_Send_Submission_Log_Edit_Fns()

?>
