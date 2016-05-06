/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: colour_picker.js,v 1.9 2012/08/30 01:09:21 ewang Exp $
*
*/

var colour_fields = new Array();
var colour_pickers = new Array();
var colour_picker_count = 0;
function load_colour_picker(field,picker_path) {
	colour_picker_count++;
	colour_fields[colour_picker_count] = field;
	colour_pickers[colour_picker_count] = window.open(picker_path + '/colour_picker.php?color=' + field.value + '&pickerid='+colour_picker_count, colour_picker_count, 'toolbar=no,width=238,height=164,titlebar=false,status=no,scrollbars=no,resizeable=yes');
}

function update_colour(colour,id) {
	if (colour_fields[id].value != colour) {
		colour_fields[id].value = colour;
		show_colour_change(colour_fields[id].name);
	} else {
		colour_fields[id].value = colour;
	}
}

function show_colour_change(name) {
	if (document.getElementById) {
		var changed_image = document.getElementById('colour_change_' + name);
		if (changed_image) { changed_image.src = colour_change_image_dir + 'tick.png'; }
		var changed_span = document.getElementById('colour_span_' + name);
		if (changed_span) {
			colour_box = document.getElementById('colour_box_' + name);
			changed_span.style.backgroundColor = colour_box.value;
		}
	} else {
		var changed_image = document['colour_change_' + name];
		if (changed_image) { changed_image.src = colour_change_image_dir + 'tick.png'; }
	}
}

var nonhexdigits  = new RegExp('[^0-9a-fA-F]');
var nonhexletters = new RegExp('[g-zG-Z]');

function check_colour(value, allow_blanks) {
	if (value.length == 0 && allow_blanks) return '';

	var c;
	for (i=0;i<value.length;i++) {
		c = value.substring(i,i+1);
		if (c.match(nonhexdigits)) {
			if (c.match(nonhexletters)) {
				value = value.substring(0,i) + 'f' + value.substring(i+1,value.length);
			} else {
				value = value.substring(0,i-1) + '0' + value.substring(i+1,value.length);
			}
		}
	}
	var extra = 6 - value.length;
	for (i=0;i<extra;i++) value += '0';
	return value.toLowerCase();
}
