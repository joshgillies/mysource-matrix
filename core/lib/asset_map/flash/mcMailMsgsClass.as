
#include "mcMailMsgsMessageClass.as"

/**
* MailMsgs
*
* Holds the the mail messages
*
* NOTE: the list items in this container are just stored as
* normal attributes and not in there own array (as I would have liked)
* this is due to the attachMovie() fn not being able to accept arrays
* elements as the instance name for the new movie
*
*/

// Create the Class
function mcMailMsgsClass()
{
	this._x = 0;
	this._y = 0;
	this._visible = true;

	this.bg_colour = 0xC0C0C0;

	this.col_gap = 10; // gap between columns

	this.msgs = new Array();

	this.max_widths = {priority: 0, subject: 0, from: 0};

	// Set ourselves up as a listener for any external calls
	// Used to refresh the message list
	_root.external_call.addListener(this);

}// end constructor

// Make it inherit from Nested Mouse Movements MovieClip
mcMailMsgsClass.prototype = new NestedMouseMovieClip(false, NestedMouseMovieClip.NM_ON_PRESS);

/**
* Refreshes the msg box display, called when the tabs is resized
*
*/
mcMailMsgsClass.prototype.refresh = function()
{

	var w = this._parent.scroll_pane.getInnerPaneWidth();

	var max_width = this.max_widths.priority + this.col_gap + 
					this.max_widths.subject  + this.col_gap + 
					this.max_widths.from;

	var from_pos = 0;
	var subject_pos = this.max_widths.priority + this.col_gap;
	// if the width isn't wide enough to accomodate the widest entries then
	// the from pos is moved over the subject
	if (max_width > w) {
		from_pos = w - this.max_widths.from;

	// otherwise just place after longest subject
	} else {
		from_pos = subject_pos + this.col_gap + this.max_widths.subject;
	}

	for(var i = 0; i < this.msgs.length; i++) {
		this[this.msgs[i]].setWidth(w, subject_pos, from_pos, this.col_gap);
	}
}


/**
* Refreshes the mail list
*
*/
mcMailMsgsClass.prototype.refreshMail = function()
{
	var xml = new XML();
	var cmd_elem = xml.createElement("command");
	cmd_elem.attributes.action = "get mail";
	xml.appendChild(cmd_elem);

	// start the loading process
	var exec_indentifier = _root.server_exec.init_exec(xml, this, "loadMailFromXML", "mail");
	_root.server_exec.exec(exec_indentifier, "Loading Mail");

}


/**
* Called after the XML has been loaded
*
* @param object XML xml   the xml object that contain the information that we need
*
*/
mcMailMsgsClass.prototype.loadMailFromXML = function(xml, exec_indentifier)
{

	var mc_name = null;
	while ((mc_name = this.msgs.pop()) !== undefined) {
		if (this[mc_name]) this[mc_name].removeMovieClip();
	}// end for

	var ypos = 0;

	for (var i = 0; i < xml.firstChild.childNodes.length; i++) {
		// get a reference to the child node
		var msg_node = xml.firstChild.childNodes[i];
		if (msg_node.nodeName.toLowerCase() != "message") continue;

		var mc_name = "msg_" + msg_node.attributes.messageid;
		this.msgs.push(mc_name);

		this.attachMovie("mcMailMsgsMessageId", mc_name, this.msgs.length);
		this[mc_name].setInfo(	msg_node.attributes.messageid,
								msg_node.attributes.userfrom,
								msg_node.firstChild.firstChild.nodeValue, // subject
								msg_node.lastChild.firstChild.nodeValue,  // body
								msg_node.attributes.sent,
								msg_node.attributes.priority,
								msg_node.attributes.status
								);
		this[mc_name]._x = 0;
		this[mc_name]._y = ypos;
		this[mc_name]._visible = true;

		if (this.max_widths.priority < this[mc_name].priority_field._width)	this.max_widths.priority = this[mc_name].priority_field._width;
		if (this.max_widths.subject	< this[mc_name].subject_field._width)	this.max_widths.subject	= this[mc_name].subject_field._width;
		if (this.max_widths.from		< this[mc_name].from_field._width)		this.max_widths.from		= this[mc_name].from_field._width;

		ypos += this[mc_name]._height;

	}//end for

	this.refresh();

	// force a refresh of the scroller
	this._parent.scroll_pane.refreshPane();

}// end loadMailFromXML()

/**
* Event fired whenever a command is made from outside the flash movie
*
* @param string	cmd		the command to perform
* @param object	params	the parameters for the command
*
* @access public
*/
mcMailMsgsClass.prototype.onExternalCall = function(cmd, params)
{
	switch(cmd) {
		case "refresh_mail" :
			this._parent.openTab();
			this.refreshMail();
		break;
	}// end switch

}// end onExternalCall()


Object.registerClass("mcMailMsgsID", mcMailMsgsClass);

