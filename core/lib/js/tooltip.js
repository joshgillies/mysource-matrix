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
* $Id: tooltip.js,v 1.24 2012/08/30 01:09:21 ewang Exp $
*
*/

/**
* Tooltip class. Constructor
*
* @return null
* @access public
*/
function ToolTip()
{
	this.normal_font	= "100% Tahoma,Arial";
	this.title_font		= "bold 120% Tahoma,Arial";
	this.title_align	= "left";
	this.normal_bg		= "#F4F4EB";
	this.title_bg		= "#594165";
	this.border			= "solid 1px #000000";
	this.normal_color	= "#000000";
	this.title_color	= "#FFFFFF";
	this.showing		= false;

	this.print	= tt_print;
	this.show	= tt_show;
	this.hide	= tt_hide;
	this.paint	= tt_paint;

	this.set	= tt_set;
	this.get	= tt_get;

}//end ToolTip()


/**
* Add tooltip block to the page, if it is not exists.
*
* @return void
* @access public
*/
function tt_print()
{
	if (!document.getElementById("ToolBox")) 
	{
		output = '<iframe class="sq-tooltip-popup" scrolling="no" border="0" frameborder="0" id="hider" style="position:absolute; top:-200px;left:-110px;width:10px; height:30px; z-index: 999" src="/__lib/web/images/icons/asset_locator.png"></iframe>';
		output += '<div id="ToolBox" class="sq-toolbox-wrapper"><table class="sq-toolbox-table"><tr><td id="ToolBoxTitle" class="sq-toolbox-title"></td></tr><tr><td class="sq-toolbox-content" id="ToolBoxContent"></td></tr></table></div>';
		if (document.body.insertAdjacentHTML) {
			document.body.insertAdjacentHTML('afterBegin', output);
		}
		else if (document.documentElement) {
            var div = document.createElement('div');
            div.innerHTML = output;
            document.body.insertBefore(div, document.body.firstChild);
		}
		else {
			document.body.innerHTML = output + document.body.innerHTML;
		}
	}

}//end tt_print()


/**
* Attach tooltip to corresponding object and make it visible.
*
* @param	obj	HTML object which position we would like to know
*
* @return void
* @access private
*/
function findPosX(obj)
{
		var curleft = 0;
		if (obj.offsetParent)
		{
			while (obj.offsetParent)
			{
				curleft += obj.offsetLeft
				obj = obj.offsetParent;
			}
		}
		else if (obj.x)
			curleft += obj.x;
		return curleft;

}//end findPosX()


/**
* Attach tooltip to corresponding object and make it visible.
*
* @param	obj	HTML object which position we would like to know
*
* @return void
* @access private
*/
function findPosY(obj)
{
		var curtop = 0;
		if (obj.offsetParent)
		{
			while (obj.offsetParent)
			{
				curtop += obj.offsetTop
				obj = obj.offsetParent;
			}
		}
		else if (obj.y)
			curtop += obj.y;
		return curtop;

}//end findPosY()


/**
* Attach tooltip to corresponding object and make it visible.
*
* @param	obj	HTML object which will be base for tooltip
* @param	text	content of the tooltip (could be HTML)
* @param	title	title of the tooltip [optional]
*
* @return
* @access
*/
function tt_show(obj, text, title, close_button)
{
	if (obj == null) return;

	this.print();

	var top = findPosY(obj);
	top += obj.offsetHeight;
	var left = findPosX(obj);
	this.paint(top, left, text, title, close_button);

	this.showing = true;

}//end tt_show()


/**
* Make tooltip invisible.
*
* @return void
* @access public
*/
function tt_hide()
{
	var tool_box = document.getElementById("ToolBox");

	if (tool_box.filters && tool_box.filters[0]) tool_box.filters[0].Apply();
	tool_box.style.visibility 	= "hidden";
	document.getElementById("hider").style.visibility = "hidden";
	if (tool_box.filters && tool_box.filters[0]) tool_box.filters[0].Play();

	this.showing = false;

}//end tt_hide()


