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
* $Id: authentication_ldap_management.inc,v 1.9 2012/08/30 00:58:58 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Authentication_LDAP_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.9 $
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
		$this->vars['name']['default'] = 'LDAP Authentication';
		$this->vars['log_in_as']       = Array(
											'type'			=> 'selection',
											'parameters'	=> Array(
																'options'		=> Array(
																					'ldap'	=> 'LDAP User',
																					'local'	=> 'Local User',
																				   ),
																'multiple'		=> FALSE,
																'allow_empty'	=> FALSE,
															   ),
											'default'		=> 'ldap',
											'added'			=> '0.2',
										 );

	}//end constructor


}//end class

?>