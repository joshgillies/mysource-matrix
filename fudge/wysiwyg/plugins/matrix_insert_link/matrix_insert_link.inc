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
* $Id: matrix_insert_link.inc,v 1.60 2013/09/25 03:18:26 akarelia Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');
require_once dirname(__FILE__).'/../../../../core/include/init.inc';

/**
* WYSIWYG Plugin - Matrix Insert Link
*
* Purpose
*     A WYSIWYG plugin to allow the user to insert and remove
*     a link in the editable document using an asset from MySoure Matrix
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.60 $
* @package MySource_Matrix
* @subpackage wysiwyg
*/
class matrix_insert_link extends wysiwyg_plugin
{


	/**
	* Constructor
	* Adds the insert and remove link buttons to this plugin
	*
	* @returns object Insert_Link
	* @access public
	*/
	function matrix_insert_link(&$wysiwyg)
	{
		wysiwyg_plugin::wysiwyg_plugin($wysiwyg);
		$this->_add_button('matrixcreatelink','MatrixCreateLink', translate('Insert Link'),'false','32');
		$this->_add_button('matrixunlink','Unlink', translate('Remove Link'),'false','33');

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
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/js/general.js' ?>"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('fudge').'/var_serialise/var_serialise.js' ?>"></script>

		<script type="text/javascript" language="Javascript">
			var wysiwyg_editing = true; // just so that onclick link's don't execute in wysiwyg


			HTMLArea.prototype._matrixUnLink = function() {
				var editor = this; // for nested functions

				var current_a = this.getClosest("a");
				
				if (!current_a) {
					if (HTMLArea.is_ie) { //IMG tag support for IE as they land in controlRange collection.
						var sel = this._getSelection();
						var range = this._createRange(sel);
						if (sel.type == "Control"){
							for (i = 0; i < range.length; i++){
								if (range.item(i).tagName == "IMG"){
									if (range.item(i).parentNode.tagName == "A"){//check parent is anchor
										current_a = range.item(i).parentNode;
										break;
									}
								}
							}
						}
						
					} 
					if (!current_a) return false; //still not found
				}
				var inside_html = current_a.innerHTML;
				var selection =  editor.getSelectedHTML();
				//IMG tag support
				//Firefox: selection same as inside_html but attributes re-arranged.
				//IE: can't get selection because lands in controlRange collection (see above).
				//selection only needed to process partial selection so just equate to inside_html (see below).
				var reg = new RegExp("^\s*<img ?[^>]+src=[^>]+>\s*$", "i");
				if (!selection || selection.match(reg)) selection = inside_html;
				
				// Remove surrounding anchor tag from selected HTML for comparison
				if (HTMLArea.is_ie) {
					// Remove anchor start and end
					var e = '^<a[^>]*>';
					var re = new RegExp(e, "i");
					selection = selection.replace(re, "");
					e = '</a>$';
					re = new RegExp(e, "i");
					selection = selection.replace(re, "");
					// Remove newlines from anchor HTML so we can perform a meaningful comparison
					e = '\r\n';
					re = new RegExp(e, "i");
					selection = selection.replace(re, "");
				}

				if (selection != '' && selection != inside_html) {
					// Partial selection within the link tag
					// will not work if selection partially includes other tags
					// for example:
					// <a href="example.com">partial <b>link</b> here</a>
					// and user selects "partial <b>li" as the selection
					var e = '(\<a [^\>]+>)(.*)' + '(' + selection + ')(.*)(</a>)';
					var re = new RegExp(e, "i");
					// turns into "<a>before</a> selection <a>after</a>"
					var remove_tag = HTMLArea.getHTML(current_a, true).replace(re, "$1$2$5$3$1$4$5");
					// Remove empty link tags
					var e = '\<a [^\>]+>\\s*</a>';
					var re = new RegExp(e, "gi");
					inside_html = remove_tag.replace(re, "");
				}
				var text = this._doc.createDocumentFragment();
				var div = this._doc.createElement("div");
				div.innerHTML = inside_html;
				while (div.firstChild) {
					text.appendChild(div.firstChild);
				}
				current_a.parentNode.replaceChild(text, current_a);
			}


			// Called when the user clicks on "InsertImage" button
			HTMLArea.prototype._matrixInsertLink = function() {
				var editor = this; // for nested functions

				// all available new window options
				var NEW_WINDOW_OPTIONS = new Array('toolbar','menubar','location','status','scrollbars','resizable','width','height');

				// OK, let's create the current args array that we'll pass to the dialog
				var currArgs = new Object();
				currArgs["assetid"] = "0";
				currArgs["url"] = "http://";
				currArgs["class_name"] = "";
				currArgs["id_name"] = "";
				currArgs["anchor"] = "";
				currArgs["status_text"] = "";
				currArgs["link_title"] = "";
				currArgs["target"] = "";
				currArgs["new_window"]  = 0;
				currArgs["show_summary"] = 0;

				currArgs["new_window_options"] = new Object();
				for(var i=0; i < NEW_WINDOW_OPTIONS.length; i++) {
					currArgs["new_window_options"][NEW_WINDOW_OPTIONS[i]] = '';
				}

				var sel = this._getSelection();
				var range = this._createRange(sel);
				var text_range = false;

				// OK, so they have a control object selected, let's try and make it a text selection
				var inside_html = '';
				if (sel.type && sel.type.toLowerCase() == "control") {
					var control_range = this._createRange(sel);
					var text_range = this._createTextRange(sel);
					if (text_range != null) {
						text_range = true;
						if (Object.prototype.toString.call(control_range) == '[object ControlRangeCollection]') {
							inside_html = control_range.item(0).outerHTML;
						} else {
							editor.selectNodeContents(control_range(0));
							sel = this._getSelection();
							range = this._createRange(sel);
							inside_html = editor.getSelectedHTML();
						}
					} else {
						alert(js_translate('You need to select this object as though it was text, rather than just clicking on it.'));
						sel.empty();
						return;
					}
				}// end if a control

				var current_a = this.getClosest("a");
				if (current_a) {
					inside_html = current_a.innerHTML;
					this.selectNodeContents(current_a);
				} else if (inside_html == '') {
					inside_html = editor.getSelectedHTML();
				}

				sel = this._getSelection();
				range = this._createRange(sel);

				// is this aready a link?
				if (current_a) {

					var e = '^(\<a [^\>]+>)(.*)</a>$';
					var re = new RegExp(e, "i");
					var tag_html = HTMLArea.getHTML(current_a, true).replace(re, "$1");

					var re = new RegExp("\n", "gi");
					var tag_html = tag_html.replace(re, "");

					currArgs["url"] = current_a.href;

					re = new RegExp('if \\(!window\\.wysiwyg_editing\\)[\\s]+\\{[\\s]+window\\.open\\([\'"]([^\'"]+)[\'"], [\'"][0-9]+[\'"], [\'"]([^\'"]+)[\'"]\\);', "gi");
					results = re.exec(tag_html);

					if (results != null) {
						currArgs["url"] = results[1];
						currArgs["new_window"] = 2;
						var options_str = results[2];

						for(var i=0; i < NEW_WINDOW_OPTIONS.length; i++) {
							e = NEW_WINDOW_OPTIONS[i] + "=([^,]+)";
							re = new RegExp(e, "gi");
							var res = re.exec(options_str);
							currArgs["new_window_options"][NEW_WINDOW_OPTIONS[i]] = (res != null) ? res[1] : "";
						}
					}// end if

					// let's try and get the window.status text if any
					re = new RegExp('onmouseover="([^"]+)"', "gi");
					results = re.exec(tag_html);
					if (results != null) {
						var onmouseover = results[1];
						re = new RegExp('window.status=\'(.+)\'; return true;', "gi");
						results = re.exec(onmouseover);
						if (results != null) {
							currArgs["status_text"] = rev_htmlspecialchars(results[1]).replace(/\\\'/gi, "\'");
						}
					}// end if

					//Get the link title, if there is one
					re = new RegExp('title="([^"]+)"', "gi");
					results = re.exec(tag_html);
					if (results != null) {
						currArgs["link_title"] = rev_htmlspecialchars(results[1]);
					}
					// let's try and get the target if any
					re = new RegExp('target="([^"]+)"', "gi");
					results = re.exec(tag_html);
					if (results != null) {
						currArgs["target"] = rev_htmlspecialchars(results[1]).replace(/\\\'/gi, "\'");
					}
					if (currArgs["target"] == "_blank") { currArgs["new_window"] = 1; }

					// let's try and get the class if any
					re = new RegExp('class="([^"]+)"', "gi");
					results = re.exec(tag_html);
					if (results != null) {
						currArgs["class_name"] = rev_htmlspecialchars(results[1]).replace(/\\\'/gi, "\'");
					}

					// let's try and get the class if any
					re = new RegExp('id="([^"]+)"', "gi");
					results = re.exec(tag_html);
					if (results != null) {
						currArgs["id_name"] = rev_htmlspecialchars(results[1]).replace(/\\\'/gi, "\'");
					}

					// it seems that the make_relative_hrefs() fn will take out the hash and think that a lone
					// anchor is actually the url link. Work out here whether it might need to be added on...
					var needsHash = false;
					if (currArgs["url"].indexOf('#') != -1) {
						needsHash = true;
					}

					// because the editor automatically prepend the http:// business to
					// all relative urls remove it
					currArgs["url"] = unescape(editor.make_relative_hrefs(currArgs["url"]));

					e = '.*(\\./\\?a=[0-9]+)((:[^$]*)\\$)?';
					re = new RegExp(e, "gi");
					currArgs["url"] = currArgs["url"].replace(re, "$1$3");

					if (currArgs["url"].substr(-1) == '/') currArgs["url"] = currArgs["url"].substr(0, currArgs["url"].length - 1)

					// if it's hashed and it's disappeared, add it back on here
					if (needsHash && (currArgs["url"].indexOf('#') == -1)) {
						currArgs["url"] = '#' + currArgs["url"];
					}

					// get the assetid for the link if there is one
					// only matches './?a=(normal or shadow id)'
					e = '\\./\\?a=([0-9]+)((:[^$]*)\\$)?';
					re = new RegExp(e, "gi");
					results = re.exec(currArgs["url"]);
					if (results != null) {
						currArgs["assetid"] = currArgs["url"].replace(re, "$1$3");
					}

					// if the link is made from asset attribute url
					// make sure to remove the url of it
					e = '(%globals_asset_attribute_(redirect|link)_url:([0-9]+[^$]+)%)';
					re = new RegExp(e, "gi");
					results = re.exec(currArgs["url"]);

					if (results != null) {
						currArgs["url"] = results[1];
						currArgs["assetid"] = results[3];
					}

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
							alert(js_translate('Please select only one paragraph to link at a time.'));
							return;
						}

					}// end if paragraphs exist

					// check for multiple list items <li>
					var para_html = inside_html.replace(/(<\/?)LI/gi, "$1li");

					if (para_html.indexOf("<li") > -1) {
						if (para_html.indexOf("<li") != para_html.lastIndexOf("<li")) {
							alert(js_translate('Please select only one list item to link at a time.'));
							return;
						} 
					}

				}// end if existing link

				var in_popup = '0';
				if (this._inPopup == true) { in_popup = '1'; }

				editor_<?php echo $this->wysiwyg->name?>._object = this;
				var strPage = "<?php echo $this->get_popup_href('insert_link_frames.php')?>?assetid=" + encodeURIComponent(currArgs["assetid"]) + "&url=" + encodeURIComponent(currArgs["url"]) + "&status_text=" + encodeURIComponent(currArgs["status_text"]) + "&link_title=" + encodeURIComponent(currArgs["link_title"])+ "&target=" + encodeURIComponent(currArgs["target"]) + "&new_window=" + encodeURIComponent(currArgs["new_window"]) + "&new_window_options=" + encodeURIComponent(var_serialise(currArgs["new_window_options"])) + "&editor_name=" + encodeURIComponent("<?php echo $this->wysiwyg->name; ?>") + "&in_popup=" + encodeURIComponent(in_popup) + "&class_name=" + encodeURIComponent(currArgs["class_name"]) + "&id_name=" + encodeURIComponent(currArgs["id_name"]);

				var popup_height = 495;

				// A bit more breathing room for Safari
				if (HTMLArea.is_safari) popup_height += 5;

				// Should we insert the span wrapping around the asset summary?
				// Avoid inserting it repeatedly
				var insert_asset_summary_span = true;
				if(current_a) {
					var parent = current_a.parentNode;
					if(parent) {
						if(parent.nodeName == 'SPAN' && parent.className == 'include_asset_summary') {
								if(editor.getSelectedHTML().match(/<span class="?include_asset_summary"?>/gi) == null) {
									insert_asset_summary_span = false;
								}
						}
					}
				}

				
				this._popupDialog("matrixInsertLink", strPage, 750, popup_height, true, function(param) {
					if (param && param["url"] != "") {

						var html = '';

						// if we have an advanced new window, generate the new javascript url string
						if (param["new_window"] == "1") { param["target"] = "_blank"; }
						if (param["new_window"] == "2") {
							var now = new Date();
							var option_string = "";
							for(var i=0; i < NEW_WINDOW_OPTIONS.length; i++) {
								if (option_string != "") option_string += ",";
								option_string += NEW_WINDOW_OPTIONS[i] + "=" + param["new_window_options"][NEW_WINDOW_OPTIONS[i]];
							}

							param["onClick"] = "javascript: if (!window.wysiwyg_editing) { window.open('" + param["url"] + "', '" + now.getTime() + "', '" + option_string + "'); } return false;";
							param["target"] = "";
						}

						var htmlTextToCheck = '';
						if (Object.prototype.toString.call(range) == '[object ControlRangeCollection]') {
							htmlTextToCheck = control_range.item(0).outerHTML;
						} else {
							htmlTextToCheck = range.htmlText;
						}
						// This fixes some probs IE has with Paragraphs and list
						if (HTMLArea.is_ie && (htmlTextToCheck.indexOf('<p') != -1 || htmlTextToCheck.indexOf('<P') != -1 || htmlTextToCheck.indexOf('<li') != -1  || htmlTextToCheck.indexOf('<LI') != -1)) {
							//See bug #987 for details on why this is done
							//It avoids <a> tags outside <p> tags
							var start_new = htmlTextToCheck.substring(0, htmlTextToCheck.indexOf('>') + 1);
							var end_new = htmlTextToCheck.substring(htmlTextToCheck.lastIndexOf('<'));
							var new_middle = htmlTextToCheck.substring(start_new.length, htmlTextToCheck.length - end_new.length);
							
							html = '<a href="' + param["url"] + '"';
							if (param["class_name"] != "") {
								html += ' class="' + htmlspecialchars(param["class_name"]).replace(/\'/gi, "\\'") + '"';
							}
							if (param["id_name"] != "") {
								html += ' id="' + htmlspecialchars(param["id_name"]).replace(/\'/gi, "\\'") + '"';
							}
							if (param["target"] != "") {
								html += ' target="' + htmlspecialchars(param["target"]).replace(/\'/gi, "\\'") + '"';
							}
							if (param["status_text"] != "") {
								html += ' onMouseOver="javascript: window.status=\'' + htmlspecialchars(param["status_text"]).replace(/\'/gi, "\\'") + '\'; return true;" onMouseOut="javascript: window.status=\'\'; return true;"';
							}
							if (param["link_title"] != "") {
								html += ' title="' + htmlspecialchars(param["link_title"]) + '"';
							}
							if (param["onClick"] != null && param["onClick"] != "") {
								html += ' onClick="' + param["onClick"] + '"';
							}
				

							// insert file summary keyword
							new_middle = new_middle.replace(/%asset_summary_[0-9]+%/gim, '');
							
							if (param['show_summary'] && param['assetid']) {
								html += '>' + new_middle + "%asset_summary_" + param['assetid'] + "%" + '</a>';		
								if (insert_asset_summary_span) {
									html = '<span class="include_asset_summary">' + html + '</span>';
								}				
							}
							else {
								html += '>' + new_middle + '</a>';
							}

							editor.insertHTML(start_new + html + end_new, range);
						} else {
							//We don't have IE, or there is no paragraph present to warrant different handling
							html += '<a href="' + param["url"] + '"';
							if (param["class_name"] != "") {
								html += ' class="' + htmlspecialchars(param["class_name"]).replace(/\'/gi, "\\'") + '"';
							}
							if (param["id_name"] != "") {
								html += ' id="' + htmlspecialchars(param["id_name"]).replace(/\'/gi, "\\'") + '"';
							}
							if (param["target"] != "") {
								html += ' target="' + htmlspecialchars(param["target"]).replace(/\'/gi, "\\'") + '"';
							}
							if (param["status_text"] != "") {
								html += ' onMouseOver="javascript: window.status=\'' + htmlspecialchars(param["status_text"]).replace(/\'/gi, "\\'") + '\'; return true;" onMouseOut="javascript: window.status=\'\'; return true;"';
							}
							if (param["link_title"] != "") {
								html += ' title="' + htmlspecialchars(param["link_title"]) + '"';
							}
							if (param["onClick"] != null && param["onClick"] != "") {
								html += ' onClick="' + param["onClick"] + '"';
							}


							
							// remove existing link(s) before inserting a new link
							if (current_a) {
								current_a.parentNode.removeChild(current_a);
							} else {
								// overlap/partially overlap (existing links)
								// note: regex doesn't work on nested tags, like in '<a>one<a>two</a>one</a>'
								var e = '<a[^>]*>(.*?)</a>';
								var re = new RegExp(e, "gim");
								inside_html = inside_html.replace(re, "$1");
							}


							
							// insert file summary keyword
							inside_html = inside_html.replace(/%asset_summary_[0-9]+%/gim, '');
							if (param['show_summary'] && param['assetid']) {
								html += '>' + inside_html + "%asset_summary_" + param['assetid'] + "%" + '</a>';	
								if (insert_asset_summary_span) {
									html = '<span class="include_asset_summary">' + html + '</span>';
								}
							}
							else {
								html += '>' + inside_html + '</a>';
							}
								
							// replace with new URL
							editor.insertHTML(html, range);
						}


					}// end if param exists

				}, null);

			};
		</script>
		<?php

	}//end paint_generic()


	/**
	* Prints shortcuts for the buttons
	*
	* @return void
	* @access public
	*/
	function print_plugin_shortcuts()
	{
		?>
		case 'k':
			this._matrixInsertLink();
			HTMLArea._stopEvent(ev);
		break;
		<?php

	}//end print_plugin_shortcuts()


	/**
	* Call the insert image function when the button is clicked
	*
	* @return void
	* @access public
	*/
	function print_plugin_button_click()
	{
		?>
		case "matrixcreatelink":
			this._matrixInsertLink();
		break;
		case "unlink":
			this._matrixUnLink();
		break;
		<?php

	}//end print_plugin_button_click()


	/**
	* Disable the toolbar when the popup is open
	*
	* @return void
	* @access public
	*/
	function print_plugin_update_toolbar()
	{
		?>
		case "matrixcreatelink":
			if (allEnabled == null && this._tmp['disable_toolbar'] == true) {
				this.updateToolbar(false);
				return;
			}
		break;
		<?php

	}//end print_plugin_update_toolbar()


}//end class

?>
