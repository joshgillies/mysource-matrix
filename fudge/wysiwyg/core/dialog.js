/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: dialog.js,v 1.6 2003/09/26 05:26:37 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

// An array of dialogs that have been opened
var dialogWins = new Array();

/*******************************
  BEGIN SEMI-MODAL DIALOG CODE 
*******************************/

//
// An article about this code can be found at:
// http://developer.netscape.com/viewsource/goodman_modal/goodman_modal.html
//

// Global for brower version branching.
var Nav4 = ((navigator.appName == "Netscape") && (parseInt(navigator.appVersion) == 4))

// Generate a modal dialog.
// Parameters:
//    url -- URL of the page/frameset to be loaded into dialog
//    width -- pixel width of the dialog window
//    height -- pixel height of the dialog window
//    returnFunc -- reference to the function (on this page)
//                  that is to act on the data returned from the dialog
//    args -- [optional] any data you need to pass to the dialog
function openDialog(code, url, width, height, returnFunc, args) {
	if (!dialogWins[code] || !dialogWins[code].win || (dialogWins[code].win && dialogWins[code].win.closed)) {
		// Initialize properties of the modal dialog object.
		dialogWins[code] = new Object();
		dialogWins[code].isModal = false;
		dialogWins[code].returnFunc = returnFunc
		dialogWins[code].returnedValue = ""
		dialogWins[code].args = args
		dialogWins[code].url = url
		dialogWins[code].width = width
		dialogWins[code].height = height
		// Keep name unique so Navigator doesn't overwrite an existing dialog.
		dialogWins[code].name = (new Date()).getSeconds().toString()
		// Assemble window attributes and try to center the dialog.
		if (Nav4) {
			// Center on the main window.
			dialogWins[code].left = window.screenX + 
			   ((window.outerWidth - dialogWins[code].width) / 2)
			dialogWins[code].top = window.screenY + 
			   ((window.outerHeight - dialogWins[code].height) / 2)
			var attr = "screenX=" + dialogWins[code].left + 
			   ",screenY=" + dialogWins[code].top + ",resizable=no,width=" + 
			   dialogWins[code].width + ",height=" + dialogWins[code].height
		} else {
			// The best we can do is center in screen.
			dialogWins[code].left = (screen.width - dialogWins[code].width) / 2
			dialogWins[code].top = (screen.height - dialogWins[code].height) / 2
			var attr = "left=" + dialogWins[code].left + ",top=" + 
			   dialogWins[code].top + ",resizable=no,width=" + dialogWins[code].width + 
			   ",height=" + dialogWins[code].height
		}
		
		// Generate the dialog and make sure it has focus.
		dialogWins[code].win = window.open(dialogWins[code].url, dialogWins[code].name, attr)
		dialogWins[code].win.focus()
	} else {
		dialogWins[code].win.focus()
	}
}

// Event handler to inhibit Navigator form element 
// and IE link activity when dialog window is active.
function deadend(code) {
	if (dialogWins[code] && dialogWins[code].win && !dialogWins[code].win.closed) {
		dialogWins[code].win.focus()
		return false
	}
}

// Since links in IE4 cannot be disabled, preserve 
// IE link onclick event handlers while they're "disabled."
// Restore when re-enabling the main window.
var IELinkClicks

// Disable form elements and links in all frames for IE.
function disableForms(code) {
	IELinkClicks = new Array()
	for (var h = 0; h < frames.length; h++) {
		for (var i = 0; i < frames[h].document.forms.length; i++) {
			for (var j = 0; j < frames[h].document.forms[i].elements.length; j++) {
				frames[h].document.forms[i].elements[j].disabled = true
			}
		}
		IELinkClicks[h] = new Array()
		for (i = 0; i < frames[h].document.links.length; i++) {
			IELinkClicks[h][i] = frames[h].document.links[i].onclick
			frames[h].document.links[i].onclick = deadend(code)
		}
	}
}

// Restore IE form elements and links to normal behavior.
function enableForms() {
	for (var h = 0; h < frames.length; h++) {
		for (var i = 0; i < frames[h].document.forms.length; i++) {
			for (var j = 0; j < frames[h].document.forms[i].elements.length; j++) {
				frames[h].document.forms[i].elements[j].disabled = false
			}
		}
		for (i = 0; i < frames[h].document.links.length; i++) {
			frames[h].document.links[i].onclick = IELinkClicks[h][i]
		}
	}
}

// Grab all Navigator events that might get through to form
// elements while dialog is open. For IE, disable form elements.
function blockEvents(code) {
	if (Nav4) {
		window.captureEvents(Event.CLICK | Event.MOUSEDOWN | Event.MOUSEUP | Event.FOCUS)
		window.onclick = deadend
	} else {
		disableForms(code)
	}
	window.onFocus = checkModal(code)
}

// As dialog closes, restore the main window's original
// event mechanisms.
function unblockEvents() {
	if (Nav4) {
		window.releaseEvents(Event.CLICK | Event.MOUSEDOWN | Event.MOUSEUP | Event.FOCUS)
		window.onclick = null
		window.onfocus = null
	} else {
		enableForms()
	}
}

// Invoked by onFocus event handler of EVERY frame,
// return focus to dialog window if it's open.
function checkModal(code) {
	setTimeout("finishChecking('" + code + "')", 50)
	return true
}

function finishChecking(code) {
	if (dialogWins[code] && dialogWins[code].win && !dialogWins[code].win.closed) {
		dialogWins[code].win.focus()
	}
}


/*****************************
  END SEMI-MODAL DIALOG CODE
*****************************/



/**************************
  BEGIN MODAL DIALOG CODE 
**************************/


// Though "Dialog" looks like an object, it isn't really an object.  Instead
// it's just namespace for protecting global symbols.

function openModalDialog(code, url, width, height, action, args) {
	if (document.all) { // here we hope that Mozilla will never support document.all
		var value =
			showModalDialog(url, args, "resizable: no; help: no; status: no; scroll: no;");
		if (action) {
			action(value);
		}
	} else {
		return openDialog(code, url, width, height, action, args)
	}
};

/************************
  END MODAL DIALOG CODE 
************************/