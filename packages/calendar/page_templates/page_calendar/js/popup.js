// Funky mouse-following popup code based on the script from http://javascriptkit.com

var offsetfrommouse=[12,4] //image x,y offsets from cursor position in pixels. Enter 0,0 for no offset
var popupObj = null;

function getTrailObj()
{
	if (popupObj == null) return null;
	if (document.getElementById)
	  return document.getElementById(popupObj).style
	else if (document.all)
	  exec('return document.all.'+popupObj+'.style');
}

function stopTrailingPopup()
{
	if ((typeof movingElt == 'undefined') || (movingElt == null)) {
		document.onmousemove=null;
		if ((o = getTrailObj()) != null) o.display='none';
	}
}

function startTrailingPopup(name)
{
	if ((typeof movingElt == 'undefined') || (movingElt == null)) {
		popupObj = name;
		document.onmousemove = followMouse;
	}
}

function truebody()
{
	return (!window.opera && document.compatMode && document.compatMode!="BackCompat")? document.documentElement : document.body
}

function followMouse(e)
{
	if (!(document.getElementById || document.all)) return false;
	var xcoord=offsetfrommouse[0];
	var ycoord=offsetfrommouse[1];
	if (typeof e != "undefined")
	{
		xcoord+=e.pageX;
		ycoord+=e.pageY;
	}
	else if (typeof window.event !="undefined")
	{
		xcoord+=truebody().scrollLeft+event.clientX;
		ycoord+=truebody().scrollTop+event.clientY;
	}
	var docwidth = document.all ? truebody().scrollLeft + truebody().clientWidth : pageXOffset + window.innerWidth-15;
	var docheight = document.all ? Math.max(truebody().scrollHeight, truebody().clientHeight) : Math.max(document.body.offsetHeight, window.innerHeight);
	trailObj = getTrailObj();
	if (((trailObj.width != 0) && ((xcoord + trailObj.width + 3) > docwidth)) || ((trailObj.height != 0) && ((ycoord + trailObj.height) > docheight))) {
		trailObj.display="none"
	} else {
		trailObj.display="block"
	}
	trailObj.left=xcoord+"px"
	trailObj.top=ycoord+"px"
}

