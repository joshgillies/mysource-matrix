/**
* NestedMouseMovieClip
*
* This class inherits from MovieClip, but allows any nested MCs to 
* be able to receive the any mouse related event
*
*/

function NestedMouseMovieClip(never_overlap, event_types) 
{
	// If we can guarantee that there will be no overlapping nested MCs then we can save on 
	// processing time
	this.never_overlap = never_overlap;

	if (!(event_types & NestedMouseMovieClip.NM_ON_PRESS		)) this.onPress		= undefined;
	if (!(event_types & NestedMouseMovieClip.NM_ON_RELEASE		)) this.onRelease	= undefined;
	if (!(event_types & NestedMouseMovieClip.NM_ON_MOUSE_MOVE	)) this.onMouseMove	= undefined;
	if (!(event_types & NestedMouseMovieClip.NM_ON_MOUSE_UP		)) this.onMouseUp	= undefined;
	if (!(event_types & NestedMouseMovieClip.NM_ON_MOUSE_DOWN	)) this.onMouseDown	= undefined;
}

NestedMouseMovieClip.prototype = new MovieClip();
/* Constants for deciding what events to provide the nested mouse movements for */
NestedMouseMovieClip.NM_ON_PRESS        = 1;   
NestedMouseMovieClip.NM_ON_RELEASE      = 2;
NestedMouseMovieClip.NM_ON_MOUSE_MOVE   = 4;
NestedMouseMovieClip.NM_ON_MOUSE_UP     = 8;
NestedMouseMovieClip.NM_ON_MOUSE_DOWN   = 16;


NestedMouseMovieClip.prototype.__findMc__ = function (x, y) 
{
	var pos = {x: x, y: x};
	this.localToGlobal(pos);
	var mcs = new Array();
	for(var i in this) {
		if (this[i] instanceof MovieClip && this[i].hitTest(pos.x, pos.y, true)) {
			mcs.push(i);
			if (this.never_overlap) break;
		}
	}
	if (mcs.length) {
		var highest = mcs[0];
		for(var i = 0; i < mcs.length; i++) {
			if (this[mcs[i]].getDepth() > this[highest].getDepth()) {
				highest = mcs[i];
			}
		}
		trace('---->' + this[highest]);
		return highest;
	}
	return null;
}// end __findMc__

NestedMouseMovieClip.prototype.__execMCEvent__ = function (event) 
{
	var mc_name = this.__findMc__(this._xmouse, this._ymouse);
	if (mc_name === null) {
		return false;
	} else {
		return this[mc_name][event]();
	}
}// end __execMCEvent__()


NestedMouseMovieClip.prototype.onPress = function () 
{
	return this.__execMCEvent__('onPress');
}

NestedMouseMovieClip.prototype.onRelease = function () 
{
	return this.__execMCEvent__('onRelease');
}
NestedMouseMovieClip.prototype.onMouseMove = function () 
{
	return this.__execMCEvent__('onMouseMove');
}
NestedMouseMovieClip.prototype.onMouseUp = function () 
{
	return this.__execMCEvent__('onMouseUp');
}
NestedMouseMovieClip.prototype.onMouseDown = function () 
{
	return this.__execMCEvent__('onMouseDown');
}
