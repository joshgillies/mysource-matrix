<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: table_editing.inc,v 1.11 2003/10/20 04:37:56 gsherwood Exp $
* $Name: not supported by cvs2svn $
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
* @version $Version$ - 1.0
* @package Fudge
* @subpackage wysiwyg
*/
class table_editing extends wysiwyg_plugin {

	/**
	* Constructor
	* Adds buttons needed for table editing
	*
	* @returns object Table_Editing
	* @access  public
	*/
	function table_editing(&$wysiwyg)
	{
		wysiwyg_plugin::wysiwyg_plugin($wysiwyg);
		$this->_open_menu('tablefunctions', 'Table Functions','false','35');
			$this->_add_button('inserttable','InsertTable','Insert Table','false','35');
			$this->_add_button('tableproperties','TableProperties','Table Properties','false','36','table');
		$this->_close_menu('tablefunctions');
		
		$this->_open_menu('rowfunctions', 'Row Functions','false','38','tr');
			$this->_add_button('rowproperties','RowProperties','Row Properties','false','38','tr');
			$this->_add_button('insertrowbefore','InsertRowBefore','Insert Row Before','false','39','tr');
			$this->_add_button('insertrowafter','InsertRowAfter','Insert Row After','false','40','tr');
			$this->_add_button('deleterow','DeleteRow','Delete Row','false','41','tr');
			$this->_add_button('splitrow','SplitRow','Split Row','false','42','td[rowSpan!=1]');
			$this->_add_button('mergerows','MergeRows','Merge Rows','false','43','tr');
		$this->_close_menu('rowfunctions');
		
		$this->_open_menu('columnfunctions', 'Column Functions','false','44','td');
			$this->_add_button('insertcolumnbefore','InsertColumnBefore','Insert Column Before','false','44','td');
			$this->_add_button('insertcolumnafter','InsertColumnAfter','Insert Column Afer','false','45','td');
			$this->_add_button('deletecolumn','DeleteColumn','Delete Column','false','46','td');
			$this->_add_button('splitcolumn','SplitColumn','Split Column','false','46','td[colSpan!=1]');
		$this->_close_menu('columnfunctions');

		$this->_open_menu('cellfunctions', 'Cell Functions','false','47','td');
			$this->_add_button('cellproperties','CellProperties','Cell Properties','false','47','td');
			$this->_add_button('insertcellbefore','InsertCellBefore','Insert Cell Before','false','48','td');
			$this->_add_button('insertcellafter','InsertCellAfter','Insert Cell After','false','49','td');
			$this->_add_button('deletecell','DeleteCell','Delete Cell','false','50','td');
			$this->_add_button('splitcell','SplitCell','Split Cell','false','51','td');
			$this->_add_button('mergecells','MergeCells','Merge Cells','false','52','tr');
		$this->_close_menu('cellfunctions');

		$this->_add_button('showborders','ShowBorders','Show 0 borders','false','37');

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
		this.config._showborders = false;
		<?php

	}//end print_plugin_vars()


