<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: page_remote_content.inc,v 1.3 2004/09/06 02:09:00 arailean Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_FUDGE_PATH.'/general/www.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once 'HTTP/Request.php';
require_once 'HTTP/Client/CookieManager.php';


/**
* Page_Remote_Content
*
* Purpose
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Remote_Content extends Page
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*/
	function Page_Remote_Content($assetid=0)
	{
		$this->Page($assetid);

	}//end constructor


	/**
	* Print the processed page.
	*
	* @access public
	* @return void
	*/
	function printBody()
	{
		// all we do is print, as the name suggests
		$result = $this->_process();
		// if we did not receive a reply
		if (!$result) {
			// requested file is binary
			if (array_get_index($this->_tmp,'binary',false)) {
				$type	= $this->_tmp['type'];
				$url	= $this->_tmp['url'];

				$replacements['remote_url'] = $url;

				$result = replace_keywords($this->attr('binary_message'), $replacements);

			} else if ($this->attr('display_error')) {
				// if error message is enabled, display it
				$result = $this->attr('error_message');
			} else {
				$result = '';
			}
		}

		echo $result;

	}//end printBody()


	/**
	* Process the remote content request and return the processed HTML
	*
	* Apart from converting actions and images to full URLs, this also converts
	* href's into full URLs.
	*
	* @access public
	* @return string
	*/
	function _process()
	{
		$remote_url = $this->attr('remote_url');

		$fetch_url	= array_get_index($_REQUEST, 'sq_remote_page_url');
		$action		= array_get_index($_REQUEST, 'sq_remote_page_action');

		if (!isset($fetch_url) || ($action != 'fetch_url')) {
			$fetch_url = $remote_url;
		}

		if (!$fetch_url) return;

		$fetch_url	= urldecode($fetch_url);

		$Fetch_URL	= &new Net_URL($fetch_url);

		// let's see if we're getting any variables in the get string
		// so we can pass them back to the  original page ..
		unset($_GET['sq_remote_page_action']);
		unset($_GET['sq_remote_page_url']);
		unset($_POST['sq_remote_page_action']);
		unset($_POST['sq_remote_page_url']);

		if (!empty($_GET)) {
			foreach ($_GET as $key => $data) {
				$Fetch_URL->addQueryString($key, $data);
			}
		}


		////// Remote Communication Section. Using HTTP_Request PEAR package

		// parameters for HTTP_Request. See Package for description.
		$request_parameters = Array(
									'timeout'			=> $this->attr('timeout'),
									'allowRedirects'	=> $this->attr('allow_redirect'),
									'maxRedirects'		=> 10,
									);

		// set HTTP authentication
		$user = $this->attr('username');
		if (!empty($user)){
			$request_parameters['user'] = $user;
			$request_parameters['pass'] = $this->attr('password');
		}

		$HTTP_Request = &new HTTP_Request($Fetch_URL->getURL(), $request_parameters);

		$HTTP_Request->addHeader('User-Agent', SQ_SYSTEM_LONG_NAME);

		if (!empty($_POST)) {
			$HTTP_Request->setMethod(HTTP_REQUEST_METHOD_POST);
			$HTTP_Request->addRawPostData(make_raw_post_data($_POST));
		} else {
			$HTTP_Request->setMethod(HTTP_REQUEST_METHOD_GET);
		}

		// cookies are handled by the cookie manager (PEAR)
		$Cookie_Manager = array_get_index($_SESSION, 'SQ_REMOTE_COOKIE_MANAGER');
		if (!empty($Cookie_Manager)) {
			$Cookie_Manager = unserialize($Cookie_Manager);
			$Cookie_Manager->passCookies($HTTP_Request);
		} else {
			$Cookie_Manager = new HTTP_Client_CookieManager();
		}

		// process the request
		$result = $HTTP_Request->sendRequest();

		// get new cookies and save cookie manager
		$Cookie_Manager->updateCookies($HTTP_Request);
		$_SESSION['SQ_REMOTE_COOKIE_MANAGER'] = serialize($Cookie_Manager);

		// check if there was an error of type 4xx or 5xx in the HTTP response
		$is_errror = preg_match("/^[45].*?/", $HTTP_Request->getResponseCode());

		// exit on error, checking for object because if it is true, than it is
		// a PEAR error object
		if (!$result || is_object($result) || $is_errror) {
			return false;
		}


		// ignore binary data
		// we need to make sure we are not trying to display anything
		// of type other than text
		$type = $HTTP_Request->getResponseHeader('content-type');
		if (strpos($type,'text')===false) {
			$this->_tmp['binary']	= true;
			$this->_tmp['type']		= $type;
			$this->_tmp['url']		= $Fetch_URL->getURL();

			unset($HTTP_Request);

			return false;
		}

		// everything is good. get the page body.
		$response_body	= $HTTP_Request->getResponseBody();

		// obtain new fetch url. important if there were redirects.
		$Fetch_URL = $HTTP_Request->_url;

		////// Now let's grab the required part of the remote page

		// prepare the tags
		$start_tag	= preg_quote($this->attr('start_tag'), '/');
		$end_tag	= preg_quote($this->attr('end_tag'), '/');

		// strip anything outside of the special tags
		// if the start or stop tags are specified, we scan the page for the necessary content
		// otherwise the page is good as it is, no scanning necessary
		if (isset($start_tag) || isset($end_tag)) {
			$pattern	= "/($start_tag.*$end_tag)/Us";

			// note that we can find more than one section identified by the tags
			preg_match_all($pattern, $response_body, $matches);
			if (count($matches[0])) {
				$response_body = '';
				foreach ($matches[0] as $match) $response_body .= $match;
			}
		}

		$replacement_url = $this->getURL().'?sq_remote_page_action=fetch_url&sq_remote_page_url=';

		// convert the URLs on the remote page into the correct format
		$page_content = $this->_transformContent($response_body, $Fetch_URL->getURL(), $replacement_url);

		return $page_content;

	}//end _process()


	/**
	* Convert all the URLs of the target
	*
	* There are 3 different scenarios for urls. Images and CSS are changed
	* to point to the remote server. Forms and links point back to our server.
	*
	* @param string	$content			the input
	* @param string	$fetch_url			the url
	* @param string	$replacement_url	the replacement
	*
	* @return string
	* @access private
	*/
	function _transformContent($content, $fetch_url, $replacement_url)
	{
		if (!$content || !$fetch_url || !$replacement_url) return false;

		// get all info about the remote URL
		$parsed_fetch_url = parse_url($fetch_url);

		$matches      = Array();
		$subjects     = Array();
		$replacements = Array();

		// patterns for the urls that ONLY need conversion to the absolute URL, not redirected to our page
		// all of these urls ignore the absolute URLs that have http or https in them

		// images, ebedded objects, scripts and linked CSS or JavaScript
		$patterns[] = <<<REGEX
/(<(?:img|embed|script|param|link)\s+.*?(?:value|src|href)\s*=\s*[\\'\"]*)(?:\s)*((?:(?!https?))[^'\" \>]+)(['\"\> ]?[^\<]*\>)/i
REGEX;

		// linked CSS
		$patterns[] = <<<REGEX
/(<(?:input)\s+.*?(?:src)\s*=\s*[\\'\"]+)(?:\s)*((?:(?!https?))[^'\" \>]+)(['\"\> ]?[^\<]*\>)/i
REGEX;

		// CSS url definitions to images and scripts
		$patterns[] = <<<REGEX
/(url\s*\(['\"]?\s*)((?:(?!https?))[^'\" ]+)(['\"]?\s*\))/i
REGEX;

		// table background images
		$patterns[] = <<<REGEX
/(background\s*=\s*['\"]?)(?:\s)*((?:(?!https?))[^'\" ]+)(['\"\> ]?[^<]*\>)/i
REGEX;

		// find whatever matches the above patterns
		foreach ($patterns as $pattern) {
			preg_match_all($pattern, $content, $matches_local, PREG_SET_ORDER);
			$matches = array_merge($matches,$matches_local);

		}

		// fix these matches up
		foreach ($matches as $data) {
			$subject	= $data[0];
			$prefix		= $data[1];
			$url		= $data[2];
			$postfix	= $data[3];

			$full_url = $this->_makeFullURL($parsed_fetch_url, $url);

			// if conversion is unnecessary, ignore this url
			if (!$this->_makeFullURL($parsed_fetch_url, $url)) {
				continue;
			} else {
				$full_url = $url;
			}

			$replacement = $prefix.$full_url.$postfix;

			$subjects[]		= $subject;
			$replacements[] = $replacement;

		}

		// prepare for the next round of url replacements
		$matches = Array();

		// here we handle all the urls that need to be re-written to point back to our site
		$pattern = <<<REGEX
/(<(?:(?:a(?:rea)?[^>]*?href)|(?:i?frame[^>]*?src)|(?:(form)[^>]*action))\s*=\s*['\"]?)(?:\s)*((?:(?![\#]|mailto|javascript))[^'\"> ]+)(['\"\> ]?[^<]*\>)/i
REGEX;

		preg_match_all($pattern, $content, $matches, PREG_SET_ORDER);
		foreach ($matches as $data) {
			$subject	= $data[0];
			$prefix		= $data[1];
			$identifier = $data[2];
			$url		= $data[3];
			$postfix	= $data[4];


			// if new_url cannot not be made, assume the original was correct
			if (!$this->_makeFullURL($parsed_fetch_url, $url)) {
				continue;
			} else {
				$full_url = $url;
			}

			// forms require the parameters to be set to the hidden fields for proper functioning
			if ($identifier == 'form') {
				$postfix .= '<input type="hidden" name="sq_remote_page_action" value="fetch_url" /><input type="hidden" name="sq_remote_page_url" value="'.htmlspecialchars($full_url).'" />';
			}

			// if urls need to point back to us, prepare a new url
			if ($this->attr('load_links')) {
				$parsed_full_url = parse_url($full_url);
				if ($this->attr('only_local_links') && ($parsed_full_url['host'] != $parsed_fetch_url['host'])) {
					continue;
				}
				$full_url = $replacement_url.rawurlencode($full_url);
			}

			$replacement = $prefix.$full_url.$postfix;

			$subjects[]		= $subject;
			$replacements[] = $replacement;

		}

		// finally, we replace all the urls with the re-written ones.
		$content = str_replace($subjects,$replacements,$content);

		return $content;

	}//end _transformContent()


	/**
	* Turn a url into a full url, together with the host and protocol
	*
	* The required missing information is taken from the parsed_fetch_url,
	* which is a url of the remote file that references the item whose url is supplied
	*
	* @access public
	* @return string processed page
	*/
	function _makeFullURL($parsed_fetch_url, &$url)
	{
		// ignore invalid urls
		if (!isset($url) || (substr($url, -3) == '://' )) {
			return false;
		}

		$parsed_url = parse_url($url);

		// ignore absolute urls
		if (isset($parsed_url['scheme']) && !empty($parsed_url['host'])) {
			return true;
		}

		// start building full URL
		$full_url = $parsed_fetch_url['scheme'].'://'.$parsed_fetch_url['host'];
		if ($url{0} == '/') { // Its an absolute path
			$full_url .= $url;
		} else {
			// convert '././foo.ext' into 'foo.ext'
			$url = preg_replace("|^(?:\.\/)+(\w.*)$|","\${1}",$url);
			$url = preg_replace("|(\/+)|",'/',$url);
			$path = array_get_index($parsed_fetch_url,'path','/');
			if (substr($path,-1) != '/'){
				$path = dirname($path);
				if (strlen($path) > 1) {
					$path .= '/';
				}
			}

			$full_url .= $path.$url;
		}

		$url = $full_url;

		return true;

	}//end _makeFullURL()


}//end class

?>