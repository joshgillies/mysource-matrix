/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: cal_navigator.js,v 1.2 2004/09/10 01:48:05 dbaranovskiy Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Constructor of the calendar object
*
* @param	varname	name of the object variable (for internal links)
* @param	divname	name of the target DIV or TD or other parent HTML object
* @param	width	width of the calendar [optional]
* @param	height	height of the calendar [optional]
* @param	year	initial year [optional]
* @param	month	initial month [optional]
* @param	day	initial day [optional]
*
* @return object
* @access public
*/
function Calendar(varname, divname, width, height, year, month, day)
{
	this.width = (typeof(width) == "undefined")?"100%":width;
	this.height = (typeof(height) == "undefined")?"100%":height;
	var dt = new Date();
	this.selday = (typeof(day) == "undefined")?dt.getDate():day;
	this.month = (typeof(month) == "undefined")?dt.getMonth():month - 1;
	this.year = (typeof(year) == "undefined")?dt.getFullYear():year;
	this.varname = varname;
	this.divname = divname;
	this.week_start = 1;
	this.mon_names = new Array("January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December");
	this.day_names = new Array("Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday");
	this.day_name_length = 2;
	this.popup = false;
	this.imageURL = null;
	this.first_time = true;

	this.output	= c_output;
	this.draw	= c_draw;
	this.show	= c_show;
	this.hide	= c_hide;
	this.setYear	= c_setYear;
	this.setMonth	= c_setMonth;
	this.setDay	= c_setDay;
	this.setDate	= c_setDate;
	this.today	= c_today;

	this.dayClick	= c_dayClick;
	this.weekClick	= c_weekClick;
	this.monthClick	= c_monthClick;
	this.yearClick	= c_yearClick;
	
	this.onDayClick = null;
	this.onWeekClick = null;
	this.onMonthClick = null;
	this.onYearClick = null;

}//end Calendar()


/**
* Pop up calendar in the setted DIV
*
* @param	e	event [should be called as "cal.show(event);"]
*
* @return
* @access public
*/
function c_show(e)
{
	this.popup = true;
	var div = document.getElementById(this.divname);
	div.style.visibility = "hidden";
	div.style.position = "absolute";
	div.style.left = e.clientX + "px";
	div.style.top  = e.clientY + "px";
	div.innerHTML = this.output();
	if (document.getElementById('ie_'+this.varname+'_iframe') == null && document.body.insertAdjacentHTML) {
		var iframe = '<iframe id="ie_'+this.varname+'_iframe" scrolling="no" border="0" frameborder="0" style="position:absolute;top:-1000px;left:-1000px;width:10px; height:10px;visibility:hidden" src="about:blank"></iframe>'+div.outerHTML;
		document.body.insertAdjacentHTML('beforeEnd', iframe);
		div.id = this.divname + "_old";
		div.innerHTML = "";
		
		div = document.getElementById(this.divname);
	}
	if (document.body.insertAdjacentHTML) {
		var cal_height = document.getElementById(this.divname).offsetHeight;
		var cal_width  = document.getElementById(this.divname).offsetWidth;
		var cal_top    = document.getElementById(this.divname).style.top;
		var cal_left   = document.getElementById(this.divname).style.left;
		var iframe = document.getElementById('ie_'+this.varname+'_iframe');
		iframe.style.top	= cal_top;
		iframe.style.left	= cal_left;
		iframe.style.width	= cal_width + "px";
		iframe.style.height	= cal_height + "px";
		iframe.style.visibility	= "visible";
	}
	div.style.visibility = "visible";

}//end c_show()


/**
* Hide pop up calendar in the setted DIV
*
*
* @return
* @access public
*/
function c_hide()
{
	this.popup = true;
	var div = document.getElementById(this.divname);
	div.style.visibility = "hidden";
	if (window.event) document.getElementById('ie_'+this.varname+'_iframe').style.visibility = "hidden";

}//end c_hide()


/**
* Draws calendar in the setted DIV
*
* @param
*
* @return
* @access public
*/
function c_draw()
{
	var output = "";
	if (!this.popup || !this.first_time) document.getElementById(this.divname).innerHTML = this.output();
	else	if (this.imageURL != null) output = '<img src="' + this.imageURL + '" style="cursor:pointer" onclick="'+this.varname+'.show(event);">';
	        else output = '<div style="width:20px;height:20px;background:#CCCCCC;border:solid 1px #DDDDDD;cursor:pointer" onclick="'+this.varname+'.show(event);"';

	if (this.popup && this.first_time) document.write(output);
	
	this.first_time = false;

}//end c_draw()


/**
* Moves calendar to today
*
*
* @access public
*/
function c_today()
{
	dt = new Date();
	this.month  = dt.getMonth();
	this.year   = dt.getFullYear();
	this.selday = dt.getDate();
	this.draw();

}//end c_today()


