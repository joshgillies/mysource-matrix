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
* $Id: trigger_action_clear_cache_management.inc,v 1.2 2006/12/06 00:08:53 bcaldwell Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Trigger_Action_Clear_Cache_Management
*
* @see Asset_Management
*
* @author Rayn Ong <rong@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Clear_Cache_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Trigger_Action_Clear_Cache_Management(&$pm)
	{
		$this->Asset_Management($pm);

	}//end constructor


}//end class

?>
