

// Create the Class
function mcAssetFinderHeadingClass()
{

	trace("CREATE ASSET FINDER");

	// create the text field
	this.createTextField("label_text", 1, 0, 0, 0, 0);
	this.label_text.multiline  = false;	// }
	this.label_text.wordWrap   = false;	// } Using these 3 properties we have a text field that autosizes 
	this.label_text.autoSize   = "left";	// } horizontally but not vertically
	this.label_text.border     = false;
	this.label_text.selectable = false;
	this.label_text._visible   = true;
	this.label_text.text = "Asset Finder";

	var text_format = new TextFormat();
	text_format.font  = "Arial";
	text_format.size  = 12;

	this.label_text.setTextFormat(text_format); 

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
	trace("Cancel Width : " + this.cancel._width);
	this.cancel._x = w - this.cancel._width;
}// setWidth()

Object.registerClass("mcAssetFinderHeadingID", mcAssetFinderHeadingClass);
