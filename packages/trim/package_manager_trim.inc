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
* $Id: package_manager_trim.inc,v 1.3 2012/08/30 00:59:28 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/package_manager.inc';

/**
* Package_Manager_TRIM
*
* Purpose
*    Manages the install and upgrade of the TRIM package and TRIM related assets,
*    uses info gleened from the package.xml and asset.xml files
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage data
*/
class Package_Manager_TRIM extends Package_Manager
{


	/**
	* Constructor
	*
	*/
	function Package_Manager_TRIM()
	{
		$this->_full_path = SQ_PACKAGES_PATH.'/trim';
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
		$system_asset_types = Array();
		if ($GLOBALS['SQ_SYSTEM']->am->installed('cron_job_update_trim_record')) {
			$system_asset_types[] = 'cron_job_update_trim_record';
		}
		if ($GLOBALS['SQ_SYSTEM']->am->installed('cron_job_update_hprm_record')) {
			$system_asset_types[] = 'cron_job_update_hprm_record';
		}

		return $system_asset_types;

	}//end getSystemAssetTypes()


	/**
	* Create the specified type TRIM package cron job
	*
	* @param $string $type_code	TRIM cron job type code
	*
	* @return object
	* @access private
	*/
	function _createCronJob($type_code)
	{
		$cron_job = NULL;
		if (!in_array($type_code, Array('cron_job_update_trim_record', 'cron_job_update_hprm_record'))) {
			return $cron_job;
		}

		$cron_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (is_null($cron_manager)) {
			trigger_error('Couldn\'t get cron manager, couldn\'t create Update Trim Record cron job', E_USER_WARNING);
			$null = NULL;
			return $null;
		}//end if

		$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
		$cron_job = new $type_code;
		$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
		if (!$cron_manager->addJob($cron_job, $root_user, SQ_LINK_TYPE_2)) {
			trigger_error('Could not create Cron Job with type code "'.$type_code.'".', E_USER_ERROR);
		}//end if

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset($type_code, $cron_job->id);

		return $cron_job;


	}//end _createCronJob()


	/**
	* Create the Update Trim Record Cron Job
	*
	* @return object
	* @access public
	*/
	function createCronJobUpdateTrimRecord()
	{
		return $this->_createCronJob('cron_job_update_trim_record');

	}//end createCronJobUpdateTrimRecord()


	/**
	* Create the Update Trim Record Cron Job
	*
	* @return object
	* @access public
	*/
	function createCronJobUpdateHprmRecord()
	{
		return $this->_createCronJob('cron_job_update_hprm_record');

	}//end createCronJobUpdateHprmRecord()

}//end class
?>
