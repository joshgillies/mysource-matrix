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
* $Id: quotation.inc,v 1.1 2006/02/28 06:13:55 skim Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - Quotation
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
class Quotation extends WYSIWYG_Plugin
{


	/**
	* Constructor
	* Adds buttons needed for table editing
	*
	* @returns object Table_Editing
	* @access  public
	*/
	function Quotation(&$wysiwyg)
	{
		wysiwyg_plugin::wysiwyg_plugin($wysiwyg);
		$this->_add_button('quotation','Quotation','Quotation','false','60');

	}//end constructor


	/**
	* Prints a generic insert image function that can be used by any editor
	* A URL must be entered for the image you want to insert. Alt text must
	* also be entered before the image will be inserted.
	*
	* @returns void
	* @access  public
	*/
	function paint_generic()
	{
		?>
		<script type="text/javascript" language="Javascript">
			// Called when the user clicks on "Insert Blockquote" button
			HTMLArea.prototype._insertBlockquote = function() {
				var editor = this; // for nested functions

				var sel = this._getSelection();
				var range = this._createRange(sel);
				var parent = this.getParentElement();

				// Is the selected text is inside blockquote?
				var inside_blockquote = false;
				var ancestors = this.getAllAncestors();
				for (var i = ancestors.length; --i >= 0;) {
					var el = ancestors[i];
					if (el && el.tagName == "BLOCKQUOTE") {
						alert("The selected text is already in the blockquote");
						inside_blockquote = true;
						break;
					}
				}

				// Does the selected text contain any blockquote?
				var include_blockquote = false;
				if (parent.firstChild.tagName == "BLOCKQUOTE") {
					alert("The selected text includes the blockquote tags");
					include_blockquote = true;
				} else {
					var pattern = /<blockquote>.*<\/blockquote>/i;
					for (var i = parent.childNodes.length; --i >= 0;) {
						if (parent.childNodes[i].tagName == "BLOCKQUOTE" || pattern.test(parent.childNodes[i].innerHTML)) {
							alert("The selected text includes the blockquote tags");
							include_blockquote = true;
						}
					}
				}

				if (!inside_blockquote && !include_blockquote) {

					if (parent.tagName == "P") {

						if (HTMLArea.is_gecko) {
							var content = range;
						} else if (HTMLArea.is_ie) {
							var content = range.text;
						}

						if (parent.innerHTML == content) {
							// the whole paragraph is selected, now wrap it with
							// blockquote tags
							var blockquote = this._doc.createElement("blockquote");
							var paragraph = this._doc.createElement("p");
							var text = this._doc.createTextNode(content);
							paragraph.appendChild(text);
							blockquote.appendChild(paragraph);

							var parent_parent = parent.parentNode;
							parent_parent.insertBefore(blockquote, parent);
							parent_parent.removeChild(parent);
						} else {
							alert("The partial text within paragraph can be blockquoted.");
						}

					} else {
						// the selected text is not in p tag, just wrap it with blockquote
						this.surroundHTML('<blockquote>', '</blockquote>');
					}
				}

			};
		</script>
		<?php
	}


	/**
	* Call the appropriate function when a button is clicked
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_button_click()
	{
		?>
		case "quotation":
			this._insertBlockquote();
		break;
		<?php

	}//end print_plugin_button_click()


}//end class

?>