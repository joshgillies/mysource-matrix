<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: www.inc,v 1.6 2003/10/03 04:10:05 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


/**
* WWW Functions
*
* Purpose
*     Any Functions for dealing with the web, it's paths etc
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Fudge
* @subpackage General
*/

/**
* Takes a two URLs and returns the relative HREF required to get
* you from the first URL to the second.
*
* @param string	$from	the absolute URL to come from, in the form protocol://domain/dir/dir2/file
* @param string	$to		the absolute URL to get to, in the form protocol://domain/dir/dir2/file
*
* @return string
* @access public
*/
function relative_href($from,$to)
{
	$from_bits = preg_split('/\\/+/', trim($from));
	$to_bits   = preg_split('/\\/+/', trim($to));

	// Ensure the first element is the protocol
	if(!preg_match('/^[A-Za-z]+:$/', $from_bits[0])) array_unshift($from_bits, 'http:');
	if(!preg_match('/^[A-Za-z]+:$/', $to_bits[0]  )) array_unshift($to_bits,   'http:');
	// Different protocols or domains? ABSOLUTE HREF!
	if(strtolower($from_bits[0]) != strtolower($to_bits[0]) || strtolower($from_bits[1]) != strtolower($to_bits[1])) {
		$to_bits[0] .= '/';
		return implode('/', $to_bits);
	}

	// Different first directories? Root path!
	if(empty($from_bits[2]) || empty($to_bits[2]) || $from_bits[2] != $to_bits[2]) {
		// Shift off protocol and domain
		array_splice($to_bits,   0, 2);
		return '/'.implode('/', $to_bits);
	}

	// Start from the second directory and find the place where the urls start to vary
	$split_point = 3; 
	while(!empty($from_bits[$split_point]) 
			&& !empty($to_bits[$split_point]) 
			&& $from_bits[$split_point] == $to_bits[$split_point]) {
		$split_point++;
	}

	$href = str_repeat('../', count($from_bits) - $split_point);
	// Forward to the destination
	for($i = $split_point - 1, $max = count($to_bits); $i < $max; $i++) {
		$href .= $to_bits[$i].'/';
	}

	// if they wanted a trailing slash make sure there is one, else remove it
	if (substr($to, -1) == '/')	$href = preg_replace('/\\/+$/', '/', $href);
	else						$href = preg_replace('/\\/$/', '', $href);

	// No remove any bit where we go back a directory then forward again to the 
	// same last common directory
	return str_replace('../'.$from_bits[$split_point - 1].'/', '', $href);

}// end relative_href()


/**
* Checks an email address for validity
*
* @param string	$e	the email address to check
*
* @return boolean
* @access public
*/
function valid_email($e) {
	return preg_match("|^[_\.0-9a-zA-Z-]+@([0-9a-zA-Z]([0-9A-Za-z-]*)+\.)+[a-zA-Z]{2,4}$|",$e);
}

?>
