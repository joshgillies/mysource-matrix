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
* $Id: popup.js,v 1.3 2005/05/16 06:40:15 tbarrett Exp $
*
*/

// Funky mouse-following popup code derived from the script from http://javascriptkit.com

var offsetfrommouse= { x:13, y:13 } //image x,y offsets from cursor position in pixels. Enter 0,0 for no offset
var popupId = null;

function stopTrailingPopup()
{
	document.onmousemove=null;
	if (null !== popupId) {
		document.getElementById(popupId).style.display = 'none';
	}

}//end stopTrailingPopup()

function startTrailingPopup(name)
{
	popupId = name;
	var movingElt = document.getElementById(popupId);
	movingElt.style.display = 'block';
	var scrollingParent = movingElt.parentNode;
	while ((scrollingParent.tagName != 'BODY') && (scrollingParent.style.overflow != 'auto') && (scrollingParent.parentNode)) {
		scrollingParent = scrollingParent.parentNode;
	}
	movingElt.parentNode.removeChild(movingElt);
	scrollingParent.appendChild(movingElt);
	document.onmousemove = followMouse;

}//end startTrailingPopup()

function truebody()
{
	return (!window.opera && document.compatMode && document.compatMode!="BackCompat")? document.documentElement : document.body

}//end truebody()

function followMouse(e)
{
	if (!e) var e = window.event;
	var movingElt = document.getElementById(popupId);
	var mousepos = null;
	if (e.pageX || e.pageY) {
		mousepos = { x:e.pageX, y:e.pageY };
	} else if (e.clientX || e.clientY) {
		mousepos = { x : e.clientX + truebody().scrollLeft, y : e.clientY + truebody().scrollTop };
	}
	movingElt.style.left = (offsetfrommouse.x + mousepos.x) + "px";
	movingElt.style.top = (offsetfrommouse.y + mousepos.y) + "px";

}//end followMouse()