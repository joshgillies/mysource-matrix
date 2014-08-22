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
* $Id: soap_common.inc,v 1.13 2013/09/10 03:15:04 ewang Exp $
*
*/


require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once 'soap_form_based_common.inc';


// WSDL sometime defines a complex type in a loop, we need to get out of this loop if it repeats too many times while parsing
define('MAX_COMPLEX_TYPE_PASRSE_LOOP', 3);

/**
* SOAP_Common
* This class will have all the common functions used for SOAP trigger, form and form section actions,
* and SOAP Data Souruce asset
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @author  Ash Karelia <akarelia@squiz.com.au>
* @version $Revision: 1.13 $
* @package MySource_Matrix_Packages
* @subpackage web_service
*/


class SOAP_Common
{


	private static $recent_parsed_type = Array();

	public static $wsdl_caching_opts = Array(
											WSDL_CACHE_BOTH => 'Cache to both disk and memory',
											WSDL_CACHE_DISK => 'Cache to disk',
											WSDL_CACHE_MEMORY => 'Cache to memory',
											WSDL_CACHE_NONE => 'Never cache',
										);

	// Available SOAP authentication types
	public static $auth_types = Array(
									'HTTP_BASIC' => 'HTTP Basic',
									'JSESSIONID' => 'JSESSIONID Cookie',
									'WSS_USERNAME_TOKEN' => 'WSS Username Token',
									'WSS_PASSWORD_DIGEST' => 'WSS Password Digest',
									'MARKETO_SIGNATURE'	=> 'Marketo Signature',
								);



	/**
	* This function recursively checks an object properties and store them in session
	*
	* @return void
	* @access private
	*/
	public static function _store_results($object, $prefix = 'soap_', &$soap_response_keywords=Array(), $file_keywords)
	{
		if (empty($object)) return;

		$vars = is_object($object) ? get_object_vars($object) : $object;
		if (empty($vars)) return;

		foreach ($vars as $index => $value) {
			if (is_object($value) || is_array($value)) {
				self::_store_results ($value, $prefix.$index, $soap_response_keywords, $file_keywords);
			} else {
				// if the keyword we are trying to save is file
				// response and is configured to be saved
				// and to provide a link to the file
				// then deal with it
				if (in_array('%globals_session_'.$prefix.$index.'%', $file_keywords)) {
					$link_value = self::_save_file($value);
					$_SESSION[SQ_SESSION_SANDBOX_INDEX][$prefix.$index] = $link_value;
					$soap_response_keywords[$prefix.$index] = $link_value;
				} else {
					$_SESSION[SQ_SESSION_SANDBOX_INDEX][$prefix.$index] = $value;
					$soap_response_keywords[$prefix.$index] = $value;
				}
			}
		}

	}//end _store_results()


	/**
	* This function returns XML representation of param struct
	*
	* @return object | void
	* @access public
	*/
	public static function _parse_type($type, $types)
	{
		$type_name = preg_replace('/ .*/', '', $type);
		//$param_name = preg_replace('/.* (.*);/', '$1', $param);
		$sub_types = self::_is_complex_type($type_name, $types);

		// If simple type arguments
		if (!$sub_types) {
			return self::_is_array_type($type_name, $types);
		}//end if

		// reset infinity loop check counter
		self::$recent_parsed_type = Array();

		// It's a complext type, let's parse it recursively
		$node = new SimpleXMLElement('<'.$type_name.'/>');
		self::_parse_type_recursive($type, $types, $node);

		return $node;

	}//end _parse_type()

	/**
	* Checks if the given param is array
	*
	* @return boolean
	* @access private
	*/
	private static function _is_array_type($param, $types)
	{
		// Get array types
		$array_types = Array();
		foreach($types as $type) {
			if (substr($type, -2) == '[]') {
				$array_type = explode(' ', $type);
				if (count($array_type) > 1) {
					$array_types[rtrim($array_type[1], '[]')] = $array_type[0];
				}
			}
		}

		return isset($array_types[$param]);

	}//end _is_array_type()


