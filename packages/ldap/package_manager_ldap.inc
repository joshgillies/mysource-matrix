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
* $Id: package_manager_ldap.inc,v 1.7 2005/08/23 16:50:34 brobertson Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/package_manager.inc';

/**
* Package_Manager_LDAP
*
* Purpose
*    Manages the install and upgrade of the LDAP package and LDAP assets,
*    uses info gleened from the package.xml and asset.xml files
*
* @author  Blair Robertson <brobertson@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage ldap
*/
class Package_Manager_LDAP extends Package_Manager
{


	/**
	* Constructor
	*
	*/
	function Package_Manager_LDAP()
	{
		$this->_full_path = SQ_PACKAGES_PATH.'/ldap';
		$this->Package_Manager();

	}//end constructor


	/**
	* Checks to see if the installation/upgrade of this package is able to go ahead
	*
	* Will return 'true' if all assets are okay, otherwise will return an array
	* of assetids that failed requirements.
	*
	* @param array	$update_assets	an array of asset types to check details for
	*								if empty, will check all assets
	* @return mixed
	* @access public
	*/
	function checkPackageDetails($update_assets=Array())
	{

		// include asset to get locales (if they have been generated yet)
		$GLOBALS['SQ_SYSTEM']->lm->includePackageStrings($this->getPackageName());

		if (!function_exists('ldap_connect')) {
			// just a friendly pointer
			trigger_localised_error('LDAP0002', E_USER_WARNING);
		}

		return parent::checkPackageDetails($update_assets);

	}//end checkPackageDetails()


}//end class

?>