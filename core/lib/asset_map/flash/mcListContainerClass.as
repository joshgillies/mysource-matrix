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

}// end constructor

// Make is inherit from MovieClip
mcListContainerClass.prototype = new MovieClip();

/**
* Called by the mcListItem's on the pressing of the plus button
* Calls the root getAssetKids() which gets the XML data then 
* calls displayKids() below
*
* @param int $parent_assetid   the assetid of the asset who's kids to show
*
*/
mcListContainerClass.prototype.showKids = function(parent_assetid) 
{

	// we don't know anything about this or it's got know kids, bugger off
	if (this.assets[parent_assetid] == undefined || !this.assets[parent_assetid].has_kids) return;

	// check to see if we have already been to this asset's kids
	if (this.assets[parent_assetid].kids.length > 0) {
		this._displayKids(parent_assetid);

	// else load from server
	} else {

		var xml = new XML();
		var cmd_elem = xml.createElement("command");
		cmd_elem.attributes.action = "get kids";
		cmd_elem.attributes.parent_assetid = parent_assetid;
		xml.appendChild(cmd_elem);

		trace(xml);

		// start the loading process
		_root.server_exec.exec(xml, this, "loadKids", "assets", "Loading Assets");
		this.tmp.parent_assetid = parent_assetid;
	}
	
}// end showKids()

/**
* Called after the XML has been loaded 
*
* @param object XML $xml   the xml object that contain the information that we need
*
*/
mcListContainerClass.prototype.loadKids = function(xml) 
{

	// get a reference to the parent item
	var parent_assetid = this.tmp.parent_assetid;
	var parent_name = (this.selected_item == null) ? this.item_prefix : this.selected_item._name;

	children = xml.firstChild.childNodes;
	for (var i = 0; i < children.length; i++) {
		// get a reference to the child node
		var asset_node = children[i];
		if (asset_node.nodeName.toLowerCase() == "child") {

			var assetid = asset_node.attributes.assetid;
			this.assets[assetid] = new Asset(assetid, 
											 asset_node.attributes.type_code, 
											 asset_node.firstChild.nodeValue, 
											 asset_node.attributes.has_kids);

			this.num_items++;

			var indent      = (this.selected_item == null) ? 0 : this.selected_item.indent + 1;

			var item_name = parent_name + "_" + assetid;
			this.attachMovie("mcListItemID", item_name, this.num_items);
			this[item_name]._visible = false;

			this[item_name].setInfo(parent_name, this.assets[assetid]);
			this[item_name].setIndent(indent);

			this.assets[parent_assetid].kids.push(assetid);

		}//end if
	}//end for

	this._displayKids(parent_assetid);

}// end loadKids()


/**
* Called to display the assets, in the correct order after they have been loaded into the 
* container
*
* @param string $parent_assetid   the asset whose kids we are showing
*
*/
mcListContainerClass.prototype._displayKids = function(parent_assetid) 
{

	// see if we can find this parent in the list
	var parent_i = (this.selected_item == null) ? -1 : this.selected_item.pos;

	this._recurseDisplayKids(parent_assetid, ((this.selected_item == null) ? this.item_prefix : this.selected_item._name), parent_i);

	// Because we only want to change the collapse sign for the top level
	if (this.selected_item != null) {
		this.selected_item.setKidState("minus");
	}

	this.refreshList(parent_i);

}// end _displayKids()

/**
* Splices the items order array recursively, assing the kids of the passed parent
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

mcListContainerClass.prototype.hideKids = function(parent_assetid) 
{

	// we don't know anything about it - "should" never happen
	if (this.selected_item == null) return;
	var parent_i = this.selected_item.pos;
	
	var num_to_remove = this._recurseHideKids(parent_assetid, this.selected_item._name, parent_i);

	// Now remove the kids from the items order
	this.items_order.splice(parent_i + 1, num_to_remove);

	// Because we only want to change the expand sign for the top level
	if (this.selected_item != null) {
		this.selected_item.setKidState("plus");
	}

	this.refreshList(parent_i);

}// end hideKids()

mcListContainerClass.prototype._recurseHideKids = function(parent_assetid, parent_name, parent_i) 
{

	var i = parent_i + 1;
	var num_kids = this.assets[parent_assetid].kids.length;
	
	// loop through all the kids and make them invisible, also recursivly remove their kids
	for(var j = 0; j < this.assets[parent_assetid].kids.length; j++) {
		var name = parent_name + "_" + this.assets[parent_assetid].kids[j];
		if (this[name] == undefined) continue;
		if (this[name].expanded()) {
			num_kids += this._recurseHideKids(this[name].assetid, name, i);
		}
		this[name]._visible = false;
	}

	return num_kids;

}// end _recurseHideKids()

/**
* Refreshes the display of the items, from a certain position onwards
*
* @param int start_i   the index in the items_order array to start the refresh from 
*
* @access public
*/
mcListContainerClass.prototype.refreshList = function(start_i) 
{

	if (start_i == null) start_i = 0;
	else if (start_i < 0) start_i = 0;

	// now cycle through every item from the parent down and reset their positions
	var branch_count = (start_i >= 0) ? this.items_order[start_i].branch_count : 0;
	for(var i = start_i; i < this.items_order.length; i++) {
		// set for future use
		this.items_order[i].branch_count = branch_count;
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
	var ypos = Math.max(_root.scroller.getPaneHeight(), this._height + _root.LIST_ITEM_END_BRANCH_GAP);
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
							this.showKids(this.selected_item.assetid);
						break;
						case "minus" :
							//	Collapse Branch
							this.hideKids(this.selected_item.assetid);
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
	_root.server_exec.exec(xml, this, "execActionGoUrl", "url", "Sending Request");

	this.action = '';

}// end execActionAddAsset()



/**
* call back fn used by the execAction*() fns to after they sedn their urls requests to the server
*
* @param object xml	the xml object returned
*
*/
mcListContainerClass.prototype.execActionGoUrl = function(xml)
{
	var frame = xml.firstChild.attributes.frame;
	var link  = xml.firstChild.firstChild.nodeValue;

	trace('getURL(' + link + ', ' + frame + ');');
	getURL(link, frame);

}// end execActionGoUrl()


Object.registerClass("mcListContainerID", mcListContainerClass);
