 /////////////////////////////////////////////////////////////////////////////
// NOTE: the list items in this container are just stored as
// normal attributes and not in there own array (as I would have liked)
// this is due to the attachMovie() fn not being able to accept arrays
// elements as the instance name for the new movie


// Create the Class
function mcActionsBarClass()
{

	// Set ourselves up as a listener on the menu, so we know when an item is pressed
	_root.list_container.addListener(this);

}// end constructor

// Make it inherit from MovieClip
mcActionsBarClass.prototype = new MovieClip();

/**
* Event fired when list container selects a list item
*
* @param asset
*
*/
mcActionsBarClass.prototype.onListItemSelection = function(assetid)
{
	this.test_text = "Selected Item : " + _root.asset_manager.assets[assetid];
}

/**
* Event fired when list container un-selects a list item
*/
mcActionsBarClass.prototype.onListItemUnSelection = function()
{
	this.test_text = "";
}

Object.registerClass("mcActionsBarID", mcActionsBarClass);
