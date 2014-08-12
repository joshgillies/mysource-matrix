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
* $Id: mollom.inc,v 1.6 2013/04/08 08:02:22 ewang Exp $
*
*/

define('SQ_MOLLOM_SPAM', 0);
define('SQ_MOLLOM_UNSURE', 1);
define('SQ_MOLLOM_HAM', 2);
define('SQ_MOLLOM_UNKNOWN', 3);
define('SQ_MOLLOM_INCORRECT_CAPTCHA', 4);

require_once SQ_DATA_PATH.'/private/conf/tools.inc';


/**
* Mollom
*
* Purpose: Allows Matrix to communicate with Mollom (http://mollom.com)
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.6 $
* @package Fudge
*/


/**
 * Mollom class
 *
 * This source file can be used to communicate with mollom (http://mollom.com)
 *
 * The class is documented in the file itself, but you can find more documentation and examples on the docs-page (http://mollom.crsolutions.be/docs).
 * If you find any bugs help me out and report them. Reporting can be done by sending an email to php-mollom-bugs[at]verkoyen[dot]eu. If you report a bug, make sure you give me enough ination (include your code).
 * If you have questions, try the Mollom-forum, don't send them by mail, I won't read them.
 *
 * Changelog since 1.0.1
 * - Fixed a nasty bug. Possible infinite loop in doCall().
 * - Fixed getServerList. I misinterpreted the documentation, so now the defaultserver is xmlrpc.mollom.com instead of the first fallback-server.
 * - Fixed the timeout-issue. With fsockopen the timeout on connect wasn't respected. Rewrote the doCall function to use CURL over sockets.
 *
 * Changelog since 1.1.0
 * - Fixed a problem with the IP-addresses. see http://blog.verkoyen.eu/2008/07/12/important-php-mollom-update/
 *
 * Changelog since 1.1.1
 * - PHPMollom was using HTTP 1.1, now HTTP 1.0.
 * - Fallbackserver are hardcoded, when no servers could be retrieved the fallbacks are used
 *
 * Changelog since 1.1.2
 * - Typo
 * - New Licence: BSD Modified
 *
 * License
 * Copyright (c) 2008, Tijs Verkoyen. All rights reserved.
 *
 * Redistribution and use in source and binary s, with or without modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary  must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products derived from this software without specific prior written permission.
 *
 * This software is provided by the author ``as is'' and any express or implied warranties, including, but not limited to, the implied warranties of merchantability and fitness for a particular purpose are disclaimed. In no event shall the author be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.
 *
 * @author			Tijs Verkoyen <mollom@verkoyen.eu>
 * @version			1.1.3
 *
 * @copyright		Copyright (c) 2008, Tijs Verkoyen. All rights reserved.
 * @license			http://mollom.local/license BSD License
 */
class Mollom
{
	/**
	 * The allowed reverse proxy addresses
	 *
	 * @var	array
	 */
	private static $allowedReverseProxyAddresses = array();


	/**
	 * Your private key
	 *
	 * Set it by calling Mollom::setPrivateKey('<your-key>');
	 *
	 * @var	string
	 */
	private static $privateKey;


	/**
	 * Your public key
	 *
	 * Set it by calling Mollom::setPublicKey('<your-key>');
	 *
	 * @var	string
	 */
	private static $publicKey;


	/**
	 * Reverse proxy allowed?
	 *
	 * @var	bool
	 */
	private static $reverseProxy = false;


	/**
	 * The default server
	 *
	 * No need to change
	 *
	 * @var	string
	 */
	private static $serverHost = 'http://xmlrpc.mollom.com';


	/**
	 * The cache for the serverlist
	 *
	 * No need to change
	 *
	 * @var	array
	 */
	private static $serverList = array();


	/**
	 * Default timeout
	 *
	 * @var	int
	 */
	private static $timeout = 10;


	/**
	 * The default user-agent
	 *
	 * Change it by calling Mollom::setUserAgent('<your-user-agent>');
	 *
	 * @var	string
	 */
	private static $userAgent = 'MollomPHP/1.1.3';


	/**
	 * The current Mollom-version
	 *
	 * No need to change
	 *
	 * @var	string
	 */
	private static $version = '1.0';


	/**
	 * Mollom session expire time (in minutes)
	 *
	 * If the form with Mollom captcha is idle for more then specified time 
	 * the Mollom session is refreshed
	 *
	 * @var int
	 */
	private static $session_expire_time = 30;

	/**
	 * Default server list
	 *
	 * @var array
	 */
	public static $default_server_list = array('http://xmlrpc3.mollom.com', 'http://xmlrpc2.mollom.com', 'http://xmlrpc1.mollom.com');

