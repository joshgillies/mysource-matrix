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
* $Id: matrix_insert_anchor.inc,v 1.3 2004/09/09 02:09:28 amiller Exp $
* $Name: not supported by cvs2svn $
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
* @version $Version$ - 1.0
* @package MySource_Matrix
* @subpackage wysiwyg
*/
class matrix_insert_anchor extends wysiwyg_plugin {

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
		$this->_add_button('matrixcreateanchor','MatrixCreateAnchor','Insert Anchor','false','30');
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
					if (HTMLArea.is_ie) {
						inside_html = current_a.innerHTML;
					} else {
						this.selectNodeContents(current_a);
						inside_html = editor.getSelectedHTML();
					}
					this.selectNodeContents(current_a);
				} else {
					inside_html = editor.getSelectedHTML();
				}

				sel = this._getSelection();
				range = this._createRange(sel);

				// Is this aready a link?
				if (current_a) {

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
				var strPage = "<?php echo $this->get_popup_href('insert_anchor.php')?>?&name=" + escape(currArgs["name"]) + "&editor_name=" + escape("<?php echo $this->wysiwyg->name; ?>") + "&in_popup=" + escape(in_popup);

				this._popupDialog("matrixInsertAnchor", strPage, 360, 180, true, function(param) {
					if (!param) { 
						// user must have pressed Cancel
						return false;
					}
					
					if (param["name"] != "") {
		
						var html = '<a name="' + param["name"] + '"';
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
