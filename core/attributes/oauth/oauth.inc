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
* $Id: oauth.inc,v 1.12 2013/10/11 07:20:30 ewang Exp $
*
*/

require_once SQ_ATTRIBUTES_PATH.'/serialise/serialise.inc';
require_once SQ_ATTRIBUTES_PATH.'/option_list/option_list.inc';
require_once SQ_ATTRIBUTES_PATH.'/http_request/http_request.inc';

/**
* Asset_Attribute_OAuth
*
* <!-- Sample Edit Interface XML Node -->
* 	<oauth />
*
* @author  Maki Maruyama <mmaruyama@squiz.co.uk>
* @version $ $
* @package MySource_Matrix
* @subpackage attributes
*/
class Asset_Attribute_OAuth extends Asset_Attribute_HTTP_Request
{
	private $_methods = Array(
		'GET'    => 'GET',
		'POST'   => 'POST',
	);
	private $_signature_methods = Array(
		'HMAC-SHA1' => 'HMAC-SHA1',
		//'RSA-SHA1' => 'RSA-SHA1',
		'PLAINTEXT' => 'PLAINTEXT',
	);
	private $_type = Array(
		'THREE_LEGGED' => '3 legged',
		'TWO_LEGGED' => '2 legged',
	);

	private $_header_params = Array();
	private $_errors = Array();

	/**
	 * Constructor
	 *
	 * @param string $assetid    the asset id to be loaded
	 *
	 */
	function __construct($attribute = 0, $value = NULL)
	{
		parent::__construct($attribute, $value);

	}//end constructor


