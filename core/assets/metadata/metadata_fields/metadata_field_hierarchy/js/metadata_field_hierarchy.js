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
* $Id: metadata_field_hierarchy.js,v 1.5 2012/08/30 01:09:09 ewang Exp $
*
*/

function setHierarchyInputsEnabled(prefix, parent, default_box, cascade_box, enabled)
{
	var inputs = parent.getElementsByTagName('INPUT');
	for (var i=0; i < inputs.length; i++) {
		var matched = inputs[i].name.search(prefix);
		if (matched != -1) {
			inputs[i].disabled = !enabled;
		}

		if (inputs[i].name === default_box || inputs[i].name === cascade_box) {
			inputs[i].disabled = false;
		}
	}

	var selects = parent.getElementsByTagName('SELECT');
	for (var i=0; i < selects.length; i++) {
		var matched = selects[i].name.search(prefix);
		if (matched != -1) {
			selects[i].disabled = !enabled;
		}
	}
}
function in_array(elt, ar)
{
	for (var i=0; i < ar.length; i++) {
		if (ar[i] == elt) return true;
	}
	return false;
}
function setSelectionHierarchy(prefix, keys, default_values, drill, selected)
{
	var select = document.getElementById(prefix);
	if ((select !== null) && (typeof select.options != "undefined")) {

		if (drill) {

			// add elements to the receptacle because we are handling a drill-down view
			if (selected) {
				// add elements
				select.options.length = 0;
				for (var i=0; i < keys.length; i++) {
					if (typeof default_values[i] !== 'undefined' && default_values[i] !== null) {
						var text = keys[i] + '. ' + default_values[i];
					} else {
						var text = keys[i];
					}
					select.options[select.options.length] = new Option(text, keys[i]);
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
				if (in_array(select.options[i].value, keys)) {
					select.options[i].selected = selected;
				}
			}

		}
	} else {
		for (var i=0; i < keys.length; i++) {
			var obj = document.getElementById(prefix+'_'+keys[i]);
			if (null === obj) {
				alert(prefix+'_'+keys[i]+' is null!');
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
function handleDefaultClickHierarchy(defaultCheckbox, prefix, default_keys, default_values, non_default_keys, drill_down)
{
	if (defaultCheckbox.checked) {
		setSelectionHierarchy(prefix, default_keys, default_values, drill_down, true);
		if (!drill_down) {
			setSelectionHierarchy(prefix, non_default_keys, default_values, drill_down, false);
		}
	} else {
		if (drill_down) {
			setSelectionHierarchy(prefix, default_keys, default_values, drill_down, false);
		}
	}
	setHierarchyInputsEnabled(prefix, document.getElementById(prefix+'_field'), prefix+'_default', prefix+'_cascade_value', !defaultCheckbox.checked);
}
