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
* $Id: text_indicator.inc,v 1.6 2003/11/18 15:42:13 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - Text Indicator
*
* Purpose
*     A WYSIWYG plugin to show a sample character in the
*     font style/colours etc of the selected text.
*     Pure decoration - no real functionality.
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Fudge
* @subpackage wysiwyg
*/
class text_indicator extends wysiwyg_plugin {

	/**
	* Constructor
	* Adds a blank button which will become the text indicator
	*
	* @returns object Insert_Link
	* @access  public
	*/
	function text_indicator() {
		$this->_add_button('textindicator','','','','');
	}


	/**
	* When the toolbar is updated, change the font style/colour etc of the indicator
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_update_toolbar() {
		?>
		case "textindicator":
			if (!text) {
				try {
					with (btn.element.style) {
						backgroundColor = HTMLArea._makeColor(doc.queryCommandValue("backcolor"));
						color = HTMLArea._makeColor(doc.queryCommandValue("forecolor"));
						fontFamily = doc.queryCommandValue("fontname");
						fontWeight = doc.queryCommandState("bold") ? "bold" : "normal";
						fontStyle = doc.queryCommandState("italic") ? "italic" : "normal";
					}
				} catch (e) {
					with (btn.element.style) {
						backgroundColor = "#FFFFFF";
						color = "#000000";
					}
				}
			}
		break;
		<?php
	}


	/**
	* Prints the text indicator on the toolbar instead of standard button
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_button_type() {
		?>
		case "textindicator":
			el = document.createElement("span");
			el.appendChild(document.createTextNode("A"));
			el.className = "htmlarea-indicator";
			el.style.position = "relative";
			el.style.top = -7;
			el.title = HTMLArea.I18N.tooltips.textindicator;
			var obj = {
				name: txt,     // the button name (i.e. 'bold')
				element: el,   // the UI element (DIV)
				enabled: true, // is it enabled?
				active: false, // is it pressed?
				text: false,   // enabled in text mode?
				cmd: "textindicator", // the command ID
				state: setButtonStatus // for changing state
			};
			tb_objects[txt] = obj;
		break;
		<?php
	}
}

?>