
_root.test_pressed_count = 0;

function testOnClick1(component)
{ 
//	_root.asset_manager.reloadAssets([1]);
//	_root.asset_manager.reloadAsset(3);
//	_root.dialog_box.show("TEst, Test", "Lorem 'ipsum dolor' sit \"amet\", consectetuer adipiscing elit, sed diam nonummy nibh euismod tincidunt ut laoreet dolore magna aliquam erat volutpat.  Ut wisi enim ad minim veniam, quis nostrud exerci tation ullamcorper suscipit lobortis nisl ut aliquip ex ea commodo consequat. ");
	trace("       HEIGHT : " + _root.msgs_bar._height);
	trace("OC BTN HEIGHT : " + _root.msgs_bar.open_close_button._height);
	trace("       WIDTH : " + _root.msgs_bar._width);
	trace("OC BTN WIDTH : " + _root.msgs_bar.open_close_button._width);

	trace("OC BTN POS   : " + _root.msgs_bar.open_close_button._x + ", " + _root.msgs_bar.open_close_button._y);

}

function testOnClick2 (component)
{
	for(var i in _root.asset_manager.types) {
		trace(i + ' :\t' + _root.asset_manager.types[i]);
	}
	trace("-------------------------------------------------------------------------------------------------------------------------");
	for(var i = 0; i < _root.list_container.items_order.length; i++) {
		trace(i + ' :\t' + _root.list_container.items_order[i].end_branch + ' :\t' + _root.list_container.items_order[i].name);
	}
} 


_root.test_button1.setClickHandler("testOnClick1", _root);
_root.test_button2.setClickHandler("testOnClick2", _root);


