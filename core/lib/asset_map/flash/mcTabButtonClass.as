/**
* TabButton
*
* This is the code for a single instance of a tab button that is used by mcTabs
*
*/

// Create the Class
function mcTabButtonClass()
{
	this.stop();
	this._visible = false;

	this.dims = {w: 0, h: 0};

	// create the text field
	this.createTextField("label_text", 1, 0, 0, 0, 0);
	this.label_text.multiline = false;	// }
	this.label_text.wordWrap  = false;	// } Using these 3 properties we have a text field that autosizes 
	this.label_text.autoSize  = "left";	// } horizontally but not vertically
	this.label_text.border     = false;
	this.label_text.selectable = false;
	this.label_text._visible   = true;

	this.button_left._x = 0;
	this.button_left._y = 0;
	this.button_middle._x = 0;
	this.button_middle._y = 0;
	this.button_middle._width = 0;
	this.button_right._x = 0;
	this.button_right._y = 0;
}

// Make it inherit from MovieClip
mcTabButtonClass.prototype = new MovieClip();

mcTabButtonClass.prototype.text_format = new TextFormat();
mcTabButtonClass.prototype.text_format.font  = "Arial";
mcTabButtonClass.prototype.text_format.size  = 10;
mcTabButtonClass.prototype.text_format.color = 0xffffff;

/**
* Returns the tab name that we belong to
*
* @return string
* @access public
*/
mcTabButtonClass.prototype.getTabName = function() 
{
	// remove the "tab_button_" from start
	return this._name.substr(11);
}// end getTabName()

/**
* Sets the label for the tab
*
* @param string	label	text name to be displayed
*
* @access public
*/
mcTabButtonClass.prototype.setLabel = function(label) 
{
	this.label_text.text = label;
	// set here so that the style get's taken into account with the text width
	this.label_text.setTextFormat(this.text_format); 
	// do this so that the text is the widest part
	this.clear(); 

	this._setStyle('normal');
}// end setLabel()


/**
* Sets the icon for the tab
*
* @param string iconID	the linkage ID of the movie clip to be used as an icon
*
* @access public
*/

mcTabButtonClass.prototype.setIcon = function (iconID)
{
//	trace (this + "::mcTabButtonClass.setIcon(" + iconID + ")");
	if (this.icon != undefined)
		this.icon.removeMovieClip();

	this.attachMovie (iconID, 'icon', 100);
//	trace ("this.icon: " + this.icon);
	this.refresh();
}

/**
* OK, set the width by creating with a BG filler
*
* @access public
*/
mcTabButtonClass.prototype._setStyle = function(style) 
{
	this.label_text.setTextFormat(this.text_format);
	this.refresh();
}

/**
* Changes the 3D look of the button to be depressed
*
* @access public
*/
mcTabButtonClass.prototype.select = function() 
{
	this._setStyle('selected');
	this.button_left.gotoAndStop('on');
	this.button_right.gotoAndStop('on');
	this.button_middle.gotoAndStop('on');
	this._y += this._parent.tab_spacing;
}

/**
* Changes the 3D look of the button to be up
*
* @access public
*/
mcTabButtonClass.prototype.unselect = function() 
{
	this._setStyle('normal');
	this.button_left.gotoAndStop('off');
	this.button_right.gotoAndStop('off');
	this.button_middle.gotoAndStop('off');
	this._y -= this._parent.tab_spacing;
}

/**
* Fired when we are pressed and then release
*
* @access public
*/
mcTabButtonClass.prototype.onRelease = function() 
{
	// only proceed if there is no modal status, or we or one of ours kids owns it
	//var modal = _root.system_events.checkModal(this);
	this._parent.setCurrentTab(this.getTabName());
	_root.system_events.screenPress(this);
	return true;
}// end 


mcTabButtonClass.prototype.refresh = function()
{
//	trace (this + "::mcTabButtonClass.refresh()");
	var nextX = 0;
	var iconPadding = 5;
	this.button_left._x = nextX;
	this.button_left._y = 0;
	nextX += this.button_left._width - 2;
	
	if (this.icon != undefined) {
		nextX += iconPadding;
		this.icon._x = nextX;
		nextX += this.icon._width;
		nextX += iconPadding;

		this.icon._y = (this._height - this.icon._height) / 2;
	}

	this.label_text._x = nextX;
	this.label_text._y = (this._height - this.label_text._height) / 2;
	
	if (this.icon != undefined) {
		this.button_middle._x = this._icon._x;
		this.button_middle._width = this.icon._width + this.label_text._width + 2 * iconPadding;
	} else {
		this.button_middle._x = this.label_text._x;
		this.button_middle._width = this.label_text._width;
	}

	nextX += this.label_text._width;

	this.button_middle._y = 0;


	this.button_right._x = nextX;
	this.button_right._y = 0;

}

Object.registerClass("mcTabButtonID", mcTabButtonClass);
