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
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job_edit_fns.inc';

/**
* Cron_Job_Purge_Trash_Edit_Fns
*
* Purpose
*
*
* @author  Ash Karelia <akarelia@squiz.com.au>
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Purge_Trash_Edit_Fns extends Cron_Job_Edit_Fns
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
	* @param Cron_Job_Purge_Trash	$asset					The owning asset
	* @param Backend_Outputter		$o						the Backend Outputter object
	* @param string					$prefix					prefix for form elements
	* @param string					$when					prepared when string
	* @param string					$cascade				cascade the status change
	* @param string					$form_extra_elements	extra form elements
	* @param array					$exclude_status			exclude these statuses from the list
	*
	* @return boolean
	* @access public
	*/
	public function paintInLineBackend(Cron_Job_Purge_Trash $asset, Backend_Outputter $o, $prefix, $when='')
	{
		// paint the start time using a one off job's edit fns
		$cron_job = new Cron_Job();
		$edit_fns = $cron_job->getEditFns();
		echo translate('Purge trash at &nbsp;');
		if (empty($when)) $when = $asset->attr('when');
		$when_parts = explode(',', $when);

		if (is_numeric(substr($when_parts[0], 3)) && substr($when_parts[0], 0, 3) === 'TI=') {
			$when = 'OO='.date('Y-m-d H:i', substr($when_parts[0], 3));
		}
		$edit_fns->_paintWhenBox($cron_job, $o, $prefix, $when);

		if (substr($when_parts[0], 0, 3) === 'TI=') { 
			// paint the repeat frequency using repeating time interval job's edit fns
			echo '<br />'.translate('and repeat this process every &nbsp;');
			$this->_paintWhenBox($asset, $o, $prefix);
		}

		return TRUE;

	}//end paintInLineBackend()


	/**
	* Process the tasks interface
	*
	* @param Cron_Job_Purge_Trash	$asset	The owning asset
	* @param Backend_Outputter		$o		the Backend Outputter object
	* @param string					$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function processInLineBackend(Cron_Job_Purge_Trash $asset, Backend_Outputter $o, $prefix)
	{
		$cron_job = new Cron_Job();
		$edit_fns = $cron_job->getEditFns();
		$trash_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');
		$trash_prefix = $trash_folder->getPrefix();
		$trash_scheduling = array_get_index($_POST, $trash_prefix.'_scheduling_status', FALSE);

		$when_oo = $edit_fns->_processWhenBox($cron_job, $o, $prefix);

		if (!$when_oo && !$trash_scheduling) {
			// if the scheduling is not on then don't worry about processing anything
			return FALSE;

		} else if (!$when_oo && $trash_scheduling) {
			// repeat frequency is set, but start time not specified, error
			trigger_localised_error('CRON0050', sprintf(translate('Start time for %s is not defined.'), $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->type(), 'name')), E_USER_WARNING);
			// nothing has been set, do not add a job
			return FALSE;
		} else {

			// update the start time of this repeating time interval job
			$when_ti = $this->_processWhenBox($asset, $o, $prefix);
			if ($when_oo && strpos($when_oo, '!') === FALSE) {
				$asset->setAttrValue('type', 'repeating');
				$start_time = $cron_job->getNextRun($when_oo);
				$when_ti_arr = explode(',', $when_ti);
				$when_ti_arr[0] = 'TI='.$start_time;
				if (!isset($when_ti_arr[1])) $when_ti_arr[1] = '-';
				if (!isset($when_ti_arr[2])) $when_ti_arr[2] = '3600';

				$when_ti_str = implode(',', $when_ti_arr);
				$asset->setAttrValue('when', $when_ti_str);
			} else {
				$asset->setAttrValue('type', 'one_off');
				$asset->setAttrValue('when', $when_oo);
			}
			$asset->setAttrValue('long', TRUE);

			return TRUE;
		}

	}//end processInLineBackend()


}//end class

?>
