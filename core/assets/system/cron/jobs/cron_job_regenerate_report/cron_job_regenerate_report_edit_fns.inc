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
* $Id: cron_job_regenerate_report_edit_fns.inc,v 1.9 2012/08/30 01:09:11 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job_edit_fns.inc';

/**
* Cron_Job_Regenerate_Report_Edit_Fns
*
* Purpose
*
* Provide an editing interface to paint the repeating time interval cron job
* For scheduling a cron job that runs once every x period of time
* If frequency is not set, a one-off job will be scheduled
* see cron_job.inc for more information about the encoded 'when' str
* (TI=start_time,time_interval,time_unit)
*
*
* @author  Rayn Ong <rong@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Regenerate_Report_Edit_Fns extends Cron_Job_Edit_Fns
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
	* Prints the tasks interface in a one line format
	*
	* @param Cron_Job_Regenerate_Report	$asset	the owning asset
	* @param Backend_Outputter			$o		the Backend Outputter object
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintInLineBackend(Cron_Job_Regenerate_Report $asset, Backend_Outputter $o, $prefix)
	{
		if ($asset->writeAccess('')) {
			// paint the start time using a one off job's edit fns
			$cron_job = new Cron_Job();
			$edit_fns = $cron_job->getEditFns();
			echo translate('Regenerate this report at');

			$edit_fns->_paintWhenBox($cron_job, $o, $prefix);

			// paint the repeat frequency using repeating time interval job's edit fns
			echo '<br />'.translate('and repeat this process every');

			$this->_paintWhenBox($asset, $o, $prefix);
		}

		return TRUE;

	}//end paintInLineBackend()


	/**
	* Process the tasks interface
	*
	* @param Cron_Job_Regenerate_Report	$asset	the owning asset
	* @param Backend_Outputter			$o		the Backend Outputter object
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function processInLineBackend(Cron_Job_Regenerate_Report $asset, Backend_Outputter $o, $prefix)
	{
		$cron_job = new Cron_Job();
		$edit_fns = $cron_job->getEditFns();

		$when_oo = $edit_fns->_processWhenBox($cron_job, $o, $prefix);
		if (!$when_oo) {
			// nothing has been set, do not add a job
			return FALSE;
		}

		if (   $_REQUEST[$prefix.'_when_interval_mins'] == '--'
			&& $_REQUEST[$prefix.'_when_interval_hours'] == '--'
			&& $_REQUEST[$prefix.'_when_interval_days'] == '-') {

			// if repeat frequency is not set, this is a one off job
			$asset->setAttrValue('type', 'one_off');
			if (!$asset->setAttrValue('when', $when_oo)) {
				return FALSE;
			}
			return TRUE;
		} else {

			// repeat frequency is set, but start time not specified, error
			if (!$when_oo) {
				trigger_localised_error('CRON0050', sprintf(translate('Start time for %s is not defined.'), $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->type(), 'name')), E_USER_WARNING)
				return FALSE;
			}

			// work out the period/duration start time (one off job code reuse)
			// mute warning as we are only using this bit to working out the start time
			if (!@$cron_job->setAttrValue('when', $when_oo)) {
				$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
				trigger_localised_error('CRON0048', sprintf(translate('Unable to schedule a repeating %1$s to be run before the next job run is scheduled (which is at %2$s)'), $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->type(), 'name'), easy_datetime($cron_mgr->timeOfNextRun())), E_USER_WARNING)
				return FALSE;
			}
			$when_oo_str = $cron_job->attr('when');
			$start_time = $cron_job->getNextRun($when_oo_str);

			// update the start time of this repeating time interval job
			$when_ti = $this->_processWhenBox($asset, $o, $prefix);
			$when_ti_arr = explode(',', $when_ti);
			$when_ti_arr[0] = 'TI='.$start_time;
			$when_ti_str = implode(',', $when_ti_arr);
			$asset->setAttrValue('when', $when_ti_str);
			$asset->setAttrValue('long', TRUE);

			return TRUE;
		}
		return FALSE;

	}//end processInLineBackend()


}//end class

?>
