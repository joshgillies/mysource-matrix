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
* $Id: page_site_map_management.inc,v 1.5.2.1 2004/02/18 12:30:09 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Site_Map_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Site_Map_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Page_Site_Map_Management(&$pm)
	{
		$this->Asset_Management($pm);

		
		$this->vars = Array(
							'name' => Array (
									'added'		=> '0.0.1',
									'type'		=> 'text',
									'default'	=> '',
							), // end name
							'stalks' => Array(
									'added'		=> '0.0.1',
									'type'		=> 'boolean',
									'default'	=> true,
							), // end stalks
							'top_level'	=> Array(
									'added'		=> '0.0.1',
									'type'		=> 'boolean',
									'default'	=> false,
							), // end top_level
							'columns' => Array(
									'added'		=> '0.0.1',
									'type'		=> 'int',
									'default'	=> 1,
							), // end levels
							'levels' => Array(
									'added'		=> '0.0.1',
									'type'		=> 'int',
									'default'	=> 0,
							),
							'exclude_list'	=>	Array(
									'added'		=> '0.0.1',
									'type'		=> 'serialise',
									'default'	=> Array(),
							),
							'icons'		=> Array(
								'added'		=> '0.0.1',
								'type'		=> 'boolean',
								'default'	=> false,
							),
						);

	}//end constructor

}//end class

?>