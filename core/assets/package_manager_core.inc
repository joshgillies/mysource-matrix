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
* $Id: package_manager_core.inc,v 1.6 2004/07/01 14:36:51 brobertson Exp $
* $Name: not supported by cvs2svn $
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
* @version $Version$ - 1.0
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
									'__name__' => Array (
										'description'		=> 'Asset Name',
										'default_weighting'	=> '-1',
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
	* @access public
	*/
	function updatePackageDetails()
	{
		if (!$this->checkPackageDetails()) {
			return false;
		}

		// open the transaction
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		//// Before we update any of the asset types we are going to make sure that 
		//// the default search components are in place
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		// clear all weightings for this type_code
		$sql = 'DELETE FROM '. SQ_TABLE_PREFIX .'asset_indexing_weightings_default
				WHERE type_code = '.$db->quote('asset');

		$result = $db->query($sql);
		if (DB::isError($result)) {
			// Abort Transaction
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		foreach ($this->default_components as $component => $weighting) {
			$sql = 'INSERT INTO '. SQ_TABLE_PREFIX .'asset_indexing_weightings_default
					(type_code, component, description, weighting)
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
		if (!parent::updatePackageDetails()) {
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
					'trash_folder',
					'system_management_folder',
					'system_user_group',
					'root_user',
					'authentication_folder',
					'cron_manager',
					'layout_manager',
					'remap_manager',
					'designs_folder',
					'login_design',
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
		if (!$root_folder->create($link)) trigger_error('ROOT FOLDER NOT CREATED', E_USER_ERROR);
		pre_echo('Root Folder Asset Id : '.$root_folder->id);
		
		$GLOBALS['SQ_SYSTEM_ASSETS']['root_folder'] = $root_folder->id;
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
		$root_folder = &$GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM_ASSETS']['root_folder']);
		
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('trash_folder');
		$trash_link = Array('asset' => &$root_folder, 'link_type' => SQ_LINK_TYPE_1, 'exclusive' => 1);
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('trash_folder');
		$trash_folder = new Trash_Folder();
		if (!$trash_folder->create($trash_link)) trigger_error('TRASH FOLDER NOT CREATED', E_USER_ERROR);
		pre_echo('Trash Asset Id : '.$trash_folder->id);
		
		$GLOBALS['SQ_SYSTEM_ASSETS']['trash_folder'] = $trash_folder->id;
		return $trash_folder;

	}//end createTrashFolder()


	/**
	* Create the system management folder system asset
	*
	* @return object System_Management_Folder
	* @access public
	*/
	function &createSystemManagementFolder()
	{
		$root_folder = &$GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM_ASSETS']['root_folder']);
		
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('system_management_folder');
		$system_management_folder = new System_Management_Folder();
		$system_management_folder_link = Array('asset' => &$root_folder, 'link_type' => SQ_LINK_TYPE_1, 'exclusive' => 1);
		if (!$system_management_folder->create($system_management_folder_link)) trigger_error('SYSTEM MANAGEMENT FOLDER NOT CREATED', E_USER_ERROR);
		pre_echo('System Management Asset Id : '.$system_management_folder->id);
		
		$GLOBALS['SQ_SYSTEM_ASSETS']['system_management_folder'] = $system_management_folder->id;
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
		$system_management_folder = &$GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM_ASSETS']['system_management_folder']);
		
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('system_user_group');
		$system_user_group = new System_User_Group();
		$system_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'exclusive' => 1);
		if (!$system_user_group->create($system_link)) trigger_error('SYSTEM ADMIN GROUP NOT CREATED', E_USER_ERROR);
		pre_echo('System Administrators User Group Asset Id : '.$system_user_group->id);
		
		$GLOBALS['SQ_SYSTEM_ASSETS']['system_user_group'] = $system_user_group->id;
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
		$system_user_group = &$GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM_ASSETS']['system_user_group']);
		
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('root_user');
		$root_user = new Root_User();
		$user_link = Array('asset' => &$system_user_group, 'link_type' => SQ_LINK_TYPE_1);

		$root_pass = isset($_SESSION['install_root_pass']) ? $_SESSION['install_root_pass'] : 'root';
		
		$root_user->setAttrValue('password',   $root_pass);
		$root_user->setAttrValue('first_name', 'Root');
		$root_user->setAttrValue('last_name',  'User');
		$root_email = (SQ_CONF_DEFAULT_EMAIL) ? SQ_CONF_DEFAULT_EMAIL : ('root@'.((SQ_PHP_CLI) ? $_SERVER['HOSTNAME'] : $_SERVER['HTTP_HOST']));
		$root_user->setAttrValue('email', $root_email);

		if (!$root_user->create($user_link)) trigger_error('ROOT USER NOT CREATED', E_USER_ERROR);
		pre_echo('Root User Asset Id : '.$root_user->id);
		
		$GLOBALS['SQ_SYSTEM_ASSETS']['root_user'] = $root_user->id;
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
		$system_management_folder = &$GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM_ASSETS']['system_management_folder']);
		
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_manager');
		$cron_manager = new Cron_Manager();
		$cron_manager_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'exclusive' => 1);
		if (!$cron_manager->create($cron_manager_link)) trigger_error('Cron Manager NOT CREATED', E_USER_ERROR);
		pre_echo('Cron Manager Asset Id : '.$cron_manager->id);
		
		$GLOBALS['SQ_SYSTEM_ASSETS']['cron_manager'] = $cron_manager->id;
		return $cron_manager;

	}//end createCronManager()


	/**
	* Create the layout manager system asset
	*
	* @return object Layout_Manager
	* @access public
	*/
	function &createLayoutManager()
	{
		$system_management_folder = &$GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM_ASSETS']['system_management_folder']);
		
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('layout_manager');
		$layout_manager = new Layout_Manager();
		$layout_manager_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'exclusive' => 1);
		if (!$layout_manager->create($layout_manager_link)) trigger_error('Layout Manager NOT CREATED', E_USER_ERROR);
		pre_echo('Layout Manager Asset Id : '.$layout_manager->id);
		
		$GLOBALS['SQ_SYSTEM_ASSETS']['layout_manager'] = $layout_manager->id;
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
		$system_management_folder = &$GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM_ASSETS']['system_management_folder']);
		
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('remap_manager');
		$remap_manager = new Remap_Manager();
		$remap_manager_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'exclusive' => 1);
		if (!$remap_manager->create($remap_manager_link)) trigger_error('Remap Manager NOT CREATED', E_USER_ERROR);
		pre_echo('Remap Manager Asset Id : '.$remap_manager->id);
		
		$GLOBALS['SQ_SYSTEM_ASSETS']['remap_manager'] = $remap_manager->id;
		return $remap_manager;

	}//end createRemapManager()


	/**
	* Create the designs folder system asset
	*
	* @return object Designs_Folder
	* @access public
	*/
	function &createDesignsFolder()
	{
		$system_management_folder = &$GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM_ASSETS']['system_management_folder']);
		
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('designs_folder');
		$designs_folder = new Designs_Folder();
		$designs_folder_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'exclusive' => 1);
		if (!$designs_folder->create($designs_folder_link)) trigger_error('Designs Folder NOT CREATED', E_USER_ERROR);
		pre_echo('Design Folder Asset Id : '.$designs_folder->id);
		
		$GLOBALS['SQ_SYSTEM_ASSETS']['designs_folder'] = $designs_folder->id;
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
		$system_management_folder = &$GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM_ASSETS']['system_management_folder']);
		
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('authentication_folder');
		$authentication_folder = new Authentication_Folder();
		$authentication_folder_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'exclusive' => 1);
		if (!$authentication_folder->create($authentication_folder_link)) trigger_error('Authentication Folder NOT CREATED', E_USER_ERROR);
		pre_echo('Authentication Folder Asset Id : '.$authentication_folder->id);
		
		$GLOBALS['SQ_SYSTEM_ASSETS']['authentication_folder'] = $authentication_folder->id;
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
		$designs_folder = &$GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM_ASSETS']['designs_folder']);
	
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('login_design');
		$login_design = new Login_Design();
		$login_design_link = Array('asset' => &$designs_folder, 'link_type' => SQ_LINK_TYPE_1, 'exclusive' => 1);
		$login_design->setAttrValue('id_name', 'login_design');
		if (!$login_design->create($login_design_link)) trigger_error('Login Design NOT CREATED', E_USER_ERROR);
		pre_echo('Login Design Asset Id : '.$login_design->id);
	
		$GLOBALS['SQ_SYSTEM_ASSETS']['login_design'] = $login_design->id;
		return $login_design;

	}//end createLoginDesign()


}//end class

?>