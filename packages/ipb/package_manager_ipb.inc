<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: package_manager_ipb.inc,v 1.4 2012/08/30 00:58:55 ewang Exp $
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
* @version $Revision: 1.4 $
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
