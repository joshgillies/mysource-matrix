var weekDays = Array('Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday');

var months = Array('January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September', 'October', 'November', 'December');

var monthLengths = Array(0,31,28,31,30,31,30,31,31,30,31,30,31);

var suffixes = Array('th','st','nd','rd','th','th','th','th','th','th');


function setSpanContents(eltId, text) {
	elt = document.getElementById(eltId);
	while (elt.childNodes.length > 0)
		elt.removeChild(elt.firstChild);
	elt.appendChild(document.createTextNode(text));
	
}//end setSpanContents()


function ordinalSuffix(num) {
	if ((10 < num) && (num < 20))
		return num + 'th';
	else
		return num + suffixes[num % 10];
	
}//end ordinalSuffix()


/**
 * Used by 'xth last' fields in month freq to stop showing '1st last', by suppressing the '1st'
 */
function reverseOrdinalSuffix(num) {
	if (num == 1)
		return '';
	else
		return ordinalSuffix(num) + ' ';
	
}//end reverseOrdinalSuffix()


function daysInFebruary(year){
	// February has 29 days in any year evenly divisible by four,
	// EXCEPT for centennial years which are not also divisible by 400.
	return (((year % 4 == 0) && ( (!(year % 100 == 0)) || (year % 400 == 0))) ? 29 : 28 );
	
}//end daysInFebruary()


function getMonthLength(month, year) {
	if (month == 2)
		return daysInFebruary(year);
	else
		return monthLengths[month];
	
}//end getMonthLength()


function updateValues(prefix) {
	d = new Date(document.getElementById(prefix + '_start_year').value, document.getElementById(prefix + '_start_month').value - 1, document.getElementById(prefix + '_start_day').value);

	document.getElementById(prefix + '_month_date_warning').style.display = 'none';
	document.getElementById(prefix + '_month_week_warning').style.display = 'none';
	document.getElementById(prefix + '_reverse_month_date_warning').style.display = 'none';
	document.getElementById(prefix + '_reverse_month_week_warning').style.display = 'none';

	setSpanContents(prefix + '_week_day', weekDays[d.getDay()]);
	setSpanContents(prefix + '_week_day_2', weekDays[d.getDay()]);
	setSpanContents(prefix + '_week_day_3', weekDays[d.getDay()]);
	setSpanContents(prefix + '_week_day_4', weekDays[d.getDay()]);

	setSpanContents(prefix + '_month_date_ord', ordinalSuffix(d.getDate()));
	if (d.getDate() > 28)
		document.getElementById(prefix + '_month_date_warning').style.display = 'inline';
	var weekNumber = parseInt(((d.getDate()-1)/7)+1);
	setSpanContents(prefix + '_month_week_ord', ordinalSuffix(weekNumber));

	if (weekNumber > 4)
		document.getElementById(prefix + '_month_week_warning').style.display = 'inline';
	var reverseDate = getMonthLength(d.getMonth()+1, d.getFullYear()) - d.getDate()+1;
	setSpanContents(prefix + '_reverse_month_date_ord', reverseOrdinalSuffix(reverseDate));
	if (reverseDate > 28)
		document.getElementById(prefix + '_reverse_month_date_warning').style.display = 'inline';

	reverseWeekNumber = parseInt((getMonthLength(d.getMonth()+1, d.getFullYear()) - d.getDate()) / 7)+1;
	setSpanContents(prefix + '_reverse_month_week_ord', reverseOrdinalSuffix(reverseWeekNumber));
	if (reverseWeekNumber > 4)
		document.getElementById(prefix + '_reverse_month_week_warning').style.display = 'inline';

}//end updateValues()
			

var freqs = Array('Daily', 'Weekly', 'Monthly');
		
function showFreqOptions(prefix, freqName) {
	updateValues(prefix);
	for (i=0; i<freqs.length; i++) {
		if ((elt = document.getElementById(prefix+freqs[i]+'Options')) != null) {
			if (freqs[i] == freqName)
				elt.style.display = 'block';
			else
				elt.style.display = 'none';
		} else {
			alert("Javascript error: Could not find frequency: " + freqs[i]);
		}
	}
}//end showFreqOptions()

