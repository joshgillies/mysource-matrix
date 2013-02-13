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
* $Id: js_calendar.js,v 1.18 2013/02/13 07:17:27 ewang Exp $
*
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
* @author	Dmitry Baranovskiy	<dbaranovskiy@squiz.net>
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
	this.initMonth	= this.month;
	this.initYear	= this.year;
	this.prefix = "";
	this.varname = varname;
	this.divname = divname;
	this.week_start = 1;
	this.mon_names = new Array("January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December");
	this.day_names = new Array("Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday");
	this.day_name_length = 2;
	this.popup = false;			//is the cal popup
	this.imageURL = null;		//URL of image if it popup
	this.fadeit = true;			//fade cal on show or not
	this.scrollit = true;		//if fadeit == false scroll cal on show

	this.first_time = true;
	this.time = null;
	this.fade = 0;
	this.scroll = 0;

	this.output	= c_output;
	this.draw	= c_draw;
	this.show	= c_show;
	this.hide	= c_hide;
	this.setYear	= c_setYear;
	this.setMonth	= c_setMonth;
	this.setDay	= c_setDay;
	this.setDate	= c_setDate;
	this.today	= c_today;
	this.fadeOn	= c_fadeOn;
	this.fadeOff	= c_fadeOff;
	this.scrollOn	= c_scrollOn;
	this.scrollOff	= c_scrollOff;

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

	var coordX = 0;
	var coordY = 0;
	if (e.pageX || e.pageY) {
		coordX = e.pageX;
		coordY = e.pageY;
	} else if (e.clientX || e.clientY) {
		var top = (document.documentElement && document.documentElement.scrollTop) || document.body.scrollTop;
		var left = (document.documentElement && document.documentElement.scrollLeft) || document.body.scrollLeft;
		coordX = e.clientX + left;
		coordY = e.clientY + top;
	}

	// move the div to the top level of the document tree so that other absolutely-positioned
	// elements don't affect its position, then move it to the correct co-ordinates
	div.parentNode.removeChild(div);
	document.body.appendChild(div);
	div.style.left = (coordX + 6) + "px";
	div.style.top  = (coordY + 6) + "px";
	div.style.zIndex = 99999;
	div.style.background = 'white';
	div.innerHTML = this.output();

	if (document.getElementById('ie_'+this.divname+'_iframe') == null && (typeof document.body.insertAdjacentHTML != 'undefined')) {
		var shadow = '<span id="ie_'+this.divname+'_shadow" style="background:#000000;position:absolute;top:0px;left:0px;filter:progid:DXImageTransform.Microsoft.blur(pixelradius=6, enabled=\'true\', makeshadow=\'true\', ShadowOpacity=0.7)"></span>';
		var iframe = '<iframe id="ie_'+this.divname+'_iframe" scrolling="no" border="0" frameborder="0" style="filter:alpha(opacity=0);position:absolute;top:-1000px;left:-1000px;width:10px; height:10px;visibility:hidden;z-index:9999" src="about:blank"></iframe>' + shadow + outerHTML(div);
		document.body.insertAdjacentHTML('beforeEnd', iframe);
		div.id = this.divname + "_old";
		div.innerHTML = "";

		div = document.getElementById(this.divname);
	}
	if ((typeof document.body.insertAdjacentHTML != 'undefined')) {
		var cal_top    = document.getElementById(this.divname).style.top;
		var cal_height = document.getElementById(this.divname).offsetHeight;
		var cal_width  = document.getElementById(this.divname).offsetWidth;
		var cal_left   = document.getElementById(this.divname).style.left;
		var iframe = document.getElementById('ie_'+this.divname+'_iframe');
		var shadow = document.getElementById('ie_'+this.divname+'_shadow');
		iframe.style.top	= cal_top;
		iframe.style.left	= cal_left;
		iframe.style.width	= cal_width + "px";
		iframe.style.height	= cal_height + "px";
		iframe.style.visibility	= "visible";
		shadow.style.top	= cal_top.substring(0, cal_top.length - 2) - 2 + "px";
		shadow.style.left	= cal_left.substring(0, cal_left.length - 2) - 6 + "px";
		shadow.style.width	= cal_width + "px";
		shadow.style.height	= cal_height + "px";
		iframe.style.zIndex     = 3000;
		if (this.fadeit || this.scrollit) shadow.style.visibility	= "hidden";
		else shadow.style.visibility	= "visible";
	}
	if (this.fadeit) {
		this.fade = 0;
		this.fadeOn();
	} else if (this.scrollit) {
		this.scroll = 0;
		this.scrollOn();
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
	if (this.fadeit) {
		this.fade = 0;
		this.fadeOff();
	} else
	if (this.scrollit) {
		this.scroll = 96;
		this.scrollOff();
	} else div.style.visibility = "hidden";
	if (document.body.insertAdjacentHTML){
		if (!this.fadeit && !this.scrollit) document.getElementById('ie_'+this.divname+'_iframe').style.visibility = "hidden";
		document.getElementById('ie_'+this.divname+'_shadow').style.visibility = "hidden";
	}

}//end c_hide()


