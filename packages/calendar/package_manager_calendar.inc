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
* $Id: package_manager_calendar.inc,v 1.6 2006/12/07 05:51:49 bcaldwell Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/package_manager.inc';

/**
* Package_Manager_Calendar
*
* Purpose
*    Manages the install and upgrade of the Calendar system assets,
*    uses info gleaned from the package.xml and asset.xml files
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.6 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Package_Manager_Calendar extends Package_Manager
{


	/**
	* Constructor
	*
	*/
	function Package_Manager_Calendar()
	{
		$this->_full_path = SQ_PACKAGES_PATH.'/calendar';
		$this->Package_Manager();

	}//end constructor


}//end class

?>