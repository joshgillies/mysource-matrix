

THeader = function(r, c)
{
	this.r = r;
	this.c = c;
}


TCell = function(parent)
{
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
		out += ' onclick="' + name + '.select(this)"';
		if (this.colspan > 1) out += ' colspan="' + this.colspan + '"';
		if (this.rowspan > 1) out += ' rowspan="' + this.rowspan + '"';
		if (this.abbr != null) out += ' abbr="' + this.abbr + '"';
		if (this.scope != null) out += ' scope="' + this.scope + '"';
		if (this.align != "left") out += ' align="' + this.align + '"';
		if (this.valign != "middle") out += ' valign="' + this.valign + '"';
		if (this.headers.length > 0) {
			out +=' headers="';
			for (i = 0;i<this.headers.length;i++)
				out += name + '_td' + this.headers[i].r + '_' + this.headers[i].c + ' ';
			out +='"';
		}
		var style = ' style="';
		if (this.borderColor != null) style += 'border-color:' + this.borderColor + ';';
		if (this.borderStyle != null) style += 'border-style:' + this.borderStyle + ';';
		if (this.borderWidth != null) style += 'border-width:' + this.borderWidth + ';';
		if (this.bg != null) style += 'background-color:' + this.bg + ';';
		if (this.style != null) style += this.style;
		if (this.selected) style += 'background-image:url(' + this.parent.parent.semigray + ');';

		if (style != ' style="') out += style + '"';
		//alert(this.content);
		out += '>' + this.content;
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
		out += ' id="' + name + '_td' + r + '_' + c + '"';
		if (this.colspan > 1) out += ' colspan="' + this.colspan + '"';
		if (this.rowspan > 1) out += ' rowspan="' + this.rowspan + '"';
		if (this.abbr != null && this.abbr != "") out += ' abbr="' + this.abbr + '"';
		if (this.scope != null && this.scope != "") out += ' scope="' + this.scope + '"';
		if (this.align != "left") out += ' align="' + this.align + '"';
		if (this.valign != "middle") out += ' valign="' + this.valign + '"';
		if (this.headers.length > 0) {
			out +=' headers="';
			for (i = 0;i<this.headers.length;i++)
				out += name + '_td' + this.headers[i].r + '_' + this.headers[i].c + ' ';
			out +='"';
		}
		var style = ' style="';
		if (this.borderColor != null) style += 'border-color:' + this.borderColor + ';';
		if (this.borderStyle != null) style += 'border-style:' + this.borderStyle + ';';
		if (this.borderWidth != null) style += 'border-width:' + this.borderWidth + ';';
		if (this.bg != null) style += 'background-color:' + this.bg + ';';
		if (this.style != null) style += this.style;

		if (style != ' style="') out += style + '"';
		out += this.extra + '>' + this.content;
		if (this.th) out += '</th>';
		else out += '</td>';

		return out;

	}


	this.setPanels = function()
	{
		document.getElementById("aleft").style.background = "";
		document.getElementById("acenter").style.background = "";
		document.getElementById("aright").style.background = "";

		document.getElementById("atop").style.background = "";
		document.getElementById("amiddle").style.background = "";
		document.getElementById("abottom").style.background = "";

		document.getElementById("a" + this.align).style.background = "#F00";
		document.getElementById("a" + this.valign).style.background = "#F00";
		if (this.th) document.getElementById("THead").style.background = "#F00";
		else document.getElementById("THead").style.background = "#FFF";

		document.getElementById("abbr").value = (this.abbr == null)?"":this.abbr;
		document.getElementById("axis").value = (this.axis == null)?"":this.axis;
		document.getElementById("scope").value = (this.scope == null)?"":this.scope;

		document.getElementById("bg").style.background = (this.bg == null)?"url(" + this.parent.parent.empty + ")":this.bg;
		document.getElementById("border").style.background = (this.borderColor == null)?"url(" + this.parent.parent.empty + ")":this.borderColor;
	}
}


