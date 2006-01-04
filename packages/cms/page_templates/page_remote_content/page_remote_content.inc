<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: page_remote_content.inc,v 1.21.2.5 2006/01/04 05:40:38 arailean Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_FUDGE_PATH.'/general/www.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once 'HTTP/Request.php';
require_once 'HTTP/Client/CookieManager.php';


/**
* Page_Remote_Content
*
* Parse content from the remote location
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.21.2.5 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Remote_Content extends Page
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Page_Remote_Content($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Page($assetid);

	}//end constructor


	/**
	* Output the content
	*
	* NOTE: If called directly, this function does not allow the output of binary
	* data because usually the output has been started before the function is called.
	* If binary output is required, printFrontend function should be used, or _tmp variable set.
	**
	* NOTE: No Output should be done after the call to _outputContent (in this function)
	*
	* @return void
	* @access public
	*/
	function printBody()
	{

		// do not allow binary content, unless that decision has already been made elsewhere
		$allow_binary = array_get_index($this->_tmp, 'allow_binary', false);

		if (!array_get_index($this->_tmp, 'processed', false)) {
			$this->_process();
		}

		// if processing failed, display error
		if (!$this->_tmp['success']) {
			if ($this->attr('display_error')) {
				echo $this->attr('error_message');
			}

		} else {
			$this->_outputContent($allow_binary);
		}

		return true;

	}//end printBody()


	/**
	* Output Binary Content directly, avoiding matrix output wrappers
	*
	* @return boolean
	* @access public
	*/
	function _outputBinaryContent()
	{

		// first print the headers
		$headers = $this->_tmp['headers'];
		foreach ($headers as $name => $value) {
			header("$name: $value");
		}

		// then print the content itself
		echo $this->_tmp['content'];

		return true;

	}//end _outputBinaryContent()


	/**
	* Content outputter
	*
	* Assumes the processing has been performed and all the necessary
	* variables have been set in the _tmp
	* Prints to screen or outputs binary content
	*
	* @param boolean	$allow_binary	determines whether binary output is allowed
	*
	* @return void
	* @access public
	*/
	function _outputContent($allow_binary=true)
	{

		$result = '';

		$binary = array_get_index($this->_tmp, 'binary', false);
		if ($binary && $allow_binary) {
			// binary types need special treatment
			$type = $this->_tmp['type'];

			// if content is an image and we are tunneling images, otput it as binary
			if ((strpos($type,'image/')!==false) && $this->attr('tunnel_images')) {
				if ($this->attr('strip_images')) {
					$result = '[Image]';
				} else {
					$this->_outputBinaryContent();
					return;
				}
			// media content is otput as binary if tunneling
			} else if ((strpos($type,'application/')!==false) && $this->attr('tunnel_media')) {
				$this->_outputBinaryContent();
				return;

			} else {
				// otherwise display a message relating to binary content
				$replacements['remote_url'] = $this->_tmp['url'];
				$result = replace_keywords($this->attr('binary_message'), $replacements);
			}

		} else if ($binary && !$allow_binary) {
			$result = '['.translate('cms_remote_content_binary_data').']';

		} else {
			// textual content is ready for output
			$result = $this->_tmp['content'];
		}

		echo $result;

	}//end _outputContent()


	/**
	* Output this Asset to the Front end.
	*
	* Assumes that no output is being done before it is allowed to output, so
	* outputting binaries is allowed. Relies on printBody to do the outputting.
	*
	* @return void
	* @access public
	*/
	function printFrontend()
	{
		$this->_process();

		if (array_get_index($this->_tmp, 'binary', false)) {
			$this->_tmp['allow_binary'] = true;
			return $this->printBody();
		} else {
			return parent::printFrontend();
		}

	}//end printFrontend()


	/**
	* Process HTML and Text content
	*
	* Takes HTML content (text in general),
	* tries to find the sections identified by start and stop tags,
	* requests for urls to be conditionally replaced
	*.
	* Assumptions:
	*   processing has already taken place AND
	*   no error checking of input is necessary
	*
	* @return string
	* @access public
	*/
	function processHtml()
	{
		// everything is good. get the page body.
		$response_body	= $this->_tmp['content'];

		// prepare the tags
		$start_tag = preg_quote($this->attr('start_tag'), '/');
		$end_tag   = preg_quote($this->attr('end_tag'), '/');

		// strip anything outside of the special tags
		// if the start or stop tags are specified, we scan the page for the necessary content
		// otherwise the page is good as it is, no scanning necessary
		if (!empty($start_tag) || !empty($end_tag)) {
			if ($this->attr('include_tags')) {
				$pattern = "/($start_tag.*?$end_tag)/s";
			} else {
				$pattern = "/$start_tag(.*?)$end_tag/s";
			}

			// note that we can find more than one section identified by the tags
			// all the sections joined together
			preg_match_all($pattern, $response_body, $matches);
			if (count($matches[1])) {
				$response_body = '';
				foreach ($matches[1] as $match) {
					$response_body .= $match;
				}
			}
			$matches = null;
		}

		// prepare the url that will be used to replace the links that have to be tunnelled through remote content
		$replacement_url = $this->getURL().'?sq_remote_page_action=fetch_url&sq_remote_page_url=';

		// convert the URLs on the remote page into the correct format
		$page_content = $this->_transformContent($response_body, $this->_tmp['url'], $replacement_url);

		return $page_content;

	}//end processHtml()


	/**
	* Checks whether url is allowed by security settings
	*
	* @param string	$url	URL to check
	*
	* @return boolean
	* @access public
	*/
	function isUrlAllowed($url)
	{
		if (!isset($this->_tmp['_white_list_'])) {
			$this->_tmp['_white_list_'] = Array();
			$Net_URL =& new Net_URL($this->attr('remote_url'));
			if ($this->attr('only_local_links')) {
				// this is to support legacy functionality where 'Only Local Links' meant all the links in the local domain
				$this->_tmp['_white_list_'][] = strtolower($Net_URL->host);
			} else {
				$this->_tmp['_white_list_'] = $this->attr('url_white_list');
				if (!empty($this->_tmp['_white_list_'])) {
					$this->_tmp['_white_list_'][] = strtolower($Net_URL->host.$Net_URL->path);
				}
			}
		}

		if (empty($this->_tmp['_white_list_'])) return TRUE;


		$Net_URL =& new Net_URL($url);
		$url = $Net_URL->host;
		if (!empty($Net_URL->path)) {
			$url = $url.$Net_URL->path;
		}
		$url = strtolower($url);

		foreach ($this->_tmp['_white_list_'] as $white_url) {
			if (strlen($white_url) > strlen($url)) {
				continue;
			} else if (strtolower($white_url) == substr($url, 0, strlen($white_url))) {
				return true;
			}
		}

		return false;

	}//end isUrlAllowed()


	/**
	* Process the remote content request and return the processed HTML
	*
	* @return string
	* @access public
	*/
	function _process()
	{
		$remote_url = trim($this->attr('remote_url'));

		$fetch_url	= array_get_index($_REQUEST, 'sq_remote_page_url');
		$action		= array_get_index($_REQUEST, 'sq_remote_page_action');

		if (!isset($fetch_url) || ($action != 'fetch_url')) {
			$fetch_url = $remote_url;
		}

		if (!$fetch_url) {
			$this->_tmp['success'] = false;
			return false;
		}

		$fetch_url = undo_htmlspecialchars($fetch_url);
		$Fetch_URL =& new Net_URL($fetch_url);


		if (!$this->isUrlAllowed($Fetch_URL->getURL())) {
			trigger_localised_error('CMS0057', E_USER_WARNING, $fetch_url);
			$this->_tmp['success'] = false;
			$this->_tmp['processed'] = true;
			return false;
		}

		// let's see if we're getting any variables in the get string
		// so we can pass them back to the original page
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
		if (!empty($user)) {
			$request_parameters['user'] = $user;
			$request_parameters['pass'] = $this->attr('password');
		}

		$url = $Fetch_URL->getURL();

		$HTTP_Request =& new HTTP_Request($url, $request_parameters);
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
			$Cookie_Manager =& new HTTP_Client_CookieManager();
		}


		// process the request
		$result = $HTTP_Request->sendRequest();
		// get new cookies and save cookie manager
		$Cookie_Manager->updateCookies($HTTP_Request);
		$_SESSION['SQ_REMOTE_COOKIE_MANAGER'] = serialize($Cookie_Manager);

		// check if there was an error of type 4xx or 5xx in the HTTP response
		$is_error = preg_match('/^[45].*?/', $HTTP_Request->getResponseCode());

		// exit on error, checking for object because if it is true, than it is
		// a PEAR error object
		if (!$result || PEAR::isError($result) || $is_error) {
			$this->_tmp['success'] = false;
			$this->_tmp['processed'] = true;
			return false;
		}

		// obtain new fetch url. important if there were redirects.
		$Fetch_URL = $HTTP_Request->_url;
		$this->_tmp['url'] = $Fetch_URL->getURL();

		// check if we were redirected to a bad destination
		if ($this->_tmp['url'] != $url && !$this->isUrlAllowed($this->_tmp['url'])) {
			trigger_localised_error('CMS0058', E_USER_WARNING, $this->_tmp['url'], $fetch_url);
			$this->_tmp['success'] = false;
			$this->_tmp['processed'] = true;
			return false;
		}

		// prepare temporary variables for output
		$this->_tmp['type'] = $HTTP_Request->getResponseHeader('content-type');
		$this->_tmp['headers'] = $HTTP_Request->getResponseHeader();
		$this->_tmp['content'] = $HTTP_Request->getResponseBody();

		// if content is not binary, it needs further processing
		if (strpos($this->_tmp['type'],'text/') !== false) {
			$this->_tmp['content'] = $this->processHtml();
		} else {
			$this->_tmp['binary'] = true;
		}

		$this->_tmp['processed'] = true;
		$this->_tmp['success'] = true;

		unset($HTTP_Request);
		return true;

	}//end _process()


	/**
	* Replace image button with a regular HTML Submit button
	*
	* @param string	$input_string			full input tag
	* @param string	$default_button_name	default name of the button to replace the image
	*
	* @return string
	* @access private
	*/
	function _imageButtonReplace($input_string, $default_button_name='Submit Button')
	{
		$alt_pattern  = '/(?:alt\s*=[\'" ]([^\'"><]+)[\'" ])/i';
		$type_pattern = '/(type\s*=[\'" ][^\'"><]+[\'" ])/i';

		if (preg_match($alt_pattern, $input_string, $matches)) {
			$button_name = ucfirst($matches[1]);
		} else {
			$button_name = $default_button_name;
		}

		$type_replacement = ' type="Submit" value="'.$button_name.'" ';

		$result = preg_replace($type_pattern, $type_replacement, $input_string);

		return $result;

	}//end _imageButtonReplace()


	/**
	* Strip an image
	*
	* Replaces an HTML image tag with its ALT tag or a default name
	*
	* @param string	$image_string		full image tag
	* @param string	$default_image_name	default word to replace image with if Alt tag is not found
	*
	* @return string
	* @access private
	*/
	function _imageStrip($image_string, $default_image_name='Image')
	{
		$alt_pattern = '/(?:alt\s*=[\'" ]([^\'"><]+)[\'" ])/i';

		if (preg_match($alt_pattern, $image_string, $matches)) {
			$image_name = ucfirst($matches[1]);
		} else {
			$image_name = $default_image_name;
		}

		return "[$image_name]";

	}//end _imageStrip()


	/**
	* Convert all the URLs of the target
	*
	* Apart from converting actions and images to full URLs, this also converts
	* href's into full URLs.
	*
	* @param string	$content			the input
	* @param string	$fetch_url			the url of the page being transformed
	* @param string	$replacement_url	the replacement url that points back to our system
	*
	* @return string
	* @access private
	*/
	function _transformContent($content, $fetch_url, $replacement_url)
	{
		if (!$content || !$fetch_url || !$replacement_url) {
			return false;
		}

		// get all info about the remote URL
		$parsed_fetch_url = parse_url($fetch_url);

		// strip the images if necessary
		if ($this->attr('strip_images')) {

			// replace HTML images with their ALT tags
			$pattern = '/(<(?:img)[^>]*?>)/i';

			$content = preg_replace_callback(
				$pattern,
				create_function(
					'$matches',
					'return Page_Remote_Content::_imageStrip($matches[0]);'
				),
				$content
			);

			// remove HTML background images
			$pattern = '/(background\s*=\s*[\'\"]?\s*[^\'\" <>]+[\'\"]?)/i';
			$content = preg_replace($pattern, '', $content);

			// strip CSS backgrounds
			$pattern = '/(background.*?)(url\(.*?\))/i';
			$content = preg_replace($pattern, '\$1', $content);

			// change Form Image buttons with regular buttons
			$pattern = '/(<(?:input)[^>]*type=["\' ]image["\' ][^>]*?>)/i';

			$content = preg_replace_callback(
				$pattern,
				create_function(
					'$matches',
					'return Page_Remote_Content::_imageButtonReplace($matches[0]);'
				),
				$content
			);

		}

		$subjects     = Array();
		$replacements = Array();

		//// BEGIN SCRIPTS
		$matches = Array();
		$patterns = Array();

		// scripts and linked CSS or JavaScript
		$patterns[] = '/(<(?:script|link)\s+.*?(?:src|href)\s*=\s*[\'"]*)\s*([^\'" \>]+)([\'"> ]?[^<]*\>)/i';

		// CSS url definitions for scripts
		$patterns[] = '/(@import\s+url\s*\([\'"]?\s*)([^\'" ]+)([\'"]?\s*\))/i';

		// find whatever matches the above patterns
		foreach ($patterns as $pattern) {
			preg_match_all($pattern, $content, $matches_local, PREG_SET_ORDER);
			$matches = array_merge($matches, $matches_local);
		}
		// local cleanup
		$matches_local = null;

		// fix these matches up
		foreach ($matches as $data) {
			$subject = $data[0];
			$prefix  = $data[1];
			$url     = $data[2];
			$postfix = $data[3];

			// if conversion is unnecessary, ignore this url
			if (!$this->_makeFullURL($parsed_fetch_url, $url)) {
				continue;
			}

			// if images need to be tunneled, prepare a new source url
			if ($this->attr('tunnel_scripts') && $this->isUrlAllowed($url)) {
				$url = $replacement_url.rawurlencode($url);
			}

			$subjects[]     = $subject;
			$replacements[] = $prefix.$url.$postfix;

		}
		//// END SCRIPTS


		//// BEGIN OBJECTS
		$matches      = Array();

		// objects
		$pattern = '/(<(?:embed)\s+.*?(?:src)\s*=\s*[\'"]*)\s*([^\'" >]+)([\'"> ]?[^<]*>)/i';

		// find whatever matches the above patterns
		preg_match_all($pattern, $content, $matches, PREG_SET_ORDER);

		// fix these matches up
		foreach ($matches as $data) {
			$subject	= $data[0];
			$prefix		= $data[1];
			$url		= $data[2];
			$postfix	= $data[3];

			// make full url, ignore invalid urls
			if (!$this->_makeFullURL($parsed_fetch_url, $url)) {
				continue;
			}

			// if images need to be tunneled, prepare a new source url
			if ($this->attr('tunnel_media') && $this->isUrlAllowed($url)) {
				$url = $replacement_url.rawurlencode($url);
			}

			$subjects[]     = $subject;
			$replacements[] = $prefix.$url.$postfix;
		}


		// FLASH MOVIE
		$matches      = Array();

		// object parameters (flash)
		$pattern = '/(<(?:param)\s+.*?(?:value)\s*=\s*[\'"]*)\s*([^\'" >]+)([\'"> ]?[^<]*>)/i';

		preg_match_all($pattern, $content, $matches, PREG_SET_ORDER);

		// fix these matches up
		foreach ($matches as $data) {
			$subject	= $data[0];
			$prefix		= $data[1];
			$url		= $data[2];
			$postfix	= $data[3];

			// find the name of the subject
			$pattern = '/name\s*=\s*[\'"]*\s*([^\'" >]+)[\'"> ]+/i';

			preg_match($pattern, $subject, $name_matches);
			$param_name = $name_matches['1'];

			// only look at parameter whose name is movie (for flash)
			if ($param_name != 'movie') continue;

			// make full url, ignore invalid urls
			if (!$this->_makeFullURL($parsed_fetch_url, $url)) {
				continue;
			}

			// if images need to be tunneled, prepare a new source url
			if ($this->attr('tunnel_media') && $this->isUrlAllowed($url)) {
				$url = $replacement_url.rawurlencode($url);
			}

			$subjects[]		= $subject;
			$replacements[]	= $prefix.$url.$postfix;
		}
		// local cleanup
		$name_matches = null;

		//// END OBJECTS


		//// BEGIN IMAGES
		$matches = Array();
		$patterns = Array();

		// prepare the image patterns
		$patterns[] = '/(<(?:img)\s+.*?(?:src)\s*=\s*[\'"]*)\s*([^\'" <>]+)([\'"> ]?[^<>]*\>)/i';

		$patterns[] = '/(background\s*=\s*[\'"]?)\s*([^\'"<> ]+)([\'" ]?[^<]*>)/i';

		$patterns[] = '/((?:background.*?)url\s*\([\'"]?\s*)([^\'"; ]+)([\'"]?\s*\))/i';

		$patterns[] = '/(<(?:input)\s+.*?(?:src)\s*=\s*[\'"]+)\s*([^\'" >]+)([\'"> ]?[^<>]*\>)/i';

		// find whatever matches the above patterns
		foreach ($patterns as $pattern) {
			preg_match_all($pattern, $content, $matches_local, PREG_SET_ORDER);
			$matches = array_merge($matches, $matches_local);
		}
		// local cleanup
		$matches_local = null;

		foreach ($matches as $data) {
			$subject	= $data[0];
			$prefix		= $data[1];
			$url		= $data[2];
			$postfix	= $data[3];

			// make full url, ignore invalid urls
			if (!$this->_makeFullURL($parsed_fetch_url, $url)) {
				continue;
			}

			// if images need to be tunneled, prepare a new source url
			if ($this->attr('tunnel_images') && $this->isUrlAllowed($url)) {
				$url = $replacement_url.rawurlencode($url);
			}

			$subjects[]		= $subject;
			$replacements[]	= $prefix.$url.$postfix;

		}
		//// END IMAGES


		//// BEGIN LINKS
		// prepare for the next round of url replacements
		$matches	= Array();
		$patterns	= Array();

		// PATTERNS for urls that might need to be rewritten to point back to remote content
		// links and frames except mailto and javascript
		$patterns[] = '/(<(?:(?:a(?:rea)?[^>]*?href)|(?:i?frame[^>]*?src))\s*=\s*[\'"]?)\s*((?:(?![\#]|mailto|javascript))[^\'"> ]+)([\'"> ]?[^<]*>)/i';

		// this pattern is a special case. we don't generally look at JavaScript
		// because that is a whole new territory
		// this one was added for compatibility with MySource v2 ecom module
		$patterns[] = '/(<input[^>]*?onclick\s*=\s*[\'"]location=\')\s*([^\'"> ]+)([\'"> ]?[^<]*>)/i';

		// find whatever matches the above patterns
		foreach ($patterns as $pattern) {
			preg_match_all($pattern, $content, $matches_local, PREG_SET_ORDER);
			$matches = array_merge($matches, $matches_local);
		}
		// local cleanup
		$matches_local = null;

		$load_links = $this->attr('load_links');

		foreach ($matches as $data) {
			$subject	= $data[0];
			$prefix		= $data[1];
			$url		= $data[2];
			$postfix	= $data[3];

			// make full url, ignore invalid urls
			if (!$this->_makeFullURL($parsed_fetch_url, $url)) {
				continue;
			}

			// if urls need to point back to us, prepare a new url
			if ($load_links && $this->isUrlAllowed($url)) {
				$url = $replacement_url.rawurlencode($url);
			}

			$subjects[]		= $subject;
			$replacements[]	= $prefix.$url.$postfix;

		}
		//// END LINKS


		//// BEGIN FORMS
		// prepare for the next round of url replacements
		$matches  = Array();
		$patterns = Array();

		// forms. note this captures even those actions that are empty
		$patterns[] = '/(<(?:form[^>]*?action)\s*=\s*[\'\"]?)\s*([^\'\"> ]*)([\'\"\> ]?[^<]*\>)/i';

		// find whatever matches the above patterns
		foreach ($patterns as $pattern) {
			preg_match_all($pattern, $content, $matches_local, PREG_SET_ORDER);
			$matches = array_merge($matches, $matches_local);
		}
		// local cleanup
		$matches_local = null;

		foreach ($matches as $data) {
			$subject = $data[0];
			$prefix  = $data[1];
			$url     = $data[2];
			$postfix = $data[3];

			// make full url, ignore invalid urls
			if (!$this->_makeFullURL($parsed_fetch_url, $url)) {
				continue;
			}

			// if urls need to point back to us, prepare a new url
			if ($load_links && $this->isUrlAllowed($url)) {
				// forms require the parameters to be set to the hidden fields for proper functioning
				$postfix .= '<input type="hidden" name="sq_remote_page_action" value="fetch_url" /><input type="hidden" name="sq_remote_page_url" value="'.htmlspecialchars($url).'" />';

				$url = $replacement_url.rawurlencode($url);
			}

			$subjects[]		= $subject;
			$replacements[]	= $prefix.$url.$postfix;

		}
		//// END FORMS

		// finally, we replace all the urls with the re-written ones.
		$content = str_replace($subjects, $replacements, $content);

		return $content;

	}//end _transformContent()


	/**
	* Turn a url into a full url, together with the host and protocol
	*
	* The required missing information is taken from the parsed_fetch_url,
	* which is a url of the remote file that references the item whose url is supplied
	*
	* @param array	$parsed_fetch_url	fetch url that has been parsed with parse_url()
	* @param string	&$url				reference to url that needs to be converted
	*
	* @return boolean
	* @access public
	*/
	function _makeFullURL($parsed_fetch_url, &$url)
	{
		// ignore invalid urls
		if (!isset($url) || (substr($url, -3) == '://' )) {
			return false;
		}

		if (substr($url,0,2) == '//') {
			$url = $parsed_fetch_url['scheme'].':'.$url;
		}

		// ignore absolute urls
		if (preg_match('/^[a-z0-9]+:\/\//i', $url)) {
			return true;
		}

		// start building full URL
		$full_url = $parsed_fetch_url['scheme'].'://'.$parsed_fetch_url['host'];
		if (isset($parsed_fetch_url['port'])) {
			$full_url .= ':'.$parsed_fetch_url['port'];
		}

		if (strlen($url) && $url{0} == '/') {
			$final_path = $url;
		} else {
			$path = array_get_index($parsed_fetch_url,'path','/');
			if (substr($path,-1) != '/') {
				$path = dirname($path);
				if (strlen($path) > 1) $path .= '/';
			}

			$final_path = $path.$url;
		}

		$full_url .= $this->_makeProperPath($final_path);

		$url = $full_url;

		return true;

	}//end _makeFullURL()


	/**
	* Make a proper unix path
	*
	* Works similar to php's realpath(), but doesn't rely on a file system
	* Given a string representing a path, it tries to remove the relative references
	* in short: given "/this/dir/../another/file.php" will produce "/this/another/file.php"
	**
	* NOTE: paths that try to jump outside the root will produce possibly erroneous result
	* eg.: "/root/dir/../../../another/file.php" will become "/another/file.php"
	* this behaviour is similar to how browsers treat relative paths
	*
	* @param string	$path	path that needs shortening
	*
	* @return string
	* @access public
	*/
	function _makeProperPath($path='')
	{
		if (empty($path))
			return '';

		$root = '';
		$path_components = explode('/',$path);

		if (empty($path_components[0])) {
			$root = '/';
			unset($path_components[0]);
		}

		$stack = Array();

		foreach ($path_components as $component) {
			switch ($component) {
				case '..':
					if (!empty($stack)) array_pop($stack);
				break;

				case '.':
				case '':
					continue;
				break;

				default:
					array_push($stack, $component);
			}
		}

		$new_path = implode('/', $stack);

		return $root.$new_path;

	}//end _makeProperPath()


}//end class

?>