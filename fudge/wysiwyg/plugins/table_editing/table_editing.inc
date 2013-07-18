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
* $Id: table_editing.inc,v 1.32.8.2 2013/07/18 05:30:23 akarelia Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - Table Editing
*
* Purpose
*     A WYSIWYG plugin to allow the user to create
*     and manipulate tables
*
* @author  Greg Sherwood <greg@squiz.net>
* @author  Darren McKee <dmckee@squiz.net>
* @version $Version$ - 1.0
* @package Fudge
* @subpackage wysiwyg
*/
class Table_Editing extends WYSIWYG_Plugin
{


	/**
	* Constructor
	* Adds buttons needed for table editing
	*
	* @returns object Table_Editing
	* @access  public
	*/
	function Table_Editing(&$wysiwyg)
	{
		wysiwyg_plugin::wysiwyg_plugin($wysiwyg);
		$this->_add_button('inserttable','InsertTable','Insert Table','false','35');
		$this->_add_button('tableproperties','TableProperties','Table Properties','false','36','table');

	}//end constructor


	/**
	* Prints vars needed by the table editing functions
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_vars()
	{
		?>
		this.config._showborders = true;
		<?php

	}//end print_plugin_vars()


	/**
	* Prints generic functions to insert an manipulate tables
	*
	* @returns void
	* @access  public
	*/
	function paint_generic()
	{
		?>
		<script type="text/javascript">
		//<![CDATA[
		//global for holding table data. Used to workaround 4096 byte limit
		//IE imposes on dialogArguments.
		var table_structure;
			// helper function that clears the content in a table row
			HTMLArea.prototype.clearRow = function(tr) {
				var mozbr = HTMLArea.is_gecko ? "<br />" : "";
				var tds = tr.getElementsByTagName("td");
				for (var i = tds.length; --i >= 0;) {
					var td = tds[i];
					td.rowSpan = 1;
					td.innerHTML = mozbr;
				}
			};


			HTMLArea.prototype.selectNextNode = function(el) {
				var node = el.nextSibling;
				while (node && node.nodeType != 1) {
					node = node.nextSibling;
				}
				if (!node) {
					node = el.previousSibling;
					while (node && node.nodeType != 1) {
						node = node.previousSibling;
					}
				}
				if (!node) {
					node = el.parentNode;
				}
				this.selectNodeContents(node);
			};


			// Called when the user clicks the Insert Table button
			HTMLArea.prototype._insertTable = function() {
				var editor = this; // for nested functions

				// set focus in case we havnt selected anything
				if (HTMLArea.is_ie) {
					this._docContent.focus();
				} else if (HTMLArea.is_gecko) {
					this.focusEditor();
				}
				var sel = this._getSelection();
				var range = this._createRange(sel);
				this._popupDialog
				("insertTable", "<?php echo $this->get_popup_href('insert_table.html')?>", 400, 500, true, function(param) {
					if (!param) { // user must have pressed Cancel
						return false;
					}
					var doc = editor._doc;
					// create the table element
					var table = doc.createElement("table");

					var row_header = param["f_headerRow"];
					var col_header = param["f_headerCol"];

					// assign the given arguments
					for (var field in param) {
						var value = param[field];
						//alert("value="+value+"\nfield="+field);
						if (!value) {
							continue;
						}
						switch (field) {
							case "f_class":
								if (value != "") {
									table.className = value;
								}
							break;

							case "f_width":
								table.style.width = value + param["f_widthUnit"];
							break;

							case "f_border":
								table.border = parseInt(value);
							break;

							case "f_spacing":
								table.cellSpacing = parseInt(value);
							break;

							case "f_padding":
								table.cellPadding = parseInt(value);
							break;

							case "f_summary":
								if (value != "") {
									table.setAttribute("summary", value);
								}
							break;
						}
					}

					if (row_header && param["f_rows"] != 0) {
						var thead = doc.createElement("thead");
						table.appendChild(thead);
					}

					var tbody = doc.createElement("tbody");
					table.appendChild(tbody);
					for (var i = 0; i < param["f_rows"]; ++i) {
						var tr = doc.createElement("tr");
						if (row_header && i == 0) {
							thead.appendChild(tr);
						} else {
							tbody.appendChild(tr);
						}
						for (var j = 0; j < param["f_cols"]; ++j) {
							var td = null;
							// create a table header
							if ((col_header && j == 0)
								|| (row_header && i == 0)) {
								td = doc.createElement("th");
							} else {
								td = doc.createElement("td");
							}
							tr.appendChild(td);
							if (HTMLArea.is_gecko) {
								// Mozilla likes to see something
								// inside the cell.
								td.appendChild(doc.createElement("br"));
							} else if (HTMLArea.is_ie) {
								// to overcome that IE will not close with </th> tag for empty content
								td.appendChild(doc.createTextNode(''));								
							}
						}
					}

					if (HTMLArea.is_ie) {
						range.pasteHTML(HTMLArea.getHTML(table, true));
					} else {
						// insert the table
						editor.insertNodeAtSelection(table);
					}
					return true;
				}, null);
			};


			// this function requires the file PopupDiv/PopupWin to be loaded from browser
			HTMLArea.prototype._editTableProperties = function() {
				var editor = this; // for nested functions

				// retrieve existing values
				var table = this.getClosest("table");
				if (HTMLArea.is_ie) {
					//workaround against stupid IE behaviour. We will use native MS functions
					var tablekeeper = table.outerHTML;
				} else {
					var div = document.createElement("DIV");
					div.appendChild(table.cloneNode(true));
					var tablekeeper = div.innerHTML;
				}
				table_structure = tablekeeper;
				strPage = "<?php echo $this->get_popup_href('edit_table.php')?>";
				var width = 835;
				var height = 730;

				this._popupDialog("editTableProperties", strPage, width, height, true, function(params) {
					if (!params) {
						// user must have pressed Cancel
						return false;
					}
					var div = document.createElement("DIV");
					div.innerHTML = params;
					var table = div.getElementsByTagName("TABLE")[0];
					var old_table = editor.getClosest("TABLE");
					if (HTMLArea.is_ie) {
						old_table.outerHTML = table.outerHTML;
					} else {
						old_table.parentNode.replaceChild(table, old_table);
					}
					// various workarounds to refresh the table display (Gecko,
					// what's going on?! do not disappoint me!)
					editor.forceRedraw();
					editor.focusEditor();
					editor.updateToolbar();
					var save_collapse = table.style.borderCollapse;
					table.style.borderCollapse = "collapse";
					table.style.borderCollapse = "separate";
					table.style.borderCollapse = save_collapse;
				}, window);
				//above: window is passed, so that tableStructure can be accessed by the called dialog. avoids dialogArguments limit
			};

			//Functions for showing the table borders
			HTMLArea.prototype.storedBorders = Array();

			this.clonedBorder = function() {

				this.cloneSaveBorder = function(border) {
					for (i in border) {
						this[i] = border[i];
					}
				}

			}

			HTMLArea._restoreBorder = function(elt, stored)
			{
				if (stored.borderLeft) {
					elt.style.borderLeft = stored.borderLeft;
				}
				if (stored.borderRight) {
					elt.style.borderRight = stored.borderRight;
				}
				if (stored.borderTop) {
					elt.style.borderTop = stored.borderTop;
				}
				if (stored.borderBottom) {
					elt.style.borderBottom = stored.borderBottom;
				}
			}
		//]]>
		</script>
		<?php

	}//end paint_generic()


	/**
	* Call the appropriate function when a button is clicked
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_button_click()
	{
		?>
		case "inserttable":
			this._insertTable();
			if (this.config._showborders){
				if (HTMLArea.is_ie) {
					var doco = this._docContent;
				} else {
					var doco = this._iframe.contentWindow;
				}
				this.config._showborders = true;
			}
		break;

		case "tableproperties":
			this._editTableProperties();
		break;
		<?php

	}//end print_plugin_button_click()


	/**
	* When the toolbar is updated, show or hide table borders
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_update_toolbar()
	{

	}//end print_plugin_update_toolbar()


	/**
	* Removes empty class tags that the table borders button may have left
	* This code is a function - do not use script tags
	*
	* @return void
	* @access public
	*/
	function print_plugin_get_html()
	{
		?>
		retVal = retVal.replace(/class=""/gi, '');
		<?php
		return;

	}//end print_plugin_get_html()


}//end class

?>
