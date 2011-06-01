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
* $Id: role_edit_fns.inc,v 1.2 2006/12/05 06:04:32 bcaldwell Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/users/user_group/user_group_edit_fns.inc';

/**
* Role_Edit_Fns
*
* Purpose
*
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Role_Edit_Fns extends User_Group_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Role_Edit_Fns()
	{
		$this->User_Group_Edit_Fns();

	}//end constructor


}//end class

?>
