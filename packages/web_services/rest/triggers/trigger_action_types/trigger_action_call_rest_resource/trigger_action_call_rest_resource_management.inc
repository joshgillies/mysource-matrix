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
* $Id: trigger_action_call_rest_resource_management.inc,v 1.2 2009/10/07 23:48:34 akarelia Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Trigger_Action_Call_Rest_Resource_Management
*
* @see Asset_Management
*
* @author  Maki Maruyama <mmaruyama@squiz.co.uk>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage rest
*/
class Trigger_Action_Call_Rest_Resource_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager &$pm)
	{
		Asset_Management::__construct($pm);

	}//end constructor


}//end class

?>