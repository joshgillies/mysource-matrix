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
* $Id: edit.js,v 1.22.2.1 2004/10/07 06:30:03 lwright Exp $
* $Name: not supported by cvs2svn $
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

		// initilise the wysiwg if this is the first time
		// it is being shown - skip this otherwise
		if (initialisedEditors[editor._uniqueID] == null) {
			initialisedEditors[editor._uniqueID] = true;
			editor.generate();
			//editor.updateToolbar(false);
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
		lockToggle.innerHTML = 'Hide Lock Details';
	} else {
		lockInfo.style.display = 'none';
		lockToggle.innerHTML = 'Show Lock Details';
	}
}//end sq_toggle_lock_info()


// show or hide the asset info
function sq_toggle_asset_info() {
	var assetInfo = document.getElementById('sq_asset_info');
	var infoToggle = document.getElementById('sq_asset_info_toggle');
	if (assetInfo.style.display == 'none') {
		assetInfo.style.display = 'block';
		infoToggle.innerHTML = '[ less info ]';
	} else {
		assetInfo.style.display = 'none';
		infoToggle.innerHTML = '[ more info ]';
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
