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
* $Id: trigger_action_increment_hit_count_management.inc,v 1.1 2007/07/19 04:24:17 bshkara Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Trigger_Action_Add_URL_Management
*
* @see Asset_Management
*
* @author  Christophe Olivar <colivar@squiz.net>
* @author  Basil Shkara <bshkara@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Increment_Hit_Count_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Trigger_Action_Increment_Hit_Count_Management(&$pm)
	{
		$this->Asset_Management($pm);

	}//end constructor


}//end class

?>
