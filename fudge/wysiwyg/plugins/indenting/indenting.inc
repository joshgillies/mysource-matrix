<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: indenting.inc,v 1.4 2003/09/26 05:26:37 brobertson Exp $
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