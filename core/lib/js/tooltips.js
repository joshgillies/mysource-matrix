/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: tooltips.js,v 1.2 2003/09/26 05:26:34 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

/*
#######################################################################
## Requires: layer_handler.js
#######################################################################
*/

// pointer to the tooltip Layer_Handler Object
var tooltip_layer = null;
var tooltip_bgcolour = "ffffe1";
var tooltip_fontcolour = "000000";

function show_tooltip(e, heading, text, width, div_id){

	// if the div ain't created, then create it
	if (tooltip_layer == null) {
		if (div_id == null) div_id = "ToolTipDiv"
		tooltip_layer = new Layer_Handler(div_id, 0, 600, 600, 0);
		// if the div ain't ready then we can't paint it
		if (!tooltip_layer.layer_OK) {
			tooltip_layer = null;
			return;
		}
	}

	if(width == null || width < 10) width = 0;

	if (!width) width = 200;

	str  = '<table cellpadding="1" cellspacing="0" border="1" bgcolor="#'+ tooltip_fontcolour +'"' + ((width) ? ' width="' + width + '"' : '') + '><tr><td>';
	str += '<table class=backend_data width="100%" cellpadding="5" cellspacing="1" border="0" bgcolor="#'+ tooltip_bgcolour +'"><tr><td class=backend_data' + ((!width) ? ' nowrap' : '') + '>';
	str += '<b style="color: #'+ tooltip_fontcolour +';">'+ heading +'</b>';
	if (text != "" && text != null) {
		str += '</td></tr><tr><td class=backend_data style="color: #'+ tooltip_fontcolour +';">';
		text = text.replace(/(<span)/gi, '$1 style="color: #'+ tooltip_fontcolour +';"');
		str += text;
	}
	str += '</td></tr></table>';
	str += '</td></tr></table>';

	var x = (is_ie4up) ? event.clientX + document.body.scrollLeft : e.pageX;
	var y = (is_ie4up) ? event.clientY + document.body.scrollTop  : e.pageY;
	x += 10;
	y += 10;

	tooltip_layer.write(str);
	// if we have a width then make that the tooltip doesn't go past the edge of the screen
	if (width) {
		// Get the screen width
		var sw = window.screen.availWidth - 20 ;
		if (x + width > sw) x = sw - width;
		tooltip_layer.clip(null,width,null,null);
	} else {
		tooltip_layer.clip(null,600,null,null);
	}

	tooltip_layer.move(x,y);
	tooltip_layer.show();
	window.status = heading;
}

function hide_tooltip() {
	// if the div ain't ready then we can't paint it
	if (tooltip_layer == null) return;
	tooltip_layer.hide();
	window.status = '';
}
