
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

	this.setMoveState("off");

	// Create the Plus Minus Button
	this.attachMovie("mcPlusMinusID", "kids_button", 2);
	this.kids_button._x = 3;
	this.kids_button._y = 3;

	// set the text field up
	this.text_field.text = "";
	this.text_field.autoSize = "left";

	// Set the depths up properly
	this.kids_button.swapDepths(1);
	this.text_field.swapDepths(2);
	this.move_button.swapDepths(3);

}

// Make it inherit from MovieClip
mcListItemClass.prototype = new MovieClip();

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
	link.addListener(this);
}

mcListItemClass.prototype.setInfo = function(asset) 
{
	this.assetid   = asset.assetid;
	this.type_code = asset.type_code;
	this.text_field.text = this._name + ' [' + this.assetid + '] ' + asset.name;

	if (!asset.links.length) {
		this.setKidState("none");
	} else if (!this.expanded()) {
		this.setKidState("plus");
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
	this.move_button.gotoAndStop("move_" + state);
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
* Returns the button over which the mouse currently resides
*
* @returns string
* @access public
*/
mcListItemClass.prototype.getMouseButton = function() 
{
	// if we are over the kids button
	if (this.kids_button.hitTest(_root._xmouse, _root._ymouse, false)) {
		return 'kids';

	// if we are over the move button
	} else if (this.move_button.hitTest(_root._xmouse, _root._ymouse, false)) {
		return 'move';

	} else {
		return '';
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

}


Object.registerClass("mcListItemID", mcListItemClass);

