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
* $Id: help.inc,v 1.6 2003/11/26 00:51:20 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - Help
*
* Purpose
*     A WYSIWYG plugin to provide help to the end user
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Fudge
* @subpackage wysiwyg
*/
class help extends wysiwyg_plugin {

	/**
	* Constructor
	* Adds a help button to the plugin
	*
	* @returns object Help
	* @access  public
	*/
	function help() {
		$this->_add_button('help','showhelp','Help using editor','true','1');
	}


	/**
	* Help is not implemented at this time
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_button_click() {
		?>
		case "showhelp":
			alert("Help not implemented");
		break;
		<?php
	}
}

?>