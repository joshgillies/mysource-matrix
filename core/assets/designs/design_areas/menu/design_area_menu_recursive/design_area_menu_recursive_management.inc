<?php
/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
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
* $Id: design_area_menu_recursive_management.inc,v 1.9 2003/11/18 15:37:26 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Menu_Recursive_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Menu_Recursive_Management extends Asset_Management
{
	/**
	* Constructor
	*
	*/
	function Design_Area_Menu_Recursive_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
							'min_num_levels' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'int',
								'default'		=> 0,
								'description'	=> 'The minimum number of levels into the hierarchy to show'
							),
							'max_num_levels' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'int',
								'default'		=> 0,
								'description'	=> 'The maximum number of levels into the hierarchy to show, blank or zero means no maximum'
							),
						);

	}//end constructor

}//end class
?>
