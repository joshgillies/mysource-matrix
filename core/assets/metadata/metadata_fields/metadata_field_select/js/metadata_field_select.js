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
function in_array(elt, ar)
{
	for (var i=0; i < ar.length; i++) {
		if (ar[i] == elt) return true;
	}
	return false;
}
function setSelection(prefix, values, selected)
{
	select = document.getElementById(prefix);
	if ((select !== null) && (typeof select.options != "undefined")) {
		for (var i=0; i < select.options.length; i++) {
			if (in_array(select.options[i].value, values)) {
				select.options[i].selected = selected;
			}
		}
	} else {
		for (var i=0; i < values.length; i++) {
			var obj = document.getElementById(prefix+'_'+values[i]);
			if (null === obj) {
				alert(prefix+'_'+values[i]+' is null!');
			}
			if (obj.tagName == 'OPTION') {
				obj.selected = selected;
			}
			if ((obj.tagName == 'INPUT') && (obj.type == 'checkbox' || obj.type == 'radio')) {
				obj.checked = selected;
			}
		}
	}
}
function handleDefaultClick(defaultCheckbox, prefix, default_vals, non_default_vals)
{
	if (defaultCheckbox.checked) {
		setSelection(prefix, default_vals, true)
		setSelection(prefix, non_default_vals, false);
	}
	setInputsEnabled(document.getElementById(prefix+'_field'), !defaultCheckbox.checked);
}