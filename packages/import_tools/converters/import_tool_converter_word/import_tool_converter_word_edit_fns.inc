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
* $Id: import_tool_converter_word_edit_fns.inc,v 1.4.16.1 2010/04/13 23:31:10 mhaidar Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/import_tools/converters/import_tool_converter/import_tool_converter_edit_fns.inc';

/**
* Import_Tool_Converter_Word_Edit_Fns
*
* Purpose
*
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.4.16.1 $
* @package MySource_Matrix_Packages
* @subpackage import_tools
*/
class Import_Tool_Converter_Word_Edit_Fns extends Import_Tool_Converter_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Import_Tool_Converter_Word_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
		unset($this->static_screens['settings']);
  	    unset($this->static_screens['preview']);
  	    unset($this->static_screens['lookupValues']);
  	    unset($this->static_screens['metadata']);
  	    unset($this->static_screens['metadataSchemas']);
  	    unset($this->static_screens['workflow']);
  	    unset($this->static_screens['tagging']);
  	    unset($this->static_screens['dependants']);
  	    unset($this->static_screens['linking']);
  	    unset($this->static_screens['roles']);
  	    unset($this->static_screens['permissions']);
  	    unset($this->static_screens['layouts']);

	}//end constructor


}//end class
?>
