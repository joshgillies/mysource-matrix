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
* $Id: design_area_asset_lineage_management.inc,v 1.6 2004/09/08 01:25:14 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Asset_Lineage_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Asset_Lineage_Management extends Asset_Management
{
	/**
	* Constructor
	*
	*/
	function Design_Area_Asset_Lineage_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
							'divider_contents' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'serialise',
								'default'		=> Array(),
								'description'   => 'The contents that divides the asset info in the output',
							),
							'asset_contents' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'serialise',
								'default'		=> Array(),
								'description'   => 'The contents that prints the asset info in the output',
							),
							'prefix_with_home_link' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'boolean',
								'default'		=> false,
								'description'	=> 'whether or not to start the list with a link to the site\'s index',
								'parameters' 	=> Array(
															'allow_empty' => false
														),
							),
							'suffix_with_current_link' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'boolean',
								'default'		=> false,
								'description'	=> 'whether or not to end the list with a link to the current page',
								'parameters' 	=> Array(
															'allow_empty' => false
														),
							),
							'prefix_with_divider' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'boolean',
								'default'		=> false,
								'description'	=> 'whether or not to start the list with a divider',
								'parameters' 	=> Array(
															'allow_empty' => false
														),
							),
							'suffix_with_divider' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'boolean',
								'default'		=> true,
								'description'	=> 'whether or not to end the list with a divider',
								'parameters' 	=> Array(
															'allow_empty' => false
														),
							),
							'show_hidden_links' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'boolean',
								'default'		=> true,
								'description'	=> 'whether or not to show TYPE_2 links as well as TYPE_1',
								'parameters'	=> Array(
															'allow_empty' => false
														),
							),
							'levels_to_print' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'int',
								'default'		=> 0,
								'description'	=> 'How many levels deep to print, (zero, blank, empty means print all)'
							),
						);

	}//end constructor

}//end class
?>
