/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: mcListItemContainerClass.as,v 1.22 2003/10/26 23:11:26 dwong Exp $
* $Name: not supported by cvs2svn $
*/

#include "mcListItemClass.as"

 /////////////////////////////////////////////////////////////////////////////
// NOTE: the list items in this container are just stored as
// normal attributes and not in there own array (as I would have liked)
// this is due to the attachMovie() fn not being able to accept arrays
// elements as the instance name for the new movie

// Create the Class
function mcListItemContainerClass()
{
	// holds some information on all the list items, in the vertical order that they appear in the list
	// eg li_2_1_3 would be the mean this element exists here
	this.items_order = new Array();

	// the number of items that ever been created (used for depth when attaching movies)
	this.num_items = 0;

	// an array of assetid => Array(list item names)
	this.asset_list_items = new Object();

	// if an attr starts with this then it's a list item
	this.item_prefix = "li";

	// reference to the currently selected item
	this.selected_item = null;

	// some current settings/flags
	this.action      = "";

	// the types codes that we are restricting active status of assets to
	this._active_type_codes = new Array();

	// a temp object that can hold any run-time data
	this.tmp = new Object();

	// Set ourselves up as a listener on the asset types, so we know when they have been loaded
	_root.asset_manager.addListener(this);

	// listen on status view changes
	_root.header.toolbar_clip.addListener(this);

	// Set ourselves up as a broadcaster
//	ASBroadcaster.initialize(this);

	// highlighted asset items
	this._highlighted_items = new Array();
	this.status_view = false;

}// end constructor

// Make it inherit from Nested Mouse Movements MovieClip
mcListItemContainerClass.prototype = new NestedMouseMovieClip(false, NestedMouseMovieClip.NM_ON_PRESS);


/**
* Event fired when the Asset Types object has finished recieving all the asset types from the server
* We can then run init()
*/
mcListItemContainerClass.prototype.onAssetManagerInitialised = function()
{
	this.showKids('1', this.item_prefix);
}

mcListItemContainerClass.prototype.onStatusToggle = function()
{
	this.status_view = !this.status_view;
//	trace("onStatusToggle: " + this.status_view);
	for (i in this) {
		if (this[i] instanceof mcListItemClass) {
			this[i].setShowColours(this.status_view);
		}
	}

	if (!this.status_view) {
		if (this.selected_item) {
			this._highlightPath(this.selected_item);
		}
	}
}

/**
* Called by the mcListItem's on the pressing of the plus button
*
* @param int		parent_assetid	the assetid of the asset who's kids to show
* @param string		parent_name		the list item name of the parent that the kids are attached to
* @param boolean	dont_refresh	prevent the refreshing of the list
*
*/
mcListItemContainerClass.prototype.showKids = function(parent_assetid, parent_name, dont_refresh)
{

	if (dont_refresh != true) dont_refresh = false;

	// we don't know anything about this or it's got no kids, bugger off
	if (_root.asset_manager.assets[parent_assetid] == undefined || !_root.asset_manager.assets[parent_assetid].links.length) return;

	var params = {parent_assetid: parent_assetid, parent_name: parent_name, dont_refresh: dont_refresh};
	_root.asset_manager.loadAssets(_root.asset_manager.assets[parent_assetid].getLinkAssetids(), this, "showKidsLoaded", params);

}// end showKids()


/**
* A continuation of showKids() after the assets have been loaded
*/
mcListItemContainerClass.prototype.showKidsLoaded = function(params)
{
	var parent_assetid = params.parent_assetid;
	var parent_name	   = params.parent_name;

	// Now create the kids list items, if they don't already exist
	for(var j = 0; j < _root.asset_manager.assets[parent_assetid].links.length; j++) {
		var linkid = _root.asset_manager.assets[parent_assetid].links[j];
		var link = _root.asset_manager.asset_links[linkid];
		var item_name = parent_name + "_" + linkid;
		this._createItem(parent_name, item_name, link);

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
		this.refresh(parent_i);
	}

}// end showKidsLoaded()

mcListItemContainerClass.prototype._highlightPath = function(item)
{
	var next_item = item;
	while (next_item.parent_item_name != undefined) {
		next_item.setShowColours(true);
		this._highlighted_items.push(next_item);
		next_item = this[next_item.parent_item_name];
	}
}