	/**
	* This function recursively parse the complex param type
	*
	* @return null
	* @access private
	*/
	public static function _parse_type_recursive($param, $types, &$node)
	{
		// Keep recently parsed type, to do an infinity check
		foreach (self::$recent_parsed_type as $parsed) {
			if ($parsed == $param) return;
		}
		array_push(self::$recent_parsed_type, $param);
		if(count(self::$recent_parsed_type) > MAX_COMPLEX_TYPE_PASRSE_LOOP) {
			array_shift(self::$recent_parsed_type);
		}

		$param_type = preg_replace('/ .*/', '', $param);
		$param_name = preg_replace('/.* (.*);/', '$1', $param);

		$is_param_array = FALSE;
		$sub_types = self::_is_complex_type($param_type, $types);
		if(!$sub_types) return;

		// It's a complext type, let's parse its children
		foreach ($sub_types as $type) {
			$is_type_array = FALSE;
			$sub_type_type = preg_replace('/ .*/', '', $type);
			$sub_type_name = preg_replace('/.* (.*);/', '$1', $type);
			$sub_sub_types = self::_is_complex_type($sub_type_type, $types, $is_type_array);

			if (!$sub_sub_types || ($sub_type_type == $param_type)) {
				// it's a simple type
				$node->addChild($sub_type_name, '??');
			} else {
				// it's complex type, so step into its children
				$new_node = $node->addChild($sub_type_name.($is_type_array ? '[ARRAY_ELEMENT]' : ''));
				self::_parse_type_recursive($type, $types, $new_node);
			}
		}

	}//end _parse_type_recursive()


	/**
	* This function checks if a param is of complex type from types list, if so, return the sub types
	*
	* @return false | array
	* @access private
	*/
	public static function _is_complex_type($param, $types, &$is_array=FALSE)
	{
		// Get array types
		$array_types = Array();
		foreach($types as $type) {
			if (substr($type, -2) == '[]') {
				$array_type = explode(' ', $type);
				if (count($array_type) > 1) {
					$array_types[rtrim($array_type[1], '[]')] = $array_type[0];
				}
			}
		}
		if (isset($array_types[$param])) {
			$param = $array_types[$param];
			$is_array = TRUE;
		}

		foreach ($types as $type) {
			if (preg_match ('/struct '.$param.'\s*{/', $type)) {
				// find and return sub types
				preg_match_all ('/\w+ \w+;/', $type, $matches);
				return $matches[0];
			}
		}

		return FALSE;

	}//end is_complex_type()


	/**
	* This function formats a xml, inserts indentation, newlines for better display
	*
	* @return string
	* @access private
	*/
	public static function _formatXmlString($xml)
	{
	  // add marker linefeeds to aid the pretty-tokeniser (adds a linefeed between all tag-end boundaries)
	  $xml = preg_replace('/(>)(<)(\/*)/', "$1\n$2$3", $xml);

	  // now indent the tags
	  $token      = strtok($xml, "\n");
	  $result     = ''; // holds formatted version as it is built
	  $pad        = 0; // initial indent
	  $matches    = array(); // returns from preg_matches()

	  // scan each line and adjust indent based on opening/closing tags
	  while ($token !== false) :

	    // test for the various tag states

	    // 1. open and closing tags on same line - no change
	    if (preg_match('/.+<\/\w[^>]*>$/', $token, $matches)) :
	      $indent=0;
	    // 2. closing tag - outdent now
	    elseif (preg_match('/^<\/\w/', $token, $matches)) :
	      $pad--;
	    // 3. opening tag - don't pad this one, only subsequent tags
	    elseif (preg_match('/^<\w[^>]*[^\/]>.*$/', $token, $matches)) :
	      $indent=1;
	    // 4. no indentation needed
	    else :
	      $indent = 0;
	    endif;

	    // pad the line with the required number of leading spaces
	    $line    = str_pad($token, strlen($token)+$pad, ' ', STR_PAD_LEFT);
	    $result .= $line . "\n"; // add to the cumulative result, with linefeed
	    $token   = strtok("\n"); // get the next token
	    $pad    += $indent; // update the pad size for subsequent lines
	  endwhile;

	  return $result;

	}// end _formatXmlString()


	/**
	* This function recursively extarcts available session keywords from response message
	*
	* @return string
	* @access private
	*/
	public static function _parse_response_keywords (&$response_xml, $prefix = '')
	{
		if (!is_object($response_xml)) return '';

		$data = get_object_vars($response_xml);
		$keywords_list = '';

		foreach ($data as $index => $sub_object) {
			if(is_object($sub_object) && !empty($sub_object)) {
				$keywords_list .= self::_parse_response_keywords($sub_object, $prefix.$index);
			} else {
				$keywords_list .= '%'.$prefix.$index.'%<br/>';
			}
		}

		return $keywords_list;

	}//end _parse_response_keywords