/**
* Change all tooltip attributes and repaint tooltip.
*
* @param	top	top position of the tooltip
* @param	left	left position of the tooltip
* @param	text	content of the tooltip (could be HTML)
* @param	title	title of the tooltip [optional]
*
* @return
* @access
*/
function tt_paint(top, left, text, title, close_button)
{
	var tool_box = document.getElementById("ToolBox");

	if (tool_box.filters && tool_box.filters[0]) tool_box.filters[0].Apply();

	if ((typeof(title) != "undefined" && title != "") || (typeof(top) == "undefined"))
	{
		/*
		document.getElementById("ToolBoxTitle").style.textAlign = this.title_align;
		document.getElementById("ToolBoxTitle").style.font = this.title_font;
		document.getElementById("ToolBoxTitle").style.background = this.title_bg;
		document.getElementById("ToolBoxTitle").style.color = this.title_color;
		document.getElementById("ToolBoxTitle").style.padding = "2px";
		document.getElementById("ToolBoxContent").style.font = this.normal_font;
		*/

		if (typeof(title) != "undefined") document.getElementById("ToolBoxTitle").innerHTML = unescape(title);

		if (closeElement = document.getElementById("ToolBoxClose")) {
			closeElement.parentNode.removeChild(closeElement);
		}

		if (typeof close_button != 'undefined') {
			closeElement = document.createElement('th');
			/*
			closeElement.style.textAlign = 'right';
			closeElement.style.padding = '2px';
			closeElement.style.backgroundColor = this.title_bg;
			*/
			closeElement.id = 'ToolBoxClose';
			closeElement.className = 'sq-toolbox-close';

			//if (close_button == true) {
				close_button = '<img src="/__lib/web/images/icons/cancel.png" alt="Cancel" title="Cancel" class="sq-icon">';
			//}

			closeElement.innerHTML = '<a href="#" onclick="tooltip.hide(); return false;">' + close_button + '</a>';
			document.getElementById("ToolBoxTitle").parentNode.appendChild(closeElement);
			document.getElementById("ToolBoxContent").colSpan = 2;
		}
	}
	else
	{
		/*document.getElementById("ToolBoxTitle").style.background = this.normal_bg;
		document.getElementById("ToolBoxTitle").style.padding = "0px";*/
		document.getElementById("ToolBoxTitle").innerHTML = "";
	}
	if ((typeof(text) != "undefined" && text != "") || (typeof(top) == "undefined"))
	{
		if(typeof(text) != "undefined") 
		{
			var unescapedText = unescape(text);
			if(unescapedText.indexOf('<table') > -1 || unescapedText.indexOf('<p') > -1){
				//no wrapper element needed
			}else{
				unescapedText = '<div class="sq-toolbox-content-wrapper">' + unescapedText + '</div>';
			}
			document.getElementById("ToolBoxContent").innerHTML = unescapedText;
		}
		/*document.getElementById("ToolBoxContent").style.padding = "2px";
		document.getElementById("ToolBoxContent").style.background = this.normal_bg;*/
	}
	else
	{
		document.getElementById("ToolBoxContent").innerHTML = "";
		/*document.getElementById("ToolBoxContent").style.padding = "0px";
		document.getElementById("ToolBoxContent").style.background = this.title_bg;*/
	}

	if ((typeof(top) != "undefined") && (typeof(left) != "undefined"))
	{
		// Remove some amount to take into account the fact that innerWidth
		// includes the scrollbar width - 20 pixels should do
		var win_width = ((window.innerWidth)?window.innerWidth:document.body.offsetWidth) - 20;

		if ((tool_box.offsetWidth + left) > win_width) left -= (tool_box.offsetWidth + left) - win_width;

		tool_box.style.top 	= top + "px";
		tool_box.style.left	= left + "px";

		if (window.event)
		{
		  var hider = document.getElementById("hider");
		  hider.style.top = top;
		  hider.style.left = left;
		  hider.style.width = tool_box.offsetWidth;
		  hider.style.height = tool_box.offsetHeight;
		  hider.style.visibility = "visible";
		}
	}
	/*
	tool_box.style.font 		= this.normal_font;
	tool_box.style.color 		= this.color;
	tool_box.style.border 		= this.border;
	tool_box.style.background 	= this.normal_bg;
	*/
	tool_box.style.visibility 	= "visible";

	if (tool_box.filters && tool_box.filters[0]) tool_box.filters[0].Play();

}//end tt_paint()


/**
* Return value of the object's attribute.
*
* @param	attr	attribute name
*
* @return string
* @access public
*/
function tt_get(attr)
{
	if (typeof(eval("this." + attr)) == "undefined") return false;
	return eval("this." + attr);

}//end tt_get()


/**
* Set value of the object's attribute.
*
* @param	attr	attribute name
* @param	value	new value of the attribute
*
* @return
* @access
*/
function tt_set(attr, value)
{
	if (typeof(eval("this." + attr)) == "undefined") return false;
	eval("this." + attr + " = '" + value + "'");
	if (document.getElementById("ToolBox").style.visibility == "visible") this.paint();

}//end tt_set()


//default tooltip object for all pages
var tooltip = new ToolTip();
