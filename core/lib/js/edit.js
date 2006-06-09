/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: edit.js,v 1.34 2006/06/09 04:48:05 tbarrett Exp $
*
*/

// if the browser is IE, regiter the onkeydown event
if(document.all) { document.onkeydown = sq_process_key_down; }


// Execute keyboard shortcuts for IE browsers
function sq_process_key_down() {

	var key;

	// was the ALT key pressed?
	if(!event.altKey) return true;

	// okay, ALT was pressed - but what other key was pressed?
	key = String.fromCharCode(event.keyCode);
	key = key.toLowerCase();

	switch (key) {
		case "v" :
			// preview the asset on the frontend in a new window
			if (parent.frames["sq_main"]) { parent.frames["sq_main"].document.focus(); }
			if (document.main_form.sq_preview_url) {
				preview_popup = window.open(document.main_form.sq_preview_url.value, 'preview', '');
			}
		break;
	}//end switch

}//end sq_process_key_down()


// This function switches the view between wysiwyg and
// page content for a wysiwyg editing field on the page
var initialisedEditors = new Array();
function switchEditingMode(contentDivID, editDivID, editor) {
	var contentDiv = document.getElementById(contentDivID); // div with page contents
	var editDiv = document.getElementById(editDivID);       // div with wysiwyg

	if (editDiv.style.display == "none") { // the edit div is hidden
		var setDesignMode = true;

		// initilise the wysiwyg if this is the first time
		// it is being shown - skip this otherwise
		if (initialisedEditors[editor._uniqueID] == null) {
			initialisedEditors[editor._uniqueID] = true;
			editor.generate();
			//editor.updateToolbar(true);
			setDesignMode = false;
		} else if (editor._initialised != true) {
			return;
		}

		editDiv.style.display = ""; // show the wysiwyg

		// if we are using an iframe for this editor, we set its designMode property if we need to
		if (editor._iframe) {
			editor._iframe.style.width = editor.config.width;
			if (setDesignMode && HTMLArea.is_gecko) { editor._iframe.contentWindow.document.designMode = "on"; }
			editor._iframe.style.height = editor.config.height;
		}
		contentDiv.style.display = "none"; // hide the contents
	} else if (editor._initialised == true) {
		// the content div is hidden and the
		// wysiwyg editor has been initialised
		contentDiv.innerHTML = editor.getHTML();
		contentDiv.style.display = "";  // show the contents
		editDiv.style.display = "none"; // hide the wysiwyg

		if (editor._iframe) {
			var editCell = document.getElementById(editor._uniqueID + "_cell");
			if (editCell) { editCell.style.height = "100%"; }
		}
	}
}//end switchEditingMode()


// show or hide the full lock info
function sq_toggle_lock_info() {
	var lockInfo = document.getElementById('sq_lock_info');
	var lockToggle = document.getElementById('sq_lock_info_toggle');
	if (lockInfo.style.display == 'none') {
		lockInfo.style.display = 'block';
		lockToggle.innerHTML = js_translate('hide_lock_details');
	} else {
		lockInfo.style.display = 'none';
		lockToggle.innerHTML = js_translate('show_lock_details');
	}
}//end sq_toggle_lock_info()


// show or hide the asset info
function sq_toggle_asset_info(clicked) {
	var assetInfo = document.getElementById('sq_asset_info');
	if (assetInfo.style.display == 'none') {
		assetInfo.style.display = 'block';
		clicked.innerHTML = '[ '+ js_translate('less_info') +' ]';
	} else {
		assetInfo.style.display = 'none';
		clicked.innerHTML = '[ '+ js_translate('more_info') +' ]';
	}
}//end sq_toggle_asset_info()


