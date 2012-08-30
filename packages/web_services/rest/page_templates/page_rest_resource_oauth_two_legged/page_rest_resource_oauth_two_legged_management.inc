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
* $Id: page_rest_resource_oauth_two_legged_management.inc,v 1.3 2012/08/30 00:59:46 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';
require_once SQ_ATTRIBUTES_PATH.'/oauth/oauth.inc';

/**
* Page_REST_Resource_OAuth_Two_Legged_Management
*
* @see Asset_Management
*
* @author  Maki Maruyama <mmaruyama@squiz.co.uk>
* @version $ $
* @package web_services
* @subpackage rest
*/
class Page_REST_Resource_OAuth_Two_Legged_Management extends Asset_Management
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
													'added'		=> '0.1',
													'type'		=> 'oauth',
													'default'   => Array(
																	'type' => 'TWO_LEGGED',
																   ),
												   ),
						'allow_global_replace'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE
																	   ),
												   ),
		);

	}//end constructor

}//end class
?>
