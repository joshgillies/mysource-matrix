/**
* TabContentArea
*
* This is the code for a single instance of a tab content area that is used by mcTabs
*
*/

// Create the Class
function mcTabContentAreaTreeClass()
{
	super();

	// NOTE: the depth order for these MC's is important because the menu must be higner
	//       than the scroll pane and list container so that it's items will display 
	//       over the top of them

	// Now attach the menu
	this.attachMovie("mcMenuContainerID", "menu_container", 3);
	this.menu_container._x = 0;
	this.menu_container._y = 0;

	this.attachMovie("FScrollPaneSymbol", "scroll_pane", 1);
	this.scroll_pane.setHScroll(true);
	this.scroll_pane.setVScroll(true);
	this.scroll_pane._x = 0;
	this.scroll_pane._y = this.menu_container._height;

	// Now the list container
	this.attachMovie("mcListContainerID", "list_container", 2);

	// Attach the container on to the scroll pane
	this.scroll_pane.setScrollContent(this.list_container);

	// Because the scroll pane inherits from some other place 
	// we need to manually set it up for nesting
	makeNestedMouseMovieClip(this.scroll_pane, true, NestedMouseMovieClip.NM_ON_PRESS);
	makeNestedMouseMovieClip(this.scroll_pane.hScrollBar_mc, true, NestedMouseMovieClip.NM_ON_PRESS);
	makeNestedMouseMovieClip(this.scroll_pane.vScrollBar_mc, true, NestedMouseMovieClip.NM_ON_PRESS);

}// end constructor()

// Make it inherit from Tab Content Area
mcTabContentAreaTreeClass.prototype = new mcTabContentAreaClass();

/**
* Set the size of the tabs
*
* @param int	w	the width of the tabs
* @param int	h	the height of the tabs
*
*/
mcTabContentAreaTreeClass.prototype.setSize = function(w, h)
{
	super.setSize(w, h);

	this.menu_container.setWidth(w);
	this.scroll_pane.setSize(w, h - this.menu_container._height);
	this.list_container.refresh();

}// setSize()

/**
* Fired when we are pressed and then release
*
* @access public
*/
mcTabContentAreaTreeClass.prototype.onRelease = function() 
{
	return super.onRelease(); // Fucking flash see SUPER_METHOD_EG.as
}// end


Object.registerClass("mcTabContentAreaTreeID", mcTabContentAreaTreeClass);
