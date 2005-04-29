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
* $Id: ldap_user_group_management.inc,v 1.3 2005/04/29 05:38:58 gsherwood Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* User_Group_Management
*
* @see Asset_Management
*
* @author  Greg Serwood <greg@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage ldap
*/
class Ldap_User_Group_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Ldap_User_Group_Management(&$pm)
	{
		$this->Asset_Management($pm);

	}//end constructor


}//end class

?>