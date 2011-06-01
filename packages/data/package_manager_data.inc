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
* $Id: package_manager_data.inc,v 1.3 2006/12/08 00:51:59 emcdonald Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/package_manager.inc';

/**
* Package_Manager_Data
*
* Purpose
*    Manages the install and upgrade of the Data package and Data assets,
*    uses info gleened from the package.xml and asset.xml files
*
* @author  Blair Robertson <brobertson@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage data
*/
class Package_Manager_Data extends Package_Manager
{


	/**
	* Constructor
	*
	*/
	function Package_Manager_Data()
	{
		$this->_full_path = SQ_PACKAGES_PATH.'/data';
		$this->Package_Manager();

	}//end constructor


}//end class

?>
