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
* $Id: history.inc,v 1.5 2003/11/18 15:42:12 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - History Functions
*
* Purpose
*     A WYSIWYG plugin to provide undo and redo functions.
*     The commands are supported nativly by the editors for
*     all supported browsers - so only need to add the buttons
*     and not provide the functionality
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Fudge
* @subpackage wysiwyg
*/
class history extends wysiwyg_plugin {

	/**
	* Constructor
	* Adds undo and redo buttons
	*
	* @returns object History
	* @access  public
	*/
	function history() {
		$this->_add_button('undo','Undo','Undo','false','12');
		$this->_add_button('redo','Redo','Redo','false','13');
	}

}

?>