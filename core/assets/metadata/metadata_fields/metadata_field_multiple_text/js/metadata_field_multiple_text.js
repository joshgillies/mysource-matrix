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
* $Id: metadata_field_multiple_text.js,v 1.1 2006/12/28 01:48:14 lwright Exp $
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

function handleDefaultClick(defaultCheckbox, prefix, default_vals)
{
	var inputs = document.getElementById(prefix+'_field').getElementsByTagName('INPUT');

	if (defaultCheckbox.checked) {
		for (i = 0; i < default_vals.length; i++) {
			if (i == inputs.length - 1) {
				expandOptionList(inputs[i]);
				inputs = document.getElementById(prefix+'_field').getElementsByTagName('INPUT');
			}

			inputs[i].value = default_vals[i];
		}
	}
	setInputsEnabled(document.getElementById(prefix+'_field'), !defaultCheckbox.checked);
}