	/**
	 * Build the value so we can use it in XML-RPC requests
	 *
	 * @return	string
	 * @param	mixed $value
	 */
	private function buildValue($value)
	{
		// get type
		$type = gettype($value);

		// build value
		switch ($type)
		{
			case 'string':
				// escape it, cause Mollom can't handle CDATA (no pun intended)
				$value = htmlspecialchars($value, ENT_QUOTES, 'ISO-8859-15');
				return '<value><string>'. $value .'</string></value>'."\n";

			case 'array':
				// init struct
				$struct = '<value>'."\n";
				$struct .= '	<struct>'."\n";

				// loop array
				foreach ($value as $key => $value) $struct .= str_replace("\n", '', '<member>'. "\n" .'<name>'. $key .'</name>'. self::buildValue($value) .'</member>') . "\n";

				$struct .= '	</struct>'."\n";
				$struct .= '</value>'."\n";

				// return
				return $struct;

			default:
				return '<value>'. $value .'</value>'."\n";
		}
	}


	/**
	 * Validates the answer for a CAPTCHA
	 *
	 * When the answer is false, you should request a new image- or audio-CAPTCHA, make sure your provide the current Mollom-sessionid.
	 * The sessionid will be used to track spambots that try to solve CAPTHCA's by brute force.
	 *
	 * @return	bool
	 * @param	string $sessionId
	 * @param	string $solution
	 */
	public static function checkCaptcha($sessionId, $solution)
	{
		// redefine
		$sessionId = (string) $sessionId;
		$solution = (string) $solution;
		
		// set autor ip
		$authorIp = self::getIpAddress();

		// set parameters
		$parameters['session_id'] = $sessionId;
		$parameters['solution'] = $solution;
		if($authorIp != null) $parameters['author_ip'] = (string) $authorIp;

		// do the call
		$responseString = self::doCall('checkCaptcha', $parameters);
		// validate
		if(!isset($responseString->params->param->value->boolean)) throw new Exception('Invalid response in checkCapthca.');

		// return
		if((string) $responseString->params->param->value->boolean == '1') return true;

		// fallback
		return false;
	}


	/**
	 * Check if the data is spam or not, and gets an assessment of the datas quality
	 *
	 * This function will be used most. The more data you submit the more accurate the claasification will be.
	 * If the spamstatus is 'unsure', you could send the user an extra check (eg. a captcha).
	 *
	 * REMARK: the Mollom-sessionid is NOT related to HTTP-session, so don't send 'session_id'.
	 *
	 * The function will return an array with 3 elements:
	 * - spam			the spam-status (unknown/ham/spam/unsure)
	 * - quality		an assessment of the content's quality (between 0 and 1)
	 * - session_id		Molloms session_id
	 *
	 * @return	array
	 * @param	string[optional] $sessionId
	 * @param	string[optional] $postTitle
	 * @param	string[optional] $postBody
	 * @param	string[optional] $authorName
	 * @param	string[optional] $authorUrl
	 * @param	string[optional] $authorEmail
	 * @param	string[optional] $authorOpenId
	 * @param	string[optional] $authorId
	 */
	public static function checkContent($sessionId = null, $postTitle = null, $postBody = null, $authorName = null, $authorUrl = null, $authorEmail = null, $authorOpenId = null, $authorId = null)
	{	
		// validate
		if($sessionId === null && $postTitle === null && $postBody === null && $authorName === null && $authorUrl === null && $authorEmail === null && $authorOpenId === null && $authorId === null) throw new Exception('Specify at least on argument');

		// init var
		$parameters = array();
		$aReturn = array();

		// add parameters
		if($sessionId !== null) $parameters['session_id'] = (string) $sessionId;
		if($postTitle !== null) $parameters['post_title'] = (string) $postTitle;
		if($postBody !== null) $parameters['post_body'] = (string) $postBody;
		if($authorName !== null) $parameters['author_name'] = (string) $authorName;
		if($authorUrl !== null) $parameters['author_url'] = (string) $authorUrl;
		if($authorEmail !== null) $parameters['author_mail'] = (string) $authorEmail;
		if($authorOpenId != null) $parameters['author_openid'] = (string) $authorOpenId;
		if($authorId != null) $parameters['author_id'] = (string) $authorId;

		// set autor ip
		$authorIp = self::getIpAddress();
		if($authorIp != null) $parameters['author_ip'] = (string) $authorIp;

		if (SQ_TOOL_MOLLOM_TEST_MODE) {
			$parameters['testing'] = TRUE;
		}

		// do the call
		$responseString = self::doCall('checkContent', $parameters);

		// validate
		if(!isset($responseString->params->param->value->struct->member)) throw new Exception('Invalid response in checkContent.');

		// loop parts
		foreach ($responseString->params->param->value->struct->member as $part)
		{
			// get key
			$key = $part->name;
			
			// get value
			switch ($part->name)
			{
				case 'spam':
					$value = (string) $part->value->int;

					switch($value)
					{
						case '0':
							$aReturn['spam'] = 'unknown';
						break;

						case '1':
							$aReturn['spam'] = 'ham';
						break;

						case '2':
							$aReturn['spam'] = 'spam';
						break;

						case '3':
							$aReturn['spam'] = 'unsure';
						break;
					}
				break;

				case 'quality':
					$aReturn['quality'] = (float) $part->value->double;
				break;

				case 'session_id':
					$aReturn['session_id'] = (string) $part->value->string;
				break;
			}
		}

		// return
		return $aReturn;
	}


