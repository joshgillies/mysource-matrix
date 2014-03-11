<?php
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
* $Id: accessible_captcha.php,v 1.6 2013/07/25 23:23:50 lwright Exp $
*
*/


require_once dirname(dirname(dirname(__FILE__))).'/include/init.inc';


/**
* Send a CAPTCHA verification email to the supplied address
*
* @param string	$to_email_address	The destination email address
* @param string	$key				The shared CAPTCHA verification key (in the email and the user's session)
*
* @access public
* @return void
*/
function sendAccessibleCaptchaEmail($to_email_address, $key)
{
	require_once 'Mail.php';
	require_once 'Mail/mime.php';

	// Strip spaces from around the "To" address in case these are present
	$to_email_address = trim($to_email_address);

	// Provide the name of the system as supplied in the main.inc configuration for use in the email (if it is set)
	$from_address = '"Accessible CAPTCHA Form"';
	if (SQ_CONF_SYSTEM_NAME != '') {
		$from_system_name = 'from the '.SQ_CONF_SYSTEM_NAME.' website ';
		$from_address = SQ_CONF_SYSTEM_NAME;
	}

	// Quote the System Name as it could contain apos'rophes
	$from_address = '"'.$from_address.'"';

	$current_url = current_url();
	$body = 'This email has been generated '.$from_system_name."as part of a form submission which includes an Accessible CAPTCHA field.\n\n".
			"Please visit the following page to validate your submission before submitting the form\n\n".
			$current_url.'?key='.$key;

	$mime = new Mail_mime("\n");
	$mime->setTXTBody($body);

	$from_address .= ' <'.SQ_CONF_DEFAULT_EMAIL.'>';

	$headers = Array(
				'From'		=> $from_address,
				'Subject'	=> 'Accessible CAPTCHA Form Verification',
			   );

	$param = Array(
				'head_charset'  => SQ_CONF_DEFAULT_CHARACTER_SET,
				'text_charset'  => SQ_CONF_DEFAULT_CHARACTER_SET,
				'html_charset'  => SQ_CONF_DEFAULT_CHARACTER_SET,
			 );
	$body = @$mime->get($param);
	$headers = @$mime->headers($headers);
	$mail =& Mail::factory('mail');
	$status = @$mail->send($to_email_address, $headers, $body);

}//end sendAccessiblecaptchaEmail()


/**
* Verify the supplied CAPTCHA verification key against the one stored in the user's session
* If we have a match, then flag it as a pass so submission of the underlying form can proceed unhindered
*
* @param string	$key				The shared CAPTCHA verification key (in the email and the user's session)
*
* @access public
* @return void
*/
function validateAccessibleCaptcha($key)
{
	$verified_captcha = FALSE;

	if ((isset($_SESSION['SQ_ACCESSIBLE_CAPTCHA_KEY'])) && ($_SESSION['SQ_ACCESSIBLE_CAPTCHA_KEY'] === $key)) {
		// F.A.B - CAPTCHAs are go!
		$verified_captcha = TRUE;
		$_SESSION['SQ_ACCESSIBLE_CAPTCHA_PASSED'] = 1;

		// Clear the CAPTCHA key from the session and access to run this script
		unset($_SESSION['SQ_ACCESSIBLE_CAPTCHA_KEY']);
		unset($_SESSION['SQ_ACCESSIBLE_CAPTCHA_GENERATED']);
	}

	if ($verified_captcha) {
?>
<p>Thank you for verifying the Accessible CAPTCHA input.<br />
Please proceed to submit your form.</p>
<?php
	} else {
?>
<p>This Accessible CAPTCHA key has expired for this session.<br />
Please return to the form to generate a new key.</p>
<?php
	}

}//end validateAccessibleCaptcha()



/**
* The main bit
*
* GET requests:
* 'email'	=> Send a CAPTCHA verification to the supplied email address
* 'key'		=> Verify the supplied CAPTCHA key and flag the form as good
*/

// Ensure that the request originated from *this* Matrix system, otherwise drop the request like some sort of heavy feather
if (!isset($_SESSION['SQ_ACCESSIBLE_CAPTCHA_GENERATED'])) exit;

// Generate an Accessible CAPTCHA Key
if (isset($_GET['email'])) {

	$user_email = addslashes($_GET['email']);

	// Ensure that we have *one* valid email address
	if ((strpos($user_email, ',') === FALSE) || (strpos($user_email, ';') === FALSE)) {
		// sanitize email address
		if(function_exists('filter_var')) {
		    $user_email=filter_var($user_email, FILTER_SANITIZE_EMAIL);
		    if(!filter_var($user_email, FILTER_VALIDATE_EMAIL)) {
			exit;
		    }
		}
		else {
		    require_once SQ_FUDGE_PATH.'/general/www.inc';
		    if(!valid_email($user_email)) exit;
		}

		// Return a key to be used in an email message to clear this CAPTCHA hurdle
		// The trinity of email address, timestamp, user ID and a locally-generated integer should be unique enough to generate a robust key
        require_once SQ_FUDGE_PATH.'/general/security.inc';
		$local_megadice = security_rand(1, 1000000);

		$submission_time = time();
		$key = md5($user_email.$submission_time.$local_megadice);

		// Log our generated key in the user's session and send a nice email to the user for CAPTCHA validation
		$_SESSION['SQ_ACCESSIBLE_CAPTCHA_KEY'] = $key;
		sendAccessibleCaptchaEmail($user_email, $key);
	}

} else if (isset($_GET['key'])) {
// Verify an Accessible CAPTCHA Key from an email message

	$key = addslashes($_GET['key']);
	validateAccessibleCaptcha($key);

}

?>
