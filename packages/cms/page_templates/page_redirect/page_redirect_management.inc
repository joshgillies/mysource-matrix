<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: page_redirect_management.inc,v 1.23 2012/08/30 00:58:09 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Redirect_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.23 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Redirect_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'permanent_redirect'				=> Array(
														'added'			=> '0.4',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		),
													
													   ),
						'redirect_url'				=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'text',
														'default'		=> '',
														'is_admin'		=> FALSE,
														'description'	=> 'URL to redirect to, including URI scheme'
													   ),
						'url_suffix'				=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'text',
														'default'		=> '',
														'description'	=> 'Extra text to append as part of the URL',
													   ),
						'timeout'					=> Array(
														'added'			=> '0.3',
														'type'			=> 'float',
														'default'		=> 0,
														'description'	=> 'Amount of time before a page redirects (only applies when new window is turned off)',
														'parameters'	=> Array(
																			'decimal_places'	=> 1,
																		   ),
													   ),
						'new_window'				=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'new_window_options'		=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'selection',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'multiple'		=> TRUE,
																			'allow_empty'	=> TRUE,
																			'options'		=> Array(
																								'hide_location'		=> 'Hide Location Bar',
																								'hide_menubar'		=> 'Hide Menubar',
																								'hide_scrollbars'	=> 'Hide Scrollbars',
																								'hide_status'		=> 'Hide Status Bar',
																								'hide_toolbar'		=> 'Hide Tool Bar',
																								'not_resizable'		=> 'Not Resizable',
																							   ),
																		   ),
													   ),
						'new_window_width'			=> Array(
														'added'		=> '0.0.1',
														'type'		=> 'int',
														'default'	=> 0,
													   ),
						'new_window_height'			=> Array(
														'added'		=> '0.0.1',
														'type'		=> 'int',
														'default'	=> 0,
													   ),
						'append_querystring'		=> Array(
														'added'			=> '0.2',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
														'description'	=> 'Append inbound query string parameters to outbound destination url.',
													   ),
						'use_querystring_parameter'	=> Array(
														'added'			=> '0.2',
														'type'			=> 'text',
														'default'		=> '',
														'description'	=> 'Query string parameter name whose value is to be used as the redirect destination',
													   ),
						'session_variables'			=> Array(
														'added'			=> '0.4',
														'type'			=> 'parameter_map',
														'default'		=> Array(),
														'description'	=> 'Use saved variables to pass to the redirect page',
														'parameters'	=> Array(
																			'parameters'	=> Array(),
																		   ),
													   ),
						'session_vars'				=> Array(
														'added'			=> '0.4',
														'type'			=> 'option_list',
														'default'		=> '',
														'description'	=> 'Use saved variables to pass to the redirect page',
													   ),
					  );

	}//end constructor


}//end class

?>
