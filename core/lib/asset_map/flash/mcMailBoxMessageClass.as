
// Create the Class
function mcMailBoxMessageClass()
{
	this._x = 0;
	this._visible = true;

	this.messageid	= 0;
	this.subject	= '';
	this.from       = '';
	this.body		= '';
	this.sent		= 0;
	this.priority	= ''; // 'H' - High, 'N' - Normal, 'L' - Low
	this.status		= ''; // 'U' - Unread, 'R' - Read, 'D' - Deleted


	// create the text field
	this.createTextField("priority_field", 1, 0, 0, 0, 0);
	this.createTextField("subject_field",  2, 0, 0, 0, 0);
	this.createTextField("from_field",     3, 0, 0, 0, 0);
	this.priority_field.multiline  = this.subject_field.multiline  = this.from_field.multiline  = false;		// }
	this.priority_field.wordWrap   = this.subject_field.wordWrap   = this.from_field.wordWrap   = false;		// } Using these 3 properties we have a text field that autosizes 
	this.priority_field.autoSize   = this.subject_field.autoSize   = this.from_field.autoSize   = "left";	// } horizontally but not vertically
	this.priority_field.border     = this.subject_field.border     = this.from_field.border     = false;
	this.priority_field.selectable = this.subject_field.selectable = this.from_field.selectable = false;
	this.priority_field._visible   = this.subject_field._visible   = this.from_field._visible   = true;

	this.text_format = new TextFormat();
	this.text_format.color = 0x000000;
	this.text_format.font  = "Arial";
	this.text_format.size  = 11;

}// end constructor

// Make it inherit from MovieClip
mcMailBoxMessageClass.prototype = new MovieClip();

/**
* Set's the information for the message
*/
mcMailBoxMessageClass.prototype.setInfo = function(messageid, subject, from, body, sent, priority, status)
{

//	trace("BLAH : " + messageid + ", " + from + ", " + subject + ", " + body + ", " + sent + ", " + priority + ", " + status);

	priority = priority.toUpperCase();
	if (priority != "H" && priority != "N" && priority != "L") {
		_root.dialog_box.show("Unknown Mail Message Priority, Messageid #" + messageid + ", setting to normal");
		priority = "N";
	}// end if
	if (status != "U" && status != "R" && status != "D") {
		_root.dialog_box.show("Unknown Mail Message status, Messageid #" + messageid + ", setting to unread");
		status = "U";
	}// end if

	this.messageid	= messageid;
	this.subject	= subject;
	this.from		= from;
	this.body		= body;
	this.sent		= sent;
	this.priority	= priority;
	this.status		= status;

	this.priority_field.text	= this.priority;
	this.subject_field.text		= this.subject;
	this.from_field.text		= this.from;

	this.text_format.bold = (this.status == "U");

	this.priority_field.setTextFormat(this.text_format);
	this.subject_field.setTextFormat(this.text_format);
	this.from_field.setTextFormat(this.text_format);

}// end setInfo();

/**
* Set's the width of the msg
*/
mcMailBoxMessageClass.prototype.setWidth = function(w, subject_pos, from_pos, col_gap)
{


	if (this.subject_field.text != this.subject) {
		this.subject_field.text = this.subject;
		this.subject_field.setTextFormat(this.text_format);
	}


	// if the from field is going to overlap the subject field, adjust it's text
	if (this.subject_field._width > from_pos - subject_pos - col_gap) {
		var tmp_text = new String(this.subject);
		do {
			tmp_text = tmp_text.substr(0, tmp_text.length - 1);
			this.subject_field.text = tmp_text + "...";
			this.subject_field.setTextFormat(this.text_format);
		} while (tmp_text.length > 1 && this.subject_field._width > from_pos - subject_pos - col_gap);

	}// end if

	this.priority_field._x	= 0;
	this.subject_field._x	= subject_pos;
	this.from_field._x		= from_pos;

	var ypos = this.subject_field.textHeight + 5;
	this.clear();
	this.lineStyle(1, 0x000000);
	this.moveTo(0, ypos);
	this.lineTo(w, ypos);

}// setWidth()

/**
* Called when this item has been pressed and then released
*/
mcMailBoxMessageClass.prototype.onRelease = function()
{
	trace("MAIL Message Released : " + this.subject);

	var str = "From : " + this.from + "\n"
			+ "Date : " + this.sent + "\n"
			+ this.body;
	_root.dialog_box.show(this.subject, str);

	return true;
}// end onRelease()


Object.registerClass("mcMailBoxMessageID", mcMailBoxMessageClass);
