<?php

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
* @version $Version$ - 1.0
* @package Fudge::wysiwyg
*/
class format_text extends wysiwyg_plugin {


	/**
	* Constructor
	* Adds the buttons for this plugins functions
	*
	* @returns object Format_Text
	* @access  public
	*/
	function format_text() {
		$this->_add_button('bold','Bold','Bold','false','14');
		$this->_add_button('italic','Italic','Italic','false','15');
		$this->_add_button('underline','Underline','Underline','false','16');
		$this->_add_button('strikethrough','StrikeThrough','StrikeThrough','false','17');
		$this->_add_button('removeformat','RemoveFormat','Remove Formatting','false','11');
	}


	/**
	* Prints shortcuts for the buttons.
	* All of these except strikethrough are supported nativly in IE
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_shortcuts() {
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
	}

}

?>