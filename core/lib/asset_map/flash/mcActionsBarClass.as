 /////////////////////////////////////////////////////////////////////////////
// NOTE: the list items in this container are just stored as
// normal attributes and not in there own array (as I would have liked)
// this is due to the attachMovie() fn not being able to accept arrays
// elements as the instance name for the new movie


// Create the Class
function mcActionsBarClass()
{

	this._x = 0;

	this.bg_colour  = 0xC0C0C0;

	this.current_assetid = 0;
	this.buttons = new Array();


	// Set ourselves up as a listener on the menu, so we know when an item is pressed
	_root.list_container.addListener(this);

}// end constructor

// Make it inherit from MovieClip
mcActionsBarClass.prototype = new MovieClip();

/**
* Event fired when list container selects a list item
*
* @param asset
*
*/
mcActionsBarClass.prototype.onListItemSelection = function(assetid)
{
	trace("Selected Item : " + _root.asset_manager.assets[assetid] + "\n --");

	this.current_assetid = assetid;

	var asset_type = _root.asset_manager.types[_root.asset_manager.assets[assetid].type_code];

	var max_width = 0;

	for(var i = 0; i < asset_type.editing_options.length; i++) {
		var a = asset_type.editing_options[i].action;
		var n = asset_type.editing_options[i].name;

		trace(a + " : " + n);

		var btn_name = "btn_" + a;
		this.buttons.push(btn_name);
		this.attachMovie("mcActionsBarButtonID", btn_name, 100 + this.buttons.length);
		this[btn_name].setInfo(a, n);
		trace(this[btn_name]);

		max_width = Math.max(max_width, Math.ceil(this[btn_name].textWidth()));

	}

	trace("Buttons : " + this.buttons);
	trace("MAX WIDTH : " + max_width);

	var xpos = 5;
	var ypos = 5;
	for(var i = 0; i < this.buttons.length; i++) {
		trace(this[this.buttons[i]]);
		this[this.buttons[i]].setWidth(max_width);
		this[this.buttons[i]]._x = xpos;
		this[this.buttons[i]]._y = ypos;

		ypos += this[this.buttons[i]];

	}

}// end onListItemSelection()

/**
* Event fired when list container un-selects a list item
*/
mcActionsBarClass.prototype.onListItemUnSelection = function()
{
	for(var i = 0; i < this.buttons.length; i++) {
		this[this.buttons[i]].removeMovieClip();
	}

	this.buttons = new Array();

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
* @param string	action	the action that the button that was pressed represents
*
*/
mcActionsBarClass.prototype.buttonPressed = function(action)
{

	var url = new String(_root.action_bar_path);
	url = url.replace("%assetid%", escape(this.current_assetid))
	url = url.replace("%action%", escape(action));
	trace("ACTION BAR URL : " + url);

}

Object.registerClass("mcActionsBarID", mcActionsBarClass);
