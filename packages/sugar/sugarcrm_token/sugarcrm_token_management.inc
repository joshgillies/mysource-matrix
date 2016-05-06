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
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';
require_once SQ_ATTRIBUTES_PATH.'/http_request/http_request.inc';

/**
* SugarCRM_Token_Management
*
* @see Asset_Management
*
* @author  Edison Wang <ewang@squiz.com.au>
* @package MySource_Matrix_Packages
* @subpackage sugar
*/
class SugarCRM_Token_Management extends Asset_Management
{
	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array(
						'username'			=> Array(
										'added'			=> '0.1',
										'type'			=> 'text',
										'default'			=> '',
									   ),
						'password'			=> Array(
										'added'			=> '0.1',
										'type'			=> 'password',
										'default'			=> '',
									   ),
						'client_id'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'			=> 'sugar',
												   ),
						'client_secret'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'			=> '',
												   ),
		    				'token_endpoint'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'			=> '',
												   ),
    		    				'platform'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'			=> 'squizmatrix',
												   ),
		    		    		'redirect_uri'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'			=> '',
												   ),
		    				'token_storage'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'default'		=> 'global',
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
												'default'		=> 'sugar',
												'parameters'	=> Array(
																	'options'		=> Array(
																						'uri'   => 'URI',
																						'bearer'	=> 'Bearer',
																						'oauth'	=> 'Oauth',
																						'sugar'	=> 'Sugar',
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
						'token_expiry'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'int',
													'default'			=> '',
												   ),
						'api_version'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'default'		=> 'v10',
												'parameters'	=> Array(
																	'options'		=> Array(
																						'v10'   => 'v10',
																					   ),
																	'multiple'		=> FALSE,
																	'allow_empty'	=> FALSE,
																   ),
											   ),
						
			          );

	}//end constructor

}//end class
?>
