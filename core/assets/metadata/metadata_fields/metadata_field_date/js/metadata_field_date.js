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
	var dateComps = ['d', 'm', 'y', 'h', 'i', 's'];
	if (defaultCheckbox.checked) {
		if (defaultKeyword !== null) {
			for (i in dateComps) {
				var elt = document.getElementById(prefix+'_datetimevalue['+dateComps[i]+']');
				if (elt !== null) {
					elt.value = 1;
					elt.disabled = true;
				}
			}
			document.getElementById(prefix+'_repkeys').value = defaultKeyword;
		} else {
			for (i in dateComps) {
				var elt = document.getElementById(prefix+'_datetimevalue['+dateComps[i]+']');
				if (elt !== null) {
					elt.value = defaultDateTime[dateComps[i]];;
					elt.disabled = true;
				}
			}
			document.getElementById(prefix+'_repkeys').value = '';
		}
	} else {
		for (i in dateComps) {
			var elt = document.getElementById(prefix+'_datetimevalue['+dateComps[i]+']');
			if (elt !== null) {
				elt.disabled = false;
			}
		}
	}

	elt = document.getElementById(prefix+'_repkeys');
	if (elt !== null) {
		elt.disabled = defaultCheckbox.checked;
	}
}