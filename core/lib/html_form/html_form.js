/*
  ####################################################################################
 # Some useful functions for dealing with the form through javascript
######################################################################################

#######################################################
 # Copyright (c) Blair Robertson <blair_au@yahoo.com>  #
#######################################################

######################################################################################
# $Source: /home/csmith/conversion/cvs/mysource_matrix/core/mysource_matrix/core/lib/html_form/html_form.js,v $
# $Revision: 1.2 $
# $Author: brobertson $
# $Date: 2003/03/11 03:02:14 $
######################################################################################
*/

/*
##################################################
# useful functions for form element values
# Blair Robertson -> (c) 2001/2002
##################################################
*/

// Convenience function for submitting the form, useful from hrefs
function submit_form() {
	var f = document.main_form;
	// make sure we clean any stuff up before we submit
	if (form_on_submit()) {
		f.submit();
	}
}// end submit_form()

// Convenience function for setting hidden fields in the form
function set_hidden_field(name, value) {
	var f = document.main_form;
	if (f.elements[name]) {
		f.elements[name].value = value;
	}
}// end set_hidden_field()

// Convenience function for setting text fields in the form
// NOTE: works the same way as a hidden field so just alias that fn
set_text_field = set_hidden_field;


// return the form element that is represented by the passed name
function get_element(name) {
	var f = document.main_form;

	if (f.elements[name]) {
		return f.elements[name];

	} else {
		return null;

	}// endif

}// end get_element()


// returns the value for a field in the form
function get_element_value(name) {
	var f = document.main_form;
	return (f.elements[name]) ? element_value(f.elements[name]) : '';
}// get_element_value()



//////////////////////////////////////////////////////////////////////////////////////////
// Returns the value for any type of form element
//
// if select box or group of radio buttons returns the selected/checked value(s) 
//    -> for multi-select boxes returns an array of selected values
// if array of any other type of elements returns the value of the first element in array
function element_value(element) {
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


 ////////////////////////////////////////////////////////////
// Given a select box reference, returns the current text
function get_combo_text(element) {
	// just to make sure
	if (element.type != "select-one" && element.type != "select-multiple") return;

	return element.options[element.selectedIndex].text;

}// end get_combo_text();


 //////////////////////////////////////////////////////////////////////////////////////////
// Checks a specific radio button with the element, that has the passed value
function check_radio_button(element, field_val) {

	for(var i = 0; i < element.length; i++) {
		if (element[i].value == field_val) {
			element[i].checked = true;
			break;
		}
	}// end for

	return;

}// end check_radio_button()

 ///////////////////////////////////////////////////////////////////
// Sets the selected var for the option with the passed value
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

}//highlight_combo_value()

 //////////////////////////////////////////////////////////////////////////////////////////
// Moves the selected elements in a select box (can be multi-select) up or down one place
function move_combo_selection(element, move_up) {

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



  ////////////////////////////////////////////////////////////////////////////
 // used by the date_box to verify that the date the user entered is correct
// and to set the hidden var
function check_date(date_name, show_time) {
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








