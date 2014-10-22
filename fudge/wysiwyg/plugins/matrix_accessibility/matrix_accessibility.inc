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
* $Id: matrix_accessibility.inc,v 1.16 2012/08/30 00:56:52 ewang Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');
require_once dirname(__FILE__).'/../../../../core/include/init.inc';

/**
* WYSIWYG Plugin - Matrix Accessibility
*
* Purpose
*     A WYSIWYG plugin to allow the user to insert WCAG
*     Triple-A level accessibility tags (e.g. abbr and acronym)
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.16 $
* @package MySource_Matrix
* @subpackage wysiwyg
*/
class matrix_accessibility extends wysiwyg_plugin
{

	/**
	* Constructor
	* Adds the insert anchor button to this plugin
	*
	* @returns object Insert_Anchor
	* @access  public
	*/
	function matrix_accessibility(&$wysiwyg)
	{
		wysiwyg_plugin::wysiwyg_plugin($wysiwyg);
		$this->_add_button('matrixabbr','MatrixAbbr','Insert Abbreviation','false','64');
		$this->_add_button('matrixacronym','MatrixAcronym','Insert Acronym','false','66');

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

			// Called when the user clicks on "Insert Abbreviation" button
			HTMLArea.prototype._matrixInsertAbbr = function() {
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
				var current_abbr = this.getClosest("abbr");
				if (current_abbr) {
					if (HTMLArea.is_ie) {
						inside_html = current_abbr.innerHTML;
					} else {
						this.selectNodeContents(current_abbr);
						inside_html = editor.getSelectedHTML();
					}
					this.selectNodeContents(current_abbr);
				} else {
					inside_html = editor.getSelectedHTML();
				}

				sel = this._getSelection();
				range = this._createRange(sel);

				// Is this aready an abbreviation?
				if (current_abbr) {

					var e = '^(\<abbr [^\>]+>)(.*)</abbr>$';
					var re = new RegExp(e, "i");
					var tag_html = editor.getSelectedHTML().replace(re, "$1");

					var re = new RegExp("\n", "gi");
					var tag_html = tag_html.replace(re, "");

					currArgs["title"] = current_abbr.title;

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
						// a para, which would break the formatting when we go to re-insert the text with the href
						if (para_html.indexOf("<p") != para_html.lastIndexOf("<p")) {
							alert(js_translate('Please select only one paragraph to link at a time.'));
							return;
						}

					}// end if paragraphs exist

				}// end if existing link

				var in_popup = '0';
				if (this._inPopup == true) { in_popup = '1'; }

				editor_<?php echo $this->wysiwyg->name?>._object = this;
				var strPage = "<?php echo $this->get_popup_href('insert_abbr.php')?>?&title=" + escape(currArgs["title"]) + "&abbr=" + escape(inside_html) +"&editor_name=" + escape("<?php echo $this->wysiwyg->name; ?>") + "&in_popup=" + escape(in_popup);

				var popup_width  = 410;

				// A bit more breathing room for Safari
				if (HTMLArea.is_safari) popup_width += 15;

				this._popupDialog("matrixInsertAbbr", strPage, popup_width, 170, true, function(param) {
					if (!param) {
						// user must have pressed Cancel
						return false;
					}

					if (param["title"] != "") {

						if (param["abbr"] != inside_html) { inside_html = param["abbr"]; }

						var html = '<abbr title="' + param["title"] + '">' + inside_html + '</abbr>';

						// Update abbreviation if selected, otherwise create a new one
						if (current_abbr) {
							current_abbr.title = param["title"];
							current_abbr.innerHTML = inside_html;
						} else {
							if (HTMLArea.is_gecko) {
								editor.insertHTML(html, range);
							} else if (HTMLArea.is_ie) {
								range.pasteHTML(html);
							}
						}


					}// end if param exists

				}, null);

			};

			// Called when the user clicks on "Insert Acronym" button
			HTMLArea.prototype._matrixInsertAcronym = function() {
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
				var current_acronym = this.getClosest("acronym");
				if (current_acronym) {
					if (HTMLArea.is_ie) {
						inside_html = current_acronym.innerHTML;
					} else {
						this.selectNodeContents(current_acronym);
						inside_html = editor.getSelectedHTML();
					}
					this.selectNodeContents(current_acronym);
				} else {
					inside_html = editor.getSelectedHTML();
				}

				sel = this._getSelection();
				range = this._createRange(sel);

				// Is this aready an abbreviation?
				if (current_acronym) {

					var e = '^(\<acronym [^\>]+>)(.*)</acronym>$';
					var re = new RegExp(e, "i");
					var tag_html = editor.getSelectedHTML().replace(re, "$1");

					var re = new RegExp("\n", "gi");
					var tag_html = tag_html.replace(re, "");

					currArgs["title"] = current_acronym.title;

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
				var strPage = "<?php echo $this->get_popup_href('insert_acronym.php')?>?&title=" + escape(currArgs["title"]) + "&acronym=" + escape(inside_html) +"&editor_name=" + escape("<?php echo $this->wysiwyg->name; ?>") + "&in_popup=" + escape(in_popup);

				var popup_width  = 410;

				// A bit more breathing room for Safari
				if (HTMLArea.is_safari) popup_width += 15;

				this._popupDialog("matrixInsertAcronym", strPage, popup_width, 170, true, function(param) {
					if (!param) {
						// user must have pressed Cancel
						return false;
					}

					if (param["title"] != "") {

						if (param["acronym"] != inside_html) { inside_html = param["acronym"]; }

						var html = '<acronym title="' + param["title"] + '">' + inside_html + '</acronym>';

						// Update acronym if selected, otherwise create a
						if (current_acronym) {
							current_acronym.title = param["title"];
							current_acronym.innerHTML = inside_html;
						} else {
							editor.insertHTML(html, range);
						}

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
		case "matrixabbr":
			this._matrixInsertAbbr();
		break;
		case "matrixacronym":
			this._matrixInsertAcronym();
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
		case "matrixabbr":
			if (allEnabled == null && this._tmp['disable_toolbar'] == true) {
				this.updateToolbar(false);
				return;
			}
		break;
		case "matrixacronym":
			if (allEnabled == null && this._tmp['disable_toolbar'] == true) {
				this.updateToolbar(false);
				return;
			}
		break;
		<?php

	}//end print_plugin_update_toolbar()

}//end class

?>
