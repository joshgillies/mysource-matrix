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
* $Id: search_and_replace.js,v 1.6 2012/08/30 00:59:16 ewang Exp $
*
*/


function toggleTBodyDisplay(elt)
{
	var table = elt.parentNode;
	while (table.tagName != 'TABLE') {
		table = table.parentNode;
	}
	tb = table.firstChild;
	while (tb.tagName != 'TBODY') {
		tb = tb.nextSibling;
	}
	tb.style.display = (tb.style.display == 'none') ? '' : 'none';
	updateGlobalLinks();
}

function updateGlobalLinks()
{
	var container = document.getElementById('confirmations-container');
	var allTbodies = container.getElementsByTagName('TBODY');
	var text = js_translate('collapse_all');
	for (var i=0; i < allTbodies.length; i++) {
		if (allTbodies[i].style.display == 'none') {
			text = js_translate('expand_all');
			break;
		}
	}
	document.getElementById('expand-collapse-link-top').innerHTML = text;
	document.getElementById('expand-collapse-link-bottom').innerHTML = text;
}

function toggleAllTbodyDisplays(link)
{
	var container = document.getElementById('confirmations-container');
	var allTbodies = container.getElementsByTagName('TBODY');
	var display = (link.innerHTML == js_translate('collapse_all')) ? 'none' : '';
	for (var i=0; i < allTbodies.length; i++) {
		allTbodies[i].style.display = display;
	}
	text = (link.innerHTML == js_translate('collapse_all')) ? js_translate('expand_all') : js_translate('collapse_all');
	document.getElementById('expand-collapse-link-top').innerHTML = text;
	document.getElementById('expand-collapse-link-bottom').innerHTML = text;
}

var updatingParent = false;

function toggleTBodyCheckboxes(elt)
{
	if (updatingParent) return true;
	var table = elt.parentNode;
	while (table.tagName != 'TABLE') {
		table = table.parentNode;
	}
	var tbody = table.getElementsByTagName('TBODY')[0];
	var inputs = tbody.getElementsByTagName('INPUT');
	for (var i=0; i < inputs.length; i++) {
		if (inputs[i].type.toLowerCase() == 'checkbox') {
			inputs[i].checked = elt.checked;
		}
	}
}
function updateParentCheckboxes(elt)
{
	var table = elt.parentNode;
	while (table.tagName != 'TABLE') {
		if (table.tagName == 'THEAD') {
			// we want the enclosing table, not the one whose head we're in, so skip an extra level
			table = table.parentNode;
		}
		table = table.parentNode;
	}
	var thead = table.getElementsByTagName('THEAD')[0];
	var theadInputs = thead.getElementsByTagName('INPUT');
	for (var i=0; i < theadInputs.length; i++) {
		if (theadInputs[i].type.toLowerCase() == 'checkbox') {
			var theadCheckbox = theadInputs[i];
			break;
		}
	}
	if (!elt.checked) {
		if (theadCheckbox.checked) {
			updatingParent = true;
			theadCheckbox.click();
			updatingParent = false;
		}
	} else {
		var tbody = table.getElementsByTagName('TBODY')[0];
		var tbodyInputs = tbody.getElementsByTagName('INPUT');
		for (var i=0; i < tbodyInputs.length; i++) {
			if (tbodyInputs[i].type.toLowerCase() == 'checkbox') {
				if (tbodyInputs[i].checked != elt.checked) {
					return;
				}
			}
		}
		// if we get to here then all the tbody checkboxes are in the same state as elt
		if (theadCheckbox.checked != elt.checked) {
			updatingParent = true;
			theadCheckbox.click();
			updatingParent = false;
		}
	}
}

function setAllCheckboxes(link)
{
	var container = document.getElementById('confirmations-container');
	var allInputs = container.getElementsByTagName('INPUT');
	var checkState = (link.innerHTML == js_translate('select_all')) ? true : false;
	for (var i=0; i < allInputs.length; i++) {
		if (allInputs[i].type.toLowerCase() == 'checkbox') {
			allInputs[i].checked = checkState;
		}
	}
	text = (link.innerHTML == 'Select All') ? js_translate('deselect_all') : js_translate('select_all');
	document.getElementById('select-deselect-link-top').innerHTML = text;
	document.getElementById('select-deselect-link-bottom').innerHTML = text;
}
