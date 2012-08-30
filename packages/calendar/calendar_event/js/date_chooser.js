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
* $Id: date_chooser.js,v 1.18 2012/08/30 00:57:28 ewang Exp $
*
*/

var currentField = null;
var oldField = null;


function processEndDateBlur(elt, prefix)
{
	if (fieldGroupBlurred(elt)) {
		updateDurationValuesByPrefix(prefix);
		updateStartDate(prefix);
	}

}//end processEndDateBlur()


function processStartDateBlur(elt, prefix)
{
	if (fieldGroupBlurred(elt)) {
		updateEndDate(prefix);
	}

}//end processStartDateBlur()


function updateEndDate(name)
{
	endD = getDateFromField(name+'_end');
	startD = getDateFromField(name+'_start');
	if (endD < startD) {
		setDateField(name+'_end', startD);
	}

}//end updateEndDate()


function updateStartDate(name)
{
	endD = getDateFromField(name+'_end');
	startD = getDateFromField(name+'_start');
	if (endD < startD) {
	  setDateField(name+'_start', endD);
	}

}//end updateStartDate()


function updateDurationValuesByPrefix(prefix)
{
	if (isChecked(prefix + '_start_time_enabled')) {
		d = new Date(document.getElementById(prefix + '_start_year').value, document.getElementById(prefix + '_start_month').value-1, document.getElementById(prefix + '_start_day').value, ((parseInt(document.getElementById(prefix + '_start_hours').value)%12) + (12 * document.getElementById(prefix + '_start_is_pm').value)) % 24, document.getElementById(prefix + '_start_minutes').value, 0);
	} else {
		d = new Date(document.getElementById(prefix + '_start_year').value, document.getElementById(prefix + '_start_month').value-1, document.getElementById(prefix + '_start_day').value);
	}

	if (isChecked(prefix + '_duration_enabled')) {

		var newDate = new Date();
		var addSeconds = 0;
		switch(document.getElementById(prefix + '_duration_units').value) {
			case 'd':
				addSeconds = document.getElementById(prefix + '_duration').value * 86400;
			break;

			case 'h':
				addSeconds = document.getElementById(prefix + '_duration').value * 3600;
			break;

			case 'i':
				addSeconds = document.getElementById(prefix + '_duration').value * 60;
			break;

		}

		// if only days - make sure 3 days becomes, say, 7-9th
		if (!isChecked(prefix + '_end_time_enabled')) {
			addSeconds -= 86400;
		}

		newDate.setTime(d.valueOf() + addSeconds * 1000);

		document.getElementById(prefix + '_end_year').value = newDate.getFullYear();
		document.getElementById(prefix + '_end_month').value = newDate.getMonth() + 1;
		document.getElementById(prefix + '_end_day').value = newDate.getDate();

		if (isChecked(prefix + '_start_time_enabled')) {
			document.getElementById(prefix + '_end_hours').value = ((newDate.getHours() % 12 == 0) ? 12 : newDate.getHours() % 12);
			document.getElementById(prefix + '_end_is_pm').selectedIndex = (newDate.getHours() >= 12);
			document.getElementById(prefix + '_end_minutes').value = make2digits(newDate.getMinutes());
		}
	} else {
		var endDate = new Date();

		if (isChecked(prefix + '_end_time_enabled')) {
			endDate = new Date(document.getElementById(prefix + '_end_year').value, document.getElementById(prefix + '_end_month').value-1, document.getElementById(prefix + '_end_day').value, ((parseInt(document.getElementById(prefix + '_end_hours').value)%12) + (12 * document.getElementById(prefix + '_end_is_pm').value)) % 24, document.getElementById(prefix + '_end_minutes').value, 0);
		} else {
			endDate = new Date(document.getElementById(prefix + '_end_year').value, document.getElementById(prefix + '_end_month').value-1, document.getElementById(prefix + '_end_day').value);
		}

		// number of minutes between the two dates - valueOf() is returned in milli-sec's,
		// hence the extra division by 1000
		var dateDiff = (endDate.valueOf() - d.valueOf()) / (1000 * 60);
		if (!isChecked(prefix + '_end_time_enabled')) {
			dateDiff += 1440;
		}

		if ((dateDiff % 1440 == 0) && (dateDiff > 0)) {
			document.getElementById(prefix + '_duration_units').value = 'd';
			document.getElementById(prefix + '_duration').value = dateDiff / 1440;
		} else if ((dateDiff % 60 == 0) && (dateDiff > 0)) {
			document.getElementById(prefix + '_duration_units').value = 'h';
			document.getElementById(prefix + '_duration').value = dateDiff / 60;
		} else {
			document.getElementById(prefix + '_duration_units').value = 'i';
			document.getElementById(prefix + '_duration').value = dateDiff;
		}

	}

}//end updateDurationValuesByPrefix()


