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
* $Id: InputTable.js,v 1.3 2012/08/30 01:09:08 ewang Exp $
*
*/

/**
* InputTable Class
*/


/**
* Constructor of the InputTable class
*
* @param	id		id of the table
* @param	varname	name of the object variable
*
* @return void
* @access public
*/
function InputTable(id, varname)
{
	this.datapath = '';
	this.id = id;
	this.tb = new Table(id);
	this.varname = varname;
	this.tb.addCol('<table cellspacing="0" cellpadding="0" border="0" class="insidetabletop"><tr><td><input name="' + id + '[1][1]" id="' + id + '[1][1]" onkeyup="' + varname + '.rebuild()" class="titleCell" style="width:100px"></td></tr></table>');
	this.tb.addCol('<table cellspacing="0" cellpadding="0" border="0" class="insidetabletop"><tr><td><input name="' + id + '[1][2]" id="' + id + '[1][2]" onkeyup="' + varname + '.rebuild()" class="titleCell"></td><td><img id="img_' + id + '1_2" src="' + this.datapath +'/cross.png" onclick="' + this.varname + '.delCol(this.parentNode.parentNode.parentNode.parentNode.parentNode.id)" width="15" height="15"></td></tr></table>');

	this.addRow = it_addRow;
	this.addCol = it_addCol;
	this.delCol = it_delCol;
	this.delRow = it_delRow;
	this.rebuild = it_rebuild;
	this.setup = it_setup;
	this.input = it_input;
}//end InputTable


/**
* Set up table's content
*
* @param	Quests	Array of the values
*
* @return void
* @access public
*/
function it_setup(Quests)
{
	if (typeof Quests != "undefined") {
		for (var j=0;j<Quests.length;j++)
			for (var i=0;i<Quests[j].length;i++)
			{
				if (this.input(j+1,1) == null) this.addRow();
				if (this.input(1,i+1) == null) this.addCol();
				this.input(j+1, i+1).value = Quests[j][i];
			}
	}
	this.rebuild();

}//end it_setup


/**
* Returns input object at row and col
*
* @param	row	row of the input
* @param	col	column of the input
*
* @return void
* @access public
*/
function it_input(row, col)
{
	return document.getElementById(this.id + '[' + row + "][" + col + "]");

}//end it_input


/**
* Add new row to the end of the table
*
* @return void
* @access public
*/
function it_addRow()
{
	this.tb.addRow();
	this.tb.setCell(this.tb.rows,1,'<table cellspacing="0" cellpadding="0" border="0" class="insidetable"><tr><td><input name="' + this.id + '['+this.tb.rows+']['+1+']" id="' + this.id + '['+this.tb.rows+']['+1+']" onkeyup="' + this.varname + '.rebuild()" class="titleCell"></td><td><img id="img_' + this.id + this.tb.rows + '_1" src="' + this.datapath + '/cross.png" onclick="' + this.varname + '.delRow(this.parentNode.parentNode.parentNode.parentNode.parentNode.id)"></td></tr></table>');
	for (var i=2; i<=this.tb.cols; i++)
		this.tb.setCell(this.tb.rows,i,'<input name="' + this.id + '['+this.tb.rows+']['+i+']" id="' + this.id + '['+this.tb.rows+']['+i+']" onkeyup="' + this.varname + '.rebuild()" class="basicCell">');

}//end it_addRow