TRow = function(parent, row)
{
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

	this.selected	= false;

	this.toString = function()
	{
		var out = "";
		out += '<tr';
		if (this.align != "left") out += ' align="' + this.align + '"';
		if (this.valign != "middle") out += ' valign="' + this.valign + '"';
		var style = 'style="';
		if (this.borderColor != null) style += 'border-color:' + this.borderColor + ';';
		if (this.borderStyle != null) style += 'border-style:' + this.borderStyle + ';';
		if (this.borderWidth != null) style += 'border-width:' + this.borderWidth + ';';
		if (this.bg != null) style += 'background-color:' + this.bg + ';';
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
		var out = "";
		out += '<tr';
		if (this.align != "left") out += ' align="' + this.align + '"';
		if (this.valign != "middle") out += ' valign="' + this.valign + '"';
		var style = 'style="';
		if (this.borderColor != null) style += 'border-color:' + this.borderColor + ';';
		if (this.borderStyle != null) style += 'border-style:' + this.borderStyle + ';';
		if (this.borderWidth != null) style += 'border-width:' + this.borderWidth + ';';
		if (this.bg != null) style += 'background-color:' + this.bg + ';';
		if (this.style != null) style += this.style;
		if (style != 'style="') out += " " + style + '"';

		out += '>';
		for (c = 0;c<this.cells.length;c++) {
			out += this.cells[c].Export(c);
		}
		out += '</tr>';
		return out;
	}


	this.setPanels = function()
	{
		document.getElementById("aleft").style.background = "";
		document.getElementById("acenter").style.background = "";
		document.getElementById("aright").style.background = "";

		document.getElementById("atop").style.background = "";
		document.getElementById("amiddle").style.background = "";
		document.getElementById("abottom").style.background = "";

		document.getElementById("a" + this.align).style.background = "#F00";
		document.getElementById("a" + this.valign).style.background = "#F00";

		document.getElementById("bg").style.background = (this.bg == null)?"url(" + this.parent.empty + ")":this.bg;
		document.getElementById("border").style.background = (this.borderColor == null)?"url(" + this.parent.empty + ")":this.borderColor;
	}
}


