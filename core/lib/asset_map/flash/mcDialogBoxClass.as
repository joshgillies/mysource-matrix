/**
* This is an pop-up dialog box
*
*/



// Create the Class
function mcDialogBoxClass()
{
	this.stop();
	this._visible = false;

	this.bg_colour  = 0xEFEFEF;
	this.fg_colour  = 0xFFFFFF;
	this.full_width = 230;




	// Set it so that the text boxes adjust depending on the size of the text
	this.heading_text.autoSize = "center";
	this.summary_text.autoSize = "center";

	this.close_button.onRelease	= function () { this._parent.closePressed() };

	this.call_back_obj    = null;
	this.call_back_fn     = null;
	this.call_back_params = null;

}

// Make it inherit from MovieClip
mcDialogBoxClass.prototype = new MovieClip();

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
mcDialogBoxClass.prototype.show = function(heading, summary, call_back_obj, call_back_fn, call_back_params) 
{
	// check if something else is modal
	if (_root.system_events.inModal(this) || this._visible) return false;
	// attempt to get the modal status
	if (!_root.system_events.startModal(this)) return false;

	this.summary_text.autoSize = true;
	this.heading_text.text = heading;
	this.summary_text.text = summary;

	this.call_back_obj    = call_back_obj;
	this.call_back_fn     = call_back_fn;
	this.call_back_params = call_back_params;

	this.heading_text.text_color = this.fg_colour;
	this.summary_text.text_color = this.fg_colour;

	var baseHeight =	this.heading_text._y + this.heading_text._height + 5 + 
						this.summary_text._height + 5 + 
						this.close_button._height + 5;

	this.summary_text._y = ypos;

	this.heading_text._y = 5;

	var ypos = this.heading_text._y + this.heading_text._height + 5;


	if (baseHeight > Stage.height) {
		this.summary_text.autoSize = false;
		var diff = baseHeight - Stage.height;
		this.summary_text._height -= diff + 10;
		this.summary_text._width = this.heading_text._width - this.summary_scroll._width;

		this.summary_scroll._y = ypos;
		this.summary_scroll._x = this.summary_text._x + this.summary_text._width;
		
		this.summary_scroll.setSize(this.summary_text._height);
		this.summary_scroll.setScrollTarget(this.summary_text);

		this.summary_scroll._visible = true;
	} else {
		this.summary_text._width = this.heading_text._width;
		this.summary_scroll._visible = false;
	}

	ypos = this.summary_text._y + this.summary_text._height + 5;
	
	this.close_button._y = ypos;
	
	ypos += this.close_button._height + 5;
	
	this.clear();
	_root.dialog_border(this, 0, 0, this.full_width, ypos, false, false);
	this.beginFill(this.bg_colour, 100);
	this.lineStyle();
	this.moveTo(0, 0);
	this.lineTo(this.full_width, 0);
	this.lineTo(this.full_width, ypos);
	this.lineTo(0, ypos);
	this.lineTo(0, 0);
	this.endFill();

	// centre this box in the stage
	this._x = (Stage.width  - this.full_width)  / 2;
	this._y = (Stage.height - ypos) / 2;
	this._visible = true;

}// end show()

/**
 * Handles the scrolling. 
 *
*/
mcDialogBoxClass.prototype.onScroll = function()
{
	trace (this + "::mcDialogBoxClass.onScroll()  " + this.summary_scroll.getScrollPosition());
	this.summary_text.scroll = this.summary_scroll.getScrollPosition();
}

/**
* Hides the dialog box
*
* @access public
*/
mcDialogBoxClass.prototype.hide = function() 
{
	_root.system_events.stopModal(this);
	//this.summary_scroll.removeMovieClip();
	this._visible = false;
}

/**
* Fired when the close button is pressed
*
* @access public
*/
mcDialogBoxClass.prototype.closePressed = function() 
{
	if (this.call_back_obj && this.call_back_fn) {
		this.call_back_obj[this.call_back_fn](null, this.call_back_params);
	}
	this.hide();
}

Object.registerClass("mcDialogBoxID", mcDialogBoxClass);
