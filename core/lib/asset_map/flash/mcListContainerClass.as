 /////////////////////////////////////////////////////////////////////////////
// NOTE: the list items in this container are just stored as
// normal attributes and not in there own array (as I would have liked)
// this is due to the attachMovie() fn not being able to accept arrays
// elements as the instance name for the new movie


// Create the Class
function mcListContainerClass()
{
	// holds all the assets that have been referenced in this container
	this.assets      = new Object();
	// holds some information all the list items in the vertical order that they appear in the list
	// eg li_2_1_3 would be the mean this element exists here
	this.items_order = new Array();
	this.num_items = 0;

	// if an attr starts with this then it's a list item
	this.item_prefix = "li";

	// reference to the currently selected item
	this.selected_item = null;

	// some current settings/flags
	this.move_indicator_on = false;
	this.action      = '';

	// Create the Plus Minus Button
	this.attachMovie("mcMoveIndicatorID", "move_indicator", -1);
	this.move_indicator._y = 10;
	this.move_indicator._visible = false;
	this.move_indicator.onPress = mcListContainerClassonPress;

	// create an empty clip that fills out this container to be at least
	// the size of the scroller, so that the onPress event can fired 
	// from anywhere in the scroller
	this.createEmptyMovieClip("filler", -2);
	this.filler._x = 0;
	this.filler._y = 0;
	this.filler._visible = true;

	// a temp object that can hold any run-time data
	this.tmp = new Object();


	// Attach the container on to the "scroller"
	_root.scroller.setScrollContent(this);

	// Set ourselves up as a listener on the asset types, so we know when they have been loaded
	_root.asset_types.addListener(this);

	// Set ourselves up as a broadcaster
    ASBroadcaster.initialize(this);

}// end constructor

// Make it inherit from MovieClip
mcListContainerClass.prototype = new MovieClip();

/**
* Event fired when the Asset Types object has finished recieving all the asset types from the server
* We can then run init()
*/
mcListContainerClass.prototype.onAssetTypesLoaded = function()
{
	trace(' -- -- -- -- Create Asset List -- -- -- -- -- ');
//	this.init();
}

/**
* Loads the list container, by getting the root folder and showing it's kids
*
* @access public
*/
mcListContainerClass.prototype.init = function() 
{
	
	this.loadAssets([1], "initLoaded", {}, true);
	
}// end init()


/**
* continuation of init() after the root folder has been loaded
*
* @access private
*/
mcListContainerClass.prototype.initLoaded = function() 
{
	_root.list_container.showKids('1', 'li');
}// end initLoaded()


/**
* Loads the passed assetids asset values from the server, unless they exist already
*
* @param array(int)	assetids			array of assetids to load
* @param string		call_back_fn		the fn in this class to execute after the loading has finished
* @param Object		call_back_params	an object of params to send to the call back fn
* @param boolean	force_reload		reload from server, even if we already have references to the passed assets
*
*/
mcListContainerClass.prototype.loadAssets = function(assetids, call_back_fn, call_back_params, force_reload) 
{
	
	if (force_reload !== true) force_reload = false;

	trace('Load Assets : ' + assetids);

	var load_assetids = array_copy(assetids);

	if (!force_reload) {
	    for (var i = 0; i < load_assetids.length; i++) {
			if (this.assets[load_assetids[i]] != undefined) {
				load_assetids.splice(i, 1);
				i--;
			}
		}
	}

	trace('Load Assets : ' + load_assetids);
	trace('1. Asset #' + 3 + ' : ' + this.assets[3].kids);

	// if there are assets to load, get them from server
	if (load_assetids.length) {

		var xml = new XML();
		var cmd_elem = xml.createElement("command");
		cmd_elem.attributes.action = "get assets";
		xml.appendChild(cmd_elem);

	    for (var i = 0; i < load_assetids.length; i++) {
			var asset_elem = xml.createElement("asset");
			asset_elem.attributes.assetid = load_assetids[i];
			cmd_elem.appendChild(asset_elem);
		}

		trace(xml);

		// start the loading process
		var exec_indentifier = _root.server_exec.init_exec(xml, this, "loadAssetsFromXML", "assets");
		if (this.tmp.load_assets == undefined) this.tmp.load_assets = new Object();
		this.tmp.load_assets[exec_indentifier] = {fn: call_back_fn, params: call_back_params};
		_root.server_exec.exec(exec_indentifier, "Loading Assets");

	// else nothing to do, so just execute the call back fn
	} else {
		this[call_back_fn](call_back_params);

	}// end if
	
}// end loadAssets()


