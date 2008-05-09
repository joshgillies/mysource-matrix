<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: package_manager_web_services.inc,v 1.4 2008/05/09 04:54:02 hnguyen Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/package_manager.inc';

/**
* Package_Manager_Web_Services
*
* Purpose
*    Manages the install and upgrade of the Web services package and web services assets,
*    uses info gleened from the package.xml and asset.xml files
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @author  Basil Shkara <bshkara@squiz.net>
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class Package_Manager_Web_Services extends Package_Manager
{


	/**
	* Constructor
	*
	*/
	function Package_Manager_Web_Services()
	{
		$this->_full_path = SQ_PACKAGES_PATH.'/web_services';
		$this->Package_Manager();

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
				'web_folder_web_services',
			   );

	}//end getSystemAssetTypes()


	/**
	* Create the web folder for storing site web services
	*
	* @return object
	* @access public
	*/
	function &createWebFolderWebServices()
	{
		$root_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('web_folder_web_services');
		$folder_link = Array('asset' => &$root_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);
		$web_services_folder = new Web_Folder_Web_Services();
		$web_services_folder->setAttrValue('name', 'Web Services Folder');
		if (!$web_services_folder->create($folder_link)) {
			trigger_localised_error('CORE0001', E_USER_ERROR, 'Web Services Folder');
		}
		pre_echo('Web Services Folder Id : '.$web_services_folder->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('web_folder_web_services', $web_services_folder->id);

		$design_parse_file = $web_services_folder->getDesignParseFile();
		$design_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('web_folder_designs');
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('design');

		$link_array = Array (
						'asset'			=> $design_folder,
						'value'			=> '',
						'link_type'		=> SQ_LINK_TYPE_1,
						'sort_order'	=> '',
						'is_dependant'	=> 0,
						'is_exclusive'	=> 0,
					  );
		$design = new Design();
		$design->setAttrValue('id_name', 'Soap Server Design');
		$design->create($link_array);

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		create_directory($design->data_path);
		string_to_file($design_parse_file, $design->data_path.'/parse.txt');

        $design_edit_fns = $design->getEditFns();
        if (!$design_edit_fns->parseAndProcessFile($design)) {
            trigger_error('Failed parsing updated parse file');
        }//end if
		$design->generateDesignFile();

		$GLOBALS['SQ_SYSTEM']->am->createAssetLink($web_services_folder, $design, SQ_LINK_NOTICE, 'design::system::frontend');
		$web_services_folder->updateLookups();

		return $web_services_folder;

	}//end createWebFolderWebServices()


}//end class

?>
