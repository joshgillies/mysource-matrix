
#include "mcMsgsBarMessageClass.as"
 
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
	this.opened     = false;

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

	this.attachMovie("FScrollPaneSymbol", "scroll_pane", 2);
	this.scroll_pane.setHScroll(false);
	this.scroll_pane.setVScroll(true);
	this.scroll_pane._x = 0;
	this.scroll_pane._y = this.open_close_button._y + this.open_close_button._height;

	this.createEmptyMovieClip("scroll_content", 3);
	this.scroll_content._x = 0;
	this.scroll_content._y = 0;

	this.scroll_pane.setScrollContent(this.scroll_content);
	this.scroll_pane.refreshPane();

	// Set ourselves up as a listener for any external calls
	_root.external_call.addListener(this);

	// Set ourselves up as a broadcaster for stage refreshes
    ASBroadcaster.initialize(this);

}// end constructor

// Make it inherit from MovieClip
mcMsgsBarClass.prototype = new MovieClip();

/**
* Returns the height of the msgs bar in it"s current state
* needed because even when the msgs text box is empty it takes up space
*/
mcMsgsBarClass.prototype.height = function()
{
	return (this.opened) ? _root.MSG_BAR_HEIGHT : this.open_close_button._height;
}

/**
* Changes the state of the msgs bar (either Open or Close)
*/
mcMsgsBarClass.prototype.changeState = function()
{
	if (this.opened) {
		this._setSize(this._width, this.open_close_button._height);
		this.scroll_pane._visible = false;
		this.scroll_pane.refreshPane();
		this.open_close_button.gotoAndStop("open");
		this.opened = false;
		this.broadcastMessage("onMsgsBarClose");
	} else {
		this._setSize(this._width, _root.MSG_BAR_HEIGHT);
		this.scroll_pane._visible = true;
		this.scroll_pane.refreshPane();
		this.open_close_button.gotoAndStop("close");
		this.opened = true;
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
	this.scroll_pane.setSize(w, h - this.open_close_button._height);

	for(var i = 0; i < this.msgs.length; i++) {
		this.scroll_content[this.msgs[i]].setWidth(this.scroll_pane.getInnerPaneWidth());
	}

}// _setSize()

/**
* Adds a message to the list
*
* @param Array	msgs	array of Object(type => '', text => '')
*
*/
mcMsgsBarClass.prototype.addMessages = function(msgs)
{
	var scroll_pos = null;
	var open_bar   = false;

	for(var i = 0; i < msgs.length; i++) {

		var ypos = 0; 
		if (this.msgs.length) {
			var last_msg = this.scroll_content[this.msgs[this.msgs.length - 1]];
			ypos = last_msg._y + last_msg._height;
		}
		var name = 'msg_' + this.msgs.length;

		this.scroll_content.attachMovie("mcMsgsBarMessageID", name, this.msgs.length);
		this.scroll_content[name].setInfo(msgs[i].type, msgs[i].text);
		this.scroll_content[name]._x = 0;
		this.scroll_content[name]._y = ypos;
		this.scroll_content[name].setWidth(this.scroll_pane.getInnerPaneWidth());

		this.msgs.push(name);
		if (scroll_pos == null) scroll_pos = ypos;
		if (msgs[i].type == "error" || msgs[i].type == "warning") {
			open_bar = true;
		}

	}// end for

	if (open_bar && !this.opened) this.changeState();

	if (scroll_pos != null) {
		// refresh the scroll pane
		this.scroll_pane.refreshPane();
		this.scroll_pane.setScrollPosition(0, scroll_pos);
	}

}// addMessages()


/**
* Event fired whenever a command is made from outside the flash movie
*
* @param string	cmd		the command to perform
* @param object	params	the parameters for the command
*
* @access public
*/
mcMsgsBarClass.prototype.onExternalCall = function(cmd, params) 
{
	switch(cmd) {
		case "add_message" :
			if (params.msgs_xml == null || params.msgs_xml.length <= 0) return;
			var xml  = new XML(params.msgs_xml);

			// something buggered up with the connection
			if (xml.status != 0) {
				_root.dialog_box.show("XML Error, unable to print messages", "XML Status '" + xml.status + "'\nPlease Try Again");
				return;

			// we got an unexpected root node
			} else if (xml.firstChild.nodeName != "messages") {
				_root.dialog_box.show("XML Error, unable to print messages", "Unexpected Root XML Node '" + xml.firstChild.nodeName + '"');
				return;
			}// end if

			// everything went well, load 'em up
			var msgs = new Array();
			for (var i = 0; i < xml.firstChild.childNodes.length; i++) {
				// get a reference to the child node
				var msg_node = xml.firstChild.childNodes[i];
				trace('msg_node : ' + msg_node);
				if (msg_node.nodeName.toLowerCase() == "message") {
					msgs.push({type: msg_node.attributes.type, text: msg_node.firstChild.nodeValue}); 
				}//end if
			}//end for
			this.addMessages(msgs);

		break;
	}// end switch

}// end onExternalCall()



Object.registerClass("mcMsgsBarID", mcMsgsBarClass);
