<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: package_manager_cms.inc,v 1.7 2006/12/07 00:10:13 emcdonald Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/package_manager.inc';

/**
* Package_Manager_CMS
*
* Purpose
*    Manages the install and upgrade of the CMS package and CMS assets,
*    uses info gleened from the package.xml and asset.xml files
*
* @author  Blair Robertson <brobertson@squiz.net>
* @version $Revision: 1.7 $
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
		if ($GLOBALS['SQ_SYSTEM']->am->installed('cron_job_manage_pending_accounts')) {
			return Array('cron_job_manage_pending_accounts');
		} else {
			return Array();
		}

	}//end getSystemAssetTypes()


	/**
	* Create the generate log report cron job
	*
	* @return object
	* @access public
	*/
	function &createCronJobManagePendingAccounts()
	{
		$cron_manager =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (is_null($cron_manager)) {
			trigger_error('Couldn\'t get cron manager, couldn\'t create Manage Pending Accounts cron job', E_USER_WARNING);
			$null = NULL;
			return $null;
		}

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_manage_pending_accounts');

		$cron_job =& new Cron_Job_Manage_Pending_Accounts();
		$root_user =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
		if (!$cron_manager->addJob($cron_job, $root_user, SQ_LINK_TYPE_2)) {
			trigger_error('Manage Pending Accounts Cron Job NOT CREATED', E_USER_ERROR);
		}
		pre_echo('Manage Pending Accounts Cron Job Id: '.$cron_job->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('cron_job_manage_pending_accounts', $cron_job->id);

		return $cron_job;

	}//end createCronJobManagePendingAccounts()


}//end class

?>