<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: trigger_action_execute_database_query_edit_fns.inc,v 1.4 2012/08/30 00:58:16 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action_edit_fns.inc';

/**
* Trigger_Action_Execute_Database_Query_Edit_Fns
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage data
*/
class Trigger_Action_Execute_Database_Query_Edit_Fns extends Trigger_Action_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Trigger_Action_Execute_Database_Query_Edit_Fns()
	{
		parent::__construct();

	}//end constructor


}//end class

?>
