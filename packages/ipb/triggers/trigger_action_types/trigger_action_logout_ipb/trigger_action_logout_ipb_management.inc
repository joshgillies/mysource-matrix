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
* $Id: trigger_action_logout_ipb_management.inc,v 1.2 2006/12/07 03:11:33 bcaldwell Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Trigger_Action_Logout_Ipb_Management
*
* @see Asset_Management
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Logout_Ipb_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Trigger_Action_Logout_Ipb_Management(&$pm)
	{
		$this->Asset_Management($pm);

	}//end constructor


}//end class
?>
