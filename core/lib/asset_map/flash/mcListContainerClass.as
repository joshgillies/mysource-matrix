/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: mcListContainerClass.as,v 1.37 2003/10/13 01:37:37 dwong Exp $
* $Name: not supported by cvs2svn $
*/

#include "mcActionsBarClass.as"
#include "mcMoveIndicatorClass.as"
#include "mcListItemContainerClass.as"

// Create the Class
function mcListContainerClass()
{
	// some current settings/flags
	this.move_indicator_on = false;
	this.action      = "";

	// Create the container to hold all the list items
	this.attachMovie("mcListItemContainerID", "list", 1);
	this.list._x = 5;
	this.list._y = 5;

	// Create the Plus Minus Button
	this.attachMovie("mcMoveIndicatorID", "move_indicator", 2);
	this.move_indicator._x = 10;
	this.move_indicator._y = 10;
	this.move_indicator._visible = false;

	// Create the Action Bar (pop's up with the edit screens)
	this.attachMovie("mcActionsBarID", "actions_bar", 3);
	this.actions_bar._x = 10;
	this.actions_bar._y = 10;

	// a temp object that can hold any run-time data
	this.tmp = new Object();

	// Set ourselves up as a listener on the menu, so we know when an item is pressed
	this._parent.menu_container.addListener(this);

}// end constructor

// Make it inherit from Nested Mouse Movements MovieClip
mcListContainerClass.prototype = new NestedMouseMovieClip(true, NestedMouseMovieClip.NM_ON_PRESS);

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

	// now move indicator
	this.move_indicator.refresh();

}// end refresh()


/**
* Refreshes the display size without refreshing the list
*
* @access public
*/
mcListContainerClass.prototype.refreshDisplay = function() 
{
	// Now make sure that the filler is big enough for all the content
	var w = Math.max(this._parent.scroll_pane.getInnerPaneWidth(),  this.list._width);
	var h = Math.max(this._parent.scroll_pane.getInnerPaneHeight(), this.list._height + _root.LIST_ITEM_END_BRANCH_GAP);

	set_background_box(this, w - 2, h - 2, 0xFF0000, 0);

	// force a refresh of the scroller
	this._parent.scroll_pane.refreshPane();

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
	// are we in asset finder mode ?
	if (this._parent.finding_asset) {
		this.actions_bar.show(['use_me'], ['Use Me'], this._xmouse - 5, this._ymouse - 5);
	} else {

		var asset_type = _root.asset_manager.asset_types[this.list.selected_item.type_code];
		var actions = new Array();
		var labels  = new Array();

		for(var i = 0; i < asset_type.edit_screens.length; i++) {
			actions.push(asset_type.edit_screens[i].code_name);
			labels.push(asset_type.edit_screens[i].name);
		}// end for
		
		this.actions_bar.show(actions, labels, this._xmouse - 5, this._ymouse - 5);
	}
	this.onPress();

}// end showActionsBar()

/**
* Fired when the mouse button was pressed over us and when it's lifted and it's still over us
*
* @param string action		the action that was pressed in the action bar
*
* @access public
*/
mcListContainerClass.prototype.actionsBarPressed = function(action) 
{
//	trace("Actions Bar Pressed : " + action);

	// are we in asset finder mode ?
	if (this._parent.finding_asset) {
//		trace("Finding Asset");
		if (action == 'use_me') {
//			trace("finish asset finder");
			this._parent.finishAssetFinder(this.list.selected_item.assetid);
		}
	} else {
		var link = new String(_root.action_bar_path);
		link = link.replace("%assetid%", escape(this.list.selected_item.assetid))
		link = link.replace("%action%", escape(action));
		getURL(link, _root.url_frame);
	}// end if
}// end actionsBarPressed()

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
//			trace("Add " + info);
			if (this.move_indicator.startIndicator(this, "processAddAsset")) {
				this.action = "add_asset";
				this.tmp.exec_action = {type_code: info};
			} else {
				_root.dialog_box.show("Unable to acquire the Move Indicator", "");
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
//	trace("Add an asset of type '" + this.tmp.exec_action.type_code + "' to under parent : " + parent_assetid + " at pos " + relative_pos);

	var xml = new XML();
	var cmd_elem = xml.createElement("command");
	cmd_elem.attributes.action  = "get url";
	cmd_elem.attributes.cmd     = "add";
	cmd_elem.attributes.type_code      = this.tmp.exec_action.type_code;
	cmd_elem.attributes.parent_assetid = parent_assetid;
	cmd_elem.attributes.pos            = relative_pos;
	xml.appendChild(cmd_elem);

//	trace(xml);

	// start the loading process
	var exec_identifier = _root.server_exec.init_exec(xml, this, "xmlGotoUrl", "responses");
	_root.server_exec.exec(exec_identifier, "Sending Add Asset Request");

	this.action = "";

}// end processAddAsset()



/**
* call back fn used by the execAction*() fns to after they send their urls requests to the server
*
* @param object xml	the xml object returned
*
*/
mcListContainerClass.prototype.xmlGotoUrl = function(xml, exec_identifier)
{
	for (var i = 0; i < xml.childNodes.length; ++i) {
		nextChildResponse = xml.childNodes[i];

		switch (nextChildResponse.firstChild.nodeName) {
			case 'url':

				var frame = nextChildResponse.firstChild.attributes.frame;
				var link  = nextChildResponse.firstChild.firstChild.nodeValue;

				getURL(link, frame);

			break;


			default:
				_root.dialog_box.show("Connection Failure to Server", "Unexpected Response XML Node '" + nextChildResponse.firstChild.nodeName + "', expecting 'url'.\nPlease Try Again");
			
			break;
		}
	}

}// end xmlGotoUrl()

/**
* Starts the asset finder
*
* @param Array(string)	type_codes		the allowed type codes in the asset finder
*
* @access public
*/
mcListContainerClass.prototype.startAssetFinder = function(type_codes) 
{
//	trace("List Container : " + type_codes);
	this.list.restrictActiveTypes(type_codes);
}// end startAssetFinder()

/**
* Stops the asset finder
*
* @access public
*/
mcListContainerClass.prototype.stopAssetFinder = function(type_codes) 
{
	this.list.restrictActiveTypes([]);
}// end stopAssetFinder()

mcListContainerClass.prototype.getRightEdge = function() 
{
//	trace (this + "::mcListContainerClass.getRightEdge()");
//	trace (this.list);
//	trace (this.list.items_order);
	var max = null;
	for(var i = 0; i < this.list.items_order.length; i++) {
		var item = this.list[this.list.items_order[i].name];
//		trace ("item: " + item);
		var itemRight = item._x + item.text_field._x + item.text_field._width;
		if (max == null || max < itemRight)
			max = itemRight;
	}
//	trace (max);
	return max;
}

Object.registerClass("mcListContainerID", mcListContainerClass);
