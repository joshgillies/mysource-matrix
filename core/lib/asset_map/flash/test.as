
_root.test_pressed_count = 0;

function testOnClick1(component)
{ 
	_root.options_box.init("Hello There", "How Are You ? How Are You ? How Are You ? How Are You ? How Are You ? Lorem 'ipsum dolor' sit \"amet\", consectetuer adipiscing elit, sed diam nonummy nibh euismod tincidunt ut laoreet dolore magna aliquam erat volutpat.  Ut wisi enim ad minim veniam, quis nostrud exerci tation ullamcorper suscipit lobortis nisl ut aliquip ex ea commodo consequat.");
	_root.options_box.addOption("one", "Option One");
	_root.options_box.addOption("two", "Option Two");
	_root.options_box.addOption("three", "Option Three");
	_root.options_box.addOption("four", "Option Four");
	_root.options_box.show();

//	_root.asset_manager.reloadAsset(4);
//	_root.asset_manager.reloadAsset(3);

}

function testOnClick2 (component)
{
	for(var i in _root.asset_manager.assets) {
		trace(i + ' :\t' + _root.asset_manager.assets[i]);
	}
	trace("-------------------------------------------------------------------------------------------------------------------------");
	for(var i = 0; i < _root.list_container.items_order.length; i++) {
		trace(i + ' :\t' + _root.list_container.items_order[i].end_branch + ' :\t' + _root.list_container.items_order[i].name);
	}
} 


_root.test_button1.setClickHandler("testOnClick1", _root);
_root.test_button2.setClickHandler("testOnClick2", _root);