	/**
	* Print the interface for filling in a value
	*
	* @param string		$prefix		prefix for the form elements
	* @param boolean	$read_only	are we just printing the value
	*
	* @return void
	* @access public
	*/
	public function paint($prefix, $read_only = FALSE)
	{
		$prefix = str_replace(':', '_', $prefix);
		$current_value = @unserialize($this->value);

		// Set default values.
		if (!is_array($current_value)) $current_value = Array();

		if (!isset($current_value['consumer_key'])) $current_value['consumer_key'] = '';
		if (!isset($current_value['consumer_secret'])) $current_value['consumer_secret'] = FALSE;
		if (!isset($current_value['signature_method'])) $current_value['signature_method'] = 'HMAC-SHA1';
		if (!isset($current_value['method'])) $current_value['method'] = 'POST';

		if (!isset($current_value['request_token_url'])) $current_value['request_token_url'] = '';
		if (!isset($current_value['authorization_url'])) $current_value['authorization_url'] = '';
		if (!isset($current_value['access_token_url'])) $current_value['access_token_url'] = '';
		if (!isset($current_value['callback_url'])) $current_value['callback_url'] = '';

		if (!isset($current_value['timeout'])) $current_value['timeout'] = 10;
		if (!isset($current_value['request_headers'])) $current_value['request_headers'] = Array();
		if (!isset($current_value['request_body'])) $current_value['request_body'] = '';
		?>

		<table border="0" class="sq-backend-table">
			<tr>
                <th width="20%"><?php echo translate('oauth_consumer_key'); ?></th>
                <td>
                    <?php
                        if ($read_only) {
                            echo htmlspecialchars($current_value['consumer_key']);
                        } else {
                            text_box($prefix.'_consumer_key', $current_value['consumer_key'], 80);
                        }
                    ?>
                </td>
            </tr>
			<tr>
                <th width="20%"><?php echo translate('oauth_consumer_secret'); ?></th>
                <td>
                    <?php
                        if ($read_only) {
                            echo htmlspecialchars($current_value['consumer_secret']);
                        } else {
                            text_box($prefix.'_consumer_secret', $current_value['consumer_secret'], 80);
                        }
                    ?>
                </td>
            </tr>
			<?php if (!isset($current_value['type']) || $current_value['type'] == 'THREE_LEGGED') {?>
			<tr>
				<th width="20%"><?php echo translate('http_request_method'); ?></th>
				<td>
					<?php
						if ($read_only) {
							echo htmlspecialchars($current_value['method']);
						} else {
							combo_box($prefix.'_method', $this->_methods, FALSE, $current_value['method']);
						}
					?>
				</td>
			</tr>
			<tr>
				<th><?php echo translate('oauth_request_token_url'); ?></th>
				<td>
					<?php
						if ($read_only) {
	                    	echo htmlspecialchars($current_value['request_token_url']);
						} else {
							text_box($prefix.'_request_token_url', $current_value['request_token_url'], 80);
						}
					?>
				</td>
			</tr>
			<tr>
                <th><?php echo translate('oauth_authorization_url'); ?></th>
                <td>
                    <?php
                        if ($read_only) {
                            echo htmlspecialchars($current_value['authorization_url']);
                        } else {
                            text_box($prefix.'_authorization_url', $current_value['authorization_url'], 80);
                        }
                    ?>
                </td>
			</tr>
			<tr>
                <th><?php echo translate('oauth_access_token_url'); ?></th>
                <td>
                    <?php
                        if ($read_only) {
                            echo htmlspecialchars($current_value['access_token_url']);
                        } else {
                            text_box($prefix.'_access_token_url', $current_value['access_token_url'], 80);
                        }
                    ?>
                </td>
            </tr>
			<tr>
                <th><?php echo translate('oauth_callback_url'); ?></th>
                <td>
                    <?php
                        if ($read_only) {
                            echo htmlspecialchars($current_value['callback_url']);
                        } else {
							text_box($prefix.'_callback_url', $current_value['callback_url'], 80);
                        }
                    ?>
                </td>
			</tr>
			<?php }?>
			<tr>
                <th><?php echo translate('oauth_signature_method'); ?></th>
                <td>
                    <?php
                        if ($read_only) {
                            echo htmlspecialchars($this->_signature_methods[$current_value['signature_method']]);
                        } else {
							combo_box($prefix.'_signature_method', $this->_signature_methods, FALSE, $current_value['signature_method']);
						}
					?>
				</td>
			</tr>
			<?php if (!isset($current_value['type']) || $current_value['type'] == 'THREE_LEGGED') {?>
			<tr>
				<th><?php echo translate('http_request_request_headers'); ?></th>
				<td>
					<?php
					$hl = new Asset_Attribute_Option_List();
					$hl->value = implode($hl->delimiter, $current_value['request_headers']);
					$hl->setEditParam('width', '60');
					$hl->paint($prefix.'_request_headers', $read_only);
					?>
				</td>
			</tr>
			<?php
			}
			if (($current_value['method'] == 'POST') || ($current_value['method'] == 'PUT')) {
			?>
			<tr>
				<th><?php echo translate('http_request_request_body'); ?></th>
				<td>
					<?php
						if ($read_only) {
							echo htmlspecialchars($current_value['request_body']);
						} else {
							text_area($prefix.'_request_body', $current_value['request_body'], 70, 20, 0, 'style="font-family: monospace;"');
						}
					?>
				</td>
			</tr>
            <tr>
                <th><?php echo translate('http_request_timeout'); ?></th>
                <td>
                    <?php
                        if ($read_only) {
                            echo htmlspecialchars($current_value['timeout']);
                        } else {
                            text_box($prefix.'_timeout', $current_value['timeout'], 4);
                        }
                    ?>
                </td>
            </tr>
			<?php
			} else {
				hidden_field($prefix.'_request_body', $current_value['request_body']);
			}
			?>
			</table>
			<?php

	}//end paint()


