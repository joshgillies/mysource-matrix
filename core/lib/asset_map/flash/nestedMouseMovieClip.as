/**
* NestedMouseMovieClip
*
* This class inherits from MovieClip, but allows any nested MCs to 
* be able to receive the any mouse related event
*
* A boolean is returned from each of the events to indicate whether a kid MC dealt 
* with this event or not
*
*/

function NestedMouseMovieClip(never_overlaps, event_types) 
{
	// If we can guarantee that there will be no overlapping nested MCs then we can save on 
	// processing time
	this._nm_never_overlaps = (never_overlaps != true) ? false : true;

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

	this._nm_on_roll_active = false;
	this._nm_on_roll_mc     = null;
 
	this._nm_on_press_mc    = null;
	this._nm_drag_active    = false;
	this._nm_drag_in_mc     = false;

}

/* Constants for deciding what events to provide the nested mouse movements for */
NestedMouseMovieClip.NM_ON_PRESS			= 1; // onPress, onRelease, onReleaseOutside, onDragOver, onDragOut
NestedMouseMovieClip.NM_ON_ROLL				= 2; // onRollOver, onRollOut

NestedMouseMovieClip.prototype = new MovieClip();

NestedMouseMovieClip.prototype._NM_findMc = function(x, y) 
{
	var pos = {x: x, y: y};
	this.localToGlobal(pos);

	// if it doesn't hit this box there ain't no point going any further
	if (!this.hitTest(pos.x, pos.y, true)) return null;

	var mcs = new Array();
	for(var i in this) {
		if (this[i] instanceof MovieClip && this[i]._visible && this[i].hitTest(pos.x, pos.y, true)) {
			mcs.push(i);
			if (this.never_overlaps) break;
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

NestedMouseMovieClip.prototype.onMouseMove = function() 
{
	if (this._nm_on_press_mc != null || this._nm_on_roll_active) {
		var mc_name = this._NM_findMc(this._xmouse, this._ymouse);

		if (this._nm_on_press_mc != null) {

			// if we aren't still over the mc we onPress()ed, tell it we onDragOut()ed
			if (this._nm_drag_in_mc && this._nm_on_press_mc != mc_name) {
				if (this[this._nm_on_press_mc].onDragOut != undefined) this[this._nm_on_press_mc].onDragOut();
				this._nm_drag_in_mc = false;

			// else if we have just dragged back onto the MC, inform it
			// check objects because we can have 2 vars with diff names referencing same MC
			} else if (!this._nm_drag_in_mc && this[this._nm_on_press_mc] === this[mc_name]) {
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

	return ret_val;

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
//	trace("onPress : " + this);
	this._nm_drag_active = true;
	var mc_name = this._NM_findMc(this._xmouse, this._ymouse);
	if (mc_name === null) {
		this._nm_on_press_mc = null;
		this._nm_drag_in_mc  = false;
		return false; // none of the kids did anything with this
	} else {
		this._nm_on_press_mc  = mc_name;
		this._nm_drag_in_mc   = true;
//		trace(this[this._nm_on_press_mc] + ".onPress Defined : " + (this[this._nm_on_press_mc].onPress != undefined));
		return (this[this._nm_on_press_mc].onPress != undefined) ? this[this._nm_on_press_mc].onPress() : false;
	}

}// end onPress()

NestedMouseMovieClip.prototype.onRelease = function() 
{
	var ret_val = false;
//	trace("onRelease : " + this);
	// if we pressed down on an nested MC
	if (this._nm_on_press_mc != null) {
		var mc_name = this._NM_findMc(this._xmouse, this._ymouse);
		// if we are still over the mc that we onPress()ed on, call onRelease, otherwise we call onReleaseOutside
		// check by reference, just incase we have 2 var names referring to the same MC
		var fn = (this[this._nm_on_press_mc] === this[mc_name]) ? "onRelease" : "onReleaseOutside";
//		trace(this[this._nm_on_press_mc] + "." + fn + " Defined : " + (this[this._nm_on_press_mc].onPress != undefined));
		if (this[this._nm_on_press_mc][fn] != undefined) {
			ret_val = this[this._nm_on_press_mc][fn]();
		}
	}
	this._nm_on_press_mc = null;
	this._nm_drag_active = false;
	this._nm_drag_in_mc  = false;

	return ret_val;

}// end onRelease()

NestedMouseMovieClip.prototype.onReleaseOutside = function() 
{
	var ret_val = false;
	// if we pressed down on an nested MC
	if (this._nm_on_press_mc != null) {
		if (this[this._nm_on_press_mc].onReleaseOutside != undefined) {
			ret_val = this[this._nm_on_press_mc].onReleaseOutside();
		}
	}
	this._nm_on_press_mc = null;
	this._nm_drag_active = false;
	this._nm_drag_in_mc  = false;

	return ret_val;

}// end onReleaseOutside()

