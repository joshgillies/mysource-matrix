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
* $Id: misc_accessibility.inc,v 1.6 2012/08/30 00:56:52 ewang Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');
require_once dirname(__FILE__).'/../../../../core/include/init.inc';

/**
* WYSIWYG Plugin - Miscellaneous Accessibility
*
* Purpose
*     A WYSIWYG plugin to allow the user to insert
*     some extra tags like <cite> and <dfn>
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix
* @subpackage wysiwyg
*/
class misc_accessibility extends wysiwyg_plugin
{

	/**
	* Constructor
	* Adds the insert anchor button to this plugin
	*
	* @returns object Insert_Anchor
	* @access  public
	*/
	function misc_accessibility(&$wysiwyg)
	{
		wysiwyg_plugin::wysiwyg_plugin($wysiwyg);
		$this->_add_button('misccite','MiscCite','Insert Citation','false','58');
		$this->_add_button('miscdfn','MiscDfn','Insert Definition','false','59');

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

			// Called when the user clicks on "Insert Citation" button
			HTMLArea.prototype._matrixInsertCite = function() {
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
						alert(js_translate('You need to select this object as though it was text, rather than just clicking on it.'));
						sel.empty();
						return;
					}
					sel = this._getSelection();
					range = this._createRange(sel);
				}// end if a control

				var inside_html = '';
				var current_cite = this.getClosest("cite");
				if (current_cite) {
					if (HTMLArea.is_ie) {
						inside_html = current_cite.innerHTML;
					} else {
						this.selectNodeContents(current_cite);
						inside_html = editor.getSelectedHTML();
					}
					this.selectNodeContents(current_cite);
				} else {
					inside_html = editor.getSelectedHTML();
				}
				sel = this._getSelection();
				range = this._createRange(sel);

				// remove nested cite tags in selection
				var reg = new RegExp("<\/?cite ?[^>]*>", "gi");
				inside_html = inside_html.replace(reg, "");

				// if citation exists, remove it
				if (current_cite) {
					var text = this._doc.createDocumentFragment();
					// the following also deletes the selection
					var div = this._doc.createElement("div");
					div.innerHTML = inside_html;
					while (div.firstChild) {
						text.appendChild(div.firstChild);
					}
					current_cite.parentNode.replaceChild(text, current_cite);
				// else insert the citation
				} else {
					if (inside_html != '') {
						this.insertHTML('<cite>' + inside_html + '</cite>');
					}
				}
			};


			// Called when the user clicks on "Insert Definition" button
			HTMLArea.prototype._matrixInsertDfn = function() {
				var editor = this; // for nested functions

				// OK, let's create the current args array that we'll pass to the dialog
				var currArgs = new Object();
				currArgs["title"] = "";

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
						alert(js_translate('You need to select this object as though it was text, rather than just clicking on it.'));
						sel.empty();
						return;
					}
					sel = this._getSelection();
					range = this._createRange(sel);
				}// end if a control

				var inside_html = '';
				var current_dfn = this.getClosest("dfn");
				if (current_dfn) {
					if (HTMLArea.is_ie) {
						inside_html = current_dfn.innerHTML;
					} else {
						this.selectNodeContents(current_dfn);
						inside_html = editor.getSelectedHTML();
					}
					this.selectNodeContents(current_dfn);
				} else {
					inside_html = editor.getSelectedHTML();
				}

				sel = this._getSelection();
				range = this._createRange(sel);

				// Is this aready an definition?
				if (current_dfn) {

					var e = '^(\<dfn [^\>]+>)(.*)</dfn>$';
					var re = new RegExp(e, "i");
					var tag_html = editor.getSelectedHTML().replace(re, "$1");

					var re = new RegExp("\n", "gi");
					var tag_html = tag_html.replace(re, "");

					currArgs["title"] = current_dfn.title;

					re = new RegExp('if \\(!window\\.wysiwyg_editing\\)[\\s]+\\{[\\s]+window\\.open\\([\'"]([^\'"]+)[\'"], [\'"][0-9]+[\'"], [\'"]([^\'"]+)[\'"]\\);', "gi");
					results = re.exec(tag_html);

					if (results != null) {
						currArgs["title"] = results[1];
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
						// a para, which would mess up the formatting when we go to re-insert the text with the href
						if (para_html.indexOf("<p") != para_html.lastIndexOf("<p")) {
							alert(js_translate('Please select only one paragraph to link at a time.'));
							return;
						}

					}// end if paragraphs exist

				}// end if existing link

				var in_popup = '0';
				if (this._inPopup == true) { in_popup = '1'; }

				editor_<?php echo $this->wysiwyg->name?>._object = this;
				var strPage = "<?php echo $this->get_popup_href('insert_dfn.php')?>?&title=" + escape(currArgs["title"]) + "&dfn=" + escape(inside_html) +"&editor_name=" + escape("<?php echo $this->wysiwyg->name; ?>") + "&in_popup=" + escape(in_popup);

				this._popupDialog("matrixInsertDfn", strPage, 420, 170, true, function(param) {
					if (!param) {
						// user must have pressed Cancel
						return false;
					}

					if (param["title"] != "") {

						if (param["dfn"] != inside_html) { inside_html = param["dfn"]; }

						var html = '<dfn title="' + param["title"] + '">' + inside_html + '</dfn>';

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
		case "misccite":
			this._matrixInsertCite();
		break;
		case "miscdfn":
			this._matrixInsertDfn();
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
		case "misccite":
			if (allEnabled == null && this._tmp['disable_toolbar'] == true) {
				this.updateToolbar(false);
				return;
			}
		break;
		case "miscdfn":
			if (allEnabled == null && this._tmp['disable_toolbar'] == true) {
				this.updateToolbar(false);
				return;
			}
		break;
		<?php

	}//end print_plugin_update_toolbar()

}//end class

?>