	/**
	* Prints generic functions to insert an manipulate tables
	*
	* @returns void
	* @access  public
	*/
	function paint_generic() {
		?>
		<script type="text/javascript" language="Javascript">
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

			HTMLArea.prototype.splitRow = function(td) {
				var mozbr = HTMLArea.is_gecko ? "<br />" : "";
				var n = parseInt("" + td.rowSpan);
				var nc = parseInt("" + td.colSpan);
				td.rowSpan = 1;
				tr = td.parentNode;
				var itr = tr.rowIndex;
				var trs = tr.parentNode.rows;
				var index = td.cellIndex;
				while (--n > 0) {
					tr = trs[++itr];
					var otd = this._doc.createElement("td");
					otd.colSpan = td.colSpan;
					otd.innerHTML = mozbr;
					tr.insertBefore(otd, tr.cells[index]);
				}
			};

			HTMLArea.prototype.splitCol = function(td) {
				var mozbr = HTMLArea.is_gecko ? "<br />" : "";
				var nc = parseInt("" + td.colSpan);
				td.colSpan = 1;
				tr = td.parentNode;
				var ref = td.nextSibling;
				while (--nc > 0) {
					var otd = this._doc.createElement("td");
					otd.rowSpan = td.rowSpan;
					otd.innerHTML = mozbr;
					tr.insertBefore(otd, ref);
				}
			};

			HTMLArea.prototype.splitCell = function(td) {
				var nc = parseInt("" + td.colSpan);
				this.splitCol(td);
				var items = td.parentNode.cells;
				var index = td.cellIndex;
				while (nc-- > 0) {
					this.splitRow(items[index++]);
				}
			};

			// Called when the user clicks the Insert Table button
			HTMLArea.prototype._insertTable = function() {
				var editor = this; // for nested functions
				
				// set focus in case we havnt selected anything
				if (HTMLArea.is_ie) {
					this._docContent.focus();
				} else if (HTMLArea.ie_gecko) {
					this.focusEditor();
				}

				var sel = this._getSelection();
				var range = this._createRange(sel);
				this._popupDialog("insertTable", "<?php echo $this->get_popup_href('insert_table.html')?>", 340, 260, true, function(param) {
					if (!param) { // user must have pressed Cancel
						return false;
					}
					var doc = editor._doc;
					// create the table element
					var table = doc.createElement("table");
					// assign the given arguments
					for (var field in param) {
						var value = param[field];
						if (!value) {
							continue;
						}
						switch (field) {
							case "f_width":
								table.style.width = value + param["f_widthUnit"];
							break;

							case "f_border":
								table.border = parseInt(value);
							break;

							case "f_spacing":
								table.cellspacing = parseInt(value);
							break;

							case "f_padding":
								table.cellpadding = parseInt(value);
							break;
						}
					}
					var tbody = doc.createElement("tbody");
					table.appendChild(tbody);
					for (var i = 0; i < param["f_rows"]; ++i) {
						var tr = doc.createElement("tr");
						tbody.appendChild(tr);
						for (var j = 0; j < param["f_cols"]; ++j) {
							var td = doc.createElement("td");
							tr.appendChild(td);
							if (HTMLArea.is_gecko) {
								// Mozilla likes to see something
								// inside the cell.
								td.appendChild(doc.createElement("br"));
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

				var f_width = parseInt(table.style.width);
				if (isNaN(f_width)) { f_width = ""; }
				var f_height = parseInt(table.style.height);
				if (isNaN(f_height)) { f_height = ""; }

				var f_widthUnit = /%/.test(table.style.width)   ? '%' : 'px';
				var f_heightUnit = /%/.test(table.style.height) ? '%' : 'px';

				var f_spacing = table.cellSpacing;
				var f_padding = table.cellPadding;
				var f_borders = table.border;

				var f_frames  = table.frame;
				var f_rules   = table.rules;

				var f_bgcolor     = table.style.backgroundColor.replace("#", "");
				var f_color       = table.style.color.replace("#", "");
				var f_borderColor = table.style.borderColor.replace("#", "");

				var f_borderWidth = parseInt(table.style.borderWidth);
				if (isNaN(f_borderWidth)) { f_borderWidth = 0; }
				var f_borderStyle = table.style.borderStyle;
				// Gecko reports "solid solid solid solid" for "border-style: solid".
				// That is, "top right bottom left" -- we only consider the first
				// value.
				(f_borderStyle.match(/([^\s]*)\s/)) && (f_borderStyle = RegExp.$1);

				editor_<?php echo $this->wysiwyg->name?>._object = this;
				strPage = "<?php echo $this->get_popup_href('edit_table.php')?>" + 
					"?f_width="       + escape(f_width)       +
					"&f_height="      + escape(f_height)      +
					"&f_widthUnit="   + escape(f_widthUnit)   +
					"&f_heightUnit="  + escape(f_heightUnit)  +
					"&f_spacing="     + escape(f_spacing)     +
					"&f_padding="     + escape(f_padding)     +
					"&f_borders="     + escape(f_borders)     +
					"&f_frames="      + escape(f_frames)      +
					"&f_rules="       + escape(f_rules)       +
					"&f_bgcolor="     + escape(f_bgcolor)     +
					"&f_color="       + escape(f_color)       +
					"&f_borderColor=" + escape(f_borderColor) +
					"&f_borderStyle=" + escape(f_borderStyle) +
					"&f_borderWidth=" + escape(f_borderWidth) +
					"&editor_name="   + escape("<?php echo $this->wysiwyg->name; ?>");

				this._popupDialog("editTableProperties", strPage, 360, 370, true, function(params) {
					if (!params) {
						// user must have pressed Cancel
						return false;
					}

					HTMLArea.processStyle(params, table);
					for (var i in params) {
						var val = params[i];
						switch (i) {
							case "f_align":
								table.align = val;
							break;

							case "f_spacing":
								table.cellSpacing = val;
							break;

							case "f_padding":
								table.cellPadding = val;
							break;

							case "f_borders":
								table.border = val;
							break;

							case "f_frames":
								table.frame = val;
							break;

							case "f_rules":
								table.rules = val;
							break;
						}
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
				}, null);
			};



			HTMLArea.prototype._editRowCellProperties = function(cell) {
				var editor = this; // for nested functions

				// retrieve existing values
				var element = this.getClosest(cell ? "td" : "tr");
				var table = this.getClosest("table");

				var f_width = parseInt(element.style.width);
				if (isNaN(f_width)) { f_width = ""; }
				var f_height = parseInt(element.style.height);
				if (isNaN(f_height)) { f_height = ""; }

				var f_widthUnit = /%/.test(element.style.width)   ? '%' : 'px';
				var f_heightUnit = /%/.test(element.style.height) ? '%' : 'px';

				var f_align  = element.align;
				var f_valign = element.vAlign;

				var f_bgcolor     = element.style.backgroundColor.replace("#", "");
				var f_color       = element.style.color.replace("#", "");
				var f_borderColor = element.style.borderColor.replace("#", "");

				var f_borderWidth = parseInt(element.style.borderWidth);
				if (isNaN(f_borderWidth)) { f_borderWidth = 0; }
				var f_borderStyle = element.style.borderStyle;
				// Gecko reports "solid solid solid solid" for "border-style: solid".
				// That is, "top right bottom left" -- we only consider the first
				// value.
				(f_borderStyle.match(/([^\s]*)\s/)) && (f_borderStyle = RegExp.$1);

				editor_<?php echo $this->wysiwyg->name?>._object = this;
				strPage = "<?php echo $this->get_popup_href(''); ?>edit_" + (cell ? "cell" : "row") + ".php" + 
					"?f_width="       + escape(f_width)       +
					"&f_height="      + escape(f_height)      +
					"&f_widthUnit="   + escape(f_widthUnit)   +
					"&f_heightUnit="  + escape(f_heightUnit)  +
					"&f_align="       + escape(f_align)       +
					"&f_valign="      + escape(f_valign)      +
					"&f_bgcolor="     + escape(f_bgcolor)     +
					"&f_color="       + escape(f_color)       +
					"&f_borderColor=" + escape(f_borderColor) +
					"&f_borderStyle=" + escape(f_borderStyle) +
					"&f_borderWidth=" + escape(f_borderWidth) +
					"&editor_name="   + escape("<?php echo $this->wysiwyg->name; ?>");

				this._popupDialog("edit" + (cell ? "Cell" : "Row") + "Properties", strPage, 380, 270, true, function(params) {
					if (!params) {
						// user must have pressed Cancel
						return false;
					}

					HTMLArea.processStyle(params, element);
					for (var i in params) {
						var val = params[i];
						switch (i) {
							case "f_align":
								element.align = val;
							break;

							case "f_valign":
								element.vAlign = val;
							break;
						}
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
				}, null);
			};


			HTMLArea.prototype._nullBorders = function(doc, status) {
				// show table borders
				if (!doc) return;
				var borderStyle = "#ababab 1px dashed";
				var edit_Tables = doc.getElementsByTagName("TABLE");

				for (i=0; i < edit_Tables.length; i++) {
					if (edit_Tables[i].border == '' || edit_Tables[i].border == '0' ) {
						edit_Rows = edit_Tables[i].rows;
						for (j=0; j < edit_Rows.length; j++) {
							edit_Cells = edit_Rows[j].cells;
							for (k=0; k < edit_Cells.length; k++) {
								if (status == 'show') {
									edit_Cells[k].style.borderTop = borderStyle;
									if (j == (edit_Rows.length -1)) {
										edit_Cells[k].style.borderBottom = borderStyle;
									}
									edit_Cells[k].style.borderLeft = borderStyle;
									if (k == (edit_Cells.length -1)) {
										edit_Cells[k].style.borderRight = borderStyle;
									}
								} else {
									if (edit_Cells[k].style.borderTop.toLowerCase() == borderStyle) {
										edit_Cells[k].style.borderTop = "";
									}
									if (edit_Cells[k].style.borderBottom.toLowerCase() == borderStyle) {
										edit_Cells[k].style.borderBottom = "";
									}
									if (edit_Cells[k].style.borderLeft.toLowerCase() == borderStyle) {
										edit_Cells[k].style.borderLeft = "";
									}
									if (edit_Cells[k].style.borderRight.toLowerCase() == borderStyle) {
										edit_Cells[k].style.borderRight = "";
									}
								}
							}
						}
					}
				}
			};
		</script>
		<?php
	}


	/**
	* Call the appropriate function when a button is clicked
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_button_click() {
		?>
		case "inserttable":
			this._insertTable();
			if (this.config._showborders){
				if (HTMLArea.is_ie) {
					var doco = this._docContent;
				} else {
					var doco = this._iframe.contentWindow;
				}
				this._nullBorders(doco, 'show');
				this.config._showborders = true;
			}
		break;

		case "tableproperties":
			this._editTableProperties();
		break;

		case "rowproperties":
			this._editRowCellProperties(false);
		break;

		case "cellproperties":
			this._editRowCellProperties(true);
		break;

		case "insertrowbefore":
		case "insertrowafter" :
			var tr = this.getClosest("tr");
			if (!tr) { break; }
			var otr = tr.cloneNode(true);
			this.clearRow(otr);
			tr.parentNode.insertBefore(otr, /after/.test(cmd.toLowerCase()) ? tr.nextSibling : tr);
			editor.forceRedraw();
			editor.focusEditor();
			if (this.config._showborders){
				if (HTMLArea.is_ie) {
					var doco = this._docContent;
				} else {
					var doco = this._iframe.contentWindow;
				}
				this._nullBorders(doco, 'show');
				this.config._showborders = true;
			}
		break;

		case "deleterow":
			var tr = this.getClosest("tr");
			if (!tr) { break; }
			var par = tr.parentNode;
			if (par.rows.length == 1) {
				alert("Cannot delete the last row in a table");
				break;
			}
			// set the caret first to a position that doesn't disappear.
			this.selectNextNode(tr);
			par.removeChild(tr);
			editor.forceRedraw();
			editor.focusEditor();
			editor.updateToolbar();
		break;

		case "splitrow":
			var td = this.getClosest("td");
			if (!td) { break; }
			this.splitRow(td);
			editor.forceRedraw();
			editor.updateToolbar();
			if (this.config._showborders){
				if (HTMLArea.is_ie) {
					var doco = this._docContent;
				} else {
					var doco = this._iframe.contentWindow;
				}
				this._nullBorders(doco, 'show');
				this.config._showborders = true;
			}
		break;

		case "insertcolumnbefore":
		case "insertcolumnafter" :
			var mozbr = HTMLArea.is_gecko ? "<br />" : "";
			var td = this.getClosest("td");
			if (!td) { break; }
			var rows = td.parentNode.parentNode.rows;
			var index = td.cellIndex;
			for (var i = rows.length; --i >= 0;) {
				var tr = rows[i];
				var ref = tr.cells[index + (/after/.test(cmd.toLowerCase()) ? 1 : 0)];
				var otd = editor._doc.createElement("td");
				otd.innerHTML = mozbr;
				tr.insertBefore(otd, ref);
			}
			if (this.config._showborders){
				if (HTMLArea.is_ie) {
					var doco = this._docContent;
				} else {
					var doco = this._iframe.contentWindow;
				}
				this._nullBorders(doco, 'show');
				this.config._showborders = true;
			}
		break;

		case "splitcolumn":
			var td = this.getClosest("td");
			if (!td) { break; }
			this.splitCol(td);
			editor.forceRedraw();
			editor.updateToolbar();
			if (this.config._showborders){
				if (HTMLArea.is_ie) {
					var doco = this._docContent;
				} else {
					var doco = this._iframe.contentWindow;
				}
				this._nullBorders(doco, 'show');
				this.config._showborders = true;
			}
		break;

		case "deletecolumn":
			var td = this.getClosest("td");
			if (!td) { break; }
			var index = td.cellIndex;
			if (td.parentNode.cells.length == 1) {
				alert("Cannot delete the last column in a row");
				break;
			}
			// set the caret first to a position that doesn't disappear
			this.selectNextNode(td);
			var rows = td.parentNode.parentNode.rows;
			for (var i = rows.length; --i >= 0;) {
				var tr = rows[i];
				tr.removeChild(tr.cells[index]);
			}
			editor.forceRedraw();
			editor.focusEditor();
			editor.updateToolbar();
		break;

		case "insertcellbefore":
		case "insertcellafter" :
			var mozbr = HTMLArea.is_gecko ? "<br />" : "";
			var td = this.getClosest("td");
			if (!td) { break; }
			var tr = td.parentNode;
			var otd = editor._doc.createElement("td");
			otd.innerHTML = mozbr;
			tr.insertBefore(otd, /after/.test(cmd.toLowerCase()) ? td.nextSibling : td);
			editor.forceRedraw();
			editor.focusEditor();
			if (this.config._showborders){
				if (HTMLArea.is_ie) {
					var doco = this._docContent;
				} else {
					var doco = this._iframe.contentWindow;
				}
				this._nullBorders(doco, 'show');
				this.config._showborders = true;
			}
		break;

		case "splitcell":
			var td = this.getClosest("td");
			if (!td) { break; }
			this.splitCell(td);
			editor.forceRedraw();
			editor.updateToolbar();
			if (this.config._showborders){
				if (HTMLArea.is_ie) {
					var doco = this._docContent;
				} else {
					var doco = this._iframe.contentWindow;
				}
				this._nullBorders(doco, 'show');
				this.config._showborders = true;
			}
		break;

		case "deletecell":
			var td = this.getClosest("td");
			if (!td) { break; }
			if (td.parentNode.cells.length == 1) {
				alert("Cannot delete last cell in a row");
				break;
			}
			// set the caret first to a position that doesn't disappear
			this.selectNextNode(td);
			td.parentNode.removeChild(td);
			editor.forceRedraw();
			editor.updateToolbar();
		break;

		case "mergecells":
			// !! FIXME: Mozilla specific !!
			var sel = editor._getSelection();
			var range, i = 0;
			var rows = [];
			var row = null;
			var cells = null;
			if (!HTMLArea.is_ie) {
				try {
					while (range = sel.getRangeAt(i++)) {
						var td = range.startContainer.childNodes[range.startOffset];
						if (td.parentNode != row) {
							row = td.parentNode;
							(cells) && rows.push(cells);
							cells = [];
						}
						cells.push(td);
					}
				} catch(e) {/* finished walking through selection */}
				rows.push(cells);
			} else {
				// Internet Explorer "browser"
				var td = this.getClosest("td");
				if (!td) {
					alert("Please click inside a cell");
					break;
				}
				var tr = td.parentElement;
				var no_cols = prompt("How many columns would you like to merge?", 1);
				if (!no_cols) {
					// cancelled
					break;
				}
				var no_rows = prompt("How many rows would you like to merge?", 1);
				if (!no_rows) {
					// cancelled
					break;
				}
				var cell_index = td.cellIndex;
				while (no_rows-- > 0) {
					td = tr.cells[cell_index];
					cells = [td];
					for (var i = 1; i < no_cols; ++i) {
						td = td.nextSibling;
						if (!td) {
							break;
						}
						cells.push(td);
					}
					rows.push(cells);
					tr = tr.nextSibling;
					if (!tr) {
						break;
					}
				}
			}
			var HTML = "";
			for (i = 0; i < rows.length; ++i) {
				// i && (HTML += "<br />");
				var cells = rows[i];
				for (var j = 0; j < cells.length; ++j) {
					// j && (HTML += "&nbsp;");
					var cell = cells[j];
					HTML += cell.innerHTML;
					(i || j) && (cell.parentNode.removeChild(cell));
				}
			}
			var td = rows[0][0];
			td.innerHTML = HTML;
			td.rowSpan = rows.length;
			td.colSpan = rows[0].length;
			editor.selectNodeContents(td);
			editor.forceRedraw();
			editor.focusEditor();
		break;

		case "showborders":
			if (HTMLArea.is_ie) {
				var doco = this._docContent;
			} else {
				var doco = this._iframe.contentWindow;
			}
			if (this.config._showborders){
				this._nullBorders(doco, 'hide');
				this.config._showborders = false;
			} else {
				this._nullBorders(doco, 'show');
				this.config._showborders = true;
			}
		break;
		<?php
	}


	/**
	* When the toolbar is updated, show or hide table borders
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_update_toolbar() {
		?>
		case "showborders":
			if (this.config._showborders){
				btn.state("active", true);
			} else {
				btn.state("active", false);
			}
		break;
		<?php
	}
}

?>