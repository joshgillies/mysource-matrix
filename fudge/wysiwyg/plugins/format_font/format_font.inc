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
* $Id: format_font.inc,v 1.11 2012/08/30 00:56:51 ewang Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - Format Font
*
* Purpose
*     A WYSIWYG plugin to change font size, family, and style
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.11 $
* @package Fudge
* @subpackage wysiwyg
*/
class format_font extends wysiwyg_plugin
{


	/**
	* Constructor
	* Adds a three blank buttons (later to become select boxes)
	*
	* @returns object Format_Font
	* @access  public
	*/
	function format_font()
	{
		$this->_add_button('fontname','','','','');
		$this->_add_button('fontsize','','','','');
		$this->_add_button('formatblock','','','','');
	}


	/**
	* Set the options for the select boxes as vars of the object
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_vars()
	{
		?>
		this.config.fontname = {
			"Arial":           'arial,helvetica,sans-serif',
			"Courier New":     'courier new,courier,monospace',
			"Georgia":         'georgia,times new roman,times,serif',
			"Tahoma":          'tahoma,arial,helvetica,sans-serif',
			"Times New Roman": 'times new roman,times,serif',
			"Verdana":         'verdana,arial,helvetica,sans-serif',
			"impact":          'impact',
			"WingDings":       'wingdings'
		};

		this.config.fontsize = {
			"1 (10 px)": "1",
			"2 (13 px)": "2",
			"3 (16 px)": "3",
			"4 (18 px)": "4",
			"5 (24 px)": "5",
			"6 (32 px)": "6",
			"7 (48 px)": "7"
		};

		this.config.formatblock = {
			"Heading 1": "h1",
			"Heading 2": "h2",
			"Heading 3": "h3",
			"Heading 4": "h4",
			"Heading 5": "h5",
			"Heading 6": "h6",
			"Normal": "p",
			"Address": "address",
			"Formatted": "pre"
		};
		<?php
	}


	/**
	* Populate the select boxes with the options stored in the WYSIWYGs vars (set ealier)
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_create_select()
	{
		?>
		case "fontsize":
		case "fontname":
		case "formatblock":
			options = editor.config[txt]; // HACK ;)
			cmd = txt;
		break;
		<?php
	}


	/**
	* When the toolbar is updated, change the selected option to reflact the selected text
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_update_toolbar()
	{
		?>
		case "fontname":
		case "fontsize":
		case "formatblock":
			if (!text) {
				if (!doc) break;
				var value = '';
				try {
					value = ("" + doc.queryCommandValue(cmd)).toLowerCase();
				} catch (e) {}
				if (!value) {
					break;
				}
				var options = this.config[i]; // HACK!!
				var k = 0;
				// btn.element.selectedIndex = 0;
				for (var j in options) {
					// FIXME: the following line is scary.
					if ((j.toLowerCase() == value) ||
						(options[j].substr(0, value.length).toLowerCase() == value)) {
						btn.element.selectedIndex = k;
						break;
					}
					++k;
				}
			}
		break;
		<?php
	}


	/**
	* When the select box option is changed, apply changes to selected HTML
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_combo_selected()
	{
		?>
		case "fontname":
		case "fontsize":
			this._execCommand(txt, false, value);
		break;
		case "formatblock":
			if (HTMLArea.is_ie || HTMLArea.is_post_ie11) { // sad but true
				value = "<" + value + ">";
			}
			this._execCommand(txt, false, value);
		break;
		<?php
	}


	/**
	* Shortcut keys for heading tags (H!, H2 etc)
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_shortcuts()
	{
		?>
		// headings
		case '1':	// KEY heading 1
		case '2':	// KEY heading 2
		case '3':	// KEY heading 3
		case '4':	// KEY heading 4
		case '5':	// KEY heading 5
		case '6':	// KEY heading 6
		cmd = "formatblock";
		value = "h" + key;
		if (HTMLArea.is_ie) {
			value = "<" + value + ">";
		}
		break;
		<?php
	}
}//end class

?>
