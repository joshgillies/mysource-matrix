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
* $Id: package_manager_ecommerce.inc,v 1.2.2.1 2005/05/01 23:41:47 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_INCLUDE_PATH.'/package_manager.inc';

/**
* Package_Manager_Ecommerce
*
* Purpose
*    Manages the install and upgrade of the Ecommerce package and News assets,
*    uses info gleened from the package.xml and asset.xml files
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.2.2.1 $
* @package MySource_Matrix_Packages
* @subpackage news
*/
class Package_Manager_Ecommerce extends Package_Manager
{


	/**
	* Constructor
	*
	*/
	function Package_Manager_Ecommerce()
	{
		$this->_full_path = dirname(__FILE__);
		$this->Package_Manager();

	}//end constructor


}//end class

?>