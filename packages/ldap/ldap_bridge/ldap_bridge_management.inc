<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: ldap_bridge_management.inc,v 1.4 2004/09/08 04:00:01 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* LDAP_Bridge_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage ldap
*/
class LDAP_Bridge_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function LDAP_Bridge_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array('name' => Array(
											'added'     => '0.1',
											'type'      => 'text',
											'default'   => ''
											),
							'host' => Array(
											'added'     => '0.1',
											'type'      => 'text',
											'default'   => ''
											),
							'port' => Array(
											'added'     => '0.1',
											'type'      => 'text',
											'default'   => '389'
											),
							'root_dn' => Array(
											'added'     => '0.1',
											'type'      => 'text',
											'default'   => ''
											),
							'bind_dn' => Array(
											'added'     => '0.1',
											'type'      => 'text',
											'default'   => ''
											),
							'password' => Array(
											'added'     => '0.1',
											'type'      => 'password',
											'default'   => ''
											),
							'system_type' => Array(
											'added'      => '0.1',
											'type'       => 'selection',
											'default'    => 'openldap',
											'parameters' => Array(
																'options' => Array(
																		'openldap'        => 'OpenLDAP',
																		'activedirectory' => 'Active Directory',
																),
																'multiple' => false,
																'allow_empty' => false,
															),
											),
							'attributes' => Array(
											'added'     => '0.1',
											'type'      => 'serialise',
											'default'   => Array()
											),
							'user_attributes' => Array(
											'added'     => '0.1',
											'type'      => 'serialise',
											'default'   => Array()
											),
							);
	}//end constructor


}//end class

?>