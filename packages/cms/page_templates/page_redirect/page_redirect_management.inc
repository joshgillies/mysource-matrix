<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: page_redirect_management.inc,v 1.6 2003/10/21 06:25:43 dwong Exp $
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
class Page_Redirect_Management extends Page_Management
{

	/**
	* Constructor
	*
	*/
	function Page_Redirect_Management(&$pm)
	{
		$this->Page_Management($pm);

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