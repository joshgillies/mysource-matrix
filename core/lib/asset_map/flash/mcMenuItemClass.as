
// Create the Class
function mcMenuItemClass()
{
	this.text  = "";
	this.value = "";
	this.depth = 0;

	this.kids = new Array();

}

// Make it inherit from MovieClip
mcMenuItemClass.prototype = new MovieClip();

mcMenuItemClass.prototype.setInfo = function(text, value, depth) 
{
	this.text  = text;
	this.value = value;
	this.depth = depth;
}

mcMenuItemClass.prototype.show = function() 
{
	if (this.kids.length) {
		this.kids_arrow.gotoAndStop((this.depth == 0) ? "down" : "right");
		this.kids_arrow._visible = true;
	} else {
		this.kids_arrow._visible = false;
	}
	this._visible = true;
}

mcMenuItemClass.prototype.hide = function() 
{
	if (!this._visible) return;
	this.hideKids();
	this._visible = false;
	this._x = 0; // } Move back to 0,0 so that we aren't collected by the 
	this._y = 0; // } nestedMouseMovieClip when it's checking where stuff is
}

mcMenuItemClass.prototype.showKids = function() 
{
	var x = this._x + ((this.depth == 0) ? 0 : this._width);
	var y = this._y + ((this.depth == 0) ? this._height : 0);
	for (var i = 0; i < this.kids.length; i++) {
		var name = this.kids[i];
		this._parent[name]._x = x;
		this._parent[name]._y = y;
		this._parent[name].show();

		y += this._parent[name]._height;

	}// end for

}

mcMenuItemClass.prototype.hideKids = function() 
{
	for (var i = 0; i < this.kids.length; i++) {
		this._parent[this.kids[i]].hide();
	}// end for
}

mcMenuItemClass.prototype.onRelease = function() 
{
	_root.system_events.screenPress(this);

	// check if something else is modal
	if (_root.system_events.inModal(this)) return true;

	if (this.kids.length) {
		this._parent.itemOpen(this);
		this.showKids();
	} else {
		this._parent.itemPress(this);
	}

	return true;

}// end onRelease()


Object.registerClass("mcMenuItemID", mcMenuItemClass);
