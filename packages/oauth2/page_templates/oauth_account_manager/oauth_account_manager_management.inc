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
* $Id: oauth_account_manager_management.inc,v 1.1.1.1 2013/07/19 00:29:27 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Oauth_Account_Manager_Management
*
* @see Asset_Management
*
* @author  Edison Wang <ewang@squiz.com.au>
* @version $Revision: 1.1.1.1 $
* @package MySource_Matrix_Packages
* @subpackage oauth2
*/
class Oauth_Account_Manager_Management extends Asset_Management
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
		$this->vars['userid_api_url']				= Array(
										'added'			=> '0.0.1',
										'type'			=> 'text',
										'default'		=> '',
										'is_admin'		=> FALSE,
										'description'	=> 'Rest API URL to get user id',
								
									);
		$this->vars['userid_location']				= Array(
										'added'			=> '0.0.1',
										'type'			=> 'text',
										'default'		=> '',
										'is_admin'		=> FALSE,
										'description'	=> 'JSON element of user id in API response',
								
									);
		$this->vars['prefilled_fields']				= Array(
										'added'		=> '0.1',
										'type'		=> 'serialise',
										'default'	=> Array(),
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
