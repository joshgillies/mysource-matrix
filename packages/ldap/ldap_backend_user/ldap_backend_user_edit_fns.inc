<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: ldap_backend_user_edit_fns.inc,v 1.5 2010/05/05 07:13:52 ewang Exp $
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
* @version $Revision: 1.5 $
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
