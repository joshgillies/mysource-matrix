/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: html_form.js,v 1.21 2003/11/18 15:37:36 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

/*
* Some useful functions for dealing with the form through javascript
* Specific to Resolve, but easy to pull out for other use
*
*/

/**
* Convenience function for submitting the form, useful from hrefs
*
* @access public
*/
function submit_form()
{
	var f = document.main_form;
	// make sure we clean any stuff up before we submit
	if (f.onsubmit()) {
		f.submit();
	}
}// end submit_form()

/**
* Convenience function for setting hidden fields in the form
*
* @param string	$name	the name of the hidden field
* @param string	$value	the value to set it to
*
* @access public
*/
function set_hidden_field(name, value)
{
	var f = document.main_form;
	if (f.elements[name]) {
		f.elements[name].value = value;
	}
}// end set_hidden_field()

/**
* Convenience function for setting text fields in the form
* NOTE: works the same way as a hidden field so just alias that fn
*
* @param string	$name	the name of the text field
* @param string	$value	the value to set it to
*
* @access public
*/
function set_text_field(name, value)
{
	var f = document.main_form;
	if (f.elements[name]) {
		f.elements[name].value = value;
	}
}// end set_text_field()

/**
* Convenience function for setting button values fields in the form
* NOTE: works the same way as a hidden field so just alias that fn
*
* @param string	$name	the name of the text field
* @param string	$value	the value to set it to
*
* @access public
*/
set_button_value = set_hidden_field;


/**
* return the form element that is represented by the passed name
*
* @param string	$name	the name of the field
*
* @return object Form_Element
* @access public
*/
function get_form_element(name)
{
	var f = document.main_form;

	if (f.elements[name]) {
		return f.elements[name];

	} else {
		return null;

	}// endif

}// end get_form_element()


/**
* returns the value for a field in the form
*
* @param string	$name	the name of the field
*
* @return string
* @access public
*/
function get_form_element_value(name)
{
	var f = document.main_form;
	return (f.elements[name]) ? form_element_value(f.elements[name]) : '';
}// get_form_element_value()



/**
* Returns the value for any type of form element
* if select box or group of radio buttons returns the selected/checked value(s)
*    -> for multi-select boxes returns an array of selected values
* if array of any other type of elements returns the value of the first element in array
*
* @param object Form_Element	$element	the element whose value to find
*
* @return string
* @access public
*/
function form_element_value(element)
{
	// if element doesn't exist, die
	if (element == null) return "";

	// if its null then probably because it's an array, take the type from the first element
	if (element.type == null) element.type = element[0].type;

	switch (element.type) {
		case "select-one" :
			if (element.selectedIndex >= 0) {
				return element.options[element.selectedIndex].value;
			}
		break;

		case "select-multiple" :

			if (element.selectedIndex >= 0) {

				var retArr = new Array();

				for(var i = 0; i < element.options.length; i++) {
					if (element.options[i].selected) {
						retArr.push(element.options[i].value);
					}// endif
				}// end for

				if (retArr.length > 0) {
					return retArr;
				}
			}
		break;

		case "radio" :

			// if its an array of radio buttons then cycle through them
			if (element.length != null) {
				for(var i = 0; i < element.length; i++) {
					if (element[i].checked) {
						return element[i].value;
					}// endif
				}// end for

			} else {
				return element.value;
			}
		break;

		case "checkbox" :
			 return (element.checked) ? element.value : "";
		break;

		default :
			// if its an array of elements return the first ones value
			if (element.length != null && element[0] != null) {
				return element[0].value ;

			// else just return the value
			} else {
				return element.value;
			}

	}// end switch

	// else something not right so return blank
	return "";

}// end form_element_value()

/**
* Given a select box reference, returns the current text
*
* @param object Form_Select_Box	$element	the combo box element
*
* @return string
* @access public
*/
function get_combo_text(element)
{
	// just to make sure
	if (element.type != "select-one" && element.type != "select-multiple") return '';

	return element.options[element.selectedIndex].text;

}// end get_combo_text();


/**
* Checks a specific radio button with the element, that has the passed value
*
* @param object Form_Select_Box	$element	the radio button group
* @param string					$field_val	the value to look for in the radio button group
*
* @access public
*/
function check_radio_button(element, field_val)
{

	for(var i = 0; i < element.length; i++) {
		if (element[i].value == field_val) {
			element[i].checked = true;
			break;
		}
	}// end for

	return;

}// end check_radio_button()

/**
* Sets the selected var in the combo box for the option with the passed value
*
* @param object Form_Select_Box	$element	the combo box
* @param string					$field_val	the value to look for in the combo box
*
* @access public
*/
function highlight_combo_value(element, field_val) {

	// just to make sure
	if (element.type != "select-one" && element.type != "select-multiple") return;

	for(var i = 0; i < element.options.length; i++) {
		if (element.options[i].value == field_val) {
			element.options[i].selected = true;
			element.selectedIndex = i;
			break;
		}
	}// end for

}// end highlight_combo_value()

