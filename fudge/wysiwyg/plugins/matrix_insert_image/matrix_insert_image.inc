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
* $Id: matrix_insert_image.inc,v 1.25.2.2 2006/09/25 01:39:24 skim Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - Matrix Insert Image
*
* Purpose
*     A WYSIWYG plugin to allow the user to insert an image
*     in the editable document using an image asset from MySoure Matrix
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.25.2.2 $
* @package MySource_Matrix
* @subpackage wysiwyg
*/
class matrix_insert_image extends wysiwyg_plugin
{


	/**
	* Constructor
	* Adds the insert image button to this plugin
	*
	* @access public
	*/
	function matrix_insert_image(&$wysiwyg)
	{
		wysiwyg_plugin::wysiwyg_plugin($wysiwyg);
		$this->_add_button('matrixinsertimage','MatrixInsertImage','Insert Image','false','34');

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
			// Called when the user clicks on "InsertImage" button
			HTMLArea.prototype._matrixinsertImage = function() {
				var editor = this; // for nested functions

				var url      = "";
				var align    = "";
				var width    = "";
				var height   = "";
				var alt      = "";
				var hspace   = "";
				var vspace   = "";
				var border   = "0";
				var imageid  = "0";
				var is_image = false;

				var sel = this._getSelection();
				var range = this._createRange(sel);

				// loop through and try and find a selected image tag
				if (HTMLArea.is_gecko) {
					var fragment = this._doc.createDocumentFragment();
					var div = this._doc.createElement("div");
					div.innerHTML = this.getSelectedHTML();
					while (div.firstChild) {
						if (div.firstChild.tagName == "IMG") {
							is_image = true;
							var image_tag = div.firstChild;
							break;
						}
						// the following call also removes the node from div
						fragment.appendChild(div.firstChild);
					}
				} else if (HTMLArea.is_ie) {
					if(range.length) {
						if (range.item(0).tagName == "IMG") {
							is_image = true;
							var image_tag = range.item(0);
						}
					}
				}

				// if there is an existing image, get its properties
				if (is_image && image_tag) {
					url    = image_tag.src;
					align  = image_tag.align;
					alt    = image_tag.alt;
					width  = (!image_tag.width)  ? ""  : image_tag.width;
					height = (!image_tag.height) ? ""  : image_tag.height;
					hspace = (!image_tag.hspace) ? ""  : image_tag.hspace;
					vspace = (!image_tag.vspace) ? ""  : image_tag.vspace;

					// Bug #1708 - If border attribute is used in ima tag, change it to style attribute.
					if (!image_tag.border && !image_tag.style.border) {
						border = "0";
					} else {
						if (image_tag.border) {
							border = image_tag.border;
						} else if (image_tag.style.border) {
							border = image_tag.style.border;
							border = border.match(/(\d{1})px/)[1];
						}
					}

					imageid = editor.make_relative_hrefs(url);
					e = '.*\\?a=([0-9]+)((:[^$]*)\\$)?';
					re = new RegExp(e, "gi");
					imageid = imageid.replace(re, "$1$3");
				}

				var in_popup = '0';
				if (this._inPopup == true) { in_popup = '1'; }

				editor_<?php echo $this->wysiwyg->name?>._object = this;
				strPage = "<?php echo $this->get_popup_href('insert_image.php')?>?f_imageid=" + escape(imageid) + "&f_align=" + escape(align) + "&f_width=" + escape(width) + "&f_height=" + escape(height) + "&f_alt=" + escape(alt) + "&f_horiz=" + escape(hspace) + "&f_vert=" + escape(vspace) + "&f_border=" + escape(border) + "&editor_name=" + escape("<?php echo $this->wysiwyg->name; ?>") + "&in_popup=" + escape(in_popup);

				this._popupDialog("matrixInsertImage", strPage, 740, 480, true, function(param) {
					if (!param) {
						// user must have pressed Cancel
						return false;
					}

					if (param["f_imageid"] != 0) {
						var url = './?a=' + param["f_imageid"];
						// shadow asset .. needs terminating exclamation mark
						if (param["f_imageid"].search(/:/) != -1) {
							url += '$';
						}
					} else {
						if (is_image && image_tag) {
							if (image_tag.src != '') {
								url = image_tag.src;
							} else {
								url = '';
							}
						}
					}

					var html = '<img src="' + url + '"';
					if (param["f_align"] != "") {
						html += ' align="' + param["f_align"] + '"';
					}
					if (param["f_width"] != "") {
						html += ' width="' + param["f_width"] + '"';
					}
					if (param["f_height"] != "") {
						html += ' height="' + param["f_height"] + '"';
					}
					if (param["f_alt"] != "") {
						html += ' alt="' + param["f_alt"] + '"';
					}
					if (param["f_horiz"] != "") {
						html += ' hspace="' + param["f_horiz"] + '"';
					}
					if (param["f_vert"] != "") {
						html += ' vspace="' + param["f_vert"] + '"';
					}
					if (param["f_border"] != "") {
						html += ' style="border: ' + param["f_border"] + 'px solid rgb(0, 0, 0);"';
					}
					html += '>';
					html = editor.make_absolute_urls(html);

					if (!is_image || HTMLArea.is_gecko) {
						// replace text with new image
						editor.insertHTML(html, range);
					} else {
						// we are replacing an existing image in IE
						image_tag.outerHTML = html;
					}

				}, null);
			};
		</script>
		<?php

	}//end paint_generic()


	/**
	* Call the insert image function when the button is clicked
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_button_click()
	{
		?>
		case "matrixinsertimage":
			this._matrixinsertImage();
			return;
		break;
		<?php

	}//end print_plugin_button_click()


}//end class

?>