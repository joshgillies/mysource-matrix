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
* $Id: form_action_set_session_var_management.inc,v 1.2 2012/08/30 00:58:08 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';


/**
* Form_Action_Set_Session_Var_Management
*
* @see Asset_Management
*
* @author  Wojciech Szymanski <wojtek@squiz.pl>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/


class Form_Action_Set_Session_Var_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

	}//end constructor


}//end class
?>
