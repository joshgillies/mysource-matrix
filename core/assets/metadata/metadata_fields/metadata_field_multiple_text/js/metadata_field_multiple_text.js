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
* $Id: metadata_field_multiple_text.js,v 1.3 2012/08/30 01:09:09 ewang Exp $
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

function handleDefaultClickMT(defaultCheckbox, prefix, default_vals)
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

		// Ooh, this is interesting. :-D What this part does is start from
		// whatever index we left off from (because we ran out of default values)
		// and replace them with blanks
		for (; i < inputs.length; i++) {
			inputs[i].value = '';
		}
	}
	setInputsEnabled(document.getElementById(prefix+'_field'), !defaultCheckbox.checked);
}