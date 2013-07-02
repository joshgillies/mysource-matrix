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
* $Id: matrix_insert_image.inc,v 1.42 2013/07/02 04:07:10 akarelia Exp $
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
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.42 $
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
	* @return void
	* @access public
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
				var title    = "";
				var longdesc = "";
				var hspace   = "";
				var vspace   = "";
				var border   = "0";
				var imageid  = "0";
				var image_class = "";
				var image_id = "";
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
					url			= image_tag.src;
					align		= image_tag.align;
					alt			= image_tag.alt;
					title		= image_tag.title;
					image_class	= image_tag.className;
					image_id	= image_tag.id;
					width		= (!image_tag.width)	? ""  : image_tag.width;
					height		= (!image_tag.height)	? ""  : image_tag.height;
					hspace		= (!image_tag.hspace)	? ""  : image_tag.hspace;
					vspace		= (!image_tag.vspace)	? ""  : image_tag.vspace;

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

					longdesc	= (!image_tag.longDesc)	? ""  : image_tag.longDesc;
					if (image_tag.longDesc) {
						var result = image_tag.longDesc.match(/(.+)\/\?a=(\d+)/);
						if (result != null) {
							// longdesc is from assetid
							longdesc = result[2];
						} else {
							// long desc is manually typed
							e = 'http[s]?://<?php echo $_SERVER['HTTP_HOST'].$_SERVER['PHP_SELF']?>';
							re = new RegExp(e, "gi");
							longdesc = image_tag.longDesc.replace(re, "");

						}
					} else {
						longdesc = "";
					}
					imageid = editor.make_relative_hrefs(url);
					e = '.*\\?a=([0-9]+)((:[^$]*)\\$)?';
					re = new RegExp(e, "gi");
					imageid = imageid.replace(re, "$1$3");
				}

				var in_popup = '0';
				if (this._inPopup == true) { in_popup = '1'; }

				editor_<?php echo $this->wysiwyg->name?>._object = this;
				strPage = "<?php echo $this->get_popup_href('insert_image_frames.php')?>?f_imageid=" + encodeURIComponent(imageid) + "&f_align=" + encodeURIComponent(align) + "&f_width=" + encodeURIComponent(width) + "&f_height=" + encodeURIComponent(height) + "&f_alt=" + encodeURIComponent(alt) + "&f_title=" + encodeURIComponent(title) + "&f_longdesc=" + encodeURIComponent(longdesc) + "&f_horiz=" + encodeURIComponent(hspace) + "&f_vert=" + encodeURIComponent(vspace) + "&f_border=" + encodeURIComponent(border) + "&f_image_class=" + encodeURIComponent(image_class) + "&f_image_id=" + encodeURIComponent(image_id) + "&editor_name=" + encodeURIComponent("<?php echo $this->wysiwyg->name; ?>") + "&in_popup=" + encodeURIComponent(in_popup);

				var popup_height = 630;

				// A bit more breathing room for Safari
				if (HTMLArea.is_safari) popup_height += 35;				
				if (HTMLArea.is_linux && HTMLArea.is_gecko) popup_height += 35;

				this._popupDialog("matrixInsertImage", strPage, 800, popup_height, true, function(param) {		

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
					if (param["f_title"] != "") {
						html += ' title="' + param["f_title"] + '"';
					}
					if (param["f_image_id"] != "") {
						html += ' id="' + param["f_image_id"] + '"';
					}
					if (param["f_image_class"] != "") {
						html += ' class="' + param["f_image_class"] + '"';
					}

					if (param["f_longdesc"] != null && param["f_longdesc"] != "") {
						var result = param["f_longdesc"].match(/(\d)+/);
						if (result != null) {
							if (result[1] != "0") {
								html += ' longdesc="<?php echo sq_web_path('root_url'); ?>' + '/?a=' + param["f_longdesc"] + '"';
							}
						} else {
							html += ' longdesc="' + param["f_longdesc"] + '"';
						}
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
	* @return void
	* @access public
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


	/**
	* Processes and returns the submitted HTML from the editor
	* This allows plugins to modify the html as they see fit.
	*
	* @param string	&$html	the HTML that was submitted by the editor
	*
	* @return string
	* @access public
	*/
	function process(&$html)
	{
		// Check whether any width or height has been updated
		$disable_resize = $GLOBALS['SQ_SYSTEM']->getUserPrefs('content_type_wysiwyg', 'SQ_WYSIWYG_DISABLE_IMAGE_RESIZE');
		if ($disable_resize == 'yes') {
			$new_html = rawurldecode($html);
			$images = Array();
			preg_match_all('/<img[^>]*>/i', $new_html, $images);
			if (!empty($images)) {
				$html_updated = FALSE;
				foreach ($images[0] as $img_tag) {
					// matrix internal link?
					$src = Array();
					preg_match('/src=["\']?\.\/\?a=([^ "\'>]*)["\']?/i', $img_tag, $src);
					$style_width_changed = FALSE;
					$style_height_changed = FALSE;
					$attr_width_changed = FALSE;
					$attr_height_changed = FALSE;
					$tmp_img_tag = $img_tag;

					if (!empty($src)) {
						$id_parts = explode(':', $src[1]);
						if (isset($id_parts[1])) continue;

						$img_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($src[1]);
						$width = $img_asset->attr('width');
						$height = $img_asset->attr('height');

						// inlined style?
						$style = Array();
						preg_match('/style=["\']?([^>"\']*)["\']?/i', $img_tag, $style);
						if (!empty($style)) {
							$style_width = Array();
							preg_match('/width: *(\d+) *(px)?/i', $img_tag, $style_width);
							if (!empty($style_width)) {
								$style_w = (int) $style_width[1];
								if ($style_w != $width) {
									$style_width_changed = TRUE;
									$replacement = 'width: '.$width.'px';
									$tmp_img_tag = str_replace($style_width[0], $replacement, $tmp_img_tag);
								}
							}

							$style_height = Array();
							preg_match('/height: *(\d+) *(px)?/i', $img_tag, $style_height);
							if (!empty($style_height)) {
								$style_h = (int) $style_height[1];
								if ($style_h != $height) {
									$style_height_changed = TRUE;
									$replacement = 'height: '.$height.'px';
									$tmp_img_tag = str_replace($style_height[0], $replacement, $tmp_img_tag);
								}
							}

						}

						// width attribute?
						$width_attr = Array();
						preg_match('/width=["\']?([^> "\']*)["\']?/i', $img_tag, $width_attr);
						if (!empty($width_attr)) {
							$w_attr = (int) $width_attr[1];
							if ($w_attr != $width) {
								$attr_width_changed = TRUE;
								$replacement = 'width="'.$width.'"';
								$tmp_img_tag = str_replace($width_attr[0], $replacement, $tmp_img_tag);
							}
						}

						// height attribute?
						$height_attr = Array();
						preg_match('/height=["\']?([^> "\']*)["\']?/i', $img_tag, $height_attr);
						if (!empty($height_attr)) {
							$h_attr = (int) $height_attr[1];
							if ($h_attr != $height) {
								$attr_height_changed = TRUE;
								$replacement = 'height="'.$height.'"';
								$tmp_img_tag = str_replace($height_attr[0], $replacement, $tmp_img_tag);
							}
						}

						if ($style_width_changed || $style_height_changed || $attr_width_changed || $attr_height_changed) {
								$new_html = str_replace($img_tag, $tmp_img_tag, $new_html);
								if (!$html_updated) $html_updated = TRUE;
						}
					}//end if src

				}//end foreach image tags

				if ($html_updated) $html = $new_html;

			}//end if images

		}//end if disable_resize

	}//end process()


}//end class

?>
