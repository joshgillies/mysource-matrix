/**
* NOTE: 
*    The movement of this indicator relies VERY heavily on how the mcListItemContainer works
*/
 
// Create the Class
function mcMoveIndicatorClass()
{
	this._visible = false;

	this.active		= false;
	this.on_end_obj	= null;
	this.on_end_fn	= '';
	this.pos		= -1
	this.where		= "";

}// end constructor

// Make it inherit from MovieClip
mcMoveIndicatorClass.prototype = new MovieClip();

/**
* Set the background size for this bar
*/
//mcMoveIndicatorClass.prototype.setSize = function(w, h)
//{
//	this.clear();
//	this.beginFill(this.bg_colour, 100);
//	this.lineStyle();
//	this.moveTo(0, 0);
//	this.lineTo(w, 0);
//	this.lineTo(w, h);
//	this.lineTo(0, h);
//	this.lineTo(0, 0);
//	this.endFill();
//
//}// end setSize()

mcMoveIndicatorClass.prototype.startIndicator = function(on_end_obj, on_end_fn) 
{
	trace('Active : ' + this.active);
	if (this.active) return false;
	// attempt to get the modal status
	if (!_root.system_events.startModal(this)) return false;

	this.active = true;

	this.on_end_obj = on_end_obj;
	this.on_end_fn  = on_end_fn;
	this.pos   = -1;
	this.where = "";

	return true;

}// end startIndicator()

mcMoveIndicatorClass.prototype.stopIndicator = function() 
{
	if (!this.active) return;
	_root.system_events.stopModal(this);

	// call back to the end fn for whoever called us
	this.on_end_obj[this.on_end_fn](this.pos, this.where);

	// clear the move indicator
	this.active   = false;
	this._visible = false;

}// end endMoveIndicator()


mcMoveIndicatorClass.prototype.onMouseMove = function() 
{
	if (this.active) {

		var list = this._parent.list;
		var xm = list._xmouse;
		var ym = list._ymouse;

		var pos = list.getItemPos(xm, ym, true);

		// make sure the number is valid
		if (pos.pos > list.items_order.length) pos.pos = list.items_order.length;
		else if (pos.pos < 0) pos.pos = 0;

		// if we are past the end of the list then we are really at the last pos, in the gap
		if (pos.pos == list.items_order.length) {
			pos.pos    = list.items_order.length - 1;
			pos.in_gap = true;
		}

		// if we are past the end of the list, improvise
		var item_name = list.items_order[pos.pos].name;

		this.pos = pos.pos;

		if (pos.in_gap) {
			this.gotoAndStop("normal");
			this._x  = list[item_name]._x;
			this._y  = list[item_name]._y + _root.LIST_ITEM_POS_INCREMENT;
			this.where = "after";

		} else {

			var percentile = ((ym - list[item_name]._y) / _root.LIST_ITEM_POS_INCREMENT);

			if (percentile < 0.45) {
				this.gotoAndStop("normal");
				this._x    = list[item_name]._x;
				this._y    = list[item_name]._y;
				this.where = "before";

			} else {
				this.gotoAndStop("new_child");
				this._x    = list[item_name]._x + _root.LIST_ITEM_INDENT_SPACE;
				this._y    = list[item_name]._y + _root.LIST_ITEM_POS_INCREMENT;
				this.where = "child";

			}
		}

		this._visible = true;

	}// end if action

	return true;

}// end onMouseMove()

/**
* Fired when the mouse button was pressed over us and when it's lifted and it's still over us
*
* @access public
*/
mcMoveIndicatorClass.prototype.onRelease = function() 
{
	trace('Move On Release');
	this.stopIndicator();
	return true;
}// end onRelease()

/**
* Fired when the mouse button was pressed over us and when it's lifted and it's not over us
*
* @access public
*/
mcMoveIndicatorClass.prototype.onReleaseOutside = function() 
{
	this.stopIndicator();
	return true;
}// end onReleaseOutside();


Object.registerClass("mcMoveIndicatorID", mcMoveIndicatorClass);
