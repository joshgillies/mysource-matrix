/**
* TabContentArea
*
* This is the code for a single instance of a tab content area that is used by mcTabs
*
*/

// Create the Class
function mcTabContentAreaClass()
{
	this.stop();
	this._visible = false;
}

// Make it inherit from Nested Mouse Movements MovieClip
mcTabContentAreaClass.prototype = new NestedMouseMovieClip(false, NestedMouseMovieClip.NM_ON_PRESS);

/**
* Set the size of the tabs
*
* @param int	w	the width of the tabs
* @param int	h	the height of the tabs
*
*/
mcTabContentAreaClass.prototype.setSize = function(w, h)
{
	set_background_box(this, w, h, this._parent.colours.selected.bg, 100);
}// setSize()

/**
* Fired when we are pressed and then release
*
* @access public
*/
mcTabContentAreaClass.prototype.onRelease = function() 
{
	if (super.onRelease()) return true;
	_root.system_events.screenPress(this);
	return true;
}// end


Object.registerClass("mcTabContentAreaID", mcTabContentAreaClass);
