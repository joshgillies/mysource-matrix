/*
  ####################################################################################
 # Some useful functions for dealing with the form through javascript
######################################################################################

#######################################################
 # Copyright (c) Blair Robertson <blair_au@yahoo.com>  #
#######################################################

######################################################################################
# $Source: /home/csmith/conversion/cvs/mysource_matrix/core/mysource_matrix/core/lib/html_form/html_form.js,v $
# $Revision: 1.3 $
# $Author: brobertson $
# $Date: 2003/04/23 06:21:12 $
######################################################################################
*/

/*
##################################################
# useful functions for form element values
# Blair Robertson -> (c) 2001/2002
##################################################
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
	if (form_on_submit()) {
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
set_text_field = set_hidden_field;


/**
* return the form element that is represented by the passed name
*
* @param string	$name	the name of the field
*
* @return object Form_Element
* @access public
*/
function get_element(name) 
{
	var f = document.main_form;

	if (f.elements[name]) {
		return f.elements[name];

	} else {
		return null;

	}// endif

}// end get_element()


/**
* returns the value for a field in the form
*
* @param string	$name	the name of the field
*
* @return string
* @access public
*/
function get_element_value(name) 
{
	var f = document.main_form;
	return (f.elements[name]) ? element_value(f.elements[name]) : '';
}// get_element_value()



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
function element_value(element) 
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
			 return (element.checked) ? element[i].value : "";
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

}// end element_value()

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
	if (element.type != "select-one" && element.type != "select-multiple") return;

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

	var day_box   = get_element('day_'   + date_name);
	var month_box = get_element('month_' + date_name);
	var year_box  = get_element('year_'  + date_name);

	var day     = element_value(day_box);
	var month   = element_value(month_box);
	var year    = element_value(year_box);

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
		var hour_box = get_element('hour_'   + date_name);
		var min_box  = get_element('min_' + date_name);

		hour = element_value(hour_box);
		min  = element_value(min_box);

	}// end if

	var time = new Date(year, month - 1, day, hour, min, 0);
	// divide by 1000, because getTime()
	// returns milliseconds since epoch not seconds
	var timestamp = time.getTime() / 1000;
	set_hidden_field(date_name, timestamp);

	return 1;

}// end check_date();


////// FUNCTIONS RELATING TO THE COLOUR BOX ////
//var colour_fields = new Array(); //
//var colour_pickers = new Array();
//var colour_picker_count = 0;
//
///**
//* Opens up the colour picker box in the pop-up window
//*
//* @param string		$field		the name of the colour field as passed into colour_box()
//* @param boolean	$show_time	whether the time is shown or not
//*
//* @access public
//*/
//function load_colour_picker(field, picker_path) {
//	colour_picker_count++;
//	colour_fields[colour_picker_count] = field;
//	//if (colour_picker != 0 && !colour_picker.closed) colour_picker.close();
//	colour_pickers[colour_picker_count] = window.open(picker_path + '/colour_picker.php?colour=' + field.value + '&pickerid='+colour_picker_count, colour_picker_count, 'toolbar=no,width=600,height=200,titlebar=false,status=no,scrollbars=no,resizeable=yes');
//}
//
//function update_colour(colour,id) {
//	if (colour_fields[id].value != colour) {
//		colour_fields[id].value = colour;
//		show_colour_change(colour_fields[id].name);
//	} else {
//		colour_fields[id].value = colour;
//	}
//}
//
//function show_colour_change(name) {
//	if (document.getElementById) {
//		var changed_image = document.getElementById('colour_change_' + name);
//		if (changed_image) { changed_image.src = colour_change_image_dir + 'tick.gif'; }
//		var changed_span = document.getElementById('colour_span_' + name);
//		if (changed_span) {
//			colour_box = document.getElementById('colour_box_' + name);
//			changed_span.style.backgroundColor = colour_box.value;
//		}
//	} else {
//		var changed_image = document['colour_change_' + name];
//		if (changed_image) { changed_image.src = colour_change_image_dir + 'tick.gif'; }
//	}
//}
//
//var nonhexdigits  = new RegExp('[^0-9a-fA-F]');
//var nonhexletters = new RegExp('[g-zG-Z]');
//
//function check_colour(value, allow_blanks) {
//	//if (value.match(nonhexdigits)) return '000000';
//
//	if (value.length == 0 && allow_blanks) return '';
//
//	var c;
//	for (i=0;i<value.length;i++) {
//		c = value.substring(i,i+1);
//		if (c.match(nonhexdigits)) {
//			if (c.match(nonhexletters)) {
//				value = value.substring(0,i) + 'f' + value.substring(i+1,value.length);
//			} else {
//				value = value.substring(0,i-1) + '0' + value.substring(i+1,value.length);
//			}
//		}
//	}
//	var extra = 6 - value.length;
//	for (i=0;i<extra;i++) value += '0';
//	return value.toLowerCase();
//}
//





