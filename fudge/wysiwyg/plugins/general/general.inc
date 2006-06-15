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
* $Id: general.inc,v 1.8 2006/06/15 04:06:16 skim Exp $
*
*/


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
* @version $Revision: 1.8 $
* @package Fudge
* @subpackage wysiwyg
*/
class general extends wysiwyg_plugin
{


	/**
	* Constructor
	* Adds buttons for these general functions
	*
	* @return object General
	* @access public
	*/
	function general()
	{
		wysiwyg_plugin::wysiwyg_plugin($wysiwyg);
		$this->_add_button('selectall','SelectAll','Select All','false','6');
		$this->_add_button('inserthorizontalrule','InsertHorizontalRule','Horizontal Rule','false','31');

	}//end constructor


	/**
	* Prints a generic insert image function that can be used by any editor
	* A URL must be entered for the image you want to insert. Alt text must
	* also be entered before the image will be inserted.
	*
	* @return void
	* @access public
	*/
	function paint_generic()
	{
		?>
		<script type="text/javascript" language="Javascript">
			HTMLArea.prototype._insertHorizontalRule = function() {
				this.surroundHTML('', '<hr>');
			}
		</script>
		<?php

	}//end paint_generic()


	/**
	* Prints any JS the plugin wants executed before the editable form element has its way
	*
	* In the case of gecko based browser, we insert <hr> tag manually to prevent the inlined
	* style being printed.
	*
	* @return void
	* @access public
	*/
	function print_plugin_exec_command()
	{
		?>
		case "inserthorizontalrule":
			if (HTMLArea.is_gecko) {
				this._insertHorizontalRule();
			} else if (HTMLArea.is_ie) {
				this._doc.execCommand('InsertHorizontalRule');
			}
		break;
		<?php

	}//end print_plugin_exec_command()


	/**
	* Prints the keyboard shortcut for the Select All function
	*
	* @return void
	* @access public
	*/
	function print_plugin_shortcuts()
	{
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

	}//end print_plugin_shortcuts()


}//end class

?>