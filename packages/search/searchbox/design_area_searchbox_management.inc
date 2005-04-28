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
* $Id: design_area_searchbox_management.inc,v 1.10 2005/04/28 05:02:26 arailean Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_SearchBox_Management
*
* @see Asset_Management
*
* @author  Dominic Wong <dwong@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Design_Area_SearchBox_Management extends Asset_Management
{


	/**
	* Constructor
	*
	* @access public
	*/
	function Design_Area_SearchBox_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['search_page_assetid'] = Array(
												'added'			=> '0.1',
												'type'			=> 'assetid',
												'default'		=> '',
												'description'	=> 'Search Page where queries are deferred to and results displayed on.',
											 );

		$this->vars['submit_button'] = Array(
										'added'			=> '0.1',
										'type'			=> 'boolean',
										'default'		=> true,
										'description'	=> 'Whether the search button is a button or an image.',
										'parameters'	=> Array('allow_empty' => false, ),
									   );

		$this->vars['submit_button_text'] = Array(
												'added'		=> '0.2',
												'type'		=> 'text',
												'default'	=> 'Go',
											);

		$this->vars['submit_image_assetid'] = Array(
												'added'			=> '0.1',
												'type'			=> 'assetid',
												'default'		=> '',
												'description'	=> 'Image Icon Asset',
											  );

		$this->vars['querybox_width'] = Array(
											'added'			=> '0.1',
											'type'			=> 'int',
											'default'		=> '5',
											'description'	=> 'The width of the single query box.',
										);

		$this->vars['field_name'] = Array(
										'added'			=> '0.1',
										'type'			=> 'text',
										'default'		=> '',
										'description'	=> 'The field defined in the search page that this is attached to.',
									);

		$this->vars['default_text'] = Array(
										'added'			=> '0.1',
										'type'			=> 'text',
										'default'		=> '',
										'description'	=> 'The default text to display in the search box.',
									  );

	}//end constructor


}//end class
?>
