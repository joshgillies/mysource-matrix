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
* $Id: citation.inc,v 1.1 2006/02/28 06:06:13 skim Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - Citation
*
* Purpose
*     A WYSIWYG plugin to allow the user to create
*     and manipulate tables
*
* @author  Scott Kim <skim@squiz.net>
* @author  Darren McKee <dmckee@squiz.net>
* @version $Version$ - 1.0
* @package Fudge
* @subpackage wysiwyg
*/
class Citation extends WYSIWYG_Plugin
{


	/**
	* Constructor
	* Adds buttons needed for table editing
	*
	* @returns object Table_Editing
	* @access  public
	*/
	function Citation(&$wysiwyg)
	{
		wysiwyg_plugin::wysiwyg_plugin($wysiwyg);
		//$this->_add_button('citation','Citation','Insert Citation','false','35');
		$this->_add_button('cite','Cite','Cite','false','35');

	}//end constructor


	/**
	* Highlight the selected text or change the font colour
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_exec_command()
	{
		?>
		case "cite":
			if (!this._doc.queryCommandState("cite")) {
				this.surroundHTML("<cite>", "</cite>");
			} else {
				this._doc.execCommand(cmdID, UI, param);
				alert("hello2 cmdID:" + cmdID + " UI:" + UI + " param:" + param);
			}
		break;
		<?php

	}//end print_plugin_exec_command()


	/**
	* Call the appropriate function when a button is clicked
	*
	* @returns void
	* @access  public
	*/
/*	function print_plugin_button_click()
	{
		?>
		case "citation":
			alert('citation');
			this.surroundHTML('<cite>', '</cite>');
			alert('citation');
		break;
		<?php

	}//end print_plugin_button_click()
*/


}//end class

?>