
_root.test_pressed_count = 0;

function testOnClick1(component)
{ 
//	_root.asset_manager.reloadAssets([1]);
//	_root.asset_manager.reloadAsset(3);
//	_root.dialog_box.show("TEst, Test", "Lorem "ipsum dolor" sit \"amet\", consectetuer adipiscing elit, sed diam nonummy nibh euismod tincidunt ut laoreet dolore magna aliquam erat volutpat.  Ut wisi enim ad minim veniam, quis nostrud exerci tation ullamcorper suscipit lobortis nisl ut aliquip ex ea commodo consequat. ");
	_root.msgs_bar.addMessage("error", "12/3/2003 17:22\nTHIS IS AN ERROR");

}

function testOnClick2 (component)
{
//	for(var i in _root.asset_manager.types) {
//		trace(i + " :\t" + _root.asset_manager.types[i]);
//	}
//	trace("-------------------------------------------------------------------------------------------------------------------------");
//	for(var i = 0; i < _root.list_container.items_order.length; i++) {
//		trace(i + " :\t" + _root.list_container.items_order[i].end_branch + " :\t" + _root.list_container.items_order[i].name);
//	}

	trace("-------------------------------------------------------------------------------------------------------------------------");
	for(var i in _root.msgs_bar.scroll_content) {
		if (_root.msgs_bar.scroll_content[i] instanceof MovieClip) {
			trace(i + " :\t" + _root.msgs_bar.scroll_content[i]._x + ", " + _root.msgs_bar.scroll_content[i]._y + " \t :\t" + (_root.msgs_bar.scroll_content[i]._x + _root.msgs_bar.scroll_content[i]._width) + ", " + (_root.msgs_bar.scroll_content[i]._y + _root.msgs_bar.scroll_content[i]._height));
		}
	}

}

_root.test_button1.setClickHandler("testOnClick1", _root);
_root.test_button2.setClickHandler("testOnClick2", _root);