/**
* Called after the XML has been loaded 
*
* @param object XML xml   the xml object that contain the information that we need
*
*/
mcListContainerClass.prototype.loadAssetsFromXML = function(xml, exec_indentifier) 
{

	for (var i = 0; i < xml.firstChild.childNodes.length; i++) {
		// get a reference to the child node
		var asset_node = xml.firstChild.childNodes[i];
		if (asset_node.nodeName.toLowerCase() == "asset") {

			var kids = new Array();

			for (var j = 0; j < asset_node.childNodes.length; j++) {
				kids.push(asset_node.childNodes[j].attributes.assetid);
			}

			// only create if it doesn't already exist
			if (this.assets[asset_node.attributes.assetid] == undefined) {
				this.assets[asset_node.attributes.assetid] = new Asset();
			}
			this.assets[asset_node.attributes.assetid].assetid   = asset_node.attributes.assetid;
			this.assets[asset_node.attributes.assetid].type_code = asset_node.attributes.type_code;
			this.assets[asset_node.attributes.assetid].name      = asset_node.attributes.name;
			this.assets[asset_node.attributes.assetid].kids      = kids;

			trace(this.assets[asset_node.attributes.assetid]);

		}//end if
	}//end for

	this[this.tmp.load_assets[exec_indentifier].fn](this.tmp.load_assets[exec_indentifier].params);
	this.tmp.load_assets[exec_indentifier] = null;

}// end loadAssets()


/**
* Called by the mcListItem's on the pressing of the plus button
* Calls the root getAssetKids() which gets the XML data then 
* calls displayKids() below
*
* @param int		parent_assetid	the assetid of the asset who's kids to show
* @param string		parent_name		the list item name of the parent that the kids are attached to
* @param boolean	dont_refresh	prevent the refreshing of the list
*
*/
mcListContainerClass.prototype.showKids = function(parent_assetid, parent_name) 
{

	if (dont_refresh == undefined) dont_refresh = false;

	// we don't know anything about this or it's got know kids, bugger off
	if (this.assets[parent_assetid] == undefined || !this.assets[parent_assetid].kids.length) return;

	var params = {parent_assetid: parent_assetid, parent_name: parent_name, dont_refresh: dont_refresh};
	trace('2. Asset #' + 3 + ' : ' + this.assets[3].kids);
	this.loadAssets(this.assets[parent_assetid].kids, "showKidsLoaded", params);

}// end showKids()


/**
* A continuation of showKids() after the assets have been loaded
*/
mcListContainerClass.prototype.showKidsLoaded = function(params) 
{
	var parent_assetid = params.parent_assetid;
	var parent_name	   = params.parent_name;

	trace('Show Kids Loaded    : ' + parent_assetid + ', ' + parent_name);
	trace('Show Kids Kids list : ' + this.assets[parent_assetid].kids);

	// Now create the kids list items, if they don't already exist
	for(var j = 0; j < this.assets[parent_assetid].kids.length; j++) {
		var assetid = this.assets[parent_assetid].kids[j];
		var item_name = parent_name + "_" + assetid;

		if (this[item_name] == undefined) {
			trace(item_name + " is undefined");

			this.num_items++;
			var indent = (parent_assetid > 1) ? this[parent_name].indent + 1 : 0;

			this.attachMovie("mcListItemID", item_name, this.num_items);
			this[item_name]._visible = false;

			this[item_name].setInfo(parent_name, this.assets[assetid]);
			this[item_name].setIndent(indent);

			this.assets[assetid].item_names.push(item_name);

		}// end if

	}// end for

	// see if we can find this parent in the list
	var parent_i = (parent_assetid > 1) ? this[parent_name].pos : -1;

	this._recurseDisplayKids(parent_assetid, parent_name, parent_i);

	if (parent_assetid > 1) {
		// Because we only want to change the collapse sign for the parent asset
		// we do it here rather than in the recusion
		 this[parent_name].setKidState("minus");
	}

	if (!params.dont_refresh) {
		this.refreshList(parent_i);
	}

}// end showKidsLoaded()

