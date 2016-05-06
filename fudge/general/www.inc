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
* $Id: www.inc,v 1.44 2012/08/30 00:56:51 ewang Exp $
*
*/


/**
* WWW Functions
*
* Purpose
*     Any Functions for dealing with the web, it's paths etc
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.44 $
* @package Fudge
* @subpackage General
*/


/**
* Takes a two URLs and returns the relative HREF required to get
* you from the first URL to the second.
*
* @param string	$from	the absolute URL to come from, in the form protocol://domain/dir/dir2/file
* @param string	$to		the absolute URL to get to, in the form protocol://domain/dir/dir2/dir3
*
* @return string
* @access public
*/
function relative_href($from,$to)
{
	// type verification
	if (!is_string($from) || !is_string($to)) {
		return '';
	}

	$from_bits = preg_split('/\\/+/', trim($from));
	$to_bits   = preg_split('/\\/+/', trim($to));

	// Ensure the first element is the protocol
	if (!preg_match('/^[A-Za-z]+:$/', $from_bits[0])) {
		array_unshift($from_bits, 'http:');
	}
	if (!preg_match('/^[A-Za-z]+:$/', $to_bits[0]  )) {
		array_unshift($to_bits,   'http:');
	}
	// Different protocols or domains? ABSOLUTE HREF!
	if (strtolower($from_bits[0]) != strtolower($to_bits[0]) || strtolower($from_bits[1]) != strtolower($to_bits[1])) {
		$to_bits[0] .= '/';
		return implode('/', $to_bits);
	}

	// Different first directories? Root path!
	if (empty($from_bits[2]) || empty($to_bits[2]) || $from_bits[2] != $to_bits[2]) {
		// Shift off protocol and domain
		array_splice($to_bits,   0, 2);
		return '/'.implode('/', $to_bits);
	}

	// Start from the second directory and find the place where the urls start to vary
	$split_point = 3;
	while (!empty($from_bits[$split_point])
			&& !empty($to_bits[$split_point])
			&& $from_bits[$split_point] == $to_bits[$split_point]) {
		$split_point++;
	}

	$href = str_repeat('../', count($from_bits) - $split_point);
	// Forward to the destination
	for ($i = $split_point - 1, $max = count($to_bits); $i < $max; $i++) {
		$href .= $to_bits[$i].'/';
	}

	// if they wanted a trailing slash make sure there is one, else remove it
	if (substr($to, -1) == '/') {
		$href = preg_replace('/\\/+$/', '/', $href);
	} else {
		$href = preg_replace('/\\/$/', '', $href);
	}

	// No remove any bit where we go back a directory then forward again to the
	// same last common directory
	return str_replace('../'.$from_bits[$split_point - 1].'/', '', $href);

}//end relative_href()


/**
* Checks an email address for validity
*
* @param string	$e	the email address to check
*
* @return boolean
* @access public
*/
function valid_email($e)
{
	if (!is_string($e)) return FALSE;

    $local = '0-9a-zA-Z_\+\-'.'&\'\*\/=\?\^\{\}~';
	
    // dot character cannot be the first or last character in the local-part
	// and it cannot appear two or more times consecutively	
	if (preg_match('/^[a-zA-Z]+(([\'\,\.\- ][a-zA-Z ])?[a-zA-Z]*)*\s+<(['.$local.']+(\.['.$local.']+)*@(((?:[\da-zA-Z]|[\da-zA-Z][\'\-\w]*[\da-zA-Z])\.)+[a-zA-Z]{2,7}))>$/', $e)) {
        // matches email with display name, for example, 'Someone <some.one@squiz.net>'
        return TRUE;
    } else if (preg_match('/^(['.$local.']+(\.['.$local.']+)*@(((?:[\da-zA-Z]|[\da-zA-Z][\'\-\w]*[\da-zA-Z])\.)+[a-zA-Z]{2,7}))$/', $e)) {
        // matches normal email address
        return TRUE;
    } else {
        return FALSE;
    }
	
}//end valid_email()


/**
* Checks a url for validity
*
* @param string	$url		the url to check
* @param array	$schemes	a list of valid schemes to check against, defaults to 'http' and 'https'
*
* @return boolean
* @access public
*/
function valid_url($url, $schemes=Array('http','https'))
{
	return preg_match('/^('.implode('|', $schemes).'):\/\/[a-z0-9]+(([\-\.]{1}[a-z0-9]+)*\.[a-z]{2,})?((:[0-9]{1,5})?\/.*)?$/i', $url);

}//end valid_url()


