<?php

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
		$this->_add_button('inserttable','InsertTable','Insert Table','false','35');
		$this->_add_button('tableproperties','TableProperties','Table Properties','false','36', 'table');
		//$this->_add_button('showborders','ShowBorders','Show 0 borders','false','37');
		
		//$this->_add_button('rowproperties','RowProperties','Row Properties','false','38');
		//$this->_add_button('insertrowbefore','InsertRowBefore','Insert Row Before','false','39');
		//$this->_add_button('insertrowafter','InsertRowAfter','Insert Row After','false','40');
		//$this->_add_button('deleterow','DeleteRow','Delete Row','false','41');
		//$this->_add_button('splitrow','SplitRow','Split Row','false','42');
		//$this->_add_button('mergerows','MergeRows','Merge Rows','false','43');
		
		//$this->_add_button('insertcolumnbefore','InsertColumnBefore','Insert Column Before','false','44');
		//$this->_add_button('insertcolumnafter','InsertColumnAfter','Insert Column Afer','false','45');
		//$this->_add_button('deletecolumn','DeleteColumn','Delete Column','false','46');
		
		//$this->_add_button('cellproperties','CellProperties','Cell Properties','false','47');
		//$this->_add_button('insertcellbefore','InsertCellBefore','Insert Cell Before','false','48');
		//$this->_add_button('insertcellafter','InsertCellAfter','Insert Cell After','false','49');
		//$this->_add_button('deletecell','DeleteCell','Delete Cell','false','50');
		//$this->_add_button('splitcell','SplitCell','Split Cell','false','51');
		//$this->_add_button('mergecells','MergeCells','Merge Cells','false','52');

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
			// retrieves the closest element having the specified tagName in the list of
			// ancestors of the current selection/caret.
			HTMLArea.prototype.getClosest = function(tagName) {
				var ancestors = this.getAllAncestors();
				var ret = null;
				tagName = ("" + tagName).toLowerCase();
				for (var i in ancestors) {
					var el = ancestors[i];
					if (el.tagName.toLowerCase() == tagName) {
						ret = el;
						break;
					}
				}
				return ret;
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
				isNaN(f_width) && (f_width = "");
				var f_height = parseInt(table.style.height);
				isNaN(f_height) && (f_height = "");

				var f_widthUnit = /%/.test(table.style.width)   ? '%' : 'px';
				var f_heightUnit = /%/.test(table.style.height) ? '%' : 'px';

				var f_align   = table.align;
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
					"&f_align="       + escape(f_align)       +
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



			HTMLArea.prototype._nullBorders = function(doc, status) {
				// show table borders
				if (!doc) return;
				var edit_Tables = doc.getElementsByTagName("TABLE");

				for (i=0; i < edit_Tables.length; i++) {
					if (edit_Tables[i].border == '' || edit_Tables[i].border == '0' ) {
						if (status == 'show' ) {
							edit_Tables[i].style.border = "1px dotted #000000";
						} else {
							edit_Tables[i].style.border = "0px";
						}
					}
					edit_Rows = edit_Tables[i].rows;
					for (j=0; j < edit_Rows.length; j++) {
						edit_Cells = edit_Rows[j].cells;
						for (k=0; k < edit_Cells.length; k++) {
							if (edit_Tables[i].border == '' || edit_Tables[i].border == '0' ) {
								if (!edit_Cells[k].border || edit_Cells[k].border == '' || edit_Cells[k].border == '0' ) {
									if (status == 'show' ) {
										edit_Cells[k].style.border = "1px dotted #000000";
									} else {
										edit_Cells[k].removeAttribute("style");
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
		break;
		case "tableproperties":
			this._editTableProperties();
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