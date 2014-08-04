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
* $Id: page_rest_resource_js_management.inc,v 1.9 2013/02/18 05:39:53 ewang Exp $
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
* @version $Revision: 1.9 $
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
						'http_request'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'http_request',
													'default'		=> Array(),
												   ),
						'preprocess_keywords'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'parameters'	=> Array('allow_empty'	=> FALSE),
												   ),
						'javascript_files'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'serialise',
													'default'		=> Array(),
												   ),
						'javascript'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
												   ),
						'allow_global_replace'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'parameters'	=> Array('allow_empty'	=> FALSE),
												   ),
						'js_engine'			=> Array(
													'added'		=> '0.0.1',
													'type'		=> 'selection',
													'default'		=> 'spidermonkey',
													'parameters'	=> Array(
														'options'		=> Array(
																			'spidermonkey'	=> 'SpiderMonkey',
																			'v8'	=> 'V8',
																	),	
														'multiple'		=> FALSE,
														'allow_empty'	=> FALSE,	
															),
												),
					   );

	}//end constructor


}//end class
?>
