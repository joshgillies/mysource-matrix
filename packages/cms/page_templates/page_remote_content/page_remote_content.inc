<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: page_remote_content.inc,v 1.99.2.3 2013/07/11 00:36:26 akarelia Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_FUDGE_PATH.'/general/www.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

// Consider making these constants into attributes of this asset
define ('SQ_REMOTE_MAX_REDIRECTS', 10);
define ('SQ_REMOTE_SOURCE', 'sq_content_src');
define ('SQ_REMOTE_INPUT_FILE_NAMES', 'sq_remote_input_file_names');
define ('SQ_REMOTE_SESSION_COOKIE_NAME', 'sq-active-remote-session');


/**
* Page_Remote_Content
*
* Parse content from the remote location
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.99.2.3 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Remote_Content extends Page
{

	var $_transformation_options = Array(
									'use_design',
									'rewrite_urls',
									'trim_content',
									'do_tag_replacements',
									'replace_matrix_keywords',
									'strip_images',
								   );


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

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
		// start performance mode timer
		$GLOBALS['SQ_SYSTEM']->pm->startTimer($this, 'printFrontend');

		if (!$this->readAccess()) {
			$GLOBALS['SQ_SYSTEM']->paintLogin(translate('login'), translate('cannot_access_asset', $this->name));
			return;
		}

		$success = $this->_process();

		if ($success === TRUE) {
			$url = $this->_tmp['url'];
			$this->_tmp['success'][$url] = $success;
		}

		if (!$this->_tmp['transform_settings']['replace_matrix_keywords']) {
			$GLOBALS['SQ_SYSTEM']->setGlobalDefine('SQ_REPLACE_MYSOURCE_LEVEL_KEYWORDS', FALSE);
		}

		if ($this->_tmp['transform_settings']['use_design']) {
			parent::printFrontend();
			// stop performance mode timer
			$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this, 'printFrontend');
			return;
		} else {
			// do not use design
			$headers_list = array_get_index($this->_tmp, 'headers', Array());
			$headers = array_get_index($headers_list, $url, Array());
			foreach ($headers as $name => $value) {
				header("$name: $value");
			}
			$content = array_get_index($this->_tmp, 'content', Array());
			echo array_get_index($content, $url, '');
		}

		// stop performance mode timer
		$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this, 'printFrontend');

	}//end printFrontend()


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
		// start performance mode timer
		$GLOBALS['SQ_SYSTEM']->pm->startTimer($this, 'printBody');

		// Encoded URLs look like they've got keywords in them. Disable blanking out of unknown keywords
		$GLOBALS['SQ_SYSTEM']->setGlobalDefine('SQ_REPLACE_MYSOURCE_LEVEL_KEYWORDS', FALSE);

		$success = $this->_process();
		$url = array_get_index($this->_tmp, 'url', '');

		if ($success === FALSE) {
			// processing failed
			if ($this->attr('display_error')) {
				echo $this->attr('error_message');
			}
		} else {
			// if _process returns null, in some cases headers and content may not be set properly.
			// So check they are available before trying to use them.
			if (isset($this->_tmp['headers']) === TRUE && isset($this->_tmp['content']) === TRUE) {
				$type = array_get_index($this->_tmp['headers'], 'content-type');
				if (!empty($type) && (0 !== strpos($type, 'text/'))) {
					// if we have got to this point and are trying to tunnel
					// binary data, we must be nested and so we'll have to show
					// a message instead of the content
					$replacements['remote_url'] = $url;
					echo replace_keywords($this->attr('binary_message'), $replacements);
				} else {
					// everything is good. get the page body.
					echo $this->_tmp['content'][$url];
				}
			}
		}

		// stop performance mode timer
		$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this, 'printBody');
		return TRUE;

	}//end printBody()


	/**
	* Process the remote content request and return the processed HTML
	*
	* Returns:
	*   TRUE on a successful retrieval.
	*   FALSE on a failure of some sort.
	*   NULL if we have already processed the URL. This status MUST NOT be
	*   cached, and any previous success/failure result SHOULD be used.
	*
	* @return boolean
	* @access public
	*/
	function _process()
	{
		$remote_url		= trim($this->attr('remote_url'));
		$remote_source	= array_get_index($_REQUEST, SQ_REMOTE_SOURCE);

		$initial_request = FALSE;

		$is_post = (isset($_SERVER['REQUEST_METHOD']) && $_SERVER['REQUEST_METHOD'] == 'POST');
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
			$url = $this->_tmp['transform_settings']['url'];
			unset($this->_tmp['transform_settings']['url']);
		}

		if (!$url) {
			trigger_localised_error('CMS0062', E_USER_WARNING);
			return FALSE;
		}

		if (!$this->isUrlAllowed($url)) {
			trigger_localised_error('CMS0057', E_USER_WARNING, $url);
			return FALSE;
		}

		//Implode the GET array to construct the query string.
	   	//Avoid using $_SERVER['query_string'], because, some $_GET vars are passed indirectly.We have to trust $_GET[]
		$first = true;
   		$query_string = '';
		foreach($_GET as $key => $value) {
		    // if it's array format, e.g test[0]=1, PHP will convert it to multiple level array, convert it to plain string format
		    if(is_array($value)) {
			$string_param = http_build_query(array($key => $value));
			if($first) {
			    $query_string = $string_param;
			    $first = false;
			}
			else {
			    $query_string .= '&'.$string_param;
			}
		    }
		    else {
			    if ($first) {
				$query_string = $key.'='.urlencode($value);
				$first = false;
			    } else {
				$query_string .= '&'.$key.'='.urlencode($value);
			    }
		    }
		}

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
			// Replace global keywords on the HARD CODED portion of the query string only
			replace_global_keywords($url_parts[1]);
			$query_string = empty($query_string) ? $url_parts[1] : $url_parts[1].'&'.$query_string;
		}
		$url = $url_parts[0];
		if (empty($query_string) === FALSE) {
			$url .= '?'.$query_string;
		}

		// If we have already handled this complete URL, mark it as such
		$processed = array_get_index($this->_tmp, 'processed', Array());
		if (isset($processed[$url])) {
			// "NULL" implies that the URL was processed successfully before
			return $processed[$url] ? NULL : FALSE;
		}

		// here we mark that we've attempted processing of this request
		// if the processing turns out to successfull later, we will set as "TRUE" then
		$this->_tmp['processed'][$url] = FALSE;

		$post_data = NULL;
		$post_is_encoded = FALSE;

		if ($is_post) {
			unset($_POST[SQ_REMOTE_SOURCE]);

			$content_type = array_get_index($_SERVER, 'CONTENT_TYPE', '');
			if (strpos(strtolower($content_type), 'multipart/form-data') === FALSE) {
				$post_data = trim(file_get_contents('php://input'));

				// NOTE that when we trim out SQ_REMOTE_SOURCE varible and value, we also need to
				// remove '&' delimiter before or after if it exists.
				$remote_source_str = SQ_REMOTE_SOURCE.'='.urlencode($remote_source);
				$remote_source_str_len = strlen($remote_source_str);
				if (($pos = strpos($post_data,  $remote_source_str)) !== FALSE) {
					if ($pos == 0) {
						// found at the beginning, trim one after
						$post_data = substr($post_data, $remote_source_str_len + 1);
					} else if ( ($pos + strlen($remote_source_str)) == strlen($post_data) ) {
						// found at the end, trim one before
						$post_data = substr($post_data, 0, strlen($post_data) - $remote_source_str_len - 1);
					} else {
						// found in the middle, trim one after
						$post_data = substr($post_data, 0, $pos).substr($post_data, $pos + $remote_source_str_len + 1);
					}
				}
				$post_is_encoded = TRUE;
			} else {
				// NOTE that this is not fully compatible with those systems
				// where post data is allowed to look like a=1&a=2&a=3
				// PHP will only see a=3 in POST vars, so we lose a=1,a=2, so we cannot pass it along
				// that's why when content-type is NOT form-data we use the php://input
				// the reason we don't always use php://input is because when content type is multipart/form-data it is empty
				$post_data = $_POST;

				// if it's array format, e.g test[0]=1, PHP will convert it to multiple level array, convert it to string format for CURL
				foreach ($post_data as $name => $value) {
				    if(is_array($value)) {
					$string_param = http_build_query(array($name => $value));
					$array_param = explode('&', $string_param);
					foreach ($array_param as $part) {
					    $part_param = explode('=', $part);
					    if(isset($part_param[0]) && isset($part_param[1]))
						$post_data[urldecode($part_param[0])] = urldecode($part_param[1]);
					}
					unset($post_data[$name]);
				    }
				}
			}

		}//end if

		// support custom post variables if they've been set
		// NOTE: custom post variables are only sent on INITIAL request
		$post_variables = $this->attr('post_variables');
		if ($initial_request && !empty($post_variables)) {
			$is_post = TRUE;
			if ($post_is_encoded) {
				$addon_array = Array();
				if (!empty($post_data)) $addon_array[] = $post_data;
				foreach ($post_variables as $index => $variable) {
					$addon_array[] = urlencode($variable['name']).'='.urlencode(replace_global_keywords($variable['value']));
				}
				$post_data = implode('&', $addon_array);

			} else {
				foreach ($post_variables as $index => $variable) {
					$var_name = $variable['name'];
					$var_value = $variable['value'];
					$var_value = replace_global_keywords($var_value);
					if (isset($post_data[$var_name])) {
						if (!is_array($post_data[$var_name])) {
							$post_data[$var_name] = Array($post_data[$var_name], $var_value);
						} else {
							$post_data[$var_name][] = $var_value;
						}
					} else {
						$post_data[$var_name] = $var_value;
					}
				}
			}
		}

		// restore cookie from user's session storage
		// since we really want to make sure Matrix user got individual cookie jar for the remote content
		if (!is_dir($this->data_path)) {
			create_directory($this->data_path);
		}
		$sessionid = session_id();
		$cookie_filename = $this->data_path."/cookies_".$sessionid;
		if(isset($_SESSION['SQ_REMOTE_CONTENT_COOKIE'][$this->id])) {
		    string_to_file($_SESSION['SQ_REMOTE_CONTENT_COOKIE'][$this->id], $cookie_filename);
		}

		$options = array(
					'CONNECTTIMEOUT' => $this->attr('timeout'),
					'COOKIEFILE'     => $cookie_filename,
					'COOKIEJAR'      => $cookie_filename,
					'TIMEOUT'        => $this->attr('timeout'),
					'USERAGENT'      => SQ_SYSTEM_LONG_NAME,
					'RETURNTRANSFER' => 1,
				   );

		// set HTTP authentication
		$user = $this->attr('username');
		if (!empty($user)) {
			$options['http_authentication']['username'] = $user;
			$options['http_authentication']['password'] = $this->attr('password');
		}

		if ($this->attr('allow_redirect')) {
			$options['FOLLOWLOCATION'] = true;
			$options['MAXREDIRS']      = SQ_REMOTE_MAX_REDIRECTS;
		}

		setcookie(SQ_REMOTE_SESSION_COOKIE_NAME, 1);

		if ($is_post) {
			$hidden_field_file_name = $this->getPrefix().'_'.SQ_REMOTE_INPUT_FILE_NAMES;
			if (!empty($_POST[$hidden_field_file_name])) {
				$input_file_names = unserialize($_POST[$hidden_field_file_name]);
			} else {
				$input_file_names = Array();
			}

			// Only send files to the remote URL if those files are in the remote content page
			$temp_dirs = Array();
			if (!empty($input_file_names)) {
				foreach ($_FILES as $name => $details) {
					if (in_array($name, $input_file_names) && isset($_SESSION['SQ_REMOTE_CONTENT_UPLOAD_FILE'][$name])) {
						$uniqid = uniqid();
						$new_dir = SQ_TEMP_PATH.'/remote_content_'.$uniqid;
						if(!mkdir($new_dir)) continue;
						$temp_dirs[] = $new_dir;
						if (is_array($details['tmp_name'])) {
							foreach ($details['tmp_name'] as $i => $tn) {
								if ($details['error'][$i] != UPLOAD_ERR_OK) continue;
								$new_name = $new_dir.'/'.basename($details['name'][$i]);
								if (move_uploaded_file($tn, $new_name)) {
									$post_data[$name.'['.$i.']'] = '@'.$new_name;
								}
							}
						} else {
							if ($details['error'] != UPLOAD_ERR_OK) continue;
							$new_name = $new_dir.'/'.basename($details['name']);
							if (move_uploaded_file($details['tmp_name'], $new_name)) {
								$post_data[$name] = '@'.$new_name;
							}
						}
					}//end if
				}//end for
			}//end if

			// a workaround for curl's inability to handle empty post data
			if (empty($post_data) && empty($files)) {
				if ($post_is_encoded) {
					$random = md5(rand());
					$post_data = $random.'='.$random;
				}
			}

			$options['POST']       = true;
			$options['POSTFIELDS'] = $post_data;
		}

		// Make sure we're not trying to include (or redirect) to ourselves.
		$myUrls = $this->getURLs();
		foreach ($myUrls as $urlInfo) {
			foreach (array('http', 'https') as $schema) {
				if ($urlInfo[$schema] == 0) {
					continue;
				}
				$fullUrl = $schema.'://'.$urlInfo['url'];
				if ($fullUrl == $url) {
					trigger_localised_error('CMS0111', E_USER_WARNING);
					return FALSE;
				}
			}
		}

		$options['ssl_verifypeer']=$this->attr('verify_ssl');
		$result = fetch_url($url, $options);

		// remove temp folder and files
		if(isset($temp_dirs)) {
		    foreach ($temp_dirs as $dir) {
			$files = glob($dir.'/*'); // get all file names
			foreach($files as $file){ // iterate files
			  if(is_file($file))
			    unlink($file); // delete file
			}
			rmdir($dir);
		    }
		}


		// save cookie content to session
		if(is_file($cookie_filename)) {
		    $_SESSION['SQ_REMOTE_CONTENT_COOKIE'][$this->id] =  file_get_contents($cookie_filename);
		    unlink($cookie_filename);
		}

		if ($result['errornumber'] != 0) {
			trigger_localised_error('CMS0063', E_USER_WARNING, $url, $result['errorstring']);
			return FALSE;
		}

		if (preg_match('/^[45].*?/', $result['curlinfo']['http_code'])) {
			trigger_localised_error('CMS0064', E_USER_WARNING, $result['curlinfo']['http_code'], $url);
			return FALSE;
		}

		$response = $result['response'];

		$this->_tmp['url'] = $result['curlinfo']['url'];

		// check if we were redirected to the bad destination
		if ($this->_tmp['url'] != $url) {
			if (!$this->isUrlAllowed($this->_tmp['url'])) {
				trigger_localised_error('CMS0058', E_USER_WARNING, $this->_tmp['url'], $url);
				return FALSE;
			}
		}

		$keep_headers = array();
		foreach (array('content_type', 'content_disposition') as $_header) {
			if (isset($result['curlinfo'][$_header]) === TRUE) {
				$keep_headers[str_replace('_', '-', $_header)] = $result['curlinfo'][$_header];
			}
		}
		// prepare temporary variables for output
		$this->_tmp['headers'][$this->_tmp['url']] = $keep_headers;
		$this->_tmp['content'][$this->_tmp['url']] = $response;

		// wipe the response because we won't use it again and it could be too big to just ignore

		if ($this->_shouldPassStraightThrough($this->_tmp['headers'][$this->_tmp['url']])) {
			foreach ($this->_tmp['transform_settings'] as $i => $v) {
				$this->_tmp['transform_settings'][$i] = FALSE;
			}
		} else {
			$this->_transformContent($this->_tmp['content'][$this->_tmp['url']]);
		}

		// Mark the successful processing of this url
		$this->_tmp['processed'][$url] = TRUE;

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

		// Replace tags
		if ($this->_tmp['transform_settings']['do_tag_replacements']) {
			$this->_doTagReplacements($content);
		}

		// Strip the images if necessary
		if ($this->attr('strip_images') && $this->_tmp['transform_settings']['strip_images']) {
			$this->_stripAllImages($content);
		}//end if

		// Rewrite URLs
		if ($this->_tmp['transform_settings']['rewrite_urls']) {
			$this->_rewriteURLs($content);
		}

		// Replace keywords
		if ($this->_tmp['transform_settings']['replace_matrix_keywords']) {
			preg_match_all('/%(\w+)%/i', $content, $matches);
			foreach ($matches[1] as $key => $keyword) {
				$rep = $this->getKeywordReplacement($keyword);
				$content = str_replace($matches[0][$key], $rep, $content);
			}
		}

		// Store the input file type's names if they exist
		$this->_addInputFileNames($content);

	}//end _transformContent()


	/*
	 * Add the names of file inputs (<input type="file">) into the HTML content under an hidden field SQ_REMOTE_INPUT_FILE_NAMES
	 *
	 * @param string	&$content	The HTML content this remote content page will display
	 *
	 */
	function _addInputFileNames(&$content) {
		$pattern = '/<input\s+[^>]*type\s*=\s*(?:"|\')?file(?:"|\')?[^>]*>/i';
		$file_names = Array();
		if (preg_match_all($pattern, $content, $matches)) {
			//the following pattern can not get names with space inside like name="hello world" => it only extracts "hello"
			$name_pattern = '/\s+name\s*=\s*(?:"|\')?([^>"\'\s]+)/i';
			foreach($matches[0] as $match) {
				if (preg_match($name_pattern, $match, $name_matches)) {
					$file_names[] = $name_matches[1];
				}
			}
		}

		if (!empty($file_names)) {
			$serialized_names = serialize($file_names);
			require_once SQ_LIB_PATH.'/html_form/html_form.inc';
			ob_start();
			hidden_field($this->getPrefix().'_'.SQ_REMOTE_INPUT_FILE_NAMES, $serialized_names);
			$hidden_field = ob_get_clean();
			//put the hidden field right after the first input file type
			$content = str_replace($matches[0][0], $matches[0][0].$hidden_field, $content);
			// accept file upload
			foreach ($file_names as $f_name) {
				$_SESSION['SQ_REMOTE_CONTENT_UPLOAD_FILE'][$f_name] = TRUE;
			}
		}

	}//end _addInputFileNames()

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
		$multiple = $this->attr('multi_occurence');
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
				$response_body = '';
				if ($multiple) {
					foreach ($matches[1] as $match) {
						$response_body .= $match;
					}
				} else {
					$response_body .= $matches[1][0];
				}
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
		// links and frames except mailto and javascript and tel (URI Schemes)
		$patterns[] = '/(<(?:(?:a(?:rea)?[^>]*?href)|(?:i?frame[^>]*?src))\s*=\s*[\'"]?)\s*((?:(?![\#]|mailto|javascript|tel))[^\'"> #]+)([\'"> #]?[^<]*>)/i';

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
	* Replace tags as specified by the user
	*
	* @param string	&$content	The content to do the tag replacements in
	*
	* @return void
	* @access private
	*/
	function _doTagReplacements(&$content)
	{
		$tag_replacements = $this->attr('tag_replacements');
		if (!empty($tag_replacements)) {
			foreach ($tag_replacements as $tag => $replace_info) {
				if ($replace_info['remove_content']) {
					$pattern = '/(<(?:'.$tag.')(?:\s+[^>]*?)?>[^<]*)/i';
				} else {
					$pattern = '/(<(?:'.$tag.')(?:\s+[^>]*?)?>)/i';
				}
				$content = preg_replace($pattern, $replace_info['start_tag'], $content);
				$pattern = '/(<(?:\/'.$tag.')[^>]*?>)/i';
				$content = preg_replace($pattern, $replace_info['end_tag'], $content);
			}
		}

	}//end _doTagReplacements()


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
			if (substr($path,-1) != '/' && !preg_match('/^\?/', $url)) {
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

		// Very conservative fix for '&amp;'. decode HTML entity only if '&amp;' is found.
		if (strpos($url, '&amp;') !== FALSE) {
			$components = Array('url' => html_entity_decode($url));
		} else {
			$components = Array('url' => $url);
		}

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
		if ($url == $this->attr('remote_url')) return TRUE;

		$options = $this->attr('url_rewriting_options');
		if ($options['all']) return TRUE;

		if (!isset($this->_tmp['_white_list_'])) {
			$url_parts = parse_url($this->attr('remote_url'));
			$host      = $url_parts['host'];
			$path      = '';
			if (isset($url_parts['path']) === TRUE) {
				$path = $url_parts['path'];
			}

			$this->_tmp['_white_list_'] = Array();
			if ($options['remote_domain']) {
				$this->_tmp['_white_list_'][] = strtolower($host);
			}
			if ($options['remote_url']) {
				$this->_tmp['_white_list_'][] = strtolower($host.$path);
			}
			if ($options['white_list']) {
				$this->_tmp['_white_list_'] = array_merge($this->attr('url_white_list'), $this->_tmp['_white_list_']);
			}
		}

		if (empty($this->_tmp['_white_list_'])) return FALSE;

		$url_parts = parse_url($url);

		// fix for #6326 remote content url tunneling php notices
		// better  error check than to throw  notices in the logs
		if (isset($url_parts['host']) === TRUE) {
			$url = $url_parts['host'];
		} else {
			$url = '';
		}

		if (isset($url_parts['path']) === TRUE) {
			$url .= $url_parts['path'];
		}
		$url = strtolower($url);

		// see if there is a URL before goign any further
		// the the patch was relative or something like
		// file://blah/blah then the url var below will be blank
		// we are not going to use this anyways in that case
		if (empty($url)) return FALSE;

		foreach ($this->_tmp['_white_list_'] as $white_url) {
			$white_url_len = strlen(trim($white_url));
			if ($white_url_len > strlen($url)) {
				continue;
			} else if (strtolower(trim($white_url)) == substr($url, 0, $white_url_len)) {
				return TRUE;
			}
		}

		return FALSE;

	}//end isUrlAllowed()


}//end class

?>
