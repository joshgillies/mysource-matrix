
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

	// the current state that the buttons is in (normal, selected)
	this.state = "normal";
	this.actions_bar_interval = 0;

	this.text_field.swapDepths(1);

	// Create the Plus Minus Button
	this.attachMovie("mcListItemPlusMinusID", "kids_button", 2);
	this.kids_button._x = 3;
	this.kids_button._y = 3;

	// Create the Move Button
	this.attachMovie("mcListItemMoveID", "move_button", 3);
	this.move_button.setState("off");
	this.move_button._x = 20;
	this.move_button._y = 3;

	// set the text field up
	this.text_field.text = "";
	this.text_field.autoSize = "left";

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
	this.linkid = linkid
}

mcListItemClass.prototype.setAsset = function(asset) 
{
	this.setInfo(asset);
	asset.addListener(this);
}

mcListItemClass.prototype.setInfo = function(asset) 
{
	this.assetid   = asset.assetid;
	this.type_code = asset.type_code;
	this.text_field.text = this._name + " [" + this.assetid + "] " + asset.name;

	if (asset.links.length > 0) {
		this.setKidState((this.expanded()) ? "minus" : "plus");
	} else {
		this.setKidState("none");
	}

	this._drawBg();
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
	this.state = "selected";
	this._drawBg();
}

/**
* Called when this list item is unselected
*/
mcListItemClass.prototype.unselect = function() 
{
	this.state = "normal";
	this._drawBg();
}

/**
* Called when this item has been pressed
*/
mcListItemClass.prototype.onPress = function()
{
	this._parent.selectItem(this);
	// try the kids, but it they don't want it then set the interval for the actions bar
	if (!super.onPress()) {
		this.actions_bar_interval = setInterval(this, "showActionsBar", 500);
	}
	return true;
}// end onPress()

/**
* Called when this item has been pressed
*/
mcListItemClass.prototype.showActionsBar = function()
{
	clearInterval(this.actions_bar_interval);
	this.actions_bar_interval = 0;
	this._parent._parent.showActionsBar();
}// end showActionsBar()

/**
* Called when this item has been pressed and then released
*/
mcListItemClass.prototype.onRelease = function()
{
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
	if (this.actions_bar_interval) {
		clearInterval(this.actions_bar_interval);
		this.actions_bar_interval = 0;
	}
	super.onReleaseOutside();
	return true;
}// end onReleaseOutside()

/**
* Draw the Background for this list item
*/
mcListItemClass.prototype._drawBg = function() 
{
	var xpos = Math.max(200, this.text_field._x + this.text_field._width + 3);
	var ypos = _root.LIST_ITEM_POS_INCREMENT;

	this.clear();
	this.beginFill(_root.LIST_ITEM_BG_COLOURS[this.state].colour, _root.LIST_ITEM_BG_COLOURS[this.state].alpha);
	this.lineStyle();
	this.moveTo(0, 0);
	this.lineTo(xpos, 0);
	this.lineTo(xpos, ypos);
	this.lineTo(0, ypos);
	this.lineTo(0, 0);
	this.endFill();

}// end _drawBg()


Object.registerClass("mcListItemID", mcListItemClass);

