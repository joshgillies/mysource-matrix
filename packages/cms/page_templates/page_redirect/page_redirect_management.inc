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
* $Id: page_redirect_management.inc,v 1.9 2004/02/10 16:32:41 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Redirect_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
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
						'redirect_url' => Array(
											'added'     => '0.0.1',
											'type'      => 'text',
											'default'   => '',
											),
						'url_suffix' => Array(
											'added'       => '0.0.1',
											'type'        => 'text',
											'default'     => '',
											'description' => 'Extra text that is added to the end of the url we end up redirecting to',
											),
						'new_window' => Array(
											'added'     => '0.0.1',
											'type'      => 'boolean',
											'default'   => false,
											),
						'new_window_options' => Array(
											'added'      => '0.0.1',
											'type'       => 'selection',
											'default'    => false,
											'parameters' => Array(
																'multiple' => true,
																'options'  => Array(
																				'hide_location'   => 'Hide Location Bar',
																				'hide_menubar'    => 'Hide Menubar',
																				'hide_scrollbars' => 'Hide Scrollbars',
																				'hide_status'     => 'Hide Status Bar',
																				'hide_toolbar'    => 'Hide Tool Bar',
																				'not_resizable'   => 'Not Resizable',
																			),
															),
											),
						'new_window_width' => Array(
											'added'      => '0.0.1',
											'type'       => 'int',
											'default'    => 0,
											),
						'new_window_height' => Array(
											'added'      => '0.0.1',
											'type'       => 'int',
											'default'    => 0,
											),
					);

	}//end constructor


}//end class

?>