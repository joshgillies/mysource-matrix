<?php

global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - General
*
* Purpose
*     A WYSIWYG plugin to provide general functions
*     to the editor (somewhat unrelated functionality)
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Fudge
* @subpackage wysiwyg
*/
class general extends wysiwyg_plugin {

	/**
	* Constructor
	* Adds buttons for these general functions
	*
	* @returns object General
	* @access  public
	*/
	function general() {
		$this->_add_button('selectall','SelectAll','Select All','false','6');
		$this->_add_button('horizontalrule','InsertHorizontalRule','Horizontal Rule','false','31');
	}

	
	/**
	* Prints the keyboard shortcut for the Select All function
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_shortcuts() {
		?>
		case 'a':
			if (!HTMLArea.is_ie) {
				// KEY select all
				sel = this._getSelection();
				sel.removeAllRanges();
				range = this._createRange();
				range.selectNodeContents(this._doc.body);
				sel.addRange(range);
				HTMLArea._stopEvent(ev);
			}
		break;
		<?php
	}

}

?>