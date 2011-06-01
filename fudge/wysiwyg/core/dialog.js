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
* $Id: dialog.js,v 1.13 2006/12/06 05:11:07 bcaldwell Exp $
*
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

// The currently opened WYSIWYG dialogue box
var sq_wysiwyg_dialog = null;

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

		// If a dialogue is already opened, close it so the next one doesn't
		// open with the same dimensions
		if (sq_wysiwyg_dialog) {
			sq_wysiwyg_dialog.close();
		}

		// Assemble window attributes and try to center the dialog.
		if (Nav4) {
			// Center on the main window.
			dialogWins[code].left = window.screenX +
			   ((window.outerWidth - dialogWins[code].width) / 2)
			dialogWins[code].top = window.screenY +
			   ((window.outerHeight - dialogWins[code].height) / 2)
			var attr = "screenX=" + dialogWins[code].left +
			   ",screenY=" + dialogWins[code].top + ",resizable=yes,scrollbars=yes,width=" +
			   dialogWins[code].width + ",height=" + dialogWins[code].height
		} else {
			// The best we can do is center in screen.
			dialogWins[code].left = (screen.width - dialogWins[code].width) / 2
			dialogWins[code].top = (screen.height - dialogWins[code].height) / 2
			var attr = "left=" + dialogWins[code].left + ",top=" +
			   dialogWins[code].top + ",resizable=yes,scrollbars=yes,width=" + dialogWins[code].width +
			   ",height=" + dialogWins[code].height
		}

		// Generate the dialog and make sure it has focus.
		dialogWins[code].win = window.open(dialogWins[code].url, dialogWins[code].name, attr)
		dialogWins[code].win.focus()
	} else {
		dialogWins[code].win.focus()
	}

	sq_wysiwyg_dialog = dialogWins[code].win;
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
			showModalDialog(url, args, "resizable: yes; help: no; status:no; scroll: yes; center:yes");
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