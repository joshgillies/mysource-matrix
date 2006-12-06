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
* $Id: trigger_action_create_file_asset_edit_fns.inc,v 1.2 2006/12/06 00:07:40 bcaldwell Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action_edit_fns.inc';

/**
* Trigger_Action_Create_File_Asset_Edit_Fns
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Create_File_Asset_Edit_Fns extends Trigger_Action_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Trigger_Action_Create_File_Asset_Edit_Fns()
	{
		$this->Trigger_Action_Edit_Fns();

	}//end constructor


}//end class

?>
