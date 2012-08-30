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
* $Id: definition_list.inc,v 1.5 2012/08/30 00:56:51 ewang Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');
require_once dirname(__FILE__).'/../../../../core/include/init.inc';

/**
* WYSIWYG Plugin - Definition List
*
* Purpose
*     A WYSIWYG plugin to allow the user to insert
*     some extra tags like <cite> and <dfn>
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix
* @subpackage wysiwyg
*/
class definition_list extends wysiwyg_plugin
{

	/**
	* Constructor
	* Adds the insert anchor button to this plugin
	*
	* @returns object Insert_Anchor
	* @access  public
	*/
	function definition_list(&$wysiwyg)
	{
		wysiwyg_plugin::wysiwyg_plugin($wysiwyg);
		$this->_add_button('matrix_dt','Matrix_DT','Insert Definition Term','false','63');
		$this->_add_button('matrix_dd','Matrix_DD','Insert Definition Description','false','62');

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
		<script type="text/javascript" language="Javascript">

			HTMLArea.prototype._definitionListFilter = function(node) {
				switch (node.tagName) {
					case 'P':
					case 'DT':
					case 'DD' :
						return true;
					break;
					default :
						return false;
					break;
				}
			}
			// Generate the definition list.
			// This function is called when some text is selected in the WYSIWYG.
			// It attempts to create the definition list from the selected text.
			HTMLArea.prototype._definitionListWithSelection = function(type) {
				var editor = this; // for nested functions
				var sel = this._getSelection();
				var range = this._createRange(sel);
				var parent = this.getParentElement();
				var type_string = ((type == "DT") ? "term" : "definition");

				var selected_html = "";
				if (HTMLArea.is_gecko) {
					var parent_node = range.startContainer.parentNode;

					var start_node = range.startContainer;
					var end_node = range.endContainer;
					var start_text = "";
					var end_text = "";
					if (start_node.nodeType == 1) {
						start_text = start_node.innerHTML;
					} else if (start_node.nodeType == 3) {
						start_text = start_node.nodeValue;
					}
					if (end_node.nodeType == 1) {
						end_text = end_node.innerHTML;
					} else if (end_node.nodeType == 3) {
						end_text = end_node.nodeValue;
					}
					while (this._definitionListFilter(parent_node)) {
						parent_node = parent_node.parentNode;
					}
					var start_index = 0;
					var end_index = 0;
					var children = parent.childNodes;
					var valid_node_indexes = Array();
					for (var i=0; i < children.length; i++) {
						if (children[i].nodeType == 1 && children[i].innerHTML != "") {
							if (children[i].innerHTML == start_text) start_index = i;
							else if (children[i].innerHTML == end_text) end_index = i;
							valid_node_indexes.push(i);
						} else if (children[i].nodeType == 3) {
							var tmp = children[i].nodeValue.replace(/\s/g, "");
							if (tmp != "") {
								if (tmp == start_text.replace(/\s/g, "")) start_index = i;
								else if (tmp == end_text.replace(/\s/g, "")) end_index = i;
								valid_node_indexes.push(i);
							}
						}
					}
					// Create a definition list
					dl = this._doc.createElement("DL");
					var remove_list = Array();
					for (var j=0; j < valid_node_indexes.length; j++) {
						var index = valid_node_indexes[j];
						if (index >= start_index && index <= end_index) {
							if (children[index].tagName == "DT" || children[index].tagName == "DD") {
								if (type_string == children[index].tagName) continue;
								tmp = this._doc.createElement(type);
								tmp.innerHTML = children[index].innerHTML;
								parent_node.replaceChild(tmp, children[index]);
							} else {
								tmp = this._doc.createElement(type);
								if (children[index].nodeType == 1 || children[index].nodeType == 3) {
									switch (children[index].nodeType) {
										case 1:
											tmp.innerHTML = children[index].innerHTML;
										break;
										case 3:
											tmp.innerHTML = children[index].nodeValue;
										break;
									}
									dl.appendChild(tmp);
									remove_list.push(children[index]);
								}
							}
						}
					}
					if (dl.childNodes.length > 0) {
						parent_node.insertBefore(dl, children[start_index]);
						for (var k=0; k < remove_list.length; k++) {
							parent_node.removeChild(remove_list[k]);
						}
					}
				} else if (HTMLArea.is_ie) {
					var selected_html = range.htmlText;
					var tmp = "";
					var new_html = "";

					var e		= '<(P|DT|DD)>(.+)<\/(P|DT|DD)>|([^>]+)<BR>|([^>]+)';
					var re		= new RegExp(e, "ig");
					var p_re	= new RegExp('<(P)>([^>]+)<\/(P)>', "ig");
					var d_re	= new RegExp('<(DT|DD)>([^>]+)<\/(DT|DD)>', "ig");
					var b_re	= new RegExp('([^>]+)<BR>', "ig");
					var n_re	= new RegExp('([^>]+)', "ig");

					var surround_dl	= false;
					var warning		= false;

					var result = selected_html.match(re);
					for (var i = 0; i < result.length; i++) {
						var curr_type = result[i].replace(re, "$1");

						if (result[i].match(p_re)) {
							tmp += result[i].replace(p_re, "<" + type + ">$2<\/" + type + ">");
							surround_dl = true;
						} else if (result[i].match(d_re)) {
							tmp += result[i].replace(d_re, "<" + type + ">$2<\/" + type + ">");
							if (surround_dl) {
								warning = true;
								break;
							}
						} else if (result[i].match(b_re)) {
							tmp += result[i].replace(b_re, "<" + type + ">$1<\/" + type + ">");
							surround_dl = true;
						} else if (result[i].match(n_re)) {
							tmp += result[i].replace(n_re, "<" + type + ">$1<\/" + type + ">");
							surround_dl = true;
						}
					}

					if (!warning) {
						if (surround_dl) {
							new_html = '<DL>' + tmp + '</DL>';
						} else if (!surround_dl) {
							new_html = tmp;
						}
						range.pasteHTML(new_html);
					} else {
						alert("Please do not select DD or DT tags with other types of tags.");
					}
				}
			}

			// Generate the definition list.
			// This function is called when there is no selection in the WYSIWYG.
			// It creates a new definition list
			HTMLArea.prototype._definitionList = function(type) {
				var sel = this._getSelection();
				var range = this._createRange(sel);
				var parent = this.getParentElement();
				var type_string = ((type == "DT") ? "term" : "definition");

				if (parent.tagName != "DT" && parent.tagName != "DD") {

					// start a new definition list
					var dl = this._doc.createElement("dl");
					var child = this._doc.createElement(type.toLowerCase());

					var text = this._doc.createTextNode("Type the " + type_string);

					child.appendChild(text);
					dl.appendChild(child);
					parent.appendChild(dl);
					if (HTMLArea.is_gecko) {
						range.selectNode(text);
						//range.collapse(true);
						sel.removeAllRanges();
						sel.addRange(range);
					} else if (HTMLArea.is_ie) {
						range.findText("Type the " + type_string);
						range.select();
					}

				} else {

					var dl = parent.parentNode;
					var dl_parent = dl.parentNode;
					if (HTMLArea.is_gecko) {
						var data = parent.firstChild.data;
					} else if (HTMLArea.is_ie) {
						var data = parent.innerText;
					}
					if (type == parent.tagName) {
						// the same button is pressed, remove any tag
						dl.removeChild(parent);
						if (data != null && data != "Type the " + type_string) {
							var new_text = this._doc.createTextNode(data);
							dl_parent.appendChild(new_text);
						}
						if (dl.childNodes.length == 0) {
							dl_parent.removeChild(dl);
						}
					} else if (type != parent.tagName) {
						// the different tag is pressed, add a new one
						var new_child = this._doc.createElement(type);
						var next_type_string = ((type == "DT") ? "term" : "definition");
						var prev_type_string = ((type == "DT") ? "definition" : "term");
						if (data != ("Type the " + prev_type_string)) {
							var new_text = this._doc.createTextNode(data);
						} else {
							var new_text = this._doc.createTextNode("Type the " + next_type_string);
						}

						new_child.appendChild(new_text);
						dl.replaceChild(new_child, parent);
					}
				}
			}
			HTMLArea.prototype._handleDefinitionShortcut = function() {
				var editor = this; // for nested functions
				var sel = this._getSelection();
				var range = this._createRange(sel);
				var parent = this.getParentElement();

				var dl = parent.parentNode;
				if (HTMLArea.is_gecko) {
					var data = parent.firstChild.data;
				} else if (HTMLArea.is_ie) {
					var data = parent.innerText;
				}

				var next_type = (parent.tagName == "DT") ? "DD" : "DT";
				var prev_type_string = ((next_type == "DT") ? "definition" : "term");
				var type_string = ((next_type == "DT") ? "term" : "definition");

				if (dl.tagName == "DL") {
					if (data == "" || data == "Type the " + prev_type_string) {
						// exit
						var dl_parent = dl.parentNode;
						dl.removeChild(parent);
						if (data != null && data != "Type the " + prev_type_string) {
							var new_text = this._doc.createTextNode(data);
							dl_parent.appendChild(new_text);
						}
						if (dl.childNodes.length == 0) {
							dl_parent.removeChild(dl);
						}
					} else {
						var new_child = this._doc.createElement(next_type.toLowerCase());
						var new_text = this._doc.createTextNode("Type the " + type_string);
						new_child.appendChild(new_text);
						dl.appendChild(new_child);

						if (HTMLArea.is_gecko) {
							// open a new tag
							range.selectNode(new_text);
							sel.removeAllRanges();
							sel.addRange(range);
						} else if (HTMLArea.is_ie) {
							range.findText("Type the " + type_string);
							range.select();
						}
					}
				}
			}

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

		case "matrix_dt":
			var sel = this._getSelection();
			var range = this._createRange(sel);
			var range_comp = "";
			if (HTMLArea.is_gecko) {
				range_comp = range;
			} else if (HTMLArea.is_ie) {
				range_comp = range.text;
			}
			if (range_comp == "" || range_comp == "Type the term" || range_comp == "Type the definition") {
				this._definitionList("DT");
			} else {
				this._definitionListWithSelection("DT");
			}
		break;
		case "matrix_dd":
			var sel = this._getSelection();
			var range = this._createRange(sel);
			var range_comp = "";
			if (HTMLArea.is_gecko) {
				range_comp = range;
			} else if (HTMLArea.is_ie) {
				range_comp = range.text;
			}
			if (range_comp == "" || range_comp == "Type the term" || range_comp == "Type the definition") {
				this._definitionList("DD");
			} else {
				this._definitionListWithSelection("DD");
			}
		break;
		<?php

	}//end print_plugin_button_click()


	/**
	* Prints shortcuts for the buttons
	*
	* All of these except strikethrough are supported nativly in IE
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_shortcuts()
	{
		?>
		case 'd':
			this._handleDefinitionShortcut();
			HTMLArea._stopEvent(ev);
		break;
		<?php

	}//end print_plugin_shortcuts()


}//end class

?>
