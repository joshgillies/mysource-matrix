
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
	this.hideKids();
	this._visible = false;
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
	if (_root.system_events.inModal(this)) return false;

	// if there is a dialog box up do nothing
	if (_root.pop_up) return;

	if (this.kids.length) {
		this._parent.itemOpen(this);
		this.showKids();
	} else {
		this._parent.itemPress(this);
	}
}// end onRelease()


Object.registerClass("mcMenuItemID", mcMenuItemClass);
