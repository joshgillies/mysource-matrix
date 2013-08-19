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
* $Id: recaptcha.inc,v 1.1 2013/08/19 04:01:37 cupreti Exp $
*
*/

require_once SQ_DATA_PATH.'/private/conf/tools.inc';


/**
* Matrix Recaptcha class
*
* Purpose: Allows Matrix to communicate with Google reCAPTCHA API
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.1 $
* @package Fudge
*/


/*
 * This is a PHP library that handles calling reCAPTCHA.
 *    - Documentation and latest version
 *          http://recaptcha.net/plugins/php/
 *    - Get a reCAPTCHA API Key
 *          https://www.google.com/recaptcha/admin/create
 *    - Discussion group
 *          http://groups.google.com/group/recaptcha
 *
 * Copyright (c) 2007 reCAPTCHA -- http://recaptcha.net
 * AUTHORS:
 *   Mike Crawford
 *   Ben Maurer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

class Recaptcha {

	/**
	* TRUE if validation is successful
	* @var boolean
	*/
	public $is_valid;


	/**
	* Recaptcha validation failure code
	* @var string
	*/
	public $error;


	/**
	* Recaptcha error codes
	* @var array
	*/
	static $error_codes = Array(
								'invalid-site-private-key' 	=> 'Invalid private API key',
								'invalid-request-cookie'	=> 'Invalid request cookie',
								'incorrect-captcha-sol'		=> 'Incorrect captcha key. Please try again',
								'captcha-timeout'			=> 'Captcha timed out. Please try again',
								'recaptcha-not-reachable'	=> 'Unable to contact the reCAPTCHA verify server. Please try again',
							);


	/**
    * The reCAPTCHA API server URL
    */
	const RECAPTCHA_API_SERVER = "http://www.google.com/recaptcha/api";


	/**
	* The reCAPTCHA API SECURE server URL
	*/
	const RECAPTCHA_API_SECURE_SERVER = "https://www.google.com/recaptcha/api";


	/**
	* The reCAPTCHA VERIFY server URL
	*/
	const RECAPTCHA_VERIFY_SERVER = "www.google.com";


	/**
	* A private constructor to privent from initialising this class from outside
	*
 	*/
	private function __construct()
	{

    }//end __construct()


	/**
	* Get the reCAPTCHA html
	*
	* @param array	$errors
	* @param array	$theme
	*
	* @return string
	* @access public
	*/
	public static function getRecaptcha($errors, $theme='red')
	{
		if (!SQ_TOOL_RECAPTCHA_PUBLIC_KEY || !SQ_TOOL_RECAPTCHA_PRIVATE_KEY) {
			trigger_error('Public and/or Private keys not set for reCAPTCHA', E_USER_WARNING);

			return '';
		}

		$captcha = '';
		try {
			$captcha = self::_recaptcha_get_html(SQ_TOOL_RECAPTCHA_PUBLIC_KEY, current_protocol() == 'https', $theme);

			// Include the error message, if any
			$captcha .= self::_recaptcha_error_html($errors);

		} catch (Exception $e) {
			trigger_error("Error occured when getting the reCAPTCHA: ".$e->getMessage(), E_USER_WARNING);
		}

		return $captcha;

	}//end getRecaptcha()


	/**
	* Verify the captcha
	*
	* @return object
	* @access public
	*/
	public static function verifyRecaptcha()
	{
		$challenge_field = isset($_POST["recaptcha_challenge_field"]) ? $_POST["recaptcha_challenge_field"] : FALSE;
		$response_field = isset($_POST["recaptcha_response_field"]) ? $_POST["recaptcha_response_field"] : FALSE;

		try {
			$result = self::_recaptcha_check_answer(SQ_TOOL_RECAPTCHA_PRIVATE_KEY, $_SERVER["REMOTE_ADDR"], $challenge_field, $response_field);
		} catch (Exception $e) {
			trigger_error("Error occured when verifying the reCAPTCHA: ".$e->getMessage(), E_USER_WARNING);

			$result = new self();
			$result->is_valid = FALSE;
			$err_code = 'recaptcha-not-reachable';
			$result->error = Array($err_code => self::$error_codes[$err_code]);
		}

		return $result;

	}//end getRecaptcha()


	/**
	* Submits an HTTP POST to a reCAPTCHA server
	*
	* @param array $errors
	*
	* @return object
	* @return private
	*/
	private static function _recaptcha_error_html($errors)
	{
		if (!is_array($errors) || empty($errors)) {
			return '';
		}

		$html = '';
		// There can possibly only be one error
		$msg = array_shift($errors);
		$html = '<span id="errror_sq_recaptcha_input" class="sq-form-error sq-captcha-error">'.$msg.'</span>';

		return $html;

	}//end _recaptcha_error_html()


	/**
	* Submits an HTTP POST to a reCAPTCHA server
	*
	* @param string $host
	* @param string $path
	* @param array $data
	* @param int port
	*
	* @return array response
	* @access private
	*/
	private static function _recaptcha_http_post($host, $path, $data, $port = 80)
	{

		$req = self::_recaptcha_qsencode ($data);

		$http_request  = "POST $path HTTP/1.0\r\n";
		$http_request .= "Host: $host\r\n";
		$http_request .= "Content-Type: application/x-www-form-urlencoded;\r\n";
		$http_request .= "Content-Length: " . strlen($req) . "\r\n";
		$http_request .= "User-Agent: reCAPTCHA/PHP\r\n";
		$http_request .= "\r\n";
		$http_request .= $req;

		$response = '';
		if(FALSE ==($fs = @fsockopen($host, $port, $errno, $errstr, 10))) {
			die ('Could not open socket');
		}

		fwrite($fs, $http_request);

		while ( !feof($fs) )
			$response .= fgets($fs, 1160); // One TCP-IP packet
		fclose($fs);
		$response = explode("\r\n\r\n", $response, 2);

		return $response;

	}//end _recaptcha_http_post()


	/**
	* Gets the challenge HTML (javascript and non-javascript version).
	* This is called from the browser, and the resulting reCAPTCHA HTML widget
	* is embedded within the HTML form it was called from.
	*
	* @param string  $pubkey A public key for reCAPTCHA
	* @param boolean $use_ssl Should the request be made over ssl? (optional, default is false)
	* @param string  $theme Theme to use to display the reCAPTCHA
	* @param string  $error The error given by reCAPTCHA (optional, default is null)
	*
	* @return string - The HTML to be embedded in the user's form.
	* @access public
	*/
	private static function _recaptcha_get_html($pubkey, $use_ssl=false, $theme='red', $error=NULL)
	{
		if ($use_ssl) {
					$server = self::RECAPTCHA_API_SECURE_SERVER;
			} else {
					$server = self::RECAPTCHA_API_SERVER;
			}

			$errorpart = "";
			if ($error) {
			   $errorpart = "&amp;error=" . $error;
			}
			return '
		<script type="text/javascript">
			var RecaptchaOptions = {
							    theme : \''.$theme.'\',
							 };
		</script>
		<script type="text/javascript" src="'. $server . '/challenge?k=' . $pubkey . $errorpart . '"></script>

		<noscript>
			<iframe src="'. $server . '/noscript?k=' . $pubkey . $errorpart . '" height="300" width="500" frameborder="0"></iframe><br/>
			<textarea name="recaptcha_challenge_field" rows="3" cols="40"></textarea>
			<input type="hidden" name="recaptcha_response_field" value="manual_challenge"/>
		</noscript>';

	}//end _recaptcha_get_html()


	/**
	* Calls an HTTP POST function to verify if the user's guess was correct
	*
	* @param string $privkey
	* @param string $remoteip
	* @param string $challenge
	* @param string $response
	* @param array $extra_params an array of extra variables to post to the server
	*
	* @return object
	* @access public
	*/
	private static function _recaptcha_check_answer($privkey, $remoteip, $challenge, $response, $extra_params = Array())
	{
		$recaptcha_response = new self();

		//discard spam submissions
		if (!$privkey || !$challenge || strlen($challenge) == 0 || !$response || strlen($response) == 0) {
			$recaptcha_response->is_valid = FALSE;
			$err_code = 'incorrect-captcha-sol';
			$recaptcha_response->error = Array($err_code => self::$error_codes[$err_code]);

			return $recaptcha_response;
		}

		$response = self::_recaptcha_http_post(self::RECAPTCHA_VERIFY_SERVER, "/recaptcha/api/verify",
											  Array(
													 'privatekey' => $privkey,
													 'remoteip' => $remoteip,
													 'challenge' => $challenge,
													 'response' => $response
													 ) + $extra_params
											  );

		$answers = explode ("\n", $response [1]);

		$recaptcha_response->is_valid = isset($answers [0]) && $answers [0] == 'true';
		$recaptcha_response->error =  Array();
		if (isset($answers[1]) && isset(self::$error_codes[$answers[1]])) {
			$recaptcha_response->error =  Array($answers[1] => self::$error_codes[$answers[1]]);
		}

		return $recaptcha_response;

	}//end _recaptcha_check_answer()


	/**
	 * Encodes the given data into a query string format
	 *
	 * @param $data - array of string elements to be encoded
	 *
	 * @return string - encoded request
	 * @access private
	 */
	private static function _recaptcha_qsencode($data)
	{
		$req = "";
		foreach ( $data as $key => $value )
			$req .= $key . '=' . urlencode( stripslashes($value) ) . '&';
		// Cut the last '&'
		$req=substr($req,0,strlen($req)-1);

		return $req;

	}//end _recaptcha_qsencode()

}//end class

?>
