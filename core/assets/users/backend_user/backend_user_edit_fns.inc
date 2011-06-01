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
* $Id: backend_user_edit_fns.inc,v 1.10 2006/12/05 06:04:31 bcaldwell Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/users/user/user_edit_fns.inc';

/**
* Backend_User_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Backend_User_Edit_Fns extends User_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Backend_User_Edit_Fns()
	{
		$this->User_Edit_Fns();

	}//end constructor


}//end class
?>
