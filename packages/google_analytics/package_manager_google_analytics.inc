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
*/

require_once SQ_INCLUDE_PATH.'/package_manager.inc';

/**
* Package_Manager_Google_Analytics
*
* Purpose
*    Manages the initial install and upgrade of the Google Analytics package
*    and its assets, using the info from package and asset XML files and any
*    install/upgrade scripts contained in this class.
*
* @author  Squiz Matrix Team <matrix-team@squiz.com.au>
* @package MySource_Matrix_Packages
* @subpackage google_analytics
*/
class Package_Manager_Google_Analytics extends Package_Manager
{

	/**
	* Constructor
	*
	*/
	function __construct()
	{
		$this->_full_path = SQ_PACKAGES_PATH.'/google_analytics';
		parent::__construct();

	}//end constructor


	//--        FUNCTIONS TO CREATE SYSTEM ASSETS        --//


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
		return Array(
				'cron_job_generate_google_analytics_report',
			   );

	}//end getSystemAssetTypes()



	/**
	* Create the expired workflow cron job
	*
	* @return object
	* @access public
	*/
	function &createCronJobGenerateGoogleAnalyticsReport()
	{
		$cron_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (is_null($cron_manager)) {
			trigger_localised_error('CORE0022', translate('Unable to get Scheduled Jobs manager, couldn\'t create Generate Google Analytics Reports Scheduled Job'), E_USER_WARNING);
			$null = NULL;
			return $null;
		}

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_generate_google_analytics_report');

		$cron_job = new Cron_Job_Generate_Google_Analytics_Report();
		$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
		if (!$cron_manager->addJob($cron_job, $root_user, SQ_LINK_TYPE_2)) {
			trigger_localised_error('CORE0001', sprintf(translate('%s NOT CREATED'), translate('Generate Google Analytics Reports Scheduled Job')), E_USER_ERROR);
		}

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('cron_job_generate_google_analytics_report', $cron_job->id);

		return $cron_job;

	}//end createCronJobGenerateGoogleAnalyticsReport()


}//end class

?>
