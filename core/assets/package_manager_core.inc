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
* $Id: package_manager_core.inc,v 1.25 2005/04/06 07:08:11 mmcintyre Exp $
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
* @version $Revision: 1.25 $
* @package MySource_Matrix
*/
class Package_Manager_Core extends Package_Manager
{

	/**
	* @var array
	* Holds all the searchable components that ALL asset's should have
	*
	*/
	var $default_components = Array(
								'__name__'		=> Array(
													'description'		=> 'Asset Name',
													'default_weighting'	=> '1',
												   ),
								'__created__'	=> Array(
													'description'		=> 'Created Date',
													'default_weighting'	=> '1',
												   ),
								'__updated__'	=> Array(
													'description'		=> 'Updated Date',
													'default_weighting'	=> '1',
												   ),
								'__published__'	=> Array(
													'description'		=> 'Published Date',
													'default_weighting'	=> '1',
												   ),
							  );


	/**
	* Constructor
	*
	*/
	function Package_Manager_Core()
	{
		$this->_full_path = dirname(__FILE__);
		$this->Package_Manager();

	}//end constructor


	/**
	* Returns the name of the package that this management class is working for
	*
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
	* @param Array $update_assets		An array of asset types to update
	*									details for. If empty, will do all assets
	* @access public
	*/
	function updatePackageDetails($update_assets=Array())
	{
		// If there were failures in core packages, return false as dependency
		// resolution won't work (if a core package relies on another package,
		// what is the world coming to?)
		if ($this->checkPackageDetails($update_assets) !== true) {
			return false;
		}

		// open the transaction
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		//// Before we update any of the asset types we are going to make sure that
		//// the default search components are in place
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		// clear all weightings for this type_code
		$sql = 'DELETE FROM sq_sch_idx_wgt_dflt
				WHERE type_code = '.$db->quote('asset');

		$result = $db->query($sql);
		if (DB::isError($result)) {
			// Abort Transaction
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		foreach ($this->default_components as $component => $weighting) {
			$sql = 'INSERT INTO sq_sch_idx_wgt_dflt
					(type_code, component, description, weight)
					VALUES ('.$db->quote('asset').', '.$db->quote($component).',
					'.$db->quote($weighting['description']).','.$db->quote($weighting['default_weighting']).')';

			$result = $db->query($sql);
			if (DB::isError($result)) {
				// Abort Transaction
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				return false;
			}
		}// end foreach


		// Now we can do the normal package update
		if (!parent::updatePackageDetails($update_assets)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// Commit the transaction if we got this far
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end updatePackageDetails()


	//--        FUNCTIONS TO CREATE SYSTEM ASSETS        --//


	/**
	* Returns an array of all the asset types in the package that are to be treated as system assets
	* NOTE: Assets will be installed in the order they appear in this list
	*
	* @return Array()
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
				'cron_job_expired_workflow',
			   );

	}//end getSystemAssetTypes()


	/**
	* Create the root folder system asset
	*
	* @return object Root_Folder
	* @access public
	*/
	function &createRootFolder()
	{
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('root_folder');

		$root_folder = new Root_Folder();
		$link = Array();
		if (!$root_folder->create($link)) {
			trigger_error('Root Folder NOT CREATED', E_USER_ERROR);
		}

		pre_echo('Root Folder Asset Id : '.$root_folder->id);
		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('root_folder', $root_folder->id);

		return $root_folder;

	}//end createRootFolder()


	/**
	* Create the trash folder system asset
	*
	* @return object Trash_Folder
	* @access public
	*/
	function &createTrashFolder()
	{
		$root_folder = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('trash_folder');
		$trash_link = Array(
						'asset'			=> &$root_folder,
						'link_type'		=> SQ_LINK_TYPE_1,
						'is_exclusive'	=> 1,
						'sort_order'	=> 0,
					  );
		$trash_folder = new Trash_Folder();
		if (!$trash_folder->create($trash_link)) {
			trigger_error('Trash Folder NOT CREATED', E_USER_ERROR);
		}
		pre_echo('Trash Asset Id : '.$trash_folder->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('trash_folder', $trash_folder->id);
		return $trash_folder;

	}//end createTrashFolder()


	/**
	* Create the public access user system asset
	*
	* @return object Public_User
	* @access public
	*/
	function &createPublicUser()
	{
		$users_folder = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('web_folder_users');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('public_user');
		$public_user = new Public_User();
		$public_user_link = Array('asset' => &$users_folder, 'link_type' => SQ_LINK_TYPE_1);
		if (!$public_user->create($public_user_link)) {
			trigger_error('Public Access User NOT CREATED', E_USER_ERROR);
		}
		pre_echo('Public Access User Asset Id : '.$public_user->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('public_user', $public_user->id);

		// we will also turn all 'zero' permissions into the public user's while we're at it
		// THIS IS AN UPGRADE AND SHOULD BE REMOVED
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		
		$sql = 'UPDATE
					sq_ast_perm
				SET
					userid	= '.$db->quoteSmart($public_user->id).'
				WHERE
					userid	= '.$db->quoteSmart(0);

		$result = $db->query($sql);
		assert_valid_db_result($result);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return $public_user;

	}//end createPublicUser()


	/**
	* Create the system management folder system asset
	*
	* @return object System_Management_Folder
	* @access public
	*/
	function &createSystemManagementFolder()
	{
		$root_folder = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('system_management_folder');
		$system_management_folder = new System_Management_Folder();
		$system_management_folder_link = Array('asset' => &$root_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1, 'sort_order' => 0);
		if (!$system_management_folder->create($system_management_folder_link)) {
			trigger_error('System Management Folder NOT CREATED', E_USER_ERROR);
		}
		pre_echo('System Management Asset Id : '.$system_management_folder->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('system_management_folder', $system_management_folder->id);
		return $system_management_folder;

	}//end createSystemManagementFolder()


	/**
	* Create the system user group system asset
	*
	* @return object System_User_Group
	* @access public
	*/
	function &createSystemUserGroup()
	{
		$system_management_folder = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_management_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('system_user_group');
		$system_user_group = new System_User_Group();
		$system_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);
		if (!$system_user_group->create($system_link)) {
			trigger_error('System Admin Group NOT CREATED', E_USER_ERROR);
		}
		pre_echo('System Administrators User Group Asset Id : '.$system_user_group->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('system_user_group', $system_user_group->id);
		return $system_user_group;

	}//end createSystemUserGroup()


	/**
	* Create the root user system asset
	*
	* @return object Root_User
	* @access public
	*/
	function &createRootUser()
	{
		$system_user_group = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_user_group');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('root_user');
		$root_user = new Root_User();
		$user_link = Array('asset' => &$system_user_group, 'link_type' => SQ_LINK_TYPE_1);

		$root_pass = isset($_SESSION['install_root_pass']) ? $_SESSION['install_root_pass'] : 'root';

		$root_user->setAttrValue('password',   $root_pass);
		$root_user->setAttrValue('first_name', 'Root');
		$root_user->setAttrValue('last_name',  'User');
		$root_email = (SQ_CONF_DEFAULT_EMAIL) ? SQ_CONF_DEFAULT_EMAIL : ('root@'.((SQ_PHP_CLI) ? $_SERVER['HOSTNAME'] : $_SERVER['HTTP_HOST']));
		$root_user->setAttrValue('email', $root_email);

		if (!$root_user->create($user_link)) {
			trigger_error('Root User NOT CREATED', E_USER_ERROR);
		}

		pre_echo('Root User Asset Id : '.$root_user->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('root_user', $root_user->id);
		return $root_user;

	}//end createRootUser()


	/**
	* Create the cron manager system asset
	*
	* @return object Cron_Manager
	* @access public
	*/
	function &createCronManager()
	{
		$system_management_folder = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_management_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_manager');
		$cron_manager = new Cron_Manager();
		$cron_manager_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);
		if (!$cron_manager->create($cron_manager_link)) {
			trigger_error('Cron Manager NOT CREATED', E_USER_ERROR);
		}
		pre_echo('Cron Manager Asset Id : '.$cron_manager->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('cron_manager', $cron_manager->id);
		return $cron_manager;

	}//end createCronManager()


	/**
	* Create the trigger manager system asset
	*
	* @return object Trigger_Manager
	* @access public
	*/
	function &createTriggerManager()
	{
		$system_management_folder = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_management_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('trigger_manager');
		$trigger_manager = new Trigger_Manager();
		$trigger_manager_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);
		if (!$trigger_manager->create($trigger_manager_link)) {
			trigger_error('Trigger Manager NOT CREATED', E_USER_ERROR);
		}
		pre_echo('Trigger Manager Asset Id : '.$trigger_manager->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('trigger_manager', $trigger_manager->id);
		return $trigger_manager;

	}//end createTriggerManager()


	/**
	* Create the log manager system asset
	*
	* @return object Log_Manager
	* @access public
	*/
	function &createLogManager()
	{
		$system_management_folder = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_management_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('log_manager');
		$log_manager = new Log_Manager();
		$log_manager_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);
		if (!$log_manager->create($log_manager_link)) {
			trigger_error('Trigger Manager NOT CREATED', E_USER_ERROR);
		}
		pre_echo('Log Manager Asset Id : '.$log_manager->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('log_manager', $log_manager->id);
		return $log_manager;

	}//end createLogManager()


	/**
	* Create the layout manager system asset
	*
	* @return object Layout_Manager
	* @access public
	*/
	function &createLayoutManager()
	{
		$system_management_folder = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_management_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('layout_manager');
		$layout_manager = new Layout_Manager();
		$layout_manager_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);
		if (!$layout_manager->create($layout_manager_link)) {
			trigger_error('Layout Manager NOT CREATED', E_USER_ERROR);
		}
		pre_echo('Layout Manager Asset Id : '.$layout_manager->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('layout_manager', $layout_manager->id);
		return $layout_manager;

	}//end createLayoutManager()


	/**
	* Create the remap manager system asset
	*
	* @return object Remap_Manager
	* @access public
	*/
	function &createRemapManager()
	{
		$system_management_folder = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_management_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('remap_manager');
		$remap_manager = new Remap_Manager();
		$remap_manager_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);
		if (!$remap_manager->create($remap_manager_link)) trigger_error('Remap Manager NOT CREATED', E_USER_ERROR);
		pre_echo('Remap Manager Asset Id : '.$remap_manager->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('remap_manager',$remap_manager->id);
		return $remap_manager;

	}//end createRemapManager()


	/**
	* Create the cache manager system asset
	*
	* @return object Cache_Manager
	* @access public
	*/
	function &createCacheManager()
	{
		$system_management_folder = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_management_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cache_manager');
		$cache_manager = new Cache_Manager();
		$cache_manager_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);
		if (!$cache_manager->create($cache_manager_link)) {
			trigger_error('Cache Manager NOT CREATED', E_USER_ERROR);
		}
		pre_echo('Cache Manager Asset Id : '.$cache_manager->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('cache_manager', $cache_manager->id);
		return $cache_manager;

	}//end createCacheManager()


	/**
	* Create the designs folder system asset
	*
	* @return object Designs_Folder
	* @access public
	*/
	function &createDesignsFolder()
	{
		$system_management_folder = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_management_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('designs_folder');
		$designs_folder = new Designs_Folder();
		$designs_folder_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);
		if (!$designs_folder->create($designs_folder_link)) {
			trigger_error('Designs Folder NOT CREATED', E_USER_ERROR);
		}
		pre_echo('Design Folder Asset Id : '.$designs_folder->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('designs_folder', $designs_folder->id);
		return $designs_folder;

	}//end createDesignsFolder()


	/**
	* Create the authentication folder system asset
	*
	* @return object Authentication_Folder
	* @access public
	*/
	function &createAuthenticationFolder()
	{
		$system_management_folder = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_management_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('authentication_folder');
		$authentication_folder = new Authentication_Folder();
		$authentication_folder_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);
		if (!$authentication_folder->create($authentication_folder_link)) {
			trigger_error('Authentication Folder NOT CREATED', E_USER_ERROR);
		}
		pre_echo('Authentication Folder Asset Id : '.$authentication_folder->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('authentication_folder', $authentication_folder->id);
		return $authentication_folder;

	}//end createAuthenticationFolder()


	/**
	* Create the login design system asset
	*
	* @return object Login_Design
	* @access public
	*/
	function &createLoginDesign()
	{
		$designs_folder = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('designs_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('login_design');
		$login_design = new Login_Design();
		$login_design_link = Array('asset' => &$designs_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);
		$login_design->setAttrValue('id_name', 'login_design');
		if (!$login_design->create($login_design_link)) {
			trigger_error('Login Design NOT CREATED', E_USER_ERROR);
		}
		pre_echo('Login Design Asset Id : '.$login_design->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('login_design', $login_design->id);
		return $login_design;

	}//end createLoginDesign()


	/**
	* Create the web folder for storing site designs
	*
	* @return object Web_Folder_Designs
	* @access public
	*/
	function &createWebFolderDesigns()
	{
		$root_folder = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('web_folder_designs');
		$folder_link = Array('asset' => &$root_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);
		$designs_folder = new Web_Folder_Designs();
		$designs_folder->setAttrValue('name', 'Designs Folder');
		if (!$designs_folder->create($folder_link)) {
			trigger_error('Designs Folder NOT CREATED', E_USER_ERROR);
		}
		pre_echo('Designs Folder Id : '.$designs_folder->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('web_folder_designs', $designs_folder->id);
		return $designs_folder;

	}//end createWebFolderDesigns()


	/**
	* Create the web folder for storing media (images/movies etc)
	*
	* @return object Web_Folder_Media
	* @access public
	*/
	function &createWebFolderMedia()
	{
		$root_folder = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('web_folder_media');
		$folder_link = Array('asset' => &$root_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);
		$media_folder = new Web_Folder_Media();
		$media_folder->setAttrValue('name', 'Media Folder');
		if (!$media_folder->create($folder_link)) {
			trigger_error('Media Folder NOT CREATED', E_USER_ERROR);
		}
		pre_echo('Media Folder Id : '.$media_folder->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('web_folder_media', $media_folder->id);
		return $media_folder;

	}//end createWebFolderMedia()


	/**
	* Create the web folder for storing users
	*
	* @return object Web_Folder_Users
	* @access public
	*/
	function &createWebFolderUsers()
	{
		$root_folder = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('web_folder_users');
		$folder_link = Array('asset' => &$root_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);
		$users_folder = new Web_Folder_Users();
		$users_folder->setAttrValue('name', 'Users Folder');
		if (!$users_folder->create($folder_link)) {
			trigger_error('Users Folder NOT CREATED', E_USER_ERROR);
		}
		pre_echo('Users Folder Id : '.$users_folder->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('web_folder_users', $users_folder->id);
		return $users_folder;

	}//end createWebFolderUsers()


	/**
	* Create the expired workflow cron job
	*
	* @return object Cron_Job_Expired_Workflow
	* @access public
	*/
	function &createCronJobExpiredWorkflow()
	{
		$cron_manager =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (is_null($cron_manager)) {
			trigger_error('Couldn\'t get cron manager, couldn\'t create expired workflow cron job', E_USER_WARNING);
			$null = null;
			return $null;
		}

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_expired_workflow');

		$cron_job = new Cron_Job_Expired_WorkFlow();
		$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
		if (!$cron_manager->addJob($cron_job, $root_user, SQ_LINK_TYPE_2)) {
			trigger_error('Expired Workflow Cron Job NOT CREATED', E_USER_ERROR);
		}
		pre_echo('Expired Workflow Cron Job Id: '.$cron_job->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('cron_job_expired_workflow', $cron_job->id);

		return $cron_job;

	}//end createCronJobExpiredWorkflow()


}//end class

?>
