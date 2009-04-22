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
* $Id: trigger_action_remove_web_path_management.inc,v 1.2 2009/04/22 06:25:09 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Trigger_Action_Remove_Web_Path_Management
*
* @see Asset_Management
*
* @author  Edison WANG<ewang@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Remove_Web_Path_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Trigger_Action_Remove_Web_Path_Management(&$pm)
	{
		$this->Asset_Management($pm);

	}//end constructor


}//end class

?>
