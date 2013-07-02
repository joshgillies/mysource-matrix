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
* $Id: matrix_insert_anchor.inc,v 1.20 2013/07/02 04:07:08 akarelia Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');
require_once dirname(__FILE__).'/../../../../core/include/init.inc';

/**
* WYSIWYG Plugin - Matrix Insert Anchor
*
* Purpose
*     A WYSIWYG plugin to allow the user to insert and remove
*     an anchor in the editable document
*
* @author  Mark Brydon <mbrydon@squiz.net>
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.20 $
* @package MySource_Matrix
* @subpackage wysiwyg
*/
class matrix_insert_anchor extends wysiwyg_plugin
{

	/**
	* Constructor
	* Adds the insert anchor button to this plugin
	*
	* @returns object Insert_Anchor
	* @access  public
	*/
	function matrix_insert_anchor(&$wysiwyg)
	{
		wysiwyg_plugin::wysiwyg_plugin($wysiwyg);
		$this->_add_button('matrixcreateanchor','MatrixCreateAnchor',translate('insert_anchor'),'false','30');

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
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/js/general.js' ?>"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('fudge').'/var_serialise/var_serialise.js' ?>"></script>

		<script type="text/javascript" language="Javascript">
			var wysiwyg_editing = true; // just so that onclick links don't execute in wysiwyg

			// Called when the user clicks on "Insert Anchor" button
			HTMLArea.prototype._matrixInsertAnchor = function() {
				var editor = this; // for nested functions

				// OK, let's create the current args array that we'll pass to the dialog
				var currArgs = new Object();
				currArgs["name"] = "";

				var sel = this._getSelection();
				var range = this._createRange(sel);
				var text_range = false;
				// OK, so they have a control object selected, let's try and make it a text selection
				if (sel.type && sel.type.toLowerCase() == "control") {
					var control_range = this._createRange(sel);
					var text_range = this._createTextRange(sel);
					if (text_range != null) {
						text_range = true;
						editor.selectNodeContents(control_range(0));
					} else {
						alert(js_translate('object_requires_selection'));
						sel.empty();
						return;
					}
					sel = this._getSelection();
					range = this._createRange(sel);
				}//end if a control

				var inside_html = '';
				var current_a = this.getClosest("a");
				if (current_a) {
					if (HTMLArea.is_ie) {
						inside_html = current_a.innerHTML;
					} else {
						this.selectNodeContents(current_a);
						inside_html = editor.getSelectedHTML();
					}
				} else {
					inside_html = editor.getSelectedHTML();
				}
				sel = this._getSelection();
				range = this._createRange(sel);
				// Is this aready a link?
				if (current_a) {

					if (typeof(visual_aid_mode) != 'undefined' && visual_aid_mode) {
						// If visual aid is being used now, remove it before processing
						// we keep the copy here in the case of canceling
						var copied_html = current_a.innerHTML;
						if (HTMLArea.is_gecko) {
							current_a.innerHTML = current_a.innerHTML.replace(/<span id="wysiwyg-visual-aid-plugin-anchor"[^>]*>A[^>]*<\/span>/, "");
						} else if (HTMLArea.is_ie) {
							var e = '<span id=wysiwyg-visual-aid-plugin-anchor[^>]*>A[^>]*<\/span>';
							var re = new RegExp(e, "ig");
							current_a.innerHTML = current_a.innerHTML.replace(re, '');
						}
					}

					var e = '^(\<a [^\>]+>)(.*)</a>$';
					var re = new RegExp(e, "i");
					var tag_html = editor.getSelectedHTML().replace(re, "$1");

					var re = new RegExp("\n", "gi");
					var tag_html = tag_html.replace(re, "");

					currArgs["name"] = current_a.name;

					re = new RegExp('if \\(!window\\.wysiwyg_editing\\)[\\s]+\\{[\\s]+window\\.open\\([\'"]([^\'"]+)[\'"], [\'"][0-9]+[\'"], [\'"]([^\'"]+)[\'"]\\);', "gi");
					results = re.exec(tag_html);

					if (results != null) {
						currArgs["name"] = results[1];
					}// end if

				// else not a link so check for paragraphs
				} else {

					// convert all para tags to lowercase
					var para_html = inside_html.replace(/(<\/?)P/gi, "$1p");

					// OK so we have some paragraph tags caught up in the mix, let's see what we can do with them
					// because we don't want our href's going around paragraph tags
					if (para_html.indexOf("<p") > -1) {

						// if the position of the first tag is the same as the last then we have only one paragraph
						// we don't want more than one paragraph because these can start or end from half-way through
						// a para, which would break the formatting when we go to re-insert the text with the href
						if (para_html.indexOf("<p") != para_html.lastIndexOf("<p")) {
							alert("Please select only one paragraph to link at a time.");
							return;
						}

					}// end if paragraphs exist

				}// end if existing link

				var in_popup = '0';
				if (this._inPopup == true) { in_popup = '1'; }

				editor_<?php echo $this->wysiwyg->name?>._object = this;
				var strPage = "<?php echo $this->get_popup_href('insert_anchor.php')?>?&name=" + escape(currArgs["name"]) + "&editor_name=" + escape("<?php echo $this->wysiwyg->name; ?>") + "&in_popup=" + escape(in_popup);

				this._popupDialog("matrixInsertAnchor", strPage, 480, 300, true, function(param) {

					if (!param) {
						// user must have pressed Cancel
						if (typeof(visual_aid_mode) != 'undefined' && visual_aid_mode) {
							if (current_a != null) {
								current_a.innerHTML = copied_html;
							}
						}
						return false;
					}

					if (param["remove"] == "1") {
						if (current_a != null) {
							// remove this anchor tag
							var current_html = current_a.innerHTML;
							var anchor_inner_html = current_html;

							// Brackets within the HTML must be quoted when in Visual Aid mode, so the anchor can be located
							if (typeof(visual_aid_mode) != 'undefined' && visual_aid_mode) {
								if (HTMLArea.is_gecko) {
									current_html = current_html.replace(/\(/, "\\\(");
									current_html = current_html.replace(/\)/, "\\\)");

									// Remove the Visual Aid code including the anchor
									anchor_inner_html = anchor_inner_html.replace(/<span id="wysiwyg-visual-aid-plugin-anchor".*<\/span>/, '');
								} else if (HTMLArea.is_ie) {
									var re = new RegExp("\\(", "ig");
									current_html = current_html.replace(re, "\\\(");

									re = new RegExp("\\)", "ig");
									current_html = current_html.replace(re, "\\\)");

									re = new RegExp("<span id=wysiwyg-visual-aid-plugin-anchor.*<\/span>", "ig");
									anchor_inner_html = anchor_inner_html.replace(re, '');
								}
							}

							var parent = current_a.parentNode;
							if (HTMLArea.is_gecko) {
								var e = '(<a.*name="' + param["name"] + '"[^>]*>)' + current_html + '</a>';
							} else if (HTMLArea.is_ie) {
								var e = '(<a.*name="?' + param["name"] + '"?[^>]*>)' + current_html + '</a>';
							}
							var re = new RegExp(e, "ig");
							var new_html = parent.innerHTML.replace(re, anchor_inner_html);
							parent.innerHTML = new_html;
						}
					} else {
						var visual_aid_tag = "";
						var visual_aid_style = 'padding-left: 1px; padding-right: 1px; padding-top: 1px; padding-bottom: 1px; background-color : #37DF5E; font: 10px Verdana,Tahoma,sans-serif; font-weight: bold; border-left: 1px solid; border-right: 1px solid; border-top: 1px solid; border-bottom: 1px solid;';

						if (param["name"] != "") {
							if (typeof(visual_aid_mode) != 'undefined' && visual_aid_mode) {
								visual_aid_tag = '<span id="wysiwyg-visual-aid-plugin-anchor" style="' + visual_aid_style + '">A "' + param["name"] + '"</span>';
							}
							if (current_a) {
								// editing
								if (param["name"] != current_a.getAttribute("name")) {
									if (HTMLArea.is_gecko) {
										current_a.setAttribute("name", param["name"]);
										current_a.setAttribute("id", param["name"]);
										current_a.innerHTML = visual_aid_tag + current_a.innerHTML;
									} else if (HTMLArea.is_ie) {
										// somehow IE can not update the name attribute value in the wysiwyg
										// this is just mysterious...

										// Also some trickery to spot lack of quotes in IE for one-word attribute values
										var e = '<A( id="?' + current_a.getAttribute("name") + '"?){0,1} name="?' + current_a.getAttribute("name") + '"?>';
										var re = new RegExp(e, "gi");
										var replacement = '<A id="' + param["name"] + '" name="' + param["name"] + '">' + visual_aid_tag;
										current_a.parentNode.innerHTML = current_a.parentNode.innerHTML.replace(re, replacement);
									}
								}
							} else {
								// adding
								var para = new RegExp("^[\n\t\s]*<P[^>]*>(.*)<\/P>", "gim")
								var insideP = inside_html.replace(para, "$1");
								if (insideP != inside_html) {
									var html = '<p><a name="' + param["name"] + '" id="' + param["name"];
									html += '">' + visual_aid_tag + insideP + '</a></p>';
								} else {
									var html = '<a name="' + param["name"] + '" id="' + param["name"];
									html += '">' + visual_aid_tag + inside_html + '</a>';
								}

								// replace with new URL
								editor.insertHTML(html, range);
							}

						}// end if param exists
					}
					// unselect the selection
					if (HTMLArea.is_ie) {
						range.execCommand("Unselect");
					} else {
						sel.removeAllRanges();
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
		case "matrixcreateanchor":
			this._matrixInsertAnchor();
		break;
		<?php

	}//end print_plugin_button_click()


	/**
	* Disable the toolbar when the popup is open
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_update_toolbar()
	{
		?>
		case "matrixcreateanchor":
			if (allEnabled == null && this._tmp['disable_toolbar'] == true) {
				this.updateToolbar(false);
				return;
			}
		break;
		<?php

	}//end print_plugin_update_toolbar()

}//end class

?>
