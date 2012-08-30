<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: format_text.inc,v 1.13 2012/08/30 00:56:51 ewang Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - Format Text
*
* Purpose
*     A WYSIWYG plugin to make select bold, itlaic, underline
*     and strikethrough. Also has remove all formatting function
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.13 $
* @package Fudge
* @subpackage wysiwyg
*/
class Format_Text extends WYSIWYG_Plugin
{


	/**
	* Constructor
	* Adds the buttons for this plugins functions
	*
	* @returns object Format_Text
	* @access  public
	*/
	function format_text()
	{
		$this->_add_button('bold','Bold','Bold','false','14');
		$this->_add_button('italic','Italic','Italic','false','15');
		$this->_add_button('underline','Underline','Underline','false','16');
		$this->_add_button('strikethrough','StrikeThrough','StrikeThrough','false','17');
	}


	/**
	* Prints shortcuts for the buttons
	*
	* All of these except strikethrough are supported nativly in IE
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_shortcuts()
	{
		?>
		case 'b':	// KEY bold
			(!HTMLArea.is_ie) && (cmd = "bold");
		break;
		case 'i':	// KEY italic
			(!HTMLArea.is_ie) && (cmd = "italic");
		break;
		case 'u':	// KEY underline
			(!HTMLArea.is_ie) && (cmd = "underline");
		break;
		case 's':	// KEY strikethrough
			cmd = "strikethrough";
		break;
		<?php

	}//end print_plugin_shortcuts()


	/**
	* Highlight the selected text or change the font colour
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_exec_command()
	{
		?>
		case "bold":
			if (!this._doc.queryCommandState("bold")) {
				this.surroundHTML("<b>", "</b>");
			} else {
				this._doc.execCommand(cmdID, UI, param);
			}
		break;
		case "italic":
			if (!this._doc.queryCommandState("italic")) {
				this.surroundHTML("<i>", "</i>");
			} else {
				this._doc.execCommand(cmdID, UI, param);
			}
		break;
		case "underline":
			if (!this._doc.queryCommandState("underline")) {
				this.surroundHTML("<u>", "</u>");
			} else {
				this._doc.execCommand(cmdID, UI, param);
			}
		break;
		case "strikethrough":
			if (!this._doc.queryCommandState("strikethrough")) {
				this.surroundHTML("<strike>", "</strike>");
			} else {
				this._doc.execCommand(cmdID, UI, param);
			}
		break;
		<?php

	}//end print_plugin_exec_command()


}//end class

?>