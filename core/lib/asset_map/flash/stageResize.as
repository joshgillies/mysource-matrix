/**
* This listener looks after the resizing of the objects
* on the stage when it is resized
*/



stageResizeListener = new Object();
stageResizeListener.onResize = function () {

	var menu_height = 20;
	var scroller_height = Stage.height - menu_height - _root.ACTIONS_BAR_HEIGHT;

	_root.scroller.setSize(Stage.width, scroller_height);
	_root.actions_bar._y = scroller_height + menu_height;
	_root.actions_bar.setSize(Stage.width, _root.ACTIONS_BAR_HEIGHT);
	_root.list_container.refreshList();

}

Stage.addListener(stageResizeListener);
