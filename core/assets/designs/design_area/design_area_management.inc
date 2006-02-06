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
* $Id: design_area_management.inc,v 1.16 2006/02/06 23:27:39 dmckee Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.16 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Design_Area_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'id_name'			=> Array(
												'added'		=> '0.0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'contents'			=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'Holds the processed contents array',
											   ),
						'var_references'	=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'Holds the names for all variables that are "references" to values in other DAs',
											   ),
						'print'				=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'boolean',
												'default'		=> TRUE,
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
												'description'	=> 'Whether this DA should print output from its AREA tag or just initialise itself for later reference',
											   ),
						'cache'				=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
												'description'	=> 'Whether this DA should cache its output',
											   ),
					  );

	}//end constructor


}//end class
?>
