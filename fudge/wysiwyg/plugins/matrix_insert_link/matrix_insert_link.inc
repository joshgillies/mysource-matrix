<?php

global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - Matrix Insert Link
*
* Purpose
*     A WYSIWYG plugin to allow the user to insert and remove
*     a link in the editable document using an asset from MySoure Matrix
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Fudge::wysiwyg
*/
class matrix_insert_link extends wysiwyg_plugin {

	/**
	* Constructor
	* Adds the insert and remove link buttons to this plugin
	*
	* @returns object Insert_Link
	* @access  public
	*/
	function matrix_insert_link(&$wysiwyg) {
		wysiwyg_plugin::wysiwyg_plugin($wysiwyg);
		$this->_add_button('matrixcreatelink','MatrixCreateLink','Insert Link','false','32');
		$this->_add_button('matrixunlink','Unlink','Remove Link','false','33');
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
			var wysiwyg_editing = true; // just so that onclick link's don't execute in wysiwyg

			// Called when the user clicks on "InsertImage" button
			HTMLArea.prototype._matrixInsertLink = function() {
				var editor = this; // for nested functions

				// disable the toolbar
				this._tmp['disable_toolbar'] = true;
				this.updateToolbar(false);

				// all available new window options
				var NEW_WINDOW_OPTIONS = new Array('toolbar','menubar','location','status','scrollbars','resizable','width','height');

				// OK, let's create the current args array that we'll pass to the dialog
				var currArgs = new Object();
				currArgs["assetid"] = "0";
				currArgs["url"] = "http://";
				currArgs["status_text"] = "";
				currArgs["new_window"]  = 0;

				currArgs["new_window_options"] = new Object();
				for(var i=0; i < NEW_WINDOW_OPTIONS.length; i++) {
					currArgs["new_window_options"][NEW_WINDOW_OPTIONS[i]] = '';
				}

				var sel = this._getSelection();
				var range = this._createRange(sel);

				// OK, so they have a control object selected, let's try and make it a text selection
				if (sel.type && sel.type.toLowerCase() == "control") {
					var control_range = this._createRange(sel);
					var text_range = this._createTextRange(sel);
					if (text_range != null) {
						editor._selectElement(control_range(0));
					} else {
						alert("You need to select this object as though it was text, rather than just clicking on it.");
						sel.empty();
						return;
					}
					sel = this._getSelection();
					range = this._createRange(sel);
				}// end if a control

				if (HTMLArea.is_ie) {
					var el = range.parentElement();
				} else {
					var el = range.cloneContents().childNodes[0];
				}

				var htmlText = editor.getSelectedHTML();
				
				var link_text = "";
				var para_obj = null;

				// if the parent element is not an anchor but the text starts with an anchor tag
				// which happens when selecting images with existing links on them
				// OR
				// if the text starts with an img tag then try and get the parent
				if ((el && el.nodeName != "A" && htmlText.search(/^<a/gi) > -1) || htmlText.search(/^<img/gi) > -1) {
					if (el.parentNode && el.parentNode.nodeName == "A") {
						el = el.parentNode;
					} else if (sel.anchorNode.nodeName == "A") {
						editor._selectElement(sel.anchorNode);
						sel = this._getSelection();
						range = this._createRange(sel);
						el = range.cloneContents().childNodes[0];
					}
				}// end if

				if (HTMLArea.is_ie) {
					var onClick = el.outerHTML;
				} else {
					var onClick = HTMLArea.getHTML(range.cloneContents());
				}

				// Is this aready a link?
				if (el && el.nodeName == "A") {
					currArgs["url"] = el.href;

					// OK, let's see if we are doing a window.open()
					var e = 'onClick="javascript: if \\(!window\\.wysiwyg_editing\\) \\{ window\\.open\\(\'([^\']+)\', \'[0-9]+\', \'([^\']+)\'\\); \\}';
					var re = new RegExp(e, "gi");
					var results = re.exec(onClick);

					if (results != null) {
						currArgs["url"] = results[1];
						currArgs["new_window"] = 1;
						var options_str = results[2]; 

						for(var i=0; i < NEW_WINDOW_OPTIONS.length; i++) {
							e = NEW_WINDOW_OPTIONS[i] + "=([^,]+)";
							re = new RegExp(e, "gi");
							results = re.exec(options_str);
							currArgs["new_window_options"][NEW_WINDOW_OPTIONS[i]] = (results != null) ? results[1] : "";
						}
					}


					// OK, let's try and get the window.status text if any
					re = new RegExp('onmouseover="([^"]+)"', "gi");
					results = re.exec(onClick);
					if (results != null) {
						var onmouseover = results[1];
						re = new RegExp('window.status=\'(.+)\'; return true;', "gi");
						results = re.exec(onmouseover);
						if (results != null) {
							currArgs["status_text"] = rev_htmlspecialchars(results[1]).replace(/\\\'/gi, "\'");
						}
					}// end if

					// because the editor automatically prepend the http:// business to 
					// all relative urls remove it
					currArgs["url"] = unescape(editor.make_relative_hrefs(currArgs["url"]));
					// Get the text associated with this link
					link_text = el.innerHTML;

					// Get the assetid for the link if there is one
					e = '(.*)(\\?a=)([0-9]+)(.*)';
					re = new RegExp(e, "gi");
					currArgs["assetid"] = currArgs["url"].replace(re, "$3");

				// else not a link so check for paragraphs
				} else {

					// convert all para tags to lowercase
					link_text = htmlText.replace(/(<\/?)P/gi, "$1p");
					
					// OK so we have some paragraph tags caught up in the mix, let's see what we can do with them
					// because we don't want our href's going around paragraph tags
					if (link_text.indexOf("<p") > -1) {

						// if the position of the first tag is the same as the last then we have only one paragraph
						// we don't want more than one paragraph because these can start or end from half-way through 
						// a para, which would fuck up the formatting when we go to re-insert the text with the href
						if (link_text.indexOf("<p") == link_text.lastIndexOf("<p")) {
							alert('FIXME: format_link_para not done yet');
							para_obj = format_link_para(link_text);
						} else {
							alert("Please select only one paragraph to link at a time otherwise bad things happen.");
							return;
						}

					}// end if paragraphs exist

				}// end if existing link

				editor_<?php echo $this->wysiwyg->name?>._object = this;
				var strPage = "<?php echo $this->get_popup_href('insert_link.php')?>?assetid=" + escape(currArgs["assetid"]) + "&url=" + escape(currArgs["url"]) + "&status_text=" + escape(currArgs["status_text"]) + "&new_window=" + escape(currArgs["new_window"]) + "&new_window_options=" + escape(var_serialise(currArgs["new_window_options"])) + "&editor_name=" + escape("<?php echo $this->wysiwyg->name; ?>");

				this._popupDialog(strPage, function(param) {
					if (param && param["url"] != "") {
		
						var html = '';

						if (para_obj != null) {
							link_text = para_obj.html;
							html += para_obj.start;
						}

						// if we have a new window, generate the new javascript url string
						if (param["new_window"] == "1") {
							var now = new Date();
							var option_string = "";
							for(var i=0; i < NEW_WINDOW_OPTIONS.length; i++) {
								if (option_string != "") option_string += ",";
								option_string += NEW_WINDOW_OPTIONS[i] + "=" + param["new_window_options"][NEW_WINDOW_OPTIONS[i]];
							}

							param["onClick"] = "javascript: if (!window.wysiwyg_editing) { window.open('" + param["url"] + "', '" + now.getTime() + "', '" + option_string + "'); } return false;";
						}

						html += '<a href="' + param["url"] + '"';
						if (param["status_text"] != "") {
							html += ' onMouseOver="javascript: window.status=\'' + htmlspecialchars(param["status_text"]).replace(/\'/gi, "\\'") + '\'; return true;" onMouseOut="javascript: window.status=\'\'; return true;"';
						}
						if (param["onClick"] != null && param["onClick"] != "") {
							html += ' onClick="' + param["onClick"] + '"';
						}
						html += '>' + link_text + '</a>';

						if (para_obj != null) {
							html += para_obj.end;
						}

						// Replace with new URL
						editor.insertHTML(html, range);

					}// end if param exists

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
		case "matrixcreatelink":
			this._matrixInsertLink();
		break;
		<?php
	}


	/**
	* Disable the toolbar when the popup is open
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_update_toolbar() {
		?>
		case "matrixcreatelink":
			if (allEnabled == null && this._tmp['disable_toolbar'] == true) {
				this.updateToolbar(false);
				return;
			}
		break;
		<?php
	}

}

?>