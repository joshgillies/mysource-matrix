/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
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
* $Id: mcAssetFinderHeading.as,v 1.8 2003/11/18 15:37:35 brobertson Exp $
* $Name: not supported by cvs2svn $
*/



// Create the Class
function mcAssetFinderHeadingClass()
{
	this.createEmptyMovieClip("cancel", 2);
	this.cancel._x = 0;
	this.cancel._y = 0;

	// create the text field
	this.cancel.createTextField("label_text", 1, 0, 0, 0, 0);
	this.cancel.label_text.multiline = false;	// }
	this.cancel.label_text.wordWrap  = false;	// } Using these 3 properties we have a text field that autosizes 
	this.cancel.label_text.autoSize  = "left";	// } horizontally but not vertically
	this.cancel.label_text.border     = false;
	this.cancel.label_text.selectable = false;
	this.cancel.label_text._visible   = true;
	this.cancel.label_text.text = "Cancel";

	var text_format = new TextFormat();
	text_format.font  = "Arial";
	text_format.size  = 10;

	this.cancel.label_text.setTextFormat(text_format); 

	// set it up as a button
	this.cancel.onRelease = function () { 
		this._parent._parent.cancelAssetFinder(); 
	};


//	this.cancel.onRollOver = function () { 
//		trace("Roll Over");
//		var text_format = this.label_text.getTextFormat();
//		text_format.underline = true;
//		this.label_text.setTextFormat(text_format); 
//	};
//	this.cancel.onRollOut = function () { 
//		var text_format = this.label_text.getTextFormat();
//		text_format.underline = false;
//		this.label_text.setTextFormat(text_format); 
//	};

}

// Make it inherit from Nested Mouse Movements MovieClip
mcAssetFinderHeadingClass.prototype = new NestedMouseMovieClip(true, NestedMouseMovieClip.NM_ON_PRESS);

/**
* Set the width of the menu
*
* @param int	w	the width of the tabs
*
*/
mcAssetFinderHeadingClass.prototype.setWidth = function(w)
{
//	set_background_box(this, w, 20, 0x000000, 50);
//	trace("Cancel Width : " + this.cancel._width);
	this.cancel._x = w - this.cancel._width;
	this.cancel._y = (this.back._height - this.cancel._height) / 2;
	this.back._width = w;
}// setWidth()

Object.registerClass("mcAssetFinderHeadingID", mcAssetFinderHeadingClass);
