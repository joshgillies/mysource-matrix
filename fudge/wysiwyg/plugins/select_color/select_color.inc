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
* $Id: select_color.inc,v 1.8 2003/11/18 15:42:13 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - Colour Functions
*
* Purpose
*     A WYSIWYG plugin to allow the user to
*     set text and background colours
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Fudge
* @subpackage wysiwyg
*/
class select_color extends wysiwyg_plugin {

	/**
	* Constructor
	* Adds the fore and background colour buttons to the plugin
	*
	* @returns object Select_Color
	* @access  public
	*/
	function select_color()
	{
		$this->_add_button('forecolor','ForeColor','Font Color','false','20');
		$this->_add_button('backcolor','BackColor','Background Color','false','21');

	}//end select_color()


	/**
	* Open the popup colour selector to get the colour from the user
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_button_click()
	{
		?>
		case "forecolor":
		case "backcolor":
			this._popupDialog("selectColor", "<?php echo $this->get_popup_href('select_color.html')?>", 238, 182, true, function(color) {
				if (color) {
					editor._execCommand(cmd, false, "#" + color);
				}
			}, HTMLArea._colorToRgb(this._doc.queryCommandValue(btn[0])));
		break;
		<?php

	}//end print_plugin_button_click()

	
	/**
	* Highlight the selected text or change the font colour
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_exec_command()
	{
		?>
		case "backcolor":
			if (HTMLArea.is_ie) {
				this._doc.execCommand(cmdID, UI, param);
			} else {
				this._doc.execCommand("hilitecolor", UI, param);
			}
		break;
		<?php
	}//end print_plugin_exec_command()

}//end class

?>