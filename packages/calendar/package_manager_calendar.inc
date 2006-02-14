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
* $Id: package_manager_calendar.inc,v 1.5 2006/02/14 23:52:47 sdanis Exp $
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
* @version $Revision: 1.5 $ - 1.0
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