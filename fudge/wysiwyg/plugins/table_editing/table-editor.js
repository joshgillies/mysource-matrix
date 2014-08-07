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
* $Id: table-editor.js,v 1.35 2012/12/04 00:29:58 cupreti Exp $
*
*/

var colorsStr = "aqua|black|blue|fuchsia|gray|green|lime|maroon|navy|olive|purple|red|silver|teal|white|yellow";

THeader = function(r, c)
{
	this.r = r;
	this.c = c;
}

TCell = function(parent)
{
	this.className	= null;
	this.colspan 	= 1;
	this.rowspan 	= 1;
	this.visible 	= true;
	this.align		= "left";
	this.valign		= "middle";
	this.bg			= null;
	this.abbr		= null;
	this.axis		= null;
	this.th			= false;
	this.scope		= null;
	this.headers	= Array();
	this.borderColor= null;
	this.borderStyle= null;
	this.borderWidth= null;
	this.cellWidth	= null;
	this.cellHeight	= null;
	this.content	= "&nbsp;";
	this.parent		= parent;
	this.selected	= false;
	this.style		= null;
	this.extra		= "";


	this.toString = function(c)
	{
		if (!this.visible) return "";
		var out = '';
		if (this.th) out = '<th';
		else out = '<td';
		var r = this.parent.row;
		var name = this.parent.parent.varname;
		out += ' id="td' + r + '_' + c + '"';
		if (this.className != null) out += ' class="' + this.className + '"';
		out += ' onclick="' + name + '.select(this)"';
		if (this.colspan > 1) out += ' colspan="' + this.colspan + '"';
		if (this.rowspan > 1) out += ' rowspan="' + this.rowspan + '"';
		if (this.abbr != null) out += ' abbr="' + this.abbr + '"';
		if (this.scope != null) out += ' scope="' + this.scope + '"';
		if (this.axis != null) out += ' axis="' + this.axis + '"';
		if ((this.align != "left" && !this.th) || (this.align != "center" && this.th)) out += ' align="' + this.align + '"';
		if (this.valign != "middle") out += ' valign="' + this.valign + '"';
		if (this.headers.length > 0) {
			out +=' headers="';
			for (i = 0;i<this.headers.length;i++)
				out += name + '_td' + this.headers[i].r + '_' + this.headers[i].c + ' ';
			out +='"';
		}
		var style = ' style="';
		//below else statements are trying to add visible borders for those with none
		if (this.borderWidth != null) {
			style += 'border-width:' + this.borderWidth + ';';
			if (this.borderColor != null) {
				style += 'border-color:' + this.borderColor + ';';

			} else {
				style += 'border-color:' + 'black' + ';';
			}
			if (this.borderStyle != null) {
				style += 'border-style:' + this.borderStyle + ';';
			} else {
				style += 'border-style:' + 'solid' + ';';
			}
			
		} else {
			if (parent.parent.htmlborder == null) {
				style += 'border-width:' + '1px' + ';';
				style += 'border-color:' + '#99CCFF' + ';';
				style += 'border-style:' + 'dashed' + ';';
			}
		}

		this.style = style;
		this.style = this.style.replace(new RegExp('border\-width:.*?;', 'gi'), '');
		this.style = this.style.replace(new RegExp('background\-color:.*?;', 'gi'), '');
		this.style = this.style.replace(new RegExp('border\-style:.*?;', 'gi'), '');
		this.style = this.style.replace(new RegExp('border\-color:.*?;', 'gi'), '');
		
		if (this.borderColor != null) {
			style += 'border-color:' + this.borderColor + ';';
			this.style = this.style.replace(new RegExp('border\-color:.*?;', 'gi'), '');
		}
		if (this.borderStyle != null) {
			style += 'border-style:' + this.borderStyle + ';';
			this.style = this.style.replace(new RegExp('border\-style:.*?;', 'gi'), '');
		}
		if (this.borderWidth != null) {
			style += 'border-width:' + this.borderWidth + ';';
			this.style = this.style.replace(new RegExp('border\-width:.*?;', 'gi'), '');
		}	
		if (this.bg != null) {
			style += 'background-color:' + this.bg + ';';
			this.style = this.style.replace(new RegExp('background\-color:.*?;', 'gi'), '');
		}
		
		if (this.cellWidth != null) {			
			style += 'width:' + this.cellWidth + ';'; 
			this.style = this.style.replace(new RegExp('width:.*?;', 'gi'), '');
		}

		if (this.cellHeight != null) {
			style += 'height:' + this.cellHeight + ';';
			this.style = this.style.replace(new RegExp('height:.*?;', 'gi'), '');
		}

		if (this.selected) style += 'background-image:url(' + this.parent.parent.semigray + ');';

		if (style != ' style="') out += style + '"';
		out += '>' + this.content;
		if (this.content == "") {
			out += "&nbsp;";
		}
		if (this.th) out += '</th>';
		else out += '</td>';

		return out;

	}


	this.Export = function(c)
	{
	   
		if (!this.visible) return "";
		var out = '';
		if (this.th) out = '<th';
		else out = '<td';
		var r = this.parent.row;
		var name = this.parent.parent.id;
		if (name != null && name != "") out += ' id="' + name + '_td' + r + '_' + c + '"';
		if (this.className != null && this.className != "") out += ' class="' + this.className + '"';
		if (this.colspan > 1) out += ' colspan="' + this.colspan + '"';
		if (this.rowspan > 1) out += ' rowspan="' + this.rowspan + '"';
		if (this.abbr != null && this.abbr != "") out += ' abbr="' + this.abbr + '"';
		if (this.scope != null && this.scope != "") out += ' scope="' + this.scope + '"';
		if ((this.align != "left" && !this.th) || (this.align != "center" && this.th)) out += ' align="' + this.align + '"';
		if (this.valign != "middle") out += ' valign="' + this.valign + '"';
		if (this.headers.length > 0) {
			out +=' headers="';
			for (i = 0;i<this.headers.length;i++)
				out += name + '_td' + this.headers[i].r + '_' + this.headers[i].c + ' ';
			out +='"';
		}

		var style = "";
		if (this.borderColor != null) {
			style += 'border-color:' + this.borderColor + ';';
			this.style = this.style.replace(new RegExp('border\-color:.*?;', 'gi'), '');
		}
		if (this.borderStyle != null) {
			style += 'border-style:' + this.borderStyle + ';';
			this.style = this.style.replace(new RegExp('border\-style:.*?;', 'gi'), '');
		}
		if (this.borderWidth != null) {
			style += 'border-width:' + this.borderWidth + ';';
			this.style = this.style.replace(new RegExp('border\-width:.*?;', 'gi'), '');
		}	
		if (this.bg != null) {
			style += 'background-color:' + this.bg + ';';
			this.style = this.style.replace(new RegExp('background\-color:.*?;', 'gi'), '');
		}
		
		if (this.cellWidth != null) {			
			style += 'width:' + this.cellWidth + ';'; 
			this.style = this.style.replace(new RegExp('width:.*?;', 'gi'), '');
		}

		if (this.cellHeight != null) {
			style += 'height:' + this.cellHeight + ';';
			this.style = this.style.replace(new RegExp('height:.*?;', 'gi'), '');
		}

		if (this.style != null && this.style.search('style=') != -1) {
			style = this.style + style;
		} else {
			style += ' style="' + this.style;
		}

		if (style != '' && style != ' style="' && style.search('style=') != -1) {
			out += style + '"';
		} 
		
		out += this.extra + '>' + this.content;
		if (this.th) out += '</th>';
		else out += '</td>';

		return out;

	}
	
	this.setPanels = function()
	{
		document.getElementById("cell_class").value = (this.className == null)?"":this.className;
		document.getElementById("cell_aleft").style.background = "";
		document.getElementById("cell_acenter").style.background = "";
		document.getElementById("cell_aright").style.background = "";

		document.getElementById("cell_atop").style.background = "";
		document.getElementById("cell_amiddle").style.background = "";
		document.getElementById("cell_abottom").style.background = "";

		document.getElementById("cell_a" + this.align).style.background = "#F00";
		document.getElementById("cell_a" + this.valign).style.background = "#F00";
		if (this.th) document.getElementById("THead").style.background = "#F00";
		else document.getElementById("THead").style.background = "#FFF";

		document.getElementById("abbr").value = (this.abbr == null)?"":this.abbr;
		document.getElementById("axis").value = (this.axis == null)?"":this.axis;
		document.getElementById("scope").value = (this.scope == null)?"":this.scope;

		//document.getElementById("bg").style.background = (this.bg == null)?"url(" + this.parent.parent.empty + ")":this.bg;
		if (this.bg == null) {
			if (this.parent.bg == null) {
				document.getElementById("bg").style.background = "url(" + this.parent.parent.empty + ")";
			} else {
				document.getElementById("bg").style.background = this.parent.bg;
			}
		} else {
			document.getElementById("bg").style.background = this.bg;
		}
		document.getElementById("border").style.background = (this.borderColor == null)?"url(" + this.parent.parent.empty + ")":this.borderColor.split(' ')[0];
		if (this.borderWidth != null) {
			document.getElementById("cell_border").value = this.borderWidth.split('px')[0];
		} else {
			document.getElementById("cell_border").value = "";
		}
		if (this.borderStyle != null) {
			document.getElementById("cell_bordertype").value = this.borderStyle.split(' ')[0];
		} else {
			document.getElementById("cell_bordertype").value = 'solid';
		}
		if (this.cellWidth != null) {
			if (this.cellWidth.indexOf('%') == -1) {
				document.getElementById("cell_width").value = this.cellWidth.substring(0, this.cellWidth.length - 2);
				document.getElementById("cell_widthtype").value = this.cellWidth.substring(this.cellWidth.length - 2);
			} else {
				document.getElementById("cell_width").value = this.cellWidth.substring(0, this.cellWidth.length - 1);
				document.getElementById("cell_widthtype").value = this.cellWidth.substring(this.cellWidth.length - 1);
			}
		} else {
			document.getElementById("cell_width").value = "";
			document.getElementById("cell_widthtype").value = "%";
		}
		if (this.cellHeight != null) {
			if (this.cellHeight.indexOf('%') == -1) {
				document.getElementById("cell_height").value = this.cellHeight.substring(0, this.cellHeight.length - 2);
				document.getElementById("cell_heighttype").value = this.cellHeight.substring(this.cellHeight.length - 2);
			} else {
				document.getElementById("cell_height").value = this.cellHeight.substring(0, this.cellHeight.length - 1);
				document.getElementById("cell_heighttype").value = this.cellHeight.substring(this.cellHeight.length - 1);
			}
		} else {
			document.getElementById("cell_height").value = "";
			document.getElementById("cell_heighttype").value = "%";
		}
	}
}


