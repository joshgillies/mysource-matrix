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
* $Id: package_manager_bulkmail.inc,v 1.2 2005/10/27 01:38:15 ndvries Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/package_manager.inc';

/**
* Package_Manager_Bulkmail
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Rayne Ong <rong@squiz.net>
* @version $Version$
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class Package_Manager_Bulkmail extends Package_Manager
{


	/**
	* Constructor
	*
	*/
	function Package_Manager_Bulkmail()
	{
		$this->_full_path = SQ_PACKAGES_PATH.'/bulkmail';
		$this->Package_Manager();

	}//end constructor


}//end class

?>