	/**
	* Process the interface for filling in a value
	*
	* @param string	$prefix	prefix for the form elements
	*
	* @return void
	* @access public
	*/
	public function process($prefix)
	{
		$prefix = str_replace(':', '_', $prefix);

		$value = Array();

		if ($this->getType() != 'TWO_LEGGED' && !isset($_REQUEST[$prefix.'_method'])) return FALSE;

		// Assign values
		$value['consumer_key'] 		= (isset($_REQUEST[$prefix.'_consumer_key'])) ? trim($_REQUEST[$prefix.'_consumer_key']) : '';
		$value['consumer_secret'] 	= (isset($_REQUEST[$prefix.'_consumer_secret'])) ? trim($_REQUEST[$prefix.'_consumer_secret']) : '';
		$value['signature_method']	= (isset($_REQUEST[$prefix.'_signature_method'])) ? trim($_REQUEST[$prefix.'_signature_method']) : '';
		$value['method'] 			= (isset($_REQUEST[$prefix.'_method'])) ? $_REQUEST[$prefix.'_method'] : '';

		$value['request_token_url'] = (isset($_REQUEST[$prefix.'_request_token_url'])) ? $_REQUEST[$prefix.'_request_token_url'] : '';
		$value['authorization_url'] = (isset($_REQUEST[$prefix.'_authorization_url'])) ? $_REQUEST[$prefix.'_authorization_url'] : '';
		$value['access_token_url'] 	= (isset($_REQUEST[$prefix.'_access_token_url'])) ? $_REQUEST[$prefix.'_access_token_url'] : '';
		$value['callback_url'] 		= (isset($_REQUEST[$prefix.'_callback_url'])) ? $_REQUEST[$prefix.'_callback_url'] : '';

		$value['timeout'] 				= (isset($_REQUEST[$prefix.'_timeout'])) ? $_REQUEST[$prefix.'_timeout'] : 0;
		$value['follow_redirect'] 		= FALSE;
		$value['cache_options'] 		= 'NEVER';
		$value['cache_post_requests'] 	= FALSE;
		$value['default_cache_expiry']	= 60;
		$value['request_body'] 			= (isset($_REQUEST[$prefix.'_request_body'])) ? trim($_REQUEST[$prefix.'_request_body']) : '';

		$hl = new Asset_Attribute_Option_List();
		$hl->process($prefix.'_request_headers');
		$value['request_headers'] = trim($hl->value) ? explode($hl->delimiter, $hl->value) : NULL;

		$value['type']			  = $this->getType();	// the type is controlled by calling asset(which sets the type as default), not the user

		$value['run_test'] = FALSE;

		$this->processed = $this->setValue($value);

	}//end process()


	/**
	* Encode the given input into RFC3986
	*
	* @param  Mixed   $input		Array or String to encode
	* @param  Boolean $url_encode	whether to url_encode the $input
	*
	* @return	string
	* @access	private
	*/
	private function _encodeParam($input, $url_encode = TRUE)
	{
		// if the passed value is an array, encode both keys and values
		if (is_array($input)) {
			$encoded_array = Array();
			foreach ($input as $key => $val) {
				$encoded_array[$this->_encodeParam($key)] = $this->_encodeParam($val);
			}
			return $encoded_array;
		} else {
			$input = utf8_encode($input);
			if ($url_encode) {
				$input = str_replace('+', ' ', str_replace('%7E', '~', rawurlencode($input)));
			}

			return $input;
		}


	}//end _encodeParam()


	/**
	* Generate a Nonce value
	*
	*
	* @return string
	* @access private
	*/
	private function _generateNonce()
	{
        include_once SQ_FUDGE_PATH.'/general/security.inc';
		$mt   = microtime();
		$rand = security_rand();
		return md5($mt . $rand);

	}//end _generateNonce()


	/**
	* Generate a base string used for a signature
	*
	* @param String $request_method	GET/POST
	* 		 String $request_url	a url without parameters
	*        Array	$params			parameters required by oauth specs
	*
	* @return string
	* @access private
	*/
	private function _generateBaseString($request_method, $request_url, $params)
	{
		$base_string_params = Array();

		// construct parameters in name=value format
		foreach ($params as $param_key => $param_val) {
			$base_string_params[] = $param_key.'='.$param_val;
		}

		// sort the parameters
		sort($base_string_params);

		// and connect them all together with '&'
		$base_string = implode('&', $base_string_params);

		// add request method & request url in front and it's a base string for signature
		return $this->_encodeParam($request_method).'&'.$this->_encodeParam($request_url).'&'.$this->_encodeParam($base_string);

	}//end _generateBaseString()


