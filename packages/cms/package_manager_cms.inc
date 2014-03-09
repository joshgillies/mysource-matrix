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
* $Id: package_manager_cms.inc,v 1.10 2012/08/30 00:58:07 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/package_manager.inc';

/**
* Package_Manager_CMS
*
* Purpose
*    Manages the install and upgrade of the CMS package and CMS assets,
*    uses info gleaned from the package.xml and asset.xml files
*
* @author  Blair Robertson <brobertson@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Package_Manager_CMS extends Package_Manager
{

	/**
	* Constructor
	*
	*/
	function Package_Manager_CMS()
	{
		$this->_full_path = SQ_PACKAGES_PATH.'/cms';
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
							'cron_job_manage_pending_accounts',
							'cron_job_manage_incomplete_submissions',
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
	* Create the generate log report cron job
	*
	* @return object
	* @access public
	*/
	function &createCronJobManagePendingAccounts()
	{
		$cron_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (is_null($cron_manager)) {
			trigger_error('Couldn\'t get cron manager, couldn\'t create Manage Pending Accounts cron job', E_USER_WARNING);
			$null = NULL;
			return $null;
		}

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_manage_pending_accounts');

		$cron_job = new Cron_Job_Manage_Pending_Accounts();
		$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
		if (!$cron_manager->addJob($cron_job, $root_user, SQ_LINK_TYPE_2)) {
			trigger_error('Manage Pending Accounts Cron Job NOT CREATED', E_USER_ERROR);
		}

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('cron_job_manage_pending_accounts', $cron_job->id);

		return $cron_job;

	}//end createCronJobManagePendingAccounts()


	/**
	* Create the generate log report cron job
	*
	* @return object
	* @access public
	*/
	function &createCronJobManageIncompleteSubmissions()
	{
		$cron_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (is_null($cron_manager)) {
			trigger_error('Couldn\'t get cron manager, couldn\'t create Manage Pending Accounts cron job', E_USER_WARNING);
			$null = NULL;
			return $null;
		}

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_manage_incomplete_submissions');

		$cron_job = new Cron_Job_Manage_Incomplete_Submissions();
		$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
		if (!$cron_manager->addJob($cron_job, $root_user, SQ_LINK_TYPE_2)) {
			trigger_error('Manage Incomplete Submissions Cron Job NOT CREATED', E_USER_ERROR);
		}

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('cron_job_manage_incomplete_submissions', $cron_job->id);

		return $cron_job;

	}//end createCronJobManagePendingAccounts()


}//end class

?>
