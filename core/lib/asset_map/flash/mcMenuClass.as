function mcMenuClass() 
{
	//trace (this + "::mcMenuClass()");
	this._value = null;
	this._iconID = "";
	this._itemWidth = 0;

	this._rollOverColour = 0xd00000;
	
	this.attachMovie ('mcMenuItemID', '_head', 1);
	
	this.attachMovie ('mcMenuContainerID', '_childContainer', 2);
	this._childContainer._visible = false;
	this._childContainer._isVertical = true;
	//trace ("childContainer: " + this._childContainer);
}

mcMenuClass.prototype = new NestedMouseMovieClip(true, NestedMouseMovieClip.NM_ON_PRESS);
Object.registerClass ('mcMenuID', mcMenuClass);

mcMenuClass.prototype.create = function(container, label, iconID) 
{

	//trace (this + "::mcMenuClass.create(" + container + ", " + label + ", " + iconID + ")");
	//trace ("childContainer: " + this._childContainer);
	this._container = container;
	//trace (this.__headReleaseFn);
	this._head.create (this._container, label, iconID, null, null, true);
	this._head.onRelease = this.__headReleaseFn;
	this._refresh();
}

mcMenuClass.prototype.isMenu = function() 
{
	return true;
}


mcMenuClass.prototype.getWidth = function()
{
	return this._head.getWidth();
}

mcMenuClass.prototype.getHeight = function()
{
	return this._head.getHeight();
}

mcMenuClass.prototype.getHead = function()
{
	return this._head;
}

mcMenuClass.prototype.getContainer = function()
{
	//trace (this + "::mcMenuClass.getContainer()");
	//trace (this._childContainer);
	return this._childContainer;
}

mcMenuClass.prototype._refresh = function()
{
	//trace (this + '::mcMenuClass._refresh()');

	this._head._refresh();
	this._childContainer._refresh();
	//trace ("is container root?: " + this._container.isRoot());
}

mcMenuClass.prototype._resetContainerPosition = function() {

	var overlapAmount = 15;

	if (this._container.isRoot()) {
		// child container is below
		this._childContainer._x = 0;
		//trace ("head._getHeight = " + this._head.getHeight());
		this._childContainer._y = this._head.getHeight();
	} else {
/*
// child container is to the right - but not if it goes over the edge!
		var right = new Object();
		right.x = this._head._x + this._head.getWidth() + this._childContainer._width - overlapAmount;
		right.y = 0;
		trace ("this: " + this);
		trace ("head width: " + this._head.getWidth());
		trace ("child container width : " + this._childContainer._width);

		trace ("local right : " + right.x);
		localToGlobal(right);
		trace (getBounds (_root).xMax);
		
		trace ("right global / stage width : " + right.x + "/" + Stage.width);

		if (right.x > Stage.width) {
			trace ("too large  - going down instead!");
			// down and slightly indented
			right.x = Stage.width;
			globalToLocal(right);

			this._childContainer._x = Math.min (overlapAmount, right.x - this._childContainer._width);
			this._childContainer._y = this._head.getHeight();
		} else {
			// to the right
			trace ("OK");
			this._childContainer._x = this._head.getWidth() - overlapAmount;
			this._childContainer._y = 0;

		}
*/
			this._childContainer._x = overlapAmount;
			this._childContainer._y = this._head.getHeight();
		
	}
}

mcMenuClass.prototype.showChildren = function()
{
//	trace (this + '::mcMenuClass.showChildren()');
	this._childContainer._xscale = this._childContainer._yscale = 100;
	this._childContainer._visible = true;
	this._head.freezeHighlight();
//	trace ("child container: " + this._childContainer);
//	trace ("dimensions: " + this._childContainer._width + " x " + this._childContainer._height);
//	trace ("location: (" + this._childContainer._x + ", " + this._childContainer._y + ")");
//	trace ("visibility: " + this._childContainer._visible);
	this._resetContainerPosition();
}

mcMenuClass.prototype.hideChildren = function()
{
//	trace (this + '::mcMenuClass.hideChildren()');
	this._childContainer._xscale = this._childContainer._yscale = 0;
	this._childContainer._visible = false;
	this._head.unfreezeHighlight();
//	trace ("child container: " + this._childContainer);
//	trace ("dimensions: " + this._childContainer._width + " x " + this._childContainer._height);
//	trace ("location: (" + this._childContainer._x + ", " + this._childContainer._y + ")");
//	trace ("visibility: " + this._childContainer._visible);
	this._childContainer.hideChildMenus();
}

mcMenuClass.prototype.__headReleaseFn = function() 
{
//	trace (this + "::mcMenuClass.__headReleaseFn()");

//	trace ("this._parent._childContainer : " + this._parent._childContainer);
//	trace ("this._parent._childContainer._visible : " + this._parent._childContainer._visible);
//	trace ("this._container : " + this._container);

	if (!this._parent._childContainer._visible) {
		this._container.hideChildMenus();
		this._parent.showChildren();
	} else {
		this._parent.hideChildren();
	}
}

mcMenuClass.prototype.getRootContainer = function()
{
	var container = this._container;
	var menu = container._parent;
	while (menu instanceof mcMenuClass) {
		container = menu._container;
		menu = container._parent;
	}
	//trace (container);
	return container;
}

mcMenuClass.prototype.setItemWidth = function(width) {
	this._head.setItemWidth(width);
	this._refresh();
}

