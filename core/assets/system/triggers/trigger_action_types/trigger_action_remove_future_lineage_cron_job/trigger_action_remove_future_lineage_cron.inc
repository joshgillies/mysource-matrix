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
* $Id: trigger_action_remove_future_lineage_cron.inc,v 1.2 2012/08/30 01:09:13 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
define('SQ_REMOVE_FUTURE_LINEAGE_CRON_ADD', 1);
define('SQ_REMOVE_FUTURE_LINEAGE_CRON_DELETE', 2);
define('SQ_REMOVE_FUTURE_LINEAGE_CRON_BOTH', 3);

/**
* Trigger_Action_Remove_Future_Lineage_Cron
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Remove_Future_Lineage_Cron extends Trigger_Action
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
	* @return boolean|array
	* @access public
	*/
	public static function execute($settings, &$state)
	{	
		// Check settings		
		if (empty($settings['job_type'])) {
			// If no settings, fail
			return FALSE;
		}

		if (empty($state['asset'])) {
			// Grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}		

		$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (is_null($cron_mgr)) return FALSE;
		
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_lineage');
		$future_lineage_jobs = Cron_Job_Future_Lineage::getActiveJobs($state['asset']);		
		if (empty($future_lineage_jobs)) return TRUE;		
		
		$jobids = Array();		
		
		foreach ($future_lineage_jobs as $job) {
			
			$job_type = $job->attr('link_info') ? SQ_REMOVE_FUTURE_LINEAGE_CRON_ADD : SQ_REMOVE_FUTURE_LINEAGE_CRON_DELETE;
			
			if ($settings['job_type'] == SQ_REMOVE_FUTURE_LINEAGE_CRON_BOTH || $settings['job_type'] == $job_type) {			
				if (!$cron_mgr->removeJob($job)) {
					trigger_localised_error('CORE0308', E_USER_WARNING, $job->id);
				} else {
					$jobids[] = $job->id;
				}
			}//end if
		}//end foreach		

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
		if (empty($settings['job_type'])) {
			// set to default
			$selected_type = SQ_REMOVE_FUTURE_LINEAGE_CRON_BOTH;
		} else {
			$selected_type = $settings['job_type'];
		}

		$jobs_types = Trigger_Action_Remove_Future_Lineage_Cron::_getCronJobTypeList();
						

		if (!$write_access) {
			$form_element_extras = 'disabled="disabled"';
		} else {
			$form_element_extras = '';
		}

		ob_start();
			echo translate('trigger_remove_future_lineage_cron');
			combo_box($prefix.'[job_type]', $jobs_types, FALSE, $selected_type, NULL, $form_element_extras);			
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
		$job_type = array_get_index($request_data, 'job_type', FALSE);
		if (!$job_type) {
			return 'Cron job type not specified';
		}
		
		$jobs_types = Trigger_Action_Remove_Future_Lineage_Cron::_getCronJobTypeList();

		// check if the job_type is valid
		if (!isset($jobs_types[$job_type])) {
			return 'Specified cron job type is invalid';
		}
		$settings['job_type'] = $job_type;

		return FALSE;

	}//end processInterface()
	
	
	/**
	* Returns an array containing cron job action types
	*
	* The returned array contains status(int) => description(string)
	*
	* @return array
	* @access private
	*/
	protected static function _getCronJobTypeList()
	{	
		return Array(
				SQ_REMOVE_FUTURE_LINEAGE_CRON_ADD => "Create",
				SQ_REMOVE_FUTURE_LINEAGE_CRON_DELETE => "Delete",
				SQ_REMOVE_FUTURE_LINEAGE_CRON_BOTH => "Create/Delete",
			);
					
	}//end _getCronJobTypeList()



}//end class

?>
