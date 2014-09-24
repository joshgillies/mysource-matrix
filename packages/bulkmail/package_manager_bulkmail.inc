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
* $Id: package_manager_bulkmail.inc,v 1.12 2012/08/30 00:57:13 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/package_manager.inc';

/**
* Package_Manager_Bulkmail
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Rayne Ong <rong@squiz.net>
* @version $Version$
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class Package_Manager_Bulkmail extends Package_Manager
{


	/**
	* Constructor
	*
	*/
	function Package_Manager_Bulkmail()
	{
		$this->_full_path = SQ_PACKAGES_PATH.'/bulkmail';
		$this->Package_Manager();

	}//end constructor


	/**
	* Returns an array of all the asset types in the package that are to be treated as system assets
	* NOTE: Assets will be installed in the order they appear in this list
	*
	* @return array
	* @access public
	*/
	function getSystemAssetTypes()
	{
		return Array(
				'bulkmail_manager',
				'cron_job_queue_scheduled_job',
			   );

	}//end getSystemAssetTypes()


	/**
	* Create the bulkmail manager system asset
	*
	* @return object
	* @access public
	*/
	function &createBulkmailManager()
	{
		// set default asset name in management.inc and define _getName() function
		$system_management_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_management_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bulkmail_manager');
		$bulkmail_manager = new Bulkmail_Manager();
		$bulkmail_manager_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);
		if (!$bulkmail_manager->create($bulkmail_manager_link)) {
			trigger_localised_error('SCH0007', translate('Search Manager NOT CREATED'), E_USER_ERROR);
		}

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('bulkmail_manager', $bulkmail_manager->id);

		return $bulkmail_manager;

	}//end createBulkmailManager()


	/**
	* Create the Cron Job Batch Broadcast Message System Asset
	*
	* @return object
	* @access public
	*/
	function &createCronJobQueueScheduledJob()
	{
		$cron_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (is_null($cron_manager)) {
			trigger_localised_error('CORE0022', translate('Unable to get cron manager, couldn\'t create expired workflow cron job'), E_USER_WARNING);
			$null = NULL;
			return $null;
		}

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_queue_scheduled_job');

		$cron_job = new Cron_Job_Queue_Scheduled_Job();
		$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
		if (!$cron_manager->addJob($cron_job, $root_user, SQ_LINK_TYPE_2)) {
			trigger_localised_error('CORE0001', sprintf(translate('%s NOT CREATED'), 'Queue Scheduled Bulkmail Cron Job'), E_USER_ERROR);
		}

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('cron_job_queue_scheduled_job', $cron_job->id);

		return $cron_job;

	}//end createCronJobQueueScheduledJob()


}//end class

?>
