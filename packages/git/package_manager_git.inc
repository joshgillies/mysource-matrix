<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
*/

require_once SQ_INCLUDE_PATH.'/package_manager.inc';

/**
* Package_Manager_Git_Bridge
*
* Purpose
*    Manages the install and upgrade of the Git package and Filesystem assets,
*    uses info gleaned from the package.xml and asset.xml files
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: $
* @package MySource_Matrix_Packages
* @subpackage git
*/
class Package_Manager_Git extends Package_Manager
{

	/**
	* Constructor
	*
	*/
	function Package_Manager_Git()
	{
		$this->_full_path = SQ_PACKAGES_PATH.'/git';
		$this->Package_Manager();

	}//end constructor


}//end class

?>
