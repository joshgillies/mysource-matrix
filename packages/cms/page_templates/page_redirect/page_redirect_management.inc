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
* $Id: page_redirect_management.inc,v 1.16 2005/12/07 05:33:04 lwright Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Redirect_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.16 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Redirect_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Page_Redirect_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'redirect_url'              => Array(
														'added'		=> '0.0.1',
														'type'		=> 'text',
														'default'	=> '',
														'is_admin'	=> false,
													   ),
						'url_suffix'                => Array(
														'added'			=> '0.0.1',
														'type'			=> 'text',
														'default'		=> '',
														'description'	=> 'Extra text to append as part of the URL',
													   ),
						'timeout'                   => Array(
														'added'       => '0.3',
														'type'        => 'float',
														'default'     => 0,
														'description' => 'Amount of time before a page redirects (only applies when new window is turned off)',
														'parameters' => Array(
																			'decimal_places' => 1,
																		),
													   ),
						'new_window'                => Array(
														'added'      => '0.0.1',
														'type'       => 'boolean',
														'default'    => false,
														'parameters' => Array(
																			'allow_empty' => false,
																		),
													   ),
						'new_window_options'        => Array(
														'added'      => '0.0.1',
														'type'       => 'selection',
														'default'    => false,
														'parameters' => Array(
																			'multiple'    => true,
																			'allow_empty' => true,
																			'options'     => Array(
																								'hide_location'   => 'Hide Location Bar',
																								'hide_menubar'    => 'Hide Menubar',
																								'hide_scrollbars' => 'Hide Scrollbars',
																								'hide_status'     => 'Hide Status Bar',
																								'hide_toolbar'    => 'Hide Tool Bar',
																								'not_resizable'   => 'Not Resizable',
																							 ),
																		),
													   ),
						'new_window_width'          => Array(
														'added'    => '0.0.1',
														'type'     => 'int',
														'default'  => 0,
													   ),
						'new_window_height'         => Array(
														'added'      => '0.0.1',
														'type'       => 'int',
														'default'    => 0,
													   ),
						'append_querystring'        => Array(
														'added'       => '0.2',
														'type'        => 'boolean',
														'default'     => false,
														'parameters'  => Array(
																			'allow_empty' => false,
																		 ),
														'description' => 'Append inbound query string parameters to outbound destination url.',
													   ),
						'use_querystring_parameter' => Array(
														'added'       => '0.2',
														'type'        => 'text',
														'default'     => '',
														'description' => 'Query string parameter name whose value is to be used as the redirect destination',
													   ),
					  );

	}//end constructor


}//end class

?>