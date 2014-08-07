<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd   |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: package_manager_web_services.inc,v 1.10 2012/08/30 00:59:45 ewang Exp $
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
* @version $Revision: 1.10 $
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
				'rest_manager',
				'cron_job_remove_soap_response_files',
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
		$folder_link = Array(
				'asset' 		=> &$root_folder, 
				'link_type' 	=> SQ_LINK_TYPE_1, 
				'is_exclusive' 	=> 1,
				'is_locked'		=> 1,
			);
		$web_services_folder = new Web_Folder_Web_Services();
		$web_services_folder->setAttrValue('name', 'Web Services Folder');
		if (!$web_services_folder->create($folder_link)) {
			trigger_localised_error('CORE0001', E_USER_ERROR, 'Web Services Folder');
		}

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
						'is_locked'		=> 1,
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


	/**
     * Create the Rest Manager system asset.
     *
     * @return object
     * @access public
     */
    function &createRestManager()
    {
        $system_management_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_management_folder');

        $GLOBALS['SQ_SYSTEM']->am->includeAsset('rest_manager');
        $rest_manager = new Rest_Manager();
        $rest_manager_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);
        if (!$rest_manager->create($rest_manager_link)) {
            trigger_localised_error('CORE0001', E_USER_ERROR, 'REST Manager');
        }

        $GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('rest_manager', $rest_manager->id);
        return $rest_manager;

    }//end createRestManager()


	/**
	* Create the funnelback log rotation cron job
	*
	* @return object
	* @access public
	*/
	function &createCronJobremoveSoapResponseFiles()
	{
		$cron_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (is_null($cron_manager)) {
			trigger_error('Couldn\'t get cron manager, couldn\'t create Remove SOAP Response Files cron job', E_USER_WARNING);
			$null = NULL;
			return $null;
		}

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_remove_soap_response_files');

		$cron_job = new Cron_Job_Remove_SOAP_Response_Files();
		$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
		if (!$cron_manager->addJob($cron_job, $root_user, SQ_LINK_TYPE_2)) {
			trigger_error('Remove SOAP Response Files Cron Job NOT CREATED', E_USER_ERROR);
		}

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('cron_job_remove_soap_response_files', $cron_job->id);

		return $cron_job;

	}//end createCronJobFunnelbackLogRotation()


}//end class

?>
