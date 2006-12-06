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
* $Id: package_manager_news.inc,v 1.5 2006/12/06 22:14:18 emcdonald Exp $
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
* @version $Revision: 1.5 $
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