	/**
	* Generate a signature
	*
	* @param String $key			a key string used for signing
	*        String $base_string	a base_string
	*        String $method			a signing method to use
	*
	* @return string
	* @access private
	*/
	private function _generateSignature($key, $base_string, $method)
	{
		switch ($method) {
			case 'HMAC-SHA1' :
				return $this->_encodeParam(base64_encode(hash_hmac("sha1", $base_string, $key, TRUE)));

			// TODO: support RSA-SHA1
			//case 'RSA-SHA1':

			case 'PLAINTEXT':
			default:
				return $this->_encodeParam($base_string);
		}

	}//end _generateSignature()


	/**
	* Generate authorization header
	*
	* @param Array $params	parameters required by oauth
	*
	* @return String
	* @access private
	*/
	private function _generateAuthorizationHeader($params)
	{
		$headers = Array();
		foreach ($params as $param_key => $param_val) {
			$headers[] = $param_key.'="'.$param_val.'"';
		}
		return 'Authorization: OAuth '.implode(',', $headers);

	}//end _generateAuthorizationHeader()


	/**
	* get parameters specified in url
	*
	* @param String  $url			an url to take parameters from
	* @param Boolean $url_encode	whether to url_encode the query part of the $url
	*
	* @return Array
	* @access private
	*/
	private function _getQueryParameterArray($url, $url_encode = TRUE)
	{
		if (empty($url)) return NULL;

		$query_parameters = Array();

		// if the url has the parameters part, return the parameters as an array
		$url_param = explode('?', $url);
		if (isset($url_param[1])) {
			$params = explode('&', $url_param[1]);
			foreach ($params as $param) {
				$param_val = explode('=', $param);
				// the url might already be url encoded - decode it to avoid double encoding later on
				// so that it'll work whehter the url has been encoded or not
				if($url_encode) {
				    $param_val[0] = urldecode($param_val[0]);
				    $param_val[1] = urldecode($param_val[1]);
				}
				$query_parameters[$this->_encodeParam($param_val[0], $url_encode)] = $this->_encodeParam($param_val[1], $url_encode);
			}
		}

		return $query_parameters;

	}//end _getQueryParameterArray()


	/**
	* set request token and secret
	*
	* @param String $token_info	a string contains token and token secret
	*
	* @return Array
	* @access private
	*/
	private function _getTokenInfoArray($token_info_string)
	{
		$token_info = Array();

		// take out each item in token info - typically token and token secret, and possibly callback confirmed
		$token_strings = explode('&', $token_info_string);

		// if there is no more than one item, then it's not proper token info
		if (count($token_strings) <= 1) return $token_info;

		// put each info item into an key-value Array
		foreach ($token_strings as $token_string) {
			// each token info is in the 'name=value' format
			$token_item = explode('=', $token_string);
			if (count($token_item) == 2) {
				$token_info[$token_item[0]] = $token_item[1];
			}
		}

		return $token_info;

	}//end _getTokenInfoArray()