mcListItemContainerClass.prototype._unHighlightPath = function()
{
	while (this._highlighted_items.length > 0) {
		var item = this._highlighted_items.shift();
		item.setShowColours(false);
	}
}

/**
* Splices the items order array recursively, adding the kids of the passed parent
*
*/
mcListItemContainerClass.prototype._recurseDisplayKids = function(parent_assetid, parent_name, parent_i)
{

	var i = parent_i + 1;

	// Now add the kids into the items order array in the correct spot
	for(var j = 0; j < _root.asset_manager.assets[parent_assetid].links.length; j++) {
		var name = parent_name + "_" + _root.asset_manager.assets[parent_assetid].links[j];
		if (this[name] == undefined) continue;
		this._insertItem(i, name, (j == _root.asset_manager.assets[parent_assetid].links.length - 1));
		if (this[name].expanded()) {
			i = this._recurseDisplayKids(this[name].assetid, name, i);
		} else {
			i++;
		}
	}

	return i;

}// end _recurseDisplayKids()


/**
* Creates a list item mc
*
* @param string		parent_name		The parent list item name
* @param string		item_name		The item name for the new item
* @param int		link			The link for which this item is the minor party
*
*/
mcListItemContainerClass.prototype._createItem = function(parent_name, item_name, link)
{
	if (this[item_name] != undefined) 
		return;

	this.num_items++;
	var linkid = link.linkid;
	var indent = (_root.asset_manager.asset_links[linkid].majorid > 1) ? this[parent_name].indent + 1 : 0;
	var assetid = _root.asset_manager.asset_links[linkid].minorid;

	this.attachMovie("mcListItemID", item_name, this.num_items);
	this[item_name]._visible = false;

	this[item_name].setParent(parent_name);
	this[item_name].setAsset(_root.asset_manager.assets[assetid]);
	this[item_name].setLink(link);
	this[item_name].setIndent(indent);
	var active = (this._active_type_codes.length == 0 || this._active_type_codes.search(this[item_name].type_code) !== null);
	this[item_name].setActive(active);
	this[item_name].setShowColours(this.status_view);

	if (this.asset_list_items[assetid] == undefined) this.asset_list_items[assetid] = new Array();
	this.asset_list_items[assetid].push(item_name);


}// end _createItem()

/**
* Deletes a list item mc
*
* @param string		item_name		The item name for the new item
*
*/
mcListItemContainerClass.prototype._deleteItem = function(item_name)
{
	if (this[item_name] == undefined) return;

	// Delete any of our kids that exist
	var links = _root.asset_manager.assets[this[item_name].assetid].links;
	for(var j = 0; j < links.length; j++) {
		this._deleteItem(item_name + "_" + links[j]);
	}// end for

	this.asset_list_items[this[item_name].assetid].removeElement(item_name);
	this._removeItem(item_name);
	this[item_name].removeMovieClip();

}// end _deleteItem()

/**
* Inserts an item into the items_order array
*
* @param int		pos			The position to insert to
* @param string		name		The name of the list item
* @param boolean	end_branch	Whether this is an end branch or not
*
*/
mcListItemContainerClass.prototype._insertItem = function(pos, name, end_branch)
{
	this.items_order.splice(pos, 0, {name: name, branch_count: 0, end_branch: end_branch});
}// end _insertItem()

/**
* Removes an item from the items_order array
*
* @param string		item_name		The name of the list item
*
*/
mcListItemContainerClass.prototype._removeItem = function(item_name)
{
	if (this[item_name] == undefined) return;

	// if the one we are removing is the currently selected item, un select it
	if (this.selected(this[item_name])) {
		this.unselectItem();
	}

	for(var x = 0; x < this.items_order.length; x++) {
		if (this.items_order[x].name == item_name) {
			this.items_order.splice(x, 1);
			break;
		}
	}

}// end _removeItem()

/**
* Called by the mcListItem's on the pressing of the minus button
* Hiding the children of the passed
*
* @param int		parent_assetid	the assetid of the asset who's kids to hide
* @param string		parent_name		the list item name of the parent that the kids are attached to
* @param boolean	dont_refresh	prevent the refreshing of the list
*
*/
mcListItemContainerClass.prototype.hideKids = function(parent_assetid, parent_name, dont_refresh)
{
	if (dont_refresh != true) dont_refresh = false;

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
		this.refresh(parent_i);
	}

}// end hideKids()

