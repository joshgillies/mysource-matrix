
#include "mcActionsBarButtonClass.as"
 
 /////////////////////////////////////////////////////////////////////////////
// NOTE: the options in this container are just stored as
// normal attributes and not in there own array (as I would have liked)
// this is due to the attachMovie() fn not being able to accept arrays
// elements as the instance name for the new movie


// Create the Class
function mcActionsBarClass()
{
	this._visible = false;
	this._x = 0;
	this.bg_colour     = 0x606060;
	this.border_gap    = 2;

	this.current_assetid = 0;
	this.buttons = new Array();
	this.current_button = null;
	this.mouse_over_us  = null;

}// end constructor

// Make it inherit from MovieClip
mcActionsBarClass.prototype = new NestedMouseMovieClip(false, NestedMouseMovieClip.NM_ON_PRESS);

/**
* Shows the options for the passed assetid
*
* @param Array(string)	actions		the action code to use when a button is pressed
* @param Array(string)	labels		the corresponding label for each action code
* @param int	x			the x co-ord for displaying this menu
* @param int	y			the y co-ord for displaying this menu
*
*/
mcActionsBarClass.prototype.show = function(actions, labels, x, y)
{
	this.current_button  = null;
	this.mouse_over_us   = false;

	this._x = x;
	this._y = y;

	var max_width = 0;

	if (actions.length > 0) {
		for(var i = 0; i < actions.length; i++) {

			var btn_name = "btn_" + actions[i];
			this.buttons.push(btn_name);
			this.attachMovie("mcActionsBarButtonID", btn_name, this.buttons.length);
			this[btn_name].setInfo(actions[i], labels[i]);

			max_width = Math.max(max_width, Math.ceil(this[btn_name].textWidth()));

		}// end for

	} else {
		this.buttons.push('dud');
		this.attachMovie("mcActionsBarButtonID", 'dud', 1);
		this.dud.setInfo('', '[No Options Available]');
		max_width = this.dud.textWidth();
	}

//	trace("Buttons : " + this.buttons);
//	trace("MAX WIDTH : " + max_width);

	var xpos = this.border_gap;
	var ypos = this.border_gap;
	for(var i = 0; i < this.buttons.length; i++) {
		this[this.buttons[i]].setWidth(max_width);
		this[this.buttons[i]]._x = xpos;
		this[this.buttons[i]]._y = ypos;
		this[this.buttons[i]]._visible = true;
		ypos += this[this.buttons[i]]._height;
	}// end for

	this.setSize(max_width + (this.border_gap * 2), ypos + this.border_gap);
	this._visible = true;

}// end show()

/**
* Event fired when list container un-selects a list item
*/
mcActionsBarClass.prototype.hide = function()
{
	for(var i = 0; i < this.buttons.length; i++) {
		this[this.buttons[i]].removeMovieClip();
	}

	this.buttons = new Array();
	this._visible = false;
}


/**
* Set the background size for this bar
*/
mcActionsBarClass.prototype.setSize = function(w, h)
{
	this.clear();
	this.beginFill(this.bg_colour, 100);
	this.lineStyle();
	this.moveTo(0, 0);
	this.lineTo(w, 0);
	this.lineTo(w, h);
	this.lineTo(0, h);
	this.lineTo(0, 0);
	this.endFill();

}// end setSize()

/**
* Fired when the button is pressed
*
* @access public
*/
mcActionsBarClass.prototype.onPress = function() 
{
	this.mouse_over_us = true;
	// deliberatly do nothing else because we don't want to pass any of this to the kids
	return true;
}

/**
* Fired when the moves out from being over us (and it was pressed over us)
*
* @access public
*/
mcActionsBarClass.prototype.onDragOut = function() 
{
	this[this.current_button].btnUp();
	this.mouse_over_us = false;
	return true;
}

/**
* Fired when the moves over us (and it was initially pressed over us)
*
* @access public
*/
mcActionsBarClass.prototype.onDragOver = function() 
{
	this.mouse_over_us = true;
	return true;
}

/**
* Fired when the button is pressed
*
* @access public
*/
mcActionsBarClass.prototype.onMouseMove = function() 
{
	if (this.mouse_over_us) {
		var mc_name = this._NM_findMc(this._xmouse, this._ymouse);
		if (mc_name === null) {
			if (this.current_button !== null) {
				this[this.current_button].btnUp();
				this.current_button = null;
			}
		} else {
			if (this.current_button !== null && this[mc_name] !== this[this.current_button]) {
				this[this.current_button].btnUp();
				this.current_button = null;
			}

			if (this.current_button === null) {
				this.current_button = mc_name;
				this[this.current_button].btnDown();
			}

		} // end if
	}// end if
}// end onMouseMove()

/**
* Fired when the mouse button was pressed over us and when it's lifted and it's still over us
*
* @access public
*/
mcActionsBarClass.prototype.onRelease = function() 
{
	// if there is a button highlighted, use it
	if (this.current_button !== null && this[this.current_button].code_name != '') {
		trace("Action : " + this[this.current_button].code_name);
		this._parent.actionsBarPressed(this[this.current_button].code_name);
	}

	this.mouse_over_us = false;
	this.hide();
	return true;
}// end onRelease()

/**
* Fired when the mouse button was pressed over us and when it's lifted and it's not over us
*
* @access public
*/
mcActionsBarClass.prototype.onReleaseOutside = function() 
{
	this.mouse_over_us = false;
	this.hide();
	return true;
}// end onReleaseOutside();


Object.registerClass("mcActionsBarID", mcActionsBarClass);