	/**
	* Get soap options from setting, e.g http auth, https cert.
	*
	* @param array	$settings	the settings of our action we need to extract information from
	* @param object	$form		optionally pass a form object to store the error in
	*
	* @return array
	* @access private
	*/
	public static function _getSoapOptions($settings, $form=NULL)
	{
		// Work out HTTP auth
		$auth_type = array_get_index($settings, 'auth_type', 'HTTP_BASIC');
		if ($auth_type == 'HTTP_BASIC' && isset($settings['login']) && isset($settings['password'])) {
			$options['login'] = $settings['login'];
			$options['password'] = $settings['password'];
		}

		// Work out HTTPS certificate and passphrase
		if(!empty($settings['local_cert'])) {
			$file = $GLOBALS['SQ_SYSTEM']->am->getAsset($settings['local_cert']);
			if(empty($file) || !method_exists($file, 'getExistingFile')) {
				if (!is_null($form)) $form->submission_errors = array_merge($form->submission_errors,  Array(translate('no_cert')));
				return FALSE;
			}
			$file_info = $file->getExistingFile();
			$options['local_cert']  = $file_info['path'];
		}

		if(isset($settings['passphrase'])) {
			$options['passphrase'] = $settings['passphrase'];
		}

		if (isset($settings['run_test']) && $settings['run_test']) {
			$options['trace'] = 1;
		}

		if (isset($settings['mute_exceptions']) && $settings['mute_exceptions']) {
			$options['exceptions'] = 0;
		}

		$options['cache_wsdl'] = isset($settings['cache_wsdl']) ? $settings['cache_wsdl'] : WSDL_CACHE_BOTH;

		if(isset($settings['service_address']) && !empty($settings['service_address'])) {
		    $options['location'] = $settings['service_address'];
		}

		return $options;

	}//end _getSoapOptions()


	/**
	* Get sample SOAP request/response
	*
	* @return array
	* @access public
	*/
	public static function testSoapRequest($client, $settings, $function, $form=NULL)
	{
		// Work out the parameters of that soap function
		for($i = 0, $param = Array(); $i < count($settings['actions'][$settings['action']]); $i++) {
			if(isset($settings[$settings['action'].'parameters'.$i])) {

				$soap_message = $settings[$settings['action'].'parameters'.$i];
				$dom = new DOMDocument();
				// use DOMDocument::loadXML() to see if there is any entity  reference loop
				// which may exhaust us of the memory limit, if not then continue as normal
				if ($dom->loadXML($soap_message, LIBXML_NOENT)) {
					$object = @simplexml_import_dom($dom);
				} else {
					// don't worry about any warning, if we are here then PHP has already raised the warning for us
					return;
				}

				if (is_null($form)) {
					// Convert XML to array, replace file content keywords, xml is not friendly for binary data
					$object = self::toArray($object);
				} else {
					$object = SOAP_Form_Based_Common::toArray($object, $form);
				}

				$param[] = $object;
			}
		}

		// Get an array of parameters
		$eval_string = '';
		foreach ($param as $index => $object) {
			$eval_string .= $index == 0 ? '' : ', ';
			$eval_string .= '$param['.$index.']';
		}

		// Make SOAP call
		try {
			eval('$data = $client->$function('.$eval_string.');');
			$soap_request = $client->__getLastRequest();
			$soap_response = $client->__getLastResponse();

		} catch (Exception $e) {
			trigger_error($e->getMessage());
			return FALSE;
		}


		return Array('request' => $soap_request, 'response' => $soap_response);

	}//end testSoapRequest()


	/**
	* This function will save the file in data/private directory and give us a
	* ./?SQ_ACTION=get_soap_downloaded_file&soap_filename=XXXXXXX type url
	* as our keyword replacement to view this file
	*
	*
	* @param string	$content	the file content we get in the response of SOAP
	*
	* @return string
	* @access public
	*/
	public static function _save_file($content)
	{
		// get a random file name
		$file_name = random_password(10, TRUE, TRUE);
		$sessionid = session_id();

		if (!is_dir(SQ_DATA_PATH.'/private/soap_action_file_responses/'.$sessionid)) mkdir(SQ_DATA_PATH.'/private/soap_action_file_responses/'.$sessionid, 0777, TRUE);

		file_put_contents(SQ_DATA_PATH.'/private/soap_action_file_responses/'.$sessionid.'/'.$file_name, $content);

		// generate url for file and throw it back to be used in the
		// the keywords replacement array
		$file_url = './?SQ_ACTION=get_soap_downloaded_file&soap_filename='.$file_name;

		return $file_url;

	}//end _save_file()


