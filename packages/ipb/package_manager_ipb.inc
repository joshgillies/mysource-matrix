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
* $Id: package_manager_ipb.inc,v 1.2 2006/12/07 03:08:30 bcaldwell Exp $
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
* @version $Revision: 1.2 $
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