function processEndClick(box, prefix)
{
	if (box.checked) {
		enableDateField(prefix+'_end');
		enableField(prefix+'_duration_enabled');
		enableField(prefix+'_end_date_enabled');
		enableField(prefix+'_duration_units');
		enableField(prefix+'_duration');
		enableField(prefix+'_end_time_enabled');
		if (isChecked(prefix+'_end_time_enabled')) {
			enableTimeField(prefix+'_end');
		}
	} else {
		disableDateField(prefix+'_end');
		disableTimeField(prefix+'_end');
		disableField(prefix+'_duration_enabled');
		disableField(prefix+'_end_date_enabled');
		disableField(prefix+'_duration_units');
		disableField(prefix+'_duration');
		disableField(prefix+'_end_time_enabled');
	}

}//end processEndClick()


function processEndEnabledClick(elt, name)
{
	if (elt.value == 1) {
		enableDateField(name+'_end');
		if (isChecked(name+'_start_time_enabled')) {
			enableTimeField(name+'_end');
		}
	} else {
		disableDateField(name+'_end');
		disableTimeField(name+'_end');
	}

}//end processEndEnabledClick()


function processStartTimeEnabledClick(elt, name)
{
	if (elt.checked) {
		enableTimeField(name+'_start');
		if (isChecked(name+'_end_date_enabled')) {
			enableTimeField(name+'_end');
		}
	} else {
		disableTimeField(name+'_start'); disableTimeField(name+'_end');
	}

}//end processStartTimeClick()


function processEndTimeEnabledClick(elt, name)
{
	if (elt.checked) {
		enableTimeField(name+'_end');
		enableDateField(name+'_end');
		checkBox(name+'_end_date_enabled');
		enableTimeField(name+'_start');
	} else {
		disableTimeField(name+'_end');
		disableTimeField(name+'_start');
	}

}//end processEndTimeClick()



function processKeyEvent(elt)
{
	if (!window.event) return;
	key = window.event.keyCode;
	if ((key==43) && (elt.value==Number(elt.value))) {
		if (elt.name.indexOf('year') != -1)				max_value = 2030;
		else if (elt.name.indexOf('day') != -1)			max_value = 31;
		else if (elt.name.indexOf('hours') != -1)		max_value = 23;
		else if (elt.name.indexOf('minutes') != -1)		max_value = 59;


		if (elt.value < max_value) {
			elt.value=make2digits((Number(elt.value))+1);
		}
	    window.event.keyCode=null;
		elt.select();
	}
	if ((key==45) && (elt.value==Number(elt.value))) {
		if (elt.name.indexOf('year') != -1)				min_value = 1970;
		else if (elt.name.indexOf('day') != -1)			min_value = 1;
		else if (elt.name.indexOf('hours') != -1)		min_value = 0;
		else if (elt.name.indexOf('minutes') != -1)		min_value = 0;

		if (elt.value > min_value) {
			elt.value=make2digits((Number(elt.value))-1);
		}
		window.event.keyCode=null;
		elt.select();
  }
  return true;

}//end processKeyEvent()


function ______________HELPERS_____________() {}

function make2digits(num) {
	if (num < 10) {
		return "0"+parseInt(num);
	} else {
		return num;
	}

}//end make2digits()


function fieldGroupBlurred(oldField) {
	if ((currentField === null) || (oldField === null)) return true;
	fieldGroupSuffixes = Array('year','month','day','hours','minutes','is_pm');
	for (i=0; i < fieldGroupSuffixes.length; i++) {
		if ((splitPoint = oldField.id.indexOf(fieldGroupSuffixes[i])) != -1) {
			baseName = oldField.id.substring(0, splitPoint-1);
			break;
		}
	}
	return (currentField.id.indexOf(baseName) == -1);

}//end fieldGroupBlurred()


function enableField(name) {
	if ((elt = document.getElementById(name)) !== null) {
		elt.disabled = 0;
	}
}


function disableField(name) {
	if ((elt = document.getElementById(name)) !== null) {
		elt.disabled = 1;
	}
}


var dateComponents = Array('_year','_month','_day');

function enableDateField(name) {
	for (i=0; i<dateComponents.length; i++) {
		if ((elt = document.getElementById(name+dateComponents[i])) !== null) {
			elt.disabled = 0;
		}
	}

}//end enableDateField()


function disableDateField(name) {
	for (i=0; i<dateComponents.length; i++) {
		if ((elt = document.getElementById(name+dateComponents[i])) !== null) {
			elt.disabled = 1;
		}
	}

}//end disableDateField()


function enableTimeField(name) {
	if (document.getElementById(name+'_hours') === null) {
		alert("hours not found for " + name);
	}
	document.getElementById(name+'_hours').disabled=0;
	document.getElementById(name+'_minutes').disabled=0;
	document.getElementById(name+'_is_pm').disabled=0;
	checkBox(name+'_time_enabled');

}//end enableTimeField()


