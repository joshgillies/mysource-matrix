
_root.pop_up   = false;
_root.progress = false;
_root.loading_xml = false;

function showProgressBar(text) 
{
	if (_root.pop_up) return false;
	_root.pop_up = true;

	_root.progress_bar._visible = true;
	_root.progress_bar.progress_text = text;
	_root.progress_bar.gotoAndPlay(1);

	return true;
}

function hideProgressBar()
{
	_root.progress_bar.stop();
	_root.progress_bar._visible = false;
	_root.pop_up = false;
}

function showDialog(heading, str) 
{
	if (_root.pop_up) return false;
	_root.pop_up = true;

	_root.dialog_box.dialog_heading = heading;
	_root.dialog_box.dialog_text = str;
	_root.dialog_box._visible = true;

	return true;
}

function hideDialog() 
{

	_root.dialog_box._visible = false;
	_root.pop_up = false;

}


/**
* Returns an array of all values that are in arr1 but not in arr2
*
*/
function dialog_border(dialog, x, y, w, h, internal, down) 
{

	var white = 0xFFFFFF;
	var lgrey = 0xC0C0C0;
	var dgrey = 0x808080;
	var black = 0x000000;

	var top_left_outside     = (down) ? black : lgrey;
	var top_left_inside      = (down) ? dgrey : white;
	var bottom_right_outside = (down) ? lgrey : black;
	var bottom_right_inside  = (down) ? white : dgrey;


	if (!internal) {
		x -= 2;
		y -= 2;
		w += 4;
		h += 4;
	}

//	trace("X : " + x + ", Y : " + y + ", W : " + w + ", H : " + h);

	dialog.lineStyle();
	// Light Grey in top Left
	_dialog_border_top_left(dialog, x, y, w, h, top_left_outside);
	// White in top Left inside grey
	_dialog_border_top_left(dialog, x + 1, y + 1, w - 2, h - 2, top_left_inside);

	// Black in bottom right
	_dialog_border_bottom_right(dialog, x, y, w, h, bottom_right_outside);
	// Dark Grey in bottom right inside black
	_dialog_border_bottom_right(dialog, x + 1, y + 1, w - 2, h - 2, bottom_right_inside);


}// end dialog_border()

function _dialog_border_top_left(dialog, x, y, w, h, colour) 
{ 
	dialog.beginFill(colour, 100);
	dialog.moveTo(x,         y);
	dialog.lineTo(x + w - 1, y);
	dialog.lineTo(x + w - 1, y + 1);
	dialog.lineTo(x + 1,     y + 1);
	dialog.lineTo(x + 1,     y + h - 1);
	dialog.lineTo(x,         y + h - 1);
	dialog.lineTo(x,         y);
	dialog.endFill();
}

function _dialog_border_bottom_right(dialog, x, y, w, h, colour) 
{ 
	dialog.beginFill(colour, 100);
	dialog.moveTo(x,         y + h - 1);
	dialog.lineTo(x + w - 1, y + h - 1);
	dialog.lineTo(x + w - 1, y);
	dialog.lineTo(x + w,     y);
	dialog.lineTo(x + w,     y + h);
	dialog.lineTo(x,         y + h);
	dialog.lineTo(x,         y + h - 1);
	dialog.endFill();
}
