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
* $Id: package_manager_funnelback.inc,v 1.4 2012/08/30 00:58:43 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/package_manager.inc';

/**
* Funnelback 
*
* Purpose
*    Manages the install and upgrade of the Funnelback package and Funnelback assets,
*    uses info gleened from the package.xml and asset.xml files
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage funnelback
*/
class Package_Manager_Funnelback extends Package_Manager
{


	/**
	* Constructor
	*
	*/
	function Package_Manager_Funnelback()
	{
		$this->_full_path = SQ_PACKAGES_PATH.'/funnelback';
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
							'funnelback_manager',
							'cron_job_funnelback_log_rotation',
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
	* Create the funnelback manager system asset
	*
	* @return object
	* @access public
	*/
	function &createFunnelbackManager()
	{
		$system_management_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_management_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('funnelback_manager');
		$funnelback_manager = new Funnelback_Manager();
		$funnelback_manager_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);
		if (!$funnelback_manager->create($funnelback_manager_link)) {
			trigger_localised_error('FNB0010', E_USER_ERROR);
		}

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('funnelback_manager', $funnelback_manager->id);

		return $funnelback_manager;

	}//end createFunnelbackManager()


	/**
	* Create the funnelback log rotation cron job
	*
	* @return object
	* @access public
	*/
	function &createCronJobFunnelbackLogRotation()
	{
		$cron_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (is_null($cron_manager)) {
			trigger_error('Couldn\'t get cron manager, couldn\'t create Funnelback Log Rotation cron job', E_USER_WARNING);
			$null = NULL;
			return $null;
		}

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_funnelback_log_rotation');

		$cron_job = new Cron_Job_Funnelback_Log_Rotation();
		$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
		if (!$cron_manager->addJob($cron_job, $root_user, SQ_LINK_TYPE_2)) {
			trigger_error('Funnelback Log Rotation Cron Job NOT CREATED', E_USER_ERROR);
		}

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('cron_job_funnelback_log_rotation', $cron_job->id);

		return $cron_job;

	}//end createCronJobFunnelbackLogRotation()


}//end class

?>