/**
* Convert POST or GET array into raw string
*
* @param array	$data	post or get array
*
* @return string
* @access public
*/
function make_raw_post_data($data)
{
	// type verification
	if (!is_array($data) || empty($data)) {
		return '';
	}

	$val_array = NULL;

	foreach ($data as $name => $value) {
		$val_array[] = recursive_make_raw_post_data($name, $value);
	}

	$result = '';
	if (!empty($val_array)) {
		$result = implode('&', $val_array);
	}

	return $result;

}//end make_raw_post_data()


/**
* Convert POST or GET array into raw string recursive
*
* @param string	$name	name of the variable
* @param mixed	$data	value
*
* @return string
* @access public
*/
function recursive_make_raw_post_data($name, $data)
{
	if (!is_array($data)) {
		return $name.'='.rawurlencode($data);
	} else {
		$val_array=Array();

		if (empty($data)) return $name.'=';

		foreach ($data as $key => $value) {
			$val_array[] = recursive_make_raw_post_data($name.'['.$key.']', $value);
		}
		return implode('&', $val_array);
	}

}//end recursive_make_raw_post_data()


/**
* Undo the effect of htmlspecialchars
*
* @param string	$string	string containing html special characters
*
* @return string
* @access public
*/
function undo_htmlspecialchars($string)
{
	$string = str_replace('&amp;','&',$string);
	$string = str_replace('&#039;',"'",$string);
	$string = str_replace('&quot;','"',$string);
	$string = str_replace('&lt;','<',$string);
	$string = str_replace('&gt;','>',$string);

	return $string;

}//end undo_htmlspecialchars()


/**
* Make a valid (X)HTML ID from a string
* W3C's (X)HTML definition notes that ID field can only contain letters, numbers
* and these punctuation marks [:_.-]. First character must be a letter.
*
* Rules for this function:
* - Replace all characters of the ID with the $replace_char
* - If the resulting first character of the ID is not a letter, drop it
*   (regardless of $replace_char - we can't replace with $replace_char because
*    it probably won't be a letter, more likely something like '_').
*
* @param string	$id				The ID to work on
* @param string	$replace_char	The character to replace with (can be blank)
*
* @return mixed string|boolean
* @access public
*/
function make_valid_html_id($id, $replace_char='')
{
	if (!preg_match('/^[A-Za-z0-9:_.-]*$/', $replace_char)) {
		return FALSE;
	}
	$id = preg_replace('/[^A-Za-z0-9:_.-]/', $replace_char, $id);

	// keep dropping first character until we get a valid one
	while (!empty($id) && preg_match('/[^A-Za-z]/', $id{0})) {
		$id = substr($id, 1);
	}
	return $id;

}//end make_valid_html_id()


/**
* Validate the parameter as a valid ip (4) address
*
* @param	string	$address The address to validate
*
* @return boolean
* @access public
*/
function valid_ipaddress($address)
{
	$ips = explode('.', $address);

	if (4 != count($ips)) return FALSE;

	foreach ($ips as $ip) {
		if (!ctype_digit($ip)) return FALSE;
		$val = intval($ip);
		if ((0 > $ip) || (255 < $ip)) {
				return FALSE;
		}
	}
	return TRUE;

}//end valid_ipaddress()


/**
 * It checks for the links pointing to only Matrix assets which are not live.
 * If found, opening and closing anchor tags will be removed keeping the link name.
 *
 * @param string $html html content to be checked
 * @param int	 $owner_id  id of the asset such as standard page and news item which own that html content 
 *
 * @return string|FALSE
 * @access public
 */