	/**
	 * Make the call
	 *
	 * @return	SimpleXMLElement
	 * @param	string $method
	 * @param	array[optional] $parameters
	 */
	private static function doCall($method, $parameters = array(), $server = null, $counter = 0, $test_mode = true)
	{
		// count available servers
		$countServerList = count(self::$serverList);

		if($server === null && $countServerList == 0) throw new Exception('No servers found, populate the serverlist. See setServerList().');

		// redefine var
		$method = (string) $method;
		$parameters = (array) $parameters;

		// possible methods
		$aPossibleMethods = array('checkCaptcha', 'checkContent', 'getAudioCaptcha', 'getImageCaptcha', 'getServerList', 'getStatistics', 'sendFeedback', 'verifyKey');

		// check if method is valid
		if(!in_array($method, $aPossibleMethods)) throw new Exception('Invalid method. Only '. implode(', ', $aPossibleMethods) .' are possible methods.');

		// check if public key is set
		if(self::$publicKey === null) throw new Exception('Public key wasn\'t set.');

		// check if private key is set
		if(self::$privateKey === null) throw new Exception('Private key wasn\'t set.');

		// still null
		if($server === null) $server = self::$serverList[$counter];

		// cleanup server string
		$server = str_replace(array('http://', 'https://'), '', $server);

		// create timestamp
		$time = gmdate("Y-m-d\TH:i:s.\\0\\0\\0O", time());

		// create nonce
		$nonce = md5(time());

		// create has
		$hash = base64_encode(
					pack("H*", sha1((str_pad(self::$privateKey, 64, chr(0x00)) ^ (str_repeat(chr(0x5c), 64))) .
					pack("H*", sha1((str_pad(self::$privateKey, 64, chr(0x00)) ^ (str_repeat(chr(0x36), 64))) .
					$time . ':'. $nonce .':'. self::$privateKey))))
				);

		// add parameters
		$parameters['public_key'] = self::$publicKey;
		$parameters['time'] = $time;
		$parameters['hash'] = $hash;
		$parameters['nonce'] = $nonce;


		// build request
		$requestBody = '<?xml version="1.0"?>' ."\n";
		$requestBody .= '<methodCall>' ."\n";
		$requestBody .= '	<methodName>mollom.'. $method .'</methodName>' ."\n";
		$requestBody .= '	<params>' ."\n";
		$requestBody .= '		<param>'."\n";
		$requestBody .= '			'. self::buildValue($parameters) ."\n";
		$requestBody .= '		</param>'."\n";
		$requestBody .= '	</params>' ."\n";
		$requestBody .= '</methodCall>' ."\n";

		$options = array(
				    'USERAGENT'      => self::$userAgent,
					'HTTP_VERSION'   => CURL_HTTP_VERSION_1_0,
					'POST'           => true,
					'FOLLOWLOCATION' => true,
					'HEADER'         => true,
					'RETURNTRANSFER' => true,
					'CONNECTTIMEOUT' => self::$timeout,
					'TIMEOUT'        => self::$timeout,
					'POSTFIELDS'     => $requestBody,
				);

		$details     = fetch_url($server.'/'.self::$version, $options);
		$response    = $details['response'];
		$errorNumber = $details['errornumber'];
		$errorString = $details['errorstring'];

		// validate response
		if($response === false || $errorNumber != 0)
		{
			// increment counter
			$counter++;

			// no servers left
			if($errorNumber == 28 && !isset(self::$serverList[$counter]) && $countServerList != 0) throw new Exception('No more servers available, try to increase the timeout.');

			// timeout
			elseif($errorNumber == 28 && isset(self::$serverList[$counter])) return self::doCall($method, $parameters, self::$serverList[$counter], $counter);

			// other error
			else throw new Exception('Something went wrong. Maybe the following message can be handy.<br />'. $errorString, $errorNumber);
		}

		// process response
		$parts = explode("\r\n\r\n", $response);

		// validate
		if(!isset($parts[0]) || !isset($parts[1])) throw new Exception('Invalid response in doCall.');

		// get headers
		$headers = $parts[0];

		// rebuild body
		array_shift($parts);
		$body = implode('', $parts);

		// validate header
		$aValidHeaders = array('HTTP/1.0 200', 'HTTP/1.1 200');
		if(!in_array(substr($headers, 0, 12), $aValidHeaders)) throw new Exception('Invalid headers.');

		// do some validation
		$responseXML = @simplexml_load_string($body);
		if($responseXML === false) throw new Exception('Invalid body.');

		if(isset($responseXML->fault))
		{
			$code = (isset($responseXML->fault->value->struct->member[0]->value->int)) ? (int) $responseXML->fault->value->struct->member[0]->value->int : 'unknown';
			$message = (isset($responseXML->fault->value->struct->member[1]->value->string)) ? (string) $responseXML->fault->value->struct->member[1]->value->string : 'unknown';

			// handle errors
			switch ($code)
			{
				// code 1200 (Server too busy)
				case 1200:
					if(self::$serverList === null) self::getServerList();

					// do call again
					return self::doCall($method, $parameters, self::$serverList[$counter], $counter++);
				break;
				
				// code 1000 (Parse error or internal problem)
				case 1000:

				// code 1100 (Serverlist outdated)
				case 1100:

				default:
					log_dump('Mollom Error '.$code .': '. $message);
					return FALSE;
			}//end switch
		}

		// return
		return $responseXML;
	}


