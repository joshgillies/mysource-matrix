/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: mcMsgsBarClass.as,v 1.11 2003/09/26 05:26:32 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


#include "mcMsgsBarMessageClass.as"

/**
* MsgsBar
*
* Holds a list of log messages
*
* NOTE: the list items in this container are just stored as
* normal attributes and not in there own array (as I would have liked)
* this is due to the attachMovie() fn not being able to accept arrays
* elements as the instance name for the new movie
*
*/

// Create the Class
function mcMsgsBarClass()
{
	this._x = 0;
	this._visible = true;

	this.bg_colour  = 0xC0C0C0;
	this.opened     = false;

	this.msgs = new Array();

	// Set ourselves up as a listener for any external calls
	// Used to add new messages
	_root.external_call.addListener(this);

}// end constructor

// Make it inherit from Nested Mouse Movements MovieClip
mcMsgsBarClass.prototype = new NestedMouseMovieClip(false, NestedMouseMovieClip.NM_ON_PRESS);

/**
* Refreshes the msg box display, called when the tabs is resized
*
*/
mcMsgsBarClass.prototype.refresh = function()
{
	var ypos = 0; 
	var width = this._parent.scroll_pane.getInnerPaneWidth();
	for(var i = 0; i < this.msgs.length; i++) {
		var msg = this[this.msgs[i]];
		msg._y = ypos;
		msg.setWidth(width);
		ypos = msg._y + msg._height - 1;
	}

	this._parent.scroll_pane.refreshPane();
}


/**
* Adds a message to the list
*
* @param Array	msgs	array of Object(type => '', text => '')
*
*/
mcMsgsBarClass.prototype.addMessages = function(msgs)
{
	var scroll_pos = null;
	var open_tab   = false;

	for(var i = 0; i < msgs.length; i++) {

		var ypos = 0; 
		if (this.msgs.length) {
			var last_msg = this[this.msgs[this.msgs.length - 1]];
			ypos = last_msg._y + last_msg._height - 1;
		}
		var name = 'msg_' + this.msgs.length;

		this.attachMovie("mcMsgsBarMessageID", name, this.msgs.length);
		var msg = this[name];
		msg.setInfo(msgs[i].type, msgs[i].text);
		msg._x = 0;
		msg._y = ypos;
		msg.setWidth(this._parent.scroll_pane.getInnerPaneWidth());
		msg.name = name;

		this.msgs.push(name);

		if (scroll_pos == null) scroll_pos = ypos;

		if (msg.type == "error" || msg.type == "warning") {
			open_tab = true;
		}

	}// end for

	if (open_tab) this._parent.openTab();

	if (scroll_pos != null) {
		// refresh the scroll pane
		this._parent.scroll_pane.refreshPane();
		this._parent.scroll_pane.setScrollPosition(0, scroll_pos);
	}

}// addMessages()

/**
* Removes a message from the list
*/
mcMsgsBarClass.prototype.removeMessage = function(mc)
{
	var i = this.msgs.search(mc.name);
	this.msgs.splice(i, 1);
	mc.removeMovieClip();
	this.refresh();
}

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
//			trace(xml);
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
				if (msg_node.nodeName.toLowerCase() == "message") {
					msgs.push({type: msg_node.attributes.type, text: msg_node.firstChild.nodeValue}); 
				}//end if
			}//end for
			this.addMessages(msgs);

		break;
	}// end switch

}// end onExternalCall()


Object.registerClass("mcMsgsBarID", mcMsgsBarClass);

