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
* $Id: authentication_ldap_management.inc,v 1.4 2005/04/29 05:38:57 gsherwood Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Authentication_LDAP_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage ldap
*/
class Authentication_LDAP_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Authentication_LDAP_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars =	Array(
							'name' => Array(
								'default' => 'LDAP Authentication',
							),
						);

	}//end constructor


}//end class

?>