<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: matrix_insert_image.inc,v 1.13 2003/10/24 01:40:55 gsherwood Exp $
* $Name: not supported by cvs2svn $
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
* @version $Version$ - 1.0
* @package wysiwyg
*/
class matrix_insert_image extends wysiwyg_plugin {

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

				// disable the toolbar
				this._tmp['disable_toolbar'] = true;
				this.updateToolbar(false);

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
					border = (!image_tag.border) ? "0" : image_tag.border;

					imageid = editor.make_relative_hrefs(url);
					e = '(.*)(\\?a=)([0-9]+)(.*)';
					re = new RegExp(e, "gi");
					imageid = imageid.replace(re, "$3");
				}

				editor_<?php echo $this->wysiwyg->name?>._object = this;
				strPage = "<?php echo $this->get_popup_href('insert_image.php')?>?f_imageid=" + escape(imageid) + "&f_align=" + escape(align) + "&f_width=" + escape(width) + "&f_height=" + escape(height) + "&f_alt=" + escape(alt) + "&f_horiz=" + escape(hspace) + "&f_vert=" + escape(vspace) + "&f_border=" + escape(border) + "&editor_name=" + escape("<?php echo $this->wysiwyg->name; ?>");

				this._popupDialog("matrixInsertImage", strPage, 400, 300, false, function(param) {
					if (!param) {
						// user must have pressed Cancel
						return false;
					}

					var html = '<img src="./?a=' + param["f_imageid"] + '"';
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
						html += ' border="' + param["f_border"] + '"';
					}
					html += '>';

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
}

?>