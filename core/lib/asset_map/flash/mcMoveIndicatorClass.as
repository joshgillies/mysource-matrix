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

	this.parent_item_name	= '';
	this.parent_assetid		= 0;
	this.relative_pos		= -1;

	this.line_colour		= 0xff0000;
	
	this.arrow.stop();

}// end constructor

// Make it inherit from MovieClip
mcMoveIndicatorClass.prototype = new MovieClip();

mcMoveIndicatorClass.prototype.startIndicator = function(on_end_obj, on_end_fn) 
{
	if (this.active) return false;
	// attempt to get the modal status
	if (!_root.system_events.startModal(this)) return false;

	this.active = true;

	this.on_end_obj = on_end_obj;
	this.on_end_fn  = on_end_fn;
	this.parent_item_name	= '';
	this.parent_assetid		= 0;
	this.relative_pos		= -1;

	this.onMouseMove		= this.checkPositions;
	this.refresh			= this.drawLine;
	this.drawLine();
	return true;

}// end startIndicator()

mcMoveIndicatorClass.prototype.stopIndicator = function() 
{
	if (!this.active) return;
	_root.system_events.stopModal(this);

//	trace("<------- END MOVE INDICATOR ---------->");
//	trace("PARENT NAME    : " + this.parent_item_name);
//	trace("PARENT ASSETID : " + this.parent_assetid);
//	trace("RELATIVE POS   : " + this.relative_pos);

	// call back to the end fn for whoever called us
	this.on_end_obj[this.on_end_fn](this.parent_item_name, this.parent_assetid, this.relative_pos);

	// clear the move indicator
	this.active   = false;
	this._visible = false;

	this.clear();

	this.onMouseMove	= null;
	this.refresh		= null;

}// end endIndicator()


mcMoveIndicatorClass.prototype.checkPositions = function() 
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

			var item_name = list.items_order[list.items_order.length - 1].name;
			var end_pos = list[item_name]._y + _root.LIST_ITEM_POS_INCREMENT;
			var space_from_end = ym - end_pos;

			// Find out how many branch gaps past the end we are
			var gaps_from_end = Math.floor(space_from_end / _root.LIST_ITEM_END_BRANCH_GAP);
			// make sure this isn't higher than the number of indents in the last item is
			gaps_from_end = Math.min(list[item_name].indent, gaps_from_end);
			// get the new indent based upon how many gaps from the end we are
			var indent = list[item_name].indent - gaps_from_end;

			// Go back up through the parents to find the right one
			this.parent_item_name = item_name;
			for(var i = 0; i < gaps_from_end + 1; i++) {
				this.parent_assetid = list[this.parent_item_name].getParentAssetid();
				this.parent_item_name = list[this.parent_item_name].parent_item_name;
			}
			this.relative_pos = _root.asset_manager.assets[this.parent_assetid].links.length;

			// Now finally set the indicator pos
			this.arrow.gotoAndStop("normal");
			this._x  = indent * _root.LIST_ITEM_INDENT_SPACE;
			this._y  = end_pos + (_root.LIST_ITEM_END_BRANCH_GAP * gaps_from_end);

		} else {
			var item_name = list.items_order[pos.pos].name;

			if (pos.in_gap) {
				this.arrow.gotoAndStop("normal");
				this._x  = list[item_name]._x;
				this._y  = list[item_name]._y + _root.LIST_ITEM_POS_INCREMENT;

				this.parent_item_name = list[item_name].parent_item_name;
				this.parent_assetid   = list[item_name].getParentAssetid();
				this.relative_pos = _root.asset_manager.assets[list[item_name].getParentAssetid()].links.length;

			} else {

				var percentile = ((ym - list[item_name]._y) / _root.LIST_ITEM_POS_INCREMENT);

				if (percentile < 0.45) {
					this.arrow.gotoAndStop("normal");
					this._x    = list[item_name]._x;
					this._y    = list[item_name]._y;

					this.parent_item_name = list[item_name].parent_item_name;
					this.parent_assetid   = list[item_name].getParentAssetid();
					this.relative_pos     = _root.asset_manager.assets[this.parent_assetid].linkPos(list[item_name].linkid);

				} else {
					this.arrow.gotoAndStop("new_child");
					this._x    = list[item_name]._x + _root.LIST_ITEM_INDENT_SPACE;
					this._y    = list[item_name]._y + _root.LIST_ITEM_POS_INCREMENT;
					this.parent_item_name = item_name;
					this.parent_assetid   = list[item_name].assetid;
					this.relative_pos     = 0;

				}
			}// end if
		}// end if

		this._visible = true;
		this.refresh();

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


mcMoveIndicatorClass.prototype.drawLine = function()
{
	this.clear();
	this.lineStyle(1, this.line_colour, 100);
	this.moveTo(0, 0);
	
	this.lineTo(this._parent._parent.scroll_pane.getInnerPaneWidth() - this._x, 0);

}

Object.registerClass("mcMoveIndicatorID", mcMoveIndicatorClass);
