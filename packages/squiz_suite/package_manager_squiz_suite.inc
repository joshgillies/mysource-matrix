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
* $Id: package_manager_squiz_suite.inc,v 1.4 2012/08/30 00:59:25 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/package_manager.inc';

/**
* Squiz Suite Package
*
* Purpose
*    Package containing assets to talk to Squiz Suite products
*    uses info gleened from the package.xml and asset.xml files
*
* @author  Benjamin Pearson <bpearson@squiz.com.au>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage squiz_suite
*/
class Package_Manager_Squiz_Suite extends Package_Manager
{


	/**
	* Constructor
	*
	*/
	function Package_Manager_Squiz_Suite()
	{
		$this->_full_path = SQ_PACKAGES_PATH.'/squiz_suite';
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
							'suite_manager',
							'cron_job_sync_suite_products',
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
	* Create the suite manager system asset
	*
	* @return object
	* @access public
	*/
	function &createSuiteManager()
	{
		$web_services_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('web_folder_web_services');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('suite_manager');
		$suite_manager = new Suite_Manager();
        $suite_manager->setAttrValue('name', 'Squiz Suite Manager');
		$suite_manager_link = Array('asset' => &$web_services_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);
		if (!$suite_manager->create($suite_manager_link)) {
			trigger_localised_error('SQS0001', translate('Unable to install Squiz Suite Manager'), E_USER_ERROR)
		}

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('suite_manager', $suite_manager->id);

		return $suite_manager;

	}//end createSuiteManager()


	/**
	* Create the sync suite products cron job
	*
	* @return object
	* @access public
	*/
	function &createCronJobSyncSuiteProducts()
	{
		$cron_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (is_null($cron_manager)) {
			trigger_error('Couldn\'t get cron manager, couldn\'t create Sync Squiz Suite Products cron job', E_USER_WARNING);
			$null = NULL;
			return $null;
		}

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_sync_suite_products');

		$cron_job = new Cron_Job_Sync_Suite_Products();
		$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
		if (!$cron_manager->addJob($cron_job, $root_user, SQ_LINK_TYPE_2)) {
			trigger_error('Sync Squiz Suite Products Cron Job NOT CREATED', E_USER_ERROR);
		}

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('cron_job_sync_suite_products', $cron_job->id);

		return $cron_job;

	}//end createCronJobSyncSuiteProducts()


}//end class

?>
