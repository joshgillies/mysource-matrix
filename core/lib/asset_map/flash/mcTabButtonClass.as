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
	this.label_text._x = this.h_gap;
	this.label_text._y = this.v_gap;
}

// Make it inherit from MovieClip
mcTabButtonClass.prototype = new MovieClip();
mcTabButtonClass.prototype.text_format = new TextFormat();
mcTabButtonClass.prototype.text_format.font  = "Arial";
mcTabButtonClass.prototype.text_format.size  = 11;
mcTabButtonClass.prototype.h_gap= 2;
mcTabButtonClass.prototype.v_gap= 0;

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
* OK, set the width by creating with a BG filler
*
* @access public
*/
mcTabButtonClass.prototype._setStyle = function(style) 
{
	this.text_format.color = this._parent.colours[style].fg;
	this.label_text.setTextFormat(this.text_format);
	set_background_box(this, this.label_text._width + (2 * this.h_gap), this.label_text._height + (2 * this.v_gap), this._parent.colours[style].bg, 100);
}

/**
* Changes the 3D look of the button to be depressed
*
* @access public
*/
mcTabButtonClass.prototype.select = function() 
{
	this._setStyle('selected');
}

/**
* Changes the 3D look of the button to be up
*
* @access public
*/
mcTabButtonClass.prototype.unselect = function() 
{
	this._setStyle('normal');
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
}// end 


Object.registerClass("mcTabButtonID", mcTabButtonClass);
