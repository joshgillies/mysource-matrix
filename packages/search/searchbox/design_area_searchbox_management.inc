<?php

/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: design_area_searchbox_management.inc,v 1.1.1.1 2003/10/08 01:20:58 dwong Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Design_Area_SearchBox_Management
*
* @see Asset_Management
*
* @author  Dominic Wong <dwong@squiz.net>
* @version $Revision: 1.1.1.1 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage search
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

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

		$this->vars = Array(
			'search_page_assetid' => Array (
				'added'			=> '0.1',
				'type'			=> 'int',
				'default'		=> '0',
				'description'	=> 'Search Page where queries are deferred to and results displayed on.'
			),
			'submit_button' => Array (
				'added'			=> '0.1',
				'type'			=> 'boolean',
				'default'		=> true,
				'description'	=> 'Whether the search button is a button or an image.'
			),
			'submit_image_assetid' => Array(
				'added'			=> '0.1',
				'type'			=> 'int',
				'default'		=> '',
				'description'	=> 'Image Icon Asset'
			),
			'querybox_width'	=> Array(
				'added'			=> '0.1',
				'type'			=> 'int',
				'default'		=> '5',
				'description'	=> 'The width of the single query box.'
			),
			'field_name'		=> Array(
				'added'			=> '0.1',
				'type'			=> 'text',
				'default'		=> '',
				'description'	=> 'The field defined in the search page that this is attached to.',
			),
				
		);

	} // end Design_Area_Searchbox_Management()
}//end class
?>
