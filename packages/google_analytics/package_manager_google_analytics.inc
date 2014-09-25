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
*/

require_once SQ_INCLUDE_PATH.'/package_manager.inc';

/**
* Package_Manager_Google_Analytics
*
* Purpose
*    Manages the initial install and upgrade of the Google Analytics package
*    and its assets, using the info from package and asset XML files and any
*    install/upgrade scripts contained in this class.
*
* @author  Squiz Matrix Team <matrix-team@squiz.com.au>
* @package MySource_Matrix_Packages
* @subpackage google_analytics
*/
class Package_Manager_Google_Analytics extends Package_Manager
{

	/**
	* Constructor
	*
	*/
	function __construct()
	{
		$this->_full_path = SQ_PACKAGES_PATH.'/google_analytics';
		parent::__construct();

	}//end constructor


}//end class

?>
