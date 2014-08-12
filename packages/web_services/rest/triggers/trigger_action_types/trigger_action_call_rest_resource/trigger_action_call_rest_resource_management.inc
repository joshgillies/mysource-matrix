<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: trigger_action_call_rest_resource_management.inc,v 1.4 2012/08/30 00:59:46 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Trigger_Action_Call_Rest_Resource_Management
*
* @see Asset_Management
*
* @author  Maki Maruyama <mmaruyama@squiz.co.uk>
* @version $Revision: 1.4 $
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