<?php
/**
* +-----------------------------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd           |
* | ABN 77 084 670 600                                                                            |
* +-----------------------------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find            |
* | this file please contact Squiz (www.squiz.com.au) so we may provide |
* | you a copy.                                                                                         |
* +-----------------------------------------------------------------------------------------+
*
* $Id: saml_account_manager_management.inc,v 1.3 2012/09/04 07:01:49 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* SAML_Account_Manager_Management
*
* @see Asset_Management
*
* @author  Edison Wang <ewang@squiz.com.au>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage file
*/
class SAML_Account_Manager_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		parent::__construct($pm);

		$this->vars['create_type']				= Array(
													'added'		=> '0.1',
													'type'		=> 'serialise',
													'default'	=> Array('user' => TRUE),
												  );
		$this->vars['use_extensions']				= Array(
													'added'		=> '0.1',
													'type'		=> 'boolean',
													'default'		=> FALSE,
													'parameters'    => Array(
															'allow_empty'   => FALSE,
													    ),
												  );
		
		$this->vars['extensions']					= Array(
										'added'		=> '0.1',
										'type'		=> 'text',
										'default'	=>   '<samlp:Extensions  xmlns:samlp="urn:oasis:names:tc:SAML:2.0:protocol" xmlns:saml="urn:oasis:names:tc:SAML:2.0:assertion" ></samlp:Extensions>',
									   );
		$this->vars['auth_source']				= Array(
										'added'		=> '0.1',
										'type'		=> 'text',
										'default'	=>   '',
									   );
		$this->vars['saml_userid_attribute']				= Array(
										'added'		=> '0.1',
										'type'		=> 'text',
										'default'	=>   '',
									   );
		$this->vars['saml_userid_location']				= Array(
										'added'		=> '0.0.1',
										'type'		=> 'selection',
										'default'		=> 'nameid',
										'parameters'	=> Array(
														'options'		=> Array(
																			'nameid'	=> 'NameID element',
																			'attribute'	=> 'Attribute element',
																	),	
														'multiple'		=> FALSE,
														'allow_empty'	=> FALSE,	
													),
									);
		$this->vars['protocol_binding']				= Array(
										'added'		=> '0.0.1',
										'type'		=> 'selection',
										'default'		=> '',
										'parameters'	=> Array(
														'options'		=> Array(
																			'urn:oasis:names:tc:SAML:2.0:bindings:HTTP-POST'	=> 'HTTP POST',
																			'urn:oasis:names:tc:SAML:2.0:bindings:HTTP-Redirect'	=> 'HTTP Redirect',
																			'urn:oasis:names:tc:SAML:2.0:bindings:HTTP-Artifact'	=> 'HTTP Artifact',
																			'urn:oasis:names:tc:SAML:2.0:bindings:SOAP'		=> 'SOAP',
																	),	
														'multiple'		=> FALSE,
														'allow_empty'	=> TRUE,	
													),
									);
		$this->vars['nameid_policy']				= Array(
										'added'		=> '0.0.1',
										'type'		=> 'selection',
										'default'		=> '',
										'parameters'	=> Array(
														'options'		=> Array(
																			'urn:oasis:names:tc:SAML:2.0:nameid-format:persistent'	=> 'Persistent',
																			'urn:oasis:names:tc:SAML:2.0:nameid-format:transient'	=> 'Transient',

																	),	
														'multiple'		=> FALSE,
														'allow_empty'	=> TRUE,	
													),
									);
		$this->vars['authn_context_class_ref']				= Array(
										'added'		=> '0.1',
										'type'		=> 'text',
										'default'	=>   '',
									   );
		$this->vars['redirect_url']				= Array(
										'added'			=> '0.0.1',
										'type'			=> 'text',
										'default'		=> '',
										'is_admin'		=> FALSE,
										'description'	=> 'URL to redirect to after successful SAML authentication',
								
									);
		$this->vars['use_querystring_parameter']			= Array(
										'added'			=> '0.2',
										'type'			=> 'text',
										'default'		=> '',
										'description'	=> 'Query string parameter name whose value is to be used as the redirect destination',
									    );
		$this->vars['redirect_after_login']				= Array(
										'added'		=> '0.1',
										'type'		=> 'boolean',
										'default'		=> TRUE,
										'parameters'    => Array(
												'allow_empty'   => FALSE,
										    ),
									    );
		$this->vars['redirect_after_create']				= Array(
										'added'		=> '0.1',
										'type'		=> 'boolean',
										'default'		=> TRUE,
										'parameters'    => Array(
												'allow_empty'   => FALSE,
										    ),
									    );
		$this->vars['redirect_after_link']				= Array(
										'added'		=> '0.1',
										'type'		=> 'boolean',
										'default'		=> TRUE,
										'parameters'    => Array(
												'allow_empty'   => FALSE,
										    ),
									    );
		$this->vars['allow_link']					= Array(
										'added'		=> '0.1',
										'type'		=> 'boolean',
										'default'		=> TRUE,
										'parameters'    => Array(
												'allow_empty'   => FALSE,
										    ),
									    );
		$this->vars['auto_create']				= Array(
										'added'		=> '0.1',
										'type'		=> 'boolean',
										'default'		=> TRUE,
										'parameters'    => Array(
												'allow_empty'   => FALSE,
										    ),
									    );
				

	}//end constructor

	
	/**
	* Return an array with information about the events that the account manager is listening for
	*
	* @return boolean
	* @access public
	*/
	function getEventList()
	{
		return Array(
				Array(
					'event_name'					=> 'requestKeywords',
					'broadcast_type_code'			=> 'content_type',
					'broadcast_strict_type_code'	=> FALSE,
					'options'						=> Array(
														'side_of_link'	=> 'major',
														'indirect'		=> TRUE,
														'is_exclusive'	=> NULL,
														'is_dependant'	=> 1,
														'value'			=> '',
														'link_type'		=> SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
													   ),
				),
			   );

	}//end getEventList()

}//end class
?>
