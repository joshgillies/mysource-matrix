function make2digits(num) {
	if (num < 10) {
		return "0"+num;
	} else {
		return num;
	}
		
}//end make2digits()


function processEndEnabledClick(elt, name) {
	if (elt.checked) {
		enableDateField(name+'_end'); 
		if (isChecked(name+'_start_time_enabled')) {
			enableTimeField(name+'_end'); 
		}
	} else { 
		disableDateField(name+'_end'); 
		disableTimeField(name+'_end'); 
	}
	
}//end processEndEnabledClick()


function processStartTimeClick(elt, name) {
	if (elt.checked) {
		enableTimeField(name+'_start');
		if (isChecked(name+'_end_date_enabled')) {
			enableTimeField(name+'_end'); 
		}
	} else {
		disableTimeField(name+'_start'); disableTimeField(name+'_end');
	}
	
}//end processStartTimeClick()


function processEndTimeClick(elt, name) {
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
	return (yearVal !== null)  && (yearVal > 0) && ((yearVal < 100) || ((yearVal >= 1900) && (yearVal < 3000)));
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
	
}//end validateFutureYear()


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


function updateEndDate(name) {
	endD = getDateFromField(name+'_end');
	startD = getDateFromField(name+'_start');
	//alert("Got end and start dates for updating end date");
	if (endD < startD) {
		setDateField(name+'_end', startD);
	}
	
}//end updateEndDate()


function updateStartDate(name) {
	endD = getDateFromField(name+'_end');
	startD = getDateFromField(name+'_start');
	//alert("Got end and start dates for updating start date");
	if (endD < startD) {
	  setDateField(name+'_start', endD);
	}
	  
}//end updateStartDate()

function processKeyEvent(elt) {
  key = window.event.keyCode; 
  if ((key==43) && (elt.value==Number(elt.value))) { 
    elt.value=(Number(elt.value))+1; 
    window.event.keyCode=null;
  } 
  if ((key==45) && (elt.value==Number(elt.value))) { 
    elt.value=(Number(elt.value))-1; 
    window.event.keyCode=null;
  }
  return true
}
