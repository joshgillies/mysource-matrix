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
* $Id: package_manager_bulkmail.inc,v 1.7 2006/01/05 22:39:04 rong Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/package_manager.inc';

/**
* Package_Manager_Bulkmail
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Rayne Ong <rong@squiz.net>
* @version $Version$
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class Package_Manager_Bulkmail extends Package_Manager
{


	/**
	* Constructor
	*
	*/
	function Package_Manager_Bulkmail()
	{
		$this->_full_path = SQ_PACKAGES_PATH.'/bulkmail';
		$this->Package_Manager();

	}//end constructor


	/**
	* Returns an array of all the asset types in the package that are to be treated as system assets
	* NOTE: Assets will be installed in the order they appear in this list
	*
	* @return array
	* @access public
	*/
	function getSystemAssetTypes()
	{
		return Array(
				'bulkmail_manager',
			   );

	}//end getSystemAssetTypes()


	/**
	* Create the bulkmail manager system asset
	*
	* @return object
	* @access public
	*/
	function &createBulkmailManager()
	{
		// set default asset name in management.inc and define _getName() function
		$system_management_folder =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_management_folder');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bulkmail_manager');
		$bulkmail_manager =& new Bulkmail_Manager();
		$bulkmail_manager_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);
		if (!$bulkmail_manager->create($bulkmail_manager_link)) {
			trigger_localised_error('SCH0007', E_USER_ERROR);
		}
		pre_echo('Bulkmail Manager Asset Id : '.$bulkmail_manager->id);
		$GLOBALS['SQ_SYSTEM']->am->registerSystemAsset('bulkmail_manager', $bulkmail_manager->id);

		return $bulkmail_manager;

	}//end createBulkmailManager()


}//end class

?>
