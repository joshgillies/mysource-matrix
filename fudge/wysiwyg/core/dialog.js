/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
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
* $Id: dialog.js,v 1.8 2003/11/18 15:42:11 brobertson Exp $
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

// Grab all Navigator events that might get through to form
// elements while dialog is open. For IE, disable form elements.
function blockEvents(code) {
	window.onFocus = checkModal(code)
}

// As dialog closes, restore the main window's original
// event mechanisms.
function unblockEvents() {

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