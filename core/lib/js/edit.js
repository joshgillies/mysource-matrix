/*  ##############################################
   ### MySource ------------------------------###
  ##- Backend Edit file --- Javascript -------##
 #-- Copyright Squiz.net ---------------------#
##############################################
## This file is subject to version 1.0 of the
## MySource License, that is bundled with
## this package in the file LICENSE, and is
## available at through the world-wide-web at
## http://mysource.squiz.net/
## If you did not receive a copy of the MySource
## license and are unable to obtain it through
## the world-wide-web, please contact us at
## mysource@squiz.net so we can mail you a copy
## immediately.
##
## File: web/edit/edit.js
## Desc: Common Javascript functions for backend forms.
## $Source: /home/csmith/conversion/cvs/mysource_matrix/core/mysource_matrix/core/lib/js/edit.js,v $
## $Revision: 1.6 $
## $Author: gsherwood $
## $Date: 2003/09/17 01:10:32 $
#######################################################################
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
