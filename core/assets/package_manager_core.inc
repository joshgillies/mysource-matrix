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
* $Id: package_manager_core.inc,v 1.48 2013/10/18 03:57:26 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/package_manager.inc';

/**
* Package_Manager_Core
*
* Purpose
*    Manages the install and upgrade of the core package and core assets,
*    uses info gleened from the package.xml and asset.xml files
*
* @author  Blair Robertson <brobertson@squiz.net>
* @version $Revision: 1.48 $
* @package MySource_Matrix
*/
class Package_Manager_Core extends Package_Manager
{


	/**
	* Constructor
	*
	*/
	function Package_Manager_Core()
	{
		$this->_full_path = SQ_CORE_PACKAGE_PATH;
		$this->Package_Manager();

	}//end constructor


	/**
	* Returns the name of the package that this management class is working for
	*
	* @return string
	* @access public
	*/
	function getPackageName()
	{
		// need to override because we don't really want to have to name the Package_Manager___Core__ :)
		return '__core__';

	}//end getPackageName()


	/**
	* Installs/Updates the passed package in the system
	*
	* @param array	$update_assets	An array of asset types to update
	*								details for. If empty, will do all assets
	*
	* @return boolean
	* @access public
	*/
	function updatePackageDetails($update_assets=Array())
	{
		// If there were failures in core packages, return FALSE as dependency
		// resolution won't work (if a core package relies on another package,
		// what is the world coming to?)
		if ($this->checkPackageDetails($update_assets) !== TRUE) {
			return FALSE;
		}

		// open the transaction
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// Now we can do the normal package update
		if (!parent::updatePackageDetails($update_assets)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// Commit the transaction if we got this far
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end updatePackageDetails()


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
				'root_folder',
				'system_management_folder',
				'log_manager',
				'web_folder_designs',
				'web_folder_media',
				'web_folder_users',
				'public_user',
				'trash_folder',
				'system_user_group',
				'root_user',
				'authentication_folder',
				'cron_manager',
				'trigger_manager',
				'layout_manager',
				'remap_manager',
				'cache_manager',
				'designs_folder',
				'login_design',
				'ees_login_design',
				'password_change_design',
				'cron_job_expired_workflow',
				'cron_job_generate_log_reports',
				'cron_job_escalate_safe_trash',
				'tag_manager',
				'cron_job_metadata_triggers',
				'cron_job_expired_matrix_cache',
				'cron_job_expired_matrix_locks',
			   );

	}//end getSystemAssetTypes()


	/**
	* Create the root folder system asset
	*
	* @return object
	* @access public
	*/
	function &createRootFolder()
	{
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('root_folder');

		$root_folder = new Root_Folder();
		$link = Array();
		if (!$root_folder->create($link)) {
			trigger_localised_error('CORE0001', E_USER_ERROR, 'Root Folder');
		}

		pre_echo('Root Folder Asset Id : '.$root_folder->id);
		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('root_folder', $root_folder->id);

		return $root_folder;

	}//end createRootFolder()


