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
	// holds all the names of the list items in the vertical order that they appear in the list
	// eg li_2_1_3 would be the mean this element exists here
	this.items_order = new Array();
	this.num_items = 0;

	// reference to the currently selected item
	this.selected_item = null;

	// the current action for the container and any items
	this.action = '';

	// the index in the items_order array that the indicator is currently pointing on top of
	this.move_indicator_pos = 0;

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
		// start the loading process, if it returns true loading was initiated
		if (_root.getAssetKids(parent_assetid)) {
			this.tmp.parent_assetid = parent_assetid;
		}
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
//	trace('Loading Children with XML Object');

	// get a reference to the parent item
	var parent_assetid = this.tmp.parent_assetid;
	var parent_name = (this.selected_item == null) ? 'li' : this.selected_item._name;

	children = xml.firstChild.childNodes;
	for (var i = 0; i < children.length; i++) {
		// get a reference to the child node
		var assetNode = children[i];
		if (assetNode.nodeName.toLowerCase() == "child") {

			var assetid = assetNode.attributes.assetid;
			this.assets[assetid] = new Asset(assetid, 
											 assetNode.attributes.type_code, 
											 assetNode.firstChild.nodeValue, 
											 assetNode.attributes.has_kids);

			this.num_items++;

			var indent      = (this.selected_item == null) ? 0 : this.selected_item.indent + 1;

			var item_name = parent_name + "_" + assetid;
			this.attachMovie("mcListItemID", item_name, this.num_items);
			this[item_name]._visible = false;

			this[item_name].setInfo(this.assets[assetid]);
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

	this._recurseDisplayKids(parent_assetid, ((this.selected_item == null) ? 'li' : this.selected_item._name), parent_i);

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
//	trace('Display Children with XML Object : ' + parent_assetid + ' : ' + parent_name + ' : ' + parent_i);

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
//	trace('Hide Children Assets of : ' + parent_assetid + ' under the parent name : ' + parent_name);

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

	trace('Num Kids (' + parent_name + '): ' + num_kids);

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
	var ypos = Math.max(_root.scroller.getPaneHeight(), this._height);
	this.filler.clear();
	this.filler.beginFill(0xFF0000, 0); // alpha = 0 -> transparent
	this.filler.lineStyle();
	this.filler.moveTo(0, 0);
	this.filler.lineTo(xpos, 0);
	this.filler.lineTo(xpos, ypos);
	this.filler.lineTo(0, ypos);
	this.filler.lineTo(0, 0);
	this.filler.endFill();


	//Refresh the scroller
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
*
* @return int
* @access public
*/
mcListContainerClass.prototype.getItemPos = function(x, y) 
{

	if (y < 0) return -1;
	if (y > this[this.items_order[this.items_order.length - 1].name]._y +  + _root.LIST_ITEM_POS_INCREMENTS) {
		return this.items_order.length;
	}

	// OK the biggest problem we have here is the bloody end branch gaps
	// so what we can do is find the maximum position number that these co-ords
	// would produce by ignoring the gaps
	var max_pos = Math.floor(y / _root.LIST_ITEM_POS_INCREMENTS);

	// make sure the number is valid
	if (max_pos < 0) {
		return -1;
	} else if (max_pos >= this.items_order.length) {
		max_pos = this.items_order.length - 1;
	} 

	// Now we get a minimum position number, by using the branch count at the max pos
	var min_pos = max_pos - this.items_order[max_pos].branch_count;

//	trace("Max : " + max_pos + " Min : " + min_pos);

	while(min_pos <= max_pos) {

		var i = min_pos + Math.round((max_pos - min_pos) / 2);

//		trace(this.items_order[i].name + ', i : ' + i + ', y : ' + y + ' item y : ' + this[this.items_order[i].name]._y + "   Max : " + max_pos + " Min : " + min_pos);

		// if the mouse is before this element make the one above us the new max
		if (y < this[this.items_order[i].name]._y) {
			max_pos = i - 1;

		// if the mouse is after this element make the one below us the new min
		} else if (y > (this[this.items_order[i].name]._y + _root.LIST_ITEM_POS_INCREMENTS)) {
			min_pos = i + 1;

		// else mouse is in this element 
		} else {
			return i;

		}

	}//end while

	return -1;

}

mcListContainerClass.prototype.onPress = function() 
{

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
				trace('Pos : ' + pos);
				this.selectItem(this[this.items_order[pos].name]);
			}

	}// end switch

}// end onPress()

mcListContainerClass.prototype.onRelease = function() 
{

	switch(this.action) {
		case 'move' : 
			this.itemEndMove();
		break;

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

	trace("Start Item Move");
	this.action = 'move';

	// move to top of layers
	this.move_indicator.swapDepths(this.items_order.length);
	this.selected_item.move_button.gotoAndStop("btn_down");

	// reset the move indicator info
	this.move_indicator_pos = -1;

}// end itemStartMove()

mcListContainerClass.prototype.itemEndMove = function() 
{
	if (this.action != 'move') return;

	trace("End Item Move at Pos : " + this.move_indicator_pos);

	this.selected_item.move_button.gotoAndStop("btn_up");

	// clear the drag indicator info
	this.move_indicator_pos      = -1;
	this.move_indicator._visible = false;
	this.move_indicator.swapDepths(-1);


	// reset items depth and position
	this.selected_item.swapDepths(this.tmp.move_item_depth);
	this.selected_item.resetXY();

	this.action = '';

}// end itemEndMove()

mcListContainerClass.prototype.onMouseMove = function() 
{

	if (this.action != '') {

		var xm = this._xmouse;
		var ym = this._ymouse;

		var pos = Math.round(ym / _root.LIST_ITEM_POS_INCREMENTS);
//		trace('Y : ' + this._ymouse + ' Pos : ' + pos);
		// make sure the number is valid
		if (pos > this.items_order.length) pos = this.items_order.length;
		else if (pos < 0) pos = 0;

		if (pos != this.move_indicator_pos) {
			
			// if we are currently hovering over our old position 
			// or the position below us do nothing
			if (pos == this[this.move_item].pos || pos == this[this.move_item].pos + 1) {
				this.move_indicator._visible = false;
				this.move_indicator_pos      = -1;

			} else {
				// if we are past the end of the list, improvise
				var item_name = (pos == this.items_order.length) ? this.items_order[this.items_order.length - 1].name : this.items_order[pos].name;
				var incr      = (pos == this.items_order.length) ? _root.LIST_ITEM_POS_INCREMENTS : 0;

				this.move_indicator._x  = this[item_name]._x;
				this.move_indicator._y  = this[item_name]._y + incr;

//				trace('Pos : ' + pos + ' X : ' + this.move_indicator._x + ' Y : ' + this.move_indicator._y);

				this.move_indicator._visible = true;
				this.move_indicator_pos = pos;
			}

		}// endif

	}// end if moving

}// end onMouseMove()

Object.registerClass("mcListContainerID", mcListContainerClass);

