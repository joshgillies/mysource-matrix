<?php

global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - Insert Image
*
* Purpose
*     A WYSIWYG plugin to allow the user to insert an image
*     in the editable document
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Fudge::wysiwyg
*/
class insert_image extends wysiwyg_plugin {

	/**
	* Constructor
	* Adds the insert image button to this plugin
	*
	* @returns object Insert_Image
	* @access  public
	*/
	function insert_image() {
		$this->_add_button('insertimage','InsertImage','Insert Image','false','34');
	}


	/**
	* Prints a generic insert image function that can be used by any editor
	* A URL must be entered for the image you want to insert. Alt text must
	* also be entered before the image will be inserted.
	*
	* @returns void
	* @access  public
	*/
	function paint_generic() {
		?>
		<script type="text/javascript" language="Javascript">
			// Called when the user clicks on "InsertImage" button
			HTMLArea.prototype._insertImage = function() {
				var editor = this; // for nested functions

				// set focus in case we havnt selected anything
				if (HTMLArea.is_ie) {
					this._docContent.focus();
				} else if (HTMLArea.ie_gecko) {
					this.focusEditor();
				}

				var sel = this._getSelection();
				var range = this._createRange(sel);

				var url    = "";
				var align  = "";
				var alt    = "";
				var hspace = "";
				var vspace = "";
				var border = "0";
				var is_image = false;

				// loop through and try and find a selected image tag
				if (HTMLArea.is_gecko) {
					for (var i = 0; i < range.startContainer.childNodes.length; i++) {
						if (range.startContainer.childNodes[i].tagName == "IMG") {
							is_image = true;
							var image_tag = range.startContainer.childNodes[i];
						}// end if IMG
					}
				} else if (HTMLArea.is_ie) {
					if(range.length) {
						if (range.item(0).tagName == "IMG") {
							is_image = true;
							var image_tag = range.item(0);
						}// end if IMG
					}// end if length
				}

				// if there is an existing image, get its properties
				if (is_image && image_tag) {
					url    = image_tag.src;
					align  = image_tag.align;
					alt    = image_tag.alt;
					hspace = (!image_tag.hspace) ? ""  : image_tag.hspace;
					vspace = (!image_tag.vspace) ? ""  : image_tag.vspace;
					border = (!image_tag.border) ? "0" : image_tag.border;
				}

				strPage = "<?php echo $this->get_popup_href('insert_image.php')?>?f_url=" + escape(url) + "&f_align=" + escape(align) + "&f_alt=" + escape(alt) + "&f_horiz=" + escape(hspace) + "&f_vert=" + escape(vspace) + "&f_border=" + escape(border);

				this._popupDialog(strPage, function(param) {
					if (!param) { 
						// user must have pressed Cancel
						return false;
					}

					var url = new String(param["f_url"]);

					var html = '<img src="' + url + '"';
					if (param["f_align"] != "") {
						html += ' align="' + param["f_align"] + '"';
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
						editor.insertHTML(html);
					} else {
						// we are replacing an existing image in IE
						image_tag.outerHTML = html;
					}

				}, null);
			};
		</script>
		<?php
	}


	/**
	* Call the insert image function when the button is clicked
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_button_click() {
		?>
		case "insertimage":
			this._insertImage();
		break;
		<?php
	}
}

?>