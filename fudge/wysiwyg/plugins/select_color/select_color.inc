<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: select_color.inc,v 1.13.16.1 2012/02/16 04:04:46 ewang Exp $
*
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
* @version $Revision: 1.13.16.1 $
* @package Fudge
* @subpackage wysiwyg
*/
class Select_Color extends WYSIWYG_Plugin
{

	/**
	* Constructor
	* Adds the fore and background colour buttons to the plugin
	*
	* @returns object Select_Color
	* @access  public
	*/
	function select_color()
	{
		$this->_add_button('forecolor','foreColor','Text Color','false','20');
		$this->_add_button('backcolor','BackColor','Background Color','false','21');

	}//end constructor


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
			this._popupDialog("selectColor", "<?php echo $this->get_popup_href('select_color.html')?>", 248, 190, true, function(color) {
			// color span can not be wrapped on block elements.
			var blockRegex = /<(address|blockquote|body|center|dir|div|dl|fieldset|form|h[1-6]|hr|isindex|menu|noframes|noscript|ol|p|pre|table|ul|dd|dt|frameset|li|tbody|td|tfoot|th|thead|tr|html)/i;
			if(blockRegex.test(editor.getSelectedHTML())) {
			    alert("Font color can not be applied to block elements");
			    return;
			}
				if (color) {
					var current_span = editor.getClosest("span");
					if (current_span) {
						editor.selectNodeContents(current_span);
						if (cmd.toLowerCase() == "forecolor") {
							current_span.style.color = color;
						} else {
							current_span.style.backgroundColor = color;
						}
					} else {
						// if the value is not the no_style default, add the span tags
						// to apply the stlye to the selected text - otherwise we ignore it
						var colorType = "color";
						if (cmd.toLowerCase() == "backcolor") { colorType = "background-color"; }
						editor.surroundHTML("<span style=\"" + colorType + ":#" + color + ";\">", "</span>");
					}
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
