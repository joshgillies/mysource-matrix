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
* @package Fudge::wysiwyg
*/
class table_editing extends wysiwyg_plugin {

	/**
	* Constructor
	* Adds buttons needed for table editing
	*
	* @returns object Table_Editing
	* @access  public
	*/
	function table_editing() {
		$this->_add_button('inserttable','InsertTable','Insert Table','false','35');
		//$this->_add_button('tableproperties','TableProperties','Table Properties','false','36');
		//$this->_add_button('showborder','ShowBorder','Show 0 borders','false','37');
		
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
	}


	/**
	* Prints vars needed by the table editing functions
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_vars() {
		?>
		this.config._showborders = false;
		<?php
	}


	/**
	* Prints generic functions to insert an manipulate tables
	*
	* @returns void
	* @access  public
	*/
	function paint_generic() {
		?>
		<script type="text/javascript" language="Javascript">
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
				this._popupDialog("<?php echo $this->get_popup_href('insert_table.html')?>", function(param) {
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
							table.style.width = value + param["f_unit"];
							break;
							case "f_align":
							table.align = value;
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

			HTMLArea.prototype._nullBorders = function(doc,status) {
				// show table borders
				var edit_Tables = doc.document.getElementsByTagName("TABLE");

				for (i=0; i < edit_Tables.length; i++) {
					if (edit_Tables[i].border == '' || edit_Tables[i].border == '0' ) {
						if (status == 'show' ) {
							edit_Tables[i].style.border = "1px dotted #C0C0C0";
						} else {
							edit_Tables[i].removeAttribute("style");
						}
					}
					edit_Rows = edit_Tables[i].rows;
					for (j=0; j < edit_Rows.length; j++) {
						edit_Cells = edit_Rows[j].cells;
						for (k=0; k < edit_Cells.length; k++) {
							if (edit_Tables[i].border == '' || edit_Tables[i].border == '0' ) {
								if (!edit_Cells[k].border || edit_Cells[k].border == '' || edit_Cells[k].border == '0' ) {
									if (status == 'show' ) {
										edit_Cells[k].style.border = "1px dotted #C0C0C0";
									} else {
										edit_Cells[k].removeAttribute("style");
									}
								}
							} else {
								if ( edit_Cells[k].border == '0' ) {
									if (status == 'show' ) {
										edit_Cells[k].style.border = "1px dotted #C0C0C0";
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
			this.config._showborders = !this.config._showborders;
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
		case "showborder":
			if (HTMLArea.is_ie) {
				var doco = this._doc;
			} else {
				var doco = this._iframe.contentWindow;
			}
			if (this.config._showborders){
				this._nullBorders(doco,'hide');
				btn.state("active", false);
				this.config._showborders = false;
			} else {
				this._nullBorders(doco,'show');
				btn.state("active", !text);
				this.config._showborders = true;
			}
		break;
		<?php
	}
}

?>