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
* $Id: state.js,v 1.8 2008/08/29 00:45:32 hnguyen Exp $
*
*/

// This file is to check for form changes and notifies
// the user if the user moves away from the page

// Register a change in the hidden field 'changes'
function changesMade()
{
	document.forms[0]['changes'].value = '1';
	return true;
}

// Record the field (used privately)
function recordField(count, value) {
	recordState = '';
	recordState += count;
	recordState += ':';
	recordState += value;
	recordState += ';';

	return recordState;
}

// Saves the state of the current form and returns
// a string summary of the form
function saveState()
{
	var currentForm = document.forms[0];
	var currentState = '';
	
	for (i=0; i < currentForm.elements.length; i++) {
		var el = currentForm.elements[i];
		if (el.type == 'hidden' || el.type == 'password') {
			// Don't save hidden fields and password fields
		} else if (el.name == 'state' || el.name == 'changes') {
			// Don't save our control fields
		} else if (el.name == 'screen_menu' || el.name == 'screen_menu_go') {
			// Don't save the screen navigation controls
		} else if (el.type == 'button' || el.type == 'submit' || el.type == 'reset') {
			// Don't save buttons
		} else if (el.type =='checkbox' || el.type == 'radio') {
			// Save the checkbox/radio
			if (el.checked) {
				currentState += recordField(i, '1');
			} else {
				currentState += recordField(i, '0');
			}
		} else if (el.type == 'select-one') {
			currentState += recordField(i, el.selectedIndex);
		} else if (el.type == 'select-multiple') {
			var ssm = '';
			for (j=0; j < el.options.length; j++) {
				if (el.options[j].selected) {
					ssm += el.options[j].value;
					ssm += '-';
				}
			}
			currentState += recordField(i, ssm);
		} else {
			// Finally save text fields, text boxes etc.
			currentState += recordField(i, el.value);
		}
	}

	return currentState;
}

// Resets the hidden field 'changes' to show no changes have been made
function resetChanges()
{
	document.forms[0]['changes'].value = '0';
	return true;
}

function makeChanges()
{
	resetChanges();
	document.forms[0]['state'].value = saveState();
}

onbeforeunload = function(e) {
	if (document.getElementById("sq_commit_button")) {
		if (document.forms[0]['changes'].value == '1')
		{
			if (document.forms[0]['allowconfirm'].value == '1')
			{
				if (document.forms[0]['state'].value != saveState())
				{
					return "This page has unsaved changes. If you move away from this page, those changes will be lost!";
				}
			}
		}
	}
}

// Some of these functions below work on Firefox but not on IE
// In which case, you need to add them on the element directly
onload = function(e) {
	resetChanges();
	document.forms[0]['state'].value = saveState();
}

onchange = function(e) {
	changesMade();
}

onclick = function(e) {
	changesMade();
}

onkeypress = function(e) {
	changesMade();
}

onsubmit = function(e) {
	makeChanges();
}

onreset = function(e) {
	resetChanges();
}
