/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: edit.js,v 1.7 2003/09/26 05:26:34 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

// if the browser is IE, regiter the onkeydown event
if(document.all) { document.onkeydown = sqProcessKeyDown; }


// Execute keyboard shortcuts for IE browsers
function sqProcessKeyDown() {

	var key;
	
	// was the ALT key pressed?
	if(!event.altKey) return true;

	// okay, ALT was pressed - but what other key was pressed?
	key = String.fromCharCode(event.keyCode);
	key = key.toLowerCase();
	
	switch (key) {
		case "s" :
			// emulate pressing of the commit button
			top.main.document.focus();
			sqSubmitEditForm();
		break;

		case "v" :
			// preview the asset on the frontend in a new window
			top.main.document.focus();
			if (document.main_form.sq_preview_url) {
				preview_popup = window.open(document.main_form.sq_preview_url.value, 'preview', '');
			}
		break;
	}//end switch

}//end processKeyDown()


// Submit the edit form after a bit of checking
function sqSubmitEditForm(noProcess) {
	frm = document.main_form;

	// make sure the form is not processed
	if (noProcess != null) frm.am_form_submitted.value = '0';
	
	if (!frm.sq_submit_pressed || frm.sq_submit_pressed.value == '0') {
		if (frm.sq_submit_pressed) { frm.sq_submit_pressed.value = '1'; }
		if (frm.sq_release_lock_on_submit) { frm.sq_release_lock.value = frm.sq_release_lock_on_submit.value; }
		frm.onsubmit();
		frm.submit();
	}

}//end sqSubmitEditForm()


// prints an icon using transparency in IE
// ensures that PNGs have transparent background in IE and Mozilla
function sqPrintIcon (path, width, height, alt) {
	if (document.all) {
		// IE cant handle transparent PNGs
		document.write ('<span style="height:'+height+'px;width:'+width+'px; filter:progid:DXImageTransform.Microsoft.AlphaImageLoader (src=\''+path+'\', sizingMethod=\'scale\')"></span>');
	} else {
		document.write('<img src="'+path+'" width="'+width+'" height="'+height+'" border="0" alt="'+alt+'" />');
	}
}
