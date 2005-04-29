<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: authentication_ldap_edit_fns.inc,v 1.3 2005/04/29 05:38:57 gsherwood Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/authentication/authentication/authentication_edit_fns.inc';

/**
* Authentication_LDAP_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage ldap
*/
class Authentication_LDAP_Edit_Fns extends Authentication_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Authentication_LDAP_Edit_Fns()
	{
		$this->Authentication_Edit_Fns();

	}//end constructor


}//end class

?>