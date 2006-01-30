<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: package_manager_search.inc,v 1.15 2006/01/30 05:57:21 sdanis Exp $
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
* @version $Revision: 1.15 $
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