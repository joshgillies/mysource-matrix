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
* $Id: package_manager_data.inc,v 1.5 2012/08/30 00:58:15 ewang Exp $
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
* @version $Revision: 1.5 $
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
