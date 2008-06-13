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
* $Id: tool_delete_bmail_users_management.inc,v 1.2.2.2 2008/06/13 01:58:40 mbrydon Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Delete Bulkmail Users Tool - Asset Management
*
* @see Asset_Management
*
* @author Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.2.2.2 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class Tool_Delete_Bmail_Users_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Tool_Delete_Bmail_Users_Management(&$pm)
	{
		$this->Asset_Management($pm);


	}//end constructor


}//end class

?>
