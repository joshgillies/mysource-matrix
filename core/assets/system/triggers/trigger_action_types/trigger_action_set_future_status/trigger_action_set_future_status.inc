<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: trigger_action_set_future_status.inc,v 1.7 2006/01/26 22:34:07 lwright Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

/**
* Trigger Action Set Future Status
*
* A trigger action that sets a future status for an asset
*
*
* @author   Avi Miller <amiller@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Set_Future_Status extends Trigger_Action
{


	/**
	* Execute this action
	*
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:
	* <PRE>
	* Array(
	* 		'status' => int, // the status value to set the asset to
	* 		'when'   => int, // the time/date to set the cron job to
	* 		);
	* </PRE>
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return boolean
	* @access public
	*/
	function execute($settings, &$state)
	{
		// check settings, status
		if (empty($settings['status'])) {
			// if no settings, fail
			return false;
		}

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return false;
			} else {
				$state['asset'] = &$GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		$cron_mgr = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (is_null($cron_mgr)) return false;

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_status');

		$fs = new Cron_Job_Future_Status();
		$fs->setAttrValue('status', $settings['status']);
		$fs->setAttrValue('when', $settings['when']);

		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS)) {
			$user_for_status_change = $GLOBALS['SQ_SYSTEM']->user;
		} else {
			// not doing the whole "security" thing - so pretend we are root
			$user_for_status_change = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($cron_mgr->addJob($fs, $user_for_status_change)) {
			if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($fs->id, 'links')) {
				if ($fs->setAssetToUpdate($state['asset'])) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					$is_updated = true;
					$GLOBALS['SQ_SYSTEM']->am->releaseLock($fs->id, 'links');
				} else {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return false;
				}
			} else {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return false;
			}
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return false;
		}

		return Array(
				'jobid'		=> $fs->id,
				'userid'	=> $user_for_status_change->id,
				'status'	=> $fs->attr('status'),
				'when'		=> $fs->attr('when'),
			   );

	}//end execute()


	/**
	* Get the HTML editing interface for this action. Returns the string instead of printing it
	*
	* @param array		$settings		settings that this condition saves in processInterface()
	* @param string		$prefix			unique prefix
	* @param boolean	$write_access	determines whether the interface is editable
	*
	* @access public
	* @return boolean
	*/
	function getInterface($settings, $prefix, $write_access=false)
	{
		// first, munge the prefix so that we can use the Cron Job interfaces to paint/process
		$munge_prefix = str_replace('[', '_', $prefix);
		$munge_prefix = str_replace(']', '', $munge_prefix);

		// set defaults
		$settings['when']	= array_get_index($settings, 'when');
		$settings['status'] = array_get_index($settings, 'status', SQ_STATUS_LIVE_APPROVAL);
		$settings['prefix']	= $munge_prefix;

		$status_list = Trigger_Action_Set_Future_Status::_getStatusList();

		if (!$write_access) {
			$form_element_extras = 'disabled="disabled"';
		} else {
			$form_element_extras = '';
		}

		// capture anything printed
		ob_start();

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_status');
		$fs = new Cron_Job_Future_Status();

		if (!is_null($settings['status'])) {
			$fs->setAttrValue('status', $settings['status']);
		}

		$edit_fns = $fs->getEditFns();

		$edit_fns->paintInLineBackend($fs, $o, $munge_prefix, $settings['when'], $form_element_extras);
		hidden_field($prefix.'[prefix]', $munge_prefix);

		return ob_get_clean();

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param array	$settings		a container for any data the action might want to save
	* @param string	$prefix			prefix for the form elements
	* @param string	$request_data	array of data corresponding to this action as specified
	* 								by the $prefix in getInterface
	*
	* @access public
	* @return boolean
	*/
	function processInterface(&$settings, $request_data)
	{
		$munge_prefix = $request_data['prefix'];
		$set_status = $_POST[$munge_prefix.'_status'];

		if (!$set_status) return 'Status not specified';

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_status');
		$fs = new Cron_Job_Future_Status();
		$edit_fns = $fs->getEditFns();

		$settings['when'] = $edit_fns->_processWhenBox($fs, $o, $munge_prefix);
		$settings['prefix'] = $munge_prefix;

		$status_list = &Trigger_Action_Set_Future_Status::_getStatusList();

		// check if the status is valid
		if (isset($status_list[$set_status])) {
			$settings['status'] = $set_status;
			return false;
		} else {
			return 'Invalid Status specified';
		}

	}//end processInterface()


	/**
	* Returns an array containing all the valid statuses
	*
	* @access private
	* @return array(int => string)
	*/
	function _getStatusList()
	{
		$status = Array();
		foreach (get_constant_values('SQ_STATUS_') as $status_value) {
			$status[$status_value] = get_status_description($status_value);
		}
		return $status;

	}//end _getStatusList()


}//end class

?>