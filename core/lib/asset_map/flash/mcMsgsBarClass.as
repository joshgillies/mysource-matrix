 /////////////////////////////////////////////////////////////////////////////
// NOTE: the list items in this container are just stored as
// normal attributes and not in there own array (as I would have liked)
// this is due to the attachMovie() fn not being able to accept arrays
// elements as the instance name for the new movie


// Create the Class
function mcMsgsBarClass()
{
	this._x = 0;
	this._visible = true;

	this.bg_colour  = 0xFF0000; //0xC0C0C0;

	this.msgs = new Array();

	// Create the Plus Minus Button
	this.attachMovie("mcMsgsBarOpenCloseID", "open_close_button", 1);
	this.open_close_button.gotoAndStop('open');
	this.open_close_button._x = 0;
	this.open_close_button._y = 0;
	this.open_close_button._visible = true;

	// Set ourselves up as a listener on the menu, so we know when an item is pressed
    // _root..addListener(this);

	// Set ourselves up as a broadcaster for stage refreshes
    ASBroadcaster.initialize(this);

}// end constructor

// Make it inherit from MovieClip
mcMsgsBarClass.prototype = new MovieClip();

/**
* Open the Bar Up to show messages
*/
mcMsgsBarClass.prototype.open = function()
{
//	this._setSize(this._width, _root.MSG_BAR_HEIGHT);
	this.broadcastMessage("onMsgBarOpen");
}

/**
* Hide the Bar
*/
mcMsgsBarClass.prototype.close = function()
{
//	this._setSize(this._width, this.open_close_button._height);
	this.broadcastMessage("onMsgBarClose");
}

/**
* Set's the width of the msg bar
*/
mcMsgsBarClass.prototype.setWidth = function(w)
{
	this._setSize(w, this._height);
	this.open_close_button._x = Math.round((this._width / 2) - (this.open_close_button._width / 2));
}

/**
* Set's the size of this box by creating a background image of that size
*/
mcMsgsBarClass.prototype._setSize = function(w, h)
{
	trace('Set Size  : ' + w + ', ' + h);
	// We need to display the open/close button
	if (h < this.open_close_button._height) h = this.open_close_button._height;
	trace('Set Size  : ' + w + ', ' + h);
	this.clear();
	this.beginFill(this.bg_colour, 100);
	this.lineStyle();
	this.moveTo(0, 0);
	this.lineTo(w, 0);
	this.lineTo(w, h);
	this.lineTo(0, h);
	this.lineTo(0, 0);
	this.endFill();
	trace('Curr Size : ' + this._width + ', ' + this._height);
	trace('');
}

Object.registerClass("mcMsgsBarID", mcMsgsBarClass);