	/**
	* get request token from an oauth service provider
	*
	* @param String $consumer_key		consumer key registered with the service provider
	*        String $consumer_secret	consumer secret registered with the service provider
	*
	* @return Array
	* @access public
	*/
	public function getRequestToken($consumer_key='', $consumer_secret='')
	{
		$this->_config = @unserialize($this->value);
		if (empty($this->_config)) return;

		if (empty($this->_config['request_token_url'])) return;

		// Build the result data structure. This will be returned eventually, and is also used to form the cache keys.
		$this->_res = Array(
						'request' => Array(
							'method' => $this->_config['method'],
							'headers' => count($this->_config['request_headers']) ? $this->_config['request_headers'] : Array(),
							'body' => $this->_config['request_body'],
							'urls' => Array($this->_config['request_token_url']),
							'auth' => Array(
										'type' => 'NONE',
									),
							'consumer_key' => $this->_config['consumer_key'],
							'consumer_secret' => $this->_config['consumer_secret'],
							'callback_url' => $this->_config['callback_url'],
						),
						'responses' => Array(),
						'response'  => Array(),
					);

		// Replace keywords in each URL.
		foreach ($this->_res['request']['urls'] as &$url) {
			$this->_replaceKeywords($url);
		}

		// Replace keywords in each Header.
		for ($i = 0; $i < count($this->_res['request']['headers']); $i++) {
			$this->_replaceKeywords($this->_res['request']['headers'][$i]);

			// Discard badly formatted headers.
			if (preg_match('/(.+):(.+)/', $this->_res['request']['headers'][$i]) == 0) {
				unset($this->_res['request']['headers'][$i]);
			}
		}

		// Replace keywords in the body.
		$this->_replaceKeywords($this->_res['request']['body']);

		// replace keywords in oauth related values
		$this->_replaceKeywords($this->_res['request']['consumer_key']);
		$this->_replaceKeywords($this->_res['request']['consumer_secret']);
		$this->_replaceKeywords($this->_res['request']['callback_url']);

		// if the header parameters haven't been constructed yet,
		// construct the header parameters required for requesting request token
		if (empty($this->_header_params)) {
			$nonce = $this->_generateNonce();
			$timestamp = time();

			$this->_header_params = Array(
							'oauth_consumer_key' => $this->_res['request']['consumer_key'],
							'oauth_signature_method' => $this->_config['signature_method'],
							'oauth_timestamp' => $timestamp,
							'oauth_nonce' => $nonce,
							'oauth_version' => "1.0",
							'oauth_callback' => $this->_res['request']['callback_url'],
						);

			// gather parameters reuired for signature and encode them before signing the signature
			$all_params = $this->_header_params = $this->_encodeParam($this->_header_params);
			$query_params = $this->_getQueryParameterArray($this->_res['request']['urls'][0]);
			if (!empty($query_params)) {
				// if query parameters exist, they have to be included in the signature
				// but shouldn't be included in the header parameters
				$all_params = array_merge($all_params, $query_params);
			}

			// construct a base string for the signature
			// request url used for signature shouldn't contain request parameters
			$request_url = explode('?', $this->_res['request']['urls'][0]);
			$base_string = $this->_generateBaseString($this->_config['method'], $request_url[0], $all_params);

			// signature key is a combination of cosumer key and token secret
			// but we haven't got a token secret yet, so, the token secret is an empty string
			$token_secret = '';
			$signature_key = $this->_encodeParam($this->_res['request']['consumer_secret']).'&'.$this->_encodeParam($token_secret);

			// generate a signature and set is as a header parameter
			$this->_header_params['oauth_signature'] = $this->_generateSignature($signature_key, $base_string, $this->_config['signature_method']);
		}

		// generate a header string required for oauth authorization
		$header_string = $this->_generateAuthorizationHeader($this->_header_params);
		$this->_res['request']['headers'][] = $header_string;

		// Go!
		$this->_performRequests();

		// Set the first in responses to response
		$this->_res['response'] =& $this->_res['responses'][0];

		// if the response is received, the response should contain request token & request token secret
		// (and possibly callback_confirmed which is always true)
		// remember the token and secret and set secret in the session to make it available later for access token request
		// then clear the response and have the token authorized
		if (!empty($this->_res['response']['body'])) {
			$token_info = $this->_getTokenInfoArray($this->_res['response']['body']);

			// to move on to user authorization, token and token secret are required
			if (empty($token_info) || !isset($token_info['oauth_token']) || !isset($token_info['oauth_token_secret'])) {
				$this->_errors[] = translate('oauth_request_token_failed', $this->_res['request']['urls'][0], $this->_res['response']['info']['http_code'], $this->_res['response']['body']);
				return $this->_res;
			}

			// return the request token
			$this->_res['request_token'] = $token_info;
			return $this->_res;
		}

		// shouldn't reach here...but if it does, it means we didn't get a response body, so, something must have gone wrong...
		$this->_errors[] = translate('oauth_request_token_failed', $this->_res['request']['urls'][0], $this->_res['response']['info']['http_code'], $this->_res['response']['body']);

		return NULL;

	}//end getRequestToken()


