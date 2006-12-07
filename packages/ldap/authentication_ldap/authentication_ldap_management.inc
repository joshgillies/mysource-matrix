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
* $Id: authentication_ldap_management.inc,v 1.6 2006/12/07 01:04:33 bcaldwell Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Authentication_LDAP_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.6 $
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
							'name'	=> Array(
										'default'	=> 'LDAP Authentication',
									   ),
						);

	}//end constructor


}//end class

?>