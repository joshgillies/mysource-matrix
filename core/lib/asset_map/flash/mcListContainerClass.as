
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

	// Create the Action Bar (pop's up with the edit screens)
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
	// used by onPress() and onRelease()
	this.actions_bar_interval = null;

	// a temp object that can hold any run-time data
	this.tmp = new Object();

	// Attach the container on to the "scroller"
	_root.scroller.setScrollContent(this);

	// Set ourselves up as a listener on the menu, so we know when an item is pressed
	this.list.addListener(this);

	// Set ourselves up as a listener on the menu, so we know when an item is pressed
	_root.menu_container.addListener(this);

	// Set ourselves up as a broadcaster
    ASBroadcaster.initialize(this);

}// end constructor

// Make it inherit from Nested Mouse Movements MovieClip
mcListContainerClass.prototype = new NestedMouseMovieClip(true, NestedMouseMovieClip.NM_ON_PRESS | NestedMouseMovieClip.NM_ON_RELEASE);

/**
* Refreshes the display of the items, from a certain position onwards
*
* @param int start_i   the index in the items_order array to start the refresh from 
*
* @access public
*/
mcListContainerClass.prototype.refreshList = function(start_i) 
{
	// refresh the list 
	this.list.refresh();

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

}// end refreshList()


mcListContainerClass.prototype.onPress = function() 
{
	// check if something else is modal
	if (_root.system_events.inModal(this)) return true;

	if (this.move_indicator_on) return true;

	switch(this.action) {
		case 'move' : 
			// do nothing here, wait for release
			return true;
		break;

		// OK we ain't doing anything at the moment, let any of the kid MCs have this event
		default :
			return super.onPress();

	}// end switch

}// end onPress()

mcListContainerClass.prototype.showActionsBar = function() 
{
	clearInterval(this.actions_bar_interval);
	this.actions_bar_interval = null;
	this.actions_bar.show(this.selected_item.assetid, this._xmouse, this._ymouse);

}// end showActionsBar()

mcListContainerClass.prototype.onListItemSelection = function(assetid) 
{
	this.actions_bar_interval = setInterval(this, "showActionsBar", 500);
	trace('SHOW ACTIONS BAR INT : ' + this.actions_bar_interval);


	// if we in any action we don't want to select anything
	if (this.action != '') return false;

	if (this.selected_item == item) return true;

	if (this.selected_item != null) {
		this.unselectItem();
	}

	this.selected_item = item;
	this.selected_item.select();

	this.broadcastMessage("onListItemSelection", this.selected_item.assetid);

	return true;

}

mcListContainerClass.prototype.onListItemUnSelection = function() 
{
	this.selected_item.unselect();
	this.selected_item = null;
	this.broadcastMessage("onListItemUnSelection");
}

mcListContainerClass.prototype.onRelease = function() 
{
	// check if something else is modal
	if (_root.system_events.inModal(this)) return true;
	_root.system_events.screenPress(this);

	if (this.move_indicator_on) {
		this.endMoveIndicator();
		return true;
	}

	if (this.actions_bar_interval) {
		trace("CLEAR INTERVAL : " + this.actions_bar_interval);
		clearInterval(this.actions_bar_interval);
		this.actions_bar_interval = null;
	}
	if (this.actions_bar._visible) this.actions_bar.hide();



	switch(this.action) {

		default :
			// if there is nothing selected then there is nothing for us to do
			if (this.selected_item == null) return;

			switch(this.selected_item.getMouseButton()) {
				case 'kids' : 
					switch(this.selected_item.getKidState()) {
						case "plus" :
							//	Expand Branch
							this.showKids(this.selected_item.assetid, this.selected_item._name);
						break;
						case "minus" :
							//	Collapse Branch
							this.hideKids(this.selected_item.assetid, this.selected_item._name);
						break;
					}
				break;

				case 'move' :
					this.itemStartMove();
				break;
				
			}// end switch

	}// end switch

	return true;

}// end onRelease();

mcListContainerClass.prototype.selected = function(item) 
{
	return (this.selected_item == item);
}


mcListContainerClass.prototype.itemStartMove = function() 
{
	if (this.action != '') return false;

	if (this.startMoveIndicator('itemEndMove')) {
//		trace("Start Item Move");
		this.action = 'move';
		// move to top of layers
		this.selected_item.setMoveState("on");
	}

}// end itemStartMove()

