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
* $Id: quotation.inc,v 1.3.18.1 2010/10/05 05:53:50 cupreti Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - Quotation
*
* Purpose
*     A WYSIWYG plugin to allow the user to insert "Blockquote"
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
	* Prints a generic insert "Blockquote" function 
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
					
					// If the selected text is wrapped by "blockquote", toggle it off
					if (el && el.tagName == "BLOCKQUOTE") {
						newNode = document.createTextNode(el.innerHTML);
						el.parentNode.replaceChild(newNode, el);
						
						inside_blockquote = true;
						break;
					}
				}

				// Remove all the "blockquotes" in select area,
				// so that whole selected block can be wrapped by one
				var pattern = /(<blockquote>.*?<\/blockquote>)|(<blockquote \/>)/i;
				if (pattern.test(this.getSelectedHTML())) {
					var nodeContent;
					for (var i = parent.childNodes.length; --i >= 0;) {
						if (parent.childNodes[i].tagName == "BLOCKQUOTE") {
							nodeContent = parent.childNodes[i].innerHTML;
						
							newNode = document.createTextNode(nodeContent);
							parent.replaceChild(newNode, parent.childNodes[i]);
						}//end if
					}//end for
				}//end if hasBlockquotes

				if (!inside_blockquote) {

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
							alert("<BLOCKQUOTE> tags cannot be added to partial text within a paragraph.");
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
