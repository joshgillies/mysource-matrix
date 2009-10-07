<?php 
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: page_rest_resource_management.inc,v 1.1.2.4 2009/10/07 23:19:22 akarelia Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';
require_once SQ_ATTRIBUTES_PATH.'/http_request/http_request.inc';

/**
* Page_REST_Resource_Management
*
* @see Asset_Management
*
* @author  James Hurst <jhurst@squiz.co.uk>
* @version $Revision: 1.1.2.4 $
* @package Squiz_UK_Clients
* @subpackage rest_resource_js
*/
class Page_REST_Resource_Management extends Asset_Management
{
	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array(
			'http_request'	=> Array(
				'added' => '0.1',
				'type'	=> 'http_request',
				'default'	=> Array(),
			),
			'send_headers'	=> Array(
				'added'		=> '0.1',
				'type'		=> 'boolean',
				'default'	=> FALSE,
				'parameters'	=> Array('allow_empty'	=> FALSE),
			),
			'error_response'	=> Array(
				'added'			=> '0.1',
				'type'			=> 'text',
				'default'		=> '',
			),
			'allow_multiple_runs'	=> Array(
				'added'		=> '0.1',
				'type'		=> 'boolean',
				'default'	=> FALSE,
				'parameters'	=> Array('allow_empty'	=> FALSE),
			),
		);

	}//end constructor

}//end class
?>
