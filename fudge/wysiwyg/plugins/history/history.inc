<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: history.inc,v 1.4 2003/09/26 05:26:37 brobertson Exp $
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