	/**
	 * Get a CAPTCHA-mp3 generated by Mollom
	 *
	 * If your already called getAudioCaptcha make sure you provide at least the $sessionId. It will be used
	 * to identify visitors that are trying to break a CAPTCHA with brute force.
	 *
	 * REMARK: the Mollom-sessionid is NOT related to HTTP-session, so don't send 'session_id'.
	 *
	 * The function will return an array with 3 elements:
	 * - session_id		the session_id from Mollom
	 * - url			the url to the mp3-file
	 * - html			html that can be used on your website to display the CAPTCHA
	 *
	 * @return	array
	 * @param	string[optional] $sessionId
	 */
	public static function getAudioCaptcha($sessionId = null)
	{
		// init vars
		$aReturn = array();
		$parameters = array();

		// set autor ip
		$authorIp = self::getIpAddress();

		// set parameters
		if($sessionId != null) $parameters['session_id'] = (string) $sessionId;
		if($authorIp != null) $parameters['author_ip'] = (string) $authorIp;

		// do the call
		$responseString = self::doCall('getAudioCaptcha', $parameters);

		// validate
		if(!isset($responseString->params->param->value->struct->member)) throw new Exception('Invalid response in getAudioCaptcha.');

		// loop elements
		foreach ($responseString->params->param->value->struct->member as $part) $aReturn[(string) $part->name] = (string) $part->value->string;

		// add image html
		$aReturn['html'] = '<object type="audio/mpeg" data="'. $aReturn['url'] .'" width="50" height="16">'."\n"
								."\t".'<param name="autoplay" value="false" />'."\n"
								."\t".'<param name="controller" value="true" />'."\n"
							.'</object>';

		// return
		return $aReturn;
	}


	/**
	 * Get a CAPTCHA-image generated by Mollom
	 *
	 * If your already called getImageCaptcha make sure you provide at least the $sessionId. It will be used
	 * to identify visitors that are trying to break a CAPTCHA with brute force.
	 *
	 * REMARK: the Mollom-sessionid is NOT related to HTTP-session, so don't send 'session_id'.
	 *
	 * The function will return an array with 3 elements:
	 * - session_id		the session_id from Mollom
	 * - url			the url to the image
	 * - html			html that can be used on your website to display the CAPTCHA
	 *
	 * @return	array
	 * @param	string[optional] $sessionId
	 */
	public static function getImageCaptcha($sessionId = null)
	{
		// init vars
		$aReturn = array();
		$parameters = array();

		// set autor ip
		$authorIp = self::getIpAddress();

		// set parameters
		if($sessionId !== null) $parameters['session_id'] = (string) $sessionId;
		if($authorIp !== null) $parameters['author_ip'] = (string) $authorIp;

		// do the call
		$responseString = self::doCall('getImageCaptcha', $parameters);

		// validate
		if(!isset($responseString->params->param->value->struct->member)) throw new Exception('Invalid response in getImageCaptcha.');

		// loop elements
		foreach ($responseString->params->param->value->struct->member as $part) $aReturn[(string) $part->name] = (string) $part->value->string;

		// add image html
		$aReturn['html'] = '<img src="'. $aReturn['url'] .'" alt="Mollom CAPTCHA" />';

		// return
		return $aReturn;
	}