/**
* Moves the selected elements in a select box (can be multi-select) up or down one place
*
* @param object Form_Select_Box	$element	the combo box
* @param boolean				$move_up	whether to move up or down
*
* @access public
*/
function move_combo_selection(element, move_up)
{

	switch (element.type) {
		case "select-one" :
			if (element.selectedIndex >= 0) {
				if (move_up) {
					// can only move up if we ain't the first element
					if (element.selectedIndex > 0) {

						var i = element.selectedIndex;
						var tmp1 = new Option(element.options[i - 1].text, element.options[i - 1].value);
						tmp1.selected = element.options[i - 1].selected;

						var tmp2 = new Option(element.options[i].text, element.options[i].value);
						tmp2.selected = element.options[i].selected;

						element.options[i - 1] = tmp2
						element.options[i]     = tmp1;
					}// end if not first element

				// else moving down
				} else {

					// can only move down if we ain't the last element
					if (element.selectedIndex < (element.options.length - 1)) {
						var i = element.selectedIndex;
						var tmp1 = new Option(element.options[i + 1].text, element.options[i + 1].value);
						tmp1.selected = element.options[i + 1].selected;

						var tmp2 = new Option(element.options[i].text, element.options[i].value);
						tmp2.selected = element.options[i].selected;

						element.options[i + 1] = tmp2
						element.options[i]     = tmp1;

					}// end if not last element

				}// end if move_up

			}// end if selected index

		break;

		case "select-multiple" :

			if (move_up) {

				for(var i = 0; i < element.options.length; i++) {

					if (!element.options[i].selected) continue;

					// can only move up if we ain't the first element
					// and the element above it isn't selected
					if (i > 0 && !element.options[i - 1].selected) {
						var tmp1 = new Option(element.options[i - 1].text, element.options[i - 1].value);
						tmp1.selected = element.options[i - 1].selected;

						var tmp2 = new Option(element.options[i].text, element.options[i].value);
						tmp2.selected = element.options[i].selected;

						element.options[i - 1] = tmp2
						element.options[i]     = tmp1;
					}

				}// end for

			// else moving down
			} else {

				for(var i = element.options.length - 1; i > -1; i--) {

					if (!element.options[i].selected) continue;

					// can only move down if we ain't the last element
					// and the element above isn't selected
					if (i < (element.options.length - 1) && !element.options[i + 1].selected) {
						var tmp1 = new Option(element.options[i + 1].text, element.options[i + 1].value);
						tmp1.selected = element.options[i + 1].selected;

						var tmp2 = new Option(element.options[i].text, element.options[i].value);
						tmp2.selected = element.options[i].selected;

						element.options[i + 1] = tmp2
						element.options[i]     = tmp1;

					}//end if

				}// end for

			}// end if move_up

		break;

		default:
			alert('Element "' + element.name + '" is not a combo box');

	}// end switch

}// end move_combo_selection()


/**
* Used by the date_box to verify that the date the user entered is correct
* and to set the hidden var
*
* @param string		$date_name	the name of the date field as passed into date_box()
* @param boolean	$show_time	whether the time is shown or not
*
* @access public
*/
function check_date(date_name, show_time)
{
	var f = document.main_form;

	var day_box   = get_form_element('day_'   + date_name);
	var month_box = get_form_element('month_' + date_name);
	var year_box  = get_form_element('year_'  + date_name);

	var day     = form_element_value(day_box);
	var month   = form_element_value(month_box);
	var year    = form_element_value(year_box);

	if (month == 2) {
		if (day == 29) {
			// if not leap year
			if (((year % 4) != 0) || ( ((year % 100) == 0) && ((year % 400) != 0))) {
				alert (year + " is not a leap year, there is no " + day + "th of Feburary.");
				highlight_combo_element(day_box, 1);
				day_box.focus();
				return 0;
			}

		} else if (day > 29) {
			alert ("There is no " + day + "th of Feburary.");
			highlight_combo_element(day_box, 1);
			day_box.focus();
			return 0;
		}// end if
	}// end if

	if ((month == 4 || month == 6 || month == 9 || month == 11) && day == 31) {
		alert ("There is no 31st of " + get_combo_text(month_box) + ".");
		highlight_combo_element(day_box, 1);
		day_box.focus();
		return 0;
	}

	var hour = 0;
	var min  = 0;

	// if we're showing the time boxes get them as well
	if(show_time) {
		var hour_box = get_form_element('hour_'   + date_name);
		var min_box  = get_form_element('min_' + date_name);

		hour = form_element_value(hour_box);
		min  = form_element_value(min_box);

	}// end if

	var time = new Date(year, month - 1, day, hour, min, 0);
	// divide by 1000, because getTime()
	// returns milliseconds since epoch not seconds
	var timestamp = time.getTime() / 1000;
	set_hidden_field(date_name, timestamp);

	return 1;

}// end check_date();