// toggle between showing and hiding two divs
function sq_toggle_double_div(div1ID, div2ID, textID, text1, text2) {
	var div1 = document.getElementById(div1ID);
	var div2 = document.getElementById(div2ID);
	var infoToggle = document.getElementById(textID);
	if (div1.style.display == 'none') {
		div1.style.display = 'block';
		div2.style.display = 'none';
		infoToggle.innerHTML = text1;
	} else {
		div1.style.display = 'none';
		div2.style.display = 'block';
		infoToggle.innerHTML = text2;
	}

}//end sq_toggle_double_div()


/**
* Validate value of current input box of the duration attribute
*
* @param object	input		input field which fires an event
* @param object evt			event object
*
* @return boolean	false if non digit key was pressed
* @access public
*/
function checkDurationValue(input, evt)
{
	var prevalue = input.value;
	var key = (typeof evt.which == "undefined")?evt.keyCode:evt.which;
	if (key == 8 || key == 0) return true;

	textboxReplaceSelect(input, String.fromCharCode(key));
	var newval = input.value;

	// catch +/- keys (45 -> -, 43 -> +)
	if (key == 45) {
		newval = prevalue - 1;
	}
	if (key == 43) {
		newval = prevalue * 1 + 1;
	}

	if (newval < 0) newval = 0;

	if (newval * 1 == newval) {
		input.value = Math.floor(newval);
	} else {
		input.value = prevalue;
	}

	if (key == 43 || key == 45) {
		updateDurationValues(input);
	}

	// stop event here
	evt.cancelBubble = true;
	evt.returnValue = false;
	return false;

}//end checkDurationValue()


/**
* Updates whole duartion attribute input boxes set by carrying values between fields
*
* @param object	input		one of the input boxes from the attribute
*
* @return void
* @access public
*/
function updateDurationValues(input)
{
	var fields = Array("days", "hours", "minutes", "seconds");
	var weights = Array(86400, 3600, 60, 1);

	//catch the prefix
	var prefix = "";
	for (var i = 0; i < fields.length; i++) {
		if (input.id.indexOf(fields[i]) > 0) {
			prefix = input.id.substring(0, input.id.indexOf(fields[i]));
		}
	}

	var total = 0;
	for (var i = 0; i < fields.length; i++) {
		var element = document.getElementById(prefix + fields[i]);
		if (element) {
			total += element.value * weights[i];
		}
	}

	for (var i = 0; i < fields.length; i++) {
		var element = document.getElementById(prefix + fields[i]);
		if (element) {
			curvalue = Math.floor(total / weights[i]);
			total -= curvalue * weights[i];
			element.value = curvalue;
		}
	}

}//end updateDurationValues()


/**
* Changes selected text in the input field to given one.
*
* If nothing selected add the text to the end
*
* @param object	input		input field which fires an event
* @param object text		text to be set
*
* @return viod
* @access public
*/
function textboxReplaceSelect(input, text)
{
	if (document.selection) {
		var range = document.selection.createRange();
		range.text = text;
		range.collapse(true);
		range.select();
	} else {
		var start = input.selectionStart;
		input.value = input.value.substring(0, start) + text + input.value.substring(input.selectionEnd, input.value.length);
		input.setSelectionRange(start + text.length, start + text.length);
	}

	input.focus();

}//end textboxReplaceSelect()


// Functions for option list attribute

var expandListFn = new Function('expandOptionList(this)');
var deleteRowFn = new Function('deleteOptionListRow(this); return false;');
var onClickMoveUp = new Function('listMoveUp(this); return false;');
var onClickMoveDown = new Function('listMoveDown(this); return false;');

