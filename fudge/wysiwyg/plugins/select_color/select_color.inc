<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: select_color.inc,v 1.7 2003/09/26 05:26:38 brobertson Exp $
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