	/**
	 * Get the real IP-address
	 *
	 * @return	string
	 */
	public static function getIpAddress()
	{
		// pre check
		if(!isset($_SERVER['REMOTE_ADDR'])) return null;

		// get ip
		$ipAddress = $_SERVER['REMOTE_ADDR'];

		if(self::$reverseProxy)
		{
			if(isset($_SERVER['HTTP_X_FORWARDED_FOR']))
			{
				if(!empty(self::$allowedReverseProxyAddresses) && in_array($ipAddress, self::$allowedProxyAddresses, true))
				{
					return array_pop(explode(',', $_SERVER['HTTP_X_FORWARDED_FOR']));
				}
   			}

   			// running in a cluster environment
			if(isset($_SERVER['HTTP_X_CLUSTER_CLIENT_IP'])) return $_SERVER['HTTP_X_CLUSTER_CLIENT_IP'];
		}

		// fallback
		return $ipAddress;
	}


	/**
	 * Obtains a list of valid servers
	 *
	 * @return	array
	 */
	public static function getServerList($counter = 0)
	{	
		// Use default list of servers to get the server list from Mollom
		self::$serverList = self::$default_server_list;

		// do the call
		$responseString = self::doCall('getServerList', array(), self::$serverHost, $counter);

		// validate
		if(!isset($responseString->params->param->value->array->data->value)) throw new Exception('Invalid response in getServerList.');
		
		// Refresh the list
		self::$serverList = array();

		// loop servers and add them
		foreach ($responseString->params->param->value->array->data->value as $server) self::$serverList[] = (string) $server->string;

		if(count(self::$serverList) == 0) self::$serverList = self::$default_server_list; 

		// return
		return self::$serverList;
	}


	/**
	 * Retrieve statistics from Mollom
	 *
	 * Allowed types are listed below:
	 * - total_days				Number of days Mollom has been used
	 * - total_accepted			Total of blocked spam
	 * - total_rejected			Total accepted posts (not working?)
	 * - yesterday_accepted		Amount of spam blocked yesterday
	 * - yesterday_rejected		Number of posts accepted yesterday (not working?)
	 * - today_accepted			Amount of spam blocked today
	 * - today_rejected			Number of posts accepted today (not working?)
	 *
	 * @return	int
	 * @param	string $type
	 */
	public static function getStatistics($type)
	{
		// possible types
		$aPossibleTypes = array('total_days', 'total_accepted', 'total_rejected', 'yesterday_accepted', 'yesterday_rejected', 'today_accepted', 'today_rejected');

		// redefine
		$type = (string) $type;

		// validate
		if(!in_array($type, $aPossibleTypes)) throw new Exception('Invalid type. Only '. implode(', ', $aPossibleTypes) .' are possible types.');

		// do the call
		$responseString = self::doCall('getStatistics', array('type' => $type));

		// validate
		if(!isset($responseString->params->param->value->int)) throw new Exception('Invalid response in getStatistics.');

		// return
		return (int) $responseString->params->param->value->int;
	}


	/**
	 * Send feedback to Mollom.
	 *
	 * With this method you can help train Mollom. Implement this method if possible. The more feedback is provided the more accurate
	 * Mollom will be.
	 *
	 * Allowed feedback-strings are listed below:
	 * - spam			Spam or unsolicited advertising
	 * - profanity		Obscene, violent or profane content
	 * - low-quality	Low-quality content or writing
	 * - unwanted		Unwanted, taunting or off-topic content
	 *
	 * @return	bool
	 * @param	string $sessionId
	 * @param	string $feedback
	 */
	public static function sendFeedback($sessionId, $feedback)
	{
		// possible feedback
		$aPossibleFeedback = array('spam', 'profanity', 'low-quality', 'unwanted');

		// redefine
		$sessionId = (string) $sessionId;
		$feedback = (string) $feedback;

		// validate
		if(!in_array($feedback, $aPossibleFeedback)) throw new Exception('Invalid feedback. Only '. implode(', ', $aPossibleFeedback) .' are possible feedback-strings.');

		// build parameters
		$parameters['session_id'] = $sessionId;
		$parameters['feedback'] = $feedback;

		// do the call
		$responseString = self::doCall('sendFeedback', $parameters);

		// validate
		if(!isset($responseString->params->param->value->boolean)) throw new Exception('Invalid response in sendFeedback.');

		// return
		if((string) $responseString->params->param->value->boolean == 1) return true;

		// fallback
		return false;
	}


