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
* $Id: soap_common.inc,v 1.10.2.3 2013/09/10 05:14:59 ewang Exp $
*
*/


require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once 'soap_form_based_common.inc';


// WSDL sometime defines a complex type in a loop, we need to get out of this loop if it repeats too many times while parsing
define('MAX_COMPLEX_TYPE_PASRSE_LOOP', 3);

/**
* SOAP_Common
* This class will have all the common functions used for SOAP trigger and form actions
*
* @author  Ash Karelia <akarelia@squiz.com.au>
* @version $Revision: 1.10.2.3 $
* @package MySource_Matrix_Packages
* @subpackage web_service
*/


class SOAP_Common
{


	private static $recent_parsed_type = Array();

	public static $wsdl_caching_opts = Array(
											WSDL_CACHE_NONE => 'Never cache',
											WSDL_CACHE_DISK => 'Cache to disk',
											WSDL_CACHE_MEMORY => 'Cache to memory',
											WSDL_CACHE_BOTH => 'Cache to both disk and memory',
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

		$options['cache_wsdl'] = isset($settings['cache_wsdl']) ? $settings['cache_wsdl'] : WSDL_CACHE_NONE;
		
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
				$object = @simplexml_load_string($soap_message);
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
	* @param string $wsdl		WSDL address
	* @param array 	$options	SOAP options
	* @param array 	$ws_options	WEES parameters (username, password)
	*
	* @return null|object
	* @access public
	*/
	public static function &getSoapClient($wsdl, $options, $ws_options=Array())
	{
		$null = NULL;

		try {
			$soap_client = new SoapClient($wsdl, $options);
			if (isset($ws_options['username']) && isset($ws_options['password']) && isset($ws_options['passwordDigest'])) {				
				self::setWsseSoapHeader($soap_client, $ws_options['username'], $ws_options['password'], $ws_options['passwordDigest']);
			}
		} catch (Exception $e) {
			trigger_error('Error occured when trying to access the SOAP service: '.htmlentities($e->getMessage()));
			return $null;
		}

		return $soap_client;

	}//end getSoapClient()


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
			
				$nonce = mt_rand();
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
	* @parm Array $str
	*
	* @return string
	* @access public
	*/
	function arrayToXml($str)
	{
		if (is_object($str)) {
			$str = get_object_vars($str);
		}
		if (!is_array($str)) {
			return '<![CDATA['.$str.']]>';
		}

		$xml = '';
		foreach($str as $key => $value) {

			if (is_int($key)) {
				$xml .= self::arrayToXml($value);
			} else {
				// apply xml element naming rules
				$key = preg_replace('/\s+/', '', $key);
				if (empty($key) || preg_match('/^xml$/i', $key)) continue;

				$xml .= "\n".'<'.$key.'>'.self::arrayToXml($value).'</'.preg_replace('|\s.*$|', '', $key).'>';
			}
		}//end foreach

		return $xml;

	}//end arrayToXml()


}//end class

?>
