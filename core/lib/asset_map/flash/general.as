/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: general.as,v 1.14 2003/09/26 05:26:32 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


/**
* Creates a background box for the passed mc
*
* @param object MovieClip	mc		the movie clip to set the bg on
* @param int				w		the width of the background
* @param int				h		the height of the background
* @param hex				colour	the colour of the background
* @param int				alpha	the transperancy level of the background (0 - 100) DEFAULT: 100
*
*/
function set_background_box(mc, w, h, colour, alpha) 
{
	if (alpha == undefined) alpha = 100;

	mc.clear();
	mc.beginFill(colour, alpha);
	// This is commented out because when we try and explicitly set it, 
	// an extra 2 pixels gets added to the width of the MC for no f!@#$ing reason
//	mc.lineStyle();
	mc.moveTo(0, 0);
	mc.lineTo(w, 0);
	mc.lineTo(w, h);
	mc.lineTo(0, h);
	mc.lineTo(0, 0);
	mc.endFill();

}

/**
* Prints a 3D border on the passed dialog
*
*/
function dialog_border(dialog, x, y, w, h, internal, depressed) 
{

	var white = 0xFFFFFF;
	var lgrey = 0xC0C0C0;
	var dgrey = 0x808080;
	var black = 0x000000;

	var top_left_outside     = (depressed) ? black : lgrey;
	var top_left_inside      = (depressed) ? dgrey : white;
	var bottom_right_outside = (depressed) ? lgrey : black;
	var bottom_right_inside  = (depressed) ? white : dgrey;


	if (!internal) {
		x -= 2;
		y -= 2;
	}

	w += 4;
	h += 4;

	dialog.lineStyle();
	// Light Grey in top Left
	_dialog_border_top_left(dialog, x, y, w, h, top_left_outside);
	// White in top Left inside grey
	_dialog_border_top_left(dialog, x + 1, y + 1, w - 2, h - 2, top_left_inside);

	// Black in bottom right
	_dialog_border_bottom_right(dialog, x, y, w, h, bottom_right_outside);
	// Dark Grey in bottom right inside black
	_dialog_border_bottom_right(dialog, x + 1, y + 1, w - 2, h - 2, bottom_right_inside);

	return {x: x + 2, y: y + 2};

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

function adjust_brightness (colour, brightness) {
	var red = (colour >> 16) & 0xff;
	var green = (colour >> 8) & 0xff;
	var blue = (colour) & 0xff;

	var new_colour = ((red * brightness) << 16) + ((green * brightness) << 8) + (blue * brightness);

	return new_colour;

}

function logout() {
	getURL('?SQ_ACTION=logout', '_top');
}
