
#include "mcListItemPlusMinus.as"
#include "mcListItemMove.as"


// Create the Class
function mcListItemClass()
{
	this.parent_item_name = "";
	this.linkid    = "";
	this.assetid   = "";
	this.type_code = "";
	this.pos       = 0;
	this.indent    = 0;

	this._accessible = false;
	this._active = true;

	// the current state that the buttons is in 
	this.state = 'normal';
	this.selected = false;
	this.actions_bar_interval = 0;

	this.text_field.swapDepths(1);

	// Create the Plus Minus Button
	this.attachMovie("mcListItemPlusMinusID", "kids_button", 2);
	this.kids_button._x = 3;
	this.kids_button._y = 5;

	// Create the Move Button
	this.attachMovie("mcListItemMoveID", "move_button", 3);
	this.move_button.setState("off");
	this.move_button._x = 20;

	// set the text field up
	this.text_field.text = "";
	this.text_field.autoSize = "left";

	// selected and normal text formats
	this.normalTextFormat = new TextFormat();
	this.normalTextFormat.color = 0x000000;

	this.selectedTextFormat = new TextFormat();
	this.selectedTextFormat.color = 0xffffff;

}

// Make it inherit from Nested Mouse Movements MovieClip, nested mc's NEVER OVERLAP
mcListItemClass.prototype = new NestedMouseMovieClip(true, NestedMouseMovieClip.NM_ON_PRESS | NestedMouseMovieClip.NM_ON_RELEASE);

mcListItemClass.prototype.setParent = function(parent_item_name) 
{
	this.parent_item_name = parent_item_name;
}

mcListItemClass.prototype.getParentAssetid = function() 
{
	return _root.asset_manager.asset_links[this.linkid].majorid;
}


mcListItemClass.prototype.setLinkId = function(linkid) 
{
	this.linkid = linkid;
}

mcListItemClass.prototype.setAsset = function(asset) 
{
	this.setInfo(asset);
	asset.addListener(this);
}

mcListItemClass.prototype.setInfo = function(asset) 
{
//	trace (this + "::mcListItemClass.setInfo (" + asset + " )");

	this.assetid			= asset.assetid;
	this.type_code			= asset.type_code;
	this._accessible		= asset.accessible;
	this.text_field.text	= asset.name;
	this.asset_status		= asset.status;

	this.move_button.setIcon("mc_asset_type_" + asset.type_code + "_icon");

	if (asset.status & Asset.LIVE_STATUS)
		this.state = 'live';
	else if (asset.status & Asset.UNDER_CONSTRUCTION_STATUS)
		this.state = 'under_construction';

	if (!this._accessible) {
		this.state = 'error';
	}

	this.base_colour = _root.LIST_ITEM_BG_COLOURS[this.state].colour;

	if (asset.accessible && asset.links.length > 0) {
		this.setKidState((this.expanded()) ? "minus" : "plus");
	} else {
		this.setKidState("none");
	}

	this.refresh();
}

mcListItemClass.prototype.getKidState = function() 
{
	return this.kids_button.getState();
}

mcListItemClass.prototype.setKidState = function(state) 
{
	this.kids_button.setState(state);
}

mcListItemClass.prototype.setMoveState = function(state) 
{
	this.move_button.setState(state);
}


mcListItemClass.prototype.expanded = function() 
{
	return (this.kids_button.getState() == "minus") ? true : false;
}

mcListItemClass.prototype.setIndent = function(indent) 
{
	if (indent   < 0) indent   = 0;
	this._x = indent * _root.LIST_ITEM_INDENT_SPACE;
	this.indent   = indent;
}

mcListItemClass.prototype.setPos = function(pos) 
{
	if (pos < 0) pos = 0;
	this._y = (pos * _root.LIST_ITEM_POS_INCREMENT) + (this._parent.items_order[pos].branch_count * _root.LIST_ITEM_END_BRANCH_GAP);
	this.pos = pos;
}

mcListItemClass.prototype.resetXY = function() 
{
	this.setIndent(this.indent);
	this.setPos(this.pos);
}


/**
* Called by the kids button to show our kids
* @access public
*/
mcListItemClass.prototype.showKids = function() 
{
	this._parent.showKids(this.assetid, this._name);
}

/**
* Called by the kids button to hide our kids
* @access public
*/
mcListItemClass.prototype.hideKids = function() 
{
	this._parent.hideKids(this.assetid, this._name);
}


/**
* Called by the move button to start a move
* @access public
*/
mcListItemClass.prototype.startMove = function() 
{
	// if we aren't accessible or active we can't move
	if (!this._accessible || !this._active) return;
	this._parent.startMove();
}

