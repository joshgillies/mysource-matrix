#include "mcActionsBarClass.as"
#include "mcMoveIndicatorClass.as"
#include "mcListItemContainerClass.as"

// Create the Class
function mcListContainerClass()
{
	// some current settings/flags
	this.move_indicator_on = false;
	this.action      = "";

	// create an empty clip that fills out this container to be at least
	// the size of the scroller, so that the onPress event can fired 
	// from anywhere in the scroller
	this.createEmptyMovieClip("filler", 1);
	this.filler._x = 0;
	this.filler._y = 0;
	this.filler._visible = true;

	// Create the container to hold all the list items
	this.attachMovie("mcListItemContainerID", "list", 2);
	this.list._x = 0;
	this.list._y = 0;

	// Create the Plus Minus Button
	this.attachMovie("mcMoveIndicatorID", "move_indicator", 3);
	this.move_indicator._x = 10;
	this.move_indicator._y = 10;
	this.move_indicator._visible = false;

	// Create the Action Bar (pop's up with the edit screens)
	this.attachMovie("mcActionsBarID", "actions_bar", 4);
	this.actions_bar._x = 10;
	this.actions_bar._y = 10;

	// a temp object that can hold any run-time data
	this.tmp = new Object();

	// Attach the container on to the "scroller"
	_root.scroller.setScrollContent(this);

	// Set ourselves up as a listener on the menu, so we know when an item is pressed
	_root.menu_container.addListener(this);

}// end constructor

// Make it inherit from Nested Mouse Movements MovieClip
mcListContainerClass.prototype = new NestedMouseMovieClip(true, NestedMouseMovieClip.NM_ON_MOUSE | NestedMouseMovieClip.NM_ON_PRESS);

/**
* Refreshes this container's display completely
*
* @access public
*/
mcListContainerClass.prototype.refresh = function() 
{
	// refresh the list 
	this.list.refreshDisplay();
	// now ourselves
	this.refreshDisplay();

}// end refreshDisplay()


/**
* Refreshes the display size without refreshing the list
*
* @access public
*/
mcListContainerClass.prototype.refreshDisplay = function() 
{

	// Now make sure that the filler is big enough for all the content
	var xpos = Math.max(_root.scroller.getPaneWidth(),  this.list._width);
	var ypos = Math.max(_root.scroller.getPaneHeight(), this.list._height + _root.LIST_ITEM_END_BRANCH_GAP);

	this.filler.beginFill(0xFF0000, 0); // alpha = 0 -> transparent
	// This is commented out because when we try and explicitly set it, 
	// an extra 2 pixels gets added to the width of the MC for no f!@#$ing reason
	//this.filler.lineStyle();
	this.filler.moveTo(0, 0);
	this.filler.lineTo(xpos, 0);
	this.filler.lineTo(xpos, ypos);
	this.filler.lineTo(0, ypos);
	this.filler.lineTo(0, 0);
	this.filler.endFill();

	// refresh the scroller
	_root.scroller.refreshPane();

}// end refreshDisplay()


mcListContainerClass.prototype.onPress = function() 
{
	// only proceed if there is no modal status, or we or one of ours kids owns it
	var modal = _root.system_events.checkModal(this);
	if (modal & SystemEvents.OTHER_MODAL) return true;

	// if one of our kids has modal status, only execute onPress for them
	if (modal & SystemEvents.KID_MODAL) {
		var kid = _root.system_events.getModalChildName(this);
		this[kid].onPress();
	} else {
		return super.onPress();
	}

}// end onPress()

/**
* Show the actions bar for the currently selected list item
*/
mcListContainerClass.prototype.showActionsBar = function() 
{
	// OK, what we are doing here is being a bit tricky.
	// what we are going to do is tell the item that is currently selected
	// that the mouse has been released, then open the actions bar under the 
	// mouse cursor and then trick it into thinking that the mouse has just been 
	// pressed
	this.onRelease();
	this.actions_bar.show(this.list.selected_item.assetid, this._xmouse - 5, this._ymouse - 5);
	this.onPress();

}// end showActionsBar()

mcListContainerClass.prototype.onRelease = function() 
{
	_root.system_events.screenPress(this);

	// only proceed if there is no modal status, or we or one of ours kids owns it
	var modal = _root.system_events.checkModal(this);
	if (modal & SystemEvents.OTHER_MODAL) return true;

	// if one of our kids has modal status, only execute event for them
	if (modal & SystemEvents.KID_MODAL) {
		var kid = _root.system_events.getModalChildName(this);
		this[kid].onRelease();
	} else {
		return super.onRelease();
	}

}// end onRelease();

/**
* When an item is pressed in the menu, we need to check if we should take care of 
* the action or not
*
* @param string action	the action to perform
* @param string info	additional information needed for action
*
*/
mcListContainerClass.prototype.onMenuItemPress = function(action, info) 
{

	switch(action) {
		case "add" :
			trace("Add " + info);
			if (this.move_indicator.startIndicator(this, "processAddAsset")) {
				this.action = "add_asset";
				this.tmp.exec_action = {type_code: info};
			} else {
				_root.dialog_box.show("Unable to aquire the Move Indicator", "");
			}

		break;

	}// end switch

}// end onMenuItemPress()


/**
* Attempt to add the asset
*
* @param string	parent_item_name	the name of the item that the pos is to be under
* @param int	parent_assetid		the	assetid of the item that the pos is to be under
* @param int	relative_pos		the relative in the parent_asset's links array that the asset is to be placed
*
*/
mcListContainerClass.prototype.processAddAsset = function(parent_item_name, parent_assetid, relative_pos) 
{
	trace("Add an asset of type '" + this.tmp.exec_action.type_code + "' to pos " + parent_assetid + ". Where ? " + relative_pos);

	var xml = new XML();
	var cmd_elem = xml.createElement("command");
	cmd_elem.attributes.action  = "get url";
	cmd_elem.attributes.cmd     = "add";
	cmd_elem.attributes.type_code      = this.tmp.exec_action.type_code;
	cmd_elem.attributes.parent_assetid = parent_assetid;
	cmd_elem.attributes.pos            = pos;
	xml.appendChild(cmd_elem);

	trace(xml);

	// start the loading process
	var exec_identifier = _root.server_exec.init_exec(xml, this, "xmlGotoUrl", "url");
	_root.server_exec.exec(exec_identifier, "Sending Request");

	this.action = "";

}// end processAddAsset()



/**
* call back fn used by the execAction*() fns to after they sedn their urls requests to the server
*
* @param object xml	the xml object returned
*
*/
mcListContainerClass.prototype.xmlGotoUrl = function(xml, exec_identifier)
{
	var frame = xml.firstChild.attributes.frame;
	var link  = xml.firstChild.firstChild.nodeValue;

	trace("getURL(" + link + ", " + frame + ");");
	getURL(link, frame);

}// end xmlGotoUrl()


Object.registerClass("mcListContainerID", mcListContainerClass);
