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
* $Id: package_manager_news.inc,v 1.6 2012/08/30 00:59:01 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/package_manager.inc';

/**
* Package_Manager_News
*
* Purpose
*    Manages the install and upgrade of the News package and News assets,
*    uses info gleened from the package.xml and asset.xml files
*
* @author  Avi Miller <amiller@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage news
*/
class Package_Manager_News extends Package_Manager
{


	/**
	* Constructor
	*
	*/
	function Package_Manager_News()
	{
		$this->_full_path = SQ_PACKAGES_PATH.'/news';
		$this->Package_Manager();

	}//end constructor


}//end class

?>