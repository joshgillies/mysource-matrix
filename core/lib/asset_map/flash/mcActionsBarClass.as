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

}// end constructor

// Make it inherit from MovieClip
mcActionsBarClass.prototype = new MovieClip();

/**
* Shows the options for the passed assetid
*
* @param int	assetid
* @param int	x			the x co-ord for displaying this menu
* @param int	y			the y co-ord for displaying this menu
*
*/
mcActionsBarClass.prototype.show = function(assetid, x, y)
{
	trace("Show Options for Item : " + _root.asset_manager.assets[assetid] + "\n --");

	this.current_assetid = assetid;
	var asset_type = _root.asset_manager.types[_root.asset_manager.assets[assetid].type_code];

	this._x = x;
	this._y = y;

	var max_width = 0;

	for(var i = 0; i < asset_type.edit_screens.length; i++) {
		var c = asset_type.edit_screens[i].code_name;
		var n = asset_type.edit_screens[i].name;

		trace(c + " : " + n);

		var btn_name = "btn_" + c;
		this.buttons.push(btn_name);
		this.attachMovie("mcActionsBarButtonID", btn_name, this.buttons.length);
		this[btn_name].setInfo(c, n);
		trace(this[btn_name]);

		max_width = Math.max(max_width, Math.ceil(this[btn_name].textWidth()));

	}

	trace("Buttons : " + this.buttons);
	trace("MAX WIDTH : " + max_width);

	var xpos = this.border_gap;
	var ypos = this.border_gap;
	for(var i = 0; i < this.buttons.length; i++) {
		trace(this[this.buttons[i]]);
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
	trace("HIDE ACTIONS BAR");

	for(var i = 0; i < this.buttons.length; i++) {
		this[this.buttons[i]].removeMovieClip();
	}

	this.buttons = new Array();
	this._visible = false;
}


/**
* 
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

}

/**
* Fired by a button when it is pressed
* 
* @param string	code_name	the action that the button that was pressed represents
*
*/
mcActionsBarClass.prototype.buttonPressed = function(code_name)
{

	var link = new String(_root.action_bar_path);
	link = link.replace("%assetid%", escape(this.current_assetid))
	link = link.replace("%code_name%", escape(code_name));
	trace("ACTION BAR link : " + link);
	getURL(link, _root.action_bar_frame);

}

Object.registerClass("mcActionsBarID", mcActionsBarClass);
