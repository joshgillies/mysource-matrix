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
* $Id: data_source_graph_management.inc,v 1.1.1.1 2005/09/28 05:35:28 pgannavarapu Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Data_Source_Graph_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.1.1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Data_Source_Graph_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Data_Source_Graph_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'			=> Array(
											'added'			=> '0.1',
											'type'			=> 'text',
											'default'		=> '',
											'is_admin'		=> false,
										   ),
						'alt'			=> Array(
											'added'			=> '0.1',
											'type'			=> 'text',
											'default'		=> '',
											'description'	=> 'The text to be used in the Alt attribute of the IMG tag',
											'is_admin'		=> false,
										   ),
						'caption'		=> Array(
											'added'			=> '0.1',
											'type'			=> 'text',
											'default'		=> '',
											'description'	=> 'Some text to be associated with the image and can be accessed and printed out separately if required',
											'is_admin'		=> false,
										   ),
						'width'			=> Array(
											'added'			=> '0.1',
											'type'			=> 'int',
											'default'		=> 0,
											'description'	=> 'Width of the image (generated from the image file)',
											'is_admin'		=> false,
										   ),
						'height'		=> Array(
											'added'			=> '0.1',
											'type'			=> 'int',
											'default'		=> 0,
											'description'	=> 'Height of the image (generated from the image file)',
											'is_admin'		=> false,
										   ),
						'graph_details'	=> Array(
											'added'		=> '0.1',
											'type'		=> 'serialise',
											'default'	=> Array(),
										   ),
					  );


	}//end constructor


}//end class
?>
