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
* $Id: package_manager_search.inc,v 1.5 2004/06/21 22:50:02 mnyeholt Exp $
* $Name: not supported by cvs2svn $
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
* @version $Version$ - 1.0
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
		$this->_full_path = dirname(__FILE__);
		$this->Package_Manager();

	}//end constructor


	/**
	* Create the search manager system asset
	*
	* @return object Search_Manager
	* @access public
	*/
	function &createSearchManager()
	{
		$system_management_folder = &$GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM_ASSETS']['system_management_folder']);
	
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('search_manager');
		$search_manager = new Search_Manager();
		$search_manager_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'exclusive' => 1);
		if (!$search_manager->create($search_manager_link)) trigger_error('Search Manager NOT CREATED', E_USER_ERROR);
		pre_echo('Search Manager Asset Id : '.$search_manager->id);
	
		// MARCUS - No longer need to trigger this error...
		// trigger_error('You will have to run step_03.php again to set up the index weightings for the first time', E_USER_NOTICE);

		$GLOBALS['SQ_SYSTEM_ASSETS']['search_manager'] = $search_manager->id;

		return $search_manager;

	}//end createSearchManager()


}//end class

?>