/**
* Splices the items order array recursively, adding the kids of the passed parent
*
*/
mcListContainerClass.prototype._recurseDisplayKids = function(parent_assetid, parent_name, parent_i) 
{

	var i = parent_i + 1;

	// Now add the kids into the items order array in the correct spot
	for(var j = 0; j < this.assets[parent_assetid].kids.length; j++) {
		var name = parent_name + "_" + this.assets[parent_assetid].kids[j];
		if (this[name] == undefined) continue;
		this.items_order.splice(i, 0, {name: name, branch_count: 0, end_branch: (j == this.assets[parent_assetid].kids.length - 1)});
		if (this[name].expanded()) {
			i = this._recurseDisplayKids(this[name].assetid, name, i);
		} else {
			i++;
		}
	}

	return i;

}// end _recurseDisplayKids()

/**
* Called by the mcListItem's on the pressing of the minus button
* Hiding the children of the passed 
*
* @param int		parent_assetid	the assetid of the asset who's kids to hide
* @param string		parent_name		the list item name of the parent that the kids are attached to
* @param boolean	dont_refresh	prevent the refreshing of the list
*
*/
mcListContainerClass.prototype.hideKids = function(parent_assetid, parent_name, dont_refresh) 
{
	if (dont_refresh == undefined) dont_refresh = false;

	var parent_i = (parent_assetid > 1) ? this[parent_name].pos : -1;
	
	var num_to_remove = this._recurseHideKids(parent_assetid, parent_name);

	// Now remove the kids from the items order
	this.items_order.splice(parent_i + 1, num_to_remove);

	if (parent_assetid > 1) {
		// Because we only want to change the collapse sign for the parent asset
		// we do it here rather than in the recusion
		 this[parent_name].setKidState("plus");
	}

	if (!dont_refresh) {
		this.refreshList(parent_i);
	}

}// end hideKids()

mcListContainerClass.prototype._recurseHideKids = function(parent_assetid, parent_name) 
{

	var num_kids = this.assets[parent_assetid].kids.length;
	
	// loop through all the kids and make them invisible, also recursivly remove their kids
	for(var j = 0; j < this.assets[parent_assetid].kids.length; j++) {
		var name = parent_name + "_" + this.assets[parent_assetid].kids[j];
		if (this[name] == undefined) continue;
		if (this[name].expanded()) {
			num_kids += this._recurseHideKids(this[name].assetid, name);
		}
		this[name]._visible = false;
	}

	return num_kids;

}// end _recurseHideKids()

/**
* Forces the retrieval of the passed assets information, including who it's kids are
*
* @param int assetid
*
* @access public
*/
mcListContainerClass.prototype.reloadAsset = function(assetid) 
{
	if (this.assets[assetid] == undefined) return;

	var old_kids  = array_copy(this.assets[assetid].kids);
	var expanded_items = new Object();
	for(var i = 0; i < this.assets[assetid].item_names.length; i++) {
		// if they are open, then we need to 
		var name = this.assets[assetid].item_names[i];
		if (this[name].expanded()) {
			this.hideKids(assetid, name, true);
			expanded_items[name] = true;
		} else {
			expanded_items[name] = false;
		}
	}// end for

	this.loadAssets([assetid], 'reloadAssetLoaded', {assetid: assetid, old_kids: old_kids, expanded_items: expanded_items}, true);

}// end reloadAsset()

/**
* Continuation of reloadAsset() after asset is loaded from server
*
* @param object params
*
* @access public
*/
mcListContainerClass.prototype.reloadAssetLoaded = function(params) 
{
	if (this.assets[params.assetid] == undefined) return;

	var deletes = array_diff(params.old_kids, this.assets[params.assetid].kids);
	var inserts = array_diff(this.assets[params.assetid].kids, params.old_kids);
	trace("Deletes : " + deletes);

	for(var i = 0; i < this.assets[params.assetid].item_names.length; i++) {
		var name = this.assets[params.assetid].item_names[i];

		this[name].setInfo(parent_name, this.assets[params.assetid]);

		// delete any list items that aren't needed any more
		for(var j = 0; j < deletes.length; j++) {
			var item_name = name + "_" + deletes[j];
			trace("Delete " + item_name);
			this[item_name].removeMovieClip();
			array_remove_element(this.assets[this[item_name].assetid].item_names, item_name);
		}// end for

		// if they were expanded before, re-open them
		trace("expanded : " + name + " ? " + params.expanded_items[name]);
		if (params.expanded_items[name]) {
			this.showKids(params.assetid, name, true);
		}

	}// end for

	this.refreshList();

}// end reloadAssetLoaded()