/**
* Returns the button over which the mouse currently resides
*
* @returns string
* @access public
*/
mcListItemClass.prototype.getMouseButton = function() 
{
	// if we are over the kids button
	if (this.kids_button.hitTest(_root._xmouse, _root._ymouse, false)) {
		return "kids";

	// if we are over the move button
	} else if (this.move_button.hitTest(_root._xmouse, _root._ymouse, false)) {
		return "move";

	} else {
		return "";
	}

}

/**
* When the asset is changed update ourselves to reflect changes
*
* @param object Asset	asset	the asset that changed
*
*/
mcListItemClass.prototype.onAssetChange = function(asset) 
{
	this.setInfo(asset);
}


/**
* Called when this list item is selected
*/
mcListItemClass.prototype.select = function() 
{
	if (!this._active)
		return;
	this.selected = true;
	this.text_field.setTextFormat(this.selectedTextFormat);

	this.base_colour = adjust_brightness (_root.LIST_ITEM_BG_COLOURS[this.state].colour, 0.8);
	this._drawBg();
}

/**
* Called when this list item is unselected
*/
mcListItemClass.prototype.unselect = function() 
{
	if (!this._active)
		return;

	this.selected = false;
	this.text_field.setTextFormat(this.normalTextFormat);

	this.base_colour = _root.LIST_ITEM_BG_COLOURS[this.state].colour;
	this._drawBg();
}

/**
* Called to enable/disable this item
*
* @param boolean	active		are we active or not
* 
*/
mcListItemClass.prototype.setActive = function(active)
{
	if (!active)
		this.unselect();

	this._active = active;

	var text_format = this.text_field.getTextFormat();
	text_format.color = (this._active) ? 0x000000 : 0x999999;
	this.text_field.setTextFormat(text_format); 

}// end setActive()


/**
* Called when this item has been pressed
*/
mcListItemClass.prototype.onPress = function()
{
	if (!this._accessible) return false;

	this._parent.selectItem(this);
	// try the kids, but it they don't want it then set the interval for the actions bar
	if (!super.onPress() && this._active) {
		this.actions_bar_interval = setInterval(this, "showActionsBar", 100);
	}
	return true;
}// end onPress()

/**
* Called when this item has been pressed
*/
mcListItemClass.prototype.showActionsBar = function()
{
	if (!this._accessible) return;
	clearInterval(this.actions_bar_interval);
	this.actions_bar_interval = 0;
	this._parent._parent.showActionsBar();
}// end showActionsBar()

/**
* Called when this item has been pressed and then released
*/
mcListItemClass.prototype.onRelease = function()
{
	if (!this._accessible) return false;
	if (this.actions_bar_interval) {
		clearInterval(this.actions_bar_interval);
		this.actions_bar_interval = 0;
	}
	super.onRelease();
	return true;
}// end onRelease()

/**
* Called when this item has been pressed and then the mouse was released somewhere else
*/
mcListItemClass.prototype.onReleaseOutside = function()
{
	if (!this._accessible) return false;
	if (this.actions_bar_interval) {
		clearInterval(this.actions_bar_interval);
		this.actions_bar_interval = 0;
	}
	super.onReleaseOutside();
	return true;
}// end onReleaseOutside()

mcListItemClass.prototype.refresh = function()
{
//	trace (this + "::mcListItemClass.refresh()");

	var iconPadding = 5;
	var nextX = iconPadding;
	
	this.kids_button._x = nextX;
	nextX += this.kids_button._width;
	nextX += iconPadding;

	this.move_button._x = nextX;
	nextX += this.move_button._width;
	nextX += iconPadding;
	
	nextX = Math.max (nextX, 20);
	this.text_field._x = nextX;

//	this.kids_button._y = (this._height - this.kids_button._y) / 2;
//	this.move_button._y = (this._height - this.move_button._y) / 2;
//	this.text_field._y = (this._height - this.text_field._y) / 2;

	this._drawBg();

}

/**
* Draw the Background for this list item
*/
mcListItemClass.prototype._drawBg = function() 
{
	var list_container = this._parent._parent;
	var left = -this._x;
	var top = 2;
	var right = Math.max (_root.tabs.tree.scroll_pane._width - this._x - 25, list_container.getRightEdge() - this._x + 5);
	var bottom = _root.LIST_ITEM_POS_INCREMENT - 2;

	this.clear();

	this.beginFill(this.base_colour, _root.LIST_ITEM_BG_COLOURS[this.state].alpha);

	this.lineStyle();
	this.moveTo(left, top);
	this.lineTo(right, top);
	this.lineTo(right, bottom);
	this.lineTo(left, bottom);
	this.lineTo(left, top);
	this.endFill();

}// end _drawBg()

Object.registerClass("mcListItemID", mcListItemClass);

