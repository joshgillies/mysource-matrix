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
* $Id: matrix_insert_link.inc,v 1.24 2004/08/25 05:23:04 mnyeholt Exp $
* $Name: not supported by cvs2svn $
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
* @version $Version$ - 1.0
* @package MySource_Matrix
* @subpackage wysiwyg
*/
class matrix_insert_link extends wysiwyg_plugin {

	/**
	* Constructor
	* Adds the insert and remove link buttons to this plugin
	*
	* @returns object Insert_Link
	* @access  public
	*/
	function matrix_insert_link(&$wysiwyg)
	{
		wysiwyg_plugin::wysiwyg_plugin($wysiwyg);
		$this->_add_button('matrixcreatelink','MatrixCreateLink','Insert Link','false','32');
		$this->_add_button('matrixunlink','Unlink','Remove Link','false','33');

	}//end constructor()


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
			var wysiwyg_editing = true; // just so that onclick link's don't execute in wysiwyg

			// Called when the user clicks on "InsertImage" button
			HTMLArea.prototype._matrixInsertLink = function() {
				var editor = this; // for nested functions

				// all available new window options
				var NEW_WINDOW_OPTIONS = new Array('toolbar','menubar','location','status','scrollbars','resizable','width','height');

				// OK, let's create the current args array that we'll pass to the dialog
				var currArgs = new Object();
				currArgs["assetid"] = "0";
				currArgs["url"] = "http://";
				currArgs["anchor"] = "";
				currArgs["status_text"] = "";
				currArgs["target"] = "";
				currArgs["new_window"]  = 0;

				currArgs["new_window_options"] = new Object();
				for(var i=0; i < NEW_WINDOW_OPTIONS.length; i++) {
					currArgs["new_window_options"][NEW_WINDOW_OPTIONS[i]] = '';
				}

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
						alert("You need to select this object as though it was text, rather than just clicking on it.");
						sel.empty();
						return;
					}
					sel = this._getSelection();
					range = this._createRange(sel);
				}// end if a control

				var inside_html = '';
				var current_a = this.getClosest("a");
				if (current_a) {
					inside_html = current_a.innerHTML;
					this.selectNodeContents(current_a);
				} else {
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

					// let's try and get the target if any
					re = new RegExp('target="([^"]+)"', "gi");
					results = re.exec(tag_html);
					if (results != null) {
						currArgs["target"] = rev_htmlspecialchars(results[1]).replace(/\\\'/gi, "\'");
					}
					if (currArgs["target"] == "_blank") { currArgs["new_window"] = 1; }

					// it seems that the make_relative_hrefs() fn will take out the hash and think that a lone
					// anchor is actually the url link. Work out here whether it might need to be added on...
					var needsHash = false;
					if (currArgs["url"].indexOf('#') != -1) {
						needsHash = true;
					}

					// because the editor automatically prepend the http:// business to 
					// all relative urls remove it
					currArgs["url"] = unescape(editor.make_relative_hrefs(currArgs["url"]));

					// if it's hashed and it's disappeared, add it back on here
					if (needsHash && (currArgs["url"].indexOf('#') == -1)) {
						currArgs["url"] = '#' + currArgs["url"];
					}

					// get the assetid for the link if there is one
					e = '(.*)(\\?a=)([0-9]+)(.*)';
					re = new RegExp(e, "gi");
					results = re.exec(currArgs["url"]);
					if (results != null) {
						currArgs["assetid"] = currArgs["url"].replace(re, "$3");
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
						// a para, which would fuck up the formatting when we go to re-insert the text with the href
						if (para_html.indexOf("<p") != para_html.lastIndexOf("<p")) {
							alert("Please select only one paragraph to link at a time otherwise bad things happen.");
							return;
						}

					}// end if paragraphs exist

				}// end if existing link

				var in_popup = '0';
				if (this._inPopup == true) { in_popup = '1'; }

				editor_<?php echo $this->wysiwyg->name?>._object = this;
				var strPage = "<?php echo $this->get_popup_href('insert_link.php')?>?assetid=" + escape(currArgs["assetid"]) + "&url=" + escape(currArgs["url"]) + "&status_text=" + escape(currArgs["status_text"]) + "&target=" + escape(currArgs["target"]) +  "&new_window=" + escape(currArgs["new_window"]) + "&new_window_options=" + escape(var_serialise(currArgs["new_window_options"])) + "&editor_name=" + escape("<?php echo $this->wysiwyg->name; ?>") + "&in_popup=" + escape(in_popup);

				this._popupDialog("matrixInsertLink", strPage, 750, 440, true, function(param) {
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

						html += '<a href="' + param["url"] + '"';
						if (param["target"] != "") {
							html += ' target="' + htmlspecialchars(param["target"]).replace(/\'/gi, "\\'") + '"';
						}
						if (param["status_text"] != "") {
							html += ' onMouseOver="javascript: window.status=\'' + htmlspecialchars(param["status_text"]).replace(/\'/gi, "\\'") + '\'; return true;" onMouseOut="javascript: window.status=\'\'; return true;"';
						}
						if (param["onClick"] != null && param["onClick"] != "") {
							html += ' onClick="' + param["onClick"] + '"';
						}
						html += '>' + inside_html + '</a>';
						html = editor.make_absolute_urls(html);

						// replace with new URL
						editor.insertHTML(html, range);

					}// end if param exists

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
		case "matrixcreatelink":
			this._matrixInsertLink();
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
