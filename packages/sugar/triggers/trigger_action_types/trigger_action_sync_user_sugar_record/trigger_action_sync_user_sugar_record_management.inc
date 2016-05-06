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
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Trigger_Action_Sync_User_Sugar_Record_Management
*
* @see Asset_Management
*
* @package MySource_Matrix_Packages
* @subpackage sugar
*/
class Trigger_Action_Sync_User_Sugar_Record_Management extends Asset_Management
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