/**
* This listener looks after the resizing of the objects
* on the stage when it is resized
*/

function StageResize()
{
	// Listen to the stage for resize calls
	Stage.addListener(this);
	// Listen to the msgs bar for resize calls
	_root.msgs_bar.addListener(this);

	this.onResize();

}// end stageResize

StageResize.prototype.onResize = function() {

	var menu_height = 20;
	var scroller_height = Stage.height - menu_height - _root.msgs_bar.height();

	_root.scroller.setSize(Stage.width, scroller_height);
	_root.msgs_bar._y = scroller_height + menu_height;
	_root.msgs_bar.setWidth(Stage.width);

	_root.list_container.refreshList();

}// end onResize()

/**
* Event fired when the Msg Bar is opened
*/
StageResize.prototype.onMsgsBarOpen = function() {
	this.onResize();
}

/**
* Event fired when the Msg Bar is closed
*/
StageResize.prototype.onMsgsBarClose = function() {
	this.onResize();
}