	/**
	* get a request token authorized by service provider
	* redirecting user to the service provider's site
	*
	* @param String  $request_token	request token string
	*
	* @return null
	* @access public
	*/
	public function authorizeRedirect($request_token = Array())
	{
	    $this->_config = @unserialize($this->value);
	    if (empty($this->_config)) return;
	    if (empty($this->_config['authorization_url'])) return;
	    if(!isset($request_token['oauth_token'])) return;

	    $url = $this->_config['authorization_url'].'?oauth_token='.$request_token['oauth_token'];
	    do_redirect($url);
	}//end authorizeRedirect()

	/**
	* get a request token authorized by service provider
	*
	*
	* @return Array
	* @access public
	*/
	public function authorize($request_token = Array())
	{
		$this->_config = @unserialize($this->value);
		if (empty($this->_config)) return;

		if (empty($this->_config['authorization_url'])) return;

		// if a request token is not available, it cannot be authorized by the user
		if (empty($request_token) || !isset($request_token['oauth_token'])) {
			$auth_url = $this->_config['authorization_url'];
			$this->_replaceKeywords($auth_url);
			$this->_errors[] = translate('oauth_request_token_not_found', $auth_url);
			return NULL;
		}

		// Build the result data structure. This will be returned eventually, and is also used to form the cache keys.
		$this->_res = Array(
						'request' => Array(
							'method' => $this->_config['method'],
							'headers' => Array(),
							'body' => '',
							'urls' => Array($this->_config['authorization_url'].'?oauth_token='.$request_token['oauth_token']),
							'auth' => Array(
								'type' => 'NONE',
							),
						),
						'responses' => Array(),
						'response'  => Array(),
					);

		// Replace keywords in each URL.
        foreach ($this->_res['request']['urls'] as &$url) {
			$this->_replaceKeywords($url);
		}

		// Replace keywords in each Header.
		for ($i = 0; $i < count($this->_res['request']['headers']); $i++) {
			$this->_replaceKeywords($this->_res['request']['headers'][$i]);

			// Discard badly formatted headers.
			if (preg_match('/(.+):(.+)/', $this->_res['request']['headers'][$i]) == 0) {
				unset($this->_res['request']['headers'][$i]);
			}
		}

		// Replace keywords in the body.
		$this->_replaceKeywords($this->_res['request']['body']);

		// Go!
		$this->_performRequests();

		// Set the first in responses to response
		$this->_res['response'] =& $this->_res['responses'][0];

		// we need to rely on the http code they return in the response to check if the request went through
		// as the presentation of authorization/log in pages are up to the service provider...
		if ($this->_res['response']['info']['http_code'] >= 400) {
			$this->_errors[] = translate('oauth_authorization_request_failed', $this->_res['request']['urls'][0], $this->_res['response']['info']['http_code'], $this->_res['response']['body']);
			return $this->_res;
		}

		// _res now should contain the authorization/log in page of the service provider
		return $this->_res;

	}//end authorize()