	/**
	 * Set the allowed reverse proxy Addresses
	 *
	 * @return	void
	 * @param	array $addresses
	 */
	public static function setAllowedReverseProxyAddresses($addresses)
	{
		// store allowed ip-addresses
		self::$allowedReverseProxyAddresses = (array) $addresses;

		// set reverse proxy
		self::$reverseProxy = (!empty($addresses)) ? true : false;
	}


	/**
	 * Set the private key
	 *
	 * @return	void
	 * @param	string $key
	 */
	public static function setPrivateKey($key)
	{
		self::$privateKey = (string) $key;
	}


	/**
	 * Set the public key
	 *
	 * @return	void
	 * @param	string $key
	 */
	public static function setPublicKey($key)
	{
		self::$publicKey = (string) $key;
	}


	/**
	 * Set the server list
	 *
	 * @return	void
	 * @param	array $servers
	 */
	public static function setServerList($servers)
	{
		// redefine
		$server = (array) $servers;

		// loop servers
		foreach ($servers as $server) self::$serverList[] = $server;
	}


	/**
	 * Set timeout
	 *
	 * @return	void
	 * @param	int $timeout
	 */
	public static function setTimeOut($timeout)
	{
		// redefine
		$timeout = (int) $timeout;

		// validate
		if($timeout == 0) throw new Exception('Invalid timeout. Timeout shouldn\'t be 0.');

		// set property
		self::$timeout = $timeout;
	}


	/**
	 * Set the user agent
	 *
	 * @return	void
	 * @param	string $newUserAgent
	 */
	public static function setUserAgent($newUserAgent)
	{
		self::$userAgent .=  ' '. (string) $newUserAgent;
	}


	/**
	 * Verifies your key
	 *
	 * Returns ination about the status of your key. Mollom will respond with a boolean value (true/false).
	 * False means that your keys is disabled or doesn't exists. True means the key is enabled and working properly.
	 *
	 * @return	bool
	 */
	public static function verifyKey()
	{
		// do the call
		$responseString = self::doCall('verifyKey');

		// validate
		if(!isset($responseString->params->param->value->boolean)) return false;

		// return
		if((string) $responseString->params->param->value->boolean == '1') return true;

		// fallback
		return false;
	}
	
	
	//--   MATRIX MOLLOM FUNCTIONS        --//
	
	
	/**
	* Get all the Mollom content types
	*
	* @return array
	*/
	 public static function getContentTypes()
	 {
		return Array(
				0 => "Post Title",
				1 => "Post Body",
				2 => "Author Name",
				3 => "Author URL",
				4 => "Author Mail",
				5 => "Author OpenId",
				6 => "Author Id",					
			);
	 }
	 

	/* Set and loads all the required Mollom parameters
	*
	* @return boolean
	* @access private
	*/	
	public static function loadMollom()
	{	
		if (!SQ_TOOL_MOLLOM_PUBLIC_KEY || !SQ_TOOL_MOLLOM_PRIVATE_KEY) {
			return FALSE;
		}
		
		self::setPublicKey(SQ_TOOL_MOLLOM_PUBLIC_KEY);
		self::setPrivateKey(SQ_TOOL_MOLLOM_PRIVATE_KEY);
		
		$server_list = Array();
		if (!empty($_SESSION['SQ_MOLLOM_SERVER_LIST'])) {
			$server_list = unserialize($_SESSION['SQ_MOLLOM_SERVER_LIST']);
			self::setServerList($server_list);			
		} else {		
			$server_list = self::getServerList();
			$_SESSION['SQ_MOLLOM_SERVER_LIST'] = serialize($server_list);
		}		
		
		return !empty($server_list);
	}
	

