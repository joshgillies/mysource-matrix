/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: mcMsgsBarMessageClass.as,v 1.6 2003/10/08 02:24:02 dwong Exp $
* $Name: not supported by cvs2svn $
*/


// Create the Class
function mcMsgsBarMessageClass()
{
	this._x = 0;
	this._visible = true;

	this.type = ''; // error, warning, notice

	// create the text field
	this.createTextField("text_field", 2, 0, 0, 0, 0);
	this.text_field.multiline	= true;		// }
	this.text_field.wordWrap	= true;		// } Using these 3 properties we have a text field that autosizes 
	this.text_field.autoSize	= "left";	// } vertically but not horizontally
	this.text_field.border		= false;
	this.text_field.selectable	= false;
	this.text_field.html		= true;
	this.text_field._visible	= true;
//	this.text_field.embedFonts	= true;

	this.createEmptyMovieClip ('bg', -1);

	this.attachMovie('mcPlusIconID', 'delete_icon', 3);
	this.delete_icon._rotation = 45;
	this.delete_icon._y = 15;
	this.delete_icon.onRelease = this.deleteItem;
	this.delete_icon._visible = false;

}// end constructor

// Make it inherit from MovieClip
mcMsgsBarMessageClass.prototype = new NestedMouseMovieClip(false, NestedMouseMovieClip.NM_ON_PRESS);


mcMsgsBarMessageClass.prototype.deleteItem = function()
{
	// this gets called from the delete icon
	var item = this._parent;
	var msgsBar = item._parent;

	msgsBar.removeMessage(item);
	
}

/**
* Returns the colour that this message should be printed in
* depends on type of message
*/
mcMsgsBarMessageClass.prototype.getColour = function()
{
	switch(this.type) {
		case "error" :
			return 0xFF3333;
			break;

		case "warning" :
			return 0xFF9999;
			break;

		case "notice" :
		default : 
			return 0x99CCFF;
			break;
			
	}// end switch

}// end getColour()

/**
* Set's the information for the message
*/
mcMsgsBarMessageClass.prototype.setInfo = function(type, text)
{
//	trace(this + ".setInfo( " + type + ", " + text + ")");

	type = type.toLowerCase();
	switch(type) {
		case "error"   :
		case "warning" :
		case "notice"  :
			this.type = type;
			this.text_field.htmlText = unescape(text);

			var text_format = new TextFormat();
			//text_format.color = this.getColour();
			text_format.font  = "Arial";
			text_format.size  = 11;
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
	this.delete_icon._x = w - this.delete_icon._width - 5;
	this.text_field._width = this.delete_icon._x;
	
	this.clear();
	this._refreshBg(w);
	this._refreshLine(w);


}// setWidth()

/**
* Set's the width of the msg
*/
mcMsgsBarMessageClass.prototype._refreshLine = function(w)
{
	var ypos = this.text_field._y + this.text_field.textHeight + 5;

	this.lineStyle(2, this.getColour());
	this.moveTo(0, ypos);
	this.lineTo(w, ypos);
}// _refreshLine()

mcMsgsBarMessageClass.prototype._refreshBg = function(w) 
{
	this.bg._x = 0;
	this.bg._y = this.text_field._y;
	set_background_box(this.bg, w, this.text_field._height, this.getColour(), 20);
}

Object.registerClass("mcMsgsBarMessageID", mcMsgsBarMessageClass);
