/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: mcMailBoxHeadingClass.as,v 1.6 2003/11/26 00:51:12 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/



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