	/**
	* Converts xml object to array
	* Replace file content keyword
	*
	* @return array
	* @access public
	*/
	public static function toArray($data)
	{
		if (is_object($data)) $data = get_object_vars($data);

		// Don't play with empty object
		if (empty($data) && (is_array($data) || is_object($data))) return '';

		if(is_array($data)) {
			$result = Array();
			foreach ($data as $index => $element) {
				$result[$index] = self::toArray($element);
			}
			return $result;
		} else {
			return $data;
		}

	}//end toArray


	/**
	* Returns the SOAP client object with the WSSE header set, if applicable
	*
	* @param array  $settings	SOAP request settings
	* @param array 	$options	SOAP options
	*
	* @return null|object
	* @access public
	*/
	public static function &getSoapClient($settings, $options)
	{
		$soap_client = NULL;

		$wsdl = array_get_index($settings, 'wsdl', '');
		if (empty($wsdl)) {
			return $soap_client;
		}

		// Don't pass these parameters if login is not required
		if (empty($options['login']) && empty($options['password'])) {
			unset($options['login']);
			unset($options['password']);
		}

		try {
			$soap_client = new SoapClient($wsdl, $options);

			$username = array_get_index($settings, 'login', '');
			$password = array_get_index($settings, 'password', '');
			$auth_type = array_get_index($settings, 'auth_type', 'HTTP_BASIC');

			// Add authentication header, based on the settings
			switch($auth_type) {
				case 'JSESSIONID':
					$login_url = array_get_index($settings, 'login_url', '');
					 if (!self::setJSessionCookie($soap_client, $username, $password, $login_url)) {
						 throw new Exception('Error occured when setting the JSESSIONID cookie');
					 }
				break;


				case 'WSS_USERNAME_TOKEN':
				case 'WSS_PASSWORD_DIGEST':
					self::setWsseSoapHeader($soap_client, $username, $password, $auth_type == 'WSS_PASSWORD_DIGEST');
				break;

				case 'MARKETO_SIGNATURE':
					self::setMarketoSignature($soap_client, $username, $password);
				break;

				case 'HTTP_BASIC':
				default:
					// No header required here
				break;

			}//end switch

		} catch (Exception $e) {
			trigger_error('Error occured when trying to access the SOAP service: '.htmlentities($e->getMessage()));
			return $null;
		}

		return $soap_client;

	}//end getSoapClient()


	/*
	* Add the Marketo signature (HMAC-SHA11 based) header
	*
	* @param object	$soap_client
	* @param string	$userid
	* @param string	$secret_key
	*
	* @return boolean
	* @access public
	*/
	public static function setMarketoSignature(&$soap_client, $userid, $secret_key)
	{
		if (is_null($soap_client)) {
			return FALSE;
		}

		$marketo_namespace = "http://www.marketo.com/mktows/";
		$timestamp = date(DATE_W3C);
		$signature = hash_hmac('sha1', $timestamp.$userid, $secret_key);

		$header_attrs = Array(
							'mktowsUserId' 		=> $userid,
							'requestSignature' 	=> $signature,
							'requestTimestamp'	=> $timestamp,
						);
		try {
			$auth_header = new SoapHeader($marketo_namespace, 'AuthenticationHeader', $header_attrs);
			$soap_client->__setSoapHeaders($auth_header);
		}catch (Exception $e) {
			trigger_error('Error occured when setting the Marketo signature: '.htmlentities($e->getMessage()));
			return FALSE;
		}

		return TRUE;

	}//end setMarketoSignature()