/**
* Refreshes the display of the items, from a certain position onwards
*
* @param int start_i   the index in the items_order array to start the refresh from 
*
* @access public
*/
mcListContainerClass.prototype.refreshList = function(start_i) 
{

	if (start_i == undefined || start_i < 0) start_i = 0;

trace("Refresh List From : " + start_i);

	// now cycle through every item from the parent down and reset their positions
	var branch_count = (start_i >= 0) ? this.items_order[start_i].branch_count : 0;
	for(var i = start_i; i < this.items_order.length; i++) {
		// set for future use
		this.items_order[i].branch_count = branch_count;
//trace("SetPos : " + this.items_order[i].name + " : " + i);
		this[this.items_order[i].name].setPos(i);
		this[this.items_order[i].name]._visible = true;

		// if we have come across an end branch,
		// and if we aren't at the last item
		// and if this element has no kids, then we add the branch gap
		if (this.items_order[i].end_branch) {
			if (i < this.items_order.length - 1) {
				var this_indent = this[this.items_order[i].name].indent;
				var next_indent = this[this.items_order[i + 1].name].indent;
				if (this_indent != (next_indent - 1)) {
					branch_count++;
				}
			}
		}
	}// end for

	// Now make sure that the filler is big enough for all the content
	var xpos = Math.max(_root.scroller.getPaneWidth(),  this._width);
	var ypos = Math.max(_root.scroller.getPaneHeight(), this[this.items_order[this.items_order.length - 1].name]._y + _root.LIST_ITEM_POS_INCREMENT + _root.LIST_ITEM_END_BRANCH_GAP);

	this.filler.clear();
	this.filler.beginFill(0xFF0000, 0); // alpha = 0 -> transparent
	this.filler.lineStyle();
	this.filler.moveTo(0, 0);
	this.filler.lineTo(xpos, 0);
	this.filler.lineTo(xpos, ypos);
	this.filler.lineTo(0, ypos);
	this.filler.lineTo(0, 0);
	this.filler.endFill();

	// refresh the scroller
	_root.scroller.refreshPane();

}// end refreshList()

/**
* Returns the position of the item in the items_order array that is under the 
* x,y co-ordinates passed into the fn. These co-ords MUST be relative to
* this containers axis, not the Stage's
*
* Returns -1 if the y co-ord is before any clips 
* and the length of the items array if after all clips
*
* @param float x 
* @param float y
* @param boolean [bleed_gaps] default=false, if the co-ords are over a branch gap returns the pos above the gap
*
* @return int | Object
* @access public
*/
mcListContainerClass.prototype.getItemPos = function(x, y, bleed_gaps) 
{

	if (bleed_gaps == undefined) bleed_gaps = false;

	var pos    = -1;
	var in_gap = false;

	if (y > 0) {

		var last_pos = this[this.items_order[this.items_order.length - 1].name]._y + _root.LIST_ITEM_POS_INCREMENT + ((bleed_gaps) ? _root.LIST_ITEM_END_BRANCH_GAP : 0);

		// if we are past the last item in the list return the length of the items_order array
		if (y > last_pos) {
			pos = this.items_order.length;
		} else {

			// OK the biggest problem we have here is the bloody end branch gaps
			// so what we can do is find the maximum position number that these co-ords
			// would produce by ignoring the gaps
			var max_pos = Math.floor(y / _root.LIST_ITEM_POS_INCREMENT);

			// make sure the number is valid
			if (max_pos < 0) {
				return -1;
			} else if (max_pos >= this.items_order.length) {
				max_pos = this.items_order.length - 1;
			} 

			// Now we get a minimum position number, by using the branch count at the max pos
			var min_pos = max_pos - this.items_order[max_pos].branch_count;

			while(min_pos <= max_pos) {

				var i = min_pos + Math.round((max_pos - min_pos) / 2);

				var start_y = this[this.items_order[i].name]._y;
				var end_y   = this[this.items_order[i].name]._y  + _root.LIST_ITEM_POS_INCREMENT + ((bleed_gaps) ? _root.LIST_ITEM_END_BRANCH_GAP : 0);

				// if the mouse is before this element make the one above us the new max
				if (y < start_y) {
					max_pos = i - 1;

				// if the mouse is after this element make the one below us the new min
				} else if (y > end_y) {
					min_pos = i + 1;

				// else mouse is in this element 
				} else {
					pos    = i;
					if (bleed_gaps) {
						// we are in the gap if we are past the end of the item proper
						in_gap = (y > (this[this.items_order[i].name]._y  + _root.LIST_ITEM_POS_INCREMENT));
					}
					break;

				}// end if

			}//end while

		}// end if

	}// end if

	return (bleed_gaps) ? {pos: pos, in_gap: in_gap} : pos;

}// end getItemPos()


