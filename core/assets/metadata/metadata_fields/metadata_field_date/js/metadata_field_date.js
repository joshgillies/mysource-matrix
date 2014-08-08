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
* $Id: metadata_field_date.js,v 1.8 2012/08/30 01:09:09 ewang Exp $
*
*/

function setInputsEnabled(parent, enabled)
{
	var inputs = parent.getElementsByTagName('INPUT');
	for (var i=0; i < inputs.length; i++) {
		inputs[i].disabled = !enabled;
	}
	var selects = parent.getElementsByTagName('SELECT');
	for (var i=0; i < selects.length; i++) {
		selects[i].disabled = !enabled;
	}
}

function handleMetadataDatetimeDefaultClick(defaultCheckbox, prefix, defaultDateTime, defaultKeyword)
{
	var dateComps = ['c', 'd', 'm', 'y', 'h', 'i', 's'];
	if (defaultCheckbox.checked) {
		if (defaultKeyword !== null) {
			for (i in dateComps) {
				var elt = document.getElementById(prefix+'_datetimevalue_'+dateComps[i]);
				if (elt !== null) {
					elt.selectedIndex = 0;
					elt.disabled = true;
				}
			}
			document.getElementById(prefix+'_repkeys').value = defaultKeyword;
		} else if (defaultDateTime != null) {
			for (i in dateComps) {
				var elt = document.getElementById(prefix+'_datetimevalue_'+dateComps[i]);
				if (elt !== null) {
					elt.value = defaultDateTime[dateComps[i]];
					elt.disabled = true;
				}
			}
			var repKeys = document.getElementById(prefix+'_repkeys');
			if (repKeys) {
				repKeys.value = '';
			}
		}
	} else {
		for (i in dateComps) {
			var elt = document.getElementById(prefix+'_datetimevalue_'+dateComps[i]);
			if (elt !== null) {
				elt.disabled = false;
			}
		}
	}

	elt = document.getElementById(prefix+'_repkeys');
	if (elt !== null) {
		elt.disabled = defaultCheckbox.checked;
	}
	
	period = document.getElementById(prefix+'_period');
	if (period !== null) {
		period.disabled = defaultCheckbox.checked;
	}
	
	duration = document.getElementById(prefix+'_duration');
	if (duration !== null) {
		duration.disabled = defaultCheckbox.checked;
	}
}