/**
* Add new column to the end of the table
*
* @return void
* @access public
*/
function it_addCol()
{
	this.tb.addCol();
	this.tb.setCell(1,this.tb.cols,'<table cellspacing="0" cellpadding="0" border="0" class="insidetabletop"><tr><td><input name="' + this.id + '['+1+']['+this.tb.cols+']" id="' + this.id + '['+1+']['+this.tb.cols+']" onkeyup="' + this.varname + '.rebuild()" class="titleCell"></td><td><img id="img_' + this.id + '1_'+this.tb.cols+'" src="' + this.datapath +'/cross.png" onclick="' + this.varname + '.delCol(this.parentNode.parentNode.parentNode.parentNode.parentNode.id)"></td></tr></table>');
	for (var j=2; j<=this.tb.rows; j++)
		this.tb.setCell(j,this.tb.cols,'<input name="' + this.id + '['+j+']['+this.tb.cols+']" id="' + this.id + '['+j+']['+this.tb.cols+']" onkeyup="' + this.varname + '.rebuild()" class="basicCell">');

}//end it_addCol


/**
* Remove cokumn from the table
*
* @param	col	number of the table's column
*
* @return void
* @access public
*/
function it_delCol(col)
{
	col = col.substring(this.id.length + 2);
	col = col.substring(col.indexOf("_")+1)*1;
	if (col != this.tb.cols) {
		for (var j=1; j<=this.tb.rows; j++)
			for (var i=col; i<this.tb.cols; i++) {
				this.input(j, i).className = this.input(j, i+1).className;
				this.input(j, i).disabled = this.input(j, i+1).disabled;
				this.input(j, i).value = this.input(j, i+1).value;
			}
			this.tb.delCol();
	}
	this.rebuild();

}//end it_delCol


/**
* Remove row from the table
*
* @param	row	number of the table's row
*
* @return void
* @access public
*/
function it_delRow(row)
{
	row = row.substring(this.id.length + 2);
	row = row.substring(0, row.indexOf("_"))*1;
	if (row != this.tb.rows) {
		for (var j=row; j<this.tb.rows; j++) {
			for (var i=1; i<=this.tb.cols; i++) {
				this.input(j, i).className = this.input(j+1, i).className;
				this.input(j, i).disabled = this.input(j+1, i).disabled;
				this.input(j, i).value = this.input(j+1, i).value;
			}
		}
		this.tb.delRow();
	}
	this.rebuild();

}//end it_delRow

/**
* Redraw table add or remove rows and cols
*
*
* @return void
* @access public
*/
function it_rebuild()
{
	var emptycol = true;
	while (emptycol && this.tb.cols > 2)
	{
		for (var i=this.tb.cols-1;i<=this.tb.cols;i++)
			if (i>0)
			for (var j=1;j<=this.tb.rows;j++)
			{
				if (this.input(j, i) != null && (this.input(j, i).value != "")) emptycol = false;
			}
		if (emptycol) this.tb.delCol();
	}
	if (this.input(1, this.tb.cols) != null && (this.input(1, this.tb.cols).value != "")) this.addCol();

	var emptyrow = true;
	while (emptyrow && this.tb.rows > 1)
	{
		for (var i=1;i<=this.tb.cols;i++)
			for (var j=this.tb.rows-1;j<=this.tb.rows;j++)
			if (j>0) {
				if (this.input(j, i) != null && (this.input(j, i).value != "")) emptyrow = false;
			}
		if (emptyrow) this.tb.delRow();
	}
	if (this.input(this.tb.rows, 1) != null && (this.input(this.tb.rows, 1).value != "")) this.addRow();

	for (var j=2;j<=this.tb.rows;j++)
		for (var i=2;i<=this.tb.cols;i++)
		{
			if (this.input(j, 1).value != "" && this.input(1, i).value != "") {
				this.input(j, i).disabled = false;
				this.input(j, i).className = "basicCell";
			} else {
				this.input(j, i).disabled = true;
				this.input(j, i).className = "disabledCell";
			}
			if (document.getElementById("img_" + this.id + j + "_1") != null) document.getElementById("img_" + this.id + j + "_1").src = this.datapath +"/cross.png";
			if (document.getElementById("img_" + this.id + "1_" + i) != null) document.getElementById("img_" + this.id + "1_" + i).src = this.datapath +"/cross.png";
		}
		if (document.getElementById("img_" + this.id + "1_" + this.tb.cols) != null) document.getElementById("img_" + this.id + "1_" + this.tb.cols).src = this.datapath +"/cross2.png";
		if (document.getElementById("img_" + this.id + this.tb.rows + "_1") != null) document.getElementById("img_" + this.id + this.tb.rows + "_1").src = this.datapath +"/cross2.png";

}//end it_rebuild