TRow = function(parent, row)
{
	this.className	= null;
	this.align		= "left";
	this.valign		= "middle";
	this.style		= null;
	this.cells		= Array();
	this.parent		= parent;
	this.row		= row;
	this.borderColor= null;
	this.borderStyle= null;
	this.borderWidth= null;
	this.bg			= null;
	this.height		= null;

	this.selected	= false;

	this.toString = function()
	{
		var out = '';
		out += '<tr';
		if (this.className != null) out += ' class="' + this.className + '"';
		if (this.align != "left") out += ' align="' + this.align + '"';
		if (this.valign != "middle") out += ' valign="' + this.valign + '"';
		var style = 'style="';
		
		if (this.borderColor != null) {
			style += 'border-color:' + this.borderColor + ';';
			this.style = this.style.replace(new RegExp('border\-color:.*?;', 'gi'), '');
		}		
		if (this.borderStyle != null) {
			style += 'border-style:' + this.borderStyle + ';';
			this.style = this.style.replace(new RegExp('border\-style:.*?;', 'gi'), '');
		}
		if (this.borderWidth != null) {
			style += 'border-width:' + this.borderWidth + ';';
			this.style = this.style.replace(new RegExp('border\-width:.*?;', 'gi'), '');
		}
		if (this.height != null) {
			style += 'height:' + this.height + ';';
			this.style = this.style.replace(new RegExp('height:.*?;', 'gi'), '');
		}
		if (this.bg != null) {
			style += 'background-color:' + this.bg + ';';
			this.style = this.style.replace(new RegExp('background\-color:.*?;', 'gi'), '');
		}
		
		if (this.style != null) style += this.style;
		
		if (this.selected) style += 'background-image:url(' + this.parent.semigray + ');';

		if (style != 'style="') out += " " + style + '"';

		out += '>';
		for (c = 0;c<this.cells.length;c++) {
			out += this.cells[c].toString(c);
		}
		out += '</tr>';
		return out;
	}


	this.Export = function()
	{
		var out = '';
		out += '<tr';
		if (this.className != null && this.className != "") out += ' class="' + this.className + '"';
		if (this.align != "left") out += ' align="' + this.align + '"';
		if (this.valign != "middle") out += ' valign="' + this.valign + '"';
		var style = 'style="';
		
		if (this.borderColor != null) {
			style += 'border-color:' + this.borderColor + ';';
			this.style = this.style.replace(new RegExp('border\-color:.*?;', 'gi'), '');
		}		
		if (this.borderStyle != null) {
			style += 'border-style:' + this.borderStyle + ';';
			this.style = this.style.replace(new RegExp('border\-style:.*?;', 'gi'), '');
		}
		if (this.borderWidth != null) {
			style += 'border-width:' + this.borderWidth + ';';
			this.style = this.style.replace(new RegExp('border\-width:.*?;', 'gi'), '');
		}
		if (this.height != null) {
			style += 'height:' + this.height + ';';
			this.style = this.style.replace(new RegExp('height:.*?;', 'gi'), '');
		}
		if (this.bg != null) {
			style += 'background-color:' + this.bg + ';';
			this.style = this.style.replace(new RegExp('background\-color:.*?;', 'gi'), '');
		}
		
		if (this.style != null) style += this.style;
		if (style != 'style="') out += " " + style + '"';

		out += '>';
		//Test if A THEAD tag should be output
		var thead = true;
		for (c = 0;c<this.cells.length;c++) {
			out += this.cells[c].Export(c);
			if (!this.cells[c].th) thead = false;
		}
		out += '</tr>';
		//All the cells in the row are TH's, so we can wrap this in a THEAD tag
		if (thead) {
			out = '<THEAD>' + out;
			out += '</THEAD>';
		}
	
		return out;
	}


	this.setPanels = function()
	{
		document.getElementById("row_class").value = (this.className == null)?"":this.className;
		document.getElementById("row_aleft").style.background = "";
		document.getElementById("row_acenter").style.background = "";
		document.getElementById("row_aright").style.background = "";

		document.getElementById("row_atop").style.background = "";
		document.getElementById("row_amiddle").style.background = "";
		document.getElementById("row_abottom").style.background = "";

		document.getElementById("row_a" + this.align).style.background = "#F00";
		document.getElementById("row_a" + this.valign).style.background = "#F00";

		document.getElementById("bg").style.background = (this.bg == null)?"url(" + this.parent.empty + ")":this.bg;
		document.getElementById("border").style.background = (this.borderColor == null)?"url(" + this.parent.empty + ")":this.borderColor;
		document.getElementById("row_border").value = (this.borderWidth == null)? "" : this.borderWidth;
		document.getElementById("row_bordertype").value = (this.borderStyle == null)? "" : this.borderStyle;

		if (this.height != null) {
			if (this.height.indexOf('%') == -1) {
				document.getElementById("row_width").value = this.height.substring(0, this.height.length - 2);
				document.getElementById("row_widthtype").value = this.height.substring(this.height.length - 2);
			} else {
				document.getElementById("row_width").value = this.height.substring(0, this.height.length - 1);
				document.getElementById("row_widthtype").value = this.height.substring(this.height.length - 1);
			}
		} else {
			document.getElementById("row_width").value = "";
			document.getElementById("row_widthtype").value = "%";
		}
		var b_width_changed = false;
		var b_style_changed = false;
		if(this.cols == 0) return false;
		var orig_b_width = null;
		var orig_b_style = null;

		for (c = 0;c<this.cols;c++) {

			if (orig_b_width != null && this.cells[c].borderWidth != orig_b_width) {
				b_width_changed = true;
			} else {
				if (orig_b_width == null && this.cells[c].borderWidth != null) {
					orig_b_width = this.cells[c].borderWidth.split(' ')[0];
				}
			}

			if (orig_b_style != null && this.cells[c].borderStyle != orig_b_style) {
				b_style_changed = true;
			} else {
				if (orig_b_style == null) {
					orig_b_style = this.cells[c].borderStyle;
				}
			}

		}

		if (!b_width_changed && orig_b_width != null) {
			document.getElementById('row_border').value = orig_b_width.substring(0, orig_b_width.length - 2);
		} else {
			document.getElementById('row_border').value = "";
		}
		if (!b_style_changed && orig_b_style != null) {
			document.getElementById('row_bordertype').value = orig_b_style;
		} else {
			document.getElementById('row_bordertype').value = "";
		}
	}

	this.getCellCount = function()
	{
		var cellLength = this.cells.length;
		var i = 0;
		while (i >= 0) {
			if(this.cells[i] == "") {
				cellLength--;
			}
			i--;
		}
		return cellLength;
	}

}

