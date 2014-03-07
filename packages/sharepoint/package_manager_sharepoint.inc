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
* $Id: package_manager_sharepoint.inc,v 1.5 2012/08/30 00:59:19 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/package_manager.inc';

/**
* Package_Manager_Sharepoint
*
* Purpose
*    Manages the install and upgrade of the Sharepoint package and Sharepoint related assets,
*    uses info gleened from the package.xml and asset.xml files
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage sharepoint
*/
class Package_Manager_Sharepoint extends Package_Manager
{


	/**
	* Constructor
	*
	*/
	function Package_Manager_Sharepoint()
	{
		$this->_full_path = SQ_PACKAGES_PATH.'/sharepoint';
		$this->Package_Manager();

	}//end constructor

	
	/**
	* Returns an array of all the asset types in the package that are to be treated as system assets
	* NOTE: Assets will be installed in the order they appear in this list
	*
	* @return array
	* @access public
	* @static
	*/
	function getSystemAssetTypes()
	{
		$system_assets = Array(
							'cron_job_update_sharepoint_record',
							'cron_job_sync_cmis_bridges',
						 );
		$installed_assets = Array();

		foreach ($system_assets as $type_code) {
			if ($GLOBALS['SQ_SYSTEM']->am->installed($type_code)) {
				$installed_assets[] = $type_code;
			}
		}

		return $installed_assets;

	}//end getSystemAssetTypes()


	/**
	* Create the Update Trim Record Cron Job
	*
	* @return object
	* @access public
	*/
	function &createCronJobUpdateSharepointRecord()
	{
		$cron_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (is_null($cron_manager)) {
			trigger_error('Couldn\'t get cron manager, couldn\'t create Update Trim Record cron job', E_USER_WARNING);
			$null = NULL;
			return $null;
		}//end if

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_update_sharepoint_record');

		$cron_job = new Cron_Job_Update_Sharepoint_Record();
		$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
		if (!$cron_manager->addJob($cron_job, $root_user, SQ_LINK_TYPE_2)) {
			trigger_error('Update Trim Record Cron Job NOT CREATED', E_USER_ERROR);
		}//end if

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('cron_job_update_sharepoint_record', $cron_job->id);

		return $cron_job;

	}//end createCronJobUpdateSharepointRecord()	
	
	
	/**
	* Create the Sync CMIS Bridges Cron Job
	*
	* @return object
	* @access public
	*/
	function &createCronJobSyncCMISBridges()
	{
		$cron_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (is_null($cron_manager)) {
			trigger_error('Couldn\'t get cron manager, couldn\'t create Sync CMIS Bridges Cron Job', E_USER_WARNING);
			$null = NULL;
			return $null;
		}

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_sync_cmis_bridges');

		$cron_job = new Cron_Job_Sync_CMIS_Bridges();
		$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
		if (!$cron_manager->addJob($cron_job, $root_user, SQ_LINK_TYPE_2)) {
			trigger_error('Sync CMIS Bridges Cron Job NOT CREATED', E_USER_ERROR);
		}

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('cron_job_sync_cmis_bridges', $cron_job->id);

		return $cron_job;

	}//end createCronJobSyncCMISBridges()

}//end class
?>
