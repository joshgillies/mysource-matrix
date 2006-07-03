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
* $Id: package_manager_cms.inc,v 1.6 2006/07/03 05:33:16 lwright Exp $
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
* @version $Revision: 1.6 $
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