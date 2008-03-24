<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: trigger_action_calculate_average_comment_rating_management.inc,v 1.3 2008/03/24 22:09:25 bshkara Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Trigger_Action_Calculate_Average_Comment_Rating_Management
*
* @see Asset_Management
*
* @author  Basil Shkara <bshkara@squiz.net>
* @version $Revision: 1.3 $
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
