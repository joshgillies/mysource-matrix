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
* $Id: metadata_field_hierarchy.js,v 1.2 2008/07/17 00:42:04 bshkara Exp $
*
*/

function setInputsEnabled(parent, default_box, cascade_box, enabled)
{
	var inputs = parent.getElementsByTagName('INPUT');
	for (var i=0; i < inputs.length; i++) {
		if (inputs[i].name !== default_box || inputs[i].name !== cascade_box) {
			inputs[i].disabled = !enabled;
		}
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
function setSelection(prefix, values, drill, selected)
{
	var select = document.getElementById(prefix);
	if ((select !== null) && (typeof select.options != "undefined")) {

		if (drill) {

			// add elements to the receptacle because we are handling a drill-down view
			if (selected) {
				// add elements
				select.options.length = 0;
				for (var i=0; i < values.length; i++) {
					select.options[select.options.length] = new Option(values[i], values[i]);
					select.options[i].selected = true;
				}
			} else {
				// deselect elements
				for (var i=0; i < select.options.length; i++) {
					select.options[i].selected = false;
				}
			}

		} else {

			// select/deselect elements because we are handling a flat view
			for (var i=0; i < select.options.length; i++) {
				if (in_array(select.options[i].value, values)) {
					select.options[i].selected = selected;
				}
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
function handleDefaultClick(defaultCheckbox, prefix, default_vals, non_default_vals, drill_down)
{
	if (defaultCheckbox.checked) {
		setSelection(prefix, default_vals, drill_down, true);
		if (!drill_down) {
			setSelection(prefix, non_default_vals, drill_down, false);
		}
	} else {
		if (drill_down) {
			setSelection(prefix, default_vals, drill_down, false);
		}
	}
	setInputsEnabled(document.getElementById(prefix+'_field'), prefix+'_default', prefix+'_cascade_value', !defaultCheckbox.checked);
}