	/**
	* Add WSSE header to the SOAP client object
	*
	* @return boolean
	* @access public
	*/
	public static function setWsseSoapHeader(&$soap_client, $ws_username, $ws_password, $ws_passwordDigest = FALSE)
	{
		if (empty($ws_username) || empty($ws_password)) {
			return FALSE;
		}

		if ($soap_client instanceof SoapClient) {
			if ($ws_passwordDigest)
			{
                include_once SQ_FUDGE_PATH.'/general/security.inc';
				$nonce = security_rand();
				$timestamp = gmdate('Y-m-d\TH:i:s\Z');
				$packedNonce = pack('H*', $nonce);
				$packedTimestamp = pack('a*', $timestamp);
				$packedPassword = pack('a*',  $ws_password);
				$hash = sha1($packedNonce . $packedTimestamp . $packedPassword);
				$packedHash = pack('H*', $hash);
				$ws_header = '
						<wsse:Security SOAP-ENV:mustUnderstand="1" xmlns:wsse="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd">
							<wsse:UsernameToken>
								<wsse:Username>'.$ws_username.'</wsse:Username>
								<wsse:Password Type="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest">'.base64_encode($packedHash).'</wsse:Password>
								<wsse:Nonce EncodingType="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-soap-message-security-1.0#Base64Binary">'.base64_encode(pack('H*', $nonce)).'</wsse:Nonce>
								<wsu:Created xmlns:wsu="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd">' . $timestamp . '</wsu:Created>
							</wsse:UsernameToken>
						</wsse:Security>';
			}
			else {
				$ws_header = '
						<wsse:Security SOAP-ENV:mustUnderstand="1" xmlns:wsse="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd">
							<wsse:UsernameToken>
								<wsse:Username>'.$ws_username.'</wsse:Username>
								<wsse:Password>'.$ws_password.'</wsse:Password>
							</wsse:UsernameToken>
						</wsse:Security>';
			}
			try {
				$ws_header_var = new SoapVar($ws_header, XSD_ANYXML);
				$ws_header = new SoapHeader('http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd', "Security", $ws_header_var);
				$soap_client->__setSoapHeaders($ws_header);
			}catch (Exception $e) {
				trigger_error('Error occured when setting the WSSE SOAP header: '.htmlentities($e->getMessage()));
				return FALSE;
			}

			return TRUE;
		}

		return FALSE;

	}//end addUsernameTokenHeader()


	/**
	* Set JSESSIONID cookie to the SOAP client
	*
	* @param object	$soap_client
	* @param string	$username
	* @param string $password
	* @param string $login_url
	*
	* @return boolean
	* @access public
	*/
	public static function setJSessionCookie(&$soap_client, $username, $password, $login_url)
	{
		if(empty($username) || empty($password) || empty($login_url))
		    return TRUE;

		if (!($soap_client instanceof SoapClient)) {
			trigger_error('First parameter supplied is not a SOAP client object');
			return FALSE;
		}

		// First obtain the the JSESSIONID
		$jsessionid = self::getJSessionId($username, $password, $login_url);
		if (empty($jsessionid)) {
			trigger_error('JSESSIONID not found in the response: '.htmlentities($result));
			return FALSE;
		}

		// Set the the JSESSIONID cookie
		try {
			$soap_client->__setCookie("JSESSIONID", $jsessionid);
		}catch (Exception $e) {
			trigger_error('Error occured when setting the JSESSIONID cookie: '.htmlentities($e->getMessage()));
			return FALSE;
		}

		return TRUE;

	}//end setJSessionCookie()


	/**
	* Returns JSESSIONID from the given CRM login
	*
	* @param $username
	* @param $password
	* @param $login_url
	*
	* @return boolean|string
	* @access public
	*/
	function getJSessionId($username, $password, $login_url)
	{
		$url_parts = parse_url($login_url);
		$login_page = isset($url_parts['host']) ? $url_parts['host'] : '';
		if (empty($username) || empty($password) || empty($login_page)) {
			trigger_error('Cannot obtain JSESSIONID, required parameters not set login');
			return FALSE;
		}

		// Login and obtain the JSESSIONID
		$headers = Array(
						'GET '.$login_page.' HTTP/1.0',
						'UserName: '.$username,
						'Password: '.$password,
					);
		$curl = curl_init($login_url);
		curl_setopt($curl, CURLOPT_RETURNTRANSFER, TRUE);
		curl_setopt($curl, CURLOPT_HTTPHEADER, $headers);
		curl_setopt($curl, CURLOPT_SSL_VERIFYPEER, FALSE);
		curl_setopt($curl, CURLOPT_SSL_VERIFYHOST, FALSE);
		curl_setopt($curl, CURLOPT_HEADER, TRUE);
		$result = curl_exec($curl);
		if (!$result) {
			trigger_error('Error occured when login to '.$login_url.': '.curl_error($curl));
			return FALSE;
		}

		// Extract the JSESSIONID from the response
		preg_match('|Set\-Cookie: JSESSIONID=(.+?);|', $result, $match);
		$jsessionid = isset($match[1]) ? $match[1] : '';

		return $jsessionid;

	}//end getJSessionId()


