#include "mcMailMsgsClass.as"


/**
* TabContentAreaMailMsgs
*
* Looks after the displaying of the mail msgs
*
*/

// Create the Class
function mcTabContentAreaMailMsgsClass()
{
	super();

	this.attachMovie("FScrollPaneSymbol", "scroll_pane", 1);
	this.scroll_pane.setHScroll(false);
	this.scroll_pane.setVScroll(true);
	this.scroll_pane._x = 0;
	this.scroll_pane._y = 0;

	// Now the msgs container
	this.attachMovie("mcMailMsgsID", "msgs_container", 2);

	// Attach the container on to the scroll pane
	this.scroll_pane.setScrollContent(this.msgs_container);
	this.msgs_container._x = 0;
	this.msgs_container._y = 0;

	// Because the scroll pane inherits from some other place 
	// we need to manually set it up for nesting
	makeNestedMouseMovieClip(this.scroll_pane,               true, NestedMouseMovieClip.NM_ON_PRESS);
	makeNestedMouseMovieClip(this.scroll_pane.hScrollBar_mc, true, NestedMouseMovieClip.NM_ON_PRESS);
	makeNestedMouseMovieClip(this.scroll_pane.vScrollBar_mc, true, NestedMouseMovieClip.NM_ON_PRESS);

}// end constructor()

// Make it inherit from Tab Content Area
mcTabContentAreaMailMsgsClass.prototype = new mcTabContentAreaClass();

/**
* Set the size of the tabs
*
* @param int	w	the width of the tabs
* @param int	h	the height of the tabs
*
*/
mcTabContentAreaMailMsgsClass.prototype.setSize = function(w, h)
{
	super.setSize(w, h);
	this.scroll_pane.setSize(w, h);
	this.msgs_container.refresh();

}// setSize()

/**
* Fired when we are pressed and then release
*
* @access public
*/
mcTabContentAreaMailMsgsClass.prototype.onRelease = function() 
{
	return super.onRelease(); // Fucking flash see SUPER_METHOD_EG.as
}// end


Object.registerClass("mcTabContentAreaMailMsgsID", mcTabContentAreaMailMsgsClass);
