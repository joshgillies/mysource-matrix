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
* $Id: package_manager_web_services.inc,v 1.2 2006/12/07 03:50:19 bcaldwell Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/package_manager.inc';

/**
* Package_Manager_Web_Services
*
* Purpose
*    Manages the install and upgrade of the Web Services package and Web Service assets,
*    uses info gleened from the package.xml and asset.xml files
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class Package_Manager_Web_Services extends Package_Manager
{


	/**
	* Constructor
	*
	*/
	function Package_Manager_Web_Services()
	{
		$this->_full_path = SQ_PACKAGES_PATH.'/web_services';
		$this->Package_Manager();

	}//end constructor


}//end class

?>
