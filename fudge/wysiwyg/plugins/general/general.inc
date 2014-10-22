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
* $Id: general.inc,v 1.10 2012/08/30 00:56:51 ewang Exp $
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
* @version $Revision: 1.10 $
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
		$this->_add_button('selectall','SelectAll',translate('Select All'),'false','6');
		$this->_add_button('inserthorizontalrule','InsertHorizontalRule',translate('Horizontal Rule'),'false','31');

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