/**
* Activated by the pressing of the "Change..." button to start the asset finder mode in the flash menu
*
* @param string	$name			the name of the hidden field
* @param string	$safe_name		the name prefix for all the other form elements associated with the
* @param string	$type_codes_xml	xml containing type codes that we want to find
* @param string	$top_obj		a reference to the top window so we can find the sidenav
* @param string	$done_fn		a function to call when the asset finder is finished
*
* @access public
*/
var ASSET_FINDER_FIELD_NAME = null;
var ASSET_FINDER_FIELD_SAFE_NAME = null;
var ASSET_FINDER_TOP_OBJ = null;
var ASSET_FINDER_DONE_FUNCTION = null;
function asset_finder_change_btn_press(name, safe_name, type_codes_xml, top_obj, done_fn)
{
	var f = document.main_form;
	ASSET_FINDER_TOP_OBJ = top_obj;
	ASSET_FINDER_DONE_FUNCTION = done_fn;

	if (ASSET_FINDER_FIELD_NAME != null && ASSET_FINDER_FIELD_NAME != name) {
		alert('The asset finder is currently in use');
		return;
	}

	if (!ASSET_FINDER_TOP_OBJ.sidenav && !ASSET_FINDER_TOP_OBJ.sidenav.asset_finder_start) {
		alert('Unable to find flash');
	}

	// no name ? we must be starting the asset finder
	if (ASSET_FINDER_FIELD_NAME == null) {
		ASSET_FINDER_FIELD_NAME = name;
		ASSET_FINDER_FIELD_SAFE_NAME = safe_name;
		ASSET_FINDER_TOP_OBJ.sidenav.asset_finder_start(asset_finder_done, type_codes_xml);
		set_button_value(ASSET_FINDER_FIELD_SAFE_NAME + '_change_btn', 'Cancel');

	// else we must be cancelling the asset finder
	} else {
		ASSET_FINDER_TOP_OBJ.sidenav.asset_finder_cancel();
		set_button_value(ASSET_FINDER_FIELD_SAFE_NAME + '_change_btn', 'Change...');
		ASSET_FINDER_FIELD_NAME = null;
		ASSET_FINDER_FIELD_SAFE_NAME = null;
	}

}// end asset_finder_change_btn_press()


/**
* Call-back fns that stops the asset finder
*
* @param int	$assetid		the assetid that has been selected
* @param int	$label			the name of the selected asset
*
* @access public
*/
function asset_finder_done(assetid, label)
{
	if (ASSET_FINDER_FIELD_NAME == null) return;

	// if we get a -1 they cancelled, do nothing
	if (assetid != -1) {
		set_hidden_field(ASSET_FINDER_FIELD_NAME, assetid);
		set_text_field(ASSET_FINDER_FIELD_SAFE_NAME + '_label', (assetid == 0) ? '' : label + ' (Id : #' + assetid + ')');
	}
	set_button_value(ASSET_FINDER_FIELD_SAFE_NAME + '_change_btn', 'Change...');
	ASSET_FINDER_FIELD_NAME = null;
	ASSET_FINDER_FIELD_SAFE_NAME = null;
	if (ASSET_FINDER_DONE_FUNCTION !== null) ASSET_FINDER_DONE_FUNCTION();

}// end asset_finder_done()


/**
* Activated by the pressing of the "Clear" button
*
* @param string	$name			the name of the hidden field
* @param string	$safe_name		the name prefix for all the other form elements associated with the
*
* @access public
*/
function asset_finder_clear_btn_press(name, safe_name)
{
	set_hidden_field(name, 0);
	set_text_field(safe_name + '_label', '');

}// end asset_finder_clear_btn_press()


/**
* Activated by the pressing of the "Reset" button
*
* @param string	$name			the name of the hidden field
* @param string	$safe_name		the name prefix for all the other form elements associated with the
*
* @access public
*/
function asset_finder_reset_btn_press(name, safe_name, assetid, label)
{
	set_hidden_field(name, assetid);
	set_text_field(safe_name + '_label', label);

}// end asset_finder_reset_btn_press()


/**
* Activated by on an unload event to cancel the asset finder if we are currently looking
*
* @access public
*/
function asset_finder_onunload()
{
	// got a name ? we must be finding assets, cancel it
	if (ASSET_FINDER_FIELD_NAME != null) {
		if (ASSET_FINDER_TOP_OBJ.sidenav && ASSET_FINDER_TOP_OBJ.sidenav.asset_finder_cancel) {
			ASSET_FINDER_TOP_OBJ.sidenav.asset_finder_cancel();
		}
	}
}// end asset_finder_onunload()

ASSET_FINDER_OTHER_ONUNLOAD = (window.onunload) ? window.onunload : new Function;
window.onunload = asset_finder_onunload;
