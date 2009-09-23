<?php 
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: page_rest_resource_management.inc,v 1.1 2009/09/23 07:24:14 akarelia Exp $
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
* @version $ $
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
