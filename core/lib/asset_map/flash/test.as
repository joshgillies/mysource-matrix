
_root.test_pressed_count = 0;

function testOnClick(component)
{ 

_root.test_pressed_count++;


	switch(_root.test_pressed_count) {
		case 1:
			_root.list_container.reloadAsset(4);
		break;

		default :
			for(var i = 0; i < _root.list_container.items_order.length; i++) {
				trace(i + ' :\t' + _root.list_container[_root.list_container.items_order[i].name]._y + ' :\t' + _root.list_container.items_order[i].name);
			}
	}

return;

	trace("indicator\t: " + _root.list_container.move_indicator.getDepth());
	for(var i = 0; i < _root.list_container.items_order.length; i++) {
		//trace(_root.list_container.items_order[i].name + "\t: " + _root.list_container[_root.list_container.items_order[i].name]._y);
		trace(_root.list_container.items_order[i].name + "\t: " + _root.list_container.items_order[i].end_branch + "\t: " + _root.list_container.items_order[i].branch_count);
	}
/*
	trace("Dialog   :" + _root.dialog_box.getDepth());
	trace("Progress :" + _root.progress_bar.getDepth());
	trace("Container:" + _root.list_container.getDepth());
	trace("List Item:" + _level0.list_container.li_27.getDepth());
	trace("Button   :" + _level0.list_container.li_27.kids_button.getDepth());
	trace("Scroller :" +  _root.scroller.getDepth());
	trace('X :' + _root.test_item._x      + ', Y :' + _root.test_item._y);
	trace('X :' + _root.test_item._xscale + ', Y :' + _root.test_item._yscale);
	trace('W :' + _root.test_item._width  + ', H :' + _root.test_item._height);
	trace('Kids :' + _root.test_item.kids.join(', '));
*/

} 
_root.test_button.setClickHandler("testOnClick", _root);
