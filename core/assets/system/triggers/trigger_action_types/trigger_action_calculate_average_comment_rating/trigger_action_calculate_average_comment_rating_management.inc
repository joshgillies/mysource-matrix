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
* $Id: trigger_action_calculate_average_comment_rating_management.inc,v 1.2 2008/02/29 00:50:15 bshkara Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Trigger_Action_Calculate_Average_Comment_Rating_Management
*
* @see Asset_Management
*
* @author  Basil Shkara <bshkara@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Calculate_Average_Comment_Rating_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Trigger_Action_Calculate_Average_Comment_Rating_Management(&$pm)
	{
		$this->Asset_Management($pm);

	}//end constructor


}//end class
?>