/**
* ScrollOn the calendar
*
*
* @return
* @access public
*/
function c_scrollOn()
{
	clearTimeout(this.time);
	var div = document.getElementById(this.divname);
	div.style.visibility = "visible";
	var cal_height = div.offsetHeight;
	var cal_width  = div.offsetWidth;
	if (this.scroll >= 100) {
		this.time = null;
		div.style.clip =  "rect(0px, "+cal_width+"px, "+cal_height+"px, 0px)";
		if (document.body.insertAdjacentHTML){
			document.getElementById('ie_'+this.divname+'_iframe').style.visibility = "visible";
			document.getElementById('ie_'+this.divname+'_shadow').style.visibility = "visible";
		}
		return;
	}
	div.style.clip =  "rect(0px, "+(this.scroll*cal_width/100)+"px, "+(this.scroll*cal_height/100)+"px, 0px)";
	this.scroll+=4;
	this.time = setTimeout(this.varname+".scrollOn()",1);
}//end c_scrollOn()


/**
* ScrollOff the calendar
*
*
* @access public
*/
function c_scrollOff()
{
	clearTimeout(this.time);
	var div = document.getElementById(this.divname);
	var cal_height = div.offsetHeight;
	var cal_width  = div.offsetWidth;
	if (this.scroll <= 0) {
		this.time = null;
		div.style.visibility = "hidden";
		if (document.body.insertAdjacentHTML){
			document.getElementById('ie_'+this.divname+'_iframe').style.visibility = "hidden";
			document.getElementById('ie_'+this.divname+'_shadow').style.visibility = "hidden";
		}
		return;
	}
	div.style.clip =  "rect(0px, "+(this.scroll*cal_width/100)+"px, "+(this.scroll*cal_height/100)+"px, 0px)";
	this.scroll-=4;
	this.time = setTimeout(this.varname+".scrollOff()",1);
}//end c_scrollOff()


/**
* Fade calendar from not transparent to transparent
*
*
* @access public
*/
function c_fadeOn()
{
	clearTimeout(this.time);
	var div = document.getElementById(this.divname);
	div.style.visibility = "visible";
	if (this.fade >= 100) {
		this.time = null;
		div.style.MozOpacity = 0.99;
		div.style.filter= 'alpha(opacity=99)';
		if (document.body.insertAdjacentHTML){
			document.getElementById('ie_'+this.divname+'_iframe').style.visibility = "visible";
			document.getElementById('ie_'+this.divname+'_shadow').style.visibility = "visible";
		}
		return;
	}
	div.style.MozOpacity = this.fade/100;
	div.style.filter= 'alpha(opacity=' + this.fade + ')';
	this.fade+=4;
	this.time = setTimeout(this.varname+".fadeOn()",1);

}//end c_fadeOn()


