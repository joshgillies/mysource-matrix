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

	// Now attach the menu
	this.attachMovie("mcMenuContainerID", "menu_container", 1);
	this.menu_container._x = 0;
	this.menu_container._y = 0;

	trace("MENU HEIGHT : " + this.menu_container._height);

	this.attachMovie("FScrollPaneSymbol", "scroll_pane", 2);
	this.scroll_pane.setHScroll(false);
	this.scroll_pane.setVScroll(true);
	this.scroll_pane._x = 0;
	this.scroll_pane._y = this.menu_container._height;

	// Now the list container
	this.attachMovie("mcListContainerID", "list_container", 3);

	// Attach the container on to the scroll pane
	this.scroll_pane.setScrollContent(this.list_container);

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
	trace("SET WIDTH : " + w);
	super.setSize(w, h);

	this.menu_container.setWidth(w);
	this.scroll_pane.setSize(w, h - this.menu_container._height);
	this.list_container.refresh();

}// setSize()


Object.registerClass("mcTabContentAreaTreeID", mcTabContentAreaTreeClass);
