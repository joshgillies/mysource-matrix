<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: ldap_backend_user_edit_fns.inc,v 1.4 2007/06/05 02:07:57 lwright Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/ldap/ldap_user/ldap_user_edit_fns.inc';

/**
* Ldap_Backend_User_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @author  Rayn Ong <rong@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage ldap
*/
class Ldap_Backend_User_Edit_Fns extends Ldap_User_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Ldap_Backend_User_Edit_Fns()
	{
		$this->Ldap_User_Edit_Fns();

	}//end constructor


}//end class

?>