mcListItemContainerClass.prototype._recurseHideKids = function(parent_assetid, parent_name)
{

	var num_kids = _root.asset_manager.assets[parent_assetid].links.length;

	// loop through all the kids and make them invisible, also recursivly remove their kids
	for(var j = 0; j < _root.asset_manager.assets[parent_assetid].links.length; j++) {
		var name = parent_name + "_" + _root.asset_manager.assets[parent_assetid].links[j];
		if (this[name] == undefined) continue;
		if (this[name].expanded()) {
			num_kids += this._recurseHideKids(this[name].assetid, name);
		}
		this[name]._visible = false;
	}

	return num_kids;

}// end _recurseHideKids()

/**
* Event fired by asset manager
*
* @param int			assetids
* @param object Asset	new_assets	The new version of the assets
* @param object Asset	old_assets	The old version of the assets
*
* @access public
*/
mcListItemContainerClass.prototype.onAssetsReload = function(assetids, new_assets, old_assets)
{
//	trace ("onAssetReload");
	var linkids = Array();
	for (linkid in _root.asset_manager.asset_links) {
		if (parseInt(linkid)  > 0) {
			trace(_root.asset_manager.asset_links[linkid]);
			linkids.push(linkid);
		}
	}
//	trace ("linkids: " + linkids);
	for(var j = 0; j < assetids.length; j++) {

		var assetid = assetids[j];

		var deletes = old_assets[assetid].links.diff(new_assets[assetid].links);
		var inserts = new_assets[assetid].links.diff(old_assets[assetid].links);

		// if it's the root folder
		if (assetid == 1) {
			this._reloadAssetListItem(assetid, this.item_prefix, -1, true, deletes, inserts, new_assets[assetid].links);

		// else it's just a normal asset
		} else {

			for(var i = 0; i < this.asset_list_items[assetid].length; i++) {
				var item_name = this.asset_list_items[assetid][i];
				// we need to the get the pos this way because the this[item_name].pos will be out
				// of wack until we call the refresh below
				var pos  = 0; // outside for scope reasons
				for(; pos < this.items_order.length; pos++) {
					if (this.items_order[pos].name == item_name) break;
				}
				if (pos >= this.items_order.length) continue; // we didn't find it

				this._reloadAssetListItem(assetid, item_name, pos, this[item_name].expanded(), deletes, inserts, new_assets[assetid].links);

			}// end for

		}// end if

	}// end for


	// OK to make sure that every thing is in the correct order we hide everything
	// but DON'T refresh, then we show every thing again will all expanded assets
	// being displayed
	this.hideKids(1, this.item_prefix, true);
	this.showKids(1, this.item_prefix);

	// Now if there has been a asset move and we know where to select, then do so
	if (this.tmp.move_asset.new_select_name != undefined) {
		this.selectItem(this[this.tmp.move_asset.new_select_name]);
		delete this.tmp.move_asset.new_select_name;
	}

}// end onAssetReload()


/**
* Called by onAssetReload() to reload a single asset list item
*
* @param int		assetid
* @param string		item_name	the list item we are reloading
* @param int		pos			the current pos of the list item
* @param boolean	expanded	whether this list item is expanded or not
* @param Array		deletes		linkids that have been removed
* @param Array		inserts		linkids that have been added
* @param Array		all_links	all the linkids for the asset
*
* @access private
*/
mcListItemContainerClass.prototype._reloadAssetListItem = function(assetid, item_name, pos, expanded, deletes, inserts, all_kids)
{
	trace("_reloadAssetListItem (" + assetid + ", " + item_name + ", " + pos + ", " + expanded + ", " + deletes + ", " + inserts + ", " + all_kids + ")");

	// delete any child items that aren't needed any more
	for(var j = 0; j < deletes.length; j++) {
		var kid_name = item_name + "_" + deletes[j];
		this._deleteItem(kid_name);
	}// end for

	// the reset of this is only relevant if the item is expanded
	if (expanded) {
		// insert any child items that have just been added
		for(var j = 0; j < inserts.length; j++) {
			var kid_name = item_name + "_" + inserts[j];
			var link = _root.asset_manager.asset_links[inserts[j]];
			this._createItem(item_name, kid_name, link);
			this._insertItem(pos + 1, kid_name, false);

		}// end for

	}// end if expanded

}// end _reloadAssetListItem()


