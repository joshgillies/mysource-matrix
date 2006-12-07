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
* $Id: package_manager_ecommerce.inc,v 1.7 2006/12/07 05:26:27 bcaldwell Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/package_manager.inc';

/**
* Package_Manager_Ecommerce
*
* Purpose
*    Manages the install and upgrade of the Ecommerce package assets,
*    uses info gleened from the package.xml and asset.xml files
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Package_Manager_Ecommerce extends Package_Manager
{


	/**
	* Constructor
	*
	*/
	function Package_Manager_Ecommerce()
	{
		$this->_full_path = SQ_PACKAGES_PATH.'/ecommerce';
		$this->Package_Manager();

	}//end constructor


}//end class

?>