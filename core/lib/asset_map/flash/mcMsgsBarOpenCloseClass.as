
// Create the Class
function mcMsgsBarOpenCloseClass()
{
	this.stop();
	this._visible = false;
}

// Make it inherit from MovieClip
mcMsgsBarOpenCloseClass.prototype = new MovieClip();

/**
* Initialises a new Options box
*
* @param string	heading
* @param string	summary
* @param string	call_back_obj		the object to run the call back fn on
* @param string	call_back_fn		the fn in this class to execute when the user has selected an option
* @param Object	call_back_params	an object of params to send to the call back fn
*
* @access public
*/
mcMsgsBarOpenCloseClass.prototype.setInfo = function(action, label) 
{

	this.action = action;
	this.label_text.text = label;

	this.clear(); // do this so that the text is the widest part

}// end show()


/**
* OK, set the width by creating a nice border, with a BG
*
* @access public
*/
mcMsgsBarOpenCloseClass.prototype.textWidth = function() 
{
	return this.label_text._width;
}


/**
* OK, set the width by creating a nice border, with a BG
*
* @access public
*/
mcMsgsBarOpenCloseClass.prototype.setWidth = function(width) 
{
	this.clear();


	this.dims = {w: width + (this.border_gap * 2), h: this.label_text._height + (this.border_gap * 2)};

	var new_pos = this.btnUp();

	this.beginFill(this.bg_colour, 100);
	this.lineStyle();
	this.moveTo(new_pos.x, new_pos.y);
	this.lineTo(new_pos.x + this.dims.w, new_pos.y);
	this.lineTo(new_pos.x + this.dims.w, new_pos.y + this.dims.h);
	this.lineTo(new_pos.x, new_pos.y + this.dims.h);
	this.lineTo(new_pos.x, new_pos.y);
	this.endFill();

	this.label_text._x = new_pos.x + this.border_gap;
	this.label_text._y = new_pos.y + this.border_gap;

	this._visible = true;

}

/**
* Changes the 3D look of the button to be depressed
*
* @access public
*/
mcMsgsBarOpenCloseClass.prototype.btnDown = function() 
{
	return _root.dialog_border(this, 0, 0, this.dims.w, this.dims.h, true, true);
}

/**
* Changes the 3D look of the button to be up
*
* @access public
*/
mcMsgsBarOpenCloseClass.prototype.btnUp = function() 
{
	return _root.dialog_border(this, 0, 0, this.dims.w, this.dims.h, true, false);
}

/**
* Fired when the button is pressed
*
* @access public
*/
mcMsgsBarOpenCloseClass.prototype.onPress = function() 
{
	this.btnDown();
}

/**
* Fired when the button is pressed
*
* @access public
*/
mcMsgsBarOpenCloseClass.prototype.onDragOut = function() 
{
	this.btnUp();
}
/**
* Fired when the button is pressed
*
* @access public
*/
mcMsgsBarOpenCloseClass.prototype.onDragOver = function() 
{
	this.btnDown();
}

/**
* Fired when the close button is pressed
*
* @access public
*/
mcMsgsBarOpenCloseClass.prototype.onRelease = function() 
{
	_root.dialog_border(this, 0, 0, this.dims.w, this.dims.h, true, false);

	// check if something else is modal
	if (_root.system_events.inModal(this)) return false;
	_root.system_events.screenPress(this);

	this._parent.buttonPressed(this.action);

}


Object.registerClass("mcMsgsBarOpenCloseID", mcMsgsBarOpenCloseClass);
