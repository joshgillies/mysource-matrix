
// Create the Class
function mcMenuItemClass()
{
	this.text      = "";
	this.value     = "";

	this.kids = new Array();
	this.kids_under = false;

}

// Make is inherit from MovieClip
mcMenuItemClass.prototype = new MovieClip();

mcMenuItemClass.prototype.setInfo = function(text, value, kids_under) 
{
	this.text  = text;
	this.value = value;
	this.kids_under = kids_under;
}

mcMenuItemClass.prototype.show = function() 
{
	if (this.kids.length) {
		this.kids_arrow.gotoAndStop((this.kids_under) ? "down" : "right");
		this.kids_arrow._visible = true;
	} else {
		this.kids_arrow._visible = false;
	}
	this._visible = true;
}

mcMenuItemClass.prototype.hide = function() 
{
	for (var i = 0; i < this.kids.length; i++) {
		this._parent[this.kids[i]].hide();
	}// end for

	this._visible = false;
}

mcMenuItemClass.prototype.onPress = function() 
{

	if (this.kids.length) {
		trace('Show Kids :' + this.kids_under);

		var x = this._x + ((this.kids_under) ? 0 : this._width);
		var y = this._y + ((this.kids_under) ? this._height : 0);
		for (var i = 0; i < this.kids.length; i++) {
			var name = this.kids[i];
			this._parent[name]._x = x;
			this._parent[name]._y = y;
			this._parent[name].show();

			y += this._parent[name]._height;

		}// end for

	} else {
		trace('Do Stuff for ' + this.value);
	}

}

Object.registerClass("mcMenuItemID", mcMenuItemClass);