/**
* Refreshes the display of this container's items, and inform the list container (our parent)
* of the refresh
*
* @param int start_i   the index in the items_order array to start the refresh from
*
* @access public
*/
mcListItemContainerClass.prototype.refresh = function(start_i)
{
	this.refreshDisplay(start_i);
	this._parent.refreshDisplay();
}

/**
* Refreshes the display of the items, from a certain position onwards
* don't inform our parent
*
* @param int start_i   the index in the items_order array to start the refresh from
*
* @access public
*/
mcListItemContainerClass.prototype.refreshDisplay = function(start_i)
{

	if (start_i == undefined || start_i < 0) start_i = 0;

	// now cycle through every item from the parent down and reset their positions
	var branch_count = (start_i >= 0) ? this.items_order[start_i].branch_count : 0;
	
	for(var i = start_i; i < this.items_order.length; i++) {
		// set for future use
		this.items_order[i].branch_count = branch_count;
		var item = this[this.items_order[i].name];
		item.setPos(i);

		var active;
		if (this._active_type_codes.length == 0 || (this._active_type_codes.search(item.type_code) != null)) {
			active = true;
		} else {
			active = false;
		}
//		trace(item + ": " + active);
		item.setActive(active);

		item._visible = true;
		item.refresh();

		// if we have come across an end branch,
		// and if we aren't at the last item
		// and if this element has no kids, then we add the branch gap
		if (this.items_order[i].end_branch) {
			if (i < this.items_order.length - 1) {
				var this_indent = item.indent;
				var next_indent = this[this.items_order[i + 1].name].indent;
				if (this_indent != (next_indent - 1)) {
					branch_count++;
				}
			}
		}

	}// end for

}// end refreshDisplay()