TTable = function(name, rows, cols)
{
	this.className	= null;
	this.matrix		= Array();
	this.summary	= null;
	this.caption	= null;
	this.cellPadding= 0;
	this.cellSpacing= 0;
	this.htmlborder	= null;
	this.border		= null;
	this.borderStyle= null;
	this.borderColor= null;
	this.rows		= rows;
	this.cols		= cols;
	this.varname	= name;
	this.id			= "";
	this.width		= "";
	this.htmlwidth	= "";
	this.frame		= "";
	this.rules		= "";
	this.style		= "";
	this.extra		= "";
	this.bg 		= "";

	this.lastSelect = 'table';
	this.selector	= "table";
	this.color		= "bg";

	this.r			= null;
	this.c			= null;

	this.mouse		= new TMouse(this);

	this.semigray	= document.getElementById("semigray").src;
	this.semired	= document.getElementById("semired").src;
	this.empty		= document.getElementById("empty").src;
	this.selected	= false;

	if (typeof rows != "undefined") {
		for (r = 0;r<rows;r++) {
			var temp = new TRow(this, r);
			for (c = 0;c<cols;c++) {
				var Cell = new TCell(temp);
				temp.cells.push(Cell);
			}
			this.matrix.push(temp);
		}
	}

	this.toString = function()
	{
		var out = '<table id="js_' + this.id + '"';
		if (!isNaN(this.cellSpacing)) out += '" cellspacing="' + this.cellSpacing + '"';
		if (!isNaN(this.cellPadding)) out += '" cellpadding="' + this.cellPadding + '"';
		if (this.className != null) out += ' class="' + this.className + '"';
		if (this.htmlwidth != "") out += ' width=' + this.htmlwidth;
		if (this.htmlborder != null) out += ' border=' + this.htmlborder;

		var style = '';
		if (this.width != "" && this.width != null) style += 'width:' + this.width + ';';
		if (this.borderColor != "" && this.borderColor != null) out += 'border-color:' + this.borderColor + ';';
		if (this.borderStyle != "" && this.borderStyle != null) out += 'border-style:' + this.borderStyle + ';';
		if (this.border != "" && this.border != null) out += 'border-width:' + this.border + ';';
		if (this.bg != "" && this.bg != null) style += 'background=color: ' + this.bg + ';';
		if (style != "") out += ' style="' + style + '"';

		if (this.summary != null) out += ' summary="' + this.summary + '"';
		if (this.frame != "") out += ' frame="' + this.frame + '"';
		if (this.rules != "") out += ' rules="' + this.rules + '"';
		out += '>';
		if (this.caption != null) out += '<caption>' + this.caption + '</caption>';
		for (r = 0;r<this.rows;r++) {
			out += this.matrix[r];
		}
		out += '</table>';

		return out;

	}

	this.Export = function()
	{
		var out = '<table ';
		if (this.id != null && this.id != "") out += ' id="' + this.id + '"';
		if (!isNaN(this.cellSpacing) && this.cellSpacing != "") out += ' cellspacing="' + this.cellSpacing + '"';
		if (!isNaN(this.cellPadding) && this.cellPadding != "") out += ' cellpadding="' + this.cellPadding + '"';
		if (this.className != null) out += ' class="' + this.className + '"';
		if (this.htmlwidth != "") out += ' width=' + this.htmlwidth;
		if (this.htmlborder != null) out += ' border=' + this.htmlborder;

		var style = '';
		if (this.width != "" && this.width != null) style += 'width:' + this.width + ';';
		if (this.style != "" && this.style != null) style += this.style + ';';
		if (this.borderColor != "" && this.borderColor != null) style += 'border-color:' + this.borderColor + ';';
		if (this.borderStyle != "" && this.borderStyle != null) style += 'border-style:' + this.borderStyle + ';';
		if (this.border != "" && this.border != null) style += 'border-width:' + this.border + ';';
		if (this.bg != "" && this.bg != null) style += 'background: ' + this.bg + ';';
		if (style != "") out += ' style="' + style + '"';

		if (this.summary != null && this.summary != "") out += ' summary="' + this.summary + '"';
		if (this.frame != "") out += ' frame="' + this.frame + '"';
		if (this.rules != "") out += ' rules="' + this.rules + '"';
		out += this.extra + '>';
		if (this.caption != null) out += '<caption>' + this.caption + '</caption>';
		for (r = 0;r<this.rows;r++) {
			var row_code = this.matrix[r].Export();
			//Below condition determines if we have two theads in a row, meaning they should be grouped together
			if ((row_code.substring(0, 7) == '<THEAD>') && (out.substring(out.length - 8) == '</THEAD>')) {
				out = out.substring(0, out.length - 8); //strip out end of last thead
				row_code = row_code.substring(7);		//strip out start of new one.
			}
			out += row_code;
		}
		out += '</table>';

		return out;

	}


	this.refresh = function()
	{
		document.getElementById("table_container").innerHTML = this;
		if (this.selector == "head") this.showHeaders();
	}


	this.refreshCell = function(r, c)
	{
		var rowspan = this.matrix[r].cells[c].rowspan;
		var colspan = this.matrix[r].cells[c].colspan;
		for (i=r;i<this.rows && i<r + rowspan;i++)
			for (j=c;j<this.cols && j<c + colspan;j++)
				if ((i != r || j != c) && (this.matrix[i].cells[j].colspan > 1 || this.matrix[i].cells[j].rowspan > 1)) {
					return false;
				}

		for (i=r;i<this.rows && i<r + rowspan;i++)
			for (j=c;j<this.cols && j<c + colspan;j++) {
				if (i != r || j != c) this.matrix[i].cells[j].visible = false;
			}
		return true;

	}


	this.refreshVisibility = function()
	{
		for (r = 0;r<this.rows;r++) {
			for (c = 0;c<this.cols;c++) {
				this.matrix[r].cells[c].visible = true;
			}
		}
		
		for (r = 0;r<this.rows;r++) {
			for (c = 0;c<this.cols;c++) {
				var rowspan = this.matrix[r].cells[c].rowspan;
				var colspan = this.matrix[r].cells[c].colspan;
				if (rowspan > 1 || colspan > 1) {
					if (!this.refreshCell(r, c)) return false;
				}
			}
		}
		return true;

	}


	this.addColSpan = function()
	{
		if (this.c == null) return false;
		r = this.r;
		c = this.c;
		if (c >= this.cols) return false;

		this.matrix[r].cells[c].colspan++;
		if (!this.refreshVisibility()) {
			this.matrix[r].cells[c].colspan--;
			this.refreshVisibility();
			return false;
		}
		this.refresh();
		return true;

	}


	this.addRowSpan = function()
	{
		if (this.c == null) return false;
		r = this.r;
		c = this.c;
		if (r >= this.rows) return false;

		this.matrix[r].cells[c].rowspan++;
		if (!this.refreshVisibility()) {
			this.matrix[r].cells[c].rowspan--;
			this.refreshVisibility();
			return false;
		}
		this.refresh();
		return true;

	}


	this.delColSpan = function()
	{
		if (this.c == null) return false;
		r = this.r;
		c = this.c;
		if (this.matrix[r].cells[c].colspan == 1) return false;

		this.matrix[r].cells[c].colspan--;
		if (!this.refreshVisibility()) {
			this.matrix[r].cells[c].colspan++;
			this.refreshVisibility();
			return false;
		}
		this.refresh();
		return true;

	}


	this.delRowSpan = function()
	{
		if (this.c == null) return false;
		r = this.r;
		c = this.c;
		if (this.matrix[r].cells[c].rowspan == 1) return false;

		this.matrix[r].cells[c].rowspan--;
		if (!this.refreshVisibility()) {
			this.matrix[r].cells[c].rowspan--;
			this.refreshVisibility();
			return false;
		}
		this.refresh();
		return true;

	}


	this.setColumnAlign = function(align)
	{
		for (r = 0; r < this.rows; r++) {
			this.matrix[r].cells[this.c].align = align;
		}
		this.refresh();

	}


	this.setAlign = function(align)
	{
		if (this.lastSelect == 'row') {
			this.matrix[this.r].align = align;
			this.matrix[this.r].setPanels();
		} else if(this.lastSelect == 'cell') {
			this.matrix[this.r].cells[this.c].align = align;
			this.matrix[this.r].cells[this.c].setPanels();
		} else {
			this.setColumnAlign(align);
			this.setColumnPanels();
		}
		this.refresh();

	}


	this.setColumnValign = function(align)
	{
		for (r = 0; r < this.rows; r++) {
			this.matrix[r].cells[this.c].valign = align;
		}
		this.refresh();

	}


	this.setVAlign = function(valign)
	{
		if (this.lastSelect == 'row') {
			this.matrix[this.r].valign = valign;
			this.matrix[this.r].setPanels();
		} else if(this.lastSelect == 'cell') {
			this.matrix[this.r].cells[this.c].valign = valign;
			this.matrix[this.r].cells[this.c].setPanels();
		} else {
			this.setColumnValign(valign);
			this.setColumnPanels();
		}
		this.refresh();

	}


	this.setAbbr = function(abbr)
	{
		if (this.r == null || this.c == null) return false;
		this.matrix[this.r].cells[this.c].abbr = abbr;
		this.refresh();

	}


	this.setAxis = function(axis)
	{
		if (this.r == null || this.c == null) return false;
		this.matrix[this.r].cells[this.c].axis = axis;
		this.refresh();

	}


	this.setScope = function(scope)
	{
		if (this.r == null || this.c == null) return false;
		this.matrix[this.r].cells[this.c].scope = scope;
		this.refresh();

	}

	//Function to reenable disabled options
	this.enable = function(start, disable)
	{
		var status = "";
		if (disable) status = "disabled";
		if (start == 'row_' || start == 'col_' || start == 'cell_') {
			document.getElementById(start + 'class').disabled = status;
			document.getElementById(start + 'border').disabled = status;
			//document.getElementById(start + 'bordercolor').disabled = status;
			document.getElementById(start + 'bordertype').disabled = status;
			document.getElementById(start + 'width').disabled = status;
			document.getElementById(start + 'widthtype').disabled = status;
			if (start == 'cell_') {
				document.getElementById(start + 'height').disabled = status;
				document.getElementById(start + 'heighttype').disabled = status;
				document.getElementById('axis').disabled = status;
				document.getElementById('abbr').disabled = status;
				document.getElementById('scope').disabled = status;
				document.getElementById('headings').disabled = status;
			}
			if (start == 'col_' || start == 'row_') {
				document.getElementById(start + 'add').disabled = status;
				document.getElementById(start + 'delete').disabled = status;
			}
		}

	}

	//Sets the column options according to the settings of the cells contained in the row.
	//If there is a descrepancy like 2 different widths or 2 different border styles,
	//the option will be displayed as empty
	this.setColumnPanels = function()
	{
		document.getElementById("col_aleft").style.background = "";
		document.getElementById("col_acenter").style.background = "";
		document.getElementById("col_aright").style.background = "";

		document.getElementById("col_atop").style.background = "";
		document.getElementById("col_amiddle").style.background = "";
		document.getElementById("col_abottom").style.background = "";

		if (this.matrix[0].cells[this.c].className != null) {
			document.getElementById("col_class").value = this.matrix[0].cells[this.c].className;
		}
		var w_changed = false;
		var b_width_changed = false;
		var b_style_changed = false;
		var align_changed = false;
		var valign_changed = false;
		var bg_changed = false;
		if(this.rows == 0) return false;
		var orig_width = this.matrix[0].cells[this.c].cellWidth;
		var orig_b_width = this.matrix[0].cells[this.c].borderWidth;
		var orig_b_style = this.matrix[0].cells[this.c].borderStyle;
		var orig_valign = this.matrix[0].cells[this.c].valign;
		var orig_align = this.matrix[0].cells[this.c].align;
		var orig_bg = this.matrix[0].cells[this.c].bg;
		for (r = 1;r<this.rows;r++) {
			if (orig_width != null && this.matrix[r].cells[this.c].cellWidth != orig_width && this.matrix[r].cells[this.c].visible) {
				w_changed = true;
			} else {
				if (orig_width == 'null' && this.matrix[r].cells[this.c].visible) {
					orig_width = this.matrix[r].cells[this.c].cellWidth;
				}
			}

			if (b_width_changed != null && this.matrix[r].cells[this.c].borderWidth != orig_b_width && this.matrix[r].cells[this.c].visible) {
				b_width_changed = true;
			} else {
				if (orig_b_width == 'null' && this.matrix[r].cells[this.c].visible) {
					orig_b_width = this.matrix[r].cells[this.c].borderWidth;
				}
			}

			if (orig_b_style != null && this.matrix[r].cells[this.c].borderStyle != orig_b_style && this.matrix[r].cells[this.c].visible) {
				b_style_changed = true;
			} else {
				if (orig_b_style == 'null' && this.matrix[r].cells[this.c].visible) {
					orig_b_style = this.matrix[r].cells[this.c].borderStyle;
				}
			}
			if (this.matrix[r].cells[this.c].align != orig_align && this.matrix[r].cells[this.c].visible) {
				align_changed = true;
			}
			if (this.matrix[r].cells[this.c].valign != orig_valign && this.matrix[r].cells[this.c].visible) {
				valign_changed = true;
			}
			if (this.matrix[r].cells[this.c].bg != orig_bg) {
				bg_changed = true;
			}
		}
		if (!w_changed && orig_width != null) {
			if (orig_width.indexOf('%') != -1) {
				document.getElementById('col_width').value = orig_width.substring(0, orig_width.length - 1);
				document.getElementById('col_widthtype').value = orig_width.substring(orig_width.length - 1);
			} else {
				document.getElementById('col_width').value = orig_width.substring(0, orig_width.length - 2);
				document.getElementById('col_widthtype').value = orig_width.substring(orig_width.length - 2);
			}

		} else {
			document.getElementById('col_width').value = "";
		}
		if (this.borderWidth != null) {
			document.getElementById("cell_border").value = this.borderWidth.split('px')[0];
		} else {
			document.getElementById("cell_border").value = "";
		}
		if (!b_width_changed && orig_b_width != null) {
			document.getElementById('col_border').value = orig_b_width.split('px')[0];
		} else {
			document.getElementById('col_border').value = "";
		}
		if (!b_style_changed && orig_b_style) {
			document.getElementById('col_bordertype').value = orig_b_style;
		} else {
			document.getElementById('col_bordertype').value = "";
		}
		if (!align_changed) {
			document.getElementById('col_a' + orig_align).style.background = "#F00";
		}
		if (!valign_changed) {
			document.getElementById('col_a' + orig_valign).style.background = "#F00";
		}
		if (!bg_changed && orig_bg != null) {
			document.getElementById('bg').style.background = orig_bg;
		} else {
			document.getElementById("bg").style.background = "url(" + this.empty + ")";
		}

	}

	this.select = function(td)
	{
		this.selected = true;
		this.lastSelect = this.selector;
		var id = td.id;
		rr = id.substring(2, id.indexOf("_"))*1;
		cc = id.substring(id.indexOf("_") + 1)*1;
		if (this.selector != "head") {
			for (r = 0;r<this.rows;r++) {
				this.matrix[r].selected = false;
				for (c = 0;c<this.cols;c++) {
					this.matrix[r].cells[c].selected = false;
				}
			}
		}

		if (this.selector == "row") {
			this.matrix[rr].selected = true;
			this.matrix[rr].setPanels();
			this.c = null;
			this.r = rr;
			this.enable('row_', false);
		}
		if (this.selector == "cell") {
			this.matrix[rr].cells[cc].selected = true;
			this.matrix[rr].cells[cc].setPanels();
			this.c = cc;
			this.r = rr;
			this.enable('cell_', false)
		}
		if (this.selector == "col") {
			for (r = 0;r<this.rows;r++) {
					this.matrix[r].cells[cc].selected = true;
					if (this.matrix[r].cells[cc].rowspan > 1) {
						r += this.matrix[r].cells[cc].rowspan - 1;
					}
			}
			this.c = cc;
			this.r = 0;
			this.setColumnPanels();
			this.enable('col_', false);
		}
		//disable other functionality
		var all = Array("col", "row", "cell");
		var i = 0;
		while (i < all.length) {
			if (all[i] != this.selector) {
				this.enable(all[i] + '_', true);
			}
			i++;
		}
		if (this.selector == "head") {
			var header = new THeader(rr, cc);
			var isInside = false;
			for (i=0;i<this.matrix[this.r].cells[this.c].headers.length;i++) {
				if (this.matrix[this.r].cells[this.c].headers[i].r == header.r && this.matrix[this.r].cells[this.c].headers[i].c == header.c) isInside = true;
			}
			if (isInside) {
				var newArr = Array();
				for (i=0;i<this.matrix[this.r].cells[this.c].headers.length;i++) {
					if (this.matrix[this.r].cells[this.c].headers[i].r != header.r || this.matrix[this.r].cells[this.c].headers[i].c != header.c) newArr.push(this.matrix[this.r].cells[this.c].headers[i]);
				}
				this.matrix[this.r].cells[this.c].headers = newArr;
			} else {
				this.matrix[this.r].cells[this.c].headers.push(header);
			}
		}
		this.refresh();
		return true;

	}


	this.th = function()
	{
		if (this.lastSelect == 'cell') {
			this.matrix[this.r].cells[this.c].th = !this.matrix[this.r].cells[this.c].th;
			this.matrix[this.r].cells[this.c].setPanels();
		}
		if (this.lastSelect == 'col') {
			for (i = 0; i < this.rows; i++) {
				this.matrix[i].cells[this.c].th = !this.matrix[i].cells[this.c].th;
			}
		}
		if (this.lastSelect == 'row') {
			for (i = 0; i < this.matrix[this.r].cells.length; i++) {
				this.matrix[this.r].cells[i].th = !this.matrix[this.r].cells[i].th;
			}
		}
		this.refresh();
		return false;

	}


	this.showHeaders = function()
	{
		if (this.selected == false) return false;
		for (i=0;i<this.matrix[this.r].cells[this.c].headers.length;i++) {
			document.getElementById("td" + this.matrix[this.r].cells[this.c].headers[i].r + "_" + this.matrix[this.r].cells[this.c].headers[i].c).style.background = "url(" + this.semired + ")";
		}
	}


	this.toggleHeaders = function()
	{
		if (this.selector != "head") {
			this.selector = "head";
			this.showHeaders();
			document.getElementById("headings").innerHTML = "Done";
		} else {
			this.selector = "cell";
			this.refresh();
			document.getElementById("headings").innerHTML = "Click to select";
		}

	}


	this.addrow = function()
	{
		if (this.r == null) return false;
		this.rows++
		var temp = new TRow(this, this.r + 1);
		for (c = 0;c<this.cols;c++) {
			var Cell = new TCell(temp);
			temp.cells.push(Cell);
		}
		this.matrix.push(temp);
		for (var i = this.matrix.length - 2;i>this.r;i--) {
			this.matrix[i + 1] = this.matrix[i];
			this.matrix[i + 1].row = i + 1;
		}
		this.matrix[this.r + 1] = temp;
		this.refreshVisibility();
		this.refresh();

	}


	this.delrow = function()
	{
		if (this.r == null || this.rows == 1) return false;
		for (var i = this.r;i<this.rows - 1;i++) {
			this.matrix[i] = this.matrix[i + 1];
			this.matrix[i].row = i;
		}
		this.rows--;
		this.c = null;
		this.r = null;
		this.refresh();

	}


	this.addcol = function()
	{
		if (this.c == null) return false;
		this.cols++;
		for(i = 0; i<this.rows; i++) {
			var row = this.matrix[i];
			var Cell = new TCell(row);
			row.cells.push(Cell);
			for (j = row.cells.length - 2;j>this.c;j--) {
				row.cells[j + 1] = row.cells[j];
			}
			row.cells[this.c + 1] = Cell;
		}
		this.refreshVisibility();
		this.refresh();

	}


	this.delcol = function()
	{
		if (this.c == null || this.cols == 1) return false;
		for(i = 0; i<this.rows; i++) {
			var row = this.matrix[i];
			for (j = this.c;j< row.cells.length - 1;j++) {
				row.cells[j] = row.cells[j + 1];
			}
			row.cells.pop();
		}
		this.cols--;
		this.c = null;
		this.r = null;
		this.refreshVisibility();
		this.refresh();
	}


	this.toggleBgBorder = function()
	{
		var border = "url(" + this.empty + ")";
		var bg = "url(" + this.empty + ")";
		var obj = null;
		if (this.r != null) obj = (this.c == null)?this.matrix[this.r]:this.matrix[this.r].cells[this.c];
		if (obj != null) {
			border = (obj.borderColor == null)?"url(" + this.empty + ")":obj.borderColor;
			bg = (obj.bg == null)?"url(" + this.empty + ")":obj.bg;
		}
		if (this.color == "bg") {
			this.color = "border";
			document.getElementById("bgborder").innerHTML = '<div id="bg" style="width:30px;height:30px;position:absolute;left:0px;top:0px;border:inset 1px;background:' + bg + '"></div>' +
				'<div id="border" style="width:30px;height:30px;position:absolute;left:10px;top:10px;border:inset 1px;background:' + border + '">' +
				'	<div style="width:19px;height:19px;position:absolute;left:5px;top:5px;border:outset 1px;background:#FFF;font-size:5px"></div>' +
				'</div>';
		} else {
			this.color = "bg";
			document.getElementById("bgborder").innerHTML = '<div id="border" style="width:30px;height:30px;position:absolute;left:10px;top:10px;border:inset 1px;background:' + border + '">' +
				'	<div style="width:19px;height:19px;position:absolute;left:5px;top:5px;border:outset 1px;background:#FFF;font-size:5px"></div>' +
				'</div>' +
				'<div id="bg" style="width:30px;height:30px;position:absolute;left:0px;top:0px;border:inset 1px;background:' + bg + '"></div>';
		}
	}

	this.setColumnColor = function(color)
	{
		this.lastSelect = '';
		for (i = 0; i < this.rows; i++) {
			this.r = i;
			this.setColor(color);
		}
		this.lastSelect = 'col';
		this.r = 0;
	}

	this.setColor = function(color)
	{
		if (this.lastSelect == 'col' && this.selector != 'table') {
			this.setColumnColor(color);
			this.refresh();
			return false;
		}
		var obj = (this.c == null)?this.matrix[this.r]:this.matrix[this.r].cells[this.c];
		if (this.selector == 'table') {
			obj = this;
		}
		if (this.color == "bg") {
			if (color != null) {
				obj.bg = "#" + color;
				document.getElementById("bg").style.background = obj.bg;
			} else {
				obj.bg = null;
				document.getElementById("bg").style.background = "url(" + this.empty + ")";
			}
		} else {
			if (color != null) {
				obj.borderColor = "#" + color;
				document.getElementById("border").style.background = obj.borderColor;
			} else {
				obj.borderColor = null;
				document.getElementById("border").style.background = "url(" + this.empty + ")";
			}
		}
		this.refresh();
	}


	this.Import = function(table)
	{
		function processEvent(evt)
		{
			if (typeof evt == "undefined" || evt == null) return null;
			var f = evt + "";
			f = f.substring(f.indexOf("{") + 1);
			if (f.charAt(f.length - 1) == "\n") f = f.substring(0, f.length - 2);
			else f = f.substring(0, f.length - 1);
			var res = "";
			for (i=0;i<f.length;i++)
				if (f.charAt(i) == "\n") res += "";
				else if (f.charAt(i) == '"') res += "'";
					 else res += f.charAt(i);
			var startSpaces = new RegExp("^[ ]+", "gim");
			var endSpaces = new RegExp("[ ]+$", "gim");
			res = res.replace(startSpaces, "");
			res = res.replace(endSpaces, "");
			return res;

		}


		function getExtra(obj)
		{
			var res = "";
			if (obj.className != "")							res += ' class="' + obj.className + '"';
			if (obj.title != "")								res += ' title="' + obj.title + '"';
			if ((evt = processEvent(obj.onclick)) != null)		res += ' onclick="' + evt + '"';
			if ((evt = processEvent(obj.ondblclick)) != null)	res += ' ondblclick="' + evt + '"';
			if ((evt = processEvent(obj.onmousedown)) != null)	res += ' onmousedown="' + evt + '"';
			if ((evt = processEvent(obj.onmouseup)) != null)	res += ' onmouseup="' + evt + '"';
			if ((evt = processEvent(obj.onmouseover)) != null)	res += ' onmouseover="' + evt + '"';
			if ((evt = processEvent(obj.onmousemove)) != null)	res += ' onmousemove="' + evt + '"';
			if ((evt = processEvent(obj.onmouseout)) != null)	res += ' onmouseout="' + evt + '"';
			if ((evt = processEvent(obj.onkeypress)) != null)	res += ' onkeypress="' + evt + '"';
			if ((evt = processEvent(obj.onkeydown)) != null)	res += ' onkeydown="' + evt + '"';
			if ((evt = processEvent(obj.onkeyup)) != null)		res += ' onkeyup="' + evt + '"';

			return res;

		}


		function getStyle(table)
		{
			var res = "";
			if (table.style.position != "")			res += "position: " + table.style.position + ";";
			if (table.style.display != "")			res += "display: " + table.style.display + ";";
			if (table.style.left != "")				res += "left: " + table.style.left + ";";
			if (table.style.right != "")			res += "right: " + table.style.right + ";";
			if (table.style.top != "")				res += "top: " + table.style.top + ";";
			if (table.style.bottom != "")			res += "bottom: " + table.style.bottom + ";";
			if (table.style.clear != "")			res += "clear: " + table.style.clear + ";";
			if (table.style.clip != "")				res += "clip: " + table.style.clip + ";";
			if (table.style.color != "")			res += "color: " + table.style.color + ";";
			if (table.style.cursor != "")			res += "cursor: " + table.style.cursor + ";";
			if (table.style.font != "")				res += "font: " + table.style.font + ";";
			if (table.style.width != "" && table.tagName != "TABLE")
													res += "width: " + table.style.width + ";";
			if (table.style.height != "")			res += "height: " + table.style.height + ";";
			if (table.style.margin != "")			res += "margin: " + table.style.margin + ";";
			if (table.style.padding != "")			res += "padding: " + table.style.padding + ";";
			if (table.style.overflow != "")			res += "overflow: " + table.style.overflow + ";";
			if (table.style.textAlign != "")		res += "text-align: " + table.style.textAlign + ";";
			if (table.style.textDecoration != "")	res += "text-decoration: " + table.style.textDecoration + ";";
			if (table.style.textIndent != "")		res += "text-indent: " + table.style.textIndent + ";";
			if (table.style.verticalAlign != "")	res += "vertical-align: " + table.style.verticalAlign + ";";
			if (table.style.visibility != "")		res += "visibility: " + table.style.visibility + ";";
			if (table.style.MozOpacity != "" && table.style.MozOpacity + "" != "undefined")
													res += "-moz-opacity: " + table.style.MozOpacity + ";";
			if (table.style.opacity != "" && table.style.opacity + "" != "undefined")
													res += "opacity: " + table.style.opacity + ";";

			return res;

		}

		var div = document.createElement("DIV");
		div.innerHTML = table;
		table = div.getElementsByTagName("TABLE")[0];

		this.id			= table.id;
		this.summary	= table.summary;
		this.className	= table.className;
		this.htmlborder	= (table.border == "")?null:table.border;
		this.frame		= table.frame;
		this.rules		= table.rules;
		this.cellSpacing= table.cellSpacing;
		this.cellPadding= table.cellPadding;
		if (table.style.width != "") this.width = table.style.width;
		if (table.width != "") this.htmlwidth = table.width;
		this.extra = getExtra(table);

		this.style = "";
		if (table.style.background != "")		this.style += "background: " + table.style.background + ";";
		var borderParts = table.style.border.split(' ');
		var i = 0;
		while (i < borderParts.length) {
			if (borderParts[i].indexOf('px') != -1) {
				this.border = borderParts[i].substring(0, borderParts[i].indexOf('px'));
			} else if ((borderParts[i].indexOf('#') != -1) || (colorsStr.indexOf(borderParts[i]) != -1)) {
				// FIXME: Mozilla uses rgb(10, 20, 0) while while IE uses #FFF000
				// this wont work on Mozilla
				this.borderColor = borderParts[i];
			} else {
				this.borderStyle = borderParts[i];
			}
			i++;
		}
		// IE has "1px" but Gecko has "1px 1px 1px 1px"
		if (this.border == null || this.border == "") {
			widthParts = table.style.borderWidth.split(' ');
			this.border = widthParts[0].substring(0, widthParts[0].indexOf('px'));
		}
		// IE has "dotted" but Gecko has "solid solid solid solid"
		if (this.borderStyle == null || this.borderStyle == "") {
			styleParts = table.style.borderStyle.split(' ');
			this.borderStyle = styleParts[0];
		}

		this.style += getStyle(table);
		this.rows = table.rows.length;
		this.cols = 0;
		for (i=0;i<table.rows.length;i++)
			if (this.cols < table.rows[i].cells.length) this.cols = table.rows[i].cells.length;

		this.matrix = Array();
		var placeHolders = Array(); //used to hold cells to be inserted due to colspan
		
		// To keep track of dummy cells inserted in the table (for rowspan)
		var addedplaceHolders = Array();
		for(r=0; r<this.rows; r++) {
			addedplaceHolders[r] = Array();
			for(c=0; c<this.cols; c++)
				addedplaceHolders[r][c] = false;
		}

		for (r = 0;r<this.rows;r++) {
			var temp = new TRow(this, r);
			var row = table.rows[r];
			temp.className = row.className;
			temp.align = (row.align == "")?"left":row.align;
			// IE reports empty string for undefined vAlign
			temp.valign = ((row.vAlign + "" == "undefined") || (row.vAlign == ""))?"middle":row.vAlign;
			temp.extra = getExtra(row);
			temp.style = getStyle(row);
			temp.bg = (row.style.backgroundColor == "")?null:row.style.backgroundColor;
			temp.height = (row.style.height == "")?null:row.style.height;
			for (c = 0, cfake = 0; cfake<this.cols; c++, cfake++) {
				
				for(pc = c; pc < this.cols; pc++) {
					for(i = 0; i < placeHolders.length; i++) {
						if (!addedplaceHolders[r][pc] && placeHolders[i].r == r && placeHolders[i].c == pc) {
							var CellDummy = new TCell(temp);
							CellDummy.visible = false;
							temp.cells.push(CellDummy);
							addedplaceHolders[r][pc] = true;
							continue;
						}
					}
				}// end for pc

				var Cell = new TCell(temp);
				if (c<row.cells.length) {
					var cell = row.cells[c];
					Cell.className	= cell.className;
					Cell.content	= cell.innerHTML;
					Cell.th = (cell.tagName == "TH");
					Cell.align		= (cell.align == "")?"left":cell.align;
					Cell.valign		= (cell.vAlign + "" == "undefined" || cell.vAlign == '')?"middle":cell.vAlign;
					Cell.colspan	= cell.colSpan;
					Cell.rowspan	= cell.rowSpan;
					Cell.abbr		= cell.abbr;
					Cell.axis		= cell.axis;
					Cell.scope		= cell.scope;
					Cell.extra		= getExtra(cell);
					Cell.style		= getStyle(cell);
					Cell.bg			= (cell.style.backgroundColor == "")?null:cell.style.backgroundColor;
					Cell.borderColor= (cell.style.borderColor == "")?null:cell.style.borderColor;
					Cell.borderStyle= (cell.style.borderStyle == "")?null:cell.style.borderStyle;
					Cell.borderWidth= (cell.style.borderWidth == "")?null:cell.style.borderWidth;
					Cell.cellWidth	= (cell.style.width == "")		?null:cell.style.width;
					Cell.cellHeight	= (cell.style.height == "")		?null:cell.style.height;
					if (Cell.borderColor != null && Cell.borderColor.indexOf(")") > 0) Cell.borderColor = Cell.borderColor.substring(0,Cell.borderColor.indexOf(")") + 1)
					var headers	= cell.headers;
					if (headers != "") {
						headers += " ";
						for (i = 0;i<table.rows.length;i++) {
							for (j = 0;j<table.rows[i].cells.length;j++) {
								if (headers.indexOf(table.rows[i].cells[j].id) > -1) {
									Cell.headers.push(new THeader(i, j));
								}
							}
						}
					}
				} else {
					Cell.visible	= false;
				}
				temp.cells.push(Cell);
				// to deal with inconsistencies brought up between imported tables and
				// ones modified. The following loops add dummy cells for rowspan and colspan
				if (Cell.colspan > 1) {
					for (i = 2; i <= Cell.colspan; i++) {
						var CellDummy = new TCell(temp);
						CellDummy.visible = false;
						temp.cells.push(CellDummy);
						cfake++;
					}
				}
				if (Cell.rowspan > 1) {
					for (i = 1; i < Cell.rowspan; i++) {
						var dummy = new THeader(r + i, cfake);
						placeHolders.push(dummy);
					}
				}
			}
			this.matrix.push(temp);
		}
		try {
			this.caption = table.caption.innerHTML;
		}
		catch(e) {
			this.caption = null;
		}
		document.getElementById("tid").value = this.id;
		document.getElementById("caption").value = this.caption == null? "" : this.caption;
		document.getElementById("class").value = this.className == null? "" : this.className;
		document.getElementById("cellspacing").value = this.cellSpacing;
		document.getElementById("cellpadding").value = this.cellPadding;
		document.getElementById("summary").value = this.summary;
		if (this.width.indexOf("%") > -1) {
			document.getElementById("width").value = this.width.substring(0, this.width.length - 1);
			document.getElementById("widthtype").value = "%";
		} else {
			document.getElementById("width").value = this.width.substring(0, this.width.length - 2);
			document.getElementById("widthtype").value = this.width.substring(this.width.length - 2);
		}
		document.getElementById("frame").value = this.frame;
		document.getElementById("rules").value = this.rules;
		document.getElementById("html_table_border").value = (this.htmlborder == null)? "" : this.htmlborder;
		document.getElementById("table_border").value = (this.border == null)? "" : this.border;
		document.getElementById("table_bordertype").value = (this.borderStyle == null)? "" : this.borderStyle;
		if (this.htmlwidth.indexOf('%') != -1) {
			document.getElementById("htmlwidth").value = this.htmlwidth.substring(0, (this.htmlwidth.length - 1));
			document.getElementById("htmlwidthtype").value = "%";
		} else {
			document.getElementById("htmlwidth").value = this.htmlwidth;
			document.getElementById("htmlwidthtype").value = "";
		}

		this.refresh();
	}


	this.setClass = function(name)
	{
		this.className = name;
	}
	this.setRowClass = function(name)
	{
		if (table.r == null) return false;
		table.matrix[table.r].className = name;
		this.refresh();
	}
	this.setCellClass = function(name)
	{
		if (table.r == null || table.c == null) return false;
		table.matrix[table.r].cells[table.c].className = name;
		this.refresh();
	}
	this.setColClass = function(name)
	{
		// only show the class name of the first cell as column class name
		// every cell in the selected column will get the new class name
		if (table.c == null) return false;
		for (i = 0; i < this.rows; i++) {
			this.matrix[i].cells[this.c].className = name;
		}
		this.refresh();
	}


	this.setID = function(id)
	{
		this.id = id;
	}


	this.setCaption = function(val)
	{
		this.caption = val;
		this.refresh();
	}


	this.setCellSpacing = function(val)
	{
		this.cellSpacing = val;
		this.refresh();
	}


	this.setCellPadding = function(val)
	{
		this.cellPadding = val;
		this.refresh();
	}


	this.setSummary = function(val)
	{
		this.summary = val;
		this.refresh();
	}


	this.setWidth = function(val)
	{
		this.width = val;
		this.refresh();
	}

	this.setHTMLWidth = function(val)
	{
		this.htmlwidth = val;
		this.refresh();
	}

	//Set Column widths
	this.setColumnWidth = function(new_width)
	{
		for (i = 0; i < this.rows; i++) {
			this.matrix[i].cells[this.c].cellWidth = new_width;
		}
		this.refresh();
	}

	//Set row Heights
	this.setRowHeight = function(new_height)
	{
		this.matrix[this.r].height = new_height;
		this.refresh();
	}

	//Set width of individual cell
	this.setCellWidth = function(new_width)
	{
		this.matrix[this.r].cells[this.c].cellWidth = new_width;
		this.refresh();
	}

	//Set width of individual cell
	this.setCellHeight = function(new_height)
	{
		this.matrix[this.r].cells[this.c].cellHeight = new_height;
		this.refresh();
	}

	this.setTableHtmlBorder = function(new_size) {
		if (isNaN(new_size) || new_size == 0 || new_size == "") {
			this.htmlborder = null;
		} else {
			this.htmlborder = new_size
		}
		this.refresh();
	}

	this.setElementBorder = function(new_size, style)
	{
		//If border size text box is empty or text, set the size to zero
		if (isNaN(new_size) || new_size.length == 0) {
			this.border = null;
			this.borderStyle = null;
		} else {
			if (this.lastSelect == 'row') {
				for (i = 0; i < this.cols; i++) {
					this.matrix[this.r].cells[i].borderWidth = new_size + 'px';
					this.matrix[this.r].cells[i].borderStyle = style;
				}
			} else if (this.lastSelect == 'col') {
				for (i = 0; i < this.rows; i++) {
					this.matrix[i].cells[this.c].borderWidth = new_size + 'px';
					this.matrix[i].cells[this.c].borderStyle = style;
				}
			} else if (this.lastSelect == 'cell') {
					this.matrix[this.r].cells[this.c].borderWidth = new_size + 'px';
					this.matrix[this.r].cells[this.c].borderStyle = style;
			} else if (this.selector == 'table') {
					this.border = new_size;
					this.borderStyle = style;
			}
		}
		this.refresh();
	}

	this.setFrame = function(val)
	{
		this.frame = val;
		this.refresh();
	}


	this.setRules = function(val)
	{
		this.rules = val;
		this.refresh();
	}

}


TMouse = function(parent)
{
	this.parent = parent;

	this.Move = function(e)
	{
		var img = document.getElementById("m" + parent.selector);
		img.style.display = "block";
		img.style.left = e.clientX + 15 + "px";
		img.style.top  = e.clientY + 15 + "px";
	}


	this.Out = function()
	{
		document.getElementById("mcell").style.display = "none";
		document.getElementById("mrow").style.display = "none";
		document.getElementById("mhead").style.display = "none";
		document.getElementById("mcol").style.display = "none";
	}
}
