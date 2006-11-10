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
* $Id: page_remote_content.inc,v 1.32.2.11 2006/11/10 00:59:27 tbarrett Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_FUDGE_PATH.'/general/www.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once 'HTTP/Client.php';
require_once 'persistent_cookie_manager.inc';

define ('SQ_REMOTE_MAX_REDIRECTS', 10);
define ('SQ_REMOTE_SOURCE', 'sq_content_src');
define ('SQ_REMOTE_SESSION_COOKIE_NAME', 'sq-active-remote-session');


/**
* Page_Remote_Content
*
* Parse content from the remote location
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.32.2.11 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Remote_Content extends Page
{

	var $_transformation_options = Array(
									'use_design',
									'rewrite_urls',
									'trim_content',
									'replace_matrix_keywords',
									'strip_images',
								   );


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Page_Remote_Content($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		$this->Page($assetid);

	}//end constructor


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
		if (!$this->readAccess()) {
			$GLOBALS['SQ_SYSTEM']->paintLogin(translate('login'), translate('cannot_access_asset', $this->name));
			return;
		}

		$this->_tmp['success'] = $this->_process();

		if ($this->_tmp['transform_settings']['use_design']) {
			return parent::printFrontend();
		} else {
			// do not use design
			$headers = array_get_index($this->_tmp, 'headers', Array());
			foreach ($headers as $name => $value) {
				header("$name: $value");
			}
			echo array_get_index($this->_tmp, 'content', '');
		}

	}//end printFrontend()


	/**
	* Output the content
	*
	* NOTE: If called directly, this function cannot output binary data because
	* usually the output has been started before the function is called.
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		if (!array_get_index($this->_tmp, 'processed', FALSE)) {
			$this->_tmp['success'] = $this->_process();
		}

		if (!$this->_tmp['success']) {
			// processing failed
			if ($this->attr('display_error')) {
				echo $this->attr('error_message');
			}
		} else {
			$type = array_get_index($this->_tmp['headers'], 'content-type');
			if (!empty($type) && (0 !== strpos($type, 'text/'))) {
				// if we have got to this point and are trying to tunnel
				// binary data, we must be nested and so we'll have to show
				// a message instead of the content
				$replacements['remote_url'] = $this->_tmp['url'];
				echo replace_keywords($this->attr('binary_message'), $replacements);
			} else {
				// everything is good. get the page body.
				echo $this->_tmp['content'];

			}
		}
		return TRUE;

	}//end printBody()


	/**
	* Process the remote content request and return the processed HTML
	*
	* @return string
	* @access public
	*/
	function _process()
	{
		$remote_url		= trim($this->attr('remote_url'));
		$remote_source	= array_get_index($_REQUEST, SQ_REMOTE_SOURCE);

		$initial_request = FALSE;

		$is_post = ($_SERVER['REQUEST_METHOD'] == 'POST');
		$url = NULL;

		if (is_null($remote_source)) {
			$initial_request = TRUE;
			$url = $remote_url;
			// enable all transformation by default
			foreach ($this->_transformation_options as $v) {
				$this->_tmp['transform_settings'][$v] = TRUE;
			}
		} else {
			$this->_tmp['transform_settings'] = $this->_decodeHash($remote_source);
			if (empty($this->_tmp['transform_settings'])) {
				return FALSE;
			}
			$url = $this->_tmp['transform_settings']['url'];
			unset($this->_tmp['transform_settings']['url']);
		}

		// here we mark that we've attempted processing of this request and that it failed
		// requires explicit override on success
		$this->_tmp['processed'] = TRUE;

		if (!$url) {
			trigger_localised_error('CMS0062', E_USER_WARNING);
			return FALSE;
		}

		if (!$this->isUrlAllowed($url)) {
			trigger_localised_error('CMS0057', E_USER_WARNING, $url);
			return FALSE;
		}

		$query_string = array_get_index($_SERVER, 'QUERY_STRING');
		if (!empty($query_string) && !is_null($remote_source)) {
			// we're preparing to pass all the query vars to the remote target
			// remove the query var that contains the reference to the remote target
			// because it is for our consumption only
			$source = $is_post ? $remote_source : urlencode($remote_source);
			$query_string = trim(str_replace(SQ_REMOTE_SOURCE.'='.$source, '', $query_string));
		}
		// merge the get vars of current request with those of the source string
		$url_parts = explode('?', $url);
		if (isset($url_parts[1])) {
			$query_string = $url_parts[1].'&'.$query_string;
		}
		$url = $url_parts[0].'?'.$query_string;


		$raw_post = '';
		if ($is_post) {
			$content_type = array_get_index($_SERVER, 'CONTENT_TYPE');
			if (!is_null($content_type) && strpos(strtolower($content_type), 'multipart/form-data') !== FALSE) {
				// NOTE that this is not fully compatible with those systems
				// where post data is allowed to look like a=1&a=2&a=3
				// PHP will only see a=3 in POST vars, so we lose a=1,a=2, so we cannot pass it along
				// that's why when content-type is NOT form-data we use the php://input
				unset($_POST[SQ_REMOTE_SOURCE]);
				$raw_post = make_raw_post_data($_POST);
			} else {
				// the reason we don't always use php://input is because when content type is multipart/form-data it is empty
				$raw_post = file_get_contents('php://input');
				$raw_post = trim(str_replace(SQ_REMOTE_SOURCE.'='.urlencode($remote_source), '', $raw_post));
			}

		}

		// support custom post variables if they've been set
		// NOTE: custom post variables are only sent on INITIAL request
		$post_variables = $this->attr('post_variables');
		if ($initial_request && !empty($post_variables)) {
			$is_post = TRUE;
			$addon_array = Array();
			if (!empty($raw_post)) $addon_array[] = $raw_post;

			foreach ($post_variables as $index => $variable) {
				$addon_array[] = urlencode($variable['name']).'='.urlencode(replace_global_keywords($variable['value']));
			}
			$raw_post = implode('&', $addon_array);
		}

		////// Remote Communication Section. Using HTTP_Client PEAR package
		// check whether or not the HTTP_Client object has already been
		// cached in $_SESSION[SQ_REMOTE_HTTP_CLIENT][AssetID]
		$HTTP_Client = array_get_index(array_get_index($_SESSION, 'SQ_REMOTE_HTTP_CLIENT'), $this->id);
		if (!empty($HTTP_Client)) {
			$HTTP_Client = unserialize($HTTP_Client);
			if (!array_get_index($_COOKIE, SQ_REMOTE_SESSION_COOKIE_NAME)) {
				$HTTP_Client->_cookieManager->deleteSessionCookies();
				setcookie(SQ_REMOTE_SESSION_COOKIE_NAME, 1);
			}
		} else {
			// parameters for HTTP_Request. See Package for description.
			$request_parameters['timeout'] = $this->attr('timeout');
			$request_parameters['useBrackets'] = FALSE;

			// set HTTP authentication
			$user = $this->attr('username');
			if (!empty($user)) {
				$request_parameters['user'] = $user;
				$request_parameters['pass'] = $this->attr('password');
			}

			$default_headers['User-Agent'] = SQ_SYSTEM_LONG_NAME;

			// we need a persistent cookie manager for this
			$cookie_man =& new Persistent_Cookie_Manager();

			$HTTP_Client =& new HTTP_Client($request_parameters, $default_headers, $cookie_man);
			setcookie(SQ_REMOTE_SESSION_COOKIE_NAME, 1);

			if ($this->attr('allow_redirect')) {
				$HTTP_Client->setMaxRedirects(SQ_REMOTE_MAX_REDIRECTS);
			}
			$HTTP_Client->enableHistory(FALSE);
		}


		if ($is_post) {
			// Pass on uploaded files if necessary
			$files = Array();
			foreach ($_FILES as $name => $details) {
				if (is_array($details['tmp_name'])) {
					foreach ($details['tmp_name'] as $i => $tn) {
						if ($details['error'][$i] != UPLOAD_ERR_OK) continue;
						$new_name = SQ_TEMP_PATH.'/'.basename($details['name'][$i]);
						if (move_uploaded_file($tn, $new_name)) {
							$files[] = Array($name.'['.$i.']', $new_name, $details['type'][$i]);
						}
					}
				} else {
					if ($details['error'] != UPLOAD_ERR_OK) continue;
					$new_name = SQ_TEMP_PATH.'/'.basename($details['name']);
					if (move_uploaded_file($details['tmp_name'], $new_name)) {
						$files[] = Array($name, $new_name, $details['type']);
					}
				}
			}
			// a workaround for HTTP_Client's inability to handle empty post data
			if (empty($raw_post) && empty($files)) {
				$random = md5(rand());
				$raw_post = $random.'='.$random;
			}

			$result = $HTTP_Client->post($url, $raw_post, TRUE, $files);
		} else {
			$result = $HTTP_Client->get($url);
		}

		// exit on error
		if (PEAR::isError($result)) {
			trigger_localised_error('CMS0063', E_USER_WARNING, $url, $result->getMessage());
			unset($HTTP_Client);
			return FALSE;
		}
		if (preg_match('/^[45].*?/', $result)) {
			trigger_localised_error('CMS0064', E_USER_WARNING, $result, $url);
			unset($HTTP_Client);
			return FALSE;
		}

		$response = $HTTP_Client->currentResponse();

		$this->_tmp['url']  = $HTTP_Client->_defaultHeaders['Referer'];

		// check if we were redirected to the bad destination
		if ($this->_tmp['url'] != $url) {
			if (!$this->isUrlAllowed($this->_tmp['url'])) {
				trigger_localised_error('CMS0058', E_USER_WARNING, $this->_tmp['url'], $url);
				return FALSE;
			}
		}

		// prepare temporary variables for output
		$this->_tmp['headers'] = $response['headers'];
		$this->_tmp['content'] = $response['body'];

		// wipe the response because we won't use it again and it could be too big to just ignore
		$HTTP_Client->_responses = NULL;
		$_SESSION['SQ_REMOTE_HTTP_CLIENT'][$this->id] = serialize($HTTP_Client);
		unset($HTTP_Client);

		if ($this->_shouldPassStraightThrough($this->_tmp['headers'])) {
			foreach ($this->_tmp['transform_settings'] as $i => $v) {
				$this->_tmp['transform_settings'][$i] = FALSE;
			}
		} else {
			$this->_transformContent($this->_tmp['content']);
		}

		return TRUE;

	}//end _process()


	/**
	* Perform all the transformation of content according to the current settings
	*
	* @param string	&$content	The content to transform
	*
	* @return void
	* @access private
	*/
	function _transformContent(&$content)
	{
		// Get the content between the selected start and end tags if necessary
		if ($this->_tmp['transform_settings']['trim_content']) {
			$this->_trimContent($content);
		}

		// Strip the images if necessary
		if ($this->attr('strip_images') && $this->_tmp['transform_settings']['strip_images']) {
			$this->_stripAllImages($content);
		}//end if

		// Rewrite URLs
		if ($this->_tmp['transform_settings']['rewrite_urls']) {
			$this->_rewriteURLs($content);
		}

	}//end _transformContent()


	/**
	* Returns whether the content should be treated as binary, given the headers supplied
	*
	* @param array	$headers	The headers returned in the HTTP response by the remote server
	*
	* @return boolean
	* @access private
	*/
	function _shouldPassStraightThrough($headers)
	{
		// we shouldn't interfere with attachments
		$disposition = array_get_index($headers, 'content-disposition');
		if (!is_null($disposition) && strpos($disposition, 'attachment') !== FALSE) {
			return TRUE;
		}

		// we shouldn't interfere with non-text content
		// (this actually includes javascript in some cases, but that's OK)
		$content_type = array_get_index($headers, 'content-type');
		if (!empty($content_type) && (strpos($content_type, 'text/') === FALSE)) {
			return TRUE;
		}

		return FALSE;

	}//end _shouldPassStraightThrough()