/**
* Refreshes the display of the items, from a certain position onwards
* don't inform our parent
*
* @param Arrsy(string) start_i   the index in the items_order array to start the refresh from
*
* @access public
*/
mcListItemContainerClass.prototype.restrictActiveTypes = function(type_codes)
{
//	trace("Restrict Active Types : " + type_codes);
	this._active_type_codes = type_codes;

	for(var i = 0; i < this.items_order.length; i++) {
		// if there are no type codes or this is one of the active type_codes, set it so
		var item = this[this.items_order[i].name];
		var active = false;
		if (this._active_type_codes.length == 0) {
			active = true;
		} else if (this._active_type_codes.search(item.type_code) !== null) {
			active = true;
		}
//		trace(item.name + " : " + (this._active_type_codes.length == 0) + " || " + (this._active_type_codes.search(item.type_code) !== null));
		item.setActive(active);
	}// end for

}// end restrictActiveTypes()

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
mcListItemContainerClass.prototype.getItemPos = function(x, y, bleed_gaps)
{

	if (bleed_gaps != true) bleed_gaps = false;

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

mcListItemContainerClass.prototype.selectItem = function(item)
{
	// if we in any action we don't want to select anything
	if (this.action != "") return false;

	if (this.selected_item == item) return true;

	if (this.selected_item != null) {
		this.unselectItem();
	}

	this.selected_item = item;
	this.selected_item.select();

	if (!this.status_view)
		this._highlightPath(this.selected_item);

//	this.broadcastMessage("onListItemSelection", this.selected_item.assetid);

	return true;

}

mcListItemContainerClass.prototype.unselectItem = function()
{
	this.selected_item.unselect();
	this.selected_item = null;

	if (!this.status_view)
		this._unHighlightPath();
//	this.broadcastMessage("onListItemUnSelection");
}

mcListItemContainerClass.prototype.selected = function(item)
{
	return (this.selected_item == item);
}


/**
* Called to start the moving of an asset
*/
mcListItemContainerClass.prototype.startMove = function()
{
	if (this.action != "") return false;
	if (this._parent._parent.finding_asset) return false;

	if (this._parent.move_indicator.startIndicator(this, "endMove")) {
		this.action = "move";
		// move to top of layers
		this.selected_item.setMoveState("on");
	}

}// end startMove()

/**
* Called after the move position has been selected
*
* @param string	parent_item_name	the name of the item that the pos is to be under
* @param int	parent_assetid		the	assetid of the item that the pos is to be under
* @param int	relative_pos		the relative in the parent_asset's links array that the asset is to be placed
*
*/
mcListItemContainerClass.prototype.endMove = function(parent_item_name, parent_assetid, relative_pos)
{
	if (this.action != "move") return;

	var do_move = false;
	var diff_parents = (this.selected_item.getParentAssetid() != parent_assetid);
	var curr_relative_pos = _root.asset_manager.assets[this.selected_item.getParentAssetid()].linkPos(this.selected_item.linkid);

	// if we are staying under the same parent
	// AND if we are moving the asset down the list
	if (!diff_parents && curr_relative_pos < relative_pos) {
		// then we need to decrement the pos - see me for a demo of why (BCR)
		relative_pos -= 1;
	}

	// if we actually moved
	if (diff_parents || curr_relative_pos != relative_pos) {

		var params = new Object();
		params.parent_item_name	= parent_item_name; // we save this to highlight it's new kid when it appears
		params.parent_assetid	= parent_assetid;
		params.relative_pos		= relative_pos;

		// if the parent has changed during the move or if they have pressed CONTROL
		// then we need to ask what we want done
		if (diff_parents || Key.isDown(Key.CONTROL)) {
			_root.options_box.init("Move Type", "Are you moving this asset, creating a new link or duplicating this asset?", this, "moveConfirm", params);
			_root.options_box.addOption("move asset", "Moving");
			if (diff_parents) _root.options_box.addOption("new link",   "New Link");
			_root.options_box.addOption("dupe", "Duplicate");
			_root.options_box.show();

		// otherwise just move it
		} else {
			this.moveConfirm("move asset", params);

		}// end if

	} else {
		// same as hitting cancel
		this.moveConfirm(null, {});
	}// end if

}// end endMove()


mcListItemContainerClass.prototype.moveConfirm = function(move_type, params)
{
	if (this.action != "move") return;

	// if they didn't hit cancel
	if (move_type != null) {

		var xml = new XML();
		var cmd_elem = xml.createElement("command");
		cmd_elem.attributes.action              = move_type;
		cmd_elem.attributes.from_parent_assetid = this.selected_item.getParentAssetid();
		cmd_elem.attributes.linkid              = this.selected_item.linkid;
		cmd_elem.attributes.to_parent_assetid   = params.parent_assetid;
		cmd_elem.attributes.to_parent_pos       = params.relative_pos;
		xml.appendChild(cmd_elem);

		// start the loading process
		var exec_identifier = _root.server_exec.init_exec(xml, this, "xmlMoveItem", "responses");
		if (this.tmp.move_asset == undefined) this.tmp.move_asset = new Object();
		this.tmp.move_asset[exec_identifier] = params;
		_root.server_exec.exec(exec_identifier, ((move_type == "move asset") ? "Moving Asset" : "Creating new link"));

	}

	this.selected_item.setMoveState("off");
	this.action = "";

}// end moveConfirm()

mcListItemContainerClass.prototype.xmlMoveItem = function(xml, exec_identifier)
{
	for (var i = 0; i < xml.childNodes.length; ++i) {
		nextChildResponse = xml.childNodes[i];

		switch (nextChildResponse.firstChild.nodeName) {
			case "success" : 
				var select_linkid = nextChildResponse.firstChild.attributes.linkid;

				// OK let's save the pos of that we were dragged to so that we can select the correct
				// list item later after the parents have been reloaded
				this.tmp.move_asset.new_select_name = this.tmp.move_asset[exec_identifier].parent_item_name + "_" + select_linkid;

				// get the asset manager to reload the assets
				_root.asset_manager.reloadAssets([this.selected_item.getParentAssetid(), this.tmp.move_asset[exec_identifier].parent_assetid]);

				break;

			case "url" : 
				_root.external_call.makeExternalCall(nextChildResponse.firstChild.attributes.js_function, {url: nextChildResponse.firstChild.firstChild.nodeValue});
				break;

			default :
				_root.dialog_box.show("Connection Failure to Server", "Unexpected Response XML Node '" + nextChildResponse.firstChild.nodeName + "', expecting 'url' or 'success'.\nPlease Try Again");
				break;
		}// end switch
	}// end for

}// end xmlMoveItem()

Object.registerClass("mcListItemContainerID", mcListItemContainerClass);