mcListContainerClass.prototype.itemEndMove = function(pos, where) 
{
	if (this.action != 'move') return;

//	trace("End Item Move");

	var do_move = false;

	// if we actually moved
	if (pos != this.selected_item.pos) {
		// if the pos is the item after the selected 
		// then only move if the indicator is after that item
		if (pos == this.selected_item.pos + 1) {
			do_move = (where == "after" || where == "child");
		} else {
			do_move = true;
		}
	}// end if

	if (do_move) {

		var moved_to_item = this[this.items_order[pos].name];

		// if the indicator was before the position
		if (where == "before") {
			// AND if we are staying under the same parent 
			if (this.selected_item.getParentAssetid() == moved_to_item.getParentAssetid()) {
				var parent_asset = _root.asset_manager.assets[this.selected_item.getParentAssetid()];
				// AND if we are moving the asset down the list
				if (parent_asset.linkPos(this.selected_item.linkid) < parent_asset.linkPos(moved_to_item.linkid)) {
					// then we need to decrement the pos - see me for a demo of why (BCR)
					pos -= 1;
				}
			}
		}

		var params = new Object();
		params.moved_info           = this.list.posToAssetInfo(pos, where);
		params.moved_to_parent_name = moved_to_item.parent_item_name;
		
		trace("Move to Pos : " + pos + ", where : " + where);
		trace("Move Info   : " + params.moved_info.parent_assetid + ", pos : " + params.moved_info.pos);


		// if the parent has changed during the move then we need to ask what we want done
		if (this.selected_item.getParentAssetid() != moved_to_item.getParentAssetid()) {
			_root.options_box.init("Move Type", "Are you moving this asset, or creating a new link ?", this, "itemMoveConfirm", params);
			_root.options_box.addOption("new link",   "New Link");
			_root.options_box.addOption("move asset", "Moving");
			_root.options_box.show();

		// otherwise just move it
		} else {
			this.itemMoveConfirm("move asset", params);

		}// end if

	} else {
		// same as hitting cancel
		this.itemMoveConfirm(null, {});
	}// end if

}// end itemEndMove()


mcListContainerClass.prototype.itemMoveConfirm = function(move_type, params) 
{
	if (this.action != 'move') return;

	// if they didn't hit cancel
	if (move_type != null) { 
		trace("End Item Move : move_type -> " + move_type);

		var xml = new XML();
		var cmd_elem = xml.createElement("command");
		cmd_elem.attributes.action              = move_type;
		cmd_elem.attributes.from_parent_assetid = this.selected_item.getParentAssetid();
		cmd_elem.attributes.linkid              = this.selected_item.linkid;
		cmd_elem.attributes.to_parent_assetid   = params.moved_info.parent_assetid;
		cmd_elem.attributes.to_parent_pos       = params.moved_info.pos;
		xml.appendChild(cmd_elem);

		trace(xml);

		// start the loading process
		var exec_identifier = _root.server_exec.init_exec(xml, this, "xmlMoveItem", "success");
		if (this.tmp.move_asset == undefined) this.tmp.move_asset = new Object();
		this.tmp.move_asset[exec_identifier] = params;
		_root.server_exec.exec(exec_identifier, ((move_type == "move asset") ? "Moving Asset" : "Creating new link"));

	}

	this.selected_item.setMoveState("off");
	this.action = '';

}// end itemMoveConfirm()


mcListContainerClass.prototype.xmlMoveItem = function(xml, exec_identifier)
{

	var select_linkid = xml.firstChild.attributes.linkid;

	// OK let's save the pos of that we were dragged to so that we can select the correct
	// list item later after the parents have been reloaded
	this.tmp.move_asset.new_select_name = this.tmp.move_asset[exec_identifier].moved_to_parent_name + "_" + select_linkid;

	trace("#### New Select Name : " + this.tmp.move_asset.new_select_name);

	// get the asset manager to reload the assets
	_root.asset_manager.reloadAssets([this.selected_item.getParentAssetid(), this.tmp.move_asset[exec_identifier].moved_info.parent_assetid]);

}// end xmlMoveItem()