//--        CONTENT TRANSFORMATION FNS & THEIR HELPERS        --//


	/**
	* Trim the content to that within the selected start and end tags
	*
	* @param string	&$response_body	The content to modify
	*
	* @return void
	* @access private
	*/
	function _trimContent(&$response_body)
	{
		// prepare the tags
		$start_tag = preg_quote($this->attr('start_tag'), '/');
		$end_tag   = preg_quote($this->attr('end_tag'), '/');
		// strip anything outside of the special tags
		// if the start or stop tags are specified, we scan the page for the necessary content
		// otherwise the page is good as it is, no scanning necessary
		if (!empty($start_tag) || !empty($end_tag)) {
			if ($this->attr('include_tags')) {
				$pattern = "/($start_tag.*$end_tag)/s";
			} else {
				$pattern = "/$start_tag(.*)$end_tag/s";
			}

			// note that we can find more than one section identified by the tags
			// all the sections joined together
			preg_match_all($pattern, $response_body, $matches);
			if (count($matches[1])) {
				$response_body = $matches[1][0];
			} else if ($this->attr('no_tag_clear')) {
				// we might be configured to clear the contents if we don't find a match
				$response_body = '';
			}
			$matches = NULL;
		}

	}//end _trimContent()


	/**
	* Rewrite URLs in the content
	*
	* @param string	&$content	the content to rewrite URLs in
	*
	* @return void
	* @access public
	*/
	function _rewriteURLs(&$content)
	{
		$replacement_url = $this->getURL().'?'.SQ_REMOTE_SOURCE.'=';
		$fetch_url = $this->_tmp['url'];

		// get all info about the remote URL
		$parsed_fetch_url = parse_url($fetch_url);
		$subjects     = Array();
		$replacements = Array();

		//// BEGIN SCRIPTS
		$matches = Array();
		$patterns = Array();

		// scripts and linked CSS or JavaScript
		$patterns[] = '/(<(?:script|link)\s+.*?(?:src|href)\s*=\s*[\'"]*)\s*([^\'" \>]+)([\'"> ]?[^<]*\>)/i';

		// CSS url definitions for scripts
		$patterns[] = '/(@import\s+url\s*\([\'"]?\s*)([^\'" ]+)([\'"]?\s*\))/i';
		$patterns[] = '/(@import\s+[\'"]\s*)([^\'" ]+)([\'"])/i';

		// find whatever matches the above patterns
		foreach ($patterns as $pattern) {
			preg_match_all($pattern, $content, $matches_local, PREG_SET_ORDER);
			$matches = array_merge($matches, $matches_local);
		}
		// local cleanup
		$matches_local = NULL;

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

			// if scripts need to be tunneled, prepare a new source url
			if ($this->attr('tunnel_scripts') && $this->isUrlAllowed($url)) {
				$encoded_url = $this->_encodeHash($url, Array('rewrite_urls'));
				$url = $replacement_url.$encoded_url;
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

			// if media need to be tunneled, prepare a new source url
			if ($this->attr('tunnel_media') && $this->isUrlAllowed($url)) {
				$encoded_url = $this->_encodeHash($url);
				$url = $replacement_url.$encoded_url;
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

			// if media need to be tunneled, prepare a new source url
			if ($this->attr('tunnel_media') && $this->isUrlAllowed($url)) {
				$encoded_url = $this->_encodeHash($url);
				$url = $replacement_url.$encoded_url;
			}

			$subjects[]		= $subject;
			$replacements[]	= $prefix.$url.$postfix;
		}
		// local cleanup
		$name_matches = NULL;

		//// END OBJECTS


		//// BEGIN IMAGES
		$matches = Array();
		$patterns = Array();

		// prepare the image patterns
		$patterns[] = '/(<(?:img)\s+.*?(?:src)\s*=\s*[\'"]*)\s*([^\'" <>]+)([\'"> ]?[^<>]*\>)/i';

		$patterns[] = '/(background\s*=\s*[\'"]?)\s*([^\'"<> ]+)([\'" ]?[^<]*>)/i';

		$patterns[] = '/((?:background.*?)url\s*\([\'"]?\s*)([^\'"; ]+)([\'"]?\s*\))/i';
		$patterns[] = '/((?:list-style-image.*?)url\s*\([\'"]?\s*)([^\'"; ]+)([\'"]?\s*\))/i';

		$patterns[] = '/(<(?:input)\s+.*?(?:src)\s*=\s*[\'"]+)\s*([^\'" >]+)([\'"> ]?[^<>]*\>)/i';

		// find whatever matches the above patterns
		foreach ($patterns as $pattern) {
			preg_match_all($pattern, $content, $matches_local, PREG_SET_ORDER);
			$matches = array_merge($matches, $matches_local);
		}
		// local cleanup
		$matches_local = NULL;

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
				$encoded_url = $this->_encodeHash($url);
				$url = $replacement_url.$encoded_url;
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
		$matches_local = NULL;
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
				$encoded_url = $this->_encodeHash($url, Array('all'));
				$url = $replacement_url.$encoded_url;
			}

			$subjects[]		= $subject;
			$replacements[]	= $prefix.$url.$postfix;

		}
		//// END LINKS


		//// BEGIN FORMS
		// prepare for the next round of url replacements
		$matches  = Array();
		$patterns = Array();

		// forms. NOTE: this captures even those actions that are empty
		$patterns[] = '/(<(?:form[^>]*?action)\s*=\s*[\'\"]?)\s*([^\'\"> ]*)([\'\"\> ]?[^<]*\>)/i';

		// find whatever matches the above patterns
		foreach ($patterns as $pattern) {
			preg_match_all($pattern, $content, $matches_local, PREG_SET_ORDER);
			$matches = array_merge($matches, $matches_local);
		}
		// local cleanup
		$matches_local = NULL;

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
				$postfix .= '<input type="hidden" name="'.SQ_REMOTE_SOURCE.'" value="'.$this->_encodeHash($url, Array('all')).'" />';

				$encoded_url = $this->_encodeHash($url, Array('all'));
				$url = $replacement_url.$encoded_url;
			}

			$subjects[]		= $subject;
			$replacements[]	= $prefix.$url.$postfix;

		}
		//// END FORMS

		// finally, we replace all the urls with the re-written ones.
		$content = str_replace($subjects, $replacements, $content);

	}//end _rewriteURLs()


	/**
	* Strip all image tags from the supplied content
	*
	* @param string	&$content	The content to strip image tags from
	*
	* @return void
	* @access private
	*/
	function _stripAllImages(&$content)
	{
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

	}//end _stripAllImages()


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
		// reject invalid urls
		if (!isset($url) || (substr($url, -3) == '://' )) {
			return FALSE;
		}

		if (substr($url,0,2) == '//') {
			$url = $parsed_fetch_url['scheme'].':'.$url;
		}

		// do nothing for absolute urls
		if (preg_match('/^[a-z0-9]+:\/\//i', $url)) {
			return TRUE;
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

		return TRUE;

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


//--        GENERAL HELPERS        --//


	/**
	* Create a hash representing the URL and treatment flags supplied
	*
	* @param array	$url				URL to encode
	* @param array	$transformations	Transformations to perform when tunneling
	*									See $_transformation_options for valid values
	*									May also specify 'all' for all transformations
	*
	* @return string
	* @access private
	*/
	function _encodeHash($url, $transformations=Array())
	{
		if (empty($url)) {
			trigger_localised_error('CMS0065', E_USER_WARNING);
			return '';
		}
		$components = Array('url' => $url);
		foreach ($transformations as $arg) {
			$components[$arg] = 1;
		}
		return urlencode('+'.base64_encode(make_raw_post_data($components)));

	}//end _encodeHash()


	/**
	* Decode a URL hash to a URL and treatment options
	*
	* @param string	$hash	The hash value to decode
	*
	* @return void
	* @access public
	*/
	function _decodeHash($hash)
	{
		// this is a micro hack to detect if PHP has prematurely decoded the url
		if ($hash{0} != '+') $hash = urldecode($hash);

		$hash = base64_decode(substr($hash, 1));
		$components = Array();
		parse_str(html_entity_decode($hash), $components);
		if (empty($components['url'])) {
			trigger_localised_error('CMS0065', E_USER_WARNING);
			return $components;
		}
		// insert any missing options (presumed false) and expand the 'all' option
		foreach ($this->_transformation_options as $i => $v) {
			$components[$v] = !empty($components[$v]) || !empty($components['all']);
		}
		return $components;

	}//end _decodeHash()


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
			$white_url_len = strlen($white_url);
			if ($white_url_len > strlen($url)) {
				continue;
			} else if (strtolower($white_url) == substr($url, 0, $white_url_len)) {
				return TRUE;
			}
		}

		return FALSE;

	}//end isUrlAllowed()


}//end class

?>
