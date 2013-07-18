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
* $Id: matrix_insert_quick_link.inc,v 1.3.8.1 2013/07/18 05:12:40 akarelia Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - Matrix Insert Quick Link
*
* Purpose
*     A WYSIWYG plugin to make quick links from text
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.3.8.1 $
* @package Fudge
* @subpackage wysiwyg
*/
class Matrix_Insert_Quick_Link extends WYSIWYG_Plugin
{


	/**
	* Constructor
	* Adds the buttons for this plugins functions
	*
	* @returns object
	* @access  public
	*/
	function matrix_insert_quick_link()
	{
		$this->_add_button('matrixinsertquicklink','matrixInsertQuickLink','Create a Quick Link','false','68');
	}


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

			// Called when the user clicks on "InsertImage" button
			HTMLArea.prototype._matrixInsertQuickLink = function() {
				// declare some stuff
				var editor = this;
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

				// convert all para tags to lowercase
				var para_html = inside_html.replace(/(<\/?)P/gi, "$1p");

				// OK so we have some paragraph tags caught up in the mix, lets see what we can do with them
				// because we dont want our hrefs going around paragraph tags
				if (para_html.indexOf("<p") > -1) {

					// if the position of the first tag is the same as the last then we have only one paragraph
					// we don't want more than one paragraph because these can start or end from half-way through
					// a para, which would break the formatting when we go to re-insert the text with the href
					if (para_html.indexOf("<p") != para_html.lastIndexOf("<p")) {
						alert(js_translate('select_one_paragraph_at_a_time'));
						return;
					}

				}// end if paragraphs exist

				if (sel != "") {
					var link_url = "";

					// Is an email? Keep it an email, keep it mail
					var test = new String(sel);
					var mailregexp = new RegExp("^(.+)(\@)(.+)$", "gi");
					if (test.search(mailregexp) == -1) {
						checkhttplink = new RegExp("^http\:\/\/", "gi");
						if (test.search(checkhttplink) == -1) {
							checkanchorlink = new RegExp("^\#", "gi");
							if (test.search(checkanchorlink) == -1) {
								link_url = "http://" + sel;
							} else {
								link_url = sel;
							}
						} else {
							link_url = sel;
						}
					} else {
						checkmaillink = new RegExp("^mailto\:", "gi");
						if (test.search(checkmaillink) == -1) {
							link_url = "mailto:" + sel;
						} else {
							link_url = sel;
						}
					}

					var html_code = '<a href="' + link_url + '">' + sel + '</a>';

					// replace with new URL
					editor.insertHTML(html_code, range);
				}//end if

			};
		</script>
		<?php

	}//end paint_generic()


	/**
	* Create the link a URL entered by the user
	* IE has its own linking interface, while we use a JS prompt for Mozilla
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_exec_command()
	{
		?>
		case "createlink":
			this._matrixInsertQuickLink();
		break;
		<?php
	}


	/**
	* Call the insert image function when the button is clicked
	*
	* @return void
	* @access public
	*/
	function print_plugin_button_click()
	{
		?>
		case "matrixinsertquicklink":
			this._matrixInsertQuickLink();
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

	}//end print_plugin_update_toolbar()


}//end class

?>
