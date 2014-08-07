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
* $Id: data_source_ldap_management.inc,v 1.5 2012/08/30 00:58:16 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Data_Source_Ldap_Management
*
* @see Asset_Management
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage data
*/

class Data_Source_Ldap_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Data_Source_Ldap_Management(&$pm)
	{
		$this->Asset_Management($pm);
		
		$this->vars	= Array (
						'name'				=> Array (
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'The name of the feed',
											   ),
						'ldap_options'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'serialise',
												'default'	=> Array(),
											   ),
						'aliases'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'default'		=> 'LDAP_DEREF_NEVER',
												'parameters'	=> Array(
																	'options'		=> Array(
																						'LDAP_DEREF_NEVER'		=> 'Never dereferenced',
																						'LDAP_DEREF_SEARCHING'	=> 'Dereferenced during search',
																						'LDAP_DEREF_FINDING'	=> 'Dereferenced when locating the base object',
																						'LDAP_DEREF_ALWAYS'		=> 'Dereferenced always',
																					   ),
																	'multiple'		=> FALSE,
																	'allow_empty'	=> FALSE,
																   ),
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
						'search_filter'		=> Array (
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'LDAP search filter query',
											   ),
						'sort_by'	=> Array (
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
											   ),											   
						'attributes'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'Comma seperated list of required attributes to extract from LDAP',
												),
						'binary_attributes'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'Comma seperated list of required binary attributes to extract from LDAP',
												),
					  );

	}//end constructor


}//end class
?>