	/**
	* Create the trash folder system asset
	*
	* @return object
	* @access public
	*/
	function &createTrashFolder()
	{
		$root_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('trash_folder');
		$trash_link = Array(
						'asset'			=> &$root_folder,
						'link_type'		=> SQ_LINK_TYPE_1,
						'is_exclusive'	=> 1,
						'sort_order'	=> 0,
						'is_locked' 	=> 1,
					  );
		$trash_folder = new Trash_Folder();
		if (!$trash_folder->create($trash_link)) {
			trigger_localised_error('CORE0001', E_USER_ERROR, 'Trash Folder');
		}
		pre_echo('Trash Asset Id : '.$trash_folder->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('trash_folder', $trash_folder->id);
		return $trash_folder;

	}//end createTrashFolder()


	/**
	* Create the public access user system asset
	*
	* @return object
	* @access public
	*/
	function &createPublicUser()
	{
		$users_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('web_folder_users');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('public_user');
		$public_user = new Public_User();
		$public_user_link = Array(
				'asset' 	=> &$users_folder, 
				'link_type' => SQ_LINK_TYPE_1,
				'is_locked' => 1,
			);
		if (!$public_user->create($public_user_link)) {
			trigger_localised_error('CORE0001', E_USER_ERROR, 'Public Access User');
		}
		pre_echo('Public Access User Asset Id : '.$public_user->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('public_user', $public_user->id);

		return $public_user;

	}//end createPublicUser()


	/**
	* Create the system management folder system asset
	*
	* @return object
	* @access public
	*/
	function &createSystemManagementFolder()
	{
		$root_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('system_management_folder');
		$system_management_folder = new System_Management_Folder();
		$system_management_folder_link = Array(
				'asset' 		=> &$root_folder, 
				'link_type' 	=> SQ_LINK_TYPE_1, 
				'is_exclusive' 	=> 1, 
				'sort_order' 	=> 0, 
				'is_locked' 	=> 1,
			);
		if (!$system_management_folder->create($system_management_folder_link)) {
			trigger_localised_error('CORE0001', E_USER_ERROR, 'System Management Folder');
		}
		pre_echo('System Management Asset Id : '.$system_management_folder->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('system_management_folder', $system_management_folder->id);
		return $system_management_folder;

	}//end createSystemManagementFolder()


	/**
	* Create the system user group system asset
	*
	* @return object
	* @access public
	*/
	function &createSystemUserGroup()
	{
		$system_management_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_management_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('system_user_group');
		$system_user_group = new System_User_Group();
		$system_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);
		if (!$system_user_group->create($system_link)) {
			trigger_localised_error('CORE0001', E_USER_ERROR, 'System Admin Group');
		}
		pre_echo('System Administrators User Group Asset Id : '.$system_user_group->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('system_user_group', $system_user_group->id);
		return $system_user_group;

	}//end createSystemUserGroup()


	/**
	* Create the root user system asset
	*
	* @return object
	* @access public
	*/
	function &createRootUser()
	{
		// turn off password validation
		$current_run_level = $GLOBALS['SQ_SYSTEM']->getRunLevel();
		$GLOBALS['SQ_SYSTEM']->setRunLevel($current_run_level - SQ_SECURITY_PASSWORD_VALIDATION);

		$system_user_group = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_user_group');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('root_user');
		$root_user = new Root_User();
		$user_link = Array('asset' => &$system_user_group, 'link_type' => SQ_LINK_TYPE_1);

		$root_pass = isset($_SESSION['install_root_pass']) ? $_SESSION['install_root_pass'] : 'root';

		$root_user->setAttrValue('password',   $root_pass);
		$root_user->setAttrValue('first_name', 'Root');
		$root_user->setAttrValue('last_name',  'User');
		$host = array_get_index($_SERVER, 'HTTP_HOST', '');
		$root_email = (SQ_CONF_DEFAULT_EMAIL) ? SQ_CONF_DEFAULT_EMAIL : ('root@'.((SQ_PHP_CLI) ? php_uname('n') : $host));
		$root_user->setAttrValue('email', $root_email);

		if (!$root_user->create($user_link)) {
			trigger_localised_error('CORE0001', E_USER_ERROR, 'Root User');
		}

		pre_echo('Root User Asset Id : '.$root_user->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('root_user', $root_user->id);
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
		return $root_user;

	}//end createRootUser()


	/**
	* Create the cron manager system asset
	*
	* @return object
	* @access public
	*/
	function &createCronManager()
	{
		$system_management_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_management_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_manager');
		$cron_manager = new Cron_Manager();
		$cron_manager_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);
		if (!$cron_manager->create($cron_manager_link)) {
			trigger_localised_error('CORE0001', E_USER_ERROR, 'Cron Manager');
		}
		pre_echo('Cron Manager Asset Id : '.$cron_manager->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('cron_manager', $cron_manager->id);
		return $cron_manager;

	}//end createCronManager()


	/**
	* Create the trigger manager system asset
	*
	* @return object
	* @access public
	*/
	function &createTriggerManager()
	{
		$system_management_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_management_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('trigger_manager');
		$trigger_manager = new Trigger_Manager();
		$trigger_manager_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);
		if (!$trigger_manager->create($trigger_manager_link)) {
			trigger_localised_error('CORE0001', E_USER_ERROR, 'Trigger Manager');
		}
		pre_echo('Trigger Manager Asset Id : '.$trigger_manager->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('trigger_manager', $trigger_manager->id);
		return $trigger_manager;

	}//end createTriggerManager()


	/**
	* Create the log manager system asset
	*
	* @return object
	* @access public
	*/
	function &createLogManager()
	{
		$system_management_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_management_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('log_manager');
		$log_manager = new Log_Manager();
		$log_manager_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);
		if (!$log_manager->create($log_manager_link)) {
			trigger_localised_error('CORE0001', E_USER_ERROR, 'Trigger Manager');
		}
		pre_echo('Log Manager Asset Id : '.$log_manager->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('log_manager', $log_manager->id);
		return $log_manager;

	}//end createLogManager()


	/**
	* Create the layout manager system asset
	*
	* @return object
	* @access public
	*/
	function &createLayoutManager()
	{
		$system_management_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_management_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('layout_manager');
		$layout_manager = new Layout_Manager();
		$layout_manager_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);
		if (!$layout_manager->create($layout_manager_link)) {
			trigger_localised_error('CORE0001', E_USER_ERROR, 'Layout Manager');
		}
		pre_echo('Layout Manager Asset Id : '.$layout_manager->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('layout_manager', $layout_manager->id);
		return $layout_manager;

	}//end createLayoutManager()


	/**
	* Create the remap manager system asset
	*
	* @return object
	* @access public
	*/
	function &createRemapManager()
	{
		$system_management_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_management_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('remap_manager');
		$remap_manager = new Remap_Manager();
		$remap_manager_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);
		if (!$remap_manager->create($remap_manager_link)) {
			trigger_localised_error('CORE0001', E_USER_ERROR, 'Remap Manager');
		}
		pre_echo('Remap Manager Asset Id : '.$remap_manager->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('remap_manager',$remap_manager->id);
		return $remap_manager;

	}//end createRemapManager()


	/**
	* Create the cache manager system asset
	*
	* @return object
	* @access public
	*/
	function &createCacheManager()
	{
		$system_management_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_management_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cache_manager');
		$cache_manager = new Cache_Manager();
		$cache_manager_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);
		if (!$cache_manager->create($cache_manager_link)) {
			trigger_localised_error('CORE0001', E_USER_ERROR, 'Cache Manager');
		}
		pre_echo('Cache Manager Asset Id : '.$cache_manager->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('cache_manager', $cache_manager->id);
		return $cache_manager;

	}//end createCacheManager()


	/**
	* Create the designs folder system asset
	*
	* @return object
	* @access public
	*/
	function &createDesignsFolder()
	{
		$system_management_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_management_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('designs_folder');
		$designs_folder = new Designs_Folder();
		$designs_folder_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);
		if (!$designs_folder->create($designs_folder_link)) {
			trigger_localised_error('CORE0001', E_USER_ERROR, 'Designs Folder');
		}
		pre_echo('Design Folder Asset Id : '.$designs_folder->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('designs_folder', $designs_folder->id);
		return $designs_folder;

	}//end createDesignsFolder()


	/**
	* Create the authentication folder system asset
	*
	* @return object
	* @access public
	*/
	function &createAuthenticationFolder()
	{
		$system_management_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_management_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('authentication_folder');
		$authentication_folder = new Authentication_Folder();
		$authentication_folder_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);
		if (!$authentication_folder->create($authentication_folder_link)) {
			trigger_localised_error('CORE0001', E_USER_ERROR, 'Authentication Folder');
		}
		pre_echo('Authentication Folder Asset Id : '.$authentication_folder->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('authentication_folder', $authentication_folder->id);
		return $authentication_folder;

	}//end createAuthenticationFolder()


	/**
	* Create the login design system asset
	*
	* @return object
	* @access public
	*/
	function &createLoginDesign()
	{
		$designs_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('designs_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('login_design');
		$login_design = new Login_Design();
		$login_design_link = Array('asset' => &$designs_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);
		$login_design->setAttrValue('id_name', 'login_design');
		if (!$login_design->create($login_design_link)) {
			trigger_localised_error('CORE0001', E_USER_ERROR, 'Login Design');
		}
		pre_echo('Login Design Asset Id : '.$login_design->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('login_design', $login_design->id);
		return $login_design;

	}//end createLoginDesign()


	/**
	* Create the login design system asset
	*
	* @return object
	* @access public
	*/
	function &createEESLoginDesign()
	{
		$designs_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('designs_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('ees_login_design');
		$login_design = new EES_Login_Design();
		$login_design_link = Array('asset' => &$designs_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);
		$login_design->setAttrValue('id_name', 'ees_login_design');
		if (!$login_design->create($login_design_link)) {
			trigger_localised_error('CORE0001', E_USER_ERROR, 'EES Login Design');
		}
		pre_echo('EES Login Design Asset Id : '.$login_design->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('ees_login_design', $login_design->id);
		return $login_design;

	}//end createLoginDesign()
	
	/**
	* Create the password change design system asset
	*
	* @return object
	* @access public
	*/
	function &createPasswordChangeDesign()
	{
		$designs_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('designs_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('password_change_design');
		$password_change_design = new Password_Change_Design();
		$password_change_design_link = Array('asset' => &$designs_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);
		$password_change_design->setAttrValue('id_name', 'password_change_design');
		if (!$password_change_design->create($password_change_design_link)) {
				trigger_localised_error('CORE0001', E_USER_ERROR, 'Password Change Design');
		}
		pre_echo('Password Change Design Asset Id : '.$password_change_design->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('password_change_design', $password_change_design->id);
		return $password_change_design;

	}//end createPasswordChangeDesign()


	/**
	* Create the web folder for storing site designs
	*
	* @return object
	* @access public
	*/
	function &createWebFolderDesigns()
	{
		$root_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('web_folder_designs');
		$folder_link = Array(
				'asset' 		=> &$root_folder, 
				'link_type' 	=> SQ_LINK_TYPE_1, 
				'is_exclusive' 	=> 1, 
				'is_locked' 	=> 1,
			);
		$designs_folder = new Web_Folder_Designs();
		$designs_folder->setAttrValue('name', 'Designs Folder');
		if (!$designs_folder->create($folder_link)) {
			trigger_localised_error('CORE0001', E_USER_ERROR, 'Designs Folder');
		}
		pre_echo('Designs Folder Id : '.$designs_folder->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('web_folder_designs', $designs_folder->id);
		return $designs_folder;

	}//end createWebFolderDesigns()


	/**
	* Create the web folder for storing media (images/movies etc)
	*
	* @return object
	* @access public
	*/
	function &createWebFolderMedia()
	{
		$root_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('web_folder_media');
		$folder_link = Array(
				'asset' 		=> &$root_folder, 
				'link_type' 	=> SQ_LINK_TYPE_1, 
				'is_exclusive' 	=> 1, 
				'is_locked' 	=> 1,
			);
		$media_folder = new Web_Folder_Media();
		$media_folder->setAttrValue('name', 'Media Folder');
		if (!$media_folder->create($folder_link)) {
			trigger_localised_error('CORE0001', E_USER_ERROR, 'Media Folder');
		}
		pre_echo('Media Folder Id : '.$media_folder->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('web_folder_media', $media_folder->id);
		return $media_folder;

	}//end createWebFolderMedia()


	/**
	* Create the web folder for storing users
	*
	* @return object
	* @access public
	*/
	function &createWebFolderUsers()
	{
		$root_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('web_folder_users');
		$folder_link = Array(
				'asset' 		=> &$root_folder, 
				'link_type' 	=> SQ_LINK_TYPE_1, 
				'is_exclusive' 	=> 1, 
				'is_locked' 	=> 1,
			);
		$users_folder = new Web_Folder_Users();
		$users_folder->setAttrValue('name', 'Users Folder');
		if (!$users_folder->create($folder_link)) {
			trigger_localised_error('CORE0001', E_USER_ERROR, 'Users Folder');
		}
		pre_echo('Users Folder Id : '.$users_folder->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('web_folder_users', $users_folder->id);
		return $users_folder;

	}//end createWebFolderUsers()


	/**
	* Create the expired workflow cron job
	*
	* @return object
	* @access public
	*/
	function &createCronJobExpiredWorkflow()
	{
		$cron_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (is_null($cron_manager)) {
			trigger_localised_error('CORE0022', E_USER_WARNING);
			$null = NULL;
			return $null;
		}

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_expired_workflow');

		$cron_job = new Cron_Job_Expired_WorkFlow();
		$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
		if (!$cron_manager->addJob($cron_job, $root_user, SQ_LINK_TYPE_2)) {
			trigger_localised_error('CORE0001', E_USER_ERROR, 'Expired Workflow Cron Job');
		}
		pre_echo('Expired Workflow Cron Job Id: '.$cron_job->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('cron_job_expired_workflow', $cron_job->id);

		return $cron_job;

	}//end createCronJobExpiredWorkflow()


	/**
	* Create the generate log report cron job
	*
	* @return object
	* @access public
	*/
	function &createCronJobGenerateLogReports()
	{
		$cron_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (is_null($cron_manager)) {
			trigger_error('Couldn\'t get cron manager, couldn\'t create generate log reports cron job', E_USER_WARNING);
			$null = NULL;
			return $null;
		}

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_generate_log_reports');

		$cron_job = new Cron_Job_Generate_Log_Reports();
		$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
		if (!$cron_manager->addJob($cron_job, $root_user, SQ_LINK_TYPE_2)) {
			trigger_error('Generate Log Reports Cron Job NOT CREATED', E_USER_ERROR);
		}
		pre_echo('Generate Log Reports Cron Job Id: '.$cron_job->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('cron_job_generate_log_reports', $cron_job->id);

		return $cron_job;

	}//end createCronJobGenerateLogReports()


	/**
	* Create the Escalate Safe Trash cron job
	*
	* @return object
	* @access public
	*/
	function &createCronJobEscalateSafeTrash()
	{
		$cron_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (is_null($cron_manager)) {
			trigger_error('Couldn\'t get cron manager, couldn\'t create Escalate Safe Trash Cron Job', E_USER_WARNING);
			$null = NULL;
			return $null;
		}

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_generate_log_reports');

		$cron_job = new Cron_Job_Escalate_Safe_Trash();
		$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
		if (!$cron_manager->addJob($cron_job, $root_user, SQ_LINK_TYPE_2)) {
			trigger_error('Escalate Safe Trash Cron Job NOT CREATED', E_USER_ERROR);
		}
		pre_echo('Escalate Safe Trash Cron Job Id: '.$cron_job->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('cron_job_escalate_safe_trash', $cron_job->id);

		return $cron_job;

	}//end createCronJobEscalateSafeTrash()


	/**
	* Create the tag manager system asset
	*
	* @return object
	* @access public
	*/
	function &createTagManager()
	{
		$system_management_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_management_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('tag_manager');
		$tag_manager = new Tag_Manager();
		$tag_manager_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);
		if (!$tag_manager->create($tag_manager_link)) {
			trigger_localised_error('CORE0001', E_USER_ERROR, 'Trigger Manager');
		}
		pre_echo('Tag Manager Asset Id : '.$tag_manager->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('tag_manager', $tag_manager->id);
		return $tag_manager;

	}//end createTagManager()


	/**
	* Create the Metadata Triggers cron job
	*
	* @return object
	* @access public
	*/
	function &createCronJobMetadataTriggers()
	{
		$cron_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (is_null($cron_manager)) {
			trigger_error('Couldn\'t get cron manager, couldn\'t create Metadata Trigger cron job', E_USER_WARNING);
			$null = NULL;
			return $null;
		}

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_metadata_triggers');

		$cron_job = new Cron_Job_Metadata_Triggers();
		$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
		if (!$cron_manager->addJob($cron_job, $root_user, SQ_LINK_TYPE_2)) {
			trigger_error('Metadata Triggers Cron Job NOT CREATED', E_USER_ERROR);
		}
		pre_echo('Metadata Triggers Cron Job Id: '.$cron_job->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('cron_job_metadata_triggers', $cron_job->id);

		return $cron_job;

	}//end createCronJobMetadataTriggers()


	/**
	* Create the Expired Matrix Cache cron job
	*
	* @return object
	* @access public
	*/
	function &createCronJobExpiredMatrixCache()
	{
		$cron_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (is_null($cron_manager)) {
			trigger_error('Couldn\'t get cron manager, couldn\'t create Expired Matrix Cache cron job', E_USER_WARNING);
			$null = NULL;
			return $null;
		}

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_expired_matrix_cache');

		$cron_job = new Cron_Job_Expired_Matrix_Cache();
		$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
		if (!$cron_manager->addJob($cron_job, $root_user, SQ_LINK_TYPE_2)) {
			trigger_error('Expired Matrix Cache Cron Job NOT CREATED', E_USER_ERROR);
		}
		pre_echo('Expired Matrix Cache Cron Job Id: '.$cron_job->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('cron_job_expired_matrix_cache', $cron_job->id);

		return $cron_job;

	}//end createCronJobExpiredMatrixCache()


	/**
	* Create the Expired Matrix Locks cron job
	*
	* @return object
	* @access public
	*/
	function &createCronJobExpiredMatrixLocks()
	{
		$cron_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (is_null($cron_manager)) {
			trigger_error('Couldn\'t get cron manager, couldn\'t create Expired Matrix Locks cron job', E_USER_WARNING);
			$null = NULL;
			return $null;
		}

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_expired_matrix_locks');

		$cron_job = new Cron_Job_Expired_Matrix_Locks();
		$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
		if (!$cron_manager->addJob($cron_job, $root_user, SQ_LINK_TYPE_2)) {
			trigger_error('Expired Matrix Locks Cron Job NOT CREATED', E_USER_ERROR);
		}
		pre_echo('Expired Matrix Locks Cron Job Id: '.$cron_job->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('cron_job_expired_matrix_locks', $cron_job->id);

		return $cron_job;

	}//end createCronJobExpiredMatrixLocks()

}//end class

?>