	/**
	* Returns available service address from WSDL file. Only picks SOAP 1.1 standard one.
	* @param $wsdl wsdl url
	* @param $username
	* @param $password
	*
	* @return array
	* @access public
	*/
	function getAvailableServiceAddress($wsdl, $username, $password)
	{
		if(empty($wsdl)) return Array();

		$address_array = Array();
		$options = array(
			'FOLLOWLOCATION' => 1,
			'RETURNTRANSFER' => 1,
			'HTTPAUTH'          => CURLAUTH_BASIC,
			'TIMEOUT'        => 3,
			'SSL_VERIFYPEER' => 0,
		   );
		if(!empty($username)) {
		    $options['USERPWD'] = $username.":".$password;
		}
		$details  = fetch_url($wsdl, $options, array(), FALSE);
		$contents = $details['response'];
		if (strpos($contents, '?xml') !== FALSE) {
		    $xml = new SimpleXMLElement($contents);
		    $xml->registerXPathNamespace('wsdl', 'http://schemas.xmlsoap.org/wsdl/');
		    $xml->registerXPathNamespace('soap', 'http://schemas.xmlsoap.org/wsdl/soap/');
		    $results = $xml->xpath('/wsdl:definitions/wsdl:service/wsdl:port/soap:address');
		    foreach($results as $node) {
			foreach($node->attributes() as $key => $value) {
			    if($key === 'location') {
				$address = (string) $value;
				$address_array[$address] = $address;
			    }
			}
		    }
		}

		return $address_array;

	}//end getAvailableServiceAddress()



	/**
	* Converts an object or an array structure into an xml string
	*
	* Example:
	*       Array(
	*           "Profile" => Array(
	*               "Name"  => "Maxx",
	*               "Age"   => "22",
	*               ),
	*           )
	* will be converted into string:
	*   "
	*   <Profile>
	*       <Name>Maxx</Name>
	*       <Age>22</Age>
	*   </Profile>
	*   "
	*
	* @param Array $str
	* @param String $parent_element
	*
	* @return string
	* @access public
	*/
	function arrayToXml($str, $parent_element='')
	{
		if (is_object($str)) {
			$str = get_object_vars($str);
		}
		if (!is_array($str)) {
 			return '<![CDATA['.$str.']]>';
		}

		$xml = '';
		foreach($str as $key => $value) {
			// apply xml element naming rules
			$key = preg_replace('/\s+/', '', $key);
			if ((empty($key) && $key != 0) || preg_match('/^xml$/i', $key)) continue;

			if (!empty($parent_element) && !preg_match('/^\d+$/', $parent_element)) {
				$xml .= "\n".'<'.$parent_element.'>'.self::arrayToXml($value, $key).'</'.$parent_element.'>';
			} else {
				$xml .= self::arrayToXml($value, $key);
			}
		}//end foreach

		return $xml;

	}//end arrayToXml()


	// Auth type fields paint/process functions //////////////

	/**
	* Paint the additional fields for the selected authentication type
	*
	* @param string $prefix
	* @param array  $settings
	* @param object $o
	* @param object $write_access
	* @param object $paint_o
	* @param boolean $array_fields
	*
	* @return void
	* @access public
	*/
	public static function paintSoapAuthenticationFields($prefix, $settings, Backend_Outputter $o, $write_access=FALSE, $paint_o=FALSE, $array_fields=FALSE)
	{
		$selected_auth_type = array_get_index($settings, 'auth_type', 'HTTP_BASIC');
		$auth_paint_fn = 'paint'.str_replace(' ', '', ucwords(str_replace('_', ' ', strtolower($selected_auth_type)))).'Fields';

		if (method_exists(get_class(), $auth_paint_fn)) {
			self::$auth_paint_fn($prefix, $settings, $o, $write_access, $paint_o, $array_fields);
		} else {
			self::paintUsernamePasswordFields($prefix, $settings, $o, $write_access, $paint_o, $array_fields);
		}

	}//end paintSoapAuthenticationFields()


