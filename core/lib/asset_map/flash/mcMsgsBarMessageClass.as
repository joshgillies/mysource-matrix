
// Create the Class
function mcMsgsBarMessageClass()
{
	this._x = 0;
	this._visible = true;

	this.type = ''; // error, warning, notice

	// create the text field
	this.createTextField("text_field", 2, 0, 0, 10, 10);
	this.text_field.multiline = true;	// }
	this.text_field.wordWrap  = true;	// } Using these 3 properties we have a text field that autosizes 
	this.text_field.autoSize  = "left";	// } vertically but not horizontally
	this.text_field.border     = true;
	this.text_field.selectable = false;
	this.text_field._visible   = true;

}// end constructor

// Make it inherit from MovieClip
mcMsgsBarMessageClass.prototype = new MovieClip();

/**
* Set's the information for the message
*/
mcMsgsBarMessageClass.prototype.setInfo = function(type, text)
{

	trace("Message Set Info : " + type + "  -> " + text);

	switch(type) {
		case "error" :
		case "warning" :
		case "notice" :
			this.type = type;
			this.text_field.text = text;
			break;

		default :
			_root.dialog_box.show("Unknown Message Type '" + type + "'", "Message Was :\n" + text);
			return;
			
	}// end switch

	var text_format = new TextFormat();

	switch(this.type) {
		case "error" :
			text_format.color = 0xFF0000;
			break;
		case "warning" :
			text_format.color = 0xDBA53B;
			break;
		case "notice" :
			text_format.color = 0x000000;
			break;
			
	}// end switch

//	this.text_field.setTextFormat(text_format);

}// end setInfo();

/**
* Set's the width of the msg
*/
mcMsgsBarMessageClass.prototype.setWidth = function(w)
{
	trace("SET WIDTH : " + this.text_field.text);
	// set this first, as the height will probably change
	this.text_field._width = w;
	this.clear();
	this.lineStyle(2, 0xFF0000);
	this.moveTo(5, this.text_field._y + this.text_field._height);
	this.lineTo(w, 0);
	trace("END SET WIDTH : " + this.text_field._y + ", " + this.text_field._height);

}// _setSize()

Object.registerClass("mcMsgsBarMessageID", mcMsgsBarMessageClass);
