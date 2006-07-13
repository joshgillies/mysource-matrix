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
* $Id: package_manager_web_services.inc,v 1.1.1.1 2006/07/13 04:25:40 bcaldwell Exp $
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
* @version $Revision: 1.1.1.1 $
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
