
// Create the Class
function mcMsgsBarMessageClass()
{
	this._x = 0;
	this._visible = true;

	this.type = ''; // error, warning, notice

	// create the text field
	this.createTextField("text_field", 2, 0, 0, 0, 0);
	this.text_field.multiline = true;	// }
	this.text_field.wordWrap  = true;	// } Using these 3 properties we have a text field that autosizes 
	this.text_field.autoSize  = "left";	// } vertically but not horizontally
	this.text_field.border     = false;
	this.text_field.selectable = false;
	this.text_field._visible   = true;

}// end constructor

// Make it inherit from MovieClip
mcMsgsBarMessageClass.prototype = new MovieClip();


/**
* Returns the colour that this message should be printed in
* depends on type of message
*/
mcMsgsBarMessageClass.prototype.getColour = function()
{
	switch(this.type) {
		case "error" :
			return 0xFF0000;
			break;

		case "warning" :
			return 0xDBA53B;
			break;

		case "notice" :
		default : 
			return 0x000000;
			break;
			
	}// end switch

}// end getColour()

/**
* Set's the information for the message
*/
mcMsgsBarMessageClass.prototype.setInfo = function(type, text)
{
	switch(type) {
		case "error"   :
		case "warning" :
		case "notice"  :
			this.type = type;
			this.text_field.text = text;

			var text_format = new TextFormat();
			text_format.color = this.getColour();
			text_format.font  = "Arial";
			text_format.size  = 10;
			this.text_field.setTextFormat(text_format);
			this._refreshLine();

			break;

		default :
			_root.dialog_box.show("Unknown Message Type '" + type + "'", "Message Was :\n" + text);
			return;
			
	}// end switch

}// end setInfo();

/**
* Set's the width of the msg
*/
mcMsgsBarMessageClass.prototype.setWidth = function(w)
{
	// set this first, as the height will probably change
	this.text_field._width = w;
	this._refreshLine();

}// setWidth()

/**
* Set's the width of the msg
*/
mcMsgsBarMessageClass.prototype._refreshLine = function()
{
	var ypos = this.text_field._y + this.text_field.textHeight + 5;
	this.clear();
	this.lineStyle(2, this.getColour());
	this.moveTo(0, ypos);
	this.lineTo(this.text_field._width, ypos);
}// _refreshLine()


Object.registerClass("mcMsgsBarMessageID", mcMsgsBarMessageClass);
