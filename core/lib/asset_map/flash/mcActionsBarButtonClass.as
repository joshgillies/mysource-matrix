
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

	var text_format = new TextFormat();
	text_format.color = this.getColour();
	text_format.font  = "Arial";
	text_format.size  = 10;
	this.text_field.setTextFormat(text_format);

}

// Make it inherit from MovieClip
mcActionsBarButtonClass.prototype = new MovieClip();
mcActionsBarButtonClass.prototype.bg_colour       = 0xC0C0C0;
mcActionsBarButtonClass.prototype.bg_ro_colour    = 0xF0F0F0;
mcActionsBarButtonClass.prototype.bg_press_colour = 0xD0D0D0;

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
mcActionsBarButtonClass.prototype.setWidth = function(width) 
{
	this.setBG(this.bg_colour, w, this.label_text._height);
}

/**
* OK, set the width by creating with a BG filler
*
* @access public
*/
mcActionsBarButtonClass.prototype.setBG = function(bg_colour, w, h) 
{
	this.clear();

	this.beginFill(bg_colour, 100);
	this.lineStyle();
	this.moveTo(0, 0);
	this.lineTo(w, 0);
	this.lineTo(w, h);
	this.lineTo(0, h);
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
	this.setBG(this.bg_press_colour, this._width, this._height);
}

/**
* Changes the 3D look of the button to be up
*
* @access public
*/
mcActionsBarButtonClass.prototype.btnUp = function() 
{
	this.setBG(this.bg_colour, this._width, this._height);
}

/**
* Changes the 3D look of the button to be up
*
* @access public
*/
mcActionsBarButtonClass.prototype.btnRollover = function() 
{
	this.setBG(this.bg_ro_colour, this._width, this._height);
}


/**
* Fired when the button is pressed
*
* @access public
*/
mcActionsBarButtonClass.prototype.onPress = function() 
{
	this.btnDown();
}

/**
* Fired when the button is pressed
*
* @access public
*/
mcActionsBarButtonClass.prototype.onDragOut = function() 
{
	this.btnUp();
}
/**
* Fired when the button is pressed
*
* @access public
*/
mcActionsBarButtonClass.prototype.onDragOver = function() 
{
	this.btnRollover();
}

/**
* Fired when the close button is pressed
*
* @access public
*/
mcActionsBarButtonClass.prototype.onRelease = function() 
{
	this.btnUp();

	// check if something else is modal
	if (_root.system_events.inModal(this)) return false;
	_root.system_events.screenPress(this);

	this._parent.buttonPressed(this.code_name);

}


Object.registerClass("mcActionsBarButtonID", mcActionsBarButtonClass);