	/**
	* Verify the content for spam
	*
	* @param string	$assetid	Assetid of Matrix asset using the Mollom
	* @param array	$parameters
	*
	* @return integer
	* @access public
	*/
	public static function verifySubmission($assetid, $parameters)
	{
		// Whether to accept the submission if Mollom fails to respond expectedly
		$unknown_submission = SQ_TOOL_MOLLOM_ACCEPT_BYPASSED_SUBMISSION ? SQ_MOLLOM_HAM : SQ_MOLLOM_SPAM;

		try {
			self::refreshMollomSession($assetid);

			if (!self::loadMollom()) {
				// Cannot load Mollom client, hence cannot check the response for spam.
				return $unknown_submission;
			}

			if (self::checkCaptchaResponse($assetid)) {
				// Captcha check correct
				unset($_SESSION['SQ_MOLLOM_CAPTCHA_'.$assetid]);
				return SQ_MOLLOM_HAM;

			} else if (isset($_SESSION['SQ_MOLLOM_CAPTCHA_'.$assetid])) {
				return SQ_MOLLOM_INCORRECT_CAPTCHA;
			}

			$response = self::checkContent(
									null,
									isset($parameters[0]) ? $parameters[0] : null,
									isset($parameters[1]) ? $parameters[1] : null,
									isset($parameters[2]) ? $parameters[2] : null,
									isset($parameters[3]) ? $parameters[3] : null,
									isset($parameters[4]) ? $parameters[4] : null,
									isset($parameters[5]) ? $parameters[5] : null,
									isset($parameters[6]) ? $parameters[6] : null
				);

			if (empty($response)) {
				// No response from server, cannot check spam. Accept it anyway
				// Better accept a spam rather than reject a legit answer
				return $unknown_submission;
			}

			$spam_status 	= array_get_index($response, 'spam', 'unknown');
			$quality 		= array_get_index($response, 'quality', 0);
			$session_id 	= array_get_index($response, 'session_id', '');

			switch ($spam_status) {

				case 'unsure':
					$mollom_captcha = Array();
					$image_captcha = self::getImageCaptcha($session_id);
					$mollom_captcha['session_id'] = $session_id;
					$mollom_captcha['image_url'] = array_get_index($image_captcha, 'url', '');
					$mollom_captcha['session_time'] = time();

					$_SESSION['SQ_MOLLOM_CAPTCHA_'.$assetid] = $mollom_captcha;
					return SQ_MOLLOM_UNSURE;

				case 'spam':
					// Reject the answer
					return SQ_MOLLOM_SPAM;

				case 'ham':
					// Its not a spam
					return SQ_MOLLOM_HAM;
			}
		} catch (Exception $e) {
			$add_msg = SQ_TOOL_MOLLOM_ACCEPT_BYPASSED_SUBMISSION ? 'Submission accepted anyway.' : 'Submission not accepted.';
			trigger_error("Error occured when trying to verify the submission with the Mollom: ".$e->getMessage()."\n".$add_msg, E_USER_WARNING);
		}

		return $unknown_submission;

	}//end verifySubmission()


	//--   MOLLOM MATRIX KEYWORD REPLACEMENT FUNCTIONS        --//
	

	/**
	* Get Mollom captcha  keyword replacement 
	*	
	* @param string	$assetid	Assetid of Matrix asset using the Mollom
	* @param array	$errors	Errors messages to display
	*
	* @return string
	* @access public
	*/
	public static function getMollomCaptchaKeywordReplacement($assetid, $errors = Array())
	{	
		self::refreshMollomSession($assetid);

		if (empty($_SESSION['SQ_MOLLOM_CAPTCHA_'.$assetid]['session_id'])) {
			return null;
		}
		
		$captcha = self::getMollomCaptchaImageKeywordReplacement($assetid).
						self::getMollomCaptchaRegenLinkKeywordReplacement($assetid).
						"<br />".self::getMollomCaptchaFieldKeywordReplacement($assetid);
		if(!empty($errors)) {
		    $error = array_pop($errors);
		    $captcha .= "<br/>".self::getMollomCaptchaErrorsKeywordReplacement($assetid, $error);
		}

		return $captcha;
	}

	
	/**
	* Get captcha input field keyword replacement 
	*
	* @param string	$assetid	Assetid of Matrix asset using the Mollom
	*
	* @return string
	* @access public
	*/
	public static function getMollomCaptchaFieldKeywordReplacement($assetid)
	{	
		self::refreshMollomSession($assetid);
		
		if (empty($_SESSION['SQ_MOLLOM_CAPTCHA_'.$assetid]['session_id']) || (empty($_SESSION['SQ_MOLLOM_CAPTCHA_'.$assetid]['image_url']))) {
			return '';
		}

		$name 	= 'sq_mollom_captcha_'.$assetid.'_input';
		$width 	= 7;
		$max 	= 20;
		$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		$label = $form->attr('mollom_captcha_label_text');

		ob_start();
			label($label, $name);
			echo '<br/>';
			text_box($name, '', $width, $max, FALSE, '');
			hidden_field($name.'_session', $_SESSION['SQ_MOLLOM_CAPTCHA_'.$assetid]['session_id']);
			$input_box = ob_get_contents();
		ob_end_clean();

		return $input_box;
	}