/**
* Returns the assetid and the position in it's kids array that a pos in the items_order array refers to
*
* @param int		pos 
* @param boolean	[in_gap] default=false, if pos is over a branch gap below the it's indicated pos
*
* @return Object
* @access public
*/
mcListContainerClass.prototype.posToAssetInfo = function(pos, in_gap) 
{

	trace('Items Order : ' + this.items_order[pos].name);
	trace('Parent  : ' + this[this.items_order[pos].name].parent_item_name);


	var pos_item = this[this.items_order[pos].name];

	var parent_assetid  = (pos_item.parent_item_name == this.item_prefix) ? 1 : this[pos_item.parent_item_name].assetid;
	var asset = this.assets[assetid];

	// Get the relative index from the kids array
	var relative_pos = 0;
	// if we are in the gap (and this pos is an end branch)
	// then our pos is the length of the kids array
	if (in_gap && this.items_order[pos].end_branch) {
		relative_pos = this.assets[parent_assetid].kids.length;

	} else {
		for(var j = 0; j < this.assets[parent_assetid].kids.length; j++) {
			if (this.assets[parent_assetid].kids[j] == pos_item.assetid) {
				relative_pos = j;
				break;
			}

		}// end for

	}// end if

	trace('Parent Assetid : ' + parent_assetid);
	trace('Relative Pos   : ' + relative_pos);

	return {parent_assetid: parent_assetid, pos: relative_pos};

}// end posToAssetInfo()


mcListContainerClass.prototype.onPress = function() 
{

	if (this.move_indicator_on) return;

	switch(this.action) {
		case 'move' : 
			// do nothing here, wait for release
		break;

		// OK we ain't doing anything at the moment let's check to see if we are over an
		// item and deal with any actions needed
		default :

			var pos = this.getItemPos(this._xmouse, this._ymouse);
			// if this is a proper index, select it
			if (pos >= 0 && pos < this.items_order.length) {
				this.selectItem(this[this.items_order[pos].name]);
			}

	}// end switch

}// end onPress()

mcListContainerClass.prototype.onRelease = function() 
{

	if (this.move_indicator_on) {
		this.endMoveIndicator();
		return;
	}

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


}// end onRelease();

mcListContainerClass.prototype.selectItem = function(item) 
{
	// if we in any action we don't want to select anything
	if (this.action != '') return false;

	if (this.selected_item == item) return true;

	if (this.selected_item != null) {
		this.itemRelease();
	}

	this.selected_item = item;
	this.selected_item.gotoAndStop("btn_down");

	return true;

}


mcListContainerClass.prototype.selected = function(item) 
{
	return (this.selected_item == item);
}


mcListContainerClass.prototype.itemRelease = function() 
{

	this.selected_item.gotoAndStop("btn_up");
	this.selected_item = null;
}


mcListContainerClass.prototype.itemStartMove = function() 
{
	if (this.action != '') return false;

	if (this.startMoveIndicator('itemEndMove')) {
		trace("Start Item Move");
		this.action = 'move';
		// move to top of layers
		this.selected_item.move_button.gotoAndStop("btn_down");
	}

}// end itemStartMove()

mcListContainerClass.prototype.itemEndMove = function(pos, in_gap) 
{
	if (this.action != 'move') return;

	trace("End Item Move");

	// if we actually moved
	if (pos != this.selected_item.pos && pos != this.selected_item.pos + 1) {

		trace("Move to Pos : " + pos + ", in gap : " + in_gap);

	}// end if

	this.selected_item.move_button.gotoAndStop("btn_up");

	this.action = '';

}// end itemEndMove()


mcListContainerClass.prototype.startMoveIndicator = function(on_end_fn) 
{
	if (this.move_indicator_on) return false;
	this.move_indicator_on = true;

	this.tmp.move_indicator = {on_end_fn: on_end_fn, pos: -1, in_gap: false};
	// move to top of layers
	this.move_indicator.swapDepths(this.items_order.length);

	// Hide the Menu so they can't do anything from there while using the move indicator
	_root.menu_container.hide();

	return true;

}// end startMoveIndicator()

