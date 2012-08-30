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
* $Id: ldap_backend_user_management.inc,v 1.4 2012/08/30 00:58:58 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Ldap_Backend_User_Management
*
*
* @author  Rayn Ong <rong@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage ldap
*/
class Ldap_Backend_User_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Ldap_Backend_User_Management(&$pm)
	{
		$this->Asset_Management($pm);

	}//end constructor


}//end class

?>