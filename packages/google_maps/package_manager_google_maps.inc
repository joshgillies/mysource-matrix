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
* $Id: package_manager_google_maps.inc,v 1.1.1.1 2008/07/04 05:19:54 hnguyen Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/package_manager.inc';

/**
* Package_Manager_CMS
*
* Purpose
*    Manages the install and upgrade of the CMS package and CMS assets,
*    uses info gleened from the package.xml and asset.xml files
*
* @author  Blair Robertson <brobertson@squiz.net>
* @version $Revision: 1.1.1.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Package_Manager_Google_Maps extends Package_Manager
{

	/**
	* Constructor
	*
	*/
	function Package_Manager_Google_Maps()
	{
		$this->_full_path = SQ_PACKAGES_PATH.'/google_maps';
		$this->Package_Manager();

	}//end constructor


}//end class

?>