function expandOptionList(input)
{
	// abort if we are not the last input in the lit
	var nextInput = input.nextSibling;
	while (nextInput !== null) {
		if (nextInput.tagName == 'INPUT') {
			return;
		}
		nextInput = nextInput.nextSibling;
	}

	// abort if we and the second-last input are both empty
	var lastInput = input.previousSibling;
	if (input.value == '') {
		while (lastInput !== null) {
			if (lastInput.tagName == 'INPUT') {
				if (lastInput.value == '') {
					return;
				}
				break;
			}
			lastInput = lastInput.previousSibling;
		}
	}

	var inputs = optionList.getElementsByTagName('INPUT');

	// add move down button to the previous input
	var moveDownButton = lastInput.nextSibling;
	while (moveDownButton != null) {
		moveDownButton = moveDownButton.nextSibling;
		if (moveDownButton.tagName == 'A' && moveDownButton.name=="movedown") {
			break;
		}
	}
	moveDownButton.id = optionItemPrefix+'_options['+(inputs.length-2)+']';
	moveDownButton = moveDownButton.cloneNode(true);
	moveDownButton.onclick = onClickMoveDown;

	var brElements = lastInput.parentNode.getElementsByTagName('BR');
	lastInput.parentNode.removeChild(brElements[brElements.length-1]);
	input.parentNode.appendChild(moveDownButton);
	input.parentNode.appendChild(document.createElement('BR'));



	// add the extra field
	var newInput = input.cloneNode(true);
	newInput.onfocus = expandListFn;
	newInput.value = '';
	newInput.id = optionItemPrefix+'_options['+inputs.length+']';
	input.parentNode.appendChild(newInput);
	var delButton = input.nextSibling;
	while (delButton.tagName != 'BUTTON') {
		delButton = delButton.nextSibling;
	}
	delButton = delButton.cloneNode(true);
	delButton.onclick = deleteRowFn;
	input.parentNode.appendChild(delButton);



	// add the move up button to the new input
	var moveUpButton = input.nextSibling;
	while (moveUpButton != null) {
		if (moveUpButton.tagName == 'A' && moveUpButton.name == 'moveup') {
			break;
		}
		moveUpButton = moveUpButton.nextSibling;
	}
	moveUpButton = moveUpButton.cloneNode(true);
	moveUpButton.id = optionItemPrefix+'_options['+(inputs.length-1)+']';
	moveUpButton.onclick = onClickMoveUp;

	input.parentNode.appendChild(moveUpButton);
	input.parentNode.appendChild(document.createElement('BR'));

}

// move up a row
function listMoveUp(obj) {
	var currentOrder = 0;
	var inputs = optionList.getElementsByTagName('INPUT');

	for (var i=0 ; i < inputs.length; i++) {
		if (obj.id == inputs[i].id) {
			currentOrder = i;
			break;
		}
	}
	if (currentOrder == 0) return;

	var temp = inputs[currentOrder-1].value;
	inputs[currentOrder-1].value = inputs[currentOrder].value;
	inputs[currentOrder].value = temp;
}

// move down a row
function listMoveDown(obj) {
	var currentOrder = 0;

	var inputs = optionList.getElementsByTagName('INPUT');
	for (var i=0 ; i < inputs.length; i++) {
		if (obj.id == inputs[i].id) {
			currentOrder = i;
			break;
		}
	}

	if (currentOrder == inputs.length) return;

	var temp = inputs[currentOrder+1].value;
	inputs[currentOrder+1].value = inputs[currentOrder].value;
	inputs[currentOrder].value = temp;
}

function deleteOptionListRow(button)
{
	var input = button.previousSibling;
	while (input.tagName != 'INPUT') {
		input = input.previousSibling;
	}
	if (input.value == '') return;

	var moveUpBut = button.nextSibling;
	while (moveUpBut != null) {
		if (moveUpBut.tagName == 'A' && moveUpBut.name == 'moveup') {
			break;
		}
		moveUpBut = moveUpBut.nextSibling;
	}
	button.parentNode.removeChild(moveUpBut);

	var moveDownBut = button.nextSibling;
	while (moveDownBut != null) {
		if (moveDownBut.tagName == 'A' && moveDownBut.name == 'movedown') {
			break;
		}
		moveDownBut = moveDownBut.nextSibling;
	}
	button.parentNode.removeChild(moveDownBut);


	var brTag = button.nextSibling;
	while (brTag.tagName != 'BR') {
		brTag = brTag.nextSibling;
	}
	button.parentNode.removeChild(input);
	button.parentNode.removeChild(brTag);
	button.parentNode.removeChild(button);
}

