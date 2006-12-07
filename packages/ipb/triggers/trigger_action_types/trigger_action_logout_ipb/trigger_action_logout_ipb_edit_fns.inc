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
* $Id: trigger_action_logout_ipb_edit_fns.inc,v 1.2 2006/12/07 03:11:33 bcaldwell Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action_edit_fns.inc';

/**
* Trigger_Action_Logout_Ipb_Edit_Fns
*
* Purpose
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Logout_Ipb_Edit_Fns extends Trigger_Action_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Trigger_Action_Logout_Ipb_Edit_Fns()
	{
		$this->Trigger_Action_Edit_Fns();

	}//end constructor


}//end class

?>
