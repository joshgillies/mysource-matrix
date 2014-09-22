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
* $Id: trigger_action_delete_future_permission.inc,v 1.2 2012/08/30 01:09:13 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

/**
* Trigger_Action_Delete_Future_Permission
*
* A trigger action that removes future permission cron jobs for an asset
*
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Delete_Future_Permission extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:
	* <PRE>
	* Array(
	* 		'permission' => string, // the asset permission
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
		// check settings, permission
		if (empty($settings['permission'])) {
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

		$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (is_null($cron_mgr)) return FALSE;

		//get the future permission cron jobs of this asset
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_permission');
		$fp_jobs = Cron_Job_Future_Permission::getActiveJobs($state['asset']);
		if (empty($fp_jobs)) return TRUE;

		$jobids = Array();

		foreach ($fp_jobs as $job) {
			if ($job->attr('permission') == $settings['permission']) {
				if ($cron_mgr->removeJob($job)) {
					$jobids[] = $job->id;
				} else {
					trigger_localised_error('CRON0065', sprintf(translate('Unable to remove the future permission cron job id: %s'), $job->id), E_USER_WARNING)
				}
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
		if (empty($settings['permission'])) {
			// set to default
			$selected_permission = SQ_PERMISSION_READ;
		} else {
			$selected_permission = $settings['permission'];
		}

		$permission_list = self::_getPermissionList();

		if (!$write_access) {
			$form_element_extras = 'disabled="disabled"';
		} else {
			$form_element_extras = '';
		}

		ob_start();
			echo translate('Delete all future permission Scheduled Jobs that would change the asset\'s').'&nbsp;';

			combo_box($prefix.'[permission]', $permission_list, FALSE, $selected_permission, NULL, $form_element_extras);
			echo '&nbsp;'.translate('Permission');

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
		$permission = array_get_index($request_data, 'permission', NULL);
		if (is_null($permission)) {
			return 'Permission has not been specified';
		}

		$permission_list = self::_getPermissionList();

		// check if the permission is valid
		if (!isset($permission_list[$permission])) {
			return 'Specified permission is invalid';
		}
		$settings['permission'] = $permission;

		return FALSE;

	}//end processInterface()


	/**
	* Returns an array containing all the valid permissions
	*
	* The returned array contains permission(int) => description(string)
	*
	* @return array
	* @access private
	*/
	protected static function _getPermissionList()
	{
		return Array(SQ_PERMISSION_READ => 'Read', SQ_PERMISSION_WRITE => 'Write', SQ_PERMISSION_ADMIN => 'Admin');

	}//end _getPermissionList()


}//end class

?>