function check_non_live_link($html, $owner_id)
{	
	$am = $GLOBALS['SQ_SYSTEM']->am;
	$pattern = '#<a .*?href\s*=\s*(?:\'|")([^\'"]*?)(?:\'|")[^>]*>([^<]*)</a>#msi';	
	$error_link = Array();
	$error_found = FALSE;

	preg_match_all($pattern, $html, $match);
	if (!empty($match)) {	
		/* 
		 * find base url from the owner or its parents 
		 */		
		$owner = $am->getAsset($owner_id);
		if ($owner instanceof Content_Type) {
			$parent   = $am->getParents($owner_id, 'page', FALSE);
			if (!empty($parent)) {
				foreach ($parent as $one_parent_id => $one_parent_type_code) {
					$parent_id = $one_parent_id;
					break 1;
				}
				$base_url = $am->getAssetURL($parent_id);
			}
		} else {
			$base_url = $am->getAssetURL($owner_id);	
		}
	
		foreach ($match[1] as $index => $url) {			
			$whole_link = $match[0][$index];
			$href 	    = $match[1][$index];
			$link_name  = $match[2][$index];			

			if (empty($url)) continue;

			if (empty($link_name)) $error_message = $href;			
			
			$pattern = '#\\?.*a=([\s0-9:v]+).*?#msi';
			preg_match($pattern, $url, $asset_id);			
			// if url is like './?a=250:v1'
			if (!empty($asset_id[0])) {							
				$asset = $am->getAsset(trim($asset_id[1]), '', TRUE);
			} else {				
				
				if (empty($base_url)) {
					continue;
				}

				// if there is no protocol, the url must be relative.			
				if (parse_url($url, PHP_URL_SCHEME) === NULL) {																		
					$url = relative_to_absolute_url($url, $base_url);
				}

				$scheme = '';
				$host   = '';
				$path   = '';
	    		// parse base URL and convert to local variables: $scheme, $host, $path
    			extract(parse_url($url));
    			$host_path = $host.$path;
				if ($scheme === 'http' || $scheme === 'https') {
					$root_url = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);  
					$is_matrix_url = FALSE;
					foreach($root_url as $one_root_url) {
						if (strpos($host_path, $one_root_url) === 0) {
							$is_matrix_url = TRUE;
							break 1;
						}
					}				

					if ($is_matrix_url) {							
						/* 					
						* find out whether url is remapped 	
						*/
						$rm = $am->getSystemAsset('remap_manager');
												
						$search = '#'.$scheme.'\\://#';						
						// This URL may contain parameters
						$url_with_params = preg_replace($search, '', $url);									
						// This URL is without parameters
						$url_without_params = $host_path;
	
						// Firstly, look for an exact match in the database
						$final_url = $rm->getRemapFromURL($scheme, $url_with_params);						
						if ($final_url === FALSE && ($url_with_params !== $url_without_params)) {
							// Fall back to the URL without parameters if there is no a match
							$final_url = $rm->getRemapFromURL($scheme, $url_without_params);						
						}

						if ($final_url !== FALSE) {
							$url    = $final_url;
							$scheme = '';
							$host   = '';
							$path   = '';
	    					// parse base URL again and convert to local variables: $scheme, $host, $path
    						extract(parse_url($url));														
						}

						$asset =  $am->getAssetFromURL($scheme, $host_path, TRUE, TRUE);
					} else {
						// don't worry about non-Matrix url or incorrect url
						continue;
					} // end if ($is_matrix_url)

				} else {
					// don't worry about other protocol or incorrect url
					continue;
				}// end if ($scheme === 'http' || $scheme === 'https')
			}							

			if (!empty($asset) && $asset->status != SQ_STATUS_LIVE) {
				$error_found = TRUE;						
				$error_link[] = ($link_name === '') ? $error_message : $link_name;
				$html  = str_replace($whole_link, $link_name, $html);				
			}									
		}// end checking each url
	}

	if ($error_found) {		
		$live_link_only_error = translate('Creating hyperlink from live asset to non-live asset is forbidden by Preference. The anchor tags for the following link have been removed.');
		foreach ($error_link as $one_link) {
			$error_message = $live_link_only_error.' \''.$one_link.'\'';					
			trigger_error($error_message, E_USER_WARNING);
		}
	}
	return $html;

}//end check_non_live_link()


/**
 * It converts the provided relative url to absolute url using the provided base url.
 * 
 * NEED TO KNOW: 
 * 1. Conversion only works for Matrix relative url which needs to be processed one level below. 
 *    e.g. base url of http://www.google.com/a/contact and relative url of ../info
 *    Normal absolute url should be http://www.google.com/info
 *    But Matrix absolute url would be http://www.google.com/a/info
 * 
 * @param string $relative relative url 
 * @param string $base     base url to be used to convert relative url to absolute url 
 *
 * @return string
 * @access public
 */
function relative_to_absolute_url($relative, $base)
{
    /* return if already absolute URL */
    if (parse_url($relative, PHP_URL_SCHEME) != '') return $relative;   

    /* queries and anchors */
    if ($relative[0]=='#' || $relative[0]=='?') return $base.$relative;    
    if (strpos($relative, './?') === 0) return $base.substr($relative, 1);

    /* parse base URL and convert to local variables:
       $scheme, $host, $path */
    extract(parse_url($base));

    /* destroy path if relative url points to root */
    if ($relative[0] == '/') $path = '';   

    /* dirty absolute URL */
    $absolute = "$host$path/$relative"; 

    /* replace '//' or '/./' or '/foo/../' with '/' */
    $re = Array('#(/\.?/)#', '#/(?!\.\.)[^/]+/\.\./#');
    for ($n=1; $n>0; $absolute=preg_replace($re, '/', $absolute, -1, $n)) {}  

    /* absolute URL is ready! */
    return $scheme.'://'.$absolute;

}

?>
