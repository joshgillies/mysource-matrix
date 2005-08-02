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
* $Id: package_manager_news.inc,v 1.4 2005/08/02 01:17:50 gsherwood Exp $
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
* @version $Revision: 1.4 $
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