	/**
	* Get Mollom captcha link regenerate keyword replacement 
	*
	* @param string	$assetid	Assetid of Matrix asset using the Mollom
	* @param string $regen_text	
	*
	* @return string
	* @access public
	*/
	public static function getMollomCaptchaRegenLinkKeywordReplacement($assetid, $regen_text='Regenerate security key')
	{
		self::refreshMollomSession($assetid);

		if (!empty($_SESSION['SQ_MOLLOM_CAPTCHA_'.$assetid]['session_id'])) {
			$image_id = 'sq_mollom_captcha_'.$assetid.'_image';
			$image_url = $_SESSION['SQ_MOLLOM_CAPTCHA_'.$assetid]['image_url'];

			return '<a href="#" onClick="var img=document.getElementById(\''.$image_id.'\'); img.src=\''.$image_url.'?SQ_MOLLOM_REGEN_PARAM=\'+Math.random(); return false;">'.$regen_text.'</a>';
		}

		return '';
	}

	
	/**
	* Get Mollom captcha audio keyword replacement
	*
	* @param string $assetid    Assetid of Matrix asset using the Mollom
	*
	* @return string
	* @access public
	*/
	public static function getMollomCaptchaAudioKeywordReplacement($assetid)
	{
		self::refreshMollomSession($assetid);
		
		if (!empty($_SESSION['SQ_MOLLOM_CAPTCHA_'.$assetid]['session_id'])) {
			if (empty($_SESSION['SQ_MOLLOM_CAPTCHA_'.$assetid]['audio_url'])) {
				if (!self::loadMollom()) {
					return '';
				}
				$audio_captcha = self::getAudioCaptcha($_SESSION['SQ_MOLLOM_CAPTCHA_'.$assetid]['session_id']);
				$_SESSION['SQ_MOLLOM_CAPTCHA_'.$assetid]['audio_url'] = $audio_captcha['url'];
			}

 			return '<object type="audio/mpeg" data="'.$_SESSION['SQ_MOLLOM_CAPTCHA_'.$assetid]['audio_url'].'" width="50" height="16">'."\n"
						."\t".'<param name="autoplay" value="false" />'."\n"
						."\t".'<param name="controller" value="true" />'."\n"
					.'</object>';
		}

		return '';
	}


	/**
	* Get Mollom captcha image keyword replacement
	*	
	* @param string	$assetid	Assetid of Matrix asset using the Mollom
	*
	* @return string
	* @access public
	*/
	public static function getMollomCaptchaImageKeywordReplacement($assetid)
	{
		self::refreshMollomSession($assetid);
			
		if (!empty($_SESSION['SQ_MOLLOM_CAPTCHA_'.$assetid]['image_url'])) {
			$image_id = 'sq_mollom_captcha_'.$assetid.'_image';
			return '<img id="'.$image_id.'" src="'.$_SESSION['SQ_MOLLOM_CAPTCHA_'.$assetid]['image_url'].'" alt="CAPTCHA Image" />';
		}

		return '';
	}


	/**
	* Get Mollom captcha error keyword replacement
	*	
	* @param string	$assetid	Assetid of Matrix asset using the Mollom
	* @param string	$error         Error message to display
	*
	* @return string
	* @access public
	*/
	public static function getMollomCaptchaErrorsKeywordReplacement($assetid, $error='Incorrect security key. Please try again')
	{
		self::refreshMollomSession($assetid);
			
		if (empty($_SESSION['SQ_MOLLOM_CAPTCHA_'.$assetid]['session_id'])) {
			return '';
		}
		
		$name 	= 'errror_sq_mollom_captcha_'.$assetid.'_input';
		
		$error_span = '<span id="'.$name.'" class="sq-form-error sq-captcha-error">'.$error.'</span>';

		return $error_span;
	}
	
	
    /**
    * Check the value submitted for captcha
    *
 	* @param string $assetid    Assetid of Matrix asset using the Mollom
	*
    * @return boolean
    * @access private
    */
    public static function checkCaptchaResponse($assetid)
    {
		if (!self::loadMollom()) {
			return FALSE;
		}

		$name = 'sq_mollom_captcha_'.$assetid.'_input';
        $captcha_solution = array_get_index($_POST, $name, '');
        $captcha_session_id = array_get_index($_POST, $name.'_session', '');
        
		if (empty($captcha_session_id) || empty($captcha_solution)) {
            return FALSE;
        }
        
		return self::checkCaptcha($captcha_session_id, $captcha_solution);
    }

	
	/**
	* Refresh Mollom session
	*
	* If the form having Mollom captcha is idle for more than given time, 
	* refresh the mollom session
	*
	* @return void
	* @access private
	*/
	public static function refreshMollomSession($assetid)
	{
		if (isset($_SESSION['SQ_MOLLOM_CAPTCHA_'.$assetid]['session_id'])) {
			
			$session_time = array_get_index($_SESSION['SQ_MOLLOM_CAPTCHA_'.$assetid], 'session_time', 0);
			$current_time = time();
			if (($current_time - $session_time) > (60 * self::$session_expire_time)) {
				unset($_SESSION['SQ_MOLLOM_CAPTCHA_'.$assetid]);
			}
		}
	}
	

}//end class Mollom

?>
