
_root.test_pressed_count = 0;

function testOnClick1(component)
{ 

	_root.asset_manager.reloadAsset(1);

}

function testOnClick2 (component)
{
	for(var i in _root.asset_manager.assets) {
		trace(i + ' :\t' + _root.asset_manager.assets[i]);
	}
	for(var i in _root.list_container.items_order) {
		trace(i + ' :\t' + _root.list_container[_root.list_container.items_order[i].name]._y + ' :\t' + _root.list_container.items_order[i].name);
	}
} 


_root.test_button1.setClickHandler("testOnClick1", _root);
_root.test_button2.setClickHandler("testOnClick2", _root);


