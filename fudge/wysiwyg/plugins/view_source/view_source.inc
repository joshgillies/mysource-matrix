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
* $Id: view_source.inc,v 1.7 2003/11/18 15:42:13 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - View Source
*
* Purpose
*     A WYSIWYG plugin to allow the user to
*     switch between HTML source and editing views
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Fudge
* @subpackage wysiwyg
*/
class View_Source extends Wysiwyg_Plugin {

	/**
	* Constructor
	* Adds the view source button (that switches the mode)
	*
	* @returns object View_Source
	* @access  public
	*/
	function View_Source()
	{
		$this->_add_button('htmlmode','HtmlMode','Toggle HTML Source','true','3');

	}//end constructor


	/**
	* On click, switch the mode of the editor based on current mode
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_button_click()
	{
		?>
		case "htmlmode":
			this.setMode(this._editMode != "textmode" ? "textmode" : "wysiwyg");
		break;
		<?php

	}//end print_plugin_button_click()


	/**
	* Show the view source button as active
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_update_toolbar()
	{
		?>
		case "htmlmode":
			btn.state("active", text);
		break;
		<?php

	}//end print_plugin_update_toolbar()

}//end class

?>