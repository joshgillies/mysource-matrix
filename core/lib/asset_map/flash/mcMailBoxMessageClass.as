#include "mcMailBoxMessagePriorityClass.as"
#include "mcMailBoxMessageStatusClass.as"

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
	this.priority	= ''; // 1-5 priority
	this.status		= ''; // 'U' - Unread, 'R' - Read, 'D' - Deleted


	// create the text field
	this.attachMovie ('mcMailBoxMessagePriorityID', 'priority_flag', 1);
	this.attachMovie('mcMailBoxMessageStatusID', 'status_flag', 2);
	this.createTextField("subject_field",  3, 0, 0, 0, 0);
	this.createTextField("from_field",     4, 0, 0, 0, 0);
	this.createEmptyMovieClip('user_type_icon', 5); // placeholder for user type icon
	
	this.subject_field.multiline  = this.from_field.multiline  = false;		// }
	this.subject_field.wordWrap   = this.from_field.wordWrap   = false;		// } Using these 3 properties we have a text field that autosizes 
	this.subject_field.autoSize   = this.from_field.autoSize   = "left";	// } horizontally but not vertically
	this.subject_field.border     = this.from_field.border     = false;
	this.subject_field.selectable = this.from_field.selectable = false;
	this.subject_field._visible   = this.from_field._visible   = true;

	this.text_format = new TextFormat();
	this.text_format.color = 0x000000;
	this.text_format.font  = "Arial";
	this.text_format.size  = 10;

}// end constructor

// Make it inherit from MovieClip
mcMailBoxMessageClass.prototype = new MovieClip();
Object.registerClass("mcMailBoxMessageID", mcMailBoxMessageClass);

/**
* Set's the information for the message
*/
mcMailBoxMessageClass.prototype.setInfo = function(messageid, subject, from, body, sent, priority, status, from_type_code)
{
//	trace(this + "::mcMailBoxMessageClass.setInfo(" + messageid + ", " + subject + ", " + from + ", " + body + ", " + sent + ", " + priority + ", " + status + ", " + from_type_code + ")");

	if (status != "U" && status != "R" && status != "D") {
		_root.dialog_box.show("Unknown Mail Message status, Messageid #" + messageid + ", setting to unread");
		status = "U";
	}// end if

	this.messageid		= messageid;
	this.subject		= subject;
	this.from			= from;
	this.from_type_code	= from_type_code;
	this.body			= body;
	this.sent			= sent;
	this.priority		= priority;
	this.status			= status;

	this.priority_flag.setPriority(this.priority);
	this.status_flag.setStatus(this.status);
	
	this.subject_field.text		= this.subject;
	this.from_field.text		= this.from;
	
	this.user_type_icon.icon.removeMovieClip();
	if (!this.user_type_icon.attachMovie('mc_asset_type_' + this.from_type_code + '_icon', 'icon', 1))
		this.user_type_icon.attachMovie('mc_asset_type_default_icon', 'icon', 1);
	
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

	this.clear();

	this.subject_field._y = this.from_field_y = this.user_type_icon._y = this.status_flag._y = this.priority_flag._y = 0;
	var baseHeight = this.subject_field._height + 6;
	set_background_box(this, w, baseHeight, 0x000000, 0);

	this.priority_flag._x	= 3;
	this.priority_flag._y	= (baseHeight - this.priority_flag._height) / 2;
	
	this.status_flag._x = this.priority_flag._x + this.priority_flag._width;
	this.status_flag._y = (baseHeight - this.status_flag._height) / 2;

	this.subject_field._x	= subject_pos;
	this.subject_field._y	= (baseHeight - this.subject_field._height) / 2;

	from_pos = Math.max(from_pos, subject_pos + this.subject_field._width);

	this.user_type_icon._x	= from_pos;
	this.user_type_icon._y	= (baseHeight - this.user_type_icon._height) / 2;

	this.from_field._x		= from_pos + this.user_type_icon._width;
	this.from_field._y		= (baseHeight - this.from_field._height) / 2;

	var ypos = this.subject_field._y + this.subject_field._height + 3;

	this.lineStyle(1, 0x000000);
	this.moveTo(0, baseHeight);
	this.lineTo(w, baseHeight);

}// setWidth()

mcMailBoxMessageClass.prototype.getFlagsColumnWidth = function() {
	return this.priority_flag._width + this.status_flag._width;
}

mcMailBoxMessageClass.prototype.getSubjectColumnWidth = function() {
	return this.subject_field._width;
}

mcMailBoxMessageClass.prototype.getFromColumnWidth = function() {
	return this.user_type_icon._width + this.subject_field._width;
}

/**
* Called when this item has been pressed and then released
*/
mcMailBoxMessageClass.prototype.onRelease = function()
{
	trace("MAIL Message Released : " + this.subject);

	var str = "From : " + this.from + "\n"
			+ "Date : " + this.sent + "\n\n"
			+ this.body;
	_root.dialog_box.show(this.subject, str);

	return true;
}// end onRelease()


