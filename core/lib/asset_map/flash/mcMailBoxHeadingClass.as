

// Create the Class
function mcMailBoxHeadingClass()
{

	// create the text field
	this.createTextField("label_text", 1, 0, 0, 0, 0);
	this.label_text.multiline	= false;	
	this.label_text.wordWrap	= false;	
	this.label_text.autoSize	= "left";	
	this.label_text.border		= false;
	this.label_text.selectable	= false;
	this.label_text._visible	= true;
	this.label_text.text		= "Go to Inbox";

	var text_format = new TextFormat();
	text_format.align = "center";
	text_format.font  = "Arial";
	text_format.size  = 12;
	text_format.color = 0xffffff;

	this.label_text.setTextFormat(text_format); 

}

// Make it inherit from MovieClip
mcMailBoxHeadingClass.prototype = new MovieClip();
Object.registerClass("mcMailBoxHeadingID", mcMailBoxHeadingClass);

/**
* Set the width of the menu
*
* @param int	w	the width of the tabs
*
*/
mcMailBoxHeadingClass.prototype.setWidth = function(w)
{
	this.label_text._x = (w - this.label_text._width) / 2;
	set_background_box(this, w, this.label_text._height, 0x000000, 100);
}// setWidth()

/**
* Set the width of the menu
*
* @param int	w	the width of the tabs
*
*/
mcMailBoxHeadingClass.prototype.onPress = function(w)
{
//	trace("getURL(" + _root.inbox_path + ", " + _root.url_frame + ");");
	getURL(_root.inbox_path, _root.url_frame);
}// setWidth()


