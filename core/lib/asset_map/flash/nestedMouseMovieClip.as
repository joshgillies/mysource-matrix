/**
* NestedMouseMovieClip
*
* This class inherits from MovieClip, but allows any nested MCs to 
* be able to receive the any mouse related event
*
*/

/* Constants for deciding what events to provide the nested mouse movements for */
NestedMouseMovieClip.NM_ON_MOUSE			= 1; // onMouseUp, onMouseDown, onMouseMove
NestedMouseMovieClip.NM_ON_PRESS			= 2; // onPress, onRelease, onReleaseOutside, onDragOver, onDragOut
NestedMouseMovieClip.NM_ON_ROLL				= 4; // onRollOver, onRollOut

function NestedMouseMovieClip(never_overlap, event_types) 
{
	// If we can guarantee that there will be no overlapping nested MCs then we can save on 
	// processing time
	this._nm_never_overlap = never_overlap;

	if (!(event_types & NestedMouseMovieClip.NM_ON_MOUSE)) {
		this.onMouseDown	= undefined;
		this.onMouseUp		= undefined;
	}

	if (!(event_types & NestedMouseMovieClip.NM_ON_PRESS)) {
		this.onDragOut			= undefined;
		this.onDragOver			= undefined;
		this.onPress			= undefined;
		this.onRelease			= undefined;
		this.onReleaseOutside	= undefined;
	}
	if (!(event_types & NestedMouseMovieClip.NM_ON_ROLL)) {
		this.onRollOut			= undefined;
		this.onRollOver			= undefined;
	}

	if (event_types & (NestedMouseMovieClip.NM_ON_MOUSE)) {
		this._nm_do_mouse_move = true;
	}

	this._nm_on_roll_active = false;
	this._nm_on_roll_mc     = null;
 
	this._nm_on_press_mc    = null;
	this._nm_drag_active    = false;
	this._nm_drag_in_mc     = false;

}

NestedMouseMovieClip.prototype = new MovieClip();

NestedMouseMovieClip.prototype._NM_findMc = function(x, y) 
{
	var pos = {x: x, y: y};
	this.localToGlobal(pos);

	// if it doesn't hit this box there ain't no point going any further
	if (!this.hitTest(pos.x, pos.y, true)) return null;

	var mcs = new Array();
	for(var i in this) {
		if (this[i] instanceof MovieClip && this[i].hitTest(pos.x, pos.y, true)) {
			mcs.push(i);
			if (this.never_overlap) break;
		}
	}
	if (mcs.length) {
		var highest = mcs[0];
		for(var i = 1; i < mcs.length; i++) {
			if (this[mcs[i]].getDepth() > this[highest].getDepth()) {
				highest = mcs[i];
			}
		}
		return highest;
	}
	return null;
}// end __findMc__

NestedMouseMovieClip.prototype._NM_execMCEvent = function(event) 
{
	var mc_name = this._NM_findMc(this._xmouse, this._ymouse);
	if (mc_name === null || this[mc_name][event] == undefined) {
		return false;
	} else {
		return this[mc_name][event]();
	}
}// end __execMCEvent__()

/**
* Because the onMouse* events get fired for all objects regardless of whether 
* the mouse is actually over the object or not just tell all the kids
*/
NestedMouseMovieClip.prototype._NM_propagateOnMouse = function(event) 
{
	for(var i in this) {
		if (this[i] instanceof MovieClip && this[i][event] != undefined) this[i][event]();
	}
}

NestedMouseMovieClip.prototype.onMouseUp = function() 
{
	this._NM_propagateOnMouse('onMouseUp');
	return true;
}
NestedMouseMovieClip.prototype.onMouseDown = function() 
{
	this._NM_propagateOnMouse('onMouseDown');
	return true;
}
NestedMouseMovieClip.prototype.onMouseMove = function() 
{
	if (this._nm_do_mouse_move) this._NM_propagateOnMouse('onMouseMove');

	if (this._nm_on_press_mc != null || this._nm_on_roll_active) {
		var mc_name = this._NM_findMc(this._xmouse, this._ymouse);

		if (this._nm_on_press_mc != null) {

			// if we aren't still over the mc we onPress()ed, tell it we onDragOut()ed
			if (this._nm_drag_in_mc && this._nm_on_press_mc != mc_name) {
				if (this[this._nm_on_press_mc].onDragOut != undefined) this[this._nm_on_press_mc].onDragOut();
				this._nm_drag_in_mc = false;

			// else if we have just dragged back onto the MC, inform it
			} else if (!this._nm_drag_in_mc && this._nm_on_press_mc == mc_name) {
				if (this[this._nm_on_press_mc].onDragOver != undefined) this[this._nm_on_press_mc].onDragOver();
				this._nm_drag_in_mc = true;

			}

		} else if (this._nm_on_roll_active && !this._nm_drag_active) {
			// if we have previously rolled over another mc, and we aren't still over it
			// then roll out of it
			if (this._nm_on_roll_mc != null && this._nm_on_roll_mc != mc_name) {
				this[this._nm_on_roll_mc].onRollOut();
				this._nm_on_roll_mc = null;
			}

			// if we have just rolled over an MC, inform it
			if (mc_name != null && this._nm_on_roll_mc == null) {
				this._nm_on_roll_mc = mc_name;
				this[this._nm_on_roll_mc].onRollOver();
			}

		}// end if

	}// end if

	return true;

}// end onMouseMove

/**
* OK what we have to do is emulate the firing of the onPress, onRelease, onReleaseOutside
* onDragOut, onDragOver, onRollOut and onRollOver events.
*
*/

NestedMouseMovieClip.prototype.onRollOut = function() 
{
	this._nm_on_roll_active = false;
}
NestedMouseMovieClip.prototype.onRollOver = function() 
{
	this._nm_on_roll_active = true;
}


NestedMouseMovieClip.prototype.onPress = function() 
{
	var mc_name = this._NM_findMc(this._xmouse, this._ymouse);
	if (mc_name === null) {
		this._nm_on_press_mc = null;
		this._nm_drag_in_mc  = false;
	} else {
		this._nm_on_press_mc  = mc_name;
		this._nm_drag_in_mc   = true;
		if (this[this._nm_on_press_mc].onPress != undefined) this[this._nm_on_press_mc].onPress();
	}
	this._nm_drag_active = true;

	return true;

}// end onPress()

NestedMouseMovieClip.prototype.onRelease = function() 
{
	// if we pressed down on an nested MC
	if (this._nm_on_press_mc != null) {
		var mc_name = this._NM_findMc(this._xmouse, this._ymouse);
		// if we are still over the mc that we onPress()ed on, call onRelease, otherwise we call onReleaseOutside
		var fn = (this._nm_on_press_mc == mc_name) ? "onRelease" : "onReleaseOutside";
		if (this[this._nm_on_press_mc][fn] != undefined) this[this._nm_on_press_mc][fn]();
	}
	this._nm_on_press_mc = null;
	this._nm_drag_active = false;
	this._nm_drag_in_mc  = false;

}// end onRelease()

NestedMouseMovieClip.prototype.onReleaseOutside = function() 
{
	// if we pressed down on an nested MC
	if (this._nm_on_press_mc != null) {
		if (this[this._nm_on_press_mc].onReleaseOutside != undefined) this[this._nm_on_press_mc].onReleaseOutside();
	}
	this._nm_on_press_mc = null;
	this._nm_drag_active = false;
	this._nm_drag_in_mc  = false;

}// end onReleaseOutside()

