<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: package_manager_squiz_suite.inc,v 1.1.1.1 2010/09/29 01:11:55 bpearson Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/package_manager.inc';

/**
* Squiz Suite Package 
*
* Purpose
*    Package containing assets to talk to Squiz Suite products 
*    uses info gleened from the package.xml and asset.xml files
*
* @author  Benjamin Pearson <bpearson@squiz.com.au>
* @version $Revision: 1.1.1.1 $
* @package MySource_Matrix_Packages
* @subpackage squiz_suite
*/
class Package_Manager_Squiz_Suite extends Package_Manager
{


	/**
	* Constructor
	*
	*/
	function Package_Manager_Squiz_Suite()
	{
		$this->_full_path = SQ_PACKAGES_PATH.'/squiz_suite';
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
		$system_assets = Array(
							'suite_manager',
						 );
		$installed_assets = Array();

		foreach ($system_assets as $type_code) {
			if ($GLOBALS['SQ_SYSTEM']->am->installed($type_code)) {
				$installed_assets[] = $type_code;
			}
		}

		return $installed_assets;

	}//end getSystemAssetTypes()


	/**
	* Create the suite manager system asset
	*
	* @return object
	* @access public
	*/
	function &createSuiteManager()
	{
		$web_services_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('web_folder_web_services');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('suite_manager');
		$suite_manager = new Suite_Manager();
        $suite_manager->setAttrValue('name', 'Squiz Suite Manager');
		$suite_manager_link = Array('asset' => &$web_services_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);
		if (!$suite_manager->create($suite_manager_link)) {
			trigger_localised_error('SQS0001', E_USER_ERROR);
		}
		pre_echo('Squiz Suite Manager Asset Id : '.$suite_manager->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('suite_manager', $suite_manager->id);

        $web_path = $suite_manager->getURL();
        if (!empty($web_path)) {
            $connection = array('url' => $web_path, 'name'=>'Squiz Matrix');
            $systemid   = $suite_manager->registerProduct('matrix', 'Squiz Matrix', $connection);

            // Set it as the current system.
            $suite_manager->updateProduct($systemid, 'current', TRUE);

            // Generate private/public keys for the secure communication.
            $suite_manager->generateSystemKeyPair();
        }//end if

		return $suite_manager;

	}//end createSuiteManager()


}//end class

?>
