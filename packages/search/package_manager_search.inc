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
* $Id: package_manager_search.inc,v 1.16 2006/12/07 01:10:13 bcaldwell Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/package_manager.inc';

/**
* Search_Manager
*
* Purpose
*    Manages the install and upgrade of the Search package and Search assets,
*    uses info gleened from the package.xml and asset.xml files
*
* @author  Blair Robertson <brobertson@squiz.net>
* @version $Revision: 1.16 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Package_Manager_Search extends Package_Manager
{


	/**
	* Constructor
	*
	*/
	function Package_Manager_Search()
	{
		$this->_full_path = SQ_PACKAGES_PATH.'/search';
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
		return Array('search_manager');

	}//end getSystemAssetTypes()


	/**
	* Create the search manager system asset
	*
	* @return object
	* @access public
	*/
	function &createSearchManager()
	{
		$system_management_folder =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_management_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('search_manager');
		$search_manager =& new Search_Manager();
		$search_manager_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);
		if (!$search_manager->create($search_manager_link)) {
			trigger_localised_error('SCH0007', E_USER_ERROR);
		}
		pre_echo('Search Manager Asset Id : '.$search_manager->id);

		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('search_manager', $search_manager->id);

		return $search_manager;

	}//end createSearchManager()


}//end class

?>