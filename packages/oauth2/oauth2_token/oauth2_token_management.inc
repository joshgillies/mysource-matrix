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
* $Id: oauth2_token_management.inc,v 1.1.1.1 2013/07/19 00:29:26 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';
require_once SQ_ATTRIBUTES_PATH.'/http_request/http_request.inc';

/**
* Oauth2_Token_Management
*
* @see Asset_Management
*
* @author  Edison Wang <ewang@squiz.com.au>
* @version $ $
* @package MySource_Matrix_Packages
* @subpackage oauth2
*/
class Oauth2_Token_Management extends Asset_Management
{
	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array(
						'client_id'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'			=> '',
												   ),
						'client_secret'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'			=> '',
												   ),
		    				'authorization_endpoint'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'			=> '',
												   ),
		    				'token_endpoint'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'			=> '',
												   ),
    		    				'scope'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'			=> '',
												   ),
		    		    		'redirect_uri'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'			=> '',
												   ),
		    				'token_storage'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'default'		=> 'session',
												'parameters'	=> Array(
																	'options'		=> Array(
																						'session'   => 'Session',
																						'global'	=> 'Global Permanent',
																						'user'	=> 'Per User Permanent',
																					   ),
																	'multiple'		=> FALSE,
																	'allow_empty'	=> FALSE,
																   ),
											   ),
		    				'global_access_token'	=> Array(
												'added'		=> '0.1',
												'type'		=> 'serialise',
												'default'	=> Array(),
											   ),
		        		    		'configuration_hash'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'			=> '',
												   ),
		    		        		'access_token_param_name'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'			=> 'access_token',
												   ),
		    		    		'authorization_type'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'default'		=> 'uri',
												'parameters'	=> Array(
																	'options'		=> Array(
																						'uri'   => 'URI',
																						'authorization_basic'	=> 'Authorization Basic',
																						'form'	=> 'Form',
																					   ),
																	'multiple'		=> FALSE,
																	'allow_empty'	=> FALSE,
																   ),
											),
						'access_token_type'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'default'		=> 'uri',
												'parameters'	=> Array(
																	'options'		=> Array(
																						'uri'   => 'URI',
																						'bearer'	=> 'Bearer',
																						'oauth'	=> 'Oauth',
																					   ),
																	'multiple'		=> FALSE,
																	'allow_empty'	=> FALSE,
																   ),
											),
						'verify_ssl_peer'			=> Array(
										'added'		=> '0.1',
										'type'		=> 'boolean',
										'default'		=> FALSE,
										'parameters'    => Array(
												'allow_empty'   => FALSE,
										    ),
									    ),
						
			          );

	}//end constructor

}//end class
?>