mcListContainerClass.prototype.endMoveIndicator = function() 
{
	if (!this.move_indicator_on) return;

	// call back to the end fn for whoever called us
	this[this.tmp.move_indicator.on_end_fn](this.tmp.move_indicator.pos, this.tmp.move_indicator.in_gap);

	// clear the move indicator
	this.move_indicator._visible = false;
	this.move_indicator.swapDepths(-1);

	// Re-Show the menus
	_root.menu_container.show();

	this.move_indicator_on = false;

}// end endMoveIndicator()


mcListContainerClass.prototype.onMouseMove = function() 
{

	if (this.action != '') {

		var xm = this._xmouse;
		var ym = this._ymouse;

		var pos = this.getItemPos(xm, ym, true);
		// make sure the number is valid
		if (pos.pos > this.items_order.length) pos.pos = this.items_order.length;
		else if (pos.pos < 0) pos.pos = 0;

		if (pos.pos != this.tmp.move_indicator.pos || pos.in_gap != this.tmp.move_indicator.in_gap) {
			
			// if we are past the end of the list then we are really at the last pos, in the gap
			if (pos.pos == this.items_order.length) {
				pos.pos    = this.items_order.length - 1;
				pos.in_gap = true;
			}

			// if we are past the end of the list, improvise
			var item_name = this.items_order[pos.pos].name;
			var incr      = (pos.in_gap) ? _root.LIST_ITEM_POS_INCREMENT : 0;

			this.move_indicator._x  = this[item_name]._x;
			this.move_indicator._y  = this[item_name]._y + incr;
			this.move_indicator._visible = true;
			this.tmp.move_indicator.pos    = pos.pos;
			this.tmp.move_indicator.in_gap = pos.in_gap;

		}// endif

	}// end if action

}// end onMouseMove()


/**
* Executes and manages actions related to the tree, 
* actions mainly from the menu
*
* @param string action	the action to perform
* @param string info	additional information needed for action
*
*/
mcListContainerClass.prototype.execAction = function(action, info) 
{

	switch(action) {
		case "add" :
			trace('Add ' + info);
			if (this.startMoveIndicator("execActionAddAsset")) {
				this.action = "add_asset";
				this.tmp.exec_action = {type_code: info};
			} else {
				_root.showDialog("Unable to aquire the Move Indicator");
			}

		break;

		default:
			_root.showDialog("List Action Error", "Unknown action '" + action + "'");

	}// end switch

}// end execAction()


/**
* Attempt to add the asset
*
* @param int		pos	the	position in the item_order array that the move indicator came to it's final rest
* @param boolean	in_gap	whether the indicator finished up in the gap at the end of the list
*
*/
mcListContainerClass.prototype.execActionAddAsset = function(pos, in_gap) 
{

	trace("Add an asset of type '" + this.tmp.exec_action.type_code + "' to pos " + pos + ". In Gap ? " + in_gap);
	var info = this.posToAssetInfo(pos, in_gap);

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
	var exec_identifier = _root.server_exec.init_exec(xml, this, "execActionGoUrl", "url");
	_root.server_exec.exec(exec_identifier, "Sending Request");

	this.action = '';

}// end execActionAddAsset()



/**
* call back fn used by the execAction*() fns to after they sedn their urls requests to the server
*
* @param object xml	the xml object returned
*
*/
mcListContainerClass.prototype.execActionGoUrl = function(xml, exec_identifier)
{
	var frame = xml.firstChild.attributes.frame;
	var link  = xml.firstChild.firstChild.nodeValue;

	trace('getURL(' + link + ', ' + frame + ');');
	getURL(link, frame);

}// end execActionGoUrl()



/**
* Called by the javascript from the page containing the movie
* Reloads the assetid in the params
*
* @param object params	an object of the params that have been passed in from the outside
*
*/
mcListContainerClass.prototype.externReloadAsset = function(params)
{
	if (params.assetid == undefined || this.assets[params.assetid] == undefined) {
		_root.showDialog("External Reload Asset Error", "Asset #" + params.assetid + " not known");
		return;
	}

	this.reloadAsset(params.assetid);

}// end externReloadAsset()


Object.registerClass("mcListContainerID", mcListContainerClass);
