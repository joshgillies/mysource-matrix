/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: thesaurus_browser.js,v 1.3 2005/04/29 05:39:49 gsherwood Exp $
*
*/

var Browser = null;

/**
* Constructor of the Thesaurus Browser object
*
* @param	divname		name of the target DIV or TD or other parent HTML object
* @param	framename	name of the frame
* @param	framepath	path to the frame content HTML file
* @param	imagespath	path to the images
* @param	varname		name of the object variable (for internal links)
* @param	helperframe	name of the helper frame (hidden)
* @param	helperpath	path to the script, which will return necessary
*						information (ie. helper.php)
* @param	inputname	id of the input field
* @param	prefix		prexif of all IDs
*
* @return object
* @access public
*/
ThesaurusBrowser = function (	divname,
								framename,
								framepath,
								imagespath,
								varname,
								helperframe,
								helperpath,
								inputname,
								prefix)
{
	this.panelTimer		= null;
	this.panelX			= 0;
	this.panelXtemp		= 0;
	this.panelY			= 0;
	this.lastentity		= "";
	this.varname		= varname;
	this.divname		= divname;
	this.framename		= framename;
	this.framepath		= framepath;
	this.helperframe	= helperframe;
	this.helperpath		= helperpath;
	this.imagespath		= imagespath;
	this.prefix			= prefix
	this.infoContainer	= prefix + "sq_InfoContainer";
	this.imgArrow		= prefix + "sq_imgArrow";
	this.info_title		= prefix + "info_title";
	this.info_description = prefix + "info_description";
	this.blanket		= prefix + "sq_blanket";
	this.inputname		= inputname;
	this.fade			= 0;
	this.fadeTimer		= null;
	this.searchable		= true;


/**
* opens browser window
*
* @param	e		event object
*
* @return void
* @access public
*/
	this.open = function(e)
		{
			var date = new Date();
			date = date.getTime();
			this.div.style.visibility = "hidden";
			this.div.style.position = "absolute";
			this.div.style.left = e.clientX + "px";
			var scrollX = "";
			var scrollY = "";
			if (navigator.userAgent.indexOf("Safari")==-1) {
				eval("scrollX = document.body.scrollLeft;");
				eval("scrollY = document.body.scrollTop;");
			}
			this.panelX = scrollX + e.clientX + 300 - 150;
			this.panelXtemp = this.panelX;
			this.panelY = scrollY + e.clientY + 20;
			this.div.style.left = scrollX + e.clientX + "px";
			this.div.style.top  = scrollY + e.clientY + "px";
			Browser = this;
			this.frame.src = this.framepath + "browser.html?time=" + date;

			this.div.style.MozOpacity = this.fade/100;
			this.div.style.opacity = this.fade/100;
			this.div.style.filter = 'alpha(opacity=' + this.fade + ')';

			this.div.style.display = "block";
			this.div.style.visibility = "visible";
			var x,y;
			var test1 = document.body.scrollHeight;
			var test2 = document.body.offsetHeight
			if (test1 > test2)
			{
				x = document.body.scrollWidth;
				y = document.body.scrollHeight;
			}
			else
			{
				x = document.body.offsetWidth;
				y = document.body.offsetHeight;
			}
			document.getElementById(this.blanket).style.display		= "block";
			document.getElementById(this.blanket).style.height		= y + 50 + "px";
			document.getElementById(this.blanket).style.width		= x + 50 + "px";
			document.getElementById(this.blanket).style.visibility	= "visible";
			document.body.style.overflow = "hidden";
			this.fade = 0;
			this.fadein();
			window.scrollTo(scrollX + e.clientX + 350, scrollY + e.clientY + 250);

		}//end open()


/**
* fade browser to visible mode
*
* @return void
* @access public
*/
	this.fadein = function()
		{
			clearTimeout(this.fadeTimer);
			this.div.style.visibility = "visible";
			if (this.fade >= 100) {
				this.fadeTimer = null;
				this.div.style.MozOpacity = "";
				this.div.style.opacity = "";
				this.div.style.filter= "";
				return;
			}
			this.div.style.MozOpacity = this.fade/100;
			this.div.style.opacity = this.fade/100;
			this.div.style.filter = 'alpha(opacity=' + this.fade + ')';
			this.fade += 7;
			this.fadeTimer = setTimeout(this.varname+".fadein()", 1);

		}//end fadein


/**
* fade browser window to invisible mode
*
* @return void
* @access public
*/
	this.fadeout = function()
		{
			clearTimeout(this.fadeTimer);
			this.div.style.visibility = "visible";
			if (this.fade <= 0) {
				this.fadeTimer = null;
				this.div.style.visibility = "hidden";
				document.getElementById(this.blanket).style.visibility = "hidden";
				this.div.style.MozOpacity = "";
				this.div.style.opacity = "";
				this.div.style.filter= "";
				return;
			}
			this.div.style.MozOpacity = this.fade/100;
			this.div.style.opacity = this.fade/100;
			this.div.style.filter = 'alpha(opacity=' + this.fade + ')';
			this.fade -= 7;
			this.fadeTimer = setTimeout(this.varname+".fadeout()", 1);

		}//end fadeout


/**
* closes browser window
*
* @return void
* @access public
*/
	this.close = function()
		{
			document.getElementById(this.imgArrow).src = this.imagespath + "arrowr.gif";
			document.getElementById(this.info_title).innerHTML = "";
			document.getElementById(this.info_description).innerHTML = "";
			this.div.style.visibility = "hidden";
			document.getElementById(this.infoContainer).style.visibility	= "hidden";
			document.getElementById(this.infoContainer).style.display		= "none";
			document.getElementById(this.blanket).style.visibility			= "hidden";
			document.body.style.overflow = "";
			document.getElementById(this.blanket).style.height				= 50 + "px";
			document.getElementById(this.blanket).style.width				= 50 + "px";
			this.div.style.display = "none";
			document.getElementById(this.blanket).style.display				= "none";

		}//close


/**
* opens info window
*
* @return void
* @access public
*/
	this.infoOpen = function()
		{
			clearTimeout(this.panelTimer);
			if (this.panelXtemp - this.panelX >= 150) {
				document.getElementById(this.imgArrow).src = this.imagespath + "arrowl.gif";
				return;
			}
			this.panelXtemp += 5;
			var panel = document.getElementById(this.infoContainer);
			panel.style.top			= this.panelY + "px";
			panel.style.left		= this.panelXtemp + "px";
			panel.style.display		= "block";
			panel.style.visibility	= "visible";
			this.panelTimer = setTimeout(this.varname + ".infoOpen()", 1);

		}//end infoOpen


/**
* closes info window
*
* @return void
* @access public
*/
	this.infoClose = function()
		{
			clearTimeout(this.panelTimer);
			if (this.panelXtemp <= this.panelX) {
				document.getElementById(this.imgArrow).src = this.imagespath + "arrowr.gif";
				document.getElementById(this.infoContainer).style.visibility	= "hidden";
				document.getElementById(this.infoContainer).style.display		= "none";
				return;
			}
			this.panelXtemp -= 5;
			var panel = document.getElementById(this.infoContainer);
			panel.style.top		= this.panelY + "px";
			panel.style.left	= this.panelXtemp + "px";
			this.panelTimer = setTimeout(this.varname + ".infoClose()", 1);

		}//end infoClose


/**
* toggle info window: open it if it is closed and close if it is opened
*
* @return void
* @access public
*/
	this.togglePanel = function()
		{
			if (this.panelXtemp > this.panelX) this.infoClose();
			else {
				this.infoOpen();
				this.loadEntityInfo(this.lastentity);
			}

		}//end togglePanel


/**
* loads list of linked entities by entity
*
* @param	entityid		name of the entity
*
* @return void
* @access public
*/
	this.loadListByEntity = function(entityid)
		{
			if (entityid == "") return;
			this.lastentity = entityid;
			document.getElementById(this.helperframe).src =
				this.helperpath + "&" +
				this.prefix + "_entity_name=" + entityid;

		}//end loadListByEntity


/**
* loads initial list of entities
*
* @param	entityid		entity name
*
* @return void
* @access public
*/
	this.loadInitialList = function(entityid)
		{
			if (!this.searchable) entityid = "";
			document.getElementById(this.helperframe).src =
				this.helperpath + "&" +
				this.prefix + "_entity_name=" + entityid + "&" +
				this.prefix + "_init=true";

		}//end loadInitialList


/**
* adds new list of entities to browser
*
* @param	list		entities list
*
* @return void
* @access public
*/
	this.setNewList = function(list)
		{
			if (list.length > 0) frames[this.framename].addList(list);
			this.loadEntityInfo(this.lastentity);

		}//end setNewList


/**
* loads info of the entity
*
* @param	entityid		entity name
*
* @return void
* @access public
*/
	this.loadEntityInfo = function(entityid)
		{
			if (entityid == "") return;
			if (this.panelXtemp > this.panelX) {
				document.getElementById(this.helperframe).src =
					this.helperpath + "&" +
					this.prefix + "_entity_name=" + entityid + "&" +
					this.prefix + "_info=true";
				this.lastentity = "";
			}

		}//end loadEntityInfo


/**
* shows entity info
*
* @param	list		entities list
*
* @return void
* @access public
*/
	this.entityInfo = function(list)
		{
			document.getElementById(this.info_title).innerHTML = list[0];
			document.getElementById(this.info_description).innerHTML = list[1];

		}//end entityInfo


/**
* returns selected entity to input box
*
* @param	str		entity name
*
* @return void
* @access public
*/
	this.receiveString = function(str)
		{
			document.getElementById(this.inputname).value = str;
			this.close();

		}//end recieveString


/**
* prints browser window
*
* @return void
* @access public
*/
	this.print = function()
		{
			var out = "";
			out += '<table cellpadding="0" cellspacing="0" border="0" style="display:none;visibility:hidden;position:absolute;z-index:10001" id="' + this.infoContainer + '">';
			out += '	<tr>';
			out += '		<td style="background:url(' + this.imagespath + 'lts.png);background: expression(\'none\');filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src=\'' + this.imagespath + 'lts.png\', sizingMethod=\'scale\')"><img alt="" src="' + this.imagespath + '1x1.gif" style="height:20px;width:20px;" /></td>';
			out += '		<td style="background:url(' + this.imagespath + 'ts.png);background: expression(\'none\');filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src=\'' + this.imagespath + 'ts.png\', sizingMethod=\'scale\')"></td>';
			out += '		<td style="background:url(' + this.imagespath + 'rts2.png);background: expression(\'none\');filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src=\'' + this.imagespath + 'rts2.png\', sizingMethod=\'scale\')"><img alt="" src="' + this.imagespath + '1x1.gif" style="height:20px;width:20px;" /></td>';
			out += '	</tr>';
			out += '	<tr>';
			out += '		<td style="background:url(' + this.imagespath + 'ls.png);background: expression(\'none\');filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src=\'' + this.imagespath + 'ls.png\', sizingMethod=\'scale\')"><img alt="" src="' + this.imagespath + '1x1.gif" style="width:20px;" /></td>';
			out += '		<td style="background: #FFFFFF"><div style="overflow:auto;padding-left: 20px;height:150px;width:150px;"><h3 id="' + this.info_title + '"></h3><p id="' + this.info_description + '"></p></div></td>';
			out += '		<td style="background:url(' + this.imagespath + 'rs.png);background: expression(\'none\');filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src=\'' + this.imagespath + 'rs.png\', sizingMethod=\'scale\')"><img alt="" src="' + this.imagespath + '1x1.gif" style="width:20px;" /></td>';
			out += '	</tr>';
			out += '	<tr>';
			out += '		<td style="background:url(' + this.imagespath + 'lbs.png);background: expression(\'none\');filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src=\'' + this.imagespath + 'lbs.png\', sizingMethod=\'scale\')"><img alt="" src="' + this.imagespath + '1x1.gif" style="height:20px;width:20px;" /></td>';
			out += '		<td style="background:url(' + this.imagespath + 'bs.png);background: expression(\'none\');filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src=\'' + this.imagespath + 'bs.png\', sizingMethod=\'scale\')"></td>';
			out += '		<td style="background:url(' + this.imagespath + 'rbs.png);background: expression(\'none\');filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src=\'' + this.imagespath + 'rbs.png\', sizingMethod=\'scale\')"><img alt="" src="' + this.imagespath + '1x1.gif" style="height:20px;width:20px;" /></td>';
			out += '	</tr>';
			out += '</table>';

			out += '<table cellpadding="0" cellspacing="0" border="0" style="display:none;visibility:hidden;position:absolute;;z-index:10002" id="' + this.divname + '">';
			out += '	<tr>';
			out += '		<td style="background:url(' + this.imagespath + 'lts.png);background: expression(\'none\');filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src=\'' + this.imagespath + 'lts.png\', sizingMethod=\'scale\')"><img alt="" src="' + this.imagespath + '1x1.gif" style="height:20px;width:20px;" /></td>';
			out += '		<td style="background:url(' + this.imagespath + 'ts.png);background: expression(\'none\');filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src=\'' + this.imagespath + 'ts.png\', sizingMethod=\'scale\')"></td>';
			out += '		<td onclick="' + this.varname + '.close();" style="background:url(' + this.imagespath + 'rts.png);background: expression(\'none\');filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src=\'' + this.imagespath + 'rts.png\', sizingMethod=\'scale\')"><img alt="" src="' + this.imagespath + '1x1.gif" style="height:20px;width:20px;" /></td>';
			out += '	</tr>';
			out += '	<tr>';
			out += '		<td style="background:url(' + this.imagespath + 'ls.png);background: expression(\'none\');filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src=\'' + this.imagespath + 'ls.png\', sizingMethod=\'scale\')"></td>';
			out += '		<td><iframe frameborder="0" id="' + this.framename + '" name="' + this.framename + '" marginheight="0" marginwidth="0" width="300" height="200"></iframe></td>';
			out += '		<td style="background:url(' + this.imagespath + 'rs.png);background: expression(\'none\');filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src=\'' + this.imagespath + 'rs.png\', sizingMethod=\'scale\');cursor:pointer;" onclick="' + this.varname + '.togglePanel()"><img id="' + this.imgArrow + '" alt="" src="' + this.imagespath + 'arrowr.gif" /></td>';
			out += '	</tr>';
			out += '	<tr>';
			out += '		<td style="background:url(' + this.imagespath + 'lbs.png);background: expression(\'none\');filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src=\'' + this.imagespath + 'lbs.png\', sizingMethod=\'scale\')"><img alt="" src="' + this.imagespath + '1x1.gif" style="height:20px;width:20px;" /></td>';
			out += '		<td style="background:url(' + this.imagespath + 'bs.png);background: expression(\'none\');filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src=\'' + this.imagespath + 'bs.png\', sizingMethod=\'scale\')"></td>';
			out += '		<td style="background:url(' + this.imagespath + 'rbs.png);background: expression(\'none\');filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src=\'' + this.imagespath + 'rbs.png\', sizingMethod=\'scale\')"><img alt="" src="' + this.imagespath + '1x1.gif" style="height:20px;width:20px;" /></td>';
			out += '	</tr>';
			out += '</table>';

			out += '<iframe id="' + this.helperframe + '" name="' + this.helperframe + '" style="visibility:hidden;position:absolute;top:-1000px; left:-1000px;width:10px;height:10px"></iframe>';
//			out += '<iframe id="' + this.helperframe + '" name="' + this.helperframe + '" style="visibility:visible;position:absolute;top:100px; left:100px;width:500px;height:300px"></iframe>';  // leave this line for debugging purposes
			out += '<iframe id="' + this.blanket + '" style="visibility:hidden;filter:alpha(opacity=50);-moz-opacity:0.5;opacity:0.5;position:absolute;top:0px; left:0px;width:100px;height:100px;z-index:10000;background:#FFFFFF" frameborder="0" marginheight="0" marginwidth="0" src="about:blank"></iframe>';
			document.write(out);

		}//end print

		this.print();
		this.div = document.getElementById(this.divname);
		this.frame = document.getElementById(this.framename);
}
