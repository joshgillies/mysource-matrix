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
* $Id: indenting.inc,v 1.9 2006/12/06 05:11:09 bcaldwell Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - Indenting
*
* Purpose
*     A WYSIWYG plugin to increase and decrease the
*     indent of the select elements
*
* @author  Greg Sherwood <greg@squiz.net>
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.9 $
* @package Fudge
* @subpackage wysiwyg
*/
class indenting extends wysiwyg_plugin
{

	/**
	* Constructor
	* Adds indent and outdent buttons to the plugin
	*
	* @returns object Indenting
	* @access  public
	*/
	function indenting()
	{
		$this->_add_button('outdent','Outdent','Decrease Indent','false','28');
		$this->_add_button('indent','Indent','Increase Indent','false','29');
	}


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
			HTMLArea.prototype._changeIndent = function(direction) {

				var editor = this; // for nested functions

				var sel = this._getSelection();
				var range = this._createRange(sel);
				var parent = this.getParentElement();

				if (HTMLArea.is_gecko) {

					if (range == null || range == "") {

						while (parent.tagName != "P") {
							parent = parent.parentNode
						}

						// No text is selected
						if (this._paragraphFilter(parent)) {
							this._setLeftMargin(direction, parent);
						}

					} else {

						var new_node = range.startContainer.parentNode;
						while (new_node.tagName != "P") {
							new_node = new_node.parentNode
						}
						var tree_walker = this._doc.createTreeWalker(
											new_node.parentNode,
											NodeFilter.SHOW_ELEMENT,
											this._paragraphFilter,
											false);
						el = tree_walker.firstChild();

						do {

							if (el.innerHTML == new_node.innerHTML) {
								start_node = el;
								start_node_found = true;
								break;
							}
							el = tree_walker.nextSibling();
						} while (el != null)

						var attrs;
						if (start_node_found) {

							// Now start traverse the sibling so that we can adjust the indentation
							var current_indent;
							var pattern = /(\d)+px/g;
							var new_px;
							do {
								this._setLeftMargin(direction, el);
								var new_node = range.endContainer.parentNode;
								while (new_node.tagName != "P") {
									new_node = new_node.parentNode
								}
								if (el.innerHTML == new_node.innerHTML) break;
							} while ((el = tree_walker.nextSibling()) != null)
						}
					}

				} else if (HTMLArea.is_ie) {

					if (range.text == null || range.text == "") {

						while (parent.tagName != "P") {
							parent = parent.parentNode
						}

						// No text is selected
						if (this._paragraphFilter(parent)) {
							this._setLeftMargin(direction, parent);
						}

					} else {
						// IE 6 does not support TreeWalker interface yet!
						// Things should be done differently...
						if (range.text == null) {
							// No text is selected
							if (this._paragraphFilter(parent) == true) {
								this._setLeftMargin(direction, parent);
							}
						} else {
							// Create paragraph map
							var selected_parent = range.parentElement();
							var child_nodes = selected_parent.children;
							var text = "";
							var offset = new Array();
							for (var i = 0; i < child_nodes.length; i++) {
								// TODO: _paragraphFilter?????
								if (child_nodes.item(i).tagName == "P") {
									text = text + child_nodes.item(i).innerText.replace(/\s/g,"");
									offset.push(text.length);
								}
							}

							var start_index = text.indexOf(range.text.replace(/\s/g,""));
							var end_index = text.indexOf(range.text.replace(/\s/g,"")) + range.text.replace(/\s/g,"").length;

							// Now we have the tools for cooking
							for (var i = 0; i < child_nodes.length; i++) {
								if (child_nodes.item(i).tagName == "P" && offset[i] > start_index) {

									if (offset[i] < end_index) {
										this._setLeftMargin(direction, child_nodes.item(i));
									} else if ((i+1) == child_nodes.length) {
										if (offset[i] >= end_index) {
											this._setLeftMargin(direction, child_nodes.item(i));
										}
									} else if ((i+1) < child_nodes.length) {
										if (offset[i+1] >= end_index) {
											this._setLeftMargin(direction, child_nodes.item(i));
											break;
										}
									}
								}
							}
						}
					}
				}//end if-else
			};

			// Helper function to filter the wanted element type
			// during tree traversing
			HTMLArea.prototype._paragraphFilter = function(node) {
				if (node.tagName == 'P') return true;
				else return false;
			}

			// Set the left margin value of the element's style
			HTMLArea.prototype._setLeftMargin = function(direction, el) {
				if (el.style.marginLeft == "" || el.style.marginLeft == "0pt"|| el.style.marginLeft == "0px") {
					if (direction == "right") {
						el.style.marginLeft = "40px";
					}
				} else {
					var new_px;
					var result = el.style.marginLeft.match(/(\d)+px/g);
					var current_indent = parseInt(result[0]);
					if (direction == "right") {
						new_px = current_indent + 40;
					} else if (direction == "left") {
						if (current_indent > 40) {
							new_px = current_indent - 40;
						} else if (current_indent == 40) {
							new_px = 0;
						}
					}
					el.style.marginLeft = new_px.toString();
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
		case "outdent":
			this._changeIndent("left");
		break;
		case "indent":
			this._changeIndent("right");
		break;
		<?php

	}//end print_plugin_button_click()

}

?>