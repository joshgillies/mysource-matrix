/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: mcMenuItemClass.as,v 1.15 2003/10/08 02:24:02 dwong Exp $
* $Name: not supported by cvs2svn $
*/

function mcMenuItemClass() 
{
	this._value = null;
	this._iconID = "";
	this._itemWidth = 0;

	this._baseTextFormat = new TextFormat();
	this._baseTextFormat.font = 'Arial';
	this._baseTextFormat.size = 9;
	this._baseTextFormat.color = 0x000000;

	
	this._rollOverColour = 0x50607F;
	this._rollOverTextFormat = new TextFormat();
	this._rollOverTextFormat.color = 0xffffff;

	this.onRollOver = this._drawHighlight;
	this.onRollOut = this.onReleaseOutside = this._clearHighlight;

}

mcMenuItemClass.prototype = new MovieClip();
Object.registerClass ('mcMenuItemID', mcMenuItemClass);


// public methods

mcMenuItemClass.prototype.onPress = function() {
	if (this._parent._head == this) {
		this._parent.swapDepths(100); 
	}
	return true;
}

mcMenuItemClass.prototype.onRelease = function() {
	trace (this + "::mcMenuitemClass.onRelease()");

// check if something else is modal
	if (_root.system_events.inModal(this)) return true;

	if (this._action != null) {
		//this._parent.getRootContainer().hideChildMenus();
//		trace ("action : " + this._action);
		this._action();
		this.getRootContainer().hideChildMenus();
	}
	return true;
}

mcMenuItemClass.prototype._drawHighlight = function()
{
//	trace (this + '::mcMenuItemClass.onRollOver()');
	if (this.frozen) 
		return;

	this._textBox.setTextFormat (this._rollOverTextFormat);
	this._drawBackground(this.getWidth(), this.getHeight());
	return true;
}

mcMenuItemClass.prototype._clearHighlight = function() 
{
	if (this.frozen)
		return;

	this._textBox.setTextFormat (this._baseTextFormat);
	this._drawBackground(this.getWidth(), this.getHeight(), 0);
	return true;
}

mcMenuItemClass.prototype.freezeHighlight = function()
{
//	trace (this + ":freeze");
	this.frozen = true;
}

mcMenuItemClass.prototype.unfreezeHighlight = function()
{
//	trace (this + ":unfreeze");
	this.frozen = false;
}

mcMenuItemClass.prototype.create = function(container, label, iconID, value, action, showArrow) 
{
//	trace  (this + "::mcMenuItem.create(" + container + ", " + label + ", " + iconID + ", " + value + ", " + action + ", " + showArrow + ")");
	// private variables
	this._container = container;
	this._iconID = iconID;
	this._value = value;
	this._action = action;

	// child movie clips

	// background
	this.createEmptyMovieClip ('_bg', 1);
	// icon
	if (iconID != null) {
		this.attachMovie(iconID, '_icon', 2);
		if (typeof this._icon != "movieclip") {
			this.attachMovie("mc_asset_type_default_icon", '_icon', 2);
		}
	} else {
		this._icon = null;
	}

	// label
	this.createTextField('_textBox', 3, 0, 0, 10, 10);
	this._initTextField(label);

	// arrow
	this.attachMovie('mcMenuItemArrowID', '_arrow', 4);
	if (showArrow == null || !showArrow)
		this._arrow._alpha = 0;

	if (this._container.isRoot())
		this._arrow.gotoAndStop ('down');

	this._refresh();
}

mcMenuItemClass.prototype.isMenu = function() 
{
	return false;
}

mcMenuItemClass.prototype.getParentMenu = function() 
{
	var containerParent = this._container._parent;

	if (containerParent instanceof mcMenuClass)
		return containerParent;
	else
		return null;
}

mcMenuItemClass.prototype.getRootContainer = function()
{
	var container = this._container;
	var menu = container._parent;
	while (menu instanceof mcMenuClass) {
		container = menu._container;
		menu = container._parent;
	}
	return container;
}

mcMenuItemClass.prototype.getWidth = function() {
	// allow room for padding
	if (this._arrow._visible)
		return this._arrow._x + this._arrow._width + 5;
	else
		return this._width;
}

mcMenuItemClass.prototype.getHeight = function() {
	return this._height;
}

mcMenuItemClass.prototype.setItemWidth = function(width) {
	this._itemWidth = width;
	this._refresh();
}

// private methods

mcMenuItemClass.prototype._initTextField = function(label) 
{
	this._textBox.setNewTextFormat(this._baseTextFormat);
	this._textBox.autoSize = 'left';
	this._textBox.text = label;
}

mcMenuItemClass.prototype._refresh = function() 
{
	var padding = 3;
	
	// x - direction
	var nextX = padding;
	this._bg._x = nextX;

	if (this._icon != null) {
		this._icon._x = nextX;
	}
	nextX = 25;

	this._textBox._x = nextX;
	nextX += this._textBox._width + padding;

	if (nextX + this._arrow._width > this._itemWidth) {
		this._arrow._x = nextX;
		nextX += this._arrow._width;
	} else {
		this._arrow._x = this._itemWidth - this._arrow._width - padding;
		nextX = this._arrow._x + this._arrow._width;
	}

	this._icon._y = 0;
	this._arrow._y = 0;
	this._textBox._y = 0;

	this._bg.clear();
	this._drawBackground(nextX, this._height + 2 * padding, 0);

	this._icon._y = (this._height - this._icon._height) / 2;
	this._arrow._y = (this._height - this._arrow._height) / 2;
	this._textBox._y = (this._height - this._textBox._height) / 2;

}

mcMenuItemClass.prototype._drawBackground = function(width, height, alpha)
{
	if (alpha == null)
		alpha = 100;

	with (this._bg) {
		clear();
		beginFill (0, alpha);
		moveTo (1, 0);
		lineTo (width, 0);
		lineTo (width, height);
		lineTo (1, height);
		lineTo (1, 0);
		endFill();
	}
}
