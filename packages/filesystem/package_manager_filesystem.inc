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
* $Id: package_manager_filesystem.inc,v 1.2 2012/08/30 00:58:36 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/package_manager.inc';

/**
* Package_Manager_Filesystem
*
* Purpose
*    Manages the install and upgrade of the Filesystem package and Filesystem assets,
*    uses info gleaned from the package.xml and asset.xml files
*
* @author  Benjamin Pearson <bpearson@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage filesystem
*/
class Package_Manager_Filesystem extends Package_Manager
{

	/**
	* Constructor
	*
	*/
	function Package_Manager_Filesystem()
	{
		$this->_full_path = SQ_PACKAGES_PATH.'/filesystem';
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
						 );
		$installed_assets = Array();

		foreach ($system_assets as $type_code) {
			if ($GLOBALS['SQ_SYSTEM']->am->installed($type_code)) {
				$installed_assets[] = $type_code;
			}
		}

		return $installed_assets;

	}//end getSystemAssetTypes()


}//end class

?>
