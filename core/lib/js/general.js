/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: general.js,v 1.20 2007/06/06 05:35:21 rong Exp $
*
*/

 ///////////////////////////////////////////////////////
// converts certain chars to their html entity value
// converts :  '&' -> '&amp;'
//             '"' -> '&quot;'
//             '<' -> '&lt;'
//             '>' -> '&gt;'
function htmlspecialchars(str) {

	s = new String(str);

	s = s.replace(/\&/g, '&amp;');
	s = s.replace(/\"/g, '&quot;');
	s = s.replace(/</g,  '&lt;');
	s = s.replace(/>/g,  '&gt;');

	return s;

}// htmlspecialchars()

 ///////////////////////////////////////////////////////
// reverses htmlspecialchars() above
function rev_htmlspecialchars(str) {
	s = new String(str);

	s = s.replace(/\&amp;/g,  '&');
	s = s.replace(/\&quot;/g, '"');
	s = s.replace(/\&lt;/g,   '<');
	s = s.replace(/&gt;/g,    '>');

	return s;

}// rev_htmlspecialchars()

 ///////////////////////////////////////////////////////
// trims all white space from the start and end of
// the string
String.prototype.trim = function() {
	var str = this.toString();
	str = str.replace(/^\s+/, '');
	str = str.replace(/\s+$/, '');
	return str;
}// end trim()

 ///////////////////////////////////////////////////////
// sorts the array then removes any duplicates
// from it
function array_unique(arr) {

	var new_arr = new Array();
	arr.sort();
	var tmp = '';

	for(var i = 0; i < arr.length; i++) {
		if (arr[i] != tmp) {
			new_arr.push(arr[i]);
			tmp = arr[i];
		}// end if
	}// end for

	return new_arr;

}// end array_unique()

 ///////////////////////////////////////////////////////
// takes an array and a value and removes the first
// element in the array with that value
function array_remove_element(arr, val, remove_all) {

	if (remove_all == null) {
		remove_all = false;
	}

	var i = null;
	do {
		var i = array_search(arr, val);
		if (i != null) arr.splice(i, 1);
	} while (remove_all && i != null);

}// end array_remove_element()

 ///////////////////////////////////////////////////////
// takes an array and a value returns the first index
// in the array with the passed value
function array_search(arr, val) {

	for (var i = 0; i < arr.length; i++) {
		if (arr[i] == val) return i;
	}
	return null;

}// end array_search()

 ///////////////////////////////////////////////////////
// takes an array and returns a copy of it
function array_copy(arr) {

	var new_arr = new Array();
	for (var i = 0; i < arr.length; i++) {
		new_arr[i] = arr[i];
	}
	return new_arr;

}// end array_copy()


  /////////////////////////////////////////////////////////////////
 // IMAGE ROLLOVER FUNCTIONS
// holds all the img srcs for the images not currently visible
var preloaded_images = new Array();

// takes an image path and preloads it into the browser
function preload_image(src) {

	var i = preloaded_images.length;

	preloaded_images[i] = new Image();
	preloaded_images[i].src = src;

}// end preload_images()

function img_roll(id, src) {
	if (document.images) {
		document[id].src = src;
	}
}// end img_roll()


/*
* Check all checkboxes that match a certain name. This is usually added to the
* onClick event of a "Check All" style checkbox
*
* @param HTMLFormElement	f			the form that the checkbox is in
* @param string				match_name	regular expression matching the controlled checkboxes
* @param boolean			on			the checked state of the check all checkbox
*
* @return null
*/
function check_all(f, match_name, on) {
	re = new RegExp(match_name);
	for(i=0; i < f.elements.length; i++){
		if (re.test(f.elements[i].name)) {
			f.elements[i].checked = on;
		}
	}

}//end check_all()


/*
* Add this function to the onClick event of all checkboxes controlled by a
* "check all" checkbox
*
* @param HTMLFormElement	f			the form that the checkbox is in
* @param string				match_name	regular expression matching the controlled checkboxes
* @param string				dec_point	elementID of the "check all" checkbox
*
* @return null
*/
function update_checked(f, match_name, check_all_id) {
	var all_checked = true;
	re = new RegExp(match_name);
	for (i=0; i < f.elements.length; i++){
		if (re.test(f.elements[i].name)) {
			if (f.elements[i].checked == false) {
				all_checked = false;
			}
		}
	}
	document.getElementById(check_all_id).checked = all_checked;

}//end update_checked()

/*
* format a number into a string to the specified number of decimal places
* and put in the thousands separator, just like the PHP number_format() fn
*
* @param float	num				the number to format
* @param int	places			the number of decimal places to round to
* @param string	dec_point		the character to use as the decimal point, defaults to '.'
* @param string	thousands_sep	the character to use as the thousands separator, defaults to ''
*
* @return String
*/
function number_format(num, places, dec_point, thousands_sep) {
	// just to make sure we have a number
	num = parseFloat(num);
	if (isNaN(num)) num = 0;
	places = parseInt(places);
	if (isNaN(places) || places < 0) places = 0;

	// if dec_point wasn't set use default
	if (dec_point == undefined || dec_point == null) dec_point = '.';
	// if thousands_sep wasn't set use default
	if (thousands_sep == undefined || thousands_sep == null) thousands_sep = '';


	if (places == 0) {
		return _number_format_thousand_separators(Math.round(num), thousands_sep);
	} else {
		// if we are a zero then
		if (num == 0) {
			var str = '0' + dec_point;
			for(var i = 0; i < places; i++) {
				str += '0';
			}// end for
			return str;
		} else {
			var big_num = Math.round(num * Math.pow(10, places));
			str = big_num.toString();
			var dec_place = (str.length - places);
			var dec_str    = _number_format_thousand_separators(str.substr(0, dec_place), thousands_sep);
			var places_str = str.substr(dec_place);
			return dec_str + dec_point + places_str;

		}// end if
	}// end if

}// end number_format()

function _number_format_thousand_separators(str, sep) {

	str = str.toString();

	if (sep == '') return str;

	if (str.length <= 3) return str;

	var new_str = '';
	var i = str.length % 3;
	var prefix_comma = false;
	if (i > 0) {
		new_str += str.substr(0, i);
		prefix_comma = true;
	}
	while (i < str.length) {
		if (prefix_comma) new_str += sep;
		new_str += str.substr(i, 3);
		i += 3;
		prefix_comma = true;
	}// end while

	return new_str;

}// end _number_format_thousand_separators()


// prints an icon using transparency in IE
// ensures that PNGs have transparent background in IE and Mozilla
function sq_print_icon(path, width, height, alt) {

	if ((typeof window.ActiveXObject != "undefined")) {
		// IE cant handle transparent PNGs
		document.write ('<span style="height:'+height+'px;width:'+width+'px; filter:progid:DXImageTransform.Microsoft.AlphaImageLoader (src=\''+path+'\', sizingMethod=\'scale\')" title="' + alt + '"></span>');
	} else {
		document.write('<img src="'+path+'" width="'+width+'" height="'+height+'" border="0" alt="'+alt+'" />');
	}

}//end sq_print_icon()


// In IE v5.5 - 6 convert tranparent PNGs to use the filter background
// so the transparency works
function fixIcons(blankSrc)
{
	var ieVersion = 0;
	if (navigator.appVersion.indexOf("MSIE")!=-1){
		temp = navigator.appVersion.split("MSIE");
		ieVersion = parseFloat(temp[1]);
	}
	if ((ieVersion >= 5.5) && (ieVersion < 7)) {
		var images = document.getElementsByTagName('IMG');
		for (var i=0; i < images.length; i++) {
			if (images[i].className.match(/(^| )sq-icon($| )/)) {
				images[i].style.filter = "progid:DXImageTransform.Microsoft.AlphaImageLoader(src='"+images[i].src+"', sizingMethod='scale')";
				images[i].style.height = images[i].height+'px';
				images[i].style.width = images[i].width+'px';
				images[i].src = blankSrc;
			}
		}
	}
}


// redirect the user to another page with a friendly message
// and a manual click they can click if something goes wrong
function sq_redirect(url) {

	document.write("<html>");
	document.write("	<head>");
	document.write("		<style type=\"text/css\">");
	document.write("			body {");
	document.write("				font-size:			12px;");
	document.write("				font-family:		Arial, Verdana, Helvetica, sans-serif;");
	document.write("				color:				#000000;");
	document.write("				background-color:	#FFFFFF;");
	document.write("			}");
	document.write("		</style>");
	document.write("	</head>");
	document.write("	<body>");
	document.write("		Please wait while you are redirected. If you are not redirected, please click <a href=\"" + url + "\" title=\"Click here to manually redirect\">here</a>");
	document.write("	</body>");
	document.write("</html>");
	window.location = url;

}//end sq_redirect()


/**
* Simple sprintf implementation
*
* allows for string value replacements and string re-use in the form of
* %s for a single or non order important replacement
* %2(some digit)s for order important replacements i.e. %1s %2s %1s
* uses php type statements (includes the $)
* currently does not allow no string replacements i.e. %d
*
* @return string
*/
function sprintf() {
	var txt = arguments[0];
	var c = 1; //offset source string
	var pattern = /%\d*\$*s/;

	while(c < arguments.length) {
		//below line extracts the current ordered match so we know what argument
		//we are using, the $ needs to be escaped again in order to work.
		replace = new RegExp(((txt.match(pattern)).toString()).replace(/\$/, '\\\$'), "g");

		txt = txt.replace(replace, arguments[(replace.toString()).match(/\d/)]);
		c++;
	}
	return (txt);

}//end sprintf()


/**
* Simple vsprintf implementation
*
* allows for string value replacements and string re-use in the form of
* %s for a single or non order important replacement
* %2(some digit)s for order important replacements i.e. %1s %2s %1s
* uses php type statements (includes the $)
* currently does not allow no string replacements i.e. %d
*
* @return string
*/
function vsprintf() {
	var txt = arguments[0];

	// handle positioned "%1$s"
	var pattern = /%\d+\$s/;

	while(txt.match(pattern)) {
		//below line extracts the current ordered match so we know what argument
		//we are using, the $ needs to be escaped again in order to work.
		replace = new RegExp(((txt.match(pattern)).toString()).replace(/\$/, '\\\$'), "g");

		arg_index =	(replace.toString()).match(/\d/) - 1;
		txt = txt.replace(replace, arguments[1][arg_index]);
	}

	// now handle non-positioned "%s" - need to keep a running counter on this
	// because first "%s" should be replaced by first argument, second "%s" by
	// second argument, etc.
	var pattern = /%s/;
	var current_s = 0;

	while(txt.match(pattern)) {
		txt = txt.replace(pattern, arguments[1][current_s]);
		current_s++;
	}

	return (txt);

}//end vsprintf()