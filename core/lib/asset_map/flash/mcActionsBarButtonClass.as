
// Create the Class
function mcActionsBarButtonClass()
{
	this.stop();
	this._visible = false;

	this.dims = {w: 0, h: 0};

	this.code_name = "";

	
	// create the text field
	this.createTextField("label_text", 1, 0, 0, 0, 0);
	this.label_text.multiline = false;	// }
	this.label_text.wordWrap  = false;	// } Using these 3 properties we have a text field that autosizes 
	this.label_text.autoSize  = "left";	// } horizontally but not vertically
	this.label_text.border     = false;
	this.label_text.selectable = false;
	this.label_text._visible   = true;

	this.text_format = new TextFormat();
	this.text_format.font  = "Arial";
	this.text_format.size  = 10;

}

// Make it inherit from MovieClip
mcActionsBarButtonClass.prototype = new MovieClip();
mcActionsBarButtonClass.prototype.colours = {
												normal:   { fg: 0xFFFFFF, bg: 0x606060 },
												rollover: { fg: 0x000000, bg: 0xC0C0C0 }
											 };

/**
* Initialises a new Options box
*
* @param string	code_name		the unique code name for this options
* @param string	label			text name to be displayed
*
* @access public
*/
mcActionsBarButtonClass.prototype.setInfo = function(code_name, label) 
{
	this.code_name       = code_name;
	this.label_text.text = label;
	this.label_text.setTextFormat(this.text_format);
	this.clear(); // do this so that the text is the widest part

}// end setInfo()


/**
* OK, set the width by creating a nice border, with a BG
*
* @access public
*/
mcActionsBarButtonClass.prototype.textWidth = function() 
{
	return this.label_text._width;
}


/**
* OK, set the width by creating with a BG filler
*
* @access public
*/
mcActionsBarButtonClass.prototype.setWidth = function(w) 
{
	this.dims.w = w;
	this.dims.h = this.label_text._height;
	this._setStyle('normal');
}

/**
* OK, set the width by creating with a BG filler
*
* @access public
*/
mcActionsBarButtonClass.prototype._setStyle = function(style) 
{
	this.text_format.color = this.colours[style].fg;
	this.label_text.setTextFormat(this.text_format);

	this.clear();
	this.beginFill(this.colours[style].bg, 100);
	// This is commented out because when we try and explicitly set it, 
	// an extra 2 pixels gets added to the width of the MC for no f!@#$ing reason
//	this.lineStyle();
	this.moveTo(0, 0);
	this.lineTo(this.dims.w, 0);
	this.lineTo(this.dims.w, this.dims.h);
	this.lineTo(0, this.dims.h);
	this.lineTo(0, 0);
	this.endFill();
}


/**
* Changes the 3D look of the button to be depressed
*
* @access public
*/
mcActionsBarButtonClass.prototype.btnDown = function() 
{
	this._setStyle('rollover');
}

/**
* Changes the 3D look of the button to be up
*
* @access public
*/
mcActionsBarButtonClass.prototype.btnUp = function() 
{
	this._setStyle('normal');
}

Object.registerClass("mcActionsBarButtonID", mcActionsBarButtonClass);
