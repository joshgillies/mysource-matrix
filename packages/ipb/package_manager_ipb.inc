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
* $Id: package_manager_ipb.inc,v 1.1.1.1 2005/12/15 03:02:07 skim Exp $
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
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.1.1.1 $
* @package MySource_Matrix_Packages
* @subpackage ldap
*/
class Package_Manager_IPB extends Package_Manager
{


	/**
	* Constructor
	*
	*/
	function Package_Manager_IPB()
	{
		$this->_full_path = SQ_PACKAGES_PATH.'/ipb';
		$this->Package_Manager();

	}//end constructor


}//end class

?>
