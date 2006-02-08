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
* $Id: design_area_show_if_management.inc,v 1.4 2006/02/08 02:52:24 sdanis Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Show_If_Management
*
* @see Asset_Management
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Show_If_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Design_Area_Show_If_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'condition'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'The condition to be evaluated.',
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   ),
						'condition_data'	=> Array(
												'added'			=> '0.2',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'Data passed to the condition to help in evaluation',
											   ),
						'then_contents'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'The contents displayed when the condition is true',
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   ),
						'else_contents'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'The contents displayed when the condition is false',
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   ),
					  );

	}//end constructor


}//end class
?>
