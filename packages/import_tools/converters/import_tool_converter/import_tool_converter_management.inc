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
* $Id: import_tool_converter_management.inc,v 1.1.1.1 2006/02/20 22:36:45 dmckee Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Import_Tool_Converter_Management
*
* @see Asset_Management
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.1.1.1 $
* @package MySource_Matrix_Packages
* @subpackage import_tools
*/
class Import_Tool_Converter_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Import_Tool_Converter_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'				=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'valid_file_types'	=> Array(
												'added'		=> '0.1',
												'type'		=> 'serialise',
												'default'	=> Array(),
											   ),
						'upload_directory'	=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'create_asset_type'	=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> 'file',
											   ),
					  );

	}//end constructor


}//end class
?>
