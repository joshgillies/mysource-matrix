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
* $Id: captcha.js,v 1.3 2012/08/30 01:09:21 ewang Exp $
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

	captcha_textbox.style.display = 'none';
	captcha_textbox.style.visibility = 'hidden';

	captcha_image.style.display = 'none';
	captcha_image.style.visibility = 'hidden';

	// This field is optional - we expect the original CAPTCHA textbox and CAPTCHA image to be present so
	// we're not too concerned about any JS errors above if these fields are not printed. But we care about this one...
	if (typeof unreadable_captcha == 'undefined') {
		unreadable_captcha.style.visibility = 'hidden';
	}

	accessible_captcha_div.style.display = 'inline';
	accessible_captcha_div.style.visibility = 'visible';

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

	accessible_captcha_div.style.display = 'none';
	accessible_captcha_div.style.visibility = 'hidden';

	captcha_textbox.style.display = 'inline';
	captcha_textbox.style.visibility = 'visible';

	captcha_image.style.display = 'inline';
	captcha_image.style.visibility = 'visible';

	// This field is optional - we expect the original CAPTCHA textbox and CAPTCHA image to be present so
	// we're not too concerned about any JS errors above if these fields are not printed. But we care about this one...
	if (typeof unreadable_captcha == 'undefined') {
		unreadable_captcha.style.visibility = 'visible';
	}

	captcha_accessible_link.innerHTML = '&nbsp;<a href="javascript:show_accessible_captcha();">Use accessible validation</a>';
}


// AJAX Callback
function email_captcha_sent(responseText)
{
	var user_email_field = document.getElementById('SQ_SYSTEM_SECURITY_KEY_EMAIL');
	var user_email_address = user_email_field.value;
	var submit_button = document.getElementById('sq_submit_accessible_captcha');

	user_email_field.disabled = 0;
	submit_button.value = 'Email message sent';
}


function submit_email_captcha(lib_url)
{
	var user_email_field = document.getElementById('SQ_SYSTEM_SECURITY_KEY_EMAIL');
	var user_email_address = user_email_field.value;


	// Allow for a@b.zz
	if (user_email_address.length > 5)
	{
		var submit_button = document.getElementById('sq_submit_accessible_captcha');

		user_email_field.disabled = 1;
		submit_button.disabled = 1;
		submit_button.value = 'Sending email...';

		// Send AJAX request
		var params = "email="+user_email_address;
		JsHttpConnector.submitRequest(lib_url+'/web/accessible_captcha.php?'+params, email_captcha_sent);
	}
}


function enable_submission_button()
{
	var submit_button = document.getElementById('sq_submit_accessible_captcha');

	submit_button.value = 'Submit address';
	submit_button.disabled = 0;
}