	/**
	* Paint the additional fields for the selected authentication type
	*
	* @param string|array 	$prefix
	* @param array  		$settings
	* @param object 		$o
	*
	* @return void
	* @access public
	*/
	public static function processSoapAuthenticationFields($prefix, &$settings, Backend_Outputter $o)
	{
		$selected_auth_type = array_get_index($settings, 'auth_type', 'HTTP_BASIC');
		$auth_process_fn = 'process'.str_replace(' ', '', ucwords(str_replace('_', ' ', strtolower($selected_auth_type)))).'Fields';

		if (method_exists(get_class(), $auth_process_fn)) {
			$success = self::$auth_process_fn($prefix, $settings, $o);
		} else {
			$success = self::processUsernamePasswordFields($prefix, $settings, $o);
		}

		return $success;

	}//end processSoapAuthenticationFields()


	/*
	* Paint JSESSIONID auth type fields
	*
	* @param string $prefix
	* @param array  $settings
	* @param object $o
	* @param object $write_access
	* @param object $paint_o
	* @param boolean $array_fields
	*
	* @return void
	* @access public
	*/
	public static function paintJsessionidFields($prefix, $settings, Backend_Outputter $o, $write_access=FALSE, $paint_o=FALSE, $array_fields=FALSE)
	{
		$selected_auth_type = array_get_index($settings, 'auth_type', 'HTTP_BASIC');
		$login_url_field_name = $array_fields ? $prefix.'[login_url]' : $prefix.'_login_url';
		$test_auth_field_name = $array_fields ? $prefix.'[test_auth]' : $prefix.'_test_auth';

		if ($paint_o) {
			$o->openSection();
			$o->openField('Auth URL');
				if ($write_access) {
					text_box($login_url_field_name, array_get_index($settings, 'login_url', ''), 100);
					check_box($test_auth_field_name, '1', FALSE);
					echo '&nbsp;Test Authentication';
				} else {
					echo array_get_index($settings, 'login_url', '');
				}

				if (isset($settings['crm_auth_success']) && is_bool($settings['crm_auth_success'])) {
					if ($settings['crm_auth_success'] === TRUE) {
						echo '<p><span style="color: green"><b>Authentication successful</b></span></p>';
					} else {
						echo '<p><span style="color: red"><b>Authentication Failed</b></span></p>';
					}
				}
			$o->closeField();
			self::paintUsernamePasswordFields($prefix, $settings, $o, $write_access, $paint_o, $array_fields, '', '', FALSE);

			$o->closeSection();

		} else {
			?>
			<tr>
				<th>Auth URL</th>
				<td>
					<?php text_box($login_url_field_name, array_get_index($settings, 'login_url', ''), 100); ?>
					<?php check_box($test_auth_field_name, '1', FALSE); ?> Test Authentication

					<?php
						if (isset($settings['crm_auth_success']) && is_bool($settings['crm_auth_success'])) {
							if ($settings['crm_auth_success'] === TRUE) {
								echo '<p><span style="color: green"><b>Authentication successful</b></span></p>';
							} else {
								echo '<p><span style="color: red"><b>Authentication Failed</b></span></p>';
							}
						}
					?>
				</td>
			</tr>
			<?php

			self::paintUsernamePasswordFields($prefix, $settings, $o, $write_access, $paint_o, $array_fields);
		}

	}//end paintJsessionidFields()


	/*
	* Process JSESSIONID auth type fields
	*
	* @param string|array 	$prefix
	* @param array  		$settings
	* @param object 		$o
	*
	* @return boolean
	* @access public
	*/
	public static function processJsessionidFields($prefix, &$settings, Backend_Outputter $o)
	{
		// If array, the POST data has been already processed in the var
		if (is_array($prefix)) {
			$settings['login_url'] = array_get_index($prefix, 'login_url', array_get_index($settings, 'login_url', NULL));
			$settings['test_crm_auth'] = array_get_index($prefix, 'test_auth', NULL);
		} else {
			$settings['login_url'] = array_get_index($_POST, $prefix.'_login_url', array_get_index($settings, 'login_url', NULL));
			$settings['test_crm_auth'] = array_get_index($_POST, $prefix.'_test_auth', NULL);
		}

		unset($settings['crm_auth_success']);
		if ($settings['auth_type'] == 'JSESSIONID' && $settings['test_crm_auth']) {
			$crm_auth_test = FALSE;
			if (SOAP_Common::getJSessionId($settings['login'], $settings['password'], $settings['login_url'])) {
				$crm_auth_test = TRUE;
			}
			$settings['crm_auth_success'] = $crm_auth_test;
		}

		return self::processUsernamePasswordFields($prefix, $settings, $o);;

	}//end processJsessionidFields()


