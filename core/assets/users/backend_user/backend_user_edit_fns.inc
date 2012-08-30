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
* $Id: backend_user_edit_fns.inc,v 1.11 2012/08/30 01:09:16 ewang Exp $
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
* @version $Revision: 1.11 $
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
