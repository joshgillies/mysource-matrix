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
* $Id: page_rest_resource_oauth_session_management.inc,v 1.7.8.1 2013/10/11 07:27:46 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';
require_once SQ_ATTRIBUTES_PATH.'/oauth/oauth.inc';

/**
* Page_REST_Resource_OAuth_Session_Management
*
* @see Asset_Management
*
* @author  Maki Maruyama <mmaruyama@squiz.co.uk>
* @version $ $
* @package Squiz_UK_Clients
* @subpackage rest
*/
class Page_REST_Resource_OAuth_Session_Management extends Asset_Management
{
	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array(
						'oauth'					=> Array(
													'added'			=> '0.1',
													'type'			=> 'oauth',
													'default'   	=> Array(
																		'type' => 'THREE_LEGGED',
																	   ),
												   ),
						'allow_global_replace'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'parameters'	=> Array('allow_empty'	=> FALSE),
												   ),
						'redirect_authorise'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> TRUE,
													'parameters'	=> Array('allow_empty'	=> FALSE),
												   ),
		    				'store_token'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'parameters'	=> Array('allow_empty'	=> FALSE),
												   ),
						'access_token'	=> Array(
													    'added'		=> '0.1',
													    'type'		=> 'text',
													    'default'	=>   '',
												    ),
		    				'access_token_secret'	=> Array(
													    'added'		=> '0.1',
													    'type'		=> 'text',
													    'default'	=>   '',
												    ),
		    
					   );

	}//end constructor

}//end class
?>