	/**
	* get an access token from service provider
	*
	* @param String $authorized_token	token authorized by service provider
	*        String $token_secret		token secret provided by service provider
	*
	* @return Array
	* @access public
	*/
	public function getAccessToken($authorized_token, $verifier, $token_secret)
	{
		$this->_config = @unserialize($this->value);
        if (empty($this->_config)) return;

        // we need access token request url
        if (empty($this->_config['access_token_url'])) return;

		// we need authorized token, verifier and token secret returned from the service provider to carry on
		if (!isset($authorized_token) || !isset($verifier) || !isset($token_secret)) {
			$access_url = $this->_config['access_token_url'];
			$this->_replaceKeywords($access_url);
			$this->_errors[] = translate('oauth_access_token_missing_params', $access_url, $this->_res['response']['info']['http_code'], $this->_res['response']['body']);
			return NULL;
		}

		// Build the result data structure. This will be returned eventually, and is also used to form the cache keys.
		$this->_res = Array(
						'request' => Array(
							'method' => $this->_config['method'],
							'headers' => count($this->_config['request_headers']) ? $this->_config['request_headers'] : Array(),
                        	'body' => '',
							'urls' => Array($this->_config['access_token_url']),
							'auth' => Array(
								'type' => 'NONE',
							),
							'consumer_key' => $this->_config['consumer_key'],
							'consumer_secret' => $this->_config['consumer_secret'],
						),
						'responses' => Array(),
						'response'  => Array(),
					);

		// Replace keywords in each URL.
		foreach ($this->_res['request']['urls'] as &$url) {
			$this->_replaceKeywords($url);
		}

		// Replace keywords in each Header.
		for ($i = 0; $i < count($this->_res['request']['headers']); $i++) {
			$this->_replaceKeywords($this->_res['request']['headers'][$i]);

			// Discard badly formatted headers.
			if (preg_match('/(.+):(.+)/', $this->_res['request']['headers'][$i]) == 0) {
				unset($this->_res['request']['headers'][$i]);
			}
		}

		// Replace keywords in the body.
		$this->_replaceKeywords($this->_res['request']['body']);

		// replace keywords in oauth related values
		$this->_replaceKeywords($this->_res['request']['consumer_key']);
		$this->_replaceKeywords($this->_res['request']['consumer_secret']);

		// if the header parameters haven't been constructed yet,
		// construct the header parameters required for requesting access token
		if (empty($this->_header_params)) {
			$nonce = $this->_generateNonce();
			$timestamp = time();

			$this->_header_params = Array(
							'oauth_consumer_key' => $this->_res['request']['consumer_key'],
							'oauth_token' => $authorized_token,
							'oauth_signature_method' => $this->_config['signature_method'],
							'oauth_timestamp' => $timestamp,
							'oauth_nonce' => $nonce,
							'oauth_version' => "1.0",
							'oauth_verifier' => $verifier,
						);

			// gather parameters reuired for signature and encode them before signing the signature
			$all_params = $this->_header_params = $this->_encodeParam($this->_header_params);
			$query_params = $this->_getQueryParameterArray($this->_res['request']['urls'][0]);
			if (!empty($query_params)) {
				// if query parameters exist, they have to be included in the signature
				// but shouldn't be included in the header parameters
				$all_params = array_merge($all_params, $query_params);
			}

			// construct a base string for the signature
			// request url used for signature shouldn't contain request parameters
			$request_url = explode('?', $this->_res['request']['urls'][0]);
			$base_string = $this->_generateBaseString($this->_config['method'], $request_url[0], $all_params);

			// signature key is a combination of cosumer key and token secret (received when a request token was issued)
			$signature_key = $this->_encodeParam($this->_res['request']['consumer_secret']).'&'.$token_secret;

			// generate a signature and set is as a header parameter
			$this->_header_params['oauth_signature'] = $this->_generateSignature($signature_key, $base_string, $this->_config['signature_method']);
		}

		// generate a header string required for oauth authorization
		$header_string = $this->_generateAuthorizationHeader($this->_header_params);
		$this->_res['request']['headers'][] = $header_string;

		// Go!
		$this->_performRequests();

		// Set the first in responses to response
		$this->_res['response'] =& $this->_res['responses'][0];

		// if the response is received, the response should contain access token & access token secret
		// remember the token and secret in the session to make them available later to obtain user data
		// and set the session variable to indicate we have successfully exchaned the request(authorized) token
		// to an access token, then clear the response and request user data
		if (!empty($this->_res['response']['body'])) {
			$token_info = $this->_getTokenInfoArray($this->_res['response']['body']);

			if (empty($token_info) || !isset($token_info['oauth_token']) || !isset($token_info['oauth_token_secret'])) {
				$this->_errors[] = translate('oauth_access_token_failed', $this->_res['request']['urls'][0], $this->_res['response']['info']['http_code'], $this->_res['response']['body']);
				return $this->_res;
			}

			// return the access token
			$this->_res['access_token'] = $token_info;
			return $this->_res;
		}

		return $this->_res;

	}//end getAccessToken()


