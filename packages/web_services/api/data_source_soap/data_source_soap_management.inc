<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ABN 77 084 670 600												   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: data_source_soap_management.inc,v 1.2 2013/02/18 04:12:19 cupreti Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Data_Source_Soap_Management
*
* @see Asset_Management
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/

class Data_Source_Soap_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Data_Source_Soap_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars	= Array (
						'name'				=> Array (
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'The name of the datasource',
											   ),
						'wsdl'				=> Array(
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'SOAP service WSDL link',
											   ),
						'username'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'Authentication login username',
											   ),
						'password'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'password',
												'default'		=> '',
												'description'	=> 'Authentication password',
											   ),
						'auth_type'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'default'		=> 'HTTP_BASIC',
												'parameters'    => Array(
																		'multiple'      => FALSE,
																		'allow_empty'   => FALSE,
																		'options'       => Array(
																							'HTTP_BASIC'        => 'HTTP Basic',
																							'USERNAME_TOKEN'	=> 'WS Username Token',
																						),
																	),
												'description' 	=> 'Authentication type to use',
											   ),
						'ws_namespace'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'Namespace for the WS authentication',
											   ),
						'local_cert'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'assetid',
												'default'		=> '',
												'description' 	=> 'Asset ID of the file asset for the HTTPS client certificate authentication',
											   ),
						'passphrase'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'password',
												'default'	=> '',
												'description' 	=> 'Passphrase for the HTTPS client certificate authentication, if applicable',
											   ),
						'mute_exception'	=> Array(
												'added'		=> '0.1',
												'type'		=> 'boolean',
												'default'	=> FALSE,
												'parameters'    => Array( 'allow_empty' => FALSE,),
												'description' 	=> 'Whether to mute SOAP expection in response',
											   ),
						'request_fn'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
												'description' 	=> 'Selected SOAP request function',
											   ),
						'soap_requests'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'serialise',
												'default'	=> Array(),
												'description' 	=> 'Available SOAP requests',
											   ),
						'file_keywords'		=> Array (
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'Data record set keywords for SOAP responses that are expected as a file',
											   ),
						'root_tag'		=> Array (
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'Root tag in XML response to look into',
											   ),
						'selected_tags'		=> Array (
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'Comma-separated list of tag name to capture',
											   ),
						'data_delimiter'	=> Array (
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> ',',
												'description'	=> 'Delimiler to seperate data from xml elements with same name',
											   ),
					  );

	}//end constructor


}//end class
?>