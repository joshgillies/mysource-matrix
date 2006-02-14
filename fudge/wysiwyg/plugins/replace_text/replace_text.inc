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
* $Id: replace_text.inc,v 1.2.2.1 2006/02/14 23:48:50 skim Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - Format Text
*
* Purpose
*		A WYSIWYG plugin to separate text replace
*		from other formatting functions
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.2.2.1 $
* @package Fudge
* @subpackage wysiwyg
*/
class Replace_Text extends WYSIWYG_Plugin
{


	/**
	* Constructor
	* Adds the buttons for this plugins functions
	*
	* @returns object Format_Text
	* @access  public
	*/
	function replace_text()
	{
		$this->_add_button('replacetext','ReplaceText','Replace Text','false','11');
	}


}//end class

?>
