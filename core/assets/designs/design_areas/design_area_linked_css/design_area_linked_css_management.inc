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
* $Id: design_area_linked_css_management.inc,v 1.3 2004/09/08 03:59:59 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Linked_Css_Management
*
* @see Asset_Management
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Linked_Css_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Design_Area_Linked_Css_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['tag_type'] = Array(
										'added'	=> '0.0.1',
										'type'	=> 'selection',
										'parameters' => Array(
											'options' => Array(
												'link_tag'		=> 'Link Tag',
												'import_tag'	=> '@import Tag',
											),
										'multiple' => false,
										'allow_empty' => false,
										),
										'default' => 'link_tag'
									);

		$this->vars['media'] = Array(
										'added'	=> '0.0.1',
										'type'	=> 'selection',
										'parameters' => Array(
											'options' => Array(
												'all'		 => 'All',
												'aural'		 => 'Aural',
												'braille'	 => 'Braille',
												'embossed'	 => 'Embossed',
												'handheld'	 => 'Handheld',
												'print'		 => 'Print',
												'projection' => 'Projection',
												'screen'	 => 'Screen',
												'tty'		 => 'TTY',
												'tv'		 => 'TV',
											),
										'multiple' => false,
										'allow_empty' => false,
										),
										'default' => 'all'
									);

	}//end constructor

}//end class
?>
