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
* $Id: popup.js,v 1.6 2012/08/30 00:57:28 ewang Exp $
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