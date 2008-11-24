<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: package_manager_calendar.inc,v 1.7 2008/11/24 23:17:12 mbrydon Exp $
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
* @version $Revision: 1.7 $ - 1.0
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