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
* $Id: ldap_bridge_management.inc,v 1.9 2006/01/30 05:10:35 dmckee Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* LDAP_Bridge_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.9 $
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

		$this->vars = Array(
						'name'				=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'host'				=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'port'				=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '389',
											   ),
						'root_dn'			=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'bind_dn'			=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'password'			=> Array(
												'added'		=> '0.1',
												'type'		=> 'password',
												'default'	=> '',
											   ),
						'system_type'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'default'		=> 'openldap',
												'parameters'	=> Array(
																	'options'		=> Array(
																						'openldap'			=> 'OpenLDAP',
																						'openldap3'			=> 'OpenLDAP (protocol v3)',
																						'activedirectory'	=> 'Active Directory',
																					   ),
																	'multiple'		=> FALSE,
																	'allow_empty'	=> FALSE,
																   ),
											   ),
						'attributes'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'serialise',
												'default'	=> Array(),
											   ),
						'user_attributes'	=> Array(
												'added'		=> '0.1',
												'type'		=> 'serialise',
												'default'	=> Array(),
											   ),
					  );

	}//end constructor


}//end class

?>