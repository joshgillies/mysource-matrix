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
## $Revision: 1.2 $
## $Author: gsherwood $
## $Date: 2003/08/19 00:37:58 $
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
function sqSubmitEditForm() {

	frm = document.main_form;
	if (frm.sq_submit_pressed.value == '0') {
		frm.sq_submit_pressed.value = '1';
		frm.sq_release_lock.value = frm.sq_release_lock_on_submit.value;
		frm.submit();
	}

}//end sqSubmitEditForm()