/**
* Change Year of the Calendar
*
* @param	year	new year
*
* @return void
* @access public
*/
function c_setYear(year)
{
	if (year == "--") this.year--;
	else	if (year == "++") this.year++;
		else this.year = year;
	this.draw();

}//end c_setYear()


/**
* Change Month of the Calendar
*
* @param	mon	new month (0..11)
*
* @return void
* @access public
*/
function c_setMonth(mon)
{
	if (mon == "--") this.month--;
	else	if (mon == "++") this.month++;
		else this.month = mon - 1;
	if (this.month > 11)
	{
		this.month -= 12;
		this.year++;
	}
	if (this.month < 0)
	{
		this.month += 12;
		this.year--;
	}
	this.draw();

}//end c_setMonth()


/**
* Change day of the calendar
*
* @param	day	new day
*
* @return void
* @access public
*/
function c_setDay(day)
{
	this.selday = day;
	var td_day = document.getElementById(this.varname + "_td_" + day);
	if (td_day != null) td_day.style.background = this.sel_day_bg;
}//end c_setDay()


/**
* Generates output string for internal use
*
*
* @return string [HTML]
* @access public
*/
function c_output()
{
	var dt = new Date();
	var td = new Date();
	dt.setMonth(this.month);
	dt.setYear(this.year);
	dt.setDate(1);
	
	var is_week_set  = this.onWeekClick  != null;
	var is_day_set   = this.onDayClick   != null;
	var is_month_set = this.onMonthClick != null;
	var is_year_set  = this.onYearClick  != null;
	
	if (is_week_set) colspan = 6;
	else colspan = 5;

	//table difinition
	var output = '<table class="cal" cellspacing="1" width="' + this.width + '" height="' + this.height + '">';
	
	//top bar for popup
	if (this.popup)	output += '<tr style="height:1px"><td colspan="' + (colspan + 2) + '" align="right"><span class="cal_close" onclick="' + this.varname +'.hide();">&times;</span></td></tr>';
	
	
	//month caption
	output += '<tr style="height:1px"><td class="cal_arrow" onclick="' + this.varname +
		'.setMonth(\'--\')" align="left">&laquo;</td><td class="cal_month" colspan="' + colspan + '" align="center" onclick="' +
		this.varname + '.monthClick(1,' + this.month + ',' + this.year + ')">' +
		this.mon_names[this.month] + '</td><td class="cal_arrow" onclick="' + this.varname +
		'.setMonth(\'++\')" align="right">&raquo;</td></tr>';
	//year caption
	output += '<tr style="height:1px"><td class="cal_arrow" onclick="' + this.varname +
		'.setYear(\'--\')" align="left">&laquo;</td><td class="cal_year" colspan="' + colspan + '" align="center" onclick="' +
		this.varname + '.yearClick(1,1,' + this.year + ')">' +
		this.year + '</td><td class="cal_arrow" onclick="' + this.varname +
		'.setYear(\'++\')" align="right">&raquo;</td></tr>';
	//week days
	output += '<tr style="height:1px">';
	if (is_week_set) output += '<td class="cal_week">&nbsp;</td>';
	var j = this.week_start;
	var end_day = (this.week_start == 0)?6:this.week_start - 1;
	while (j != end_day)
	{
		output +='<td class="cal_week_day">' + this.day_names[j].substring(0,this.day_name_length) + '</td>';
		j++;
		if (j == 7) j = 0;
	}
	output +='<td class="cal_week_day">' + this.day_names[j].substring(0,this.day_name_length) + '</td>';
	output += '</tr>';

	//previous month days
	var day = dt.getDay();
	var j = this.week_start;
	output += '<tr id="' + this.varname + '_tr_0">';
	if (is_week_set) output += '<td class="cal_week" onmouseout="document.getElementById(\'' + this.varname + '_tr_0\').className=\'\'" onmouseover="document.getElementById(\'' + this.varname + '_tr_0\').className=\'cal_ovr\'" onclick="' + this.varname + '.weekClick(1,' + dt.getMonth() + ',' + dt.getFullYear() + ')">&raquo;</td>';

	var diff = 1 + (j - day);
	if (day < j) diff = 1 + (j - day - 7);
	dt.setDate(diff);

	while (j != day && (diff <= 0))
	{
		output += '<td class="cal_empty" onmouseover="this.className=\'cal_ovr\'" onmouseout="this.className=\'cal_empty\'" onclick="' + this.varname + '.dayClick(' + dt.getDate() + ',' + dt.getMonth() + ',' + dt.getFullYear() + ')">' + dt.getDate() + '</td>';
		dt.setDate(dt.getDate() + 1);
		j++;
		if (j == 7) j = 0;
	}

	//general month days
	dt.setMonth(this.month);
	dt.setYear(this.year);
	dt.setDate(1);

	j = 1;
	while (dt.getMonth() == this.month)
	{
		if (dt.getDay() == this.week_start && day != this.week_start)
		{
			output += '</tr><tr id="' + this.varname + '_tr_' + j + '">';
			if (is_week_set) output += '<td class="cal_week" onmouseout="document.getElementById(\'' + this.varname + '_tr_' + j +
				'\').className=\'\'" onmouseover="document.getElementById(\'' + this.varname + '_tr_' + j +
				'\').className=\'cal_ovr\'" onclick="' + this.varname +
				'.weekClick(' + dt.getDate() + ',' + dt.getMonth() + ',' + dt.getFullYear() + ')">&raquo;</td>';
			j++;
			if (j == 7) j = 0;
		}
		if (dt.getDay() == 0 || dt.getDay() == 6) clss = "cal_hol";
		else clss = "cal_day";
		if (dt.getDate() == this.selday) clss += "_sel";
		else if (dt.getFullYear() == td.getFullYear() && dt.getMonth() == td.getMonth() && dt.getDate() == td.getDate()) clss = "cal_today";
		output += '<td onmouseover="this.className=\'cal_ovr\'" onmouseout="this.className=\'' + clss + '\'" class="' + clss + '" id="' + this.varname + '_td_' + dt.getDate() + '" onclick="' + this.varname + '.dayClick(' + dt.getDate() + ',' + dt.getMonth() + ',' + dt.getFullYear() + ')">' + dt.getDate() + '</td>';
		dt.setDate(dt.getDate() + 1);
		day = this.week_start + 1;
	}

	//next month days
	dt.setDate(dt.getDate() - 1);
	j = dt.getDay();
	dt.setDate(dt.getDate() + 1);
	var endday = (this.week_start == 0)?6:this.week_start - 1;
	while (j != endday)
	{
		output += '<td class="cal_empty" onmouseover="this.className=\'cal_ovr\'" onmouseout="this.className=\'cal_empty\'" onclick="' + this.varname + '.dayClick(' + dt.getDate() + ',' + dt.getMonth() + ',' + dt.getFullYear() + ')">' + dt.getDate() + '</td>';
		dt.setDate(dt.getDate() + 1);
		j++;
		if (j == 7) j = 0;
	}
	output += '</tr></table>';
	return output;

}//end c_output()