/**
* Fade calendar from transparent to not
*
*
* @access public
*/
function c_fadeOff()
{
	clearTimeout(this.time);
	var div = document.getElementById(this.divname);
	if (this.fade <= 0) {
		this.time = null;
		div.style.visibility = "hidden";
		if (document.body.insertAdjacentHTML){
			document.getElementById('ie_'+this.divname+'_iframe').style.visibility = "hidden";
			document.getElementById('ie_'+this.divname+'_shadow').style.visibility = "hidden";
		}
		return;
	}
	div.style.MozOpacity = this.fade/100;
	div.style.filter= 'alpha(opacity=' + this.fade + ')';
	this.fade-=4;
	this.time = setTimeout(this.varname+".fadeOff()",1);

}//end c_fadeOff()


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
	dt.setDate(1);
	dt.setMonth(this.month);
	dt.setYear(this.year);

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
		this.varname + '.monthClick(1,' + (this.month + 1) + ',' + this.year + ')">' +
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
	if (is_week_set) output += '<td class="cal_week" onmouseout="document.getElementById(\'' + this.varname + '_tr_0\').className=\'\'" onmouseover="document.getElementById(\'' + this.varname + '_tr_0\').className=\'cal_ovr\'" onclick="' + this.varname + '.weekClick(1,' + (dt.getMonth()+1) + ',' + dt.getFullYear() + ')">&raquo;</td>';

	var diff = 1 + (j - day);
	if (day < j) diff = 1 + (j - day - 7);
	dt.setDate(diff);

	while (j != day && (diff <= 0))
	{
		output += '<td class="cal_empty" onmouseover="this.className=\'cal_ovr\'" onmouseout="this.className=\'cal_empty\'" onclick="' + this.varname + '.dayClick(' + dt.getDate() + ',' + (dt.getMonth()+1) + ',' + dt.getFullYear() + ')">' + dt.getDate() + '</td>';
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
				'.weekClick(' + dt.getDate() + ',' + (dt.getMonth()+1) + ',' + dt.getFullYear() + ')">&raquo;</td>';
			j++;
			if (j == 7) j = 0;
		}
		if (dt.getDay() == 0 || dt.getDay() == 6) clss = "cal_hol";
		else clss = "cal_day";
		if (dt.getDate() == this.selday && dt.getMonth() == this.initMonth && dt.getFullYear() == this.initYear) clss += "_sel";
		else if (dt.getFullYear() == td.getFullYear() && dt.getMonth() == td.getMonth() && dt.getDate() == td.getDate()) clss = "cal_today";
		output += '<td onmouseover="this.className=\'cal_ovr\'" onmouseout="this.className=\'' + clss + '\'" class="' + clss + '" id="' + this.varname + '_td_' + dt.getDate() + '" onclick="' + this.varname + '.dayClick(' + dt.getDate() + ',' + (dt.getMonth()+1) + ',' + dt.getFullYear() + ')">' + dt.getDate() + '</td>';
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
		output += '<td class="cal_empty" onmouseover="this.className=\'cal_ovr\'" onmouseout="this.className=\'cal_empty\'" onclick="' + this.varname + '.dayClick(' + dt.getDate() + ',' + (dt.getMonth()+1) + ',' + dt.getFullYear() + ')">' + dt.getDate() + '</td>';
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
		this.onDayClick(day, mon, year, this.prefix);
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
		this.onMonthClick(day, mon, year, this.prefix);
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
		this.onWeekClick(day, mon, year, this.prefix);
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
		this.onYearClick(day, mon, year, this.prefix);
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
	if (mon < 10) out += "0";
	out += mon + "-";
	if (day < 10) out += "0";
	out += day;
	return out;

}//end DateConvert2MySQL()


/**
* Sets data for Calendar. Could use first param as MySQL format string
*
* @param	year	given year OR MySQL-format complete date
* @param	mon		given month
* @param	day		given day
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
* Add func to body onload event
*
* @param	func_name	func call string [init()]
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


/**
* Perform a frame-sensitive redirect by respecting any <base> tags in the document
*
* @access public
* @return void
*/
function frameRedirect(url)
{
	basetags = document.getElementsByTagName('base');
	if (basetags.length != 0) {
		switch (basetags[0].target) {
			case '_parent':
				window.parent.location = url;
			  break;
			case '_top':
				window.top.location = url;
				break;
			case '_self':
				document.location.href = url;
				break;
			case '_blank':
				window.open(url, 'new', '');
				break;
			default:
				window.top.frames[basetags[0].target].location = url;
		}
	} else {
		document.location.href = url;
	}

}//end frameRedirect()

function addStyle(css)
{
	var myStyle = document.createElement('STYLE');
	document.getElementsByTagName('HEAD')[0].appendChild(myStyle);
	if (null == myStyle.canHaveChildren || myStyle.canHaveChildren) {
		var styleContent = document.createTextNode(css);
		myStyle.appendChild(styleContent);
	} else {
		myStyle.styleSheet.cssText = css;
	}
}


/**
* Returns the node's outer html
*
* @para node
*
* @return void
* @access public
*/
function outerHTML(node)
{
    // If browser doesn't has internal method to get outer HTML
    return node.outerHTML || (
        function(n){
            var div = document.createElement('div'), h;
            div.appendChild( n.cloneNode(true) );
            h = div.innerHTML;
            div = null;
            return h;
        })(node);
}