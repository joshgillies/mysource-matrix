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
* $Id: trigger_action_delete_future_status.inc,v 1.8 2012/08/30 01:09:13 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

/**
* Trigger_Action_Delete_Future_Status
*
* A trigger action that sets a future status for an asset
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Delete_Future_Status extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:
	* <PRE>
	* Array(
	* 		'status' => string, // the asset status
	* 		);
	* </PRE>
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return boolean
	* @access public
	*/
	public static function execute($settings, &$state)
	{
		// check settings, status
		if (empty($settings['status'])) {
			// if no settings, fail
			return FALSE;
		}

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}
		$assetid = $state['asset']->id;

		$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (is_null($cron_mgr)) return FALSE;

		// nothing to process
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_status');
		$future_status_jobs = Cron_Job_Future_Status::getActiveJobs($state['asset']);
		if (empty($future_status_jobs)) return TRUE;

		$status_list = Trigger_Action_Delete_Future_Status::_getStatusList();
		$jobids = Array();

		foreach ($future_status_jobs as $job) {
			// Does it meet the conditions?
			$status_desc = $job->statusName();
			if ($status_desc != $status_list[$settings['status']]) {
				continue;
			}

			// Okidoki, let's delete them
			if (!$cron_mgr->removeJob($job)) {
				trigger_localised_error('CORE0261', sprintf(translate('Cannot Create a Data Directory "%s" for a content xml file.'), $job->id), E_USER_WARNING)
			} else {
				$jobids[] = $job->id;
			}
		}

		return Array(
				'jobids'	=> $jobids,
			   );

	}//end execute()


	/**
	* Get the HTML editing interface for this condition. Returns the string instead of printing it
	*
	* @param array		$settings		settings that this condition save in processInterface()
	* @param string		$prefix			unique prefix
	* @param boolean	$write_access	write access flag
	*
	* @return boolean
	* @access public
	*/
	public static function getInterface($settings, $prefix, $write_access=FALSE)
	{
		if (empty($settings['status'])) {
			// set to default
			$selected_status = SQ_STATUS_UNDER_CONSTRUCTION;
		} else {
			$selected_status = $settings['status'];
		}

		$status_list = Trigger_Action_Delete_Future_Status::_getStatusList();

		if (!$write_access) {
			$form_element_extras = 'disabled="disabled"';
		} else {
			$form_element_extras = '';
		}

		ob_start();
			echo translate('Delete all future status Scheduled Jobs that would change the asset\'s status to');

			if ($write_access) {
				combo_box($prefix.'[status]', $status_list, FALSE, $selected_status, NULL, $form_element_extras);
			} else {
				echo get_asset_status_icon($selected_status).'<strong>'. get_status_description($selected_status).'</strong>';
			}
			$html = ob_get_contents();
		ob_end_clean();

		return $html;

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param array	&$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action as specified
	*								by the $prefix in getInterface
	*
	* @return boolean
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{
		// status
		$status = array_get_index($request_data, 'status', NULL);
		if (is_null($status)) {
			return 'Status has not been specified';
		}

		$status_list = Trigger_Action_Delete_Future_Status::_getStatusList();

		// check if the status is valid
		if (!isset($status_list[$status])) {
			return 'Specified status is invalid';
		}
		$settings['status'] = $status;

		return FALSE;

	}//end processInterface()


	/**
	* Returns an array containing all the valid statuses
	*
	* The returned array contains status(int) => description(string)
	*
	* @return array
	* @access private
	*/
	protected static function _getStatusList()
	{
		$status = Array();
		foreach (get_constant_values('SQ_STATUS_') as $status_value) {
			$status[$status_value] = get_status_description($status_value);
		}
		return $status;

	}//end _getStatusList()


}//end class

?>
