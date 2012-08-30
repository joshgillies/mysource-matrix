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
* $Id: package_manager_squid.inc,v 1.4 2012/08/30 00:59:22 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/package_manager.inc';

/**
* Package_Manager_Squid
*
* Purpose
*    Manages the install and upgrade of the Squid package and Squid assets,
*    uses info gleened from the package.xml and asset.xml files
*
* @author  Blair Robertson <brobertson@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Package_Manager_Squid extends Package_Manager
{

	/**
	* Constructor
	*
	*/
	function Package_Manager_Squid()
	{
		$this->_full_path = SQ_PACKAGES_PATH.'/squid';
		$this->Package_Manager();

	}//end constructor


}//end class

?>