/**
* DynTable Class
*/

/**
* Constructor of the dynamic table class
*
* @param	id	id of the table
*
* @return	object
* @access public
*/
function Table(id)
{
	this.id = id;
	this.cols = 0;
	this.rows = 1;
	thistable = document.getElementById(this.id);
	if (typeof document.all != "undefined") thistable = thistable.firstChild;
	this.table = thistable;
	tr = document.createElement("tr");
	this.table.appendChild(tr);
	tr.setAttribute("id",this.id + "tr" + this.rows);

	this.addCol = t_addCol;
	this.addRow = t_addRow;
	this.setCell = t_setCell;
	this.delCol = t_delCol;
	this.delRow = t_delRow;

}//end Table()


/**
* Adding column to the table
*
* @param	val	content of the cell [optional]
*
* @return void
* @access public
*/
function t_addCol(val)
{
	this.cols++;
	for (var i=1; i<=this.rows; i++)
	{
		td = document.createElement("td");
		tr = document.getElementById(this.id + "tr" + i)
		tr.appendChild(td);
		td.setAttribute("id", this.id + "td" + i + "_" + this.cols);
		td.innerHTML = (typeof val == "undefined" || i>1)?'':val;
	}

}//end t_addCol()


/**
* Adding row to the table
*
* @access public
*/
function t_addRow()
{
	this.rows++;
	tr = document.createElement("tr");
	this.table.appendChild(tr);
	tr.setAttribute("id",this.id + "tr" + this.rows);
	for (var i=1; i<=this.cols; i++)
	{
		td = document.createElement("td");
		tr.appendChild(td);
		td.setAttribute("id", this.id + "td" + this.rows + "_" + i);
	}

}//end t_addRow()


/**
* Remove row from the table
*
* @param	row	row number, default last
*
* @return void
* @access public
*/
function t_delRow(row)
{
	row = (typeof row == "undefined")?this.rows:row;
	tr = document.getElementById(this.id + "tr" + row);
	this.table.removeChild(tr);

	if (row != this.rows) {
		for (var j=row + 1; j<=this.rows; j++) {
			document.getElementById(this.id + "tr" + j).id = this.id + "tr" + (j-1);
			for (var i=1; i<=this.cols; i++) {
				document.getElementById(this.id + "td" + j + "_" + i).id = this.id + "td" + (j-1) + "_" + i;
			}
		}
	}
	this.rows--;

}//end t_delRow


/**
* Remove column from the table
*
* @param	col	column to remove, default last
*
* @return void
* @access public
*/
function t_delCol(col)
{
	col = (typeof col == "undefined")?this.cols:col;
	for (var i=1; i<=this.rows; i++)
	{
		td = document.getElementById(this.id + "td" + i + "_" + col);
		tr = document.getElementById(this.id + "tr" + i);
		tr.removeChild(td);
	}

	if (col != this.cols) {
		for (var j=1; j<=this.rows; j++)
			for (var i=col + 1; i<=this.cols; i++) {
				td = document.getElementById(this.id + "td" + j + "_" + i).id = this.id + "td" + j + "_" + (i-1);
			}
	}
	this.cols--;

}//end t_delCol


/**
* Put value to any cell of the table
*
* @param	row	row number of the cell
* @param	col	column number of the cell
* @param	val	new HTML content of the cell
*
* @return void
* @access public
*/
function t_setCell(row, col, val)
{
	document.getElementById(this.id + "td" + row + "_" + col).innerHTML = val;
}//end t_setCell