/**
* click on the date event handler
*
* @param	day	selected day
* @param	mon	selected month
* @param	year	selected year
*
* @return void
* @access public
*/
function c_dayClick(day, mon, year)
{
	if (this.onDayClick != null)
	{
		this.onDayClick(day, mon, year);
		if (this.popup) this.hide();
	}

}//end c_dayClick()


/**
* click on the month event handler
*
* @param	day	selected day
* @param	mon	selected month
* @param	year	selected year
*
* @return void
* @access public
*/
function c_monthClick(day, mon, year)
{
	if (this.onMonthClick != null)
	{
		this.onMonthClick(day, mon, year);
		if (this.popup) this.hide();
	}

}//end c_monthClick()


/**
* click on the week event handler
*
* @param	day	selected day
* @param	mon	selected month
* @param	year	selected year
*
* @return void
* @access public
*/
function c_weekClick(day, mon, year)
{
	if (this.onWeekClick != null)
	{
		this.onWeekClick(day, mon, year);
		if (this.popup) this.hide();
	}

}//end c_weekClick()


/**
* click on the year event handler
*
* @param	day	selected day
* @param	mon	selected month
* @param	year	selected year
*
* @return void
* @access public
*/
function c_yearClick(day, mon, year)
{
	if (this.onYearClick != null)
	{
		this.onYearClick(day, mon, year);
		if (this.popup) this.hide();
	}

}//end c_yearClick()


/**
* Converts date to MYSQL format
*
* @param	day	given day
* @param	mon	given month
* @param	year	given year
*
* @return string
* @access public
*/
function DateConvert2MySQL(day, mon, year)
{
	var out = year + "-";
	mon++;
	if (mon < 10) out += "0";
	out += mon + "-";
	if (day < 10) out += "0";
	out += day;
	return out;

}//end DateConvert2MySQL()


/**
* Sets data for Calendar. Could use first param as MySQL format string
*
* @param	day	given day
* @param	mon	given month
* @param	year	given year
*
* @return void
* @access public
*/
function c_setDate(year, mon, day)
{
	if (typeof(year) == "string")
	{
		this.year	= year.substring(0, 4) * 1;
		this.month	= year.substring(5, 7) * 1 - 1;
		this.selday	= year.substring(8) * 1;
	}
	else
	{
		this.year	= year;
		this.month	= month;
		this.selday	= day;
	}
	this.draw();

}//end c_setDate()


/**
* Add function to body onload event
*
* @param	func_name	function call string [init()]
*
* @return void
* @access public
*/
function attachOnLoad(func_name)
{
			var onload = document.body.getAttribute("onload") + ";" + func_name;
			if (document.body.onload)
			{
				var onload_old = document.body.onload;
				function onload_new()
				{
					onload_old();
					eval(func_name+";");
				}
				document.body.onload = onload_new;
			}
			else document.body.setAttribute("onload", onload)

}//end attachOnLoad()
