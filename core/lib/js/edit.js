/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: edit.js,v 1.8 2003/10/02 23:59:27 brobertson Exp $
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
		case "s" :
			// emulate pressing of the commit button
			top.main.document.focus();
			sq_submit_edit_form();
		break;

		case "v" :
			// preview the asset on the frontend in a new window
			top.main.document.focus();
			if (document.main_form.sq_preview_url) {
				preview_popup = window.open(document.main_form.sq_preview_url.value, 'preview', '');
			}
		break;
	}//end switch

}//end sq_process_key_down()


// Submit the edit form after a bit of checking
function sq_submit_edit_form(dont_process) {

	// make sure the form is not processed
	if (dont_process != null && dont_process) set_hidden_field('am_form_submitted', '0');
	
	var sq_submit_pressed = get_form_element('sq_submit_pressed');

	if (sq_submit_pressed == null || sq_submit_pressed.value == '0') {
		if (sq_submit_pressed != null) {
			sq_submit_pressed.value = '1';
		}
		if (get_form_element('sq_release_lock_on_submit') != null) {
			set_hidden_field('sq_release_lock',  get_form_element_value('sq_release_lock_on_submit'));
		}
		submit_form();
	}

}//end sq_submit_edit_form()


// prints an icon using transparency in IE
// ensures that PNGs have transparent background in IE and Mozilla
function sq_print_icon(path, width, height, alt) {
	if (document.all) {
		// IE cant handle transparent PNGs
		document.write ('<span style="height:'+height+'px;width:'+width+'px; filter:progid:DXImageTransform.Microsoft.AlphaImageLoader (src=\''+path+'\', sizingMethod=\'scale\')"></span>');
	} else {
		document.write('<img src="'+path+'" width="'+width+'" height="'+height+'" border="0" alt="'+alt+'" />');
	}
}
