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
* Accessible CAPTCHA Functions
* @author Mark Brydon <mbrydon@squiz.net>
*
* $Id: captcha.js,v 1.3.4.1 2013/05/01 02:47:50 ewang Exp $
*
*/


function show_accessible_captcha()
{
	// Would love these to be global however the CAPTCHA interface can be manually built, so we can
	// only be guaranteed to have all of these elements once the link is clicked
	var captcha_textbox = document.getElementById('SQ_SYSTEM_SECURITY_KEY_VALUE');
	var captcha_image = document.getElementById('sq_security_key');
	var captcha_accessible_link = document.getElementById('sq_accessible_validation_link');
	var unreadable_captcha = document.getElementById('sq_regen_captcha');
	var accessible_captcha_div = document.getElementById('sq_accessible_captcha');
	var sq_accessible_captcha_message = document.getElementById('sq_accessible_captcha_message');
	var captcha_error = document.getElementById('SQ_SYSTEM_SECURITY_KEY_VALUE_ERROR');
	var normal_captcha_div = document.getElementById('sq_normal_captcha');

	captcha_textbox.style.display = 'none';
	captcha_textbox.style.visibility = 'hidden';

	captcha_image.style.display = 'none';
	captcha_image.style.visibility = 'hidden';

	if (unreadable_captcha) {
		unreadable_captcha.style.visibility = 'hidden';
		unreadable_captcha.style.display = 'none';
	}
	
	if (captcha_error) {
		captcha_error.style.visibility = 'hidden';
		captcha_error.style.display = 'none';
	}
	
	if(normal_captcha_div) {
	    	normal_captcha_div.style.visibility = 'hidden';
		normal_captcha_div.style.display = 'none';
	}

	accessible_captcha_div.style.display = 'inline';
	accessible_captcha_div.style.visibility = 'visible';
	
	sq_accessible_captcha_message.style.display = 'inline';
	sq_accessible_captcha_message.style.visibility = 'visible';


	captcha_accessible_link.innerHTML = '&nbsp;<a href="javascript:hide_accessible_captcha();">Use image validation</a>';

	// Give focus to the input field just to be super nice
	document.getElementById('SQ_SYSTEM_SECURITY_KEY_EMAIL').focus();
}


function hide_accessible_captcha()
{
	// Would love these to be global however the CAPTCHA interface can be manually built, so we can
	// only be guaranteed to have all of these elements once the link is clicked
	var captcha_textbox = document.getElementById('SQ_SYSTEM_SECURITY_KEY_VALUE');
	var captcha_image = document.getElementById('sq_security_key');
	var captcha_accessible_link = document.getElementById('sq_accessible_validation_link');
	var unreadable_captcha = document.getElementById('sq_regen_captcha');
	var accessible_captcha_div = document.getElementById('sq_accessible_captcha');
	var sq_accessible_captcha_message = document.getElementById('sq_accessible_captcha_message');
	var captcha_error = document.getElementById('SQ_SYSTEM_SECURITY_KEY_VALUE_ERROR');
	var normal_captcha_div = document.getElementById('sq_normal_captcha');

	accessible_captcha_div.style.display = 'none';
	accessible_captcha_div.style.visibility = 'hidden';

	captcha_textbox.style.display = 'inline';
	captcha_textbox.style.visibility = 'visible';

	captcha_image.style.display = 'inline';
	captcha_image.style.visibility = 'visible';
	
	sq_accessible_captcha_message.style.display = 'none';
	sq_accessible_captcha_message.style.visibility = 'hidden';


	// This field is optional - we expect the original CAPTCHA textbox and CAPTCHA image to be present so
	// we're not too concerned about any JS errors above if these fields are not printed. But we care about this one...
	if (unreadable_captcha) {
		unreadable_captcha.style.visibility = 'visible';
		unreadable_captcha.style.display = 'inline';
	}
	
	if (captcha_error) {
		captcha_error.style.visibility = 'visible';
		captcha_error.style.display = 'inline';
	}
	
	if(normal_captcha_div) {
	    	normal_captcha_div.style.visibility = 'visible';
		normal_captcha_div.style.display = 'inline';
	}
	

	captcha_accessible_link.innerHTML = '&nbsp;<a href="javascript:show_accessible_captcha();">Use accessible validation</a>';
}


// AJAX Callback
function email_captcha_sent(responseText)
{
	var user_email_field = document.getElementById('SQ_SYSTEM_SECURITY_KEY_EMAIL');
	var user_email_address = user_email_field.value;
	var submit_button = document.getElementById('sq_submit_accessible_captcha');
	var instruction = document.getElementById('sq_accessible_captcha_instruction');
	
	user_email_field.disabled = 0;
	submit_button.value = 'Email message sent';
	instruction.style.display = 'inline';
	instruction.style.visibility = 'visible';
	var error = document.getElementById('sq_accessible_captcha_error');
	error.style.display = 'none';
	error.style.visibility = 'hidden';
}


function submit_email_captcha(lib_url)
{
	var user_email_field = document.getElementById('SQ_SYSTEM_SECURITY_KEY_EMAIL');
	var user_email_address = user_email_field.value;


	// if valid email address
	if (/^\w+([\.-]?\w+)*@\w+([\.-]?\w+)*(\.\w{2,3})+$/.test(user_email_address))
	{
		var submit_button = document.getElementById('sq_submit_accessible_captcha');

		user_email_field.disabled = 1;
		submit_button.disabled = 1;
		submit_button.value = 'Sending email...';

		// Send AJAX request
		var params = "email="+user_email_address;
		JsHttpConnector.submitRequest(lib_url+'/web/accessible_captcha.php?'+params, email_captcha_sent);
	}
	else {
	    var error = document.getElementById('sq_accessible_captcha_error');
	    error.style.display = 'inline';
	    error.style.visibility = 'visible';
	}
}


function enable_submission_button()
{
	var submit_button = document.getElementById('sq_submit_accessible_captcha');

	submit_button.value = 'Submit address';
	submit_button.disabled = 0;
}
