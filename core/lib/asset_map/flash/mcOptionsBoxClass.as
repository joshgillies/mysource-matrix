/**
* This is an options box dialog that allows you to create an options pop-up
* There is a call back fn for it that returns the value of the selected option
* or null if cancel is selected
*
*/



// Create the Class
function mcOptionsBoxClass()
{

	this._visible = false;

	this.bg_colour  = 0xEFEFEF;
	this.fg_colour  = 0x000000;
	this.full_width = 230;

	// Set it so that the text boxes adjust depending on the size of the text
	this.heading_text.autoSize = "center";
	this.summary_text.autoSize = "center";

	this.ok_button.onPress     = function () { this._parent.okPressed() };
	this.cancel_button.onPress = function () { this._parent.cancelPressed() };


	this.options_order = new Array();
	this.options       = new Object();

	this.call_back_obj    = null;
	this.call_back_fn     = null;
	this.call_back_params = null;

}

// Make it inherit from MovieClip
mcOptionsBoxClass.prototype = new MovieClip();

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
mcOptionsBoxClass.prototype.init = function(heading, summary, call_back_obj, call_back_fn, call_back_params) 
{
	// check if something else is modal
	if (_root.system_events.inModal(this) || this._visible) return false;
	// attempt to get the modal status
	if (!_root.system_events.startModal(this)) return false;

	this._clear();

	this.heading_text.text = heading;
	this.summary_text.text = summary;

	this.call_back_obj    = call_back_obj;
	this.call_back_fn     = call_back_fn;
	this.call_back_params = call_back_params;

}

/**
* Adds an option to the display
*
* @param mixed	value	unique value that represents the label
* @param mixed	label	label to display next to option
*
* @access public
*/
mcOptionsBoxClass.prototype.addOption = function(value, label) 
{
	if (value == undefined || value == null) return;
	this.options_order.push(value);
	this.options[value] = label;
}

/**
* Clears off all the options that are currently visible
*/
mcOptionsBoxClass.prototype._clear = function() 
{
	for (var i = 0; i < this.options_order.length; i++) {
		this["option_" + i].removeMovieClip();
	}
	this.options_order = new Array();
	this.options       = new Object();
}

/**
* Displays the options box
*/
mcOptionsBoxClass.prototype.show = function() 
{
	
	this.heading_text.text_color = this.fg_colour;
	this.summary_text.text_color = this.fg_colour;

	var ypos = this.heading_text._y + this.heading_text._height + 5;

	this.summary_text._y = ypos;


	ypos = this.summary_text._y + this.summary_text._height + 5;
	for (var i = 0; i < this.options_order.length; i++) {
		var opt_name = "option_" + i;

		this.attachMovie("FRadioButtonSymbol", opt_name, i);
		this[opt_name]._visible = true;
		this[opt_name].setGroupName("options_group");
//		this[opt_name].setEnabled(true);
		this[opt_name].setData(this.options_order[i]);
		this[opt_name].setLabel(this.options[this.options_order[i]]);

		this[opt_name].setState((i == 0));

		this[opt_name].setSize(this.full_width - 20);
		this[opt_name]._x = 10;

		this[opt_name]._y = ypos;
		ypos += this[opt_name]._height + 10;
		
	}

	this.ok_button._y     = ypos;
	this.cancel_button._y = ypos;
	
	ypos += this.ok_button._height + 5;

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
* hides the options box
*
* @access public
*/
mcOptionsBoxClass.prototype.hide = function() 
{
	_root.system_events.stopModal(this);
	this._visible = false;
}


/**
* called by the OK button to signify a press
*
* @access public
*/
mcOptionsBoxClass.prototype.okPressed = function() 
{
	this.hide();
	this.call_back_obj[this.call_back_fn](this.options_group.getValue(), this.call_back_params);
}

/**
* called by the Cancel button to signify a press
*
* @access public
*/
mcOptionsBoxClass.prototype.cancelPressed = function() 
{
	this.hide();
	this.call_back_obj[this.call_back_fn](null, this.call_back_params);
}


Object.registerClass("mcOptionsBoxID", mcOptionsBoxClass);