// Functions for date list
var expandDateListFn = new Function('expandDateList(this)');
var deleteDateRowFn = new Function('deleteDateListRow(this); return false;');

function allVisibleInputsEmpty(elt)
{
	var inputs = elt.getElementsByTagName('INPUT');
	for (var i=0; i < inputs.length; i++) {
		if (inputs[i].type == 'hidden') continue;
		if (inputs[i].value != '') {
			return false;
		}
	}
	var selects = elt.getElementsByTagName('SELECT');
	for (var i=0; i < selects.length; i++) {
		if ((selects[i].value != '') && (selects[i].value != '--')) {
			return false;
		}
	}
	return true;
}

function clearAllVisibleInputs(elt)
{
	var inputs = elt.getElementsByTagName('INPUT');
	for (var i=0; i < inputs.length; i++) {
		if (inputs[i].type == 'hidden') continue;
		inputs[i].value = '';
	}
	var selects = elt.getElementsByTagName('SELECT');
	for (var i=0; i < selects.length; i++) {
		selects[i].selectedIndex = 0;
	}
}

function expandDateList(input)
{
	// abort if we are not the last line in the list
	var currentSpan = input.parentNode;
	while (currentSpan.tagName != 'SPAN') currentSpan = currentSpan.parentNode;
	var nextSpan = currentSpan.nextSibling;
	while (nextSpan !== null) {
		if (nextSpan.tagName == 'SPAN') {
			return;
		}
		nextSpan = nextSpan.nextSibling;
	}

	// abort if we and the second-last input are both empty
	lastSpan = currentSpan.previousSibling;
	while (lastSpan.tagName != 'SPAN') lastSpan = lastSpan.previousSibling;
	if (allVisibleInputsEmpty(lastSpan) && allVisibleInputsEmpty(currentSpan)) {
		return;
	}

	// add the extra fields
	var newSpan = currentSpan.cloneNode(true);
	attachDateListEventHandlers(newSpan);
	clearAllVisibleInputs(newSpan);
	var delButton = currentSpan.nextSibling;
	while (delButton.tagName != 'BUTTON') {
		delButton = delButton.nextSibling;
	}
	delButton = delButton.cloneNode(true);
	delButton.onclick = deleteDateRowFn;
	input.parentNode.parentNode.appendChild(newSpan);
	input.parentNode.parentNode.appendChild(delButton);
	input.parentNode.parentNode.appendChild(document.createElement('BR'));
}

function deleteDateListRow(button)
{
	var span = button.previousSibling;
	while (span.tagName != 'SPAN') {
		span = span.previousSibling;
	}
	if (span.getElementsByTagName('INPUT')[0].value == '') return;
	var brTag = button.nextSibling;
	while (brTag.tagName != 'BR') {
		brTag = brTag.nextSibling;
	}
	button.parentNode.removeChild(span);
	button.parentNode.removeChild(brTag);
	button.parentNode.removeChild(button);
}

function attachDateListEventHandlers(parent)
{
	var	inputs = parent.getElementsByTagName('INPUT');
	for (var j=0; j < inputs.length; j++) {
		inputs[j].onfocus = expandDateListFn;
	}
	var	selects = parent.getElementsByTagName('SELECT');
	for (var j=0; j < selects.length; j++) {
		selects[j].onchange = expandDateListFn;
	}
	var buttons = parent.getElementsByTagName('BUTTON');
	for (var j=0; j < buttons.length; j++) {
		buttons[j].onclick = deleteDateRowFn;
	}
}

function focusFirstTextInput()
{
	var inputs = document.getElementsByTagName('INPUT');
	for (var i=0; i < inputs.length; i++) {
		if (inputs[i].type == 'text') {
			inputs[i].focus();
			return;
		}
	}
}