/**
* This is an pop-up dialog box
*
*/



// Create the Class
function mcDialogBoxClass()
{
	this.stop();
	this._visible = false;

	this.bg_colour  = 0xC0C0C0;
	this.fg_colour  = 0xFFFFFF;
	this.full_width = 230;




	// Set it so that the text boxes adjust depending on the size of the text
	this.heading_text.autoSize = "center";
	this.summary_text.autoSize = "center";

	this.close_button.onPress     = function () { this._parent.closePressed() };

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
	if (this._visible) return false;

	this.heading_text.text = heading;
	this.summary_text.text = summary;

	this.call_back_obj    = call_back_obj;
	this.call_back_fn     = call_back_fn;
	this.call_back_params = call_back_params;

	this.heading_text.text_color = this.fg_colour;
	this.summary_text.text_color = this.fg_colour;


	this.clear();
	var ypos = this.heading_text._y + this.heading_text._height + 5;

	this.summary_text._y = ypos;

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


	trace(Stage.width + "x" + Stage.height);
	trace(this._width + "x" + this._height);

	// centre this box in the stage
	this._x = (Stage.width  - this.full_width)  / 2;
	this._y = (Stage.height - ypos) / 2;
	this._visible = true;

}// end show()


/**
* Hides the dialog box
*
* @access public
*/
mcDialogBoxClass.prototype.hide = function() 
{
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
