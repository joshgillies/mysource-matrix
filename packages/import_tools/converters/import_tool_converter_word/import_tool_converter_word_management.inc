<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: import_tool_converter_word_management.inc,v 1.4.12.1 2009/05/08 00:57:10 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Import_Tool_Converter_Word_Management
*
* @see Asset_Management
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.4.12.1 $
* @package MySource_Matrix_Packages
* @subpackage import_tools
*/
class Import_Tool_Converter_Word_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Import_Tool_Converter_Word_Management(&$pm)
	{
		$this->Asset_Management($pm);

	$this->vars['name']['default'] = 'Word Import Tool Converter';
	$this->vars['valid_file_types']['default'] = Array(
													'doc',
													'dot',
													'docx',
													'docm',
													'dotx',
													'dotm',
												 );
	$this->vars['create_asset_type']['default'] = 'Word_Doc';

	}//end constructor


}//end class
?>
