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
		if ($GLOBALS['SQ_SYSTEM']->am->installed('cron_job_update_trim_record')) {
			return Array('cron_job_update_trim_record');
		} else {
			return Array();
		}

	}//end getSystemAssetTypes()


	/**
	* Create the Update Trim Record Cron Job
	*
	* @return object
	* @access public
	*/
	function &createCronJobUpdateTRIMRecord()
	{
		$cron_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (is_null($cron_manager)) {
			trigger_error('Couldn\'t get cron manager, couldn\'t create Update Trim Record cron job', E_USER_WARNING);
			$null = NULL;
			return $null;
		}//end if

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_update_trim_record');

		$cron_job = new Cron_Job_Update_Trim_Record();
		$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
		if (!$cron_manager->addJob($cron_job, $root_user, SQ_LINK_TYPE_2)) {
			trigger_error('Update Trim Record Cron Job NOT CREATED', E_USER_ERROR);
		}//end if

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('cron_job_update_trim_record', $cron_job->id);

		return $cron_job;

	}//end createCronJobUpdateTRIMRecord()


}//end class
?>
