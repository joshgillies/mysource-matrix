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

	this.bg_colour  = 0xC0C0C0;
	this.state      = "closed"; // "open" or "close"

	this.msgs = new Array();

	// Create the Plus Minus Button
	this.attachMovie("mcMsgsBarOpenCloseID", "open_close_button", 1);
	this.open_close_button.gotoAndStop("open");
	this.open_close_button._x = 0;
	this.open_close_button._y = 0;
	this.open_close_button._visible = true;
	this.open_close_button.onPress = function () {
		this._parent.changeState();
	};

	trace("MSGS HEIGHT : " + this._height);

//	this.attachMovie("FScrollPaneSymbol", "scroller", 2);
//	this.scroller.setHScroll(false);
//	this.scroller.setVScroll(true);
//	this.scroller._x = 0;
//	this.scroller._y = this.open_close_button._y + this.open_close_button._height;

	this.createEmptyMovieClip("scroll_content", 3);
	this.scroll_content._x = 0;
	this.scroll_content._y = this.open_close_button._y + this.open_close_button._height;

	trace("SCROLL CONTENT" + _root.msgs_bar.scroll_content._x + ", " + _root.msgs_bar.scroll_content._y + " \t :\t" + (_root.msgs_bar.scroll_content._x + _root.msgs_bar.scroll_content._width) + ", " + (_root.msgs_bar.scroll_content._y + _root.msgs_bar.scroll_content._height));

//	this.scroller.setScrollContent(this.scroll_content);

	// Set ourselves up as a broadcaster for stage refreshes
    ASBroadcaster.initialize(this);

}// end constructor

// Make it inherit from MovieClip
mcMsgsBarClass.prototype = new MovieClip();

/**
* Are we currently open ?
*/
mcMsgsBarClass.prototype.open = function()
{
	return (this.state == "open");
}
/**
* Are we currently closed ?
*/
mcMsgsBarClass.prototype.closed = function()
{
	return (this.state != "open");
}

/**
* Returns the height of the msgs bar in it"s current state
* needed because even when the msgs text box is empty it takes up space
*/
mcMsgsBarClass.prototype.height = function()
{
	return (this.open()) ? _root.MSG_BAR_HEIGHT : this.open_close_button._height;
}

/**
* Changes the state of the msgs bar (either Open or Close
*/
mcMsgsBarClass.prototype.changeState = function()
{
	trace("CHANGE STATE : " + this.state);
	if (this.open()) {
		this._setSize(this._width, this.open_close_button._height);
		this.scroller._visible = false;
		this.open_close_button.gotoAndStop("open");
		this.state = "closed";
		this.broadcastMessage("onMsgsBarClose");
	} else {
		this._setSize(this._width, _root.MSG_BAR_HEIGHT);
		this.scroller._visible = true;
		this.open_close_button.gotoAndStop("close");
		this.state = "open";
		this.broadcastMessage("onMsgsBarOpen");
	}
}

/**
* Set"s the width of the msg bar
*/
mcMsgsBarClass.prototype.setWidth = function(w)
{
	this._setSize(w, this.height());
}

/**
* Set"s the size of this box by creating a background image of that size
*/
mcMsgsBarClass.prototype._setSize = function(w, h)
{
	// We need to always show the open/close button
	if (h < this.open_close_button._height) h = this.open_close_button._height;
	this.clear();
	this.beginFill(this.bg_colour, 100);
	// This is commented out because when we try and explicitly set it, 
	// an extra 2 pixels gets added to the width of the MC for no f!@#$ing reason
	//this.lineStyle();
	this.moveTo(0, 0);
	this.lineTo(w, 0);
	this.lineTo(w, h);
	this.lineTo(0, h);
	this.lineTo(0, 0);
	this.endFill();

	this.open_close_button._x = Math.round((w / 2) - (this.open_close_button._width / 2));
	this.scroller.setSize(w, h - this.open_close_button._height);

	for(var i = 0; i < this.msgs.length; i++) {
		this.scroll_content[this.msgs[i]].setWidth(this.scroller.getPaneWidth());
	}

}// _setSize()

/**
* Adds a message to the list
*/
mcMsgsBarClass.prototype.addMessage = function(type, text)
{
	trace("Add Message : " + type + "  -> " + text);
	var name = 'msg_' + this.msgs.length;
	this.scroll_content.attachMovie("mcMsgsBarMessageID", name, this.msgs.length);
	this.scroll_content[name].setInfo(type, text);
	this.scroll_content[name].setWidth(this.scroller.getPaneWidth());
	this.scroll_content[name]._x = 0;
	this.scroll_content[name]._y = 0;

	w = 10;
	h = 10;
	this.scroll_content.clear();
	this.scroll_content.beginFill(0x00ff00, 100);
	// This is commented out because when we try and explicitly set it, 
	// an extra 2 pixels gets added to the width of the MC for no f!@#$ing reason
	//this.scroll_content.lineStyle();
	this.scroll_content.moveTo(0, 0);
	this.scroll_content.lineTo(w, 0);
	this.scroll_content.lineTo(w, h);
	this.scroll_content.lineTo(0, h);
	this.scroll_content.lineTo(0, 0);
	this.scroll_content.endFill();


	this.msgs.push(name);
}// addMessage()


Object.registerClass("mcMsgsBarID", mcMsgsBarClass);
