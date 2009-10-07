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
* $Id: page_rest_resource_js_management.inc,v 1.1.2.4 2009/10/07 23:19:22 akarelia Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';
require_once SQ_ATTRIBUTES_PATH.'/http_request/http_request.inc';

/**
* Page_REST_Resource_JS_Management
*
* @see Asset_Management
*
* @author  James Hurst <jhurst@squiz.co.uk>
* @version $Revision: 1.1.2.4 $
* @package Squiz_UK_Clients
* @subpackage rest_resource_js
*/
class Page_REST_Resource_JS_Management extends Asset_Management
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
			'javascript_files'	=> Array(
				'added'	=> '0.1',
				'type'	=> 'serialise',
				'default'	=> Array(),
			),
			'javascript'	=> Array(
				'added'			=> '0.1',
				'type'			=> 'text',
				'default'		=> '',
			),
		);

	}//end constructor


}//end class
?>
