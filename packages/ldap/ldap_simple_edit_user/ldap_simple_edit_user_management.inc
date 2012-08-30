<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: ldap_simple_edit_user_management.inc,v 1.2 2012/08/30 00:58:58 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Ldap_Simple_Edit_User_Management
*
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage ldap
*/
class Ldap_Simple_Edit_User_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Ldap_Simple_Edit_User_Management(&$pm)
	{
		$this->Asset_Management($pm);

	}//end constructor


}//end class

?>