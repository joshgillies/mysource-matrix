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
* $Id: page_management.inc,v 1.3 2004/09/14 01:38:33 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Page_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Page_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array('name' => Array(
												'added'       => '0.0.1',
												'type'        => 'text',  'default'   => '',
												'description' => 'The full name of the page',
												'update_fn'   => '',
												),
							'short_name' => Array(
												'added'       => '0.0.1',
												'type'        => 'text',  'default'   => '',
												'description' => 'The short name of the page (often used in menus)',
												'update_fn'   => '',
												)
							);

		$this->components = Array(
								'name' => Array (
									'description'       => 'Asset name',
									'default_weighting' => '1',
								),
								'short_name' => Array (
									'description'       => 'Short name',
									'default_weighting' => '1',
								),
							);

	}//end constructor


}//end class

?>