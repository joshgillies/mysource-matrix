
_root.test_pressed_count = 0;

function testOnClick1(component)
{ 
	trace("BCR");
	_root.tabs.tree.startAssetFinder(['root_user', 'inbox']);
//	trace('MENU SIZE : ' + _root.tabs.tree.menu_container._width + 'x' + _root.tabs.tree.menu_container._height);
//	trace('scroll pane : ' + (_root.tabs.tree.scroll_pane instanceof MovieClip));

//	_root.asset_manager.reloadAssets([1]);
//	_root.asset_manager.reloadAsset(3);
//	_root.dialog_box.show("TEst, Test", "Lorem "ipsum dolor" sit \"amet\", consectetuer adipiscing elit, sed diam nonummy nibh euismod tincidunt ut laoreet dolore magna aliquam erat volutpat.  Ut wisi enim ad minim veniam, quis nostrud exerci tation ullamcorper suscipit lobortis nisl ut aliquip ex ea commodo consequat. ");
//	for(var i = 0; i < 15; i++) {
//		_root.tabs.log.msgs_container.addMessages([{type: "notice", text: "12/3/2003 17:22\nTHIS IS AN ERROR"}]);
//	}

//	var w = 100;
//	var h = 50;
//	_root.msgs_bar.scroll_content.clear();
//	_root.msgs_bar.scroll_content.beginFill(0x00ff00, 100);
//	// This is commented out because when we try and explicitly set it, 
//	// an extra 2 pixels gets added to the width of the MC for no f!@#$ing reason
//	//this.scroll_content.lineStyle();
//	_root.msgs_bar.scroll_content.moveTo(0, 0);
//	_root.msgs_bar.scroll_content.lineTo(w, 0);
//	_root.msgs_bar.scroll_content.lineTo(w, h);
//	_root.msgs_bar.scroll_content.lineTo(0, h);
//	_root.msgs_bar.scroll_content.lineTo(0, 0);
//	_root.msgs_bar.scroll_content.endFill();
//
//	_root.msgs_bar.scroll_pane.refreshPane();
//
//	trace("SCROLL CONTENT :\t" + _root.msgs_bar.scroll_content._x + ", " + _root.msgs_bar.scroll_content._y + " \t :\t" + (_root.msgs_bar.scroll_content._x + _root.msgs_bar.scroll_content._width) + ", " + (_root.msgs_bar.scroll_content._y + _root.msgs_bar.scroll_content._height));

}

function testOnClick2 (component)
{
	for(var i = 0; i < 7; i++) {
		_root.tabs.log.msgs_container.addMessages([{type: "error", text: "12/3/2003 17:22\nTHIS IS AN ERROR"}]);
	}
//	for(var i in _root.asset_manager.types) {
//		trace(i + " :\t" + _root.asset_manager.types[i]);
//	}
//	trace("-------------------------------------------------------------------------------------------------------------------------");
//	for(var i = 0; i < _root.list_container.items_order.length; i++) {
//		trace(i + " :\t" + _root.list_container.items_order[i].end_branch + " :\t" + _root.list_container.items_order[i].name);
//	}
//
//	_root.msgs_bar.scroll_content.msg_0._refreshLine();
//	trace("TEXT F : " + _root.msgs_bar.scroll_content.msg_0.text_field._y + ", " + _root.msgs_bar.scroll_content.msg_0.text_field._height);
//
//	trace("-------------------------------------------------------------------------------------------------------------------------");
//	trace("scroll_pane       :\t" + _root.msgs_bar.scroll_pane._x + ", " + _root.msgs_bar.scroll_pane._y + " \t :\t" + (_root.msgs_bar.scroll_pane._x + _root.msgs_bar.scroll_pane._width) + ", " + (_root.msgs_bar.scroll_pane._y + _root.msgs_bar.scroll_pane._height));
//	trace("SCROLL CONTENT :\t" + _root.msgs_bar.scroll_content._x + ", " + _root.msgs_bar.scroll_content._y + " \t :\t" + (_root.msgs_bar.scroll_content._x + _root.msgs_bar.scroll_content._width) + ", " + (_root.msgs_bar.scroll_content._y + _root.msgs_bar.scroll_content._height));
//	trace("-------------------------------------------------------------------------------------------------------------------------");
//
//
//	for(var i in _root.msgs_bar.scroll_content) {
//		if (_root.msgs_bar.scroll_content[i] instanceof MovieClip) {
//			trace(i + " :\t" + _root.msgs_bar.scroll_content[i]._x + ", " + _root.msgs_bar.scroll_content[i]._y + " \t :\t" + (_root.msgs_bar.scroll_content[i]._x + _root.msgs_bar.scroll_content[i]._width) + ", " + (_root.msgs_bar.scroll_content[i]._y + _root.msgs_bar.scroll_content[i]._height));
//		}
//	}

//	for(var i in _root.list_container.list) {
//		if (_root.list_container.list[i] instanceof MovieClip) {
//			trace(i + " :\t" + _root.list_container.list[i].text_field.text);
//		}
//	}

//	trace("TABS MCs:");
//	for(var i in _root.tabs) {
//		if (_root.tabs[i] instanceof MovieClip) {
//			trace(i + " :\t" + _root.tabs[i]._name);
//		}
//	}

}


_root.test_button1.swapDepths(200);
_root.test_button2.swapDepths(201);
_root.test_button1.setClickHandler("testOnClick1", _root);
_root.test_button2.setClickHandler("testOnClick2", _root);

