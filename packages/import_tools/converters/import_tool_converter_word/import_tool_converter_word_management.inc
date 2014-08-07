<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: import_tool_converter_word_management.inc,v 1.7 2012/08/30 00:58:50 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Import_Tool_Converter_Word_Management
*
* @see Asset_Management
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.7 $
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
