/**
* TabContentArea
*
* This is the code for a single instance of a tab content area that is used by mcTabs
*
*/

// Create the Class
function mcTabContentAreaLogMsgsClass()
{
	super();

	this.attachMovie("FScrollPaneSymbol", "scroll_pane", 1);
	this.scroll_pane.setHScroll(false);
	this.scroll_pane.setVScroll(true);
	this.scroll_pane._x = 0;
	this.scroll_pane._y = 0;

	// Now the msgs container
	trace("HERE");
	this.attachMovie("mcMsgsBarID", "msgs_container", 2);

	// Attach the container on to the scroll pane
	this.scroll_pane.setScrollContent(this.msgs_container);

}// end constructor()

// Make it inherit from Tab Content Area
mcTabContentAreaLogMsgsClass.prototype = new mcTabContentAreaClass();

/**
* Set the size of the tabs
*
* @param int	w	the width of the tabs
* @param int	h	the height of the tabs
*
*/
mcTabContentAreaLogMsgsClass.prototype.setSize = function(w, h)
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
mcTabContentAreaLogMsgsClass.prototype.onRelease = function() 
{
	return super.onRelease(); // Fucking flash see SUPER_METHOD_EG.as
}// end


Object.registerClass("mcTabContentAreaLogMsgsID", mcTabContentAreaLogMsgsClass);
