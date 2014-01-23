<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd   |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
*
*/

require_once SQ_INCLUDE_PATH.'/package_manager.inc';

/**
* Package_Manager_Marketo
*
* Purpose
*    Manages the install and upgrade of the Web services package and web services assets,
*    uses info gleened from the package.xml and asset.xml files
*
* @author  Edison Wang <ewang@squiz.com.au>
* @package MySource_Matrix_Packages
* @subpackage marketo
*/
class Package_Manager_Marketo extends Package_Manager
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		$this->_full_path = SQ_PACKAGES_PATH.'/marketo';
		$this->Package_Manager();

	}//end constructor


}//end class

?>