	/*
	* Paint Marketo authentication type fields
	*
	* @param string $prefix
	* @param array  $settings
	* @param object $o
	* @param object $write_access
	* @param object $paint_o
	* @param boolean $array_fields
	*
	* @return void
	* @access public
	*/
	public static function paintMarketoSignatureFields($prefix, $settings, Backend_Outputter $o, $write_access=FALSE, $paint_o=FALSE, $array_fields=FALSE)
	{
		// Basically same as generic user/pass fields, except the fields name are different
		self::paintUsernamePasswordFields($prefix, $settings, $o, $write_access, $paint_o, $array_fields, 'Access Key', 'Secret Key');

	}//end paintMarketoSignatureFields()


	/*
	* Paint username/password interface
	*
	* @param string  $prefix
	* @param array   $settings
	* @param object  $o
	* @param object  $write_access
	* @param object  $paint_o
	* @param boolean $array_fields
	* @param string  $username_label
	* @param string  $password_label
	* @param boolean $new_section
	*
	* @return void
	* @access public
	*
	*/
	public static function paintUsernamePasswordFields($prefix, $settings, Backend_Outputter $o, $write_access=FALSE, $paint_o=FALSE, $array_fields=FALSE, $username_label='', $password_label='', $new_section=TRUE)
	{
		if (empty($username_label)) {
			$username_label = translate('username');
		}
		if (empty($password_label)) {
			$password_label = translate('password');
		}

		$login_field_name = $array_fields ? $prefix.'[login]' : $prefix.'_login';
		$password_field_name = $array_fields ? $prefix.'[password]' : $prefix.'_password';

		// Authentication note locale string
		$auth_note_locale_string = 'soap_api_authentication_'.strtolower(array_get_index($settings, 'auth_type', 'HTTP_BASIC'));

		if ($paint_o) {
			$new_section ? $o->openSection() : '';
				$o->openField($username_label);
				if ($write_access) {
					text_box($login_field_name, array_get_index($settings, 'login', ''), 35);
				} else {
					echo array_get_index($settings, 'login', '');
				}
				$o->closeField();

				$o->openField($password_label);
				if ($write_access) {
					password_box($password_field_name, array_get_index($settings, 'password', ''), 35);
				} else {
					echo translate('password_not_shown');
				}
				$o->closeField();

				$o->sectionNote(translate($auth_note_locale_string));
			$new_section ? $o->closeSection() : '';

		} else {
			?>
				<tr>
					<th><?php echo $username_label; ?></th>
					<td><?php
						if ($write_access) {
							text_box($login_field_name, array_get_index($settings, 'login', ''), 35); 
						} else {
							echo array_get_index($settings, 'login', '');
						}
					?>
					</td>
				</tr>
				<tr>
					<th><?php echo $password_label; ?></th>
					<td><?php
						if ($write_access) {
							password_box($password_field_name, array_get_index($settings, 'password', ''), 35);
						} else {
							echo translate('password_not_shown');
						}
					?>
					</td>
				</tr>
				<tr>
					<td colspan="2"><?php echo translate($auth_note_locale_string); ?>.</td>
				</tr>
			<?php
		}

	}//end paintUsernamePasswordFields()


	/*
	* Process username/password interface
	*
	* @param string|array 	$prefix
	* @param array  		$settings
	* @param object 		$o
	*
	* @return void
	* @access public
	*
	*/
	public static function processUsernamePasswordFields($prefix, &$settings, Backend_Outputter $o)
	{
		// If array, the POST data has been already processed in the var
		if (is_array($prefix)) {
			$settings['login'] = array_get_index($prefix, 'login', NULL);
			$settings['password'] = array_get_index($prefix, 'password', NULL);
		} else {
			$settings['login'] = array_get_index($_POST, $prefix.'_login', NULL);
			$settings['password'] = array_get_index($_POST, $prefix.'_password', NULL);
		}

		return TRUE;

	}//end processUsernamePasswordFields()


}//end class

?>