function disableTimeField(name) {
	document.getElementById(name+'_hours').disabled=1;
	document.getElementById(name+'_minutes').disabled=1;
	document.getElementById(name+'_is_pm').disabled=1;
	uncheckBox(name+'_time_enabled');

}//end disableTimeField()


function checkBox(id) {
	if ((elt = document.getElementById(id)) !== null) {
		elt.checked=1;
	} else {
		alert("Javascript error:  could not find box " + id + " to check");
	}
}//end checkBox()


function uncheckBox(id) {
	if ((elt = document.getElementById(id)) !== null) {
		elt.checked=0;
	} else {
		alert("Javascript error:  could not find box " + id + " to uncheck");
	}
}//end uncheckBox()


function isChecked(id) {
	if ((elt = document.getElementById(id)) !== null) {
		return elt.checked;
	} else {
		alert("Javascript error:  could not find box " + id + " so can't look at its status");
		return 0;
	}
}//end isChecked()

function getDateFromField(name) {
	d = new Date(document.getElementById(name+'_year').value, Number(document.getElementById(name+'_month').value)-1, document.getElementById(name+'_day').value);
	hoursVal = parseInt(document.getElementById(name+'_hours').value);
	if ((document.getElementById(name+'_is_pm').selectedIndex == 1) && (hoursVal < 12)) {
		hoursVal += 12;
	}
	if ((document.getElementById(name+'_is_pm').selectedIndex === 0) && (hoursVal == 12)) {
		hoursVal = 0;
	}
	d.setHours(hoursVal);
	d.setMinutes(document.getElementById(name+'_minutes').value);
	return d;

}//end getDateFromField()


function setDateField(fieldName, dateVal) {
	document.getElementById(fieldName+'_year').value = dateVal.getFullYear();
	document.getElementById(fieldName+'_month').value = dateVal.getMonth()+1;
	document.getElementById(fieldName+'_day').value = dateVal.getDate();
	if ((hourElt = document.getElementById(fieldName+'_hours')) !== null) {
		document.getElementById(fieldName+'_is_pm').selectedIndex = ((dateVal.getHours() >= 12) ? 1 : 0);
		if (dateVal.getHours() % 12 === 0) {
			hourElt.value = 12;
		} else {
			hourElt.value = dateVal.getHours() % 12;
		}
		document.getElementById(fieldName+'_minutes').value = make2digits(dateVal.getMinutes());
	}

}//end setDateField()


function ____________VALIDATION______________() {}

function dayOK(elt) {
	dayValue = parseInt(elt.value);
	return ((dayValue !== null) && (dayValue > 0) && (dayValue <= 31));
}//end dayOK()


function validateDay(elt) {
	if (!dayOK(elt)) {
		alert("You entered an invalid day of the month");
		elt.focus();
		return false;
	} else {
		return true;
	}
}//end validateDay()


function yearOK(elt) {
	yearVal = parseInt(elt.value);
	return (yearVal !== null)  && (yearVal > 0) && ((yearVal < 100) || ((yearVal >= 1970) && (yearVal <= 2030)));

}//end yearOK()


function validateYear(elt) {
	if (!yearOK(elt)) {
		alert("You entered an invalid year");
		elt.value = '';
		elt.focus();
		return false;
	} else {
		yearVal = parseInt(elt.value);
		if (yearVal < 100) {
			yearVal = yearVal + 2000;
			elt.value = yearVal + 2000;
		}
	}
	return true;

}//end validateYear()


function minutesOK(elt) {
	minutesValue = parseInt(elt.value);
	return !((minutesValue === null) || (minutesValue < 0) || (minutesValue > 59));

}//end minutesOK()


function validateMinutes(elt) {
	if (!minutesOK(elt)) {
		alert("You entered an invalid time");
		elt.focus();
		return false;
	} else {
		return true;
	}

}//end validateMinutes()


function hoursOK(elt) {
	hoursValue = parseInt(elt.value);
	return (hoursValue >= 0) && (hoursValue <= 23);

}//end hoursOK()


function validateHours(elt) {
	hoursValue = parseInt(elt.value);
	if (!hoursOK(elt)) {
		alert("You entered an invalid time");
		elt.focus();
		return false;
	} else if ((hoursValue > 12) && (hoursValue < 24)) {
		elt.value = hoursValue - 12;
		if ((pmElt = document.getElementById(elt.getAttribute('id').substring(0, elt.getAttribute('id').length-5)+'is_pm')) !== null) {
			pmElt.selectedIndex=1;
		} else {
			alert("Couldn't find " + elt.getAttribute('id').substring(0, elt.getAttribute('id').length-5)+'is_pm');
		}
	}
	return true;

}//end validateHours()