	/**
	 * Runs the remote requests.
	 *
	 * @param Boolean $url_encode	whether to url_encode the query part of the $url
	 *
	 * @access public
	 * @return void
	 */
	public function getUserDataAuthHeader($url='', $access_token='', $access_token_secret='', $url_encode = TRUE)
	{
		$this->_config = @unserialize($this->value);
		if (empty($this->_config)) return;

		// we need feed url, access token and access secret
		if (empty($url) || ($this->getType() == 'THREE_LEGGED' && (empty($access_token) || empty($access_token_secret)))) {
			$this->_errors[] = translate('oauth_user_data_missing_params', $url);
			return;
		}

		$nonce = $this->_generateNonce();
		$timestamp = time();

		// construct the header parameters required for requesting user data
		$this->_header_params = Array(
					'oauth_consumer_key' => $this->_config['consumer_key'],
					'oauth_signature_method' => $this->_config['signature_method'],
					'oauth_timestamp' => $timestamp,
					'oauth_nonce' => $nonce,
					'oauth_version' => "1.0",
					);

		// gather parameters reuired for signature and encode them before signing the signature
		// but don't encode the access token as it's already been encoded
		$all_params = $this->_header_params = $this->_encodeParam($this->_header_params);
		if ($this->getType() == 'THREE_LEGGED') $all_params['oauth_token'] = $this->_header_params['oauth_token'] = $access_token;
		$query_params = $this->_getQueryParameterArray($url, $url_encode);
		// include post body in the base string
		if($this->_config['method'] == 'POST' && !empty($this->_config['request_body'])) {
		    $post_params =  $this->_getQueryParameterArray('?'.$this->_config['request_body']);
		    $query_params = array_merge($query_params, $post_params);
		}
		if (!empty($query_params)) {
			// if query parameters exist, they have to be included in the signature
			// but shouldn't be included in the header parameters
			$all_params = array_merge($all_params, $query_params);
		}

		// construct a base string for the signature
		// request url used for signature shouldn't contain request parameters
		$request_url = explode('?', $url);
		$base_string = $this->_generateBaseString($this->_config['method'], $request_url[0], $all_params);

		// signature key is a combination of cosumer key and token secret (received when an access token was issued)
		$signature_key = $this->_encodeParam($this->_config['consumer_secret']).'&'.$access_token_secret;
		$this->_header_params['oauth_signature'] = $this->_generateSignature($signature_key, $base_string, $this->_config['signature_method']);

		// generate a header string required to get user data
		return $this->_generateAuthorizationHeader($this->_header_params);

	}//end getUserDataAuthHeader


	/**
	 * Get errors encountered during OAuth process
	 *
	 * @param	none
	 *
	 * @return	Array
	 * @access	public
	 */
	public function getErrors()
	{
		return $this->_errors;

	}//end getErrors()


	/**
	 * Get the type of this OAuth authentication - 2 legged or 3 legged
	 *
	 * @param	none
	 *
	 * @return	String
	 * @access	public
	 */
	public function getType()
	{
		$value = @unserialize($this->value);
		return isset($value['type']) ? $value['type'] : 'THREE_LEGGED';
	}


	/**
	 * Set a config value
	 *
	 * @param	String	$config_name	the name of the config to set
	 * @param	mixed	$config_value	the value of the config to set
	 *
	 * @access	public
	 */
	public function setConfig($config_name, $config_value)
	{
		$value = @unserialize($this->value);
		if ($value[$config_name] == $config_value) return;
		switch ($config_name) {
			// if the config to set is 'method' check the value is valid
			case 'method':
				if (isset($this->_methods[$config_value])) {
					$value['method'] = $config_value;
				}
				break;
			// otherwise set the config to this asset's attribute
			default:
				$value[$config_name] = $config_value;
		}

		$this->setValue($value);

	}

}
?>
