<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: www.inc,v 1.33 2009/01/18 23:45:05 lwright Exp $
*
*/


/**
* WWW Functions
*
* Purpose
*     Any Functions for dealing with the web, it's paths etc
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.33 $
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

	$local = '\da-zA-Z-_+';
	// dot character cannot be the first or last character in the local-part.
	$local_middle = $local.'.\w';

	if (preg_match('/^[a-zA-Z]+(([\'\,\.\- ][a-zA-Z ])?[a-zA-Z]*)*\s+<(['.$local.'](['.$local_middle.']*['.$local.']){0,1}@(((?:[\da-zA-Z]|[\da-zA-Z][\'-\w]*[\da-zA-Z])\.)+[a-zA-Z]{2,7}))>$/', $e)) {
		// matches email with display name, for example, 'Someone <some.one@squiz.net>'
		return TRUE;
	} else if (preg_match('/^(['.$local.'](['.$local_middle.'\']*['.$local.']){0,1}@(((?:[\da-zA-Z]|[\da-zA-Z][\'-\w]*[\da-zA-Z])\.)+[a-zA-Z]{2,7}))$/', $e)) {
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


?>