TTable = function(name, rows, cols)
{
	this.matrix		= Array();
	this.summary	= null;
	this.caption	= null;
	this.cellspacing= 2;
	this.cellpadding= 2;
	this.border		= 1;
	this.rows		= rows;
	this.cols		= cols;
	this.varname	= name;
	this.id			= "";
	this.width		= "100%";
	this.frame		= "";
	this.rules		= "";
	this.style		= "";
	this.extra		= "";

	this.selector	= "cell";
	this.color		= "bg";

	this.r			= null;
	this.c			= null;

	this.mouse		= new TMouse(this);

	this.semigray	= document.getElementById("semigray").src;
	this.semired	= document.getElementById("semired").src;
	this.empty		= document.getElementById("empty").src;


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
		var out = '<table id="js_' + this.varname + '" border="' + this.border + '" cellpadding="' + this.cellpadding + '" cellspacing="' + this.cellspacing + '"';
		out += ' style="width:' + this.width + ';' + '"';
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
		var out = '<table id="' + this.id + '" border="' + this.border + '" cellpadding="' + this.cellpadding + '" cellspacing="' + this.cellspacing + '"';
		out += ' style="width:' + this.width + ';' + this.style + '"';
		if (this.summary != null && this.summary != "") out += ' summary="' + this.summary + '"';
		if (this.frame != "") out += ' frame="' + this.frame + '"';
		if (this.rules != "") out += ' rules="' + this.rules + '"';
		out += this.extra + '>';
		if (this.caption != null) out += '<caption>' + this.caption + '</caption>';
		for (r = 0;r<this.rows;r++) {
			out += this.matrix[r].Export();
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
				if (rowspan > 1 || colspan > 1)
					if (!this.refreshCell(r, c)) return false;
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


	this.setAlign = function(align)
	{
		if (this.r == null) return false;
		if (this.c == null) {
			this.matrix[this.r].align = align;
			this.matrix[this.r].setPanels();
		} else {
			this.matrix[this.r].cells[this.c].align = align;
			this.matrix[this.r].cells[this.c].setPanels();
		}
		this.refresh();

	}


	this.setVAlign = function(valign)
	{
		if (this.r == null) return false;
		if (this.c == null) {
			this.matrix[this.r].valign = valign;
			this.matrix[this.r].setPanels();
		} else {
			this.matrix[this.r].cells[this.c].valign = valign;
			this.matrix[this.r].cells[this.c].setPanels();
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


	this.select = function(td)
	{
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
		}
		if (this.selector == "cell") {
			this.matrix[rr].cells[cc].selected = true;
			this.matrix[rr].cells[cc].setPanels();
			this.c = cc;
			this.r = rr;
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
		if (this.c == null) return false;
		this.matrix[this.r].cells[this.c].th = !this.matrix[this.r].cells[this.c].th;
		this.matrix[this.r].cells[this.c].setPanels();
		this.refresh();

	}


	this.showHeaders = function()
	{
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
		for (i = this.matrix.length - 2;i>this.r;i--) {
			this.matrix[i + 1] = this.matrix[i];
			this.matrix[i + 1].row = i + 1;
		}
		this.matrix[this.r + 1] = temp;
		this.refresh();

	}


	this.delrow = function()
	{
		if (this.r == null || this.rows == 1) return false;
		for (i = this.r;i<this.rows - 1;i++) {
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
		this.refresh();

	}


	this.delcol = function()
	{
		if (this.c == null || this.cols == 1) return false;
		for(i = 0; i<this.rows; i++) {
			var row = this.matrix[i];
			for (j = this.c;j<this.cols - 1;j++) {
				row.cells[j] = row.cells[j + 1];
			}
			row.cells.pop();
		}
		this.cols--;
		this.c = null;
		this.r = null;
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


	this.setColor = function(color)
	{
		if (this.r == null) return false;
		var obj = (this.c == null)?this.matrix[this.r]:this.matrix[this.r].cells[this.c];
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
				obj.borderStyle = "solid";
				obj.borderWidth = "1px";
				document.getElementById("border").style.background = obj.borderColor;
			} else {
				obj.borderColor = null;
				obj.borderStyle = null;
				obj.borderWidth = null;
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
		this.border		= (table.border == "")?0:table.border;
		this.frame		= table.frame;
		this.rules		= table.rules;
		this.cellspacing= (table.cellSpacing == "")?2:table.cellSpacing;
		this.cellpadding= (table.cellPadding == "")?2:table.cellPadding;
		if (table.style.width != "") this.width = table.style.width;
		this.extra = getExtra(table);


		this.style = "";
		if (table.style.background != "")		this.style += "background: " + table.style.background + ";";
		if (table.style.border != "")			this.style += "border: " + table.style.border + ";";
		this.style += getStyle(table);

		this.rows = table.rows.length;

		this.cols = 0;
		for (i=0;i<table.rows.length;i++)
			if (this.cols < table.rows[i].cells.length) this.cols = table.rows[i].cells.length;

		this.matrix = Array();
		for (r = 0;r<this.rows;r++) {
			var temp = new TRow(this, r);
			var row = table.rows[r];
			temp.align = (row.align == "")?"left":row.align;
			temp.valign = (row.valign + "" == "undefined")?"middle":row.valign;
			temp.extra = getExtra(row);
			temp.style = getStyle(row);
			temp.bg = (row.style.backgroundColor == "")?null:row.style.backgroundColor;
			for (c = 0;c<this.cols;c++) {
				var Cell = new TCell(temp);
				if (c<row.cells.length) {
					var cell = row.cells[c];
					Cell.content	= cell.innerHTML;
					Cell.th = (cell.tagName == "TH");
					Cell.align		= (cell.align == "")?"left":cell.align;
					Cell.valign		= (cell.valign + "" == "undefined")?"middle":cell.valign;
					Cell.colspan	= cell.colSpan;
					Cell.rowspan	= cell.rowSpan;
					Cell.abbr		= cell.abbr;
					Cell.axis		= cell.axis;
					Cell.scope		= cell.scope;
					Cell.extra		= getExtra(cell);
					Cell.style		= getStyle(cell);
					Cell.bg			= (cell.style.backgroundColor == "")?null:cell.style.backgroundColor;
					Cell.borderColor= (cell.style.borderColor == "")?null:cell.style.borderColor;
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
		document.getElementById("caption").value = this.caption;
		document.getElementById("cellspacing").value = this.cellspacing;
		document.getElementById("cellpadding").value = this.cellpadding;
		document.getElementById("summary").value = this.summary;
		if (this.width.indexOf("%") > -1) {
			document.getElementById("width").value = this.width.substring(0, this.width.length - 1);
			document.getElementById("widthtype").value = "%";
		} else {
			document.getElementById("width").value = this.width.substring(0, this.width.length - 2);
			document.getElementById("widthtype").value = this.width.substring(this.width.length - 2);
		}
		//document.getElementById("width").value = this.width; //TODO
		document.getElementById("frame").value = this.frame;
		document.getElementById("rules").value = this.rules;

		this.refresh();
		//alert(this.rows + "|" + this.cols);

		//for(var prop in table.rows[0]) document.getElementById("test").value += prop + "=>" + table.rows[0][prop] + "\n";
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
		this.cellspacing = val;
		this.refresh();
	}


	this.setCellPadding = function(val)
	{
		this.cellpadding = val;
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
	}
}
