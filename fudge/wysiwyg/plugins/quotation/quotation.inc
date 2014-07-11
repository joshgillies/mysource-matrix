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
* $Id: quotation.inc,v 1.8 2012/08/30 00:56:52 ewang Exp $
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
* @version $Revision: 1.8 $
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

				var selection =  this.getSelectedHTML();
				var parent = this.getParentElement();

				// Is the selected text is inside blockquote?
				var inside_blockquote = false;
				var ancestors = this.getAllAncestors();
				for (var i = ancestors.length; --i >= 0;) {
					var el = ancestors[i];
					
					// If the selected text is wrapped by "blockquote", toggle it off
					if (el && el.tagName == "BLOCKQUOTE") {
						var pattern = new RegExp("<blockquote>\r?\n?"+el.innerHTML+"\r?\n?<\/blockquote>", "i");
						el.parentNode.innerHTML = el.parentNode.innerHTML.replace(pattern, el.innerHTML);
						inside_blockquote = true;
						break;
					}
				}

				// Remove all the "blockquotes" in select area
				var pattern = /(<blockquote>\r?\n?.*?\r?\n?<\/blockquote>)/i;
				if (pattern.test(selection)) {
					inside_blockquote = true;
					var nodeContent;
					for (var i = parent.childNodes.length; --i >= 0;) {
						if (parent.childNodes[i].tagName == "BLOCKQUOTE") {
							nodeContent = parent.childNodes[i].innerHTML;
							
							pattern = new RegExp("<blockquote>\r?\n?"+nodeContent+"\r?\n?<\/blockquote>", "i");
							parent.childNodes[i].parentNode.innerHTML = parent.childNodes[i].parentNode.innerHTML.replace(pattern, nodeContent);
						}//end if
					}//end for
				}//end if hasBlockquotes

				if (!inside_blockquote) {

					if (parent.tagName == "P") {
						var parent_html = parent.innerHTML;
						parent_html = parent_html.replace(/^\s*/, "").replace(/\s*$/, "");//trim whitespace

						// Remove surrounding paragraph tag from selected HTML for comparison
						if (HTMLArea.is_ie) {
							// Remove newlines from paragraph HTML so we can perform a meaningful comparison
							e = '\r\n';
							re = new RegExp(e, "i");
							selection = selection.replace(re, "");
							// Remove paragraph start and end
							var e = '^<p[^>]*>';
							var re = new RegExp(e, "i");
							selection = selection.replace(re, "");
							e = '</p>$';
							re = new RegExp(e, "i");
							selection = selection.replace(re, "");
						}
						selection = selection.replace(/^\s*/, "").replace(/\s*$/, "");//trim whitespace
						
						if (parent_html == selection) {
							// the whole paragraph is selected, now wrap it with
							// blockquote tags
							var blockquote = this._doc.createElement("blockquote");
							var paragraph = this._doc.createElement("p");
							paragraph.innerHTML = selection;
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