mcListContainerClass.prototype.startMoveIndicator = function(on_end_fn) 
{
	if (this.move_indicator_on) return false;
	// attempt to get the modal status
	if (!_root.system_events.startModal(this)) return false;

	this.move_indicator_on = true;


	this.tmp.move_indicator = {on_end_fn: on_end_fn, pos: -1, where: ""};
	// move to top of layers
	this.move_indicator.swapDepths(this.num_items + 1);

	return true;

}// end startMoveIndicator()

mcListContainerClass.prototype.endMoveIndicator = function() 
{
	if (!this.move_indicator_on) return;
	_root.system_events.stopModal(this);

	// call back to the end fn for whoever called us
	this[this.tmp.move_indicator.on_end_fn](this.tmp.move_indicator.pos, this.tmp.move_indicator.where);

	// clear the move indicator
	this.move_indicator._visible = false;
	this.move_indicator.swapDepths(-1);

	// Re-Show the menus
	_root.menu_container.show();

	this.move_indicator_on = false;

}// end endMoveIndicator()


mcListContainerClass.prototype.onMouseMove = function() 
{
	if (this.move_indicator_on) {

		var xm = this._xmouse;
		var ym = this._ymouse;

		var pos = this.list.getItemPos(xm, ym, true);
		// make sure the number is valid
		if (pos.pos > this.items_order.length) pos.pos = this.items_order.length;
		else if (pos.pos < 0) pos.pos = 0;

		// if we are past the end of the list then we are really at the last pos, in the gap
		if (pos.pos == this.items_order.length) {
			pos.pos    = this.items_order.length - 1;
			pos.in_gap = true;
		}

		// if we are past the end of the list, improvise
		var item_name = this.items_order[pos.pos].name;

		this.tmp.move_indicator.pos    = pos.pos;

		if (pos.in_gap) {
			this.move_indicator.gotoAndStop("normal");
			this.move_indicator._x  = this[item_name]._x;
			this.move_indicator._y  = this[item_name]._y + _root.LIST_ITEM_POS_INCREMENT;
			this.tmp.move_indicator.where = "after";

		} else {

			var percentile = ((ym - this[item_name]._y) / _root.LIST_ITEM_POS_INCREMENT);

			if (percentile < 0.45) {
				this.move_indicator.gotoAndStop("normal");
				this.move_indicator._x  = this[item_name]._x;
				this.move_indicator._y  = this[item_name]._y;
				this.tmp.move_indicator.where = "before";

			} else {
				this.move_indicator.gotoAndStop("new_child");
				this.move_indicator._x  = this[item_name]._x + _root.LIST_ITEM_INDENT_SPACE;
				this.move_indicator._y  = this[item_name]._y + _root.LIST_ITEM_POS_INCREMENT;
				this.tmp.move_indicator.where = "child";

			}
		}

		this.move_indicator._visible = true;

	}// end if action

	return true;

}// end onMouseMove()


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
//			trace('Add ' + info);
			if (this.startMoveIndicator("processAddAsset")) {
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
* @param int		pos	the	position in the item_order array that the move indicator came to it's final rest
* @param boolean	in_gap	whether the indicator finished up in the gap at the end of the list
*
*/
mcListContainerClass.prototype.processAddAsset = function(pos, where) 
{

	trace("Add an asset of type '" + this.tmp.exec_action.type_code + "' to pos " + pos + ". Where ? " + where);
	var info = this.list.posToAssetInfo(pos, where);

	var xml = new XML();
	var cmd_elem = xml.createElement("command");
	cmd_elem.attributes.action  = "get url";
	cmd_elem.attributes.cmd     = "add";
	cmd_elem.attributes.type_code      = this.tmp.exec_action.type_code;
	cmd_elem.attributes.parent_assetid = info.parent_assetid;
	cmd_elem.attributes.pos            = info.pos;
	xml.appendChild(cmd_elem);

	trace(xml);

	// start the loading process
	var exec_identifier = _root.server_exec.init_exec(xml, this, "xmlGotoUrl", "url");
	_root.server_exec.exec(exec_identifier, "Sending Request");

	this.action = '';

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

	trace('getURL(' + link + ', ' + frame + ');');
	getURL(link, frame);

}// end xmlGotoUrl()


Object.registerClass("mcListContainerID", mcListContainerClass);
