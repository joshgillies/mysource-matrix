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
* $Id: indenting.inc,v 1.5 2003/11/18 15:42:12 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - Indenting
*
* Purpose
*     A WYSIWYG plugin to increase and decrease the
*     indent of the select elements
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Fudge
* @subpackage wysiwyg
*/
class indenting extends wysiwyg_plugin {

	/**
	* Constructor
	* Adds indent and outdent buttons to the plugin
	*
	* @returns object Indenting
	* @access  public
	*/
	function indenting() {
		$this->_add_button('outdent','Outdent','Decrease Indent','false','28');
		$this->_add_button('indent','Indent','Increase Indent','false','29');
	}

}

?>