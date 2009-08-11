<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: package_manager_funnelback.inc,v 1.1.1.1 2009/08/11 01:51:54 bpearson Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/package_manager.inc';

/**
* Funnelback 
*
* Purpose
*    Manages the install and upgrade of the Funnelback package and Funnelback assets,
*    uses info gleened from the package.xml and asset.xml files
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.1.1.1 $
* @package MySource_Matrix_Packages
* @subpackage funnelback
*/
class Package_Manager_Funnelback extends Package_Manager
{


	/**
	* Constructor
	*
	*/
	function Package_Manager_Funnelback()
	{
		$this->_full_path = SQ_PACKAGES_PATH.'/funnelback';
		$this->Package_Manager();

	}//end constructor


}//end class

?>
