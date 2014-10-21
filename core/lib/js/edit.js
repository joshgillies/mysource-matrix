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
* $Id: edit.js,v 1.57 2012/08/30 01:09:21 ewang Exp $
*
*/

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
		lockToggle.innerHTML = js_translate('Hide Lock Details');

	} else {
		lockInfo.style.display = 'none';
		lockToggle.innerHTML = js_translate('Show Lock Details');

	}
}//end sq_toggle_lock_info()


// show or hide the asset info
function sq_toggle_asset_info(clicked) {
	var assetInfo = document.getElementById('sq_asset_info');
	if (assetInfo.style.display == 'none') {
		assetInfo.style.display = 'block';
		clicked.innerHTML = '[ '+ js_translate('Less Info') +' ]';

	} else {
		assetInfo.style.display = 'none';
		clicked.innerHTML = '[ '+ js_translate('More Info') +' ]';

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

var expandListFn = new Function('expandOptionList(this, true)');
var noreorderExpandListFn = new Function('expandOptionList(this, false)');
var deleteRowFn = new Function('deleteOptionListRow(this, true); return false;');
var noreorderDeleteRowFn = new Function('deleteOptionListRow(this, false); return false;');

function expandOptionList(input, reorder)
{
	// abort if we are not the last input in the list
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

	if (reorder) {
		// add move down button to the previous input
		var moveDownButton = lastInput.nextSibling;
		while (moveDownButton != null) {
			moveDownButton = moveDownButton.nextSibling;
			if (moveDownButton.tagName == 'A' && moveDownButton.name == "movedown") {
				break;
			}
		}
		moveDownButton = moveDownButton.cloneNode(true);
		// Cloned button, so we *must* give it a different id
		moveDownButton.id = optionItemPrefix+'_options['+(inputs.length-1)+']';

		//If safari, we will remove the script for printing move up/down icon, it's causing document.write to overwrite the page in safari
		var buttonScript =  moveDownButton.getElementsByTagName("script")[0];
		var browserAgent = navigator.userAgent.toLowerCase();
		if ((browserAgent.indexOf("safari") != -1) && buttonScript != null) {
			moveDownButton.removeChild(buttonScript);
		}

		var brElements = lastInput.parentNode.getElementsByTagName('BR');
		lastInput.parentNode.removeChild(brElements[brElements.length-1]);
		input.parentNode.appendChild(moveDownButton);
		input.parentNode.appendChild(document.createElement('BR'));
	}

	// add the extra field
	var newInput = input.cloneNode(true);
	if (reorder) {
		newInput.onfocus = expandListFn;
	} else {
		newInput.onfocus = noreorderExpandListFn;
	}
	newInput.value = '';
	newInput.id = optionItemPrefix+'_options['+inputs.length+']';
	input.parentNode.appendChild(newInput);
	var delButton = input.nextSibling;
	while (delButton.tagName != 'BUTTON') {
		delButton = delButton.nextSibling;
	}
	delButton = delButton.cloneNode(true);
	if (reorder) {
		delButton.onclick = deleteRowFn;
	} else {
		delButton.onclick = noreorderDeleteRowFn;
	}
	input.parentNode.appendChild(delButton);

	if (reorder) {
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

		//If safari, we will remove the script for printing move up/down icon, it's causing document.write to overwrite the page in safari
		var buttonScript =  moveUpButton.getElementsByTagName("script")[0];
		var browserAgent = navigator.userAgent.toLowerCase();
		if ((browserAgent.indexOf("safari") != -1) && buttonScript != null) {
				moveUpButton.removeChild(buttonScript);
		}
		input.parentNode.appendChild(moveUpButton);
	}
	input.parentNode.appendChild(document.createElement('BR'));

}

// move up a row
function listMoveUp(obj, optionList) {
	var currentOrder = 0;

	var inputs = optionList.getElementsByTagName('INPUT');
	for (var i=0 ; i < inputs.length; i++) {
		if (obj.id == inputs[i].id) {
			currentOrder = i;
			break;
		}
	}
	if (currentOrder == 0) return;

	// If the input is disabled, don't respond
	if (inputs[currentOrder].disabled) return;

	var temp = inputs[currentOrder-1].value;
	inputs[currentOrder-1].value = inputs[currentOrder].value;
	inputs[currentOrder].value = temp;
}

// move down a row
function listMoveDown(obj, optionList) {
	var currentOrder = 0;

	var inputs = optionList.getElementsByTagName('INPUT');
	for (var i=0 ; i < inputs.length; i++) {
		if (obj.id == inputs[i].id) {
			currentOrder = i;
			break;
		}
	}

	if (currentOrder == inputs.length) return;

	// If the input is disabled, don't respond
	if (inputs[currentOrder].disabled) return;

	var temp = inputs[currentOrder+1].value;
	inputs[currentOrder+1].value = inputs[currentOrder].value;
	inputs[currentOrder].value = temp;
}

function deleteOptionListRow(button, reorder)
{
	var input = button.previousSibling;
	while (input.tagName != 'INPUT') {
		input = input.previousSibling;
	}

	if (input.value == '') return;

	// If the input is disabled, don't respond
	if (input.disabled) return;

	// Don't let the option list get down to a single element. Clear the field
	// instead, but leave it as two elements
	var inputs = input.parentNode.getElementsByTagName('INPUT');
	if (inputs.length <= 2) {
		input.value = '';
		return;
	}

	if (reorder) {
		var moveUpBut = button.nextSibling;
		while (moveUpBut != null) {
			if (moveUpBut.tagName == 'A' && moveUpBut.name == 'moveup') {
				break;
			}
			moveUpBut = moveUpBut.nextSibling;
		}

		var moveDownBut = button.nextSibling;
		while (moveDownBut != null) {
			if (moveDownBut.tagName == 'A' && moveDownBut.name == 'movedown') {
				break;
			}
			moveDownBut = moveDownBut.nextSibling;
		}

		if(moveUpBut == null || moveDownBut == null) return;
		button.parentNode.removeChild(moveUpBut);
		button.parentNode.removeChild(moveDownBut);
	}


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
		if ((inputs[i].type == 'text') && (!inputs[i].disabled)) {
			try {
				inputs[i].focus();
				return;
			} catch (e) {}
		}
	}
}

function pagePrint()
{
	window.setTimeout('window.print()',100);
}

function showPrintPopup()
{
	var args = 'width='+(self.screen.availWidth/2)+',height='+parseInt(self.screen.availHeight*0.8)+',status=0,location=0,menubar=0,directories=0,scrollbars=1,resizable=1';
	var urlSuffix = '&ignore_frames=1&print_view=1';
	var urlBase = self.location.href;
	if (urlBase.indexOf('&assetid=') == -1) {
		// the assetid is not in the url, probably cause we just created
		// try to find it elsewhere
		var mainForm = document.getElementById('main_form');
		if (mainForm) urlBase = mainForm.action;
	}
	var printWindow = window.open(urlBase + urlSuffix, 'printWindow', args);

}

function initEnableFieldLists()
{
	var uls = document.getElementsByTagName('UL');
	for (var i=0; i < uls.length; i++) {
		if (uls[i].className.match(/(^| )enable-field-list($| )/)) {
			var lis = uls[i].getElementsByTagName('INPUT');
			for (var j=0; j < lis.length; j++) {
				if (lis[j].type == 'radio') {
					lis[j].onclick = updateEnableFieldList;
					if (lis[j].checked) lis[j].click();
				}
			}
		}
	}
}

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

function updateEnableFieldList()
{
	var parentLI = this.parentNode;
	while (parentLI.tagName != 'LI') {
		parentLI = parentLI.parentNode;
	}
	var parentUL = parentLI.parentNode;
	while (parentUL.tagName != 'UL') {
		parentUL = parentUL.parentNode;
	}
	var lis = parentUL.getElementsByTagName('LI');
	for (var j=0; j < lis.length; j++) {
		var div = lis[j].getElementsByTagName('DIV')[0];
		div.className = (lis[j] == parentLI) ? 'active' : '';
		setInputsEnabled(div, lis[j] == parentLI);
	}
}

/**
* Toggle the display of the next element of type targetType
*
* This function will search elt's siblings (going back up the tree to traverse if necessary)
* until it finds one of type targetType.  It will toggle the display of the element found.
*
* @param object	elt	The element we are coming from (eg the thing that was clicked)
* @param string	targetType	The tagName of the thing we want to toggle the display of
*
* @return void
* @access public
*/
function toggleNextElt(elt, targetType)
{
	var target = elt.nextSibling;
	var i = 0;
	while (target.tagName != targetType) {
		if (i++ > 20) return;
		if (target.nextSibling) {
			target = target.nextSibling;
		} else {
			target = target.parentNode.nextSibling;
			if (target.firstChild) {
				target = target.firstChild;
			}
		}
	}
	with (target.style) { display = (display == 'none') ? 'block' : 'none'; }
}


/**
* Provide a standard interface for jumping between pages in an edit interface
*
* - Sets the value of a hidden field of your choice
* - Resubmits the form (but without 'submit_form' enabled, meaning changes
*   will not take effect)
* - Returns false so the link you use doesn't activate on onClick or similar
*   (ie. you can do "return sq_pager_jump(...);" for this to work)
*
* BYO:
* - Hidden form field to act as your pager
* - Links to do the jumping (best placed in onclick)
*
* How you do your paging is up to you - whether you send a page number or an
* offset, that is up to your processing. This is just to stop having to put
* a pager script everywhere it's necessary. :-)
*
* If val is NaN (eg. if passed from sq_pager_prompt()), nothing will happen.
*
* @param string	page_field	The hidden field's name.
* @param mixed	val			The value to send to the hidden field.
*
* @return boolean
* @access public
*/
function sq_pager_jump(page_field, val)
{
	if (isNaN(val) == false) {
		set_hidden_field(page_field, val);
		set_hidden_field('process_form', '0');
		submit_form();
	}
	return false;
}


/**
* Provide a standard interface for providing a prompt for jumping between pages
*
* Works best in situations where the page numbers are sequential, and not offsets.
* If it's an offset, you could possibly do something like:
* <pre>
*   var pageNo = sq_pager_prompt(min, max);
*   if (!isNaN(pageNo)) { sq_pager_jump('page_field', (pageNo - 1) * page_size); }
* </pre>
*
* But would be better if you just used sequential numbers instead:
* <pre>
*   sq_pager_jump(sq_pager_prompt(min, max));
* </pre>
*
* Returns NaN if not a valid number (according to parseInt). If passed to
* sq_pager_jump as in the second example, this will become a 'no-op'.
*
* @param string	page_field	The hidden field's name.
* @param mixed	val			The value to send to the hidden field.
*
* @return boolean
* @access public
*/
function sq_pager_prompt(min, max)
{
	var pageNo = prompt(js_translate('sq_pager_prompt_js', min, max));
	pageNo = parseInt(pageNo, 10);
	if (isNaN(pageNo) == false) {
		if ((pageNo >= min) && (pageNo <= max)) {
			return pageNo;
		}
	}
	return NaN;
}
