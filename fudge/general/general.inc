<?php

/**
* General Functions
*
* Purpose
*     Generic functions that are useful, but don't fall into any other category
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Fudge
* @subpackage General
*/

/**
* Strips the slashes from a variable if magic quotes is set for GPC
* Handle normal variables and arrays 
*
* @param mixed $var	the var to cleanup
*
* @return mixed
*/
function gpc_stripslashes($var) 
{
	if (get_magic_quotes_gpc()) {
		if (is_array($var)) stripslashes_array($var, true);
		else $var = stripslashes($var);
	}
	return $var;
}

/**
* Strips the slashes from an entire associative array
*
* @param array $array           the array to stripslash
* @param boolean $strip_keys    whether or not to stripslash the keys as well
*
* @return Array()
*/
function stripslashes_array(&$array, $strip_keys=false) 
{
	if(is_string($array)) return stripslashes($array);
	$keys_to_replace = Array();
	foreach($array as $key => $value) {
		if (is_string($value)) {
			$array[$key] = stripslashes($value);
		} elseif (is_array($value)) {
			stripslashes_array($array[$key], $strip_keys);
		}
		
		if ($strip_keys && $key != ($stripped_key = stripslashes($key))) {
			$keys_to_replace[$key] = $stripped_key;
		}
	}
	// now replace any of the keys that needed strip slashing
	foreach($keys_to_replace as $from => $to) {
		$array[$to]   = &$array[$from];
		unset($array[$from]);
	}
	return $array;
}


/**
* Generates a random password from letters/digits
*
* @param int $length    the length of the password
*
* @return Array()
*/
function random_password($length) 
{
	if (!$length) $length = 8;
	$consonants = 'bcdfghjklmnpqrstvwyz';
	$vowels     = 'aeiou';
	$password   = '';
	global $RANDOM_PASSWORD_SEED;
	while($length > 0) {
		$RANDOM_PASSWORD_SEED += $length;
		$seed = $RANDOM_PASSWORD_SEED + $length + ((int)(100000 * (double)microtime())) + time();
		srand($seed);
		$password .= $consonants[rand(0,strlen($consonants)-1)];
		if($length > 1) {
			srand($seed);
			$password .= $vowels[rand(0,strlen($vowels)-1)];
		}
		$length -= 2;
	}
	$RANDOM_PASSWORD_SEED++;
	return $password;
}

/**
* Chops a string at a certain maximum length and appends an ellipsis
* ignores HTML tags
*
* @param string	$string    the string to manipulate
* @param int	$length    the length of the password
*
* @return string
*/
function ellipsisize($string, $length)
{
	$full_length = strlen($string);
	$length += 2; // add 2 because an elipsis is 3 chars long
	$on = true;
	$j = 0;
	for($i = 0; $i < $full_length; $i++) {
		if ($j == $length - 2)	$chophere = $i;
		if ($on) {
			if ($string[$i] == '<') $on = false;
			else $j++;
		} else {
			if ($string[$i] == '>') $on = true;
		}
	}
	if ($length < $j) {
		$string = substr($string, 0, $chophere)."...";
	}
	return $string;
}// end ellipsisize()

?>