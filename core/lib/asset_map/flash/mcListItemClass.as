
// Create the Class
function mcListItemClass()
{
	this.assetid   = "";
	this.type_code = "";
	this.item_text = "";
	this.pos       = 0;
	this.indent    = 0;
	this.button_pressed = '';

	// Create the Plus Minus Button
	this.attachMovie("mcPlusMinusID", "kids_button", 2);
	this.kids_button._x = 3;
	this.kids_button._y = 3;
	this.kids_button.onPress = new function () { trace('Kids Pressed'); }

}

// Make is inherit from MovieClip
mcListItemClass.prototype = new MovieClip();

mcListItemClass.prototype.setInfo = function(asset) 
{
//	trace('Set Assetid   : ' + asset.assetid);
//	trace('Set type_code : ' + asset.type_code);
//	trace('Set Text      : ' + asset.name);
	this.assetid   = asset.assetid;
	this.type_code = asset.type_code;
	this.item_text = this._name + ' ' + asset.name;

	this.kids_button.setState((asset.has_kids) ? "plus" : "none");
}

mcListItemClass.prototype.getKidState = function() 
{
	return this.kids_button.getState();
}

mcListItemClass.prototype.setKidState = function(state) 
{
	this.kids_button.setState(state);
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
	this._y = (pos * _root.LIST_ITEM_POS_INCREMENTS) + (this._parent.items_order[pos].branch_count * _root.LIST_ITEM_END_BRANCH_GAP);
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

Object.registerClass("mcListItemID", mcListItemClass);

