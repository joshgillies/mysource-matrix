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
* $Id: indenting.inc,v 1.11 2012/08/30 00:56:52 ewang Exp $
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
* @version $Revision: 1.11 $
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
		<script type="text/javascript">
			// Called when the user clicks on "Insert Blockquote" button
			HTMLArea.prototype._changeIndent = function(direction) {

				if (HTMLArea.is_ie) {

					// IE - we need to override some stuff
					var blockElts = "|applet|div|embed|fieldset|form|h1|h2|h3|h4|h5|h6|iframe|ilayer|img|object|p|pre|quote|table|textarea|ul|ol|";
					var sel = this._getSelection();
					var range = this._createRange(sel);
					range.expand('character');
					range.select();
					if (direction == 'right') {

						// First get the browser to do its thing
						this._execCommand('indent', false, '');

						// Second, remove the blockquote element if necessary
						var sel = this._getSelection();
						var range = this._createRange(sel);
						range.expand('character');
						var parent = range.parentElement();
						if (parent.tagName.toLowerCase() == 'p') {
							parent = parent.parentNode;
						}
						if (parent.tagName == 'BLOCKQUOTE') {
							var subElt = parent.firstChild;
							while (subElt != null) {
								newElt = subElt.nextSibling;
								if (subElt.tagName) {
									if (blockElts.indexOf('|'+subElt.tagName.toLowerCase()+'|') != -1) {
										this._setLeftMargin(direction, subElt);
									}
									parent.removeChild(subElt);
									parent.parentNode.insertBefore(subElt, parent);
								}
								subElt = newElt;
							}
							parent.parentNode.removeChild(parent);
						}
					} else {

						// First, try to trim off left margin ourselves
						var tRange =  this._doc.selection.createRange()
						if (tRange.parentElement().tagName.toLowerCase() == 'p') {
							tRange.moveToElementText(tRange.parentElement());
						}
						tRange.moveStart('character', -1);
						tRange.moveEnd('character', 1);
						var parent = this.getParentElement();
						if (parent.tagName.toLowerCase() == 'p') {
							parent = parent.parentNode;
						}
						var subElt = parent.firstChild;
						var success = false;
						var foundFirst = false;
						while (subElt != null) {
							var nextElt = subElt.nextSibling;
							if (subElt.tagName) {
								var nodeRange = tRange.duplicate();
								nodeRange.moveToElementText(subElt);
								nodeRange.expand('character');
								if (tRange.inRange(nodeRange)) {
									if (blockElts.indexOf('|'+subElt.tagName.toLowerCase()+'|') != -1) {
										success = this._setLeftMargin(direction, subElt) || success;
									}
								}
							}
							subElt = nextElt;
						}

						// If we didn't catch anything, send it to the browser for handling
						if (!success) {
							this._doc.execCommand('outdent', false, '');
						}
					}

				} else {

					// NOT IE - browser default behaviour is fine
					this._doc.execCommand((direction == 'left') ? 'outdent' : 'indent', false, '');

				}
			}

			// Set the left margin value of the element's style
			HTMLArea.prototype._setLeftMargin = function(direction, el) {
				if (el.style.marginLeft == "" || el.style.marginLeft == "0pt"|| el.style.marginLeft == "0px") {
					if (direction == "right") {
						el.style.marginLeft = "40px";
					} else {
						return false; // could not outdent
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
						} else {
							return false; // could not outdent
						}
					}
					el.style.marginLeft = new_px.toString();
				}
				return true;
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
