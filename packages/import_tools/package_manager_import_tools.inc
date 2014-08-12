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
* $Id: package_manager_import_tools.inc,v 1.6 2012/08/30 00:58:50 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/package_manager.inc';

/**
* Package_Manager_Import_Tools
*
* Purpose
*    Manages the install and upgrade of the Import Tools system assets
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.6 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage Fedcourt
*/
class Package_Manager_Import_Tools extends Package_Manager
{


	/**
	* Constructor
	*
	*/
	function Package_Manager_Import_Tools()
	{
		$this->_full_path = SQ_PACKAGES_PATH.'/import_tools';
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
		return Array('import_tools_manager');

	}//end getSystemAssetTypes()


	/**
	* Create the Import Tools Manager System Asset
	*
	* @return object
	* @access public
	*/
	function &createImportToolsManager()
	{
		$system_management_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_management_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('import_tools_manager');
		$import_tools_manager = new Import_Tools_Manager();
		$import_tools_manager_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);
		if (!$import_tools_manager->create($import_tools_manager_link)) {
			trigger_localised_error('IMP0001', E_USER_ERROR);
		}

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('import_tools_manager', $import_tools_manager->id);

		return $import_tools_manager;

	}//end createImportToolsManager